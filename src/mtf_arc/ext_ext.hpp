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

namespace MT_EXT {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"acd", 0x2F94F561},
    {"adl", 0x199FDB52},
    {"akp", 0x57FE93BF},
    {"apl", 0x78EDB2F},
    {"apt", 0x72CCDE0E},
    {"arc", 0x73850D05},
    {"aser", 0x7E8E445},
    {"bbp", 0x4E39F8C0},
    {"bbp", 0x7B69C8BF},
    {"bcp", 0x6EEAD597},
    {"bgt", 0x4AC5C529},
    {"bil", 0x3734B23A},
    {"bql", 0x7EC37134},
    {"cia", 0x57265000},
    {"cil", 0x64596447},
    {"cls", 0x7B7469E5},
    {"crd", 0x180BA45F},
    {"csl", 0x5C7FF59D},
    {"dit", 0x6360F04F},
    {"dmp", 0x6FCFA2EC},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"ebp", 0x78E0DEE6},
    {"ecl", 0x70108A0A},
    {"efl", 0x6D5AE854},
    {"efs", 0x2833703},
    {"epv", 0x12191BA1},
    {"equ", 0x2B40AE8F}, // dupe
    {"equr", 0x2B40AE8F},
    {"etp", 0x3A7C0812},
    {"evp", 0x72D5B36A},
    {"exi", 0x6EF9A55},
    {"ezt", 0x6C39D392},
    {"fcp", 0x7AEFA722},
    {"fmu", 0x44C79ADC},
    {"fsm", 0x66B45610},
    {"gal", 0x1E064791},
    {"gbl", 0x3E8C2211},
    {"geo3", 0x2672F2D4},
    {"gfd", 0x2D462600},
    {"gii", 0x7F768AF},
    {"gmd", 0x242BB29A},
    {"gnc", 0x180E149D},
    {"gui", 0x22948394},
    {"h29", 0x7FFD90D7},
    {"hlt", 0x77892F95},
    {"hmp", 0x1F0CBEEB},
    {"hsp", 0x17B10794},
    {"itl", 0x157388D3},
    {"itp", 0x6D32A44B},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"lsl", 0x66BCC342},
    {"mil", 0x6E7DDC8B},
    {"mod", 0x58A15856},
    {"mrl", 0x2749C8A8},
    {"msa", 0x129FE7C4},
    {"mser", 0x13BC0ACE},
    {"msf", 0xCF7FB37},
    {"npm", 0x72B4E538},
    {"npt", 0x7963691C},
    {"rda", 0x5A4D542F},
    {"rev_ps3", 0x232E228C}, // dupe
    {"revr", 0x232E228C},
    {"sbc", 0x51FC779F},
    {"sbkr", 0x15D782FB},
    {"scs", 0xECD7DF4}, // dupe
    {"scsr", 0xECD7DF4},
    {"sdl", 0x4C0DB839},
    {"sds", 0x315E81F}, // dupe
    {"sdsr", 0x315E81F},
    {"shp", 0xF9C5829},
    {"sil", 0x4BBC1267},
    {"sol", 0x6EF2AAD4},
    {"spkg", 0x2358E1A},
    {"srq", 0x1BCC4966}, // dupe
    {"srqr", 0x1BCC4966},
    {"sta", 0x481D0CA},
    {"std", 0x1E7B1EF6},
    {"stq", 0x167DBBFF}, // dupe
    {"stqr", 0x167DBBFF},
    {"tdc", 0x51AEDE1E},
    {"tdm", 0x7DEE9C90},
    {"tex", 0x241F5DEB},
    {"tpm", 0x5E97587C},
    {"tsl", 0x780FEBEC},
    {"vib", 0x358012E8},
    {"vsm", 0x77967F14},
    {"vsp", 0x319DE76E},
    {"way", 0x5F36B659},
    {"wdl", 0x5F3DFA3C},
    {"wil", 0x7174F374},
    {"wmi", 0x22E107B2},
    {"wpp", 0x69F530CE},
    {"wul", 0x72486806},
    {"zon", 0x1B520B68},
};

static const MtExtFixupStorage fixups{
    /**/ //
    {0x0315E81F, "sdsr"},
    {0x0ECD7DF4, "scsr"},
    {0x167DBBFF, "stqr"},
    {0x1BCC4966, "srqr"},
    {0x232E228C, "revr"},
    {0x2B40AE8F, "equr"},
};

static const TitleSupport suppPS3{
    ARC_PS3_GENERIC,
    ModSupport{0xD2},
    TexSupport{0x9A},
    LmtSupport{67},
};

static const TitleSupports supp{Platform::PS3, suppPS3};
} // namespace MT_EXT

static const MtExtensions extEXT{MT_EXT::extCommon, MT_EXT::supp,
                                 MT_EXT::fixups, Platform::PS3};
