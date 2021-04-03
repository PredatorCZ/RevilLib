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

namespace MT_SB3 {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"arc", 0x73850D05},
    {"asc", 0x5E0EF076},
    {"atk", 0x342366F0},
    {"base", 0x1B58D96E},
    {"bes2", 0x714EC77C}, // dupe
    {"bmse", 0x32EBC8D5},
    {"bsel", 0x1D3A0E3C},
    {"bsh", 0x23DBF8CE},
    {"bssq", 0x50D9CAAF},
    {"bts", 0x2BE3A5BA},
    {"clt", 0x6F4D08},
    {"col", 0x5B9071CF},
    {"dmt", 0x4A1D2F4C},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"eco", 0x76042FD2},
    {"efl", 0x6D5AE854},
    {"efs", 0x2833703},
    {"ein", 0x3D2E1661},
    {"emd", 0xA0E48D4},
    {"emg", 0x3CD26CFD},
    {"ems", 0x62ED419A},
    {"epm", 0x75E854B1},
    {"epr", 0x3934A1BA},
    {"equ", 0x2B40AE8F},
    {"etp", 0x1ACCC2DD},
    {"evs", 0x714EC77C},
    {"fcl", 0x6005E229},
    {"fif", 0xC103FAA},
    {"fnt", 0x1D609FFB},
    {"fpd", 0x4115DFF1},
    {"gway", 0x25FA21CB},
    {"idx", 0x3B5A0DA5},
    {"lcla", 0x340AD7F6},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"lsp", 0x60DD1B16},
    {"ltsa", 0x255926EF},
    {"mef", 0x52CCE4E},
    {"mif", 0x2EA515BF},
    {"mod", 0x58A15856},
    {"mpd", 0x140F4D20},
    {"msg", 0x10C460E6},
    {"nam", 0x6450A37A},
    {"ofst", 0x404BA9F4},
    {"otm", 0x398BD2C5},
    {"pla", 0x42EA212F},
    {"plt", 0x619CF7E7},
    {"ppc", 0x3CFBE7D5},
    {"ppm", 0xB52347D},
    {"pre", 0xD49BAAE},
    {"qif", 0x7DC513BA},
    {"qni", 0x34338E9A},
    {"rdt", 0x19DB4587},
    {"rev_ps3", 0x232E228C},
    {"rew", 0x494CE020},
    {"rtd", 0x6812D2EC},
    {"rtex", 0x7808EA10},
    {"sbc", 0x51FC779F},
    {"scn", 0x6246E90D},
    {"scs", 0xECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sds", 0x315E81F},
    {"seg", 0x38F66FC3},
    {"smi", 0x280C9CB7},
    {"smx", 0x30FC745F},
    {"spc", 0x7E33A16C},
    {"spkg", 0x2358E1A},
    {"srd", 0x2D12E086},
    {"srq", 0x1BCC4966},
    {"sst", 0x15D1B6B},
    {"stex", 0x4323D83A},
    {"stq", 0x167DBBFF},
    {"svc", 0x1E1E93BB},
    {"tev", 0x468C2F93},
    {"tex", 0x241F5DEB},
    {"thk", 0x62A68441}, // dupe
    {"tpi", 0x1DA0352D},
    {"ttb", 0x62A68441}, // dupe
    {"tul", 0x5B99F299},
    {"v30", 0x314ACB2A},
    {"vii", 0x4CE8577B},
    {"vip", 0x603E58D5},
    {"vpi", 0x4CD902BC},
    {"wpd", 0x7C067B17},
};

static const MtExtFixupStorage fixups{
    {0x62A68441, "thk"},
    {0x714EC77C, "evs"},
};

static const TitleSupport suppPS3{
    ARC_PS3_GENERIC,
    ModSupport{}, // RMD
    TexSupport{0x97},
    LmtSupport{56},
};

static const TitleSupports supp{Platform::PS3, suppPS3};
} // namespace MT_SB3

static const MtExtensions extSB3{MT_SB3::extCommon, MT_SB3::fixups,
                                 MT_SB3::supp, Platform::PS3};
