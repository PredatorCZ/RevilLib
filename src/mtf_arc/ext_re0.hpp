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

namespace MT_RE0 {
static const MtExtensionsStorage extCommon{
    {"bct2", 0x405FF76E},    //
    {"bds2", 0x01F28535},    //
    {"bes2", 0x714EC77C},    //
    {"bscn", 0x0B2FACE7},    //
    {"ccl", 0x0026E7FF},     //
    {"cspp", 0x04E9EBE9},    //
    {"ctc", 0x535D969F},     //
    {"e2d", 0x276DE8B7},     //
    {"ean", 0x4E397417},     //
    {"efcc", 0x09D775FC},    //
    {"efl", 0x6D5AE854},     //
    {"efs", 0x02833703},     //
    {"epv", 0x12191BA1},     //
    {"evc2", 0x2B399B97},    //
    {"gfd", 0x2D462600},     //
    {"gii", 0x07F768AF},     //
    {"gmd", 0x242BB29A},     //
    {"gui", 0x22948394},     //
    {"itm", 0x5E640ACC},     //
    {"jex", 0x2282360D},     //
    {"lcm", 0x39C52040},     //
    {"lmt", 0x76820D81},     //
    {"loc2", 0x059BC928},    //
    {"mem.wmv", 0x5F84F7C4}, //
    {"mloc", 0x44C8AC26},    // arc only
    {"mod", 0x58A15856},     //
    {"mps", 0x053C8864},     //
    {"mrl", 0x2749C8A8},     //
    {"msd", 0x3991981E},     //
    {"pcos", 0x0BCF07BD},    //
    {"pmtt", 0x0F9F3E69},    //
    {"prp", 0x272B80EA},     //
    {"rbl2", 0x1F9FA62C},    //
    {"rclp", 0x25E98D8C},    //
    {"rev_win", 0x232E228C}, //
    {"rtex", 0x7808EA10},    //
    {"rvt", 0x039D71F2},     //
    {"sbc", 0x51FC779F},     //
    {"sbkr", 0x15D782FB},    //
    {"scs", 0x0ECD7DF4},     //
    {"sdl", 0x4C0DB839},     //
    {"sds", 0x0315E81F},     //
    {"spkg", 0x02358E1A},    //
    {"srq", 0x1BCC4966},     //
    {"stef", 0x1DC202EA},    //
    {"tde", 0x2D6455B1},     //
    {"tex", 0x241F5DEB},     //
    {"uce2", 0x68E301A1},    //
    {"ucp2", 0x3CBBFD41},    //
    {"uct2", 0x531EA143},    //
    {"vib", 0x358012E8},     //
    {"wad", 0x3EE10653},     //
    {"wao", 0x3C318636},     //
    {"wpt", 0x5F12110E},     //
    {"xsew", 0x724DF879},    //
};

static const TitleSupport suppWin{
    ARC_WINPC_GENERIC,
    ModSupport{0xD2},
    TexSupport{0x9D},
    LmtSupport{67},
};

static const TitleSupports supp{Platform::Win32, suppWin};
} // namespace MT_RE0

static const MtExtensions extRE0{MT_RE0::extCommon, MT_RE0::supp,
                                 Platform::Win32};
