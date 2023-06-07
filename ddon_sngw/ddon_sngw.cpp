/*  DDONSngw
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

#include "datas/app_context.hpp"
#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/reflector.hpp"
#include "datas/vectors_simd.hpp"
#include "project.h"
#include <vector>

std::string_view filters[]{
    ".sngw$",
};

static struct DDONSngw : ReflectorBase<DDONSngw> {
  bool encrypt = false;
} settings;

REFLECT(CLASS(DDONSngw),
        MEMBER(encrypt, "e",
               ReflDesc{"Switch between encrypt or decrypt only."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = DDONSngw_DESC " v" DDONSngw_VERSION ", " DDONSngw_COPYRIGHT
                            "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

constexpr uint32 SNGWID = CompileFourCC("OggS");
const UIVector4A16 KEY = 0x64958637;
const UIVector4A16 LMASK = 0x0f0f0f0f;
const UIVector4A16 RMASK = ~LMASK;

UIVector4A16 Decrypt(UIVector4A16 input) {
  const auto ldata = _mm_slli_epi64(_mm_and_si128(input._data, LMASK._data), 4);
  const auto rdata = _mm_srli_epi64(_mm_and_si128(input._data, RMASK._data), 4);

  return _mm_xor_si128(_mm_or_si128(ldata, rdata), KEY._data);
}

UIVector4A16 Encrypt(UIVector4A16 input) {
  input._data = _mm_xor_si128(input._data, KEY._data);
  const auto ldata = _mm_slli_epi64(_mm_and_si128(input._data, LMASK._data), 4);
  const auto rdata = _mm_srli_epi64(_mm_and_si128(input._data, RMASK._data), 4);

  return _mm_or_si128(ldata, rdata);
}

void AppProcessFile(AppContext *ctx) {
  BinReaderRef rd(ctx->GetStream());
  uint32 id;
  rd.Read(id);
  const size_t fileSize = rd.GetSize();
  const size_t numChunks = fileSize / sizeof(UIVector4A16);
  const size_t numRest = fileSize % sizeof(UIVector4A16);
  const size_t numItems = numChunks + (numRest ? 1 : 0);
  std::vector<UIVector4A16> store;

  if (id == SNGWID && settings.encrypt) {
    rd.ReadContainer(store, numItems);
    memset(reinterpret_cast<void *>(store.data()), 0, sizeof(SNGWID));

    for (size_t i = 0; i < numItems; i++) {
      store[i] = Encrypt(store[i]);
    }

    BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".enc")).str);
    wr.WriteBuffer(reinterpret_cast<const char *>(store.data()), fileSize);
  } else if (!settings.encrypt) {
    UIVector4A16 sample;
    rd.Read(sample);
    rd.Seek(0);
    sample = Decrypt(sample);

    if (sample.X) {
      throw std::runtime_error("Decryption error!");
    }

    rd.ReadContainer(store, numItems);

    for (size_t i = 0; i < numItems; i++) {
      store[i] = Decrypt(store[i]);
    }

    memcpy(reinterpret_cast<void *>(store.data()), &SNGWID, sizeof(SNGWID));
    BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".dec")).str);
    wr.WriteBuffer(reinterpret_cast<const char *>(store.data()), fileSize);
  }
}
