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


#pragma once
#include "datas/vectors.hpp"
#include "datas/flags.hpp"

class BinReader;
class REAsset;

template<class C>
union REPointer
{
	uint64 varPtr;
	C *ptr;

	void Fixup(char *masterBuffer) 
	{
		if (!varPtr)
			return;

		ptr = reinterpret_cast<C *>(masterBuffer + varPtr); 
	}

	template<class _C = C>
	typename std::enable_if<!std::is_void_v<_C>, _C>::type &operator *() { return *ptr; }

	template<class _C = C>
	typename std::enable_if<std::is_void_v<_C>, void>::type operator *() {}

	template<class _C = C>
	typename std::enable_if<!std::is_void_v<_C>, _C>::type &operator [](size_t index) { return ptr[index]; }

	template<class _C = C>
	typename std::enable_if<std::is_void_v<_C>, void>::type operator [](size_t index) {}

	C *operator->() { return ptr; }
};

class REAssetBase
{
public:
	virtual int Fixup(char *masterBuffer) = 0;
	virtual ~REAssetBase() {}
};


class REMotlist : REAssetBase
{
	int Fixup(char *masterBuffer);
public:
	static const uint ID = CompileFourCC("mlst");
	REPointer<REPointer<REAssetBase>> motions;
	REPointer<void> unkOffset00;
	REPointer<char16_t> fileName;
	REPointer<void> null;
	int numMotions;
};

struct REMotionBone
{
	int boneID,
		boneHash;
	int64 numChildrenFromRootBone;
	REPointer<char16_t> boneName;
	REPointer<REPointer<char16_t>>
		parentBoneNamePtr,
		firstChildBoneNamePtr,
		lastChildBoneNamePtr;
	Vector4 position;
	Vector4 rotation;

	int Fixup(char *masterBuffer);
};

struct REMimMaxBounds
{
	Vector4 min;
	Vector4 max;
};

struct RETrackCurve
{
	short unk01,
		compression;
	int numFrames,
		framesPerSecond;
	float duration;
	REPointer<short> frames;
	REPointer<char> controlPoints;
	REPointer<REMimMaxBounds> minMaxBounds;

	int Fixup(char *masterBuffer);
};

struct REMotionTrack
{
	enum TrackType
	{
		TrackType_Position,
		TrackType_Rotation,
		TrackType_Scale,
	};

	short unk;
	esFlags<short, TrackType> usedCurves;
	int boneHash;
	float weight;
	REPointer<RETrackCurve> curves;

	int Fixup(char *masterBuffer);
};

class REMotion : REAssetBase
{
	int Fixup(char *masterBuffer);
public:
	static const uint ID = CompileFourCC("mot ");

	REPointer<REMotionBone> bones;
	REPointer<REMotionTrack> tracks;

	REPointer<void> null[5];

	REPointer<void> unkOffset02;
	REPointer<char16_t> animationName;

	float intervals[4];
	short numBones;
	short numTracks;
	short numUNK00;
	short framesPerSecond;
	short unks00[2];
};

class REAsset
{
	uint assetID,
		assetFourCC;
	REAssetBase *object;
	char *masterBuffer;

	template<class _Ty0> 
	//typedef wchar_t _Ty0;
	int _Load(const _Ty0 *fileName);
public:

	REAsset() : assetID(0), assetFourCC(0), object(nullptr), masterBuffer(nullptr) {}
	~REAsset();
	int Load(const wchar_t *fileName) { return _Load(fileName); }
	int Load(const char *fileName) { return _Load(fileName); }
	int Load(BinReader *rd);
	int Assign(char *ptr);
	REAssetBase *Object() { return object; }
};