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

static const MtExtensionsStorage extLP2Common{
    {"abe", 0x56CF8411},  //
    {"ahl", 0x2C14D261},  //
    {"arc", 0x73850D05},  //
    {"asl", 0x552E1B82},  //
    {"bta", 0x64BFE66D},  //
    {"ccl", 0x0026E7FF},  //
    {"dsc", 0x75967AD6},  //
    {"dwm", 0x69A5C538},  //
    {"e2d", 0x276DE8B7},  //
    {"ean", 0x4E397417},  //
    {"efl", 0x6D5AE854},  //
    {"efs", 0x02833703},  //
    {"epv", 0x12191BA1},  //
    {"fes", 0x27D81C81},  //
    {"fsm", 0x66B45610},  //
    {"gfc", 0x40D14033},  //
    {"gmd", 0x3FDA9B90},  //
    {"grd", 0x2CC70A66},  //
    {"grs", 0x2739B57C},  //
    {"grw", 0x0437BCF2},  //
    {"hcl", 0x7494F854},  //
    {"hit", 0x0253F147},  //
    {"hkx", 0x36E29465},  //
    {"hlc", 0x55EF9710},  //
    {"hmg", 0x2A9FBCEC},  //
    {"hvd", 0x753B3A8C},  //
    {"chn", 0x3E363245},  //
    {"ibl", 0x37B53731},  //
    {"ik", 0x5A7FEA62},   //
    {"ips", 0x5CC5C1E2},  //
    {"lcm", 0x39C52040},  //
    {"lmt", 0x76820D81},  //
    {"lnv", 0x56CF93D4},  //
    {"lpeq", 0x755BDD19}, //
    {"mod", 0x58A15856},  //
    {"mse", 0x4CA26828},  //
    {"oba", 0x0DADAB62},  //
    {"obc", 0x2350E584},  //
    {"occ", 0x6A5CDD23},  //
    {"osf", 0x711BD05E},  //
    {"pai", 0x0DE13237},  //
    {"pef", 0x20A81BF0},  //
    {"pvb", 0x745DAB77},  //
    {"rsl", 0x2E590330},  //
    {"rtex", 0x7808EA10}, //
    {"sbc", 0x51FC779F},  //
    {"sbl", 0x1C775347},  //
    {"scs", 0x0ECD7DF4},  //
    {"sdl", 0x4C0DB839},  //
    {"sds", 0x0315E81F},  //
    {"seg", 0x38F66FC3},  //
    {"smx", 0x30FC745F},  //
    {"spc", 0x7E33A16C},  //
    {"spkg", 0x02358E1A}, //
    {"srd", 0x2D12E086},  //
    {"srq", 0x1BCC4966},  //
    {"ssq", 0x271D08FE},  //
    {"stex", 0x4323D83A}, //
    {"stq", 0x167DBBFF},  //
    {"swc", 0x31AC0B5C},  //
    {"swm", 0x257D2F7C},  //
    {"tde", 0x6F62D575},  //
    {"tex", 0x241F5DEB},  //
    {"thk", 0x50F3721F},  //
    {"ttb", 0x62A68441},  //
    {"vib", 0x358012E8},  //
    {"vts", 0x1AE50150},  //
    {"way", 0x5F36B659},  //
};

static const MtExtensionsStorage extLP2Win{
    {"gnd", 0x20FA758C},  //
    {"hkm", 0x018735A6},     //
    {"mot", 0x5AF4E4FE},     //
    {"msl", 0x1C635F38},     //
    {"wb", 0x67015AE5},      //
    {"sngw", 0x7D1530C2},    //
    {"rev_win", 0x4857FD94}, //
};

static const MtExtensionsStorage extLP2PS3{
    {"at3", 0x7D1530C2},     //
    {"gnd", 0x2993EA18},  //
    {"rev_ps3", 0x4857FD94}, //
};

static const MtExtensions extLP2{&extLP2Common, &extLP2PS3, &extLP2Win};
