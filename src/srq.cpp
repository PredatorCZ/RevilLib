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

#include "revil/srq.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <cassert>
#include <vector>

struct Element {
  uint16 requestId;
  uint32 category;
  uint32 command;
  uint8 global;
  uint16 ids[3];
  uint8 priority;
  uint8 priorityMode;
  uint32 limit;
  int16 link;
  uint16 programIndex;
  uint16 splitIndex;
  uint8 volume;
  int16 pan;
  int32 pitchShift;
  uint32 reverbSend;
  uint32 LFESend;
  uint32 randomReqIndex;
  uint32 delayTimer;
  uint32 bookingTimer;
  uint32 centerVolume;
  int32 volumeCurveID;
  int32 reverbCurveID;
  int32 LFECurveID;
  int32 directionalCurveID;
  int16 eqIndex;
  int16 eqReverbIndex;
  float interiorDistance;
  float dopplerScaler;
  uint32 freeArea[8];
  int16 randomVolumeMax;
  int16 randomVolumeMin;
  int16 randomPitchMax;
  int16 randomPitchMin;
  int32 packagePathTableIndex;
  int32 speakerSetIndex;
  uint32 mpPackage;
  uint32 mpSpeakerSet;
};

struct SoundRequestHeader {
  uint32 id;
  revil::SRQVersion version;
  uint32 numElements;
  uint32 numSpeakerSets;
  uint32 numSpeakers;
  uint32 numDirectionalCurves;
  uint32 numDirectionalCurveElements;
  uint32 packagePathsTable;
  uint32 soundRandomPath;
  uint32 speakerSets;
  uint32 speakers;
  uint32 directionalCurves;
  uint32 directionalCurveElements;
};

void ToXML(const Element &data, pugi::xml_node node) {
  NewPrimitive(node, "requestId", data.requestId);
  NewPrimitive(node, "category", data.category);
  NewPrimitive(node, "command", data.command);
  NewPrimitive(node, "global", data.global);
  NewPrimitive(node, "ID[0]", data.ids[0]);
  NewPrimitive(node, "ID[1]", data.ids[1]);
  NewPrimitive(node, "ID[2]", data.ids[2]);
  NewPrimitive(node, "priority", data.priority);
  NewPrimitive(node, "priorityMode", data.priorityMode);
  NewPrimitive(node, "limit", data.limit);
  NewPrimitive(node, "link", data.link);
  NewPrimitive(node, "programIndex", data.programIndex);
  NewPrimitive(node, "splitIndex", data.splitIndex);
  NewPrimitive(node, "volume", data.volume);
  NewPrimitive(node, "pan", data.pan);
  NewPrimitive(node, "pitchShift", data.pitchShift);
  NewPrimitive(node, "reverbSend", data.reverbSend);
  NewPrimitive(node, "LFESend", data.LFESend);
  NewPrimitive(node, "randomReqIndex", data.randomReqIndex);
  NewPrimitive(node, "delayTimer", data.delayTimer);
  NewPrimitive(node, "bookingTimer", data.bookingTimer);
  NewPrimitive(node, "centerVolume", data.centerVolume);
  NewPrimitive(node, "volumeCurveID", data.volumeCurveID);
  NewPrimitive(node, "reverbCurveID", data.reverbCurveID);
  NewPrimitive(node, "LFECurveID", data.LFECurveID);
  NewPrimitive(node, "directionalCurveID", data.directionalCurveID);
  NewPrimitive(node, "eqIndex", data.eqIndex);
  NewPrimitive(node, "eqReverbIndex", data.eqReverbIndex);
  NewPrimitive(node, "interiorDistance", data.interiorDistance);
  NewPrimitive(node, "dopplerScaler", data.dopplerScaler);
  NewPrimitive(node, "freeArea[0]", data.freeArea[0]);
  NewPrimitive(node, "freeArea[1]", data.freeArea[1]);
  NewPrimitive(node, "freeArea[2]", data.freeArea[2]);
  NewPrimitive(node, "freeArea[3]", data.freeArea[3]);
  NewPrimitive(node, "freeArea[4]", data.freeArea[4]);
  NewPrimitive(node, "freeArea[5]", data.freeArea[5]);
  NewPrimitive(node, "freeArea[6]", data.freeArea[6]);
  NewPrimitive(node, "freeArea[7]", data.freeArea[7]);
  NewPrimitive(node, "randomVolumeMax", data.randomVolumeMax);
  NewPrimitive(node, "randomVolumeMin", data.randomVolumeMin);
  NewPrimitive(node, "randomPitchMax", data.randomPitchMax);
  NewPrimitive(node, "randomPitchMin", data.randomPitchMin);
  NewPrimitive(node, "packagePathTableIndex", data.packagePathTableIndex);
  NewPrimitive(node, "speakerSetIndex", data.speakerSetIndex);
}

struct revil::SRQImpl {
  std::vector<Element> elements;
  std::string soundRandom;
  std::vector<std::string> bankList;

  void Load(BinReaderRef rd) {
    SoundRequestHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("SREQ")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    assert(hdr.numDirectionalCurveElements == 0);
    assert(hdr.numDirectionalCurves == 0);
    assert(hdr.numSpeakers == 0);
    assert(hdr.numSpeakerSets == 0);

    if (hdr.version == SRQVersion::V_14) {
      rd.ReadContainer(elements, hdr.numElements);
      rd.Seek(hdr.soundRandomPath);
      rd.ReadString(soundRandom);
      int32 maxPackId = -1;

      for (auto &e : elements) {
        maxPackId = std::max(e.packagePathTableIndex, maxPackId);
      }

      rd.Seek(hdr.packagePathsTable);

      for (int32 p = 0; p <= maxPackId; p++) {
        uint32 offset;
        rd.Read(offset);
        rd.Push();
        rd.Seek(offset);
        rd.ReadString(bankList.emplace_back());
        rd.Pop();
      }
    } else {
      throw es::InvalidVersionError(uint32(hdr.version));
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node srqNode = NewProperty(MtPropertyType::class_, node);
    srqNode.append_attribute("type").set_value("rSoundRequest");
    srqNode.append_attribute("id").set_value(curId++);

    pugi::xml_node randNode = NewProperty(MtPropertyType::custom, srqNode);
    randNode.append_attribute("name").set_value("mpRandom");
    randNode.append_attribute("ctype").set_value("resource");
    randNode.append_attribute("rtype").set_value("rSoundRandom");
    randNode.append_attribute("path").set_value(soundRandom.c_str());

    pugi::xml_node banksNode =
        NewArray(MtPropertyType::custom, srqNode, "mBankList", bankList.size());

    for (auto &b : bankList) {
      pugi::xml_node bankNode = NewProperty(MtPropertyType::custom, banksNode);
      bankNode.append_attribute("ctype").set_value("resource");
      bankNode.append_attribute("rtype").set_value("rSoundBank");
      bankNode.append_attribute("path").set_value(b.c_str());
    }

    pugi::xml_node elementsNode =
        NewArray(MtPropertyType::class_, srqNode, "mpElement", elements.size());

    for (auto &s : elements) {
      pugi::xml_node curNode =
          NewProperty(MtPropertyType::class_, elementsNode);
      curNode.append_attribute("type").set_value("rSoundRequest::Element");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode);
    }
  }
};

namespace revil {
SRQ::SRQ() : pi(std::make_unique<SRQImpl>()) {}
SRQ::~SRQ() = default;

void SRQ::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SRQ::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
