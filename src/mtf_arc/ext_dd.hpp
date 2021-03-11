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

static const MtExtensionsStorage extDDCommon{
    {"ablparam", 0x59D80140},      //
    {"ahc", 0x5802B3FF},           //
    {"ahs", 0x754B82B4},           //
    {"AIPlActParam", 0x472022DF},  //
    {"ajp", 0x4046F1E1},           //
    {"amr", 0x3D97AD80},           //
    {"aor", 0x4FB35A95},           //
    {"arc", 0x73850D05},           //
    {"ase", 0x07437CCE},           //
    {"atr", 0x1C2B501F},           //
    {"bap", 0x5B334013},           //
    {"bed", 0x36019854},           //
    {"bll", 0x312607A4},           //
    {"ccl", 0x0026E7FF},           //
    {"cit", 0x0E16DFBA},           //
    {"cmc", 0x6DB9FA5F},           //
    {"cnsshake", 0x456B6180},      //
    {"cpl", 0x12C3BFA7},           //
    {"cql", 0x3A947AC1},           //
    {"ctc", 0x535D969F},           //
    {"ddfcv", 0x534BF1A0},         //
    {"dwm", 0x69A5C538},           //
    {"e2d", 0x276DE8B7},           //
    {"ean", 0x4E397417},           //
    {"eap", 0x7AA81CAB},           //
    {"efl", 0x6D5AE854},           //
    {"efs", 0x02833703},           //
    {"epd", 0x5F88B715},           //
    {"epv", 0x12191BA1},           //
    {"equ", 0x2B40AE8F},           //
    {"esp", 0x33B21191},           //
    {"fbik_human", 0x7817FFA5},    //
    {"fca", 0x07B8BCDE},           //
    {"fcp", 0x522F7A3D},           //
    {"fed", 0x5A61A7C8},           //
    {"fpe", 0x4E44FB6D},           //
    {"fsm", 0x66B45610},           //
    {"gce", 0x14428EAE},           //
    {"geo2", 0x5175C242},          //
    {"gfd", 0x2D462600},           //
    {"gii", 0x07F768AF},           //
    {"gmd", 0x242BB29A},           //
    {"gop", 0x2B303957},           //
    {"gpl", 0x2A37242D},           //
    {"gr2", 0x11C35522},           //
    {"gr2s", 0x628DFB41},          //
    {"grw", 0x0437BCF2},           //
    {"gui", 0x22948394},           //
    {"hed", 0x60BB6A09},           //
    {"hpe", 0x0022FA09},           //
    {"chn", 0x3E363245},           //
    {"ik", 0x5A7FEA62},            //
    {"imx", 0x3FB52996},           //
    {"irp", 0x169B7213},           //
    {"ist", 0x48538FFD},           //
    {"itemlv", 0x4509FA80},        //
    {"itl", 0x157388D3},           //
    {"jex", 0x2282360D},           //
    {"joblvl", 0x2CE309AB},        //
    {"lcm", 0x39C52040},           //
    {"lmt", 0x76820D81},           //
    {"lot", 0x15302EF4},           //
    {"ltg", 0x63B524A7},           //
    {"lvl", 0x354284E7},           //
    {"map", 0x2B0670A5},           //
    {"mia", 0x4B704CC0},           //
    {"mlm", 0x1823137D},           //
    {"mod", 0x58A15856},           //
    {"mrl", 0x2749C8A8},           //
    {"mse", 0x4CA26828},           //
    {"msl", 0x48C0AF2D},           //
    {"mss", 0x133917BA},           //
    {"nav", 0x4EF19843},           //
    {"nmr", 0x15773620},           //
    {"nnl", 0x19054795},           //
    {"ntr", 0x00FDA99B},           //
    {"ocl", 0x199C56C0},           //
    {"olp", 0x069A1911},           //
    {"oml", 0x437662FC},           //
    {"pcf", 0x6EE70EFF},           //
    {"pci", 0x079B5F3E},           //
    {"pcs", 0x7E1C8D43},           //
    {"pjp", 0x12688D38},           //
    {"PlDefendParam", 0x0C4FCAE4}, //
    {"plexp", 0x0086B80F},         //
    {"PlNeckPos", 0x22B2A2A2},     //
    {"plw", 0x6F302481},           //
    {"prp", 0x272B80EA},           //
    {"prt", 0x6D0115ED},           //
    {"qct", 0x3BBA4E33},           //
    {"qif", 0x05A36D08},           //
    {"qlv", 0x64387FF1},           //
    {"qmk", 0x7DA64808},           //
    {"qr", 0x31B81AA5},            //
    {"qsp", 0x3B350990},           //
    {"rbd", 0x2A4F96A8},           //
    {"rdd", 0x52DBDCD6},           //
    {"rdp", 0x32E2B13B},           //
    {"rnp", 0x0A74682F},           //
    {"rpi", 0x63747AA7},           //
    {"rpn", 0x76DE35F6},           //
    {"rst", 0x0737E28B},           //
    {"rtex", 0x7808EA10},          //
    {"rvt", 0x039D71F2},           //
    {"sap", 0x089BEF2C},           //
    {"sbc", 0x51FC779F},           //
    {"sce", 0x65B275E5},           //
    {"scs", 0x0ECD7DF4},           //
    {"sdl", 0x4C0DB839},           //
    {"sds", 0x0315E81F},           //
    {"shl", 0x325AACA5},           //
    {"shp", 0x619D23DF},           //
    {"skl", 0x50F3D713},           //
    {"sky", 0x5EA7A3E9},           //
    {"sms", 0x39A0D1D6},           //
    {"smx", 0x30FC745F},           //
    {"sn2", 0x2052D67E},           //
    {"spc", 0x7E33A16C},           //
    {"spj", 0x31EDC625},           //
    {"spkg", 0x02358E1A},          //
    {"spl", 0x6FE1EA15},           //
    {"spn", 0x02373BA7},           //
    {"spr", 0x1EB3767C},           //
    {"sps", 0x7BEC319A},           //
    {"srd", 0x2D12E086},           //
    {"srq", 0x1BCC4966},           //
    {"ssc", 0x49B5A885},           //
    {"ssq", 0x271D08FE},           //
    {"statusparam", 0x215896C2},   //
    {"stc", 0x3E356F93},           //
    {"stg", 0x7E4152FF},           //
    {"stm", 0x72821C38},           //
    {"stp", 0x671F21DA},           //
    {"stq", 0x167DBBFF},           //
    {"swm", 0x257D2F7C},           //
    {"tex", 0x241F5DEB},           //
    {"tmd", 0x04B4BE62},           //
    {"tmn", 0x0D06BE6B},           //
    {"vib", 0x358012E8},           //
    {"way", 0x5F36B659},           //
    {"wep", 0x6186627D},           //
    {"wfp", 0x0525AEE2},           //
    {"zon", 0x1B520B68},           //
};

static const MtExtensionsStorage extDDWin{
    {"rev_win", 0x232E228C}, //
};

static const MtExtensionsStorage extDDPS3{
    {"rev_ps3", 0x232E228C}, //
};

static const MtExtensions extDD{&extDDCommon, &extDDPS3, &extDDPS3};
