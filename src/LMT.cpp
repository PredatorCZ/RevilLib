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
#include <array>
#include <algorithm>
#include "LMT.h"
#include "pugixml.hpp"
#include "datas/disabler.hpp"
#include "datas/reflector.hpp"
#include "datas/flags.hpp"
#include "datas/pugiex.hpp"
#include "datas/endian.hpp"
#include "datas/vectors.hpp"
#include "datas/fileinfo.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/masterprinter.hpp"
#include "datas/allocator_hybrid.hpp"
#include "datas/reflectorRegistry.hpp"

//#define FIX_C2970

struct LMTFixupStorage
{
	struct _data
	{
		uint from,
			to;
	};

	std::vector<_data> fixupStorage;
	uint toIter = 0;

	void SaveFrom(uint offset)
	{
		fixupStorage.push_back({ offset, 0 });
	}

	void SaveTo(BinWritter *wr)
	{
		fixupStorage[toIter++].to = wr->Tell();
	}

	void SkipTo()
	{
		toIter++;
	}
};

static const char *idents[] =
{
	"",
	"\t",
	"\t\t",
	"\t\t\t",
	"\t\t\t\t",
	"\t\t\t\t\t",
};

template<class C>
union PointerX64 
{
	uint64 fullPtr;
	C *ptr;
	int offset;
	esIntPtr varPtr;

	static const esIntPtr mask = ~static_cast<esIntPtr>(1);

	PointerX64() : fullPtr(0) {}
	ES_FORCEINLINE C *GetData(char *) { return reinterpret_cast<C*>(varPtr & mask); }
	ES_FORCEINLINE void Fixup(char *masterBuffer, bool &swapEndian)
	{ 
		if (swapEndian && !offset)
			FByteswapper(fullPtr);

		if (offset & 1)
		{
			swapEndian = false;
			return;
		}

		if (offset)
			ptr = reinterpret_cast<C *>(masterBuffer + offset);

		offset |= 1;
	}
};

template<class C>
union PointerX86
{
	uint varPtr;

	static const uint mask = ~1U;

	template<bool enbabled = ES_X64>
	ES_FORCEINLINE typename std::enable_if<enbabled, C*>::type GetData(char *masterBuffer)
	{
		uint tempPtr = varPtr & mask;

		if (!tempPtr)
			return nullptr;

		return reinterpret_cast<C *>(masterBuffer + tempPtr);
	}

	template<bool enbabled = ES_X64>
	ES_FORCEINLINE typename std::enable_if<!enbabled, C *>::type GetData(char *)
	{
		return reinterpret_cast<C *>(varPtr & mask);
	}

	ES_FORCEINLINE void Fixup(char *masterBuffer, bool &swapEndian)
	{
		if (swapEndian && !(varPtr & 0x01000001))
			FByteswapper(varPtr);

		if (varPtr & 1)
		{
			swapEndian = false;
			return;
		}

		if (ES_X64 || !varPtr)
		{
			varPtr |= 1;
			return;
		}

		varPtr += reinterpret_cast<uint&>(masterBuffer);
		varPtr |= 1;
	}
};

/************************************************************************/
/***************************** EVENTS ***********************************/
/************************************************************************/

struct AnimEvent
{
	int runEventBit;
	int numFrames;

	Reflector::xmlNodePtr ToXML(pugi::xml_node &node) const
	{
		int numItems = 0;
		pugi::xml_node currentNode = node.append_child("event");

		if (runEventBit)
		{
			std::string resVal;

			for (int b = 0; b < 32; b++)
				if (runEventBit & (1 << b))
				{
					resVal += std::to_string(b) + ", ";
					numItems++;
				}

			resVal.pop_back();
			resVal.pop_back();

			currentNode.append_buffer(resVal.c_str(), resVal.size());
		}

		currentNode.append_attribute("additiveFrames").set_value(numFrames);
		currentNode.append_attribute("numItems").set_value(numItems);

		return currentNode.internal_object();
	}

	void SwapEndian()
	{
		FByteswapper(runEventBit);
		FByteswapper(numFrames);
	}
};

template<template<class C> class PtrType>
struct EventTable
{
	DECLARE_REFLECTOR;
	std::array<short, 32> eventRemaps;
	int	numEvents;
	PtrType<AnimEvent> events;

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		events.Fixup(masterBuffer, swapEndian);

		if (!swapEndian)
			return;

		for (auto &v : eventRemaps)
			FByteswapper(v);

		FByteswapper(numEvents);
	}
};

typedef EventTable<PointerX86> EventTablePointerX86;
REFLECTOR_START_WNAMES(EventTablePointerX86, eventRemaps);

typedef EventTable<PointerX64> EventTablePointerX64;
REFLECTOR_START_WNAMES(EventTablePointerX64, eventRemaps);

/************************************************************************/
/****************************** ENUMS ***********************************/
/************************************************************************/

REFLECTOR_ENUM(TrackType,
	LocalRotation,
	LocalPosition,
	LocalScale,
	AbsoluteRotation,
	AbsolutePosition
)

REFLECTOR_ENUM(TrackV1BufferTypes,
	Unknown,
	SingleVector3,
	SinglePositionVector3,
	Unused00,
	SingleRotationQuat3,
	HermiteVector3,
	SphericalRotation,
	Unused01,
	Unused02,
	LinearVector3
)

REFLECTOR_ENUM(TrackV1_5BufferTypes,
	Unknown,
	SingleVector3,
	LinearVector3,
	Unused00,
	SingleRotationQuat3,
	HermiteVector3,
	LinearRotationQuat4_14bit,
	Unused01,
	Unused02,
	SinglePositionVector3
)

REFLECTOR_ENUM(TrackV2BufferTypes,
	Unknown,
	SingleVector3,
	SingleRotationQuat3,
	LinearVector3,
	BiLinearVector3_16bit,
	BiLinearVector3_8bit,
	LinearRotationQuat4_14bit,
	BiLinearRotationQuat4_7bit,
	Unused02,
	Unused03,
	Unused04,
	BiLinearRotationQuatXW_14bit,
	BiLinearRotationQuatYW_14bit,
	BiLinearRotationQuatZW_14bit,
	BiLinearRotationQuat4_11bit,
	BiLinearRotationQuat4_9bit
)

/************************************************************************/
/**************************** EVALUATORS ********************************/
/************************************************************************/

static const float fPI = 3.14159265;
static const float fPI2 = 0.5 * fPI;

struct TrackMinMax
{
	DECLARE_REFLECTOR;
	Vector4 min;
	Vector4 max;
};

REFLECTOR_START_WNAMES(TrackMinMax, min, max);

//https://en.wikipedia.org/wiki/Slerp
ES_INLINE Vector4 slerp(const Vector4 &v0, Vector4 v1, float t) 
{
	float dot = v0.Dot(v1);

	// If the dot product is negative, slerp won't take
	// the shorter path. Fix by reversing one quaternion.
	if (dot < 0.0f) 
	{
		v1 *= -1;
		dot *= -1;
	}

	static const float DOT_THRESHOLD = 0.9995f;
	if (dot > DOT_THRESHOLD) 
	{
		// If the inputs are too close for comfort, linearly interpolate
		// and normalize the result.

		Vector4 result = v0 + (v1 - v0) * t;
		return result.Normalize();
	}

	const float theta00 = acos(dot);		// theta00 = angle between input vectors
	const float theta01 = theta00 * t;		// theta01 = angle between v0 and result
	const float theta02 = sin(theta01);
	const float theta03 = 1.0f / sin(theta00);
	const float s0 = cos(theta01) - dot * theta02 * theta03;
	const float s1 = theta02 * theta03;

	return (v0 * s0) + (v1 * s1);
}

template <typename T>
ES_INLINE Vector4 lerp(const Vector4 &v0, const Vector4 &v1, T t)
{
	return v0 + (v1 - v0) * t;
}

ES_INLINE Vector4 blerp(const Vector4 &v0, const Vector4 &v1, const TrackMinMax *minMax, float t)
{
	return lerp(lerp(minMax->min, minMax->max, v0), lerp(minMax->min, minMax->max, v1), t);
}

ES_INLINE Vector4 bslerp(const Vector4 &v0, const Vector4 &v1, const TrackMinMax *minMax, float t)
{
	return slerp(lerp(minMax->min, minMax->max, v0), lerp(minMax->min, minMax->max, v1), t);
}

template<class C> void AppendToStringRaw(const C *clPtr, std::stringstream &buffer)
{
	const uchar *rawData = reinterpret_cast<const uchar *>(clPtr);
	const int hexSize = clPtr->Size() * 2;
	int cBuff = 0;

	for (int i = 0; i < hexSize; i++)
	{
		bool idk = i & 1;
		char temp = 0x30 + (idk ? rawData[cBuff++] & 0xf : rawData[cBuff] >> 4);

		if (temp > 0x39)
			temp += 7;

		buffer << temp;
	}
}

