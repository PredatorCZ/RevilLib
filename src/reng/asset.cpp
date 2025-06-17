/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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

#include "asset.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader.hpp"
#include <map>

REAsset::~REAsset() = default;
REAsset::REAsset() = default;
REAsset::REAsset(revil::REAsset &&) = default;

void REAsset::Load(const std::string &fileName) {
  BinReader rd(fileName);
  return Load(rd);
}

void REAsset::Load(BinReaderRef rd) {
  rd.Push();
  REAssetBase base;
  rd.Read(base);
  rd.Pop();

  i = REAssetImpl::Create(base);

  i->Load(rd);
}

void REAssetImpl::Load(BinReaderRef rd) {
  const size_t fleSize = rd.GetSize();
  rd.ReadContainer(internalBuffer, fleSize);
  buffer = internalBuffer.data();
  std::vector<void *> ptrStore;
  Fixup(ptrStore);
}

void REAssetImpl::Assign(REAssetBase *data) {
  char *rawData = reinterpret_cast<char *>(data);
  buffer = rawData;
  Build();
}
