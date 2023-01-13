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

namespace MT_LP {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"ahl", 0x54503672},
    {"anm", 0x55A8FB34},
    {"arc", 0x21034C90},
    {"asl", 0x338C1FEC},
    {"bfx", 0x4E32817C},
    {"cdf", 0x234D7104},
    {"efa", 0x03FAE282},
    {"efs", 0x528770DF},
    {"esl", 0x482B5B95},
    {"fca", 0x264087B8},
    {"fcp", 0x4F6FFDDC},
    {"hcl", 0x57BAE388},
    {"hit", 0x1D35AF2B},
    {"hkm", 0x5435D27B},
    {"hkx", 0x3C3D0C05},
    {"hlc", 0x6C3B4904},
    {"hvl.xml", 0x1AEB54D1},
    {"lcm", 0x5EF1FB52},
    {"lmt", 0x139EE51D},
    {"mod", 0x1041BD9E},
    {"oba", 0x11C82587},
    {"osf", 0x71D6A0D4},
    {"route", 0x2B93C4AD},
    {"rrd", 0x5A3CED86},
    {"rsl", 0x652071B0},
    {"rtex", 0x27CE98F6},
    {"sbc", 0x3900DAD0},
    {"sbl", 0x0D3BE7B5},
    {"scs", 0x094973CF},
    {"sdl", 0x44E79B6E},
    {"sds", 0x340F49F9},
    {"seg", 0x2E47C723},
    {"seq", 0x48459606},
    {"spc", 0x33AE5307},
    {"sprmap", 0x34A8C353},
    {"srd", 0x29948FBA},
    {"srq", 0x6C1D2073},
    {"stq", 0x07D5909F},
    {"tex", 0x3CAD8076},
    {"wed", 0x3D007115},
};

static const MtExtensionsStorage extWin{
    {"fur", 0x191F0AC9},
    {"sngw", 0x3821B94D},
    {"rev_win", 0x7A038F4C},
};

static const MtExtensionsStorage extPS3{
    /**/ //
    {"at3", 0x3821B94D},
    {"rev_ps3", 0x7A038F4C},
    {"hvd.xml", 0x26C299D0},
    {"esd", 0x32231BD1},
    {"msg", 0x10C460E6},
};

static const TitleSupport suppWin{
    ARC_WINPC_GENERIC,
    ModSupport{0x99},
    TexSupport{0x70},
    LmtSupport{40},
};

static const TitleSupport suppPS3{
    ARC_PS3_GENERIC,
    ModSupport{0x99},
    TexSupport{0x66},
    LmtSupport{50},
};

static const TitleSupports supp{
    Platform::Win32, suppWin, //
    Platform::PS3, suppPS3,   //
};
} // namespace MT_LP

static const MtExtensions extLP{
    MT_LP::extCommon,                //
    MT_LP::supp,                     //
    Platform::PS3,    MT_LP::extPS3, //
    Platform::Win32,  MT_LP::extWin,
};
