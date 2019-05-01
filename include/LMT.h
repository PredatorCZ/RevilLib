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
#include "datas/supercore.hpp"

class BinReader;
class BinWritter;

namespace pugi
{
	class xml_node;
}

class LMTAnimation
{
public:
	virtual const int NumTracks() const = 0;
	virtual int NumEventBlocks() const = 0;
	virtual int ToXML(pugi::xml_node &node, bool standAlone = false) const = 0;
	virtual int FromXML(pugi::xml_node &node) = 0;
	virtual int Save(const char *fileName) const = 0;
	virtual ~LMTAnimation() {}
};

class LMTTrack
{
public:
	enum TrackType
	{
		TrackType_LocalRotation,
		TrackType_LocalPosition,
		TrackType_LocalScale,
		TrackType_AbsoluteRotation,
		TrackType_AbsolutePosition
	};
	virtual ~LMTTrack() {}
};

class LMT
{
	char *masterBuffer;
	short version;
	std::vector<LMTAnimation *> animations;
public:
	static const short x64Flag = 0x100;
	static const int ID = CompileFourCC("LMT\0");
	static const int ID_R = CompileFourCC("\0TML");

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

	void AddAnimation(LMTAnimation *ani);
	void InsertAnimation(LMTAnimation *ani, int at);
	
	int Load(BinReader *rd);
	int ToXML(pugi::xml_node &node, const char *fileName, ExportSettings settings);
	int ToXML(const char *fileName, ExportSettings settings);

	int FromXML(pugi::xml_node &node, const char *fileName, Architecture forceArchitecture = Xudefined);
	int FromXML(const char *fileName, Architecture forceArchitecture = Xudefined);

	LMT();
	~LMT();
};

// TODO:
/*
XML event input,
events for Version 3 animations
lmt export

low priority:
conversion system << super low
devaluators for buf_ classes
encoder
*/