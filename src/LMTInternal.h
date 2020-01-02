/*      Revil Format Library
        Copyright(C) 2017-2019 Lukas Cone

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
#include "datas/endian.hpp"
#include "datas/reflector.hpp"
#include <vector>
#include <memory>
#include "datas/deleter_hybrid.hpp"
#include "datas/allocator_hybrid.hpp"
#include "LMT.h"

#include "datas/VectorsSimd.hpp"

class LMTFixupStorage;

static const char *idents[] = {
    "", "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t",
};

template <class C> union PointerX64 {
  uint64 fullPtr;
  C *ptr;
  int offset;
  esIntPtr varPtr;

  static const esIntPtr mask = ~static_cast<esIntPtr>(1);

  PointerX64() : fullPtr(0) {}
  ES_FORCEINLINE C *GetData(char *) {
    return reinterpret_cast<C *>(varPtr & mask);
  }
  ES_FORCEINLINE void Fixup(char *masterBuffer, bool &swapEndian) {
    if (swapEndian && !offset)
      FByteswapper(fullPtr);

    if (offset & 1) {
      swapEndian = false;
      return;
    }

    if (offset)
      ptr = reinterpret_cast<C *>(masterBuffer + offset);

    offset |= 1;
  }
};

template <class C> union PointerX86 {
  uint varPtr;

  static const uint mask = ~1U;

  PointerX86() : varPtr(0) {}

  template <bool enbabled = ES_X64>
  ES_FORCEINLINE typename std::enable_if<enbabled, C *>::type
  GetData(char *masterBuffer) {
    uint tempPtr = varPtr & mask;

    if (!tempPtr)
      return nullptr;

    return reinterpret_cast<C *>(masterBuffer + tempPtr);
  }

  template <bool enbabled = ES_X64>
  ES_FORCEINLINE typename std::enable_if<!enbabled, C *>::type GetData(char *) {
    return reinterpret_cast<C *>(varPtr & mask);
  }

  ES_FORCEINLINE void Fixup(char *masterBuffer, bool &swapEndian) {
    if (swapEndian && !(varPtr & 0x01000001))
      FByteswapper(varPtr);

    if (varPtr & 1) {
      swapEndian = false;
      return;
    }

    if (ES_X64 || !varPtr) {
      varPtr |= 1;
      return;
    }

    varPtr += reinterpret_cast<uint &>(masterBuffer);
    varPtr |= 1;
  }
};

struct TrackMinMax {
  DECLARE_REFLECTOR;
  Vector4A16 min;
  Vector4A16 max;
};

enum class TrackTypesShared {
  None,
  SingleVector3,
  HermiteVector3,
  StepRotationQuat3,
  SphericalRotation,
  LinearVector3,
  BiLinearVector3_16bit,
  BiLinearVector3_8bit,
  LinearRotationQuat4_14bit,
  BiLinearRotationQuat4_7bit,
  BiLinearRotationQuatXW_14bit,
  BiLinearRotationQuatYW_14bit,
  BiLinearRotationQuatZW_14bit,
  BiLinearRotationQuat4_11bit,
  BiLinearRotationQuat4_9bit
};

struct LMTTrackController {
  virtual int NumFrames() const = 0;
  virtual bool IsCubic() const = 0;
  virtual void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                           int frame) const = 0;
  virtual void Evaluate(Vector4A16 &out, int frame) const = 0;
  virtual void Interpolate(Vector4A16 &out, int frame, float delta,
                           TrackMinMax *bounds = nullptr) const = 0;
  virtual short GetFrame(int frame) const = 0;
  virtual void NumFrames(int numItems) = 0;
  virtual void ToString(std::string &strBuf, int numIdents) const = 0;

  virtual void FromString(std::string &input) = 0;
  virtual void Assign(char *ptr, int size) = 0;
  virtual void SwapEndian() = 0;
  virtual void Devaluate(Vector4A16 in, int frame) = 0;
  virtual void Save(BinWritter *wr) const = 0;

  virtual ~LMTTrackController() {}

  static LMTTrackController *CreateCodec(int type, int subVersion);
  static LMTTrackController *CreateCodec(TrackTypesShared type) { return CreateCodec(static_cast<int>(type), -1); }
  static float GetTrackMaxFrac(TrackTypesShared type);
};