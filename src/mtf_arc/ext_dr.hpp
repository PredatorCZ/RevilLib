/*  Revil Format Library
    Copyright(C) 2020 Lukas Cone

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
#include "ext_base.hpp"

namespace MT_DR {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"ahl", 0x54503672},
    {"anm", 0x55A8FB34},
    {"arc", 0x21034C90},
    {"ast", 0x0532063F},
    {"bfx", 0x4E32817C},
    {"cdf", 0x234D7104},
    {"cdp", 0x75D21272},
    {"efa", 0x03FAE282},
    {"efs", 0x528770DF},
    {"ems", 0x2A51D160},
    {"esd", 0x32231BD1},
    {"esl", 0x482B5B95},
    {"fca", 0x264087B8},
    {"fcp", 0x4F6FFDDC},
    {"hkx", 0x3C3D0C05},
    {"hvd.xml", 0x26C299D0},
    {"hvl.xml", 0x1AEB54D1},
    {"lcm", 0x5EF1FB52},
    {"lge", 0x5E3DC9F3},
    {"lmt", 0x139EE51D},
    {"mdi.xml", 0x07D3088E},
    {"mod", 0x1041BD9E},
    {"modlayout.xml", 0x08EF36C1},
    {"mrk", 0x543E41DE},
    {"msg", 0x4CDF60E9},
    {"mtg", 0x20208A05},
    {"nls", 0x5E4C723C},
    {"npcfsmbrn", 0x3E394A0E},
    {"npcmac", 0x4126B31B},
    {"route", 0x2B93C4AD},
    {"rrd", 0x5A3CED86},
    {"rtex", 0x27CE98F6},
    {"sbc", 0x3900DAD0},
    {"scoop.xml", 0x4A31FCD8},
    {"sdl.xml", 0x11AFA688},
    {"seg", 0x2E47C723},
    {"smadd.xml", 0x0B0B8495},
    {"snd", 0x586995B1},
    {"sprmap", 0x34A8C353},
    {"tex", 0x3CAD8076},
    {"tex2", 0x7470D7E9},
    {"ubcell", 0x044BB32E},
    {"wed", 0x3D007115},
    {"xml", 0x124597FE},
    {"xml", 0x33B68E3E},
    {"xml", 0x5A45BA9C},
};

static const TitleSupport suppWin{
    {4},
    ModSupport{0x170},
    TexSupport{0x256},
    LmtSupport{22, true},
};
static const TitleSupport suppX360{
    {4},
    ModSupport{0x70},
    TexSupport{0x56},
    LmtSupport{22},
};

static const TitleSupports supp{
    Platform::Win32, suppWin, //
    Platform::X360, suppX360, //
};
} // namespace MT_DR

static const MtExtensions extDR{MT_DR::extCommon, MT_DR::supp, Platform::Win32};
