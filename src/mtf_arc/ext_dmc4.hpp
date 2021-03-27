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

static const MtExtensionsStorage extDMC4Common{
    {"anm", 0x55A8FB34},    //
    {"arc", 0x21034C90},    //
    {"ase", 0xC88D5D1},     //
    {"atk", 0x652A93A4},    //
    {"bfx", 0x4E32817C},    //
    {"bin", 0x19DDF06A},    //
    {"bin", 0x7A5DCF86},    //
    {"cam", 0x743FE170},    //
    {"clt", 0x4D990996},    //
    {"col", 0x4EA4E09A},    //
    {"dfd", 0x27F3C33D},    //
    {"e2d", 0x76AA6987},    //
    {"ean", 0x5E7D6A45},    //
    {"efl", 0x294488A8},    //
    {"efs", 0x528770DF},    //
    {"eng", 0x16640CD4},    //
    {"engv", 0x1D076492},   //
    {"equ", 0x39F8A71D},    //
    {"evh", 0x6125D9CD},    //
    {"idx", 0x470745CB},    //
    {"lcm", 0x5EF1FB52},    //
    {"lmt", 0x139EE51D},    //
    {"mod", 0x1041BD9E},    //
    {"msg", 0x4CDF60E9},    //
    {"msse", 0xCA6AED4},    //
    {"nls", 0x5E4C723C},    //
    {"pla", 0x3F5955F1},    //
    {"rdf", 0x4D52E593},    //
    {"rla", 0x46771E81},    //
    {"rut", 0x2B93C4AD},    //
    {"rtex", 0x27CE98F6},   //
    {"sbc", 0x3900DAD0},    //
    {"scs", 0x94973CF},     //
    {"sdl", 0x44E79B6E},    //
    {"sds", 0x340F49F9},    //
    {"seg", 0x2E47C723},    //
    {"sif", 0x5D5CBBCE},    //
    {"spc", 0x33AE5307},    //
    {"sprmap", 0x34A8C353}, //
    {"srd", 0x29948FBA},    //
    {"srq", 0x6C1D2073},    //
    {"ssd", 0x3001BEC4},    //
    {"stq", 0x7D5909F},     //
    {"tex", 0x3CAD8076},    //
    {"vib", 0xD7DA737},     //
};

static const MtExtensionsStorage extDMC4WinPC{
    {"rev_win", 0x7A038F4C}, //
    {"sngw", 0x3821B94D},    //
};

static const MtExtensionsStorage extDMC4WinPS3{
    {"at3", 0x3821B94D},     //
    {"rev_ps3", 0x7A038F4C}, //
};

static const MtExtensions extDMC4{
    &extDMC4Common,                  //
    Platform::WinPC, &extDMC4WinPC,  //
    Platform::PS3,   &extDMC4WinPS3, //
};
