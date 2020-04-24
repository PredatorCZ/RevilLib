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
#include "datas/VectorsSimd.hpp"
#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "uni/motion.hpp"
#include "uni/list_vector.hpp"

class AnimEvent;

namespace pugi {
class xml_node;
}

struct LMTConstructorPropertiesBase {
  uint8 ptrSize = 0; // 4, 8
  uint8 version = 0;
};

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
  virtual uint32 GetNumGroups() const = 0;
  virtual uint32 GetGroupTrackCount(uint32 groupID) const = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;

  virtual ~LMTFloatTrack() {}

  static LMTFloatTrack *Create(const LMTConstructorProperties &props);
};

class LMTAnimationEvent {
public:
  virtual uint32 GetVersion() const = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual uint32 GetNumGroups() const = 0;
  virtual uint32 GetGroupEventCount(uint32 groupID) const = 0;

  virtual ~LMTAnimationEvent() {}

  static LMTAnimationEvent *Create(const LMTConstructorProperties &props);
};

class LMTAnimationEventV1 : public LMTAnimationEvent {
public:
  typedef std::vector<short> EventCollection;

  uint32 GetVersion() const override;

  virtual EventCollection GetEvents(uint32 groupID, uint32 eventID) const = 0;
  virtual int32 GetEventFrame(uint32 groupID, uint32 eventID) const = 0;
};

class LMTAnimationEventV2 : public LMTAnimationEvent {
public:
  uint32 GetVersion() const override;

  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual uint32 GetGroupHash(uint32 groupID) const = 0;
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
  virtual uint32 NumFrames() const = 0;
  virtual bool IsCubic() const = 0;
  virtual void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                           uint32 frame) const = 0;
  virtual void Evaluate(Vector4A16 &out, uint32 frame) const = 0;
  virtual int32 GetFrame(uint32 frame) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual uint32 Stride() const = 0;
  virtual uint32 BoneType() const = 0;
  
  MotionTrack::TrackType_e TrackType() const override;
  void GetValue(uni::RTSValue &output, float time) const override;
  void GetValue(esMatrix44 &output, float time) const override;
  void GetValue(float &output, float time) const override;

  virtual ~LMTTrack() {}

  static LMTTrack *Create(const LMTConstructorProperties &props);
};

class LMTAnimation : public uni::Motion {
protected:
  LMTConstructorPropertiesBase props;

public:
  LMTAnimation() : props{} {}

  virtual uint32 GetVersion() const = 0;
  virtual uint32 NumFrames() const = 0;
  virtual int32 LoopFrame() const = 0;

  virtual int ToXML(pugi::xml_node &node, bool standAlone = false) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual int Save(BinWritterRef wr, bool standAlone = true) const = 0;

  virtual ~LMTAnimation() {}

  virtual void Sanitize() const = 0;
  int Save(const char *fileName, bool supressErrors = false) const;

  static LMTAnimation *Create(const LMTConstructorProperties &props);
  static bool SupportedVersion(uint16 version);

  bool operator==(const LMTConstructorPropertiesBase &input) {
    return props.ptrSize == input.ptrSize && props.version == input.version;
  }

  bool operator!=(const LMTConstructorPropertiesBase &input) {
    return !operator==(input);
  }
};

class LMT : public uni::VectorList<uni::Motion, LMTAnimation> {
  static constexpr uint32 ID = CompileFourCC("LMT\0");
  static constexpr uint32 ID_R = CompileFourCC("\0TML");

public:

  enum V {
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

  enum Architecture { Xundefined, X86, X64 };

  enum ExportSettings {
    ExportSetting_FullXML,
    ExportSetting_FullXMLLinkedMotions,
    ExportSetting_BinaryMotions
  };

  uint8 Version() const { return props.version; }
  void Version(V version, Architecture arch);

  Architecture GetArchitecture() const {
    return (props.ptrSize == 8) ? X64 : X86;
  }

  LMTAnimation *AppendAnimation();
  void AppendAnimation(LMTAnimation *ani);
  void InsertAnimation(LMTAnimation *ani, uint32 at, bool replace = false);

  LMTAnimation *CreateAnimation() const;

  int Load(BinReaderRef rd);
  int Load(const char *fileName, bool supressErrors = false);
  int Save(BinWritterRef wr) const;
  int Save(const char *fileName, bool swapEndian = false,
           bool supressErrors = false) const;

  int ToXML(pugi::xml_node &node, const char *fileName,
            ExportSettings settings);
  int ToXML(const char *fileName, ExportSettings settings);

  int FromXML(pugi::xml_node &node, const char *fileName,
              Architecture forceArchitecture = Xundefined);
  int FromXML(const char *fileName,
              Architecture forceArchitecture = Xundefined);

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