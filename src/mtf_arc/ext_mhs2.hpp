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

namespace MT_MHS2 {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"adpcm", 0x79C47B59},
    {"asd", 0x3565348B},
    {"bccam", 0x37CBA824},
    {"btat", 0x1688C822},
    {"ccinfo", 0x18D25FEC},
    {"ccl", 0x0026E7FF},
    {"cfid", 0x494EA0DD},
    {"clc", 0x21684FE4},
    {"cli", 0x2D4CF80A},
    {"cmdt", 0x50FE2B3B},
    {"ctc", 0x535D969F},
    {"cut", 0x10A9B96B},
    {"dai", 0x51A88271},
    {"dpd", 0x2FE088C0},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"efs", 0x02833703},
    {"equ", 0x2B40AE8F},
    {"fci", 0x125E09BD},
    {"fco", 0x790F56DE},
    {"fcr", 0x3C6F8994},
    {"fdi", 0x31FDE194},
    {"fntl", 0x18C80D7A},
    {"fol", 0x1D8CC9B7},
    {"fpc", 0x3366F207},
    {"fpctl", 0x27662FAE},
    {"fpi", 0x10A7BB93},
    {"fpl", 0x445F93C9},
    {"fpos", 0x666F5559},
    {"fslm", 0x7370D1FC},
    {"fsm", 0x66B45610},
    {"gar", 0x02178810},
    {"gfd", 0x2D462600},
    {"gii", 0x07F768AF},
    {"gmd", 0x242BB29A},
    {"gr2", 0x11C35522},
    {"gr2s", 0x628DFB41},
    {"gst", 0x091F3631},
    {"gstd", 0x0DF1E4CD},
    {"gui", 0x22948394},
    {"chcl", 0x06CBC2AF},
    {"idcol", 0x50475E5B},
    {"idd", 0x101B237C},
    {"ik", 0x5A7FEA62},
    {"ipr", 0x5554B934},
    {"kofb", 0x1B028405},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"lsnl", 0x5A7A72DE},
    {"mea", 0x7534679E},
    {"mod", 0x58A15856},
    {"mpd", 0x3F53ECC9},
    {"mrl", 0x2749C8A8},
    {"nasl", 0x1FA3FF7A},
    {"npd", 0x4F1544F5},
    {"npsl", 0x3876D6FA},
    {"occ2", 0x5DCEA23F},
    {"pec", 0x6FCC7AD4},
    {"pel", 0x5A525C16},
    {"pep", 0x20ED9750},
    {"pmc", 0x0AD816B6},
    {"pos", 0x763119DC},
    {"psl", 0x254309C9},
    {"pts", 0x5842F0B0},
    {"pvi", 0x54DD639F},
    {"rdp", 0x29A5C1D1},
    {"revr", 0x232E228C},
    {"rtex", 0x7808EA10},
    {"sbc", 0x51FC779F},
    {"sbkr", 0x15D782FB},
    {"scl", 0x1D6CAB29},
    {"scs", 0x0ECD7DF4}, // dupe
    {"scsr", 0x0ECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sds", 0x0315E81F},
    {"snd", 0x5B9300CB},
    {"spkg", 0x02358E1A},
    {"spll", 0x67DE29F7},
    {"sptl", 0x1657A2D0},
    {"srcd", 0x06A7028B},
    {"srqr", 0x1BCC4966},
    {"stqr", 0x167DBBFF},
    {"swm", 0x257D2F7C},
    {"tex", 0x241F5DEB},
    {"ts", 0x62711A14},
    {"vib", 0x358012E8},
    {"vjr", 0x215E5305},
    {"wgi", 0x049F01BD},
    {"wofb", 0x6ABA51B0},
    {"work", 0x2B73DB4A},
    {"wpdt", 0x701B8556},
};

static const MtExtFixupStorage fixups{
    /**/ //
    {0xECD7DF4, "scsr"},
};

static const TitleSupport suppNSW{
    ArcSupport{7, 15, false, false, false, true},
    ModSupport{0xD6},
    TexSupport{0xA3},
    LmtSupport{68},
};

static const TitleSupports supp{Platform::NSW, suppNSW};
} // namespace MT_MHS2

static const MtExtensions extMHS2{MT_MHS2::extCommon, MT_MHS2::fixups,
                                  MT_MHS2::supp, Platform::NSW};
