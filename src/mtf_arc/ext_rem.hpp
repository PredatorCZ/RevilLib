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

static const MtExtensionsStorage extREMCommon{
    /**/ //
    {"amp", 0x264D1A85},
    {"arc", 0x73850D05},
    {"arg", 0x59BC928},
    {"ase", 0x7437CCE},
    {"atp", 0x6E45FABB},
    {"cad", 0x1A5444D9},
    {"ctc", 0x535D969F},
    {"cwc", 0x7A098625},
    {"ean", 0x4E397417},
    {"ecd", 0x2B399B97},
    {"efl", 0x6D5AE854},
    {"elt", 0x25697144},
    {"epl", 0x1CC22F91},
    {"epm", 0x235A4890},
    {"evc2", 0x2B399B97}, // dupe
    {"fcm", 0x928F2DD},
    {"fsm", 0x66B45610},
    {"gui", 0x22948394},
    {"lar", 0x3121865A},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"mca", 0x67195A2E},
    {"mod", 0x58A15856},
    {"mot", 0x5AF4E4FE},
    {"mrl", 0x2749C8A8},
    {"mse", 0x4CA26828},
    {"msn", 0x361EA2A5},
    {"rvl", 0x254A0337},
    {"sbc", 0x51FC779F},
    {"sbk", 0x14B5C8E6},
    {"scm", 0x6CA1D49A},
    {"sdl", 0x4C0DB839},
    {"srq", 0x2618DE3F},
    {"stp", 0x2298C7AA},
    {"stq", 0x3A6A5A4D},
    {"svd", 0x1C0776A9},
    {"svl", 0x3C9269F0},
    {"swp", 0x51120F02},
    {"tcm", 0xB1E982A},
    {"tex", 0x241F5DEB},
    {"way", 0x5F36B659},
    {"zon", 0x1B520B68},
};

static const MtExtFixupStorage fixupREM{
    {0x2B399B97, "ecd"},
};

static const MtExtensions extREM{&extREMCommon, &fixupREM, Platform::N3DS};
