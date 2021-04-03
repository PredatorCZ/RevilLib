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

namespace MT_PWAASOJ {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"ccl", 0x0026E7FF},
    {"cli", 0x6D96778B},
    {"ctc", 0x535D969F},
    {"dsc", 0x75967AD6},
    {"e2d", 0x276DE8B7},
    {"ean", 0x4E397417},
    {"efl", 0x6D5AE854},
    {"fng", 0x64A7CFA9},
    {"gfd", 0x2D462600},
    {"gii", 0x07F768AF},
    {"gmd", 0x242BB29A},
    {"gui", 0x22948394},
    {"chr", 0x194BE415},
    {"lcm", 0x39C52040},
    {"lmt", 0x76820D81},
    {"lpm", 0x0C41C74D},
    {"mca", 0x79C47B59},
    {"mhd", 0x2C97686F},
    {"mod", 0x58A15856},
    {"mrl", 0x2749C8A8},
    {"sbc", 0x51FC779F},
    {"sbkr", 0x15D782FB},
    {"sdl", 0x4C0DB839},
    {"srq", 0x1BCC4966}, // dupe
    {"srqr", 0x1BCC4966},
    {"stq", 0x167DBBFF}, // dupe
    {"stqr", 0x167DBBFF},
    {"tex", 0x241F5DEB},
};

static const MtExtFixupStorage fixups{
    {0x167DBBFF, "stqr"},
    {0x1BCC4966, "srqr"},
};

static const TitleSupport supp3DS{
    ARC_N3DS_GENERIC,
    ModSupport{0xE6},
    TexSupport{0xA6},
    LmtSupport{67},
};

static const TitleSupports supp{Platform::N3DS, supp3DS};
} // namespace MT_PWAASOJ

static const MtExtensions extPWAASOJ{MT_PWAASOJ::extCommon, MT_PWAASOJ::fixups,
                                     MT_PWAASOJ::supp, Platform::N3DS};