template<class C> void RetreiveFromRawString(C *clPtr, const std::string &buffer, int &bufferIter)
{
	uchar *rawData = reinterpret_cast<uchar *>(clPtr);
	const int hexSize = clPtr->Size() * 2;
	int cBuff = 0;

	for (int i = 0; i < hexSize; i++, bufferIter++)
	{
		if (bufferIter >= buffer.size())
		{
			bufferIter = -1;
			return;
		}

		const char &cRef = buffer.at(bufferIter);

		if (cRef < '0' || cRef > 'F' || (cRef > '9' && cRef < 'A'))
		{
			i--;
			continue;
		}

		bool idk = i & 1;

		if (!idk)
			rawData[cBuff] = cRef << 4;
		else
			rawData[cBuff++] |= cRef;

	}
}

static void ltrim(const std::string &s, int &curIterPos) 
{
	for (auto it = s.begin() + curIterPos; it != s.end(); it++)
	{
		switch (*it)
		{
		case '\n':
		case '\t':
		case ' ':
			curIterPos++;
			break;
		default:
			return;
		}
	}
}

 ES_FORCEINLINE void SeekTo(const std::string &s, int &curIterPos, const char T = '\n')
{
	for (auto it = s.begin() + curIterPos; it != s.end(); it++)
	{
		curIterPos++;
		
		if (*it == T)
			return;
	}
}

struct Buf_SingleVector3
{
	DECLARE_REFLECTOR;
	Vector data;

	int Size() const { return 12; }
	static const int NEWLINEMOD = 1;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		ReflectorWrapConst<Buf_SingleVector3> tRefl(this);

		buffer << tRefl.GetReflectedValue(0);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		ltrim(buffer, bufferIter);

		ReflectorWrap<Buf_SingleVector3> tRefl(this);
		tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);
		SeekTo(buffer, bufferIter);
	}

	void Evaluate(Vector4 &out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;
		out.W = 1.0f;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame++;
	}

	void Iterpolate(Vector4 &out, const Buf_SingleVector3 &leftFrame, const Buf_SingleVector3 &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = lerp(startPoint, endPoint, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
	}
};

REFLECTOR_START_WNAMES(Buf_SingleVector3, data);

struct Buf_StepRotationQuat3 : Buf_SingleVector3
{
	void Evaluate(Vector4 &out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;
		out.W = std::sqrtf(1.0f - out.X * out.X - out.Y * out.Y - out.Z * out.Z);
	}

	void Iterpolate(Vector4 &out, const Buf_StepRotationQuat3 &leftFrame, const Buf_StepRotationQuat3 &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = slerp(startPoint, endPoint, delta);
	}
};

struct Buf_LinearVector3
{
	DECLARE_REFLECTOR;
	Vector data;
	int additiveFrames;

	int Size() const { return 16; }
	static const int NEWLINEMOD = 1;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		ReflectorWrapConst<Buf_LinearVector3> tRefl(this);

		buffer << "{ " << tRefl.GetReflectedValue(0) << ", " << tRefl.GetReflectedValue(1) << " }";
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		SeekTo(buffer, bufferIter, '{');
		ltrim(buffer, bufferIter);

		ReflectorWrap<Buf_LinearVector3> tRefl(this);
		tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);

		SeekTo(buffer, bufferIter, ']');
		SeekTo(buffer, bufferIter, ',');
		ltrim(buffer, bufferIter);

		tRefl.SetReflectedValue(1, buffer.c_str() + bufferIter);

		SeekTo(buffer, bufferIter);
	}

	void Evaluate(Vector4 &out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;
		out.W = 1.0f;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += additiveFrames;
	}

	void Iterpolate(Vector4 &out, const Buf_LinearVector3 &leftFrame, const Buf_LinearVector3 &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = lerp(startPoint, endPoint, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
		FByteswapper(additiveFrames);
	}
};

REFLECTOR_START_WNAMES(Buf_LinearVector3, data, additiveFrames);

REFLECTOR_ENUM(Buf_HermiteVector3_Flags, InTangentX, InTangentY, InTangentZ, OutTangentX, OutTangentY, OutTangentZ);

struct Buf_HermiteVector3
{
	DECLARE_REFLECTOR;

	uchar size;
	esFlags<uchar, Buf_HermiteVector3_Flags> flags;
	short additiveFrames;
	Vector data;
	float tangents[6];

	int Size() const { return size; }
	static const int NEWLINEMOD = 1;
	static const bool VARIABLE_SIZE = true;

	void AppendToString(std::stringstream &buffer) const
	{
		ReflectorWrapConst<Buf_HermiteVector3> tRefl(this);

		buffer << "{ " << tRefl.GetReflectedValue(2)
			<< ", " << tRefl.GetReflectedValue(1)
			<< ", " << tRefl.GetReflectedValue(0);

		int curTang = 0;

		for (int f = 0; f < 6; f++)
			if (flags[static_cast<Buf_HermiteVector3_Flags>(f)])
			{
				buffer << ", " << tangents[curTang++];
			}

		buffer << " }";
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		SeekTo(buffer, bufferIter, '{');
		ltrim(buffer, bufferIter);

		ReflectorWrap<Buf_HermiteVector3> tRefl(this);
		tRefl.SetReflectedValue(2, buffer.c_str() + bufferIter);

		SeekTo(buffer, bufferIter, ']');
		SeekTo(buffer, bufferIter, ',');
		ltrim(buffer, bufferIter);

		tRefl.SetReflectedValue(1, buffer.c_str() + bufferIter);

		SeekTo(buffer, bufferIter, ',');
		ltrim(buffer, bufferIter);

		tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);

		SeekTo(buffer, bufferIter, ',');
		ltrim(buffer, bufferIter);

		int curTang = 0;

		for (int f = 0; f < 6; f++)
			if (flags[static_cast<Buf_HermiteVector3_Flags>(f)])
			{
				tangents[curTang++] = std::atof(buffer.c_str() + bufferIter);
				SeekTo(buffer, bufferIter, ',');
				ltrim(buffer, bufferIter);
			}

		size = sizeof(Buf_HermiteVector3) - (6 - curTang) * 4;

		SeekTo(buffer, bufferIter);
	}

	void Evaluate(Vector4 &out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;
		out.W = 1.0f;
	}

	void GetTangents(Vector4 &inTangs, Vector4 &outTangs) const
	{
		int currentTangIndex = 0;

		inTangs.X = flags[Buf_HermiteVector3_Flags::InTangentX] ? tangents[currentTangIndex++] : 0.0f;
		inTangs.Y = flags[Buf_HermiteVector3_Flags::InTangentY] ? tangents[currentTangIndex++] : 0.0f;
		inTangs.Z = flags[Buf_HermiteVector3_Flags::InTangentZ] ? tangents[currentTangIndex++] : 0.0f;
		outTangs.X = flags[Buf_HermiteVector3_Flags::OutTangentX] ? tangents[currentTangIndex++] : 0.0f;
		outTangs.Y = flags[Buf_HermiteVector3_Flags::OutTangentY] ? tangents[currentTangIndex++] : 0.0f;
		outTangs.Z = flags[Buf_HermiteVector3_Flags::OutTangentZ] ? tangents[currentTangIndex++] : 0.0f;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += additiveFrames;
	}

	void Iterpolate(Vector4 &out, const Buf_HermiteVector3 &leftFrame, const Buf_HermiteVector3 &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint,
			startPointOutTangent,
			endPointInTangent,
			dummy;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);
		leftFrame.GetTangents(dummy, startPointOutTangent);
		rightFrame.GetTangents(endPointInTangent, dummy);
		
		const float deltaP2 = delta * delta;
		const float deltaP3 = delta * deltaP2;
		const float deltaP32 = deltaP3 * 2;
		const float deltaP23 = deltaP2 * 3;
		const float h1 = deltaP32 - deltaP23 + 1;
		const float h2 = -deltaP32 + deltaP23;
		const float h3 = deltaP3 - 2 * deltaP2 + delta;
		const float h4 = deltaP3 - deltaP2;

		out = startPoint * h1 + endPoint * h2 + startPointOutTangent * h3 + endPointInTangent * h4;
	}

	void SwapEndian()
	{
		FByteswapper(additiveFrames);
		FByteswapper(data);

		int curTang = 0;

		for (int f = 0; f < 6; f++)
			if (flags[static_cast<Buf_HermiteVector3_Flags>(f)])
			{
				FByteswapper(tangents[curTang]);
				curTang++;
			}
	}
};

REFLECTOR_START_WNAMES(Buf_HermiteVector3, flags, additiveFrames, data);

struct Buf_SphericalRotation
{
	uint64 data;

	int Size() const { return 8; }
	static const int NEWLINEMOD = 4;
	static const bool VARIABLE_SIZE = false;
	static const int MAXFRAMES = 255;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint64 componentMask = (1 << 17) - 1;
	static const uint64 componentMaskW = (1 << 19) - 1;
	static const float componentMultiplier;
	static const float componentMultiplierInv;
	static const float componentMultiplierW;
	static const uint64 dataField = (1Ui64 << 56) - 1;
	static const uint64 frameField = ~dataField;

