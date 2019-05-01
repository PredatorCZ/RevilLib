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

int REAsset::Load(BinReader *rd)
{
	const size_t fleSize = rd->GetSize();
	masterBuffer = static_cast<char *>(malloc(fleSize));
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
	tracks.Fixup(masterBuffer);
	unkOffset02.Fixup(masterBuffer);
	animationName.Fixup(masterBuffer);

	for (int b = 0; b < numBones; b++)
		bones[b].Fixup(masterBuffer);

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

	for (int t = 0; t < 3; t++)
		if (usedCurves[static_cast<TrackType>(t)])
			curves[t].Fixup(masterBuffer);

	return 0;
}

int RETrackCurve::Fixup(char *masterBuffer)
{
	frames.Fixup(masterBuffer);
	controlPoints.Fixup(masterBuffer);
	minMaxBounds.Fixup(masterBuffer);

	return 0;
}
