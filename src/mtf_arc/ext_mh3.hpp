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

static const MtExtensionsStorage extMH3Common{
    /**/ //
    {"ain", 0x12A794D8},
    {"ase", 0x07437cce},
    {"bdd", 0x1AC7B52D},
    {"dcm", 0x8F105E6},
    {"dmem", 0x1A032CE2},
    {"dmfd", 0x57D1CECD},
    {"dmlp", 0x54AE0DF9},
    {"ean", 0x4e397417},
    {"ear", 0x2E897056},
    {"efl", 0x6d5ae854},
    {"emc", 0x3AABBA02},
    {"emg", 0x705A17BE},
    {"eml", 0x482DDCBE},
    {"emm", 0x3BFEDD61},
    {"ems", 0x4DEE8265},
    {"emt", 0x3AF73507},
    {"esq", 0x525BBF16},
    {"gfd", 0x2d462600},
    {"gii", 0x07f768af},
    {"gmd", 0x242bb29a},
    {"gui", 0x22948394},
    {"htd", 0xC6016B9},
    {"hts", 0x5653C1B0},
    {"lfp", 0x57DC1ED1},
    {"list", 0x7284DAF5},
    {"lmt", 0x76820d81},
    {"mod", 0x58a15856},
    {"mrl", 0x2749c8a8},
    {"mss", 0x6E171A6E},
    {"quest", 0x1BBFD18E},
    {"rtl", 0xAAF2DB2},
    {"rvl", 0x254A0337},
    {"sbc", 0x51fc779f},
    {"scm", 0x176FADAB},
    {"sdl", 0x4c0db839},
    {"sdtl", 0x486B233A},
    {"tex", 0x241f5deb},
};

static const MtExtensionsStorage extMH3N3DS{
    /**/ //
    {"cfl", 0x7BEA3086},
    {"mca", 0x67195A2E},
    {"rev_ctr", 0x4A4B677C},
    {"sbk", 0x14B5C8E6},
    {"srq", 0x2618DE3F},
};

static const MtExtensionsStorage extMH3Cafe{
    /**/ //
    {"dspw", 0x617B0C47},
    {"equ", 0x2B40AE8F}, // dupe
    {"equr", 0x2B40AE8F},
    {"gr2", 0x11C35522},
    {"gr2s", 0x628DFB41},
    {"revr_cafe", 0x232E228C},
    {"sbkr", 0x15D782FB},
    {"spkg", 0x4E399FFE},
    {"srqr", 0x1BCC4966},
    {"srq", 0x1BCC4966}, // dupe
};

static const MtExtFixupStorage fixupMH3{
    {0x1BCC4966, "srqr"},
    {0x2B40AE8F, "equr"},
};

static const MtExtensions extMH3{
    &extMH3Common,               //
    Platform::N3DS, &extMH3N3DS, //
    Platform::CAFE, &extMH3Cafe, //
    &fixupMH3,                   //
};
