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

namespace MT_RECV {
static const MtExtensionsStorage extCommon{
    /**/ //
    {"arc", 0x73850D05},
    {"atrm", 0x6B0369B1},
    {"cutm", 0x18FF29AB},
    {"ddsp", 0x6505B384},
    {"eftm", 0x7D9D148B},
    {"enem", 0x5FF4BE71},
    {"evcm", 0x348C831D},
    {"evlm", 0x24339E8C},
    {"evtm", 0x375F06DA},
    {"itmm", 0x681835FC},
    {"lgtm", 0x6B571E45},
    {"mpac", 0x7F68C6AF},
    {"mrl", 0x2749C8A8},
    {"mtnm", 0x7618CC9A},
    {"objm", 0x6E69693A},
    {"pmb", 0x7050198A},
    {"posm", 0x28D65BFA},
    {"pvp", 0x2ADFA358},
    {"rev_ps3", 0x232E228C},
    {"rmh", 0x130124FA},
    {"rmtn", 0x70078B5},
    {"rutm", 0x51BE0EC},
    {"sdl", 0x4C0DB839},
    {"spc", 0x7E33A16C},
    {"spkg", 0x2358E1A},
    {"srq", 0x1BCC4966},
    {"stq", 0x167DBBFF},
    {"tex", 0x241F5DEB},
};

static const TitleSupport suppPS3{
    ARC_PS3_GENERIC,
    ModSupport{},
    TexSupport{0x98},
    LmtSupport{},
};

static const TitleSupports supp{
    Platform::PS3, suppPS3, //
};
} // namespace MT_RECV
static const MtExtensions extRECV{MT_RECV::extCommon, MT_RECV::supp,
                                  Platform::PS3};