	void Devaluate(Vector4 in)
	{
		data ^= data & dataField;

		if (in.W < 0.0f)
			in = -in;

		if (in.X < 0.0f)
		{
			in.X *= -1;
			data |= 1Ui64 << 53;
		}

		if (in.Y < 0.0f)
		{
			in.Y *= -1;
			data |= 1Ui64 << 54;
		}

		if (in.Z < 0.0f)
		{
			in.Z *= -1;
			data |= 1Ui64 << 55;
		}

		const float R = std::sqrtf(1.0f - in.W);
		const float magnitude_safe = std::sqrtf(1.0f - (in.W * in.W));
		const float magnitude = magnitude_safe < 0.001f ? 1.0f : magnitude_safe;
		
		const float phi = std::asinf(in.Y / magnitude);
		const float theta = std::asinf(in.X / (std::cosf(phi) * magnitude));

		data |= static_cast<uint64>(theta * componentMultiplierInv) & componentMask;
		data |= (static_cast<uint64>(phi * componentMultiplierInv) & componentMask) << 17;
		data |= (static_cast<uint64>(R * componentMaskW) & componentMaskW) << 34;

	}

	void Evaluate(Vector4 &out, bool pass = true) const
	{
		out.X = data & componentMask;
		out.Y = (data >> 17) & componentMask;
		float wComp = static_cast<float>((data >> 34) & componentMaskW) * componentMultiplierW;
		out *= componentMultiplier;
	
		const Vector4 var1(std::sinf(out.X), std::sinf(out.Y), std::cosf(out.X), std::cosf(out.Y));
		
		//Optimized Taylor series expansion of sin function, this is faster by 4ms on debug, 1-2ms on release /wo inline
		//Since this library is not frame time heavy, it's disabled
		//If enabled: invert out.X before sign check
		/*
		out.Z = out.X - fPI2;
		out.W = out.Y - fPI2;

		const Vector4 fvar1 = out * out;
		const Vector4 fvar2 = out * 9.53992f;
		const Vector4 var1 = out * (out - fPI) * (out + fPI) * (fvar1 - fvar2 + 25.0493f) * (fvar1 + fvar2 + 25.0493f) * -0.000161476f;
		*/


		wComp = 1.0f - (wComp * wComp);
		float magnitude = std::sqrtf(1.0f - (wComp * wComp));
		
		out.X = var1.X * var1.W * magnitude;
		out.Y = var1.Y * magnitude;
		out.Z = var1.Z * var1.W * magnitude;
		out.W = wComp;

		if ((data >> 53) & 1)
			out.X *= -1;

		if ((data >> 54) & 1)
			out.Y *= -1;

		if ((data >> 55) & 1)
			out.Z *= -1;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += data >> 56;
	}

	int GetFrame() const
	{
		return data >> 56;
	}

	void SetFrame(uint64 frame)
	{
		data ^= data & frameField;
		data |= frame << 56;
	}

	void Iterpolate(Vector4 &out, const Buf_SphericalRotation &leftFrame, const Buf_SphericalRotation &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = slerp(startPoint, endPoint, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
	}
};
const float Buf_SphericalRotation::componentMultiplierInv = static_cast<float>(componentMask) / fPI2;
const float Buf_SphericalRotation::componentMultiplier = 1.0f / componentMultiplierInv;
const float Buf_SphericalRotation::componentMultiplierW = 1.0f / static_cast<float>(componentMaskW);

struct Buf_BiLinearVector3_16bit
{
	USVector data;
	ushort additiveFrames;

	int Size() const { return 8; }
	static const int NEWLINEMOD = 4;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint64 componentMask = 0xffff;
	static const float componentMultiplier;

	void Evaluate(Vector4 &out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;

		out *= componentMultiplier;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += additiveFrames;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearVector3_16bit &leftFrame, const Buf_BiLinearVector3_16bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = blerp(startPoint, endPoint, minMax, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
		FByteswapper(additiveFrames);
	}
};

const float Buf_BiLinearVector3_16bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearVector3_8bit
{
	UCVector data;
	uchar additiveFrames;

	int Size() const { return 4; }
	static const int NEWLINEMOD = 7;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint64 componentMask = 0xff;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		out.X = data.X;
		out.Y = data.Y;
		out.Z = data.Z;

		out *= componentMultiplier;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += additiveFrames;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearVector3_8bit &leftFrame, const Buf_BiLinearVector3_8bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = blerp(startPoint, endPoint, minMax, delta);
	}

	void SwapEndian() {}
};

const float Buf_BiLinearVector3_8bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_LinearRotationQuat4_14bit : Buf_SphericalRotation
{
	static const uint64 componentMask = (1 << 14) - 1;
	static const float componentMultiplier;
	static const float componentSignMax;

	void Evaluate(Vector4 & out) const
	{
		out.X = data & componentMask;
		out.Y = (data >> 14) & componentMask;
		out.Z = (data >> 28) & componentMask;
		out.W = (data >> 42) & componentMask;

		if (out.X > componentSignMax)
			out.X = componentMask - out.X;

		if (out.Y > componentSignMax)
			out.Y = componentMask - out.Y;

		if (out.Z > componentSignMax)
			out.Z = componentMask - out.Z;

		if (out.W > componentSignMax)
			out.W = componentMask - out.W;

		out *= componentMultiplier;
	}

	void Iterpolate(Vector4 &out, const Buf_LinearRotationQuat4_14bit &leftFrame, const Buf_LinearRotationQuat4_14bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = slerp(startPoint, endPoint, delta);
	}
};

const float Buf_LinearRotationQuat4_14bit::componentMultiplier = 1.0f / (static_cast<float>(componentMask) / 4.0f);
const float Buf_LinearRotationQuat4_14bit::componentSignMax = static_cast<float>(componentMask) * 0.5;

struct Buf_BiLinearRotationQuat4_7bit
{
	uint data;

	int Size() const { return 4; }
	static const int NEWLINEMOD = 8;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint componentMask = (1 << 7) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		out.X = data & componentMask;
		out.Y = (data >> 7) & componentMask;
		out.Z = (data >> 14) & componentMask;
		out.W = (data >> 21) & componentMask;

		out *= componentMultiplier;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuat4_7bit &leftFrame, const Buf_BiLinearRotationQuat4_7bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
	}
};

const float Buf_BiLinearRotationQuat4_7bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearRotationQuatXW_14bit : Buf_BiLinearRotationQuat4_7bit 
{
	static const uint componentMask = (1 << 14) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		out.X = data & componentMask;
		out.W = (data >> 14) & componentMask;

		out *= componentMultiplier;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuatXW_14bit &leftFrame, const Buf_BiLinearRotationQuatXW_14bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}
};

const float Buf_BiLinearRotationQuatXW_14bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearRotationQuatYW_14bit : Buf_BiLinearRotationQuat4_7bit
{
	static const uint componentMask = (1 << 14) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		out.Y = data & componentMask;
		out.W = (data >> 14) & componentMask;

		out *= componentMultiplier;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuatYW_14bit &leftFrame, const Buf_BiLinearRotationQuatYW_14bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}
};

const float Buf_BiLinearRotationQuatYW_14bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearRotationQuatZW_14bit : Buf_BiLinearRotationQuat4_7bit
{
	static const uint componentMask = (1 << 14) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		out.Z = data & componentMask;
		out.W = (data >> 14) & componentMask;

		out *= componentMultiplier;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuatZW_14bit &leftFrame, const Buf_BiLinearRotationQuatZW_14bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}
};

const float Buf_BiLinearRotationQuatZW_14bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearRotationQuat4_11bit
{
	USVector data;

	int Size() const { return 6; }
	static const int NEWLINEMOD = 6;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint64 componentMask = (1 << 11) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 & out) const
	{
		const uint64 &rVal = reinterpret_cast<const uint64 &> (data);

		out.X = rVal & componentMask;
		out.Y = (rVal >> 11) & componentMask;
		out.Z = (rVal >> 22) & componentMask;
		out.W = (rVal >> 33) & componentMask;

		out *= componentMultiplier;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += data.Z >> 12;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuat4_11bit &leftFrame, const Buf_BiLinearRotationQuat4_11bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}

	void SwapEndian()
	{
		FByteswapper(data);
	}
};

const float Buf_BiLinearRotationQuat4_11bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct Buf_BiLinearRotationQuat4_9bit
{
	uchar data[5];

	int Size() const { return 5; }
	static const int NEWLINEMOD = 6;
	static const bool VARIABLE_SIZE = false;

	void AppendToString(std::stringstream &buffer) const
	{
		AppendToStringRaw(this, buffer);
	}

	void RetreiveFromString(const std::string &buffer, int &bufferIter)
	{
		RetreiveFromRawString(this, buffer, bufferIter);
	}

	static const uint64 componentMask = (1 << 9) - 1;
	static const float componentMultiplier;

	void Evaluate(Vector4 &out) const
	{
		const uint64 &rVal = reinterpret_cast<const uint64 &> (data);

		out.X = rVal & componentMask;
		out.Y = (rVal >> 9) & componentMask;
		out.Z = (rVal >> 18) & componentMask;
		out.W = (rVal >> 27) & componentMask;

		out *= componentMultiplier;
	}

	void GetFrame(int &currentFrame) const
	{
		currentFrame += data[4] >> 4;
	}

	void Iterpolate(Vector4 &out, const Buf_BiLinearRotationQuat4_9bit &leftFrame, const Buf_BiLinearRotationQuat4_9bit &rightFrame, float delta, const TrackMinMax *minMax) const
	{
		Vector4 startPoint,
			endPoint;

		leftFrame.Evaluate(startPoint);
		rightFrame.Evaluate(endPoint);

		out = bslerp(startPoint, endPoint, minMax, delta);
	}

	void SwapEndian() {}
};

