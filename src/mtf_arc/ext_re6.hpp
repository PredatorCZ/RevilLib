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

static const MtExtensionsStorage extRE6Common{
    {"adh", 0x2C2DE8CA},  //
    {"ahc", 0x5802B3FF},  //
    {"ahs", 0x1FA8B594},  //
    {"ase", 0x07437CCE},  //
    {"atk", 0x6E45FABB},  //
    {"bgm", 0x25B4A6B9},  //
    {"bmt", 0x46FB08BA},  //
    {"bssq", 0x5FB399F4}, //
    {"ccl", 0x0026E7FF},  //
    {"cmr", 0x245133D9},  //
    {"cms", 0x1AADF7B7},  //
    {"cos", 0x14EA8095},  //
    {"ctc", 0x535D969F},  //
    {"dwm", 0x69A5C538},  //
    {"e2d", 0x276DE8B7},  //
    {"ean", 0x4E397417},  //
    {"efl", 0x6D5AE854},  //
    {"efs", 0x02833703},  //
    {"egv", 0x46810940},  //
    {"eng", 0x538120DE},  //
    {"epv", 0x12191BA1},  //
    {"equ", 0x2B40AE8F},  //
    {"ewy", 0x622FA3C9},  //
    {"fsm", 0x66B45610},  //
    {"geo2", 0x5175C242}, //
    {"gfd", 0x2D462600},  //
    {"gii", 0x07F768AF},  //
    {"glp", 0x1ED12F1B},  //
    {"gmd", 0x242BB29A},  //
    {"gr2", 0x11C35522},  //
    {"gr2s", 0x628DFB41}, //
    {"grs", 0x2739B57C},  //
    {"grw", 0x0437BCF2},  //
    {"gui", 0x22948394},  //
    {"hgm", 0x296BD0A6},  //
    {"hit", 0x0253F147},  //
    {"ida", 0x3B5C7FD3},  //
    {"igs", 0x45F753E8},  //
    {"ich", 0x6BB4ED5E},  //
    {"jex", 0x2282360D},  //
    {"lcm", 0x39C52040},  //
    {"lku", 0x266E8A91},  //
    {"llk", 0x4B92D51A},  //
    {"lmt", 0x76820D81},  //
    {"lnv", 0x56CF93D4},  //
    {"lot", 0x15302EF4},  //
    {"lrd", 0x02A80E1F},  //
    {"mll", 0x45E867D7},  //
    {"mod", 0x58A15856},  //
    {"mrl", 0x2749C8A8},  //
    {"mse", 0x4CA26828},  //
    {"msl", 0x54DC440A},  //
    {"mtg", 0x4E2FEF36},  //
    {"nav", 0x4EF19843},  //
    {"occ", 0x6A5CDD23},  //
    {"poa", 0x35BDD173},  //
    {"prp", 0x272B80EA},  //
    {"qcm", 0x33046CD5},  //
    {"rbd", 0x2A4F96A8},  //
    {"rdd", 0x52DBDCD6},  //
    {"rpd", 0x54E2D1FF},  //
    {"rtex", 0x7808EA10}, //
    {"rvt", 0x039D71F2},  //
    {"sbc", 0x51FC779F},  //
    {"sce", 0x65B275E5},  //
    {"scn", 0x4B768796},  //
    {"scs", 0x0ECD7DF4},  //
    {"sdl", 0x4C0DB839},  //
    {"sds", 0x0315E81F},  //
    {"sep", 0x19F6EFCE},  //
    {"sgt", 0x2F4E7041},  //
    {"shd", 0x0A4280D9},  //
    {"smh", 0x15155F8A},  //
    {"smx", 0x30FC745F},  //
    {"spc", 0x7E33A16C},  //
    {"spj", 0x31EDC625},  //
    {"spkg", 0x02358E1A}, //
    {"spl", 0x6FE1EA15},  //
    {"spr", 0x1EB3767C},  //
    {"sps", 0x7BEC319A},  //
    {"srd", 0x2D12E086},  //
    {"srh", 0x2C4666D1},  //
    {"srq", 0x1BCC4966},  //
    {"ssc", 0x49B5A885},  //
    {"sso", 0x58819BC8},  //
    {"sst", 0x6A9197ED},  //
    {"sstr", 0x3B764DD4}, //
    {"stex", 0x4323D83A}, //
    {"stp", 0x671F21DA},  //
    {"stq", 0x167DBBFF},  //
    {"swm", 0x257D2F7C},  //
    {"szs", 0x601E64CD},  //
    {"tex", 0x241F5DEB},  //
    {"ttb", 0x62A68441},  //
    {"vib", 0x358012E8},  //
    {"vzo", 0x285A13D9},  //
    {"way", 0x5F36B659},  //
    {"zon", 0x1B520B68},  //
};

static const MtExtensionsStorage extRE6Win{
    {"sngw", 0x7D1530C2},    //
    {"rev_win", 0x232E228C}, //
};

static const MtExtensionsStorage extRE6PS3{
    {"at3", 0x7D1530C2},     //
    {"rev_ps3", 0x232E228C}, //
};

static const MtExtensions extRE6{
    &extRE6Common,               //
    Platform::PS3,   &extRE6PS3, //
    Platform::WinPC, &extRE6Win,
};
