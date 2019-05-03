/*	Revil Format Library
	Copyright(C) 2017-2019 Lukas Cone

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#include <map>
#include "REAsset.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"
#include "datas/allocator_hybrid.hpp"


template<class C> esIntPtr tmpltDummy() { C cls; return reinterpret_cast<esIntPtr &>(cls); }

static const std::map<uint, esIntPtr (*)()> reassetRegistry =
{
	{REMotlist::ID, tmpltDummy<REMotlist>},
	{REMotion::ID, tmpltDummy<REMotion>},
};

union FourCCHelper
{
	char CCString[5];
	uint CCNumber;
};


REAsset::~REAsset()
{
	if (masterBuffer)
		free(masterBuffer);
}

int REAsset::Assign(char *ptr)
{
	assetID = reinterpret_cast<uint &>(*ptr);
	assetFourCC = reinterpret_cast<uint &>(*(ptr + 4));

	if (!reassetRegistry.count(assetFourCC))
	{
		FourCCHelper hlp = {};
		hlp.CCNumber = assetFourCC;

		printerror("[REAsset] Unhandled asset type: ", << std::hex << assetFourCC << '(' << hlp.CCString << ')');
		return 2;
	}

	object = reinterpret_cast<REAssetBase *>(ptr + 8);
	reinterpret_cast<esIntPtr &>(*object) = reassetRegistry.at(assetFourCC)();;
	object->Fixup(ptr);

	return 0;
}

template<class _Ty0> 
int REAsset::_Load(const _Ty0 *fileName)
{
	BinReader rd(fileName);

	if (!rd.IsValid())
	{
		printerror("[REAsset] Couldn't open file: ", << fileName);
		return 1;
	}

	return Load(&rd);
}

template int REAsset::_Load(const wchar_t *fileName);
template int REAsset::_Load(const char *fileName);

int REAsset::Load(BinReader *rd)
{
	const size_t fleSize = rd->GetSize();
	masterBuffer = static_cast<char *>(malloc(fleSize));

	if (reinterpret_cast<esIntPtr &>(masterBuffer) < fleSize)
	{
		printerror("[REAsset] Malloc just allocated memory within range of loaded asset. Please eat some memory and try again.");
		return 5;
	}

	rd->ReadBuffer(masterBuffer, fleSize);

	return Assign(masterBuffer);
}

int REMotlist::Fixup(char *masterBuffer)
{
	motions.Fixup(masterBuffer);
	unkOffset00.Fixup(masterBuffer);
	fileName.Fixup(masterBuffer);
	null.Fixup(masterBuffer);

	for (int m = 0; m < numMotions; m++)
	{
		motions[m].Fixup(masterBuffer);
		REAsset ass;
		ass.Assign(reinterpret_cast<char *>(motions[m].ptr));
		motions[m].ptr = ass.Object();
	}

	REAssetBase *huh = (*(motions)).ptr;

	return 0;
}

int REMotion::Fixup(char *masterBuffer)
{
	bones.Fixup(masterBuffer);
	bones->ptr.Fixup(masterBuffer);
	tracks.Fixup(masterBuffer);
	unkOffset02.Fixup(masterBuffer);
	animationName.Fixup(masterBuffer);

	for (int b = 0; b < numBones; b++)
		bones->ptr[b].Fixup(masterBuffer);

	for (int b = 0; b < numTracks; b++)
		tracks[b].Fixup(masterBuffer);

	return 0;
}

int REMotionBone::Fixup(char *masterBuffer)
{
	boneName.Fixup(masterBuffer);
	parentBoneNamePtr.Fixup(masterBuffer);
	firstChildBoneNamePtr.Fixup(masterBuffer);
	lastChildBoneNamePtr.Fixup(masterBuffer);

	return 0;
}

int REMotionTrack::Fixup(char *masterBuffer)
{
	curves.Fixup(masterBuffer);

	int numUsedCurves = 0;

	for (int t = 0; t < 3; t++)
		if (usedCurves[static_cast<TrackType>(t)])
			curves[numUsedCurves++].Fixup(masterBuffer);

	return 0;
}

struct RETrackController_internal : RETrackController
{
	enum FrameType
	{
		FrameType_short = 4,
		FrameType_char = 2
	};
	RETrackCurve *curve;
	FrameType frameType;
	int componentID;
	virtual void Assign() = 0;
	void Assign(RETrackCurve *iCurve)
	{
		curve = iCurve;
		frameType = static_cast<FrameType>((curve->flags >> 20) & 0xf);
		Assign();
	}
	ushort GetFrame(int id) const
	{
		if (frameType == FrameType_short)
			return *(reinterpret_cast<ushort *>(curve->frames.ptr) + id);
		else
			return *(curve->frames.ptr + id);
	}
};

struct LinearVector3Controller : RETrackController_internal
{
	static const int ID = 0xF2;
	typedef std::allocator_hybrid<Vector> Alloc_Type;
	typedef std::vector<Vector, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	void Assign()
	{
		Vector *start = reinterpret_cast<Vector *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const Vector &retreived = dataStorage[id];
		out.X = retreived.X;
		out.Y = retreived.Y;
		out.Z = retreived.Z;
	}
};

struct LinearQuat3Controller : RETrackController_internal
{
	static const int ID = 0xB0112;
	typedef std::allocator_hybrid<Vector> Alloc_Type;
	typedef std::vector<Vector, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	void Assign()
	{
		Vector *start = reinterpret_cast<Vector *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const Vector &retreived = dataStorage[id];
		out.X = retreived.X;
		out.Y = retreived.Y;
		out.Z = retreived.Z;
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}
};

struct BiLinearQuat3_10bitController : RETrackController_internal
{
	static const int ID = 0x30112;
	typedef std::allocator_hybrid<uint> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	static const int componentMask = 0x3ff;
	static const float componentMultiplier;

	void Assign()
	{
		uint *start = reinterpret_cast<uint *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const uint &retreived = dataStorage[id];
		out.X = retreived & componentMask;
		out.Y = (retreived >> 10) & componentMask;
		out.Z = (retreived >> 20) & componentMask;
		out = curve->minMaxBounds->max + (curve->minMaxBounds->min * (out * componentMultiplier));
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}
};

const float BiLinearQuat3_10bitController::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct BiLinearQuat3_21bitController : RETrackController_internal
{
	static const int ID = 0x70112;
	typedef std::allocator_hybrid<uint64> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	static const int componentMask = (1 << 21) - 1;
	static const float componentMultiplier;

	void Assign()
	{
		uint64 *start = reinterpret_cast<uint64 *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const uint64 &retreived = dataStorage[id];
		out.X = retreived & componentMask;
		out.Y = (retreived >> 21) & componentMask;
		out.Z = (retreived >> 42) & componentMask;
		out = curve->minMaxBounds->max + (curve->minMaxBounds->min * (out * componentMultiplier));
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}
};

const float BiLinearQuat3_21bitController::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct BiLinearSingleComponentQuatController : RETrackController_internal
{
	static const int ID1 = 0x21112;
	static const int ID2 = 0x22112;
	static const int ID3 = 0x23112;
	typedef std::allocator_hybrid<ushort> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	static const int componentMask = (1 << 16) - 1;
	static const float componentMultiplier;

	void Assign()
	{
		ushort *start = reinterpret_cast<ushort *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const ushort &retreived = dataStorage[id];
		out[componentID] = curve->minMaxBounds->min[1] + (curve->minMaxBounds->min[0] * (static_cast<float>(retreived) * componentMultiplier));
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}
};

const float BiLinearSingleComponentQuatController::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct BiLinearSingleComponentVector3Controller : RETrackController_internal
{
	static const int ID1 = 0x210F2;
	static const int ID2 = 0x220F2;
	static const int ID3 = 0x230F2;
	typedef std::allocator_hybrid<ushort> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	static const int componentMask = (1 << 16) - 1;
	static const float componentMultiplier;

	void Assign()
	{
		ushort *start = reinterpret_cast<ushort *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const ushort &retreived = dataStorage[id];
		out.X = curve->minMaxBounds->min.Y;
		out.Y = curve->minMaxBounds->min.Z;
		out.Z = curve->minMaxBounds->min.W;

		out[componentID] = curve->minMaxBounds->min[0] + (curve->minMaxBounds->min[componentID + 1] * (static_cast<float>(retreived) * componentMultiplier));
	}
};

const float BiLinearSingleComponentVector3Controller::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct LinearSingleComponentVector3Controller : RETrackController_internal
{
	static const int ID1 = 0x310F2;
	static const int ID2 = 0x320F2;
	static const int ID3 = 0x330F2;
	typedef std::allocator_hybrid<float> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	void Assign()
	{
		float *start = reinterpret_cast<float *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const float &retreived = dataStorage[id];
		out = curve->minMaxBounds->min;
		out[componentID] = retreived;
	}
};

struct LinearSingleComponentQuatController : RETrackController_internal
{
	static const int ID1 = 0x31112;
	static const int ID2 = 0x32112;
	static const int ID3 = 0x33112;
	typedef std::allocator_hybrid<float> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	void Assign()
	{
		float *start = reinterpret_cast<float *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const float &retreived = dataStorage[id];
		out[componentID] = retreived;
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}
};

struct BiLinearSingleComponentScaleController : RETrackController_internal
{
	static const int ID = 0x240F2;
	typedef std::allocator_hybrid<ushort> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	static const int componentMask = (1 << 16) - 1;
	static const float componentMultiplier;

	void Assign()
	{
		ushort *start = reinterpret_cast<ushort *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const ushort &retreived = dataStorage[id];
		out.X = curve->minMaxBounds->min[0] + (curve->minMaxBounds->min[1] * (static_cast<float>(retreived) * componentMultiplier));
		out.Y = out.X;
		out.Z = out.X;
	}
};

const float BiLinearSingleComponentScaleController::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct LinearSingleComponentScaleController : RETrackController_internal
{
	static const int ID = 0x340F2;
	typedef std::allocator_hybrid<float> Alloc_Type;
	typedef std::vector<Alloc_Type::value_type, Alloc_Type> Storage_Type;
	Storage_Type dataStorage;

	void Assign()
	{
		float *start = reinterpret_cast<float *>(curve->controlPoints.ptr);
		dataStorage = Storage_Type(start, start + curve->numFrames, Alloc_Type(start));
	}

	void Evaluate(int id, Vector4 &out) const
	{
		const float &retreived = dataStorage[id];
		out.X = retreived;
		out.Y = out.X;
		out.Z = out.X;
	}
};

template<class C> RETrackController_internal *controlDummy() { return new C; }

static const std::map<int, RETrackController_internal *(*)()> curveControllers =
{
	{LinearVector3Controller::ID, controlDummy<LinearVector3Controller>},
	{LinearQuat3Controller::ID, controlDummy<LinearQuat3Controller>},
	{BiLinearQuat3_10bitController::ID, controlDummy<BiLinearQuat3_10bitController>},
	{BiLinearQuat3_21bitController::ID, controlDummy<BiLinearQuat3_21bitController>},
	{BiLinearSingleComponentQuatController::ID1, controlDummy<BiLinearSingleComponentQuatController>},
	{BiLinearSingleComponentQuatController::ID2, controlDummy<BiLinearSingleComponentQuatController>},
	{BiLinearSingleComponentQuatController::ID3, controlDummy<BiLinearSingleComponentQuatController>},

	{BiLinearSingleComponentVector3Controller::ID1, controlDummy<BiLinearSingleComponentVector3Controller>},
	{BiLinearSingleComponentVector3Controller::ID2, controlDummy<BiLinearSingleComponentVector3Controller>},
	{BiLinearSingleComponentVector3Controller::ID3, controlDummy<BiLinearSingleComponentVector3Controller>},
	
	{LinearSingleComponentVector3Controller::ID1, controlDummy<LinearSingleComponentVector3Controller>},
	{LinearSingleComponentVector3Controller::ID2, controlDummy<LinearSingleComponentVector3Controller>},
	{LinearSingleComponentVector3Controller::ID3, controlDummy<LinearSingleComponentVector3Controller>},

	{LinearSingleComponentQuatController::ID1, controlDummy<LinearSingleComponentQuatController>},
	{LinearSingleComponentQuatController::ID2, controlDummy<LinearSingleComponentQuatController>},
	{LinearSingleComponentQuatController::ID3, controlDummy<LinearSingleComponentQuatController>},

	{BiLinearSingleComponentScaleController::ID, controlDummy<BiLinearSingleComponentScaleController>},
	{LinearSingleComponentScaleController::ID, controlDummy<LinearSingleComponentScaleController>},

};

RETrackController *RETrackCurve::GetController()
{
	const int type = flags & 0xff0fffff;
	const int componentID = (flags >> 12) & 0xf;

	RETrackController_internal *iCon = nullptr;

	if (curveControllers.count(type))
	{
		iCon = curveControllers.at(type)();
		iCon->Assign(this);
		iCon->componentID = componentID - 1;
	}
	else
	{
		printerror("[RETrackController]: Unhandled curve compression: ", << std::hex << type);
	}

	return iCon;
}

int RETrackCurve::Fixup(char *masterBuffer)
{
	frames.Fixup(masterBuffer);
	controlPoints.Fixup(masterBuffer);
	minMaxBounds.Fixup(masterBuffer);

	return 0;
}