const float Buf_BiLinearRotationQuat4_9bit::componentMultiplier = 1.0f / static_cast<float>(componentMask);

struct BuffEval
{
	virtual int NumItems() const = 0;
	virtual void NumItems(int numItems) = 0;
	virtual void ToString(std::string &strBuf, int numIdents) const = 0;
	virtual void FromString(std::string &input) = 0;
	virtual void Assign(char *ptr, int size) = 0;
	virtual void SwapEndian() = 0;
	virtual ~BuffEval(){}
};

template<class C>
struct Buff_EvalShared : BuffEval
{
	typedef std::vector<C, std::allocator_hybrid<C>> Store_Type;
	Store_Type data;
	int NumItems() const { return data.size(); }
	void NumItems(int numItems) { data.resize(numItems); }

	void ToString(std::string &strBuff, int numIdents) const
	{
		std::stringstream str;
		str << std::endl << idents[numIdents];

		int curLine = 1;

		for (auto &d : data)
		{
			d.AppendToString(str);

			if (!(curLine % C::NEWLINEMOD))
				str << std::endl << idents[numIdents];

			curLine++;
		}

		if (!((curLine - 1) % C::NEWLINEMOD))
			str.seekp(-1, SEEK_CUR) << '\0';
		else
			str << std::endl << idents[numIdents - 1];

		strBuff = str.str();
	}

	void FromString(std::string &input)
	{
		int iterPos = 0;

		for (auto &d : data)
		{
			d.RetreiveFromString(input, iterPos);

			if (iterPos < 0)
			{
				printerror("[LMT] Unexpected end of <data/> buffer.")
				return;
			}
		}
	}

	void Assign(char *ptr, int size) 
	{
		if (!C::VARIABLE_SIZE)
		{
			data = Store_Type(reinterpret_cast<C *>(ptr), reinterpret_cast<C *>(ptr + size), std::allocator_hybrid<C>(reinterpret_cast<C *>(ptr)));
			return;
		}

		const char *bufferEnd = ptr + size;

		while (ptr < bufferEnd)
		{
			C *block = reinterpret_cast<C *>(ptr);
			data.push_back(*block);
			ptr += block->Size();
		}

	}
	void SwapEndian()
	{ 
		for (auto &d : data)
			d.SwapEndian(); 
	}
};

template<class Derived> BuffEval *_creatorDummyBuffEval() { return new Buff_EvalShared<Derived>(); }

#if FIX_C2970
#define buffEvalRegShenanigans extern
#else
#define buffEvalRegShenanigans static
#endif

buffEvalRegShenanigans const std::map<TrackV1BufferTypes, BuffEval *(*)()> buffEvalRegistryV1 =
{
	{TrackV1BufferTypes::SingleVector3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1BufferTypes::LinearVector3, _creatorDummyBuffEval<Buf_LinearVector3>},
	{TrackV1BufferTypes::SinglePositionVector3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1BufferTypes::SingleRotationQuat3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1BufferTypes::SphericalRotation, _creatorDummyBuffEval<Buf_SphericalRotation>},
	{TrackV1BufferTypes::HermiteVector3, _creatorDummyBuffEval<Buf_HermiteVector3>},
};

buffEvalRegShenanigans const std::map<TrackV1_5BufferTypes, BuffEval *(*)()> buffEvalRegistryV1_5 =
{
	{TrackV1_5BufferTypes::SingleVector3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1_5BufferTypes::LinearVector3, _creatorDummyBuffEval<Buf_LinearVector3>},
	{TrackV1_5BufferTypes::SinglePositionVector3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1_5BufferTypes::SingleRotationQuat3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV1_5BufferTypes::LinearRotationQuat4_14bit, _creatorDummyBuffEval<Buf_LinearRotationQuat4_14bit>},
	{TrackV1_5BufferTypes::HermiteVector3, _creatorDummyBuffEval<Buf_HermiteVector3>},
};

buffEvalRegShenanigans const std::map<TrackV2BufferTypes, BuffEval *(*)()> buffEvalRegistryV2 =
{
	{TrackV2BufferTypes::SingleVector3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV2BufferTypes::LinearVector3, _creatorDummyBuffEval<Buf_LinearVector3>},
	{TrackV2BufferTypes::SingleRotationQuat3, _creatorDummyBuffEval<Buf_SingleVector3>},
	{TrackV2BufferTypes::LinearRotationQuat4_14bit, _creatorDummyBuffEval<Buf_LinearRotationQuat4_14bit>},
	{TrackV2BufferTypes::BiLinearVector3_16bit, _creatorDummyBuffEval<Buf_BiLinearVector3_16bit>},
	{TrackV2BufferTypes::BiLinearVector3_8bit, _creatorDummyBuffEval<Buf_BiLinearVector3_8bit>},
	{TrackV2BufferTypes::BiLinearRotationQuat4_7bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuat4_7bit>},
	{TrackV2BufferTypes::BiLinearRotationQuatXW_14bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuatXW_14bit>},
	{TrackV2BufferTypes::BiLinearRotationQuatYW_14bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuatYW_14bit>},
	{TrackV2BufferTypes::BiLinearRotationQuatZW_14bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuatZW_14bit>},
	{TrackV2BufferTypes::BiLinearRotationQuat4_11bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuat4_11bit>},
	{TrackV2BufferTypes::BiLinearRotationQuat4_9bit, _creatorDummyBuffEval<Buf_BiLinearRotationQuat4_9bit>},
};

/************************************************************************/
/****************************** TRACKS **********************************/
/************************************************************************/

template<template<class C> class PtrType, class BufferType>
struct TrackV1
{
	DECLARE_REFLECTOR;
	void noExtremes();
	esEnum<uchar, BufferType> compression;
	esEnum<uchar, TrackType> trackType;
	uchar boneType;
	uchar boneID;
	float weight;
	uint bufferSize;
	PtrType<char> bufferOffset;
	Vector4 referenceData;

	int NumPointers() const
	{
		return 1;
	}

	int PointerOffset(int id) const
	{
		switch (id)
		{
		case 0:
			return offsetof(TrackV1, bufferOffset);
		default:
			return -1;
		}
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		bufferOffset.Fixup(masterBuffer, swapEndian);

		if (!swapEndian)
			return;

		FByteswapper(weight);
		FByteswapper(bufferSize);
		FByteswapper(referenceData);
	}
};

typedef TrackV1<PointerX86, TrackV1BufferTypes> TrackV1TrackV1BufferTypesPointerX86;
REFLECTOR_START_WNAMES(TrackV1TrackV1BufferTypesPointerX86, compression, trackType, boneType, boneID, weight, referenceData);

typedef TrackV1<PointerX64, TrackV1BufferTypes> TrackV1TrackV1BufferTypesPointerX64;
REFLECTOR_START_WNAMES(TrackV1TrackV1BufferTypesPointerX64, compression, trackType, boneType, boneID, weight, referenceData);

typedef TrackV1<PointerX86, TrackV1_5BufferTypes> TrackV1TrackV1_5BufferTypesPointerX86;
REFLECTOR_START_WNAMES(TrackV1TrackV1_5BufferTypesPointerX86, compression, trackType, boneType, boneID, weight, referenceData);

typedef TrackV1<PointerX64, TrackV1_5BufferTypes> TrackV1TrackV1_5BufferTypesPointerX64;
REFLECTOR_START_WNAMES(TrackV1TrackV1_5BufferTypesPointerX64, compression, trackType, boneType, boneID, weight, referenceData);

template<template<class C> class PtrType>
struct TrackV2
{
	DECLARE_REFLECTOR;
	esEnum<uchar, TrackV2BufferTypes> compression;
	esEnum<uchar, TrackType> trackType;
	uchar boneType;
	uchar boneID;
	float weight;
	typename std::conditional<sizeof(PtrType<void>) == 4, uint, uint64>::type bufferSize;
	PtrType<char> bufferOffset;
	Vector4 referenceData;
	PtrType<TrackMinMax> extremes;

	int NumPointers() const
	{
		return 2;
	}

	int PointerOffset(int id) const
	{
		switch (id)
		{
		case 0:
			return offsetof(TrackV2, bufferOffset);
		case 1:
			return offsetof(TrackV2, extremes);
		default:
			return -1;
		}
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		bufferOffset.Fixup(masterBuffer, swapEndian);
		extremes.Fixup(masterBuffer, swapEndian);

		if (!swapEndian)
			return;

		FByteswapper(reinterpret_cast<int &>(compression));
		FByteswapper(weight);
		FByteswapper(bufferSize);
		FByteswapper(referenceData);
	}
};

