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
#include "datas/supercore.hpp"
#include <vector>

class BinReader;
class BinWritter;
class AnimEvent;
class V4SimdFltType;
template <class C> class _t_Vector4;
typedef _t_Vector4<V4SimdFltType> Vector4A16;

namespace pugi {
class xml_node;
}

struct LMTConstructorPropertiesBase {
  char ptrSize; // 4, 8
  char version;
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
  virtual int GetNumGroups() const = 0;
  virtual int GetGroupTrackCount(int groupID) const = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;

  virtual ~LMTFloatTrack() {}

  static LMTFloatTrack *Create(const LMTConstructorProperties &props);
};

class LMTAnimationEvent {
public:
  virtual int GetVersion() const = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual int GetNumGroups() const = 0;
  virtual int GetGroupEventCount(int groupID) const = 0;

  virtual ~LMTAnimationEvent() {}

  static LMTAnimationEvent *Create(const LMTConstructorProperties &props);
};

class LMTAnimationEventV1 : public LMTAnimationEvent {
public:
  typedef std::vector<short> EventCollection;

  int GetVersion() const override;

  virtual EventCollection GetEvents(int groupID, int eventID) const = 0;
  virtual int GetEventFrame(int groupID, int eventID) const = 0;
  
};

class LMTAnimationEventV2 : public LMTAnimationEvent {
public:
  int GetVersion() const override;

  virtual uint GetHash() const = 0;
  virtual void SetHash(uint nHash) = 0;
  virtual uint GetGroupHash(int groupID) const = 0;
};

struct LMTTrack {
  enum TrackType {
    TrackType_LocalRotation,
    TrackType_LocalPosition,
    TrackType_LocalScale,
    TrackType_AbsoluteRotation,
    TrackType_AbsolutePosition
  };

  virtual TrackType GetTrackType() const = 0;
  virtual int AnimatedBoneID() const = 0;
  virtual int NumFrames() const = 0;
  virtual bool IsCubic() const = 0;
  virtual void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                           int frame) const = 0;
  virtual void Evaluate(Vector4A16 &out, int frame) const = 0;
  virtual short GetFrame(int frame) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone) const = 0;
  virtual int Stride() const = 0;

  virtual ~LMTTrack() {}

  static LMTTrack *Create(const LMTConstructorProperties &props);
};

class LMTAnimation {
protected:
  LMTConstructorPropertiesBase props;

public:
  LMTAnimation() : props{} {}

  virtual int GetVersion() const = 0;
  virtual const int NumTracks() const = 0;
  virtual const int NumFrames() const = 0;
  virtual const int LoopFrame() const = 0;
  virtual const LMTTrack *Track(int id) const = 0;
  virtual int ToXML(pugi::xml_node &node, bool standAlone = false) const = 0;
  virtual int FromXML(pugi::xml_node &node) = 0;
  virtual int Save(BinWritter *wr, bool standAlone = true) const = 0;
  virtual ~LMTAnimation() {}
  virtual void Sanitize() const = 0;

  int Save(const char *fileName, bool supressErrors = false) const;
  static LMTAnimation *Create(const LMTConstructorProperties &props);
  static bool SupportedVersion(short version);

  bool operator==(const LMTConstructorPropertiesBase &input) {
    return props.ptrSize == input.ptrSize && props.version == input.version;
  }

  bool operator!=(const LMTConstructorPropertiesBase &input) {
    return !operator==(input);
  }
};

class LMT {
  static const int ID = CompileFourCC("LMT\0");
  static const int ID_R = CompileFourCC("\0TML");

public:
  typedef std::vector<LMTAnimation *> Storage_Type;
  typedef Storage_Type::const_iterator Iter_Type;

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

  ES_FORCEINLINE uchar Version() const { return props.version; }
  ES_FORCEINLINE Architecture GetArchitecture() const {
    return (props.ptrSize == 8) ? X64 : X86;
  }
  int Version(V version, Architecture arch);

  ES_FORCEINLINE const Iter_Type begin() const { return animations.begin(); }
  ES_FORCEINLINE const Iter_Type end() const { return animations.end(); }
  ES_FORCEINLINE int NumAnimations() const {
    return static_cast<int>(animations.size());
  }
  ES_FORCEINLINE const LMTAnimation *Animation(int id) const {
    return animations[id];
  }

  LMTAnimation *AppendAnimation();
  void AppendAnimation(LMTAnimation *ani);
  void InsertAnimation(LMTAnimation *ani, int at, bool replace = false);

  LMTAnimation *CreateAnimation() const;

  int Load(BinReader *rd);
  int Load(const char *fileName, bool supressErrors = false);
  int Save(BinWritter *wr) const;
  int Save(const char *fileName, bool swapEndian = false, bool supressErrors = false) const;

  int ToXML(pugi::xml_node &node, const char *fileName,
            ExportSettings settings);
  int ToXML(const char *fileName, ExportSettings settings);

  int FromXML(pugi::xml_node &node, const char *fileName,
              Architecture forceArchitecture = Xundefined);
  int FromXML(const char *fileName,
              Architecture forceArchitecture = Xundefined);

  LMT();
  ~LMT();

private:
  char *masterBuffer;
  LMTConstructorPropertiesBase props;
  Storage_Type animations;
};

// TODO:
/*
add header sanitizers

low priority:
conversion system << super low
encoder, exporting utility
*/