/*  Revil Format Library
    Copyright(C) 2017-2020 Lukas Cone

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

#include "datas/master_printer.hpp"
#include "motion_78.hpp"
#include <unordered_map>

struct RETrackController_internal : RETrackController {
  enum FrameType { FrameType_short = 4, FrameType_char = 2 };
  struct {
    Vector4A16 min;
    Vector4A16 max;
  } minMaxBounds;
  FrameType frameType;
  uint32 componentID;
  uint8 *frames;
  uint32 numFrames;

  template <class C> void Assign_(C *data) {
    frameType = static_cast<FrameType>((data->flags >> 20) & 0xf);

    if (data->minMaxBounds) {
      minMaxBounds.max = Vector4A16(data->minMaxBounds->max);
      minMaxBounds.min = Vector4A16(data->minMaxBounds->min);
    }

    componentID = ((data->flags >> 12) & 0xf) - 1;
    frames = data->frames.operator->();
    numFrames = data->numFrames;
    Assign(data->controlPoints.operator->());
  }

  virtual void Assign(char *data) = 0;
  void Assign(RETrackCurve43 *iCurve) { Assign_(iCurve); }
  void Assign(RETrackCurve65 *iCurve) { Assign_(iCurve); }
  void Assign(RETrackCurve78 *iCurve) { Assign_(iCurve); }

  uint16 GetFrame(uint32 id) const {
    if (frameType == FrameType_short) {
      return reinterpret_cast<uint16 *>(frames)[id];
    } else {
      return frames[id];
    }
  }
};

struct LinearVector3Controller : RETrackController_internal {
  static constexpr uint32 ID = 0xF2;
  using Alloc_Type = es::allocator_hybrid<Vector>;
  using Storage_Type = std::vector<Vector, Alloc_Type>;
  Storage_Type dataStorage;

  void Assign(char *data) override {
    Vector *start = reinterpret_cast<Vector *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    out = dataStorage[id];
  }
};

struct BiLinearVector3_5bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x200F2;
  using Alloc_Type = es::allocator_hybrid<uint16>;
  using Storage_Type = std::vector<uint16, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint32 componentMask = 0x1f;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    uint16 *start = reinterpret_cast<uint16 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));

    minMaxBounds.max.Z = minMaxBounds.max.Y;
    minMaxBounds.max.Y = minMaxBounds.max.X;
    minMaxBounds.max.X = minMaxBounds.min.W;
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint16 &retreived = dataStorage[id];
    out = Vector(static_cast<float>(retreived & componentMask),
                 static_cast<float>((retreived >> 5) & componentMask),
                 static_cast<float>((retreived >> 10) & componentMask));
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
  }
};

struct BiLinearVector3_10bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x400F2;
  using Alloc_Type = es::allocator_hybrid<uint32>;
  using Storage_Type = std::vector<uint32, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint32 componentMask = 0x3ff;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    uint32 *start = reinterpret_cast<uint32 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));

    minMaxBounds.max.Z = minMaxBounds.max.Y;
    minMaxBounds.max.Y = minMaxBounds.max.X;
    minMaxBounds.max.X = minMaxBounds.min.W;
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint32 &retreived = dataStorage[id];
    out =
        Vector4A16(static_cast<float>(retreived & componentMask),
                   static_cast<float>((retreived >> 10) & componentMask),
                   static_cast<float>((retreived >> 20) & componentMask), 0.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
  }
};

struct BiLinearVector3_21bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x800F2;
  using Alloc_Type = es::allocator_hybrid<uint64>;
  using Storage_Type = std::vector<uint64, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint64 componentMask = (1 << 21) - 1;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    uint64 *start = reinterpret_cast<uint64 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));

    minMaxBounds.max.Z = minMaxBounds.max.Y;
    minMaxBounds.max.Y = minMaxBounds.max.X;
    minMaxBounds.max.X = minMaxBounds.min.W;
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint64 &retreived = dataStorage[id];
    out =
        Vector4A16(static_cast<float>(retreived & componentMask),
                   static_cast<float>((retreived >> 21) & componentMask),
                   static_cast<float>((retreived >> 42) & componentMask), 0.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
  }
};

struct BiLinearQuat3_13bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x50112;
  struct SType {
    uint8 data[5];
  };
  using Alloc_Type = es::allocator_hybrid<SType>;
  using Storage_Type = std::vector<SType, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint64 componentMask = (1 << 13) - 1;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    SType *start = reinterpret_cast<SType *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint64 retreived =
        (static_cast<uint64>(dataStorage[id].data[0]) << 32) |
        (static_cast<uint64>(dataStorage[id].data[1]) << 24) |
        (static_cast<uint64>(dataStorage[id].data[2]) << 16) |
        (static_cast<uint64>(dataStorage[id].data[3]) << 8) |
        (static_cast<uint64>(dataStorage[id].data[4]) << 0);

    out =
        Vector4A16(static_cast<float>(retreived & componentMask),
                   static_cast<float>((retreived >> 13) & componentMask),
                   static_cast<float>((retreived >> 26) & componentMask), 0.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_16bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x60112;
  using Alloc_Type = es::allocator_hybrid<USVector>;
  using Storage_Type = std::vector<USVector, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint32 componentMask = (1 << 16) - 1;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    USVector *start = reinterpret_cast<USVector *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    out = Vector4A16(dataStorage[id].Convert<float>(), 0.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_18bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x70112;
  struct SType {
    uint8 data[7];
  };
  using Alloc_Type = es::allocator_hybrid<SType>;
  using Storage_Type = std::vector<SType, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint64 componentMask = (1 << 18) - 1;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    SType *start = reinterpret_cast<SType *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint64 retreived =
        (static_cast<uint64>(dataStorage[id].data[0]) << 48) |
        (static_cast<uint64>(dataStorage[id].data[1]) << 40) |
        (static_cast<uint64>(dataStorage[id].data[2]) << 32) |
        (static_cast<uint64>(dataStorage[id].data[3]) << 24) |
        (static_cast<uint64>(dataStorage[id].data[4]) << 16) |
        (static_cast<uint64>(dataStorage[id].data[5]) << 8) |
        (static_cast<uint64>(dataStorage[id].data[6]) << 0);

    out =
        Vector4A16(static_cast<float>(retreived & componentMask),
                   static_cast<float>((retreived >> 18) & componentMask),
                   static_cast<float>((retreived >> 36) & componentMask), 0.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_8bitController : RETrackController_internal {
  static constexpr uint32 ID = 0x30112;
  using Alloc_Type = es::allocator_hybrid<UCVector>;
  using Storage_Type = std::vector<UCVector, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint32 componentMask = 0xff;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    UCVector *start = reinterpret_cast<UCVector *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    out = Vector4A16(dataStorage[id].Convert<float>(), 1.0f);
    out = ((out * componentMultiplier) * minMaxBounds.min) + minMaxBounds.max;
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct LinearQuat3Controller : LinearVector3Controller {
  static constexpr uint32 ID1 = 0xB0112;
  static constexpr uint32 ID2 = 0xC0112;

  void Evaluate(uint32 id, Vector4A16 &out) const {
    LinearVector3Controller::Evaluate(id, out);
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_5bitController : BiLinearVector3_5bitController {
  static constexpr uint32 ID = 0x20112;

  void Assign(char *data) override {
    uint16 *start = reinterpret_cast<uint16 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    BiLinearVector3_5bitController::Evaluate(id, out);
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_10bitController : BiLinearVector3_10bitController {
  static constexpr uint32 ID1 = 0x30112;
  static constexpr uint32 ID2 = 0x40112;

  void Assign(char *data) override {
    uint32 *start = reinterpret_cast<uint32 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    BiLinearVector3_10bitController::Evaluate(id, out);
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct BiLinearQuat3_21bitController : BiLinearVector3_21bitController {
  static constexpr uint32 ID1 = 0x70112;
  static constexpr uint32 ID2 = 0x80112;

  void Assign(char *data) override {
    uint64 *start = reinterpret_cast<uint64 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    BiLinearVector3_21bitController::Evaluate(id, out);
    out *= Vector4A16(1.f, 1.f, 1.f, 0.0f);
    out.QComputeElement();
  }
};

struct LinearSCVector3Controller : RETrackController_internal {
  static constexpr uint32 ID1 = 0x310F2;
  static constexpr uint32 ID2 = 0x320F2;
  static constexpr uint32 ID3 = 0x330F2;
  static constexpr uint32 ID4 = 0x340F2;
  static constexpr uint32 ID5 = 0x410F2;
  static constexpr uint32 ID6 = 0x420F2;
  static constexpr uint32 ID7 = 0x430F2;
  static constexpr uint32 ID8 = 0x440F2;

  using Alloc_Type = es::allocator_hybrid<float>;
  using Storage_Type = std::vector<Alloc_Type::value_type, Alloc_Type>;
  Storage_Type dataStorage;

  void Assign(char *data) override {
    float *start = reinterpret_cast<float *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const float &retreived = dataStorage[id];
    if (componentID == 3) {
      out = Vector4A16(retreived);
    } else {
      out = minMaxBounds.min;
      out[componentID] = retreived;
    }
  }
};

struct BiLinearSCVector3_16bitController : RETrackController_internal {
  static constexpr uint32 ID1 = 0x210F2;
  static constexpr uint32 ID2 = 0x220F2;
  static constexpr uint32 ID3 = 0x230F2;
  static constexpr uint32 ID4 = 0x240F2;

  using Alloc_Type = es::allocator_hybrid<uint16>;
  using Storage_Type = std::vector<Alloc_Type::value_type, Alloc_Type>;
  Storage_Type dataStorage;

  static constexpr uint32 componentMask = (1 << 16) - 1;
  static constexpr float componentMultiplier =
      1.0f / static_cast<float>(componentMask);

  void Assign(char *data) override {
    uint16 *start = reinterpret_cast<uint16 *>(data);
    dataStorage = Storage_Type(start, start + numFrames, Alloc_Type(start));
  }

  void Evaluate(uint32 id, Vector4A16 &out) const override {
    const uint16 &retreived = dataStorage[id];
    float decompVal = minMaxBounds.min[(componentID % 3) + 1] +
                      (minMaxBounds.min[0] *
                       (static_cast<float>(retreived) * componentMultiplier));
    if (componentID == 3) {
      out = Vector4A16(decompVal);
    } else {
      out.X = minMaxBounds.min.Y;
      out.Y = minMaxBounds.min.Z;
      out.Z = minMaxBounds.min.W;
      out[componentID] = decompVal;
    }
  }
};

struct BiLinearSCQuat3Controller : LinearSCVector3Controller {
  static constexpr uint32 ID1 = 0x31112;
  static constexpr uint32 ID2 = 0x32112;
  static constexpr uint32 ID3 = 0x33112;
  static constexpr uint32 ID4 = 0x41112;
  static constexpr uint32 ID5 = 0x42112;
  static constexpr uint32 ID6 = 0x43112;

  void Evaluate(uint32 id, Vector4A16 &out) const {
    out[componentID] = dataStorage[id];
    out.QComputeElement();
  }
};

struct BiLinearSCQuat3_16bitController : BiLinearSCVector3_16bitController {
  static constexpr uint32 ID1 = 0x21112;
  static constexpr uint32 ID2 = 0x22112;
  static constexpr uint32 ID3 = 0x23112;

  void Evaluate(uint32 id, Vector4A16 &out) const {
    const uint16 &retreived = dataStorage[id];
    out[componentID] = minMaxBounds.min[1] +
                       (minMaxBounds.min[0] *
                        (static_cast<float>(retreived) * componentMultiplier));
    out.QComputeElement();
  }
};

struct BiLinearSCQuat3_16bitController_old : BiLinearSCQuat3_16bitController {
  static constexpr uint32 ID1 = 0x21112;
  static constexpr uint32 ID2 = 0x22112;
  static constexpr uint32 ID3 = 0x23112;

  void Evaluate(uint32 id, Vector4A16 &out) const {
    const uint16 &retreived = dataStorage[id];
    out[componentID] = minMaxBounds.max[componentID] +
                       (minMaxBounds.min[componentID] *
                        (static_cast<float>(retreived) * componentMultiplier));
    out.QComputeElement();
  }
};

using ptr_type_ = std::unique_ptr<RETrackController_internal>;

template <class C> ptr_type_ controlDummy() { return std::make_unique<C>(); }

template <class C, size_t id> struct id_ {
  constexpr static auto get() { return C::ID; }
};

template <class C> struct id_<C, 1> {
  constexpr static auto get() { return C::ID1; }
};

template <class C> struct id_<C, 2> {
  constexpr static auto get() { return C::ID2; }
};

template <class C> struct id_<C, 3> {
  constexpr static auto get() { return C::ID3; }
};

template <class C> struct id_<C, 4> {
  constexpr static auto get() { return C::ID4; }
};

template <class C> struct id_<C, 5> {
  constexpr static auto get() { return C::ID5; }
};

template <class C> struct id_<C, 6> {
  constexpr static auto get() { return C::ID6; }
};

template <class C> struct id_<C, 7> {
  constexpr static auto get() { return C::ID7; }
};

template <class C> struct id_<C, 8> {
  constexpr static auto get() { return C::ID8; }
};

template <class C, size_t id = 0> auto make() {
  return std::make_pair(id_<C, id>::get(), controlDummy<C>);
}

static const std::unordered_map<uint32, ptr_type_ (*)()> curveControllers = {
    make<LinearVector3Controller>(),
    make<LinearQuat3Controller, 1>(),
    make<BiLinearSCQuat3Controller, 1>(),
    make<BiLinearSCQuat3Controller, 2>(),
    make<BiLinearSCQuat3Controller, 3>(),
    make<BiLinearSCQuat3_16bitController, 1>(),
    make<BiLinearSCQuat3_16bitController, 2>(),
    make<BiLinearSCQuat3_16bitController, 3>(),
    make<LinearSCVector3Controller, 1>(),
    make<LinearSCVector3Controller, 2>(),
    make<LinearSCVector3Controller, 3>(),
    make<LinearSCVector3Controller, 4>(),
    make<BiLinearSCVector3_16bitController, 1>(),
    make<BiLinearSCVector3_16bitController, 2>(),
    make<BiLinearSCVector3_16bitController, 3>(),
    make<BiLinearSCVector3_16bitController, 4>(),
    make<BiLinearQuat3_10bitController, 1>(),
    make<BiLinearQuat3_21bitController, 1>(),
};

RETrackController::Ptr RETrackCurve65::GetController() {
  const uint32 type = flags & 0xff0fffff;

  if (curveControllers.count(type)) {
    auto iCon = curveControllers.at(type)();
    iCon->Assign(this);
    return std::move(iCon);
  } else {
    printerror("[RETrackController]: Unhandled curve compression: " << std::hex
                                                                    << type);
  }

  return {};
}

static const std::unordered_map<uint32, ptr_type_ (*)()> curveControllers78 = {
    make<LinearVector3Controller>(),
    make<LinearQuat3Controller, 2>(),

    make<BiLinearSCQuat3Controller, 4>(),
    make<BiLinearSCQuat3Controller, 5>(),
    make<BiLinearSCQuat3Controller, 6>(),

    make<BiLinearSCQuat3_16bitController, 1>(),
    make<BiLinearSCQuat3_16bitController, 2>(),
    make<BiLinearSCQuat3_16bitController, 3>(),

    make<LinearSCVector3Controller, 5>(),
    make<LinearSCVector3Controller, 6>(),
    make<LinearSCVector3Controller, 7>(),
    make<LinearSCVector3Controller, 8>(),

    make<BiLinearSCVector3_16bitController, 1>(),
    make<BiLinearSCVector3_16bitController, 2>(),
    make<BiLinearSCVector3_16bitController, 3>(),
    make<BiLinearSCVector3_16bitController, 4>(),

    make<BiLinearVector3_5bitController>(),
    make<BiLinearVector3_10bitController>(),
    make<BiLinearVector3_21bitController>(),

    make<BiLinearQuat3_5bitController>(),
    make<BiLinearQuat3_8bitController>(),
    make<BiLinearQuat3_10bitController, 2>(),
    make<BiLinearQuat3_13bitController>(),
    make<BiLinearQuat3_16bitController>(),
    make<BiLinearQuat3_18bitController>(),
    make<BiLinearQuat3_21bitController, 2>(),
};

RETrackController::Ptr RETrackCurve78::GetController() {
  const uint32 type = flags & 0xff0fffff;

  if (curveControllers78.count(type)) {
    auto iCon = curveControllers78.at(type)();
    iCon->Assign(this);
    return std::move(iCon);
  } else {
    printerror("[RETrackController]: Unhandled curve compression: " << std::hex
                                                                    << type);
  }

  return {};
}

static const std::unordered_map<uint32, ptr_type_ (*)()> curveControllers43 = {
    make<LinearVector3Controller>(),
    make<LinearQuat3Controller, 1>(),

    make<BiLinearSCQuat3Controller, 1>(),
    make<BiLinearSCQuat3Controller, 2>(),
    make<BiLinearSCQuat3Controller, 3>(),

    make<BiLinearSCQuat3_16bitController_old, 1>(),
    make<BiLinearSCQuat3_16bitController_old, 2>(),
    make<BiLinearSCQuat3_16bitController_old, 3>(),

    make<BiLinearQuat3_10bitController, 1>(),
    make<BiLinearQuat3_21bitController, 1>(),
};

RETrackController::Ptr RETrackCurve43::GetController() {
  const uint32 type = flags & 0xff0fffff;

  if (curveControllers43.count(type)) {
    auto iCon = curveControllers43.at(type)();
    iCon->Assign(this);
    return std::move(iCon);
  } else {
    printerror("[RETrackController]: Unhandled curve compression: " << std::hex
                                                                    << type);
  }

  return {};
}