typedef TrackV2<PointerX86> TrackV2PointerX86;
REFLECTOR_START_WNAMES(TrackV2PointerX86, compression, trackType, boneType, boneID, weight, referenceData);

typedef TrackV2<PointerX64> TrackV2PointerX64;
REFLECTOR_START_WNAMES(TrackV2PointerX64, compression, trackType, boneType, boneID, weight, referenceData);

template<template<class C> class PtrType>
struct TrackV3
{
	DECLARE_REFLECTOR;
	esEnum<uchar, TrackV2BufferTypes> compression;
	esEnum<uchar, TrackType> trackType;
	uchar boneType;
	uchar boneID;
	int mirroredBoneID;
	float weight;
	int bufferSize;
	PtrType<char> bufferOffset;
	Vector4 referenceData;
	PtrType<TrackMinMax> extremes;

	int NumPointers() const
	{
		return 2;
	}

	int PointerOffset(int id) const
	{
		switch (id)
		{
		case 0:
			return offsetof(TrackV3, bufferOffset);
		case 1:
			return offsetof(TrackV3, extremes);
		default:
			return -1;
		}
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		bufferOffset.Fixup(masterBuffer, swapEndian);
		extremes.Fixup(masterBuffer, swapEndian);

		if (!swapEndian)
			return;

		FByteswapper(reinterpret_cast<int &>(compression));
		FByteswapper(weight);
		FByteswapper(bufferSize);
		FByteswapper(referenceData);
		FByteswapper(mirroredBoneID);
	}
};

typedef TrackV3<PointerX86> TrackV3PointerX86;
REFLECTOR_START_WNAMES(TrackV3PointerX86, compression, trackType, boneType, boneID, mirroredBoneID, weight, referenceData);

typedef TrackV3<PointerX64> TrackV3PointerX64;
REFLECTOR_START_WNAMES(TrackV3PointerX64, compression, trackType, boneType, boneID, mirroredBoneID, weight, referenceData);

/************************************************************************/
/*************************** ANIM_TRAITS ********************************/
/************************************************************************/

template<class _BuffType, class _BuffEvalType, _BuffEvalType &evalReg>
struct BufferTypeTraits
{
	typedef _BuffType BufferType;
	typedef _BuffEvalType EvalMapType;
	const _BuffEvalType &evalClass = evalReg;
};

template<
	template<class C> class PtrType, 
	template<template<class C> class PtrType, class BufferType> class TrackType, 
	class BufferTraits
>
struct AnimTraitsV1
{
	typedef BufferTraits Traits;
	typedef typename Traits::BufferType Buffer_Type;
	typedef TrackType<PtrType, Buffer_Type> Track_Type;
	typedef PtrType<Track_Type> Track_Pointer;
	typedef PtrType<AnimEvent> Event_Pointer;
	typedef EventTable<PtrType> EventClass;
	typedef typename Traits::EvalMapType EvalMapType;

	static const int NUMEVENTGROUPS = 2;
	const EvalMapType &evalClass = Traits{}.evalClass;

};

template<
	template<class C> class PtrType,
	template<template<class C> class PtrType> class TrackType
>
struct AnimTraitsV2
{
	typedef TrackType<PtrType> Track_Type;
	typedef PtrType<Track_Type> Track_Pointer;
	typedef PtrType<AnimEvent> Event_Pointer;
	typedef typename decltype(buffEvalRegistryV2) EvalMapType;
	typedef EventTable<PtrType> EventClass;
	typedef PtrType<EventClass> EventClass_Pointer;

	static const int NUMEVENTGROUPS = 4;
	const EvalMapType &evalClass = buffEvalRegistryV2;
};

/************************************************************************/
/***************************** ANIM_V1 **********************************/
/************************************************************************/

template<class AnimTraits>
struct AnimV1
{
	DECLARE_REFLECTOR;

	typedef AnimTraits Traits;
	typename Traits::Track_Pointer tracks;
	int numTracks,
		numFrames,
		loopFrame;
	Vector4 endFrameAdditiveScenePosition;
	Vector4 endFrameAdditiveSceneRotation;

	typename Traits::EventClass events[Traits::NUMEVENTGROUPS];

	typename Traits::EventClass *Events(char *masterBuffer) { return events; }

	static const int VERSION = 1;
	static const int NUMPOINTERS = Traits::NUMEVENTGROUPS + 1;

	int PointerOffset(int id) const
	{
		if (!id)
			return offsetof(AnimV1, tracks);

		if (id < NUMPOINTERS)
			return offsetof(AnimV1, events[id - 1].events);

		return -1;
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		tracks.Fixup(masterBuffer, swapEndian);

		for (auto &e : events)
			e.Fixup(masterBuffer, swapEndian);

		if (!swapEndian)
			return;

		FByteswapper(numTracks);
		FByteswapper(numFrames);
		FByteswapper(loopFrame);
		FByteswapper(endFrameAdditiveScenePosition);
		FByteswapper(endFrameAdditiveSceneRotation);
	}
};

typedef AnimV1<
	AnimTraitsV1<
		PointerX86, 
		TrackV1, 
		BufferTypeTraits<
			TrackV1BufferTypes, 
			decltype(buffEvalRegistryV1), 
			buffEvalRegistryV1
		>
	>
> AnimV1X86TrackV1;

