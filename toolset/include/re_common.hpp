/*  Revil Toolset common stuff
    Copyright(C) 2021-2022 Lukas Cone

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
#include "datas/app_context.hpp"
#include "datas/reflector.hpp"
#include "revil/platform.hpp"

using namespace revil;

REFLECT(ENUMERATION(Platform), ENUM_MEMBER(Auto), ENUM_MEMBER(Win32),
        ENUM_MEMBER(PS3), ENUM_MEMBER(X360), ENUM_MEMBER(N3DS),
        ENUM_MEMBER(CAFE), ENUM_MEMBER(NSW), ENUM_MEMBER(PS4),
        ENUM_MEMBER(Android), ENUM_MEMBER(IOS), ENUM_MEMBER(Win64));
