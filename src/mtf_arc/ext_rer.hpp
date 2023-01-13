/*  Revil Format Library
    Copyright(C) 2021 Lukas Cone

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

namespace MT_RER {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"arc", 0x73850D05},
    {"arg", 0x59BC928},
    {"ase", 0x7437CCE},
    {"atk", 0x6E45FABB}, // dupe
    {"atp", 0x6E45FABB},
    {"blt", 0x5288748},
    {"cad", 0x1A5444D9},
    {"ctc", 0x535D969F},
    {"cwc", 0x7A098625},
    {"dpt", 0x13AC3D41},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"efs", 0x2833703},
    {"elt", 0x25697144},
    {"epl", 0x1CC22F91},
    {"epm", 0x235A4890},
    {"fsm", 0x66B45610},
    {"gcd", 0x2A3A6664},
    {"gfd", 0x2D462600},
    {"gmd", 0x242BB29A},
    {"gui", 0x22948394},
    {"itp", 0x7A1F334C},
    {"lar", 0x3121865A},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"loc2", 0x59BC928}, // dupe
    {"lvt", 0xA3E643B},
    {"mod", 0x58A15856},
    {"mrl", 0x2749C8A8},
    {"mse", 0x4CA26828},
    {"pfp", 0x5F03BE0B},
    {"resmap", 0x65D3AC8E},
    {"rrb", 0x5521BB96},
    {"rrh", 0x61A3CCB0},
    {"rtex", 0x7808EA10},
    {"sbc", 0x51FC779F},
    {"sdl", 0x4C0DB839},
    {"ssq", 0x271D08FE},
    {"svd", 0x1C0776A9},
    {"swp", 0x51120F02},
    {"tbl", 0x791DA5C9},
    {"tde", 0x2D6455B1},
    {"tex", 0x241F5DEB},
    {"trc", 0x3B9C41F1},
    {"way", 0x5F36B659},
    {"zon", 0x1B520B68},
};

static const MtExtensionsStorage extWinPC{
    /**/ //
    {"csl", 0x5C0B0996},
    {"dim", 0x72763DAF},
    {"equ", 0x2B40AE8F},
    {"evm", 0x49BDEA0C},
    {"fcm", 0x928F2DD},
    {"fls", 0x61BC4073},
    {"glp", 0x3F40BDB8},
    {"jex", 0x2282360D},
    {"jul", 0x49A855DA},
    {"mcl", 0x271E8A8F},
    {"mcm", 0x17917970},
    {"occ", 0x6A5CDD23},
    {"ppv", 0x891AF32},
    {"rev_win", 0x232E228C},
    {"rmm", 0x637908D0},
    {"sbp", 0x55360EC9},
    {"scd", 0x32E94AC3},
    {"scm", 0x6CA1D49A},
    {"scs", 0xECD7DF4},
    {"spkg", 0x2358E1A},
    {"ssl", 0x2A3A6196},
    {"stex", 0x4323D83A},
    {"swmt", 0x13CFA2FA},
    {"tca", 0x5A4767A9},
    {"tcm", 0xB1E982A},
    {"vib", 0x358012E8},
    {"xsew", 0x724DF879},
};

static const MtExtensionsStorage ext3DS{
    /**/ //
    {"ecd", 0x2B399B97},
    {"evc2", 0x2B399B97}, // dupe
    {"mca", 0x67195A2E},
    {"rvl", 0x254A0337},
    {"sbk", 0x14B5C8E6},
    {"srq", 0x2618DE3F},
    {"stq", 0x3A6A5A4D},
};

static const MtExtFixupStorage fixups{
    {0x059BC928, "arg"},
    {0x2B399B97, "ecd"},
    {0x6E45FABB, "atp"},
};

static const TitleSupport supp3DS{
    ArcSupport{0x10},
    ModSupport{0xE6},
    TexSupport{0xA5},
    LmtSupport{67},
};

static const TitleSupport suppWin{
    ARC_WINPC_GENERIC,
    ModSupport{0xD2},
    TexSupport{0x9D},
    LmtSupport{67},
};

static const TitleSupports supp{
    Platform::N3DS, supp3DS,  //
    Platform::Win32, suppWin, //
};
} // namespace MT_RER

static const MtExtensions extRER{
    MT_RER::extCommon,                   //
    MT_RER::fixups,                      //
    Platform::Win32,   MT_RER::extWinPC, //
    Platform::N3DS,    MT_RER::ext3DS,   //
};
