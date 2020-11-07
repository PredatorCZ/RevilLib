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

#pragma once
#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/string_view.hpp"
#include "datas/vectors_simd.hpp"
#include "uni/list_vector.hpp"
#include "uni/motion.hpp"

class AnimEvent;

namespace pugi {
class xml_node;
}

enum class LMTExportType : uint8 {
  FullBinary,      // Propper LMT format
  FullXML,         // Single XML format
  LinkedXML,       // XML format with linked XML files per animation
  BinaryLinkedXML, // XML format with linked binary files per animation
};

enum class LMTArchType : uint8 { Auto, X64, X86 };

enum class LMTVersion : uint8 {
  Auto = 0,
  V_22 = 22, // DR
  V_40 = 40, // LP
  V_49 = 49, // DMC4
  V_50 = 50, // LP PS3
  V_51 = 51, // RE5
  V_56 = 56, // LP2
  V_57 = 57, // RE:M 3DS
  V_66 = 66, // DD, DD:DA
  V_67 = 67, // Other, generic MTF v2 format
  V_92 = 92, // MH:W
};

struct LMTExportSettings {
  LMTExportType type = LMTExportType::FullBinary;
  LMTArchType arch = LMTArchType::Auto;
  LMTVersion version = LMTVersion::Auto;
  bool swapEndian = false;
};

struct alignas(2) LMTImportOverrides {
  LMTArchType arch = LMTArchType::Auto;
  LMTVersion version = LMTVersion::Auto;
  constexpr LMTImportOverrides() = default;
  constexpr LMTImportOverrides(LMTArchType a, LMTVersion v)
      : arch(a), version(v) {}

  bool operator<(const LMTImportOverrides &o) const {
    return reinterpret_cast<const uint16 &>(*this) <
           reinterpret_cast<const uint16 &>(o);
  }

  bool operator==(const LMTImportOverrides &o) const {
    return reinterpret_cast<const uint16 &>(*this) ==
           reinterpret_cast<const uint16 &>(o);
  }

  bool operator!=(const LMTImportOverrides &o) const { return !(*this == o); }
};

using LMTConstructorPropertiesBase = LMTImportOverrides;

struct LMTConstructorProperties : LMTConstructorPropertiesBase {
  bool swappedEndian; // optional, assign only
  void *dataStart;
  char *masterBuffer;

  LMTConstructorProperties() : dataStart(nullptr), masterBuffer(nullptr) {}
  LMTConstructorProperties(const LMTConstructorPropertiesBase &base)
      : LMTConstructorProperties() {
    operator=(base);
  }

  void operator=(const LMTConstructorPropertiesBase &input) {
    static_cast<LMTConstructorPropertiesBase &>(*this) = input;
  }
};

class LMTFloatTrack {
public:
  virtual size_t GetNumGroups() const = 0;
  virtual size_t GetGroupTrackCount(size_t groupID) const = 0;
  virtual void Save(pugi::xml_node &node, bool standAlone) const = 0;
  virtual void Load(pugi::xml_node &node) = 0;
  virtual ~LMTFloatTrack() = default;

  static std::unique_ptr<LMTFloatTrack>
  Create(const LMTConstructorProperties &props);
};

class LMTAnimationEvent {
public:
  virtual size_t GetVersion() const = 0;
  virtual void Save(pugi::xml_node &node, bool standAlone) const = 0;
  virtual void Load(pugi::xml_node &node) = 0;
  virtual size_t GetNumGroups() const = 0;
  virtual size_t GetGroupEventCount(size_t groupID) const = 0;
  virtual ~LMTAnimationEvent() = default;

  static std::unique_ptr<LMTAnimationEvent>
  Create(const LMTConstructorProperties &props);
};

class LMTAnimationEventV1 : public LMTAnimationEvent {
public:
  typedef std::vector<short> EventCollection;

  virtual EventCollection GetEvents(size_t groupID, size_t eventID) const = 0;
  virtual int32 GetEventFrame(size_t groupID, size_t eventID) const = 0;
};

class LMTAnimationEventV2 : public LMTAnimationEvent {
public:
  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual uint32 GetGroupHash(size_t groupID) const = 0;
};

class LMTTrack : public uni::MotionTrack {
public:
  enum TrackType_e {
    TrackType_LocalRotation,
    TrackType_LocalPosition,
    TrackType_LocalScale,
    TrackType_AbsoluteRotation,
    TrackType_AbsolutePosition
  };

  virtual TrackType_e GetTrackType() const = 0;
  virtual size_t NumFrames() const = 0;
  virtual bool IsCubic() const = 0;
  virtual void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                           size_t frame) const = 0;
  virtual void Evaluate(Vector4A16 &out, size_t frame) const = 0;
  virtual int32 GetFrame(size_t frame) const = 0;
  virtual void Load(pugi::xml_node &node) = 0;
  virtual void Save(pugi::xml_node &node, bool standAlone) const = 0;
  virtual size_t Stride() const = 0;
  virtual uint32 BoneType() const = 0;
  virtual ~LMTTrack() = default;

  static std::unique_ptr<LMTTrack>
  Create(const LMTConstructorProperties &props);
};

class LMTAnimation : public uni::Motion {
public:
  using Ptr = std::unique_ptr<LMTAnimation>;

protected:
  LMTConstructorPropertiesBase props;

public:
  LMTAnimation() : props{} {}

  virtual size_t GetVersion() const = 0;
  virtual size_t NumFrames() const = 0;
  virtual int32 LoopFrame() const = 0;

  virtual void Load(pugi::xml_node &node) = 0;
  virtual void Save(pugi::xml_node &node, bool standAlone = false) const = 0;
  virtual void Save(BinWritterRef wr, bool standAlone = true) const = 0;
  void Save(const std::string &fileName, bool asXML = false) const;
  virtual ~LMTAnimation() = default;

  static Ptr Create(const LMTConstructorProperties &props);
  static bool SupportedVersion(uint16 version);

  bool operator==(const LMTConstructorPropertiesBase &input) {
    return props.arch == input.arch && props.version == input.version;
  }

  bool operator!=(const LMTConstructorPropertiesBase &input) {
    return !operator==(input);
  }
};

class LMT
    : public uni::PolyVectorList<uni::Motion, LMTAnimation, uni::Element> {
public:
  LMTVersion Version() const { return props.version; }
  void Version(LMTVersion version, LMTArchType arch);
  LMTArchType Architecture() const { return props.arch; }
  auto CreateAnimation() const { return LMTAnimation::Create(props); }

  LMTAnimation *AppendAnimation();
  void AppendAnimation(LMTAnimation *ani);
  void InsertAnimation(LMTAnimation *ani, size_t at, bool replace = false);

  void Load(BinReaderRef rd);
  void Load(const std::string &fileName, LMTImportOverrides overrides = {});
  void Load(pugi::xml_node &node, es::string_view outPath,
            LMTImportOverrides overrides = {});
  void Save(BinWritterRef wr) const;
  void Save(const std::string &fileName, LMTExportSettings settings = {}) const;
  void Save(pugi::xml_node &node, es::string_view outPath,
            LMTExportSettings settings = {}) const;

private:
  std::string masterBuffer;
  LMTConstructorPropertiesBase props;
};

// TODO:
/*
add header sanitizers

low priority:
conversion system << super low
encoder, exporting utility
*/
