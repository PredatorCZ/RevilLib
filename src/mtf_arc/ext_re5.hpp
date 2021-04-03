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

namespace MT_RE5 {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"adh", 0x1EFB1B67},
    {"aef", 0x557ECC08},
    {"ahc", 0x5802B3FF},
    {"ahs", 0x754B82B4},
    {"ase", 0x07437CCE},
    {"bbm", 0x5898749C},
    {"bfx", 0x50F9DB3E},
    {"ccl", 0x0026E7FF},
    {"cdf", 0x2DC54131},
    {"cef", 0x758B2EB7},
    {"cmi", 0x4D894D5D},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"efs", 0x02833703},
    {"egv", 0x46810940},
    {"eng", 0x538120DE},
    {"equ", 0x2B40AE8F},
    {"fsm", 0x66B45610},
    {"glp", 0x1ED12F1B},
    {"hit", 0x0253F147},
    {"hkx", 0x36E29465},
    {"hri", 0x4F16B7AB},
    {"chn", 0x3E363245},
    {"idm", 0x2447D742},
    {"jex", 0x2282360D},
    {"lcm", 0x39C52040},
    {"lku", 0x266E8A91},
    {"llk", 0x19A59A91},
    {"lmt", 0x76820D81},
    {"lom", 0x017A550D},
    {"los", 0x176C3F95},
    {"lot", 0x15302EF4},
    {"lsp", 0x60DD1B16},
    {"mod", 0x58A15856},
    {"mse", 0x4CA26828},
    {"msg", 0x10C460E6},
    {"mtg", 0x4E2FEF36},
    {"nav", 0x4EF19843},
    {"nck", 0x1BA81D3C},
    {"oba", 0x0DADAB62},
    {"pos", 0x585831AA},
    {"pth", 0x30ED4060},
    {"ptl", 0x430B4FF4},
    {"rev_win", 0x232E228C},
    {"rtex", 0x7808EA10},
    {"rvt", 0x039D71F2},
    {"sbc", 0x51FC779F},
    {"sce", 0x65B275E5},
    {"scs", 0x0ECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sds", 0x0315E81F},
    {"seg", 0x38F66FC3},
    {"shp", 0x5204D557},
    {"shw", 0x60524FBB},
    {"smh", 0x2C4666D1},
    {"sngw", 0x7D1530C2},
    {"spc", 0x7E33A16C},
    {"srd", 0x2D12E086},
    {"srh", 0x2C4666D1}, // dupe
    {"srq", 0x1BCC4966},
    {"stp", 0x671F21DA},
    {"stq", 0x167DBBFF},
    {"tex", 0x241F5DEB},
    {"vib", 0x358012E8},
    {"way", 0x5F36B659},
};

static const MtExtFixupStorage fixups{
    {0x2C4666D1, "smh"},
};

static const TitleSupport suppWin{
    ARC_WINPC_GENERIC,
    ModSupport{0x19C},
    TexSupport{0x70},
    LmtSupport{51},
};

static const TitleSupports supp{
    Platform::WinPC, suppWin, //
};
} // namespace MT_RE5
static const MtExtensions extRE5{MT_RE5::extCommon, MT_RE5::fixups,
                                 MT_RE5::supp, Platform::WinPC};
