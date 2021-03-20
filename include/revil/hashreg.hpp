/*  Revil Format Library
    Copyright(C) 2020-2021 Lukas Cone

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
#include "datas/string_view.hpp"
#include "datas/supercore.hpp"
#include "platform.hpp"
#include "settings.hpp"

namespace revil {
es::string_view RE_EXTERN GetExtension(uint32 hash,
                                       Platform platform = Platform::WinPC);
es::string_view RE_EXTERN GetClassName(uint32 hash,
                                       Platform platform = Platform::WinPC);
uint32 RE_EXTERN GetHash(es::string_view extension, es::string_view title,
                         Platform platform = Platform::WinPC);
using TitleCallback = void (*)(es::string_view);
void RE_EXTERN GetTitles(TitleCallback cb);
}; // namespace revil
