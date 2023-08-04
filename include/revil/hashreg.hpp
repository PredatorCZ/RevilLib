/*  Revil Format Library
    Copyright(C) 2020-2023 Lukas Cone

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
#include "spike/util/supercore.hpp"
#include "platform.hpp"
#include "settings.hpp"
#include <functional>
#include <string_view>

namespace revil {
using Platforms = std::vector<Platform>;
Platforms RE_EXTERN GetPlatformSupport(std::string_view title);
const TitleSupport RE_EXTERN *GetTitleSupport(std::string_view title,
                                              Platform platform);
std::string_view RE_EXTERN GetExtension(uint32 hash, std::string_view title = {},
                                       Platform platform = Platform::Win32);
std::string_view RE_EXTERN GetClassName(uint32 hash,
                                       Platform platform = Platform::Win32);
uint32 RE_EXTERN GetHash(std::string_view extension, std::string_view title,
                         Platform platform = Platform::Win32);
using TitleCallback = std::function<void(std::string_view)>;
void RE_EXTERN GetTitles(TitleCallback cb);

uint32 RE_EXTERN MTHashV1(std::string_view text);
uint32 RE_EXTERN MTHashV2(std::string_view text);
}; // namespace revil
