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

namespace MT_MH4 {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"ase", 0x07437CCE},
    {"ccl", 0x0026E7FF},
    {"cfl", 0x7BEA3086},
    {"ctc", 0x535D969F},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"emc", 0x3AABBA02},
    {"emd", 0xA0E48D4},
    {"etd", 0x3D8BBBAC},
    {"evt", 0x60869A71},
    {"gr2", 0x11C35522},
    {"gr2s", 0x628DFB41},
    {"grw", 0x0437BCF2},
    {"ipl", 0x25FD693F},
    {"ips", 0x52776AB1},
    {"lanl", 0x708E0028},
    {"lcm", 0x39C52040},
    {"lfd", 0x3516C3D2},
    {"lmd", 0x62440501},
    {"lmt", 0x76820D81},
    {"lyt", 0x15302EF4},
    {"mca", 0x67195A2E},
    {"mef", 0x148B6F89},
    {"mod", 0x58A15856},
    {"mrl", 0x2749C8A8},
    {"mss", 0x6E171A6E},
    {"quest", 0x1BBFD18E}, //??
    {"rev_ctr", 0x4A4B677C},
    {"sai", 0x2A8800EE},
    {"sbc", 0x51FC779F},
    {"sbk", 0x14B5C8E6},
    {"scd", 0x6B41A2F9},
    {"scs", 0x0ECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sds", 0x0315E81F},
    {"ses", 0x68CD2933},
    {"sis", 0x65375D5},
    {"skmt", 0x70C56D5E},
    {"sksg", 0x2C59EEA9},
    {"skst", 0x19278D07},
    {"srq", 0x2618DE3F},
    {"stq", 0x3A6A5A4D},
    {"tex", 0x241F5DEB},
};

static const TitleSupport supp3DS{
    ArcSupport{0x13},
    ModSupport{0xE6},
    TexSupport{0xA5},
    LmtSupport{67},
};

static const TitleSupports supp{Platform::N3DS, supp3DS};
} // namespace MT_MH4

static const MtExtensions extMH4{MT_MH4::extCommon, MT_MH4::supp,
                                 Platform::N3DS};
