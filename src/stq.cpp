/*  Revil Format Library
    Copyright(C) 2026 Lukas Cone

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

#include "revil/stq.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/type/pointer.hpp"
#include <cassert>

namespace {
struct Element {
  uint16 mReqNo;
  uint32 mCategory;
  uint32 mCommand;
  uint32 mReadType;
  uint32 mDiskLocation;
  uint8 mGlobal;
  int16 mID_1;
  int16 mID_2;
  int16 mID_3;
  uint8 mPriority;
  uint8 mPrioMode;
  uint32 mLimit;
  int16 mLink;
  int16 mVol;
  int16 mPan;
  int32 mPitchShift;
  uint32 mReverbSend;
  uint32 mLFESend;
  uint32 mRandomReqNo;
  uint32 mDelayTimer;
  uint32 mBookingTimer;
  uint32 mCenterVolume;
  int32 mVolumeCurveID;
  int32 mReverbCurveID;
  int32 mLFECurveID;
  int32 mDirectionalCurveID;
  int16 mEqNo;
  int16 mEqReverbNo;
  float mInteriorDistance;
  float mDopplerScaler;
  uint32 mTime;
  int32 mFreeArea00;
  int32 mFreeArea01;
  int32 mFreeArea02;
  int32 mFreeArea03;
  int32 mFreeArea04;
  int32 mFreeArea05;
  int32 mFreeArea06;
  int32 mFreeArea07;
  int16 mRandomVolumeMax;
  int16 mRandomVolumeMin;
  int16 mRandomPitchMax;
  int16 mRandomPitchMin;
  int32 mStreamID;
  int32 mSpeakerSetID;
  uint32 mpSource;
  uint32 mpSpeakerSet;
};

struct StreamInfo {
  es::PointerX86<char> mPath;
  uint32 mStreamLength;
  uint32 mSampleNum;
  uint32 mChannelNum;
  int32 mLoopStart;
  int32 mLoopEnd;
};

struct SoundStreamRequestHeader {
  uint32 id;
  revil::STQVersion version;
  uint32 numStreams;
  uint32 numElements;
  uint32 numSpeakerSets;
  uint32 numSpeakers;
  uint32 numDirectionalCurves;
  uint32 numDirectionalCurveElements;
  es::PointerX86<StreamInfo> streamInfo;
  es::PointerX86<Element> element;
  es::PointerX86<char> speakerSet;
  es::PointerX86<char> speaker;
  es::PointerX86<char> directionalCurve;
  es::PointerX86<char> directionalCurveElement;
};
} // namespace

void ToXML(const StreamInfo &data, pugi::xml_node node) {
  pugi::xml_node path = NewProperty(MtPropertyType::string_, node);
  path.append_attribute("name").set_value("mPath");
  path.append_attribute("value").set_value(data.mPath.Get());
  NewPrimitive(node, "mStreamLength", data.mStreamLength);
  NewPrimitive(node, "mSampleNum", data.mSampleNum);
  NewPrimitive(node, "mChannelNum", data.mChannelNum);
  NewPrimitive(node, "mLoopStart", data.mLoopStart);
  NewPrimitive(node, "mLoopEnd", data.mLoopEnd);
}

void ToXML(const Element &data, pugi::xml_node node) {
  NewPrimitive(node, "mReqNo", data.mReqNo);
  NewPrimitive(node, "mCategory", data.mCategory);
  NewPrimitive(node, "mCommand", data.mCommand);
  NewPrimitive(node, "mReadType", data.mReadType);
  NewPrimitive(node, "mDiskLocation", data.mDiskLocation);
  NewPrimitive(node, "mGlobal", data.mGlobal);
  NewPrimitive(node, "mID_1", data.mID_1);
  NewPrimitive(node, "mID_2", data.mID_2);
  NewPrimitive(node, "mID_3", data.mID_3);
  NewPrimitive(node, "mPriority", data.mPriority);
  NewPrimitive(node, "mPrioMode", data.mPrioMode);
  NewPrimitive(node, "mLimit", data.mLimit);
  NewPrimitive(node, "mLink", data.mLink);
  NewPrimitive(node, "mVol", data.mVol);
  NewPrimitive(node, "mPan", data.mPan);
  NewPrimitive(node, "mPitchShift", data.mPitchShift);
  NewPrimitive(node, "mReverbSend", data.mReverbSend);
  NewPrimitive(node, "mLFESend", data.mLFESend);
  NewPrimitive(node, "mRandomReqNo", data.mRandomReqNo);
  NewPrimitive(node, "mDelayTimer", data.mDelayTimer);
  NewPrimitive(node, "mBookingTimer", data.mBookingTimer);
  NewPrimitive(node, "mCenterVolume", data.mCenterVolume);
  NewPrimitive(node, "mVolumeCurveID", data.mVolumeCurveID);
  NewPrimitive(node, "mReverbCurveID", data.mReverbCurveID);
  NewPrimitive(node, "mLFECurveID", data.mLFECurveID);
  NewPrimitive(node, "mDirectionalCurveID", data.mDirectionalCurveID);
  NewPrimitive(node, "mEqNo", data.mEqNo);
  NewPrimitive(node, "mEqReverbNo", data.mEqReverbNo);
  NewPrimitive(node, "mInteriorDistance", data.mInteriorDistance);
  NewPrimitive(node, "mDopplerScaler", data.mDopplerScaler);
  NewPrimitive(node, "mTime", data.mTime);
  NewPrimitive(node, "mFreeArea00", data.mFreeArea00);
  NewPrimitive(node, "mFreeArea01", data.mFreeArea01);
  NewPrimitive(node, "mFreeArea02", data.mFreeArea02);
  NewPrimitive(node, "mFreeArea03", data.mFreeArea03);
  NewPrimitive(node, "mFreeArea04", data.mFreeArea04);
  NewPrimitive(node, "mFreeArea05", data.mFreeArea05);
  NewPrimitive(node, "mFreeArea06", data.mFreeArea06);
  NewPrimitive(node, "mFreeArea07", data.mFreeArea07);
  NewPrimitive(node, "mRandomVolumeMax", data.mRandomVolumeMax);
  NewPrimitive(node, "mRandomVolumeMin", data.mRandomVolumeMin);
  NewPrimitive(node, "mRandomPitchMax", data.mRandomPitchMax);
  NewPrimitive(node, "mRandomPitchMin", data.mRandomPitchMin);
  NewPrimitive(node, "mStreamID", data.mStreamID);
  NewPrimitive(node, "mSpeakerSetID", data.mSpeakerSetID);
}

struct revil::STQImpl {
  std::string buffer;
  SoundStreamRequestHeader *hdr = nullptr;

  void Load(BinReaderRef rd) {
    uint32 id;
    rd.Push();
    rd.Read(id);
    rd.Pop();

    if (id != CompileFourCC("STRQ")) {
      throw es::InvalidHeaderError(id);
    }

    rd.ReadContainer(buffer, rd.GetSize());
    hdr = reinterpret_cast<SoundStreamRequestHeader *>(buffer.data());

    assert(hdr->numDirectionalCurveElements == 0);
    assert(hdr->numDirectionalCurves == 0);
    assert(hdr->numSpeakers == 0);
    assert(hdr->numSpeakerSets == 0);

    if (hdr->version != STQVersion::V_16) {
      throw es::InvalidVersionError(int(hdr->version));
    }

    es::FixupPointers(buffer.data(), hdr->streamInfo, hdr->element);

    for (uint32 i = 0; i < hdr->numStreams; i++) {
      hdr->streamInfo[i].mPath.Fixup(buffer.data());
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node srqNode = NewProperty(MtPropertyType::class_, node);
    srqNode.append_attribute("type").set_value("rSoundStreamRequest");
    srqNode.append_attribute("id").set_value(curId++);

    pugi::xml_node streamsNode = NewArray(MtPropertyType::class_, srqNode,
                                          "mpStreamInfo", hdr->numStreams);

    for (uint32 i = 0; i < hdr->numStreams; i++) {
      pugi::xml_node strNode = NewProperty(MtPropertyType::class_, streamsNode);
      strNode.append_attribute("type").set_value(
          "rSoundStreamRequest::StreamInfo");
      strNode.append_attribute("id").set_value(curId++);
      ::ToXML(hdr->streamInfo[i], strNode);
    }

    pugi::xml_node elementsNode = NewArray(MtPropertyType::class_, srqNode,
                                           "mpElement", hdr->numElements);

    for (uint32 i = 0; i < hdr->numElements; i++) {
      pugi::xml_node curNode =
          NewProperty(MtPropertyType::class_, elementsNode);
      curNode.append_attribute("type").set_value(
          "rSoundStreamRequest::Element");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(hdr->element[i], curNode);
    }
  }
};

namespace revil {
STQ::STQ() : pi(std::make_unique<STQImpl>()) {}
STQ::~STQ() = default;

void STQ::Load(BinReaderRef_e rd) { pi->Load(rd); }

void STQ::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