REFLECTOR_START_WNAMES(AnimV1X86TrackV1, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<
	AnimTraitsV1<
		PointerX64,
		TrackV1,
		BufferTypeTraits<
			TrackV1BufferTypes,
			decltype(buffEvalRegistryV1),
			buffEvalRegistryV1
		>
	>
> AnimV1X64TrackV1;

REFLECTOR_START_WNAMES(AnimV1X64TrackV1, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<
	AnimTraitsV1<
		PointerX86,
		TrackV1,
		BufferTypeTraits<
			TrackV1_5BufferTypes,
			decltype(buffEvalRegistryV1_5),
			buffEvalRegistryV1_5
		>
	>
> AnimV1X86TrackV1_5;

REFLECTOR_START_WNAMES(AnimV1X86TrackV1_5, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<
	AnimTraitsV1<
		PointerX64,
		TrackV1,
		BufferTypeTraits<
			TrackV1_5BufferTypes,
			decltype(buffEvalRegistryV1_5),
			buffEvalRegistryV1_5
		>
	>
> AnimV1X64TrackV1_5;

REFLECTOR_START_WNAMES(AnimV1X64TrackV1_5, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);


typedef AnimV1<AnimTraitsV2<PointerX86, TrackV2>> AnimV1X86TrackV2;
REFLECTOR_START_WNAMES(AnimV1X86TrackV2, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV2<PointerX64, TrackV2>> AnimV1X64TrackV2;
REFLECTOR_START_WNAMES(AnimV1X64TrackV2, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

const int sz = sizeof(AnimV1X86TrackV1);
static_assert(sizeof(AnimV1X86TrackV1) == 192, "Check assumptions");

/************************************************************************/
/***************************** ANIM_V2 **********************************/
/************************************************************************/

template<class AnimTraits>
struct AnimV2
{
	DECLARE_REFLECTOR;
	typedef AnimTraits Traits;
	typename Traits::Track_Pointer tracks;
	int numTracks,
		numFrames,
		loopFrame;
	Vector4 endFrameAdditiveScenePosition;
	Vector4 endFrameAdditiveSceneRotation;
	int unk;
	typename Traits::EventClass_Pointer eventTable;

	typename Traits::EventClass *Events(char *masterBuffer) { return eventTable.GetData(masterBuffer); }

	static const int VERSION = 2;
	static const int NUMPOINTERS = 2;

	int PointerOffset(int id) const
	{
		switch (id)
		{
		case 0:
			return offsetof(AnimV2, tracks);
		case 1:
			return offsetof(AnimV2, eventTable);
		default:
			return -1;
		}
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		tracks.Fixup(masterBuffer, swapEndian);
		eventTable.Fixup(masterBuffer, swapEndian);

		printer << unk >> 0;

		typename Traits::EventClass *dEvents = Events(masterBuffer);

		for (int e = 0; e < Traits::NUMEVENTGROUPS; e++)
			dEvents[e].Fixup(masterBuffer, swapEndian);

	}
};

typedef AnimV2<AnimTraitsV2<PointerX86, TrackV2>> AnimV2X86TrackV2;
REFLECTOR_START_WNAMES(AnimV2X86TrackV2, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV2<AnimTraitsV2<PointerX64, TrackV2>> AnimV2X64TrackV2;
REFLECTOR_START_WNAMES(AnimV2X64TrackV2, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);


/************************************************************************/
/***************************** ANIM_V3 **********************************/
/************************************************************************/

template<class AnimTraits>
struct AnimV3
{
	DECLARE_REFLECTOR;
	typedef AnimTraits Traits;
	typename Traits::Track_Pointer tracks;
	int numTracks,
		numFrames,
		loopFrame;
	int unk00[3];
	Vector4 endFrameAdditiveScenePosition;
	Vector4 endFrameAdditiveSceneRotation;
	int unk[6];
	typename Traits::EventClass_Pointer eventTable;
	typename Traits::EventClass *Events(char *masterBuffer) { return nullptr;/*eventTable.GetData(masterBuffer);*/ }

	static const int VERSION = 3;
	static const int NUMPOINTERS = 2;

	int PointerOffset(int id) const
	{
		switch (id)
		{
		case 0:
			return offsetof(AnimV3, tracks);
		case 1:
			return offsetof(AnimV3, eventTable);
		default:
			return -1;
		}
	}

	void Fixup(char *masterBuffer, bool swapEndian)
	{
		tracks.Fixup(masterBuffer, swapEndian);
		//eventTable.Fixup(masterBuffer, swapEndian);
	}
};

typedef AnimV3<AnimTraitsV2<PointerX86, TrackV3>> AnimV3X86TrackV3;
REFLECTOR_START_WNAMES(AnimV3X86TrackV3, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV3<AnimTraitsV2<PointerX64, TrackV3>> AnimV3X64TrackV3;
REFLECTOR_START_WNAMES(AnimV3X64TrackV3, numFrames, loopFrame, endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

/************************************************************************/
/**************************** LMT CLASS *********************************/
/************************************************************************/

class LMTAnimation_internal : public LMTAnimation
{
	static const int MTMI = CompileFourCC("MTMI");
public:
	bool mastered,
		masteredData;
	short version;
	char *masterBuffer;
	LMTAnimation_internal() : version(0), masterBuffer(nullptr), mastered(false), masteredData(false) {}
	virtual void Assign(char *ptr, bool swapEndian) = 0;
	virtual int SaveBase(BinWritter *wr, LMTFixupStorage &storage) const = 0;
	virtual int SaveRest(BinWritter *wr, LMTFixupStorage &storage) const = 0;

	int Save(const char *fileName) const
	{
		LMTFixupStorage fixups;
		BinWritter wr(fileName);

		if (!wr.IsValid())
		{
			printerror("[LMT] Couldn't save file: ", << fileName);
			return 1;
		}

		wr.Write(MTMI);
		wr.Write(version);
		const size_t saveposBuffSize = wr.Tell();
		wr.ApplyPadding();

		SaveBase(&wr, fixups);
		SaveRest(&wr, fixups);
		const size_t savepos = wr.Tell();
		fixups.SaveFrom(saveposBuffSize);
		fixups.SaveTo(&wr);

		for (auto &f : fixups.fixupStorage)
		{
			wr.Seek(f.from);
			wr.Write(f.to);
		}

		wr.Seek(savepos);

		return 0;
	}

	int Load(BinReader *rd, short expectedVersion)
	{
		int magic;
		rd->Read(magic);

		if (magic != MTMI)
			return 1;

		rd->Read(version);

		if (version != expectedVersion)
			return 2;
		
		int bufferSize;
		rd->Read(bufferSize);

		if (!bufferSize)
			return 3;

		rd->Seek(0);
		
		masterBuffer = static_cast<char *>(malloc(bufferSize));
		mastered = true;
		rd->ReadBuffer(masterBuffer, bufferSize);

		Assign(masterBuffer + 16, false);

		return 0;
	}

	~LMTAnimation_internal()
	{
		if (mastered && masterBuffer)
			free(masterBuffer);
	}
};

template<class C, class EvalMap>
class TrackShared : LMTTrack
{
public:
	bool locked,
		mastered;
	mutable int numIdents = 5;
	char *masterBuffer;
	C *data;
	BuffEval *controller;
	TrackMinMax *minMax;
	const EvalMap *evalMap;
	TrackShared() : masterBuffer(nullptr), data(nullptr), controller(nullptr), locked(false), mastered(false), minMax(nullptr) {}
	TrackShared(C *_data, const EvalMap &_evalMap, char *_buff, bool swapEndian) : data(_data), masterBuffer(_buff), evalMap(&_evalMap), mastered(false)
	{
		data->Fixup(masterBuffer, swapEndian);
		controller = evalMap->at(data->compression)();
		controller->Assign(data->bufferOffset.GetData(masterBuffer), data->bufferSize);
		locked = true;
		minMax = GetTrackExtremes();

		if (swapEndian)
			SwapEndian();
	}

	~TrackShared()
	{
		if (controller && !locked)
			delete controller;

		if (mastered && minMax)
			delete minMax;

		if (mastered && data)
			delete data;
	}

	ADD_DISABLERS(C, noExtremes);
	enabledFunction(TrackMinMax *) GetTrackExtremes()
	{
		return reinterpret_cast<TrackMinMax *>(data->extremes.GetData(masterBuffer));
	}
	disabledFunction(TrackMinMax *) GetTrackExtremes() { return nullptr; }

	enabledFunction(bool) UseTrackExtremes() const { return true; }
	disabledFunction(bool) UseTrackExtremes() const { return false; }

	void SwapEndian()
	{
		if (controller)
			controller->SwapEndian();
	}

	Reflector::xmlNodePtr ToXML(pugi::xml_node &node) const
	{
		ReflectorWrapConst<C> reflt(data);
		reflt.ToXML(node, false);

		if (minMax)
		{
			ReflectorWrapConst<const TrackMinMax> refl(minMax);
			refl.ToXML(node);
		}

		int currentBufferOffset = 0;
		const int maxBufferSize = static_cast<const int>(data->bufferSize);
		char *buffer = data->bufferOffset.GetData(masterBuffer);
		pugi::xml_node dataNode = node.append_child("data");
		std::string strBuff;
		controller->ToString(strBuff, numIdents);
		dataNode.append_buffer(strBuff.c_str(), strBuff.size());
		dataNode.append_attribute("numItems").set_value(controller->NumItems());

		return 0;
	}

	int FromXML(pugi::xml_node &node)
	{
		mastered = true;
		data = new C;
		ReflectorWrap<C> reflt(data);
		reflt.FromXML(node, false);

		if (UseTrackExtremes())
		{
			TrackMinMax *localMinMax = new TrackMinMax();
			ReflectorWrap<TrackMinMax> minMaxRelfl(localMinMax);
			pugi::xml_node minMaxNode(minMaxRelfl.FromXML(node));

			if (minMaxNode.empty())
				delete localMinMax;
			else
				minMax = localMinMax;
		}

		if (!evalMap->count(data->compression))
		{
			pugi::xml_node comChild = node.child("compression");

			printerror("[LMT] Unknown track compression type: ", << comChild.text().get());
			return 1;
		}

		controller = evalMap->at(data->compression)();

		pugi::xml_node dataNode = node.child("data");
		pugi::xml_attribute numDataItems = dataNode.attribute("numItems");

		if (dataNode.empty() || numDataItems.empty() || !numDataItems.as_int())
			return 0;

		controller->NumItems(numDataItems.as_int());
		std::string strBuff = dataNode.text().get();
		controller->FromString(strBuff);


		return 0;
	}

	int SaveBuffers(BinWritter *wr, LMTFixupStorage &storage) const
	{
		char *buffer = data->bufferOffset.GetData(masterBuffer);

		if (buffer)
		{
			wr->ApplyPadding();
			storage.SaveTo(wr);
			wr->WriteBuffer(buffer, static_cast<size_t>(data->bufferSize));
		}
		else
			storage.SkipTo();

		if (minMax)
		{
			wr->ApplyPadding();
			storage.SaveTo(wr);
			wr->Write(*minMax);
		}

		if (!minMax && UseTrackExtremes())
			storage.SkipTo();

		return 0;
	}

};

template<class C>
class AnimShared : public LMTAnimation_internal
{
public:
	typedef C Animation_Type;
	typedef typename Animation_Type::Traits::Track_Type Track_Type;
	typedef TrackShared<Track_Type, typename Animation_Type::Traits::EvalMapType> SharedTrack_Type;
	typedef typename Animation_Type::Traits::EventClass EventClass;
	typedef std::vector<AnimEvent, std::allocator_hybrid<AnimEvent>> Events_Type;

	std::vector<SharedTrack_Type> tracks;
	Events_Type events[Animation_Type::Traits::NUMEVENTGROUPS];
	Animation_Type *data;

	~AnimShared()
	{
		if (masteredData && data)
			delete data;
	}

	int ToXML(pugi::xml_node &node, bool standAlone) const
	{
		ReflectorWrap<Animation_Type> refl(data);
		refl.ToXML(node, false);
		EventClass *dEvents = data->Events(masterBuffer);

		if (dEvents)
		{
			pugi::xml_node evGroupsNode = node.append_child("eventGroups");
			evGroupsNode.append_attribute("numItems").set_value(NumEventBlocks());

			for (int e = 0; e < NumEventBlocks(); e++)
			{
				pugi::xml_node evGroupNode = evGroupsNode.append_child("eventGroup");

				ReflectorWrapConst<const EventClass> reflEvent(&dEvents[e]);
				reflEvent.ToXML(evGroupNode, false);

				pugi::xml_node rangesNode = evGroupNode.append_child("events");

				rangesNode.append_attribute("numItems").set_value(dEvents[e].numEvents);

				for (auto &r : events[e])
					r.ToXML(rangesNode);
			}
		}

		int curTrack = 0;

		pugi::xml_node tracksNode = node.append_child("tracks");
		tracksNode.append_attribute("numItems").set_value(tracks.size());

		for (auto &t : tracks)
		{
			t.numIdents = standAlone ? 4 : 5;
			pugi::xml_node trackNode = tracksNode.append_child("track");
			t.ToXML(trackNode);

			curTrack++;
		}

		return 0;
	}

	int NumEventBlocks() const { return Animation_Type::Traits::NUMEVENTGROUPS; }
	const int NumTracks() const { return data->numTracks; };

	void Assign(char *ptr, bool swapEndian) 
	{ 
		data = reinterpret_cast<Animation_Type *>(ptr);
		data->Fixup(masterBuffer, swapEndian);
		Track_Type *begin = data->tracks.GetData(masterBuffer);
		tracks.resize(data->numTracks);

		for (int t = 0; t < data->numTracks; t++)
			tracks[t] = SharedTrack_Type(begin + t, Animation_Type::Traits{}.evalClass, masterBuffer, swapEndian);

		for (auto &t : tracks)
			t.locked = false;

		int curEvent = 0;
		auto dEvents = data->Events(masterBuffer);

		if (dEvents)
			for (int e = 0; e < NumEventBlocks(); e++)
			{
				AnimEvent *rangeBegin = dEvents[e].events.GetData(masterBuffer);
				events[curEvent] = Events_Type(rangeBegin, rangeBegin + dEvents[e].numEvents, Events_Type::allocator_type(rangeBegin));

				if (swapEndian)
					for (auto &r : events[curEvent])
						FByteswapper(r);

				curEvent++;
			}

	}

	int SaveBase(BinWritter *wr, LMTFixupStorage &storage) const
	{
		const size_t cOff = wr->Tell();
		const int numFixupRequests = data->NUMPOINTERS;

		wr->Write(*data);

		for (int r = 0; r < numFixupRequests; r++)
			storage.SaveFrom(cOff + data->PointerOffset(r));

		return 0;
	}

	int SaveRest(BinWritter *wr, LMTFixupStorage &storage) const
	{
		wr->ApplyPadding();
		storage.SaveTo(wr);

		LMTFixupStorage localStorage;

		for (auto &t : tracks)
		{
			const size_t cOff = wr->Tell();
			const int numFixupRequests = t.data->NumPointers();

			wr->Write(*t.data);

			for (int r = 0; r < numFixupRequests; r++)
				localStorage.SaveFrom(cOff + t.data->PointerOffset(r));
		}

		for (auto &t : tracks)
		{
			t.SaveBuffers(wr, localStorage);
		}

		EventClass *dEvents = data->Events(masterBuffer);

		if (dEvents)
		{
			if (data->VERSION == 2)
			{
				const int numFixupRequests = Animation_Type::Traits::NUMEVENTGROUPS;

				wr->ApplyPadding();
				storage.SaveTo(wr);

				for (int f = 0; f < numFixupRequests; f++)
				{
					const size_t cOff = wr->Tell();
					wr->WriteBuffer(reinterpret_cast<const char *>(dEvents + f), sizeof(EventClass));
					localStorage.SaveFrom(cOff + offsetof(typename Animation_Type::Traits::EventClass, events));
				}

				for (auto &e : events)
				{
					wr->ApplyPadding();
					localStorage.SaveTo(wr);
					wr->WriteContainer(e);
				}

			}
			else
			{
				for (auto &e : events)
				{
					wr->ApplyPadding();
					storage.SaveTo(wr);
					wr->WriteContainer(e);
				}
			}
		}

		const size_t savepos = wr->Tell();

		for (auto &f : localStorage.fixupStorage)
		{
			wr->Seek(f.from);
			wr->Write(f.to);
		}

		wr->Seek(savepos);

		return 0;
	}

	int FromXML(pugi::xml_node &node)
	{
		masteredData = true;
		data = new Animation_Type;

		ReflectorWrap<Animation_Type> refl(data);
		refl.FromXML(node, false);

		pugi::xml_node tracksNode = node.child("tracks");

		if (tracksNode.empty())
		{
			printerror("[LMT] Couldn't find <tracks/> for animation.");
			return 1;
		}

		auto trackNodesIter = tracksNode.children("track");
		std::vector<pugi::xml_node> trackNodes(trackNodesIter.begin(), trackNodesIter.end());
		pugi::xml_attribute numItemsAttr = tracksNode.attribute("numItems");

		if (!numItemsAttr.empty() && numItemsAttr.as_int() != trackNodes.size())
		{
			printwarning("[LMT] Animation <tracks numItems=/> differs from actual <track/> count.");
		}

		tracks.resize(trackNodes.size());

		int curTrack = 0;

		for (auto &t : trackNodes)
		{
			tracks[curTrack].evalMap = &Animation_Type::Traits{}.evalClass;
			tracks[curTrack].FromXML(t);
			curTrack++;
		}


		return 0;
	}

};

template<class Derived> LMTAnimation_internal *_creatorDummy() { return new AnimShared<Derived>(); }

static const std::map<short, LMTAnimation_internal *(*)()> animationClassRegistry =
{
	{LMT::V_40, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_49, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_50, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_51, _creatorDummy<AnimV1X86TrackV1_5>},
	{LMT::V_56, _creatorDummy<AnimV1<AnimTraitsV2<PointerX86, TrackV2>>>},
	{LMT::V_57, _creatorDummy<AnimV1<AnimTraitsV2<PointerX86, TrackV2>>>},
	{LMT::V_66, _creatorDummy<AnimV2<AnimTraitsV2<PointerX86, TrackV2>>>},
	{LMT::V_67, _creatorDummy<AnimV2<AnimTraitsV2<PointerX86, TrackV2>>>},
	{LMT::V_92, _creatorDummy<AnimV3<AnimTraitsV2<PointerX86, TrackV3>>>},

	{LMT::V_40 | LMT::x64Flag, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_49 | LMT::x64Flag, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_50 | LMT::x64Flag, _creatorDummy<AnimV1X86TrackV1>},
	{LMT::V_51 | LMT::x64Flag, _creatorDummy<AnimV1X86TrackV1_5>},
	{LMT::V_56 | LMT::x64Flag, _creatorDummy<AnimV1<AnimTraitsV2<PointerX64, TrackV2>>>},
	{LMT::V_57 | LMT::x64Flag, _creatorDummy<AnimV1<AnimTraitsV2<PointerX64, TrackV2>>>},
	{LMT::V_66 | LMT::x64Flag, _creatorDummy<AnimV2<AnimTraitsV2<PointerX64, TrackV2>>>},
	{LMT::V_67 | LMT::x64Flag, _creatorDummy<AnimV2<AnimTraitsV2<PointerX64, TrackV2>>>},
	{LMT::V_92 | LMT::x64Flag, _creatorDummy<AnimV3<AnimTraitsV2<PointerX64, TrackV3>>>},
};

int LMT::Version(V _version, Architecture arch)
{
	if (masterBuffer)
	{
		printerror("[LMT] Cannot set version for read only class.");
		return 1;
	}

	if (animations.size())
	{
		printerror("[LMT] Cannot set version for already used class.");
		return 2;
	}

	version = static_cast<short>(_version);

	if (arch == X64)
		version |= x64Flag;

	return 0;
}

int LMT::Load(BinReader *rd)
{
	int magic;
	rd->Read(magic);

	if (magic == ID_R)
	{
		rd->SwapEndian(true);
	}
	else if (magic != ID)
	{
		printerror("[LMT] Invalid file.");
		return 1;
	}

	rd->Read(version);

	if (!animationClassRegistry.count(version))
	{
		printerror("[LMT] Unknown version: ", << version);
		return 2;
	}

	short numBlocks;
	rd->Read(numBlocks);

	const int calcutatedSize = ((numBlocks + 1) * 8) + static_cast<int>(rd->Tell());

	if (version == V_92)
		rd->Skip(4);

	magic = 0;

	while (!magic)
		rd->Read(magic);

	rd->Seek(0);

	const bool isX64 = magic == calcutatedSize;
	const size_t fleSize = rd->GetSize();
	const int multiplier = isX64 ? 2 : 1;
	const int lookupTableOffset = 8 + (version == V_92 ? (4 * multiplier) : 0);

	if (isX64)
		version |= x64Flag;

	masterBuffer = static_cast<char *>(malloc(fleSize));
	rd->ReadBuffer(masterBuffer, fleSize);

	int *lookupTable = reinterpret_cast<int *>(masterBuffer + lookupTableOffset);

	animations.resize(numBlocks);

	for (int a = 0; a < numBlocks; a++)
	{
		int &cOffset = *(lookupTable + (a * multiplier));

		if (rd->SwappedEndian())
		{
			if (isX64)
				FByteswapper(reinterpret_cast<int64 &>(cOffset));
			else
				FByteswapper(cOffset);
		}

		if (!cOffset)
			continue;

		LMTAnimation_internal *cAni = animationClassRegistry.at(version)();
		cAni->masterBuffer = masterBuffer;
		cAni->Assign(masterBuffer + cOffset, rd->SwappedEndian());
		cAni->version = version;

		animations[a] = cAni;
	}

	return 0;
}

int LMT::ToXML(pugi::xml_node &node, const char *fileName, ExportSettings settings)
{
	pugi::xml_node master = node.append_child("LMT");
	master.append_attribute("version").set_value(Version());
	master.append_attribute("numItems").set_value(animations.size());
	master.append_attribute("X64").set_value(GetArchitecture() == X64);

	int curAniID = 0;
	AFileInfo fleInf(fileName);

	for (auto &a : animations)
	{
		pugi::xml_node cAni = master.append_child("Animation");
		cAni.append_attribute("ID").set_value(curAniID);

		if (a)
		{
			if (settings == ExportSetting_FullXML)
				a->ToXML(cAni);
			else if (settings == ExportSetting_FullXMLLinkedMotions)
			{
				pugi::xml_document linkAni = {};
				pugi::xml_node subAni = linkAni.append_child("Animation");
				subAni.append_attribute("version").set_value(Version());
				a->ToXML(subAni, true);
				std::string linkedName = fleInf.GetFileName() + "_m" + std::to_string(curAniID) + ".xml";
				std::string linkedFullName = fleInf.GetPath() + linkedName;
				cAni.append_buffer(linkedName.c_str(), linkedName.size());
				linkAni.save_file(linkedFullName.c_str(), "\t", pugi::format_write_bom | pugi::format_indent);
			}
			else if (settings == ExportSetting_BinaryMotions)
			{
				std::string linkedName = fleInf.GetFileName() + "_m" + std::to_string(curAniID) + ".mti";
				std::string linkedFullName = fleInf.GetPath() + linkedName;
				cAni.append_buffer(linkedName.c_str(), linkedName.size());
				a->Save(linkedFullName.c_str());
			}

			
		}
		curAniID++;
	}


	return 0;
}

int LMT::ToXML(const char *fileName, ExportSettings settings)
{
	pugi::xml_document doc = {};
	ToXML(doc, fileName, settings);

	if (!doc.save_file(fileName, "\t", pugi::format_write_bom | pugi::format_indent))
	{
		printerror("[LMT] Couldn't save file: ", << fileName);
		return 1;
	}

	return 0;
}

int LMT::FromXML(pugi::xml_node &node, const char *fileName, Architecture forceArchitecture)
{
	auto children = node.children("LMT");
	std::vector<pugi::xml_node> mainNodes(children.begin(), children.end());

	if (!mainNodes.size())
	{
		printerror("[LMT] Couldn't find <LMT/>.");
		return 2;
	}

	if (mainNodes.size() > 1)
	{
		printwarning("[LMT] XML have too many root elements, only first processed.");
	}

	pugi::xml_node masterNode = *children.begin();	
	
	pugi::xml_attribute versionAttr = masterNode.attribute("version");

	if (versionAttr.empty())
	{
		printerror("[LMT] Missing <LMT version=/>");
		return 2;
	}

	pugi::xml_attribute archAttr = masterNode.attribute("X64");

	if (archAttr.empty())
	{
		printerror("[LMT] Missing <LMT X64=/>");
		return 2;
	}

	version = versionAttr.as_int();
	version |= (forceArchitecture == Xudefined && archAttr.as_bool()) || forceArchitecture == X64 ? x64Flag : 0;

	pugi::xml_attribute numItemsAttr = masterNode.attribute("numItems");

	auto animationIter = masterNode.children("Animation");
	std::vector<pugi::xml_node> animationNodes(animationIter.begin(), animationIter.end());

	if (!numItemsAttr.empty() && numItemsAttr.as_int() != animationNodes.size())
	{
		printwarning("[LMT] <LMT numItems=/> differs from actual <Animation/> count.");
	}

	animations.reserve(animationNodes.size());

	AFileInfo fleInf = fileName;

	for (auto &a : animationNodes)
	{
		pugi::xml_text nodeBuffer = a.text();

		if (nodeBuffer.empty())
		{
			auto animationSubNodesIter = a.children();
			std::vector<pugi::xml_node> animationSubNodes(animationSubNodesIter.begin(), animationSubNodesIter.end());

			if (!animationSubNodes.size())
			{
				animations.push_back(nullptr);
				continue;
			}

			LMTAnimation_internal *cAni = animationClassRegistry.at(version)();
			cAni->FromXML(a);
			animations.push_back(cAni);
		}
		else
		{
			const char *path = nodeBuffer.get();
			std::string absolutePath = path;
			BinReader rd(absolutePath);
			bool notMTMI = false;

			if (!rd.IsValid())
			{
				absolutePath = fleInf.GetPath() + path;
				rd.Open(absolutePath);
				if (!rd.IsValid())
				{
					printerror("[LMT] Couldn't load animation: ", << absolutePath.c_str());
					animations.push_back(nullptr);
					continue;
				}
			}

			LMTAnimation_internal *cAni = animationClassRegistry.at(version)();
			int errNo = cAni->Load(&rd, version);

			if (errNo == 1)
				notMTMI = true;
			else if (errNo == 2)
			{
				printerror("[LMT] Layout errors in animation: ", << absolutePath.c_str());

				if ((cAni->version & 0xff) != Version())
				{
					printerror("[LMT] Unexpected animation version: ", << cAni->version << ", expected: " << version);
				}

				bool expectedX64Arch = version & x64Flag;
				bool haveX64Arch = cAni->version & x64Flag;

				if (haveX64Arch != expectedX64Arch)
				{
					printerror("[LMT] Unexpected animation architecture: ", << (haveX64Arch ? "X64" : "X86") << ", expected: " << (expectedX64Arch ? "X64" : "X86"));
				}

				delete cAni;
				animations.push_back(nullptr);
				continue;
			}
			else if (errNo == 3)
			{
				printerror("[LMT] Animation is empty: ", << absolutePath.c_str());

				delete cAni;
				animations.push_back(nullptr);
				continue;
			}

			if (!notMTMI)
			{
				animations.push_back(cAni);
				continue;
			}

			pugi::xml_document subAnim = {};
			pugi::xml_parse_result reslt = subAnim.load_file(absolutePath.c_str());

			if (!reslt)
			{
				printerror("[LMT] Couldn't load animation: " << absolutePath.c_str() << ", " << GetXMLErrorMessage(reslt) << " at offset: " << reslt.offset);
				delete cAni;
				animations.push_back(nullptr);
				continue;
			}

			auto subAnimChildren = subAnim.children("Animation");
			std::vector<pugi::xml_node> subAniMainNodes(subAnimChildren.begin(), subAnimChildren.end());

			if (!subAniMainNodes.size())
			{
				printerror("[LMT] Couldn't find <Animation/>.");
				delete cAni;
				animations.push_back(nullptr);
				continue;
			}

			pugi::xml_node subAniMainNode = subAniMainNodes[0];
			pugi::xml_attribute subVersionAttr = subAniMainNode.attribute("version");

			if (subVersionAttr.empty())
			{
				printerror("[LMT] Missing <Animation version=/>");
				delete cAni;
				animations.push_back(nullptr);
				continue;
			}

			if (subVersionAttr.as_int() != Version())
			{
				printerror("[LMT] Unexpected animation version: ", << subVersionAttr.as_int() << ", expected: " << Version());
				delete cAni;
				animations.push_back(nullptr);
				continue;
			}

			cAni->FromXML(subAniMainNode);
			animations.push_back(cAni);
		}
	}
	
	return 0;
}

int LMT::FromXML(const char *fileName, Architecture forceArchitecture)
{
	pugi::xml_document doc = {};
	pugi::xml_parse_result reslt = doc.load_file(fileName);

	if (!reslt)
	{
		printerror("[LMT] Couldn't load xml file. " << GetXMLErrorMessage(reslt) << " at offset: " << reslt.offset);
		return 1;
	}

	return FromXML(doc, fileName, forceArchitecture);
}

static bool lmtInitialized = false;

LMT::LMT() : masterBuffer(nullptr), version(0)
{
	if (lmtInitialized)
		return;

	REGISTER_SUBCLASS(EventTablePointerX86);
	REGISTER_SUBCLASS(EventTablePointerX64);

	REGISTER_ENUM(TrackV1BufferTypes);
	REGISTER_ENUM(TrackV1_5BufferTypes);
	REGISTER_ENUM(TrackV2BufferTypes);
	REGISTER_ENUM(TrackType);
	REGISTER_ENUM(Buf_HermiteVector3_Flags);

	lmtInitialized = true;
}

LMT::~LMT()
{
	if (masterBuffer)
		free(masterBuffer);

	for (auto &a : animations)
		delete a;
}