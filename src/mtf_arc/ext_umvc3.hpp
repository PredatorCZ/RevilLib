/*  Revil Format Library
    Copyright(C) 2023 Lukas Cone

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

namespace MT_UMVC3 {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"acp", 0x6FBF8058},
    {"anm", 0x5A7E5D8A},
    {"ati", 0x227A8048},
    {"cba", 0x3C6EA504},
    {"ccl", 0x26E7FF},
    {"ccm", 0x28DD8317},
    {"chn", 0x3E363245},
    {"chs", 0x3C41466B},
    {"cli", 0x5B486CCE},
    {"cpi", 0x1DF3E03E},
    {"cpu", 0x5500206},
    {"cre", 0x33CB2BE8},
    {"csa", 0x7D37E18C},
    {"csp", 0x52A8DBF6},
    {"cst", 0x326F732E},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"end", 0x3B199159},
    {"fsd", 0x1DB58B09},
    {"fsm", 0x66B45610},
    {"gem", 0x448BBDD4},
    {"gvm", 0x60B821BB},
    {"icl", 0x7359DE6B},
    {"lmem", 0x9A64A8},
    {"lmt", 0x76820D81},
    {"lrp", 0x357EF6D4},
    {"lsh", 0x141D851F},
    {"mis", 0x361EA2A5},
    {"mod", 0x58A15856},
    {"mrl", 0x2749C8A8},
    {"msd", 0x5B55F5B1},
    {"msn", 0x361EA2A5}, // dupe
    {"pwg", 0x48E25BBB},
    {"scs", 0xECD7DF4}, // dupe
    {"scsr", 0xECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sht", 0x10BE43D4},
    {"slo", 0x2C7171FA},
    {"spkg", 0x2358E1A},
    {"spl", 0x30F6E705},
    {"srq", 0x1BCC4966}, // dupe
    {"srqr", 0x1BCC4966},
    {"stex", 0x4323D83A},
    {"stl", 0x501ACD5C},
    {"stp", 0x642AE284},
    {"stq", 0x167DBBFF}, // dupe
    {"stqr", 0x167DBBFF},
    {"sut", 0x1AB9DAF6},
    {"tex", 0x241F5DEB},
    {"vib", 0x358012E8},
    {"vng", 0x53E132D0},
};

static const MtExtFixupStorage fixups{
    /**/ //
    {0x361EA2A5, "mis"},
    {0xECD7DF4, "scsr"},
    {0x1BCC4966, "srqr"},
    {0x167DBBFF, "stqr"},
};

static const MtExtensionsStorage extWin{
    {"xsew", 0x724DF879},
    {"sbkr", 0x15D782FB},
};

static const MtExtensionsStorage extPS3{
    {"srd", 0x2D12E086},
    {"spc", 0x7E33A16C},
};

static const TitleSupport suppWin{
    ARC_WINPC_GENERIC,
    ModSupport{0xD3, true},
    TexSupport{0x9D, true},
    LmtSupport{67},
};

static const TitleSupport suppPS3{
    ARC_PS3_GENERIC,
    ModSupport{0xD2},
    TexSupport{0x99},
    LmtSupport{67},
};

static const TitleSupports supp{
    Platform::Win32, suppWin, //
    Platform::PS3, suppPS3,   //
};
} // namespace MT_UMVC3

static const MtExtensions extUMVC3{
    MT_UMVC3::extCommon, MT_UMVC3::fixups, //
    MT_UMVC3::supp,                        //
    Platform::PS3,       MT_UMVC3::extPS3, //
    Platform::Win32,     MT_UMVC3::extWin,
};
