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
#include <vector>
#include "datas/vectors.hpp"

class BinReader;
class BinWritter;

namespace pugi
{
	class xml_node;
}

struct LMTTrack
{
	enum TrackType
	{
		TrackType_LocalRotation,
		TrackType_LocalPosition,
		TrackType_LocalScale,
		TrackType_AbsoluteRotation,
		TrackType_AbsolutePosition
	};

	virtual TrackType GetTrackType() const = 0;
	virtual int AnimatedBoneID() const = 0;
	virtual int NumFrames() const = 0;
	virtual bool IsCubic() const = 0;
	virtual void GetTangents(Vector4 &inTangs, Vector4 &outTangs, int frame) const = 0;
	virtual void Evaluate(Vector4 &out, int frame) const = 0;
	virtual short GetFrame(int frame) const = 0;

	virtual ~LMTTrack() {}
};

class LMTAnimation
{
public:
	virtual const int NumTracks() const = 0;
	virtual const LMTTrack *Track(int id) const = 0;
	virtual int NumEventBlocks() const = 0;
	virtual int ToXML(pugi::xml_node &node, bool standAlone = false) const = 0;
	virtual int FromXML(pugi::xml_node &node) = 0;
	virtual int Save(const char *fileName) const = 0;
	virtual ~LMTAnimation() {}
};

class LMT
{	
	static const int ID = CompileFourCC("LMT\0");
	static const int ID_R = CompileFourCC("\0TML");
public:
	typedef std::vector<LMTAnimation *> Storage_Type;
	typedef Storage_Type::const_iterator Iter_Type;
	static const short x64Flag = 0x100;

	enum V
	{
		V_40 = 40, //LP
		V_49 = 49, //DMC4
		V_50 = 50, //LP PS3
		V_51 = 51, //RE5
		V_56 = 56, //LP2
		V_57 = 57, //RE:M 3DS
		V_66 = 66, //DD, DD:DA
		V_67 = 67, //Other, generic MTF v2 format
		V_92 = 92, //MH:W
	};

	enum Architecture
	{
		Xudefined,
		X86,
		X64
	};

	enum ExportSettings
	{
		ExportSetting_FullXML,
		ExportSetting_FullXMLLinkedMotions,
		ExportSetting_BinaryMotions
	};
	
	ES_FORCEINLINE short Version() const { return version & 0xff; }
	ES_FORCEINLINE Architecture GetArchitecture() const { return (version & x64Flag) ? X64 : X86; }
	int Version(V version, Architecture arch);

	ES_FORCEINLINE const Iter_Type begin() const { return animations.begin(); }
	ES_FORCEINLINE const Iter_Type end() const { return animations.end(); }
	ES_FORCEINLINE int NumAnimations() const { return static_cast<int>(animations.size()); }
	ES_FORCEINLINE const LMTAnimation *Animation(int id) const { return animations[id]; }

	LMTAnimation *AppendAnimation();
	void AppendAnimation(LMTAnimation *ani);
	void InsertAnimation(LMTAnimation *ani, int at, bool replace = false);
	
	int Load(BinReader *rd);
	int Save(BinWritter *wr);

	int ToXML(pugi::xml_node &node, const char *fileName, ExportSettings settings);
	int ToXML(const char *fileName, ExportSettings settings);

	int FromXML(pugi::xml_node &node, const char *fileName, Architecture forceArchitecture = Xudefined);
	int FromXML(const char *fileName, Architecture forceArchitecture = Xudefined);

	LMT();
	~LMT();

private:
	char *masterBuffer;
	short version;
	Storage_Type animations;
};

// TODO:
/*
float events for version 2 animation
offsets to error messages for XML
events for Version 3 animations
sanitize XML to LMT correction

low priority:
conversion system << super low
encoder, exporting utility
*/