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

#include "revil/osf.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <vector>

struct ObjSetUControlCreateParam {
  bool mIsEnable;
  uint8 mId;
  uint8 field_6;
  uint8 field_7;
  uint8 thinkKind;
  uint8 thinkType;
  uint8 thinkId;
  uint8 thinkGroup;
  bool mIsBoss;
};

struct ObjSetUCharacterCreateParam {
  bool mIsEnable;
  uint8 mGroup;
  uint8 mId;
  uint8 mType;
  uint8 mKind;
};

struct ObjSetDataBase {
  int id;
  int groupNo;
  int controlStatus;
  alignas(4) ObjSetUControlCreateParam mUctrlParam;
  alignas(4) ObjSetUCharacterCreateParam mUcharParam;
  MtVector3 mPosition;
  MtVector3 mAngle;
  uint32 mUserData[16];
  uint32 index;
};

struct HmSetData : ObjSetDataBase {
  ObjSetUCharacterCreateParam mRightArm;
  ObjSetUCharacterCreateParam mLeftArm;
  ObjSetUCharacterCreateParam mReserve;
};

struct VsSetData : ObjSetDataBase {
  ObjSetUCharacterCreateParam mRightArm;
  ObjSetUCharacterCreateParam mLeftArm;
  ObjSetUCharacterCreateParam mRightShoulder;
  ObjSetUCharacterCreateParam mLeftShoulder;
  int32 mRiderParam;
};

struct ControlEntry {
  uint16 objIndex;
  uint16 objType;
};

struct ControlData {
  uint32 mNumber;
  uint32 mInterval;
};

struct Palette {
  uint8 unused;
  uint8 charParamType;
  uint8 charParamId;
  uint8 charParamGroup;
};

struct ObjSetHeader {
  uint32 id;
  uint32 version;
  uint16 numHmSets;
  uint16 numVsSets;
  uint16 numAkSets;
  uint16 numOmSets;
  uint16 numWpSets;
  uint16 numControlParams;
  uint16 numPalettes;
  uint32 reserved[6];
};

void ToXML(const ObjSetUControlCreateParam &data, pugi::xml_node node) {
  NewPrimitive(node, "mIsEnable", data.mIsEnable);
  NewPrimitive(node, "mId", data.mId);
  NewPrimitive(node, "thinkKind(HEX)", data.thinkKind);
  NewPrimitive(node, "thinkType(HEX)", data.thinkType);
  NewPrimitive(node, "thinkId", data.thinkId);
  NewPrimitive(node, "thinkGroup", data.thinkGroup);
  NewPrimitive(node, "mIsBoss", data.mIsBoss);
}

void ToXML(const ObjSetUCharacterCreateParam &data, pugi::xml_node node) {
  NewPrimitive(node, "mIsEnable", data.mIsEnable);
  NewPrimitive(node, "mGroup", data.mGroup);
  NewPrimitive(node, "mId", data.mId);
  NewPrimitive(node, "mType", data.mType);
  NewPrimitive(node, "mKind", data.mKind);
}

void ToXML(const ObjSetDataBase &data, pugi::xml_node node, uint32 &curId) {
  NewPrimitive(node, "id", data.id);
  NewPrimitive(node, "groupNo", data.groupNo);
  NewPrimitive(node, "controlStatus", data.controlStatus);
  pugi::xml_node ctrlNode = NewProperty(MtPropertyType::class_, node);
  ctrlNode.append_attribute("name").set_value("mUctrlParam");
  ctrlNode.append_attribute("type").set_value("cUcontrolCreateParam");
  ctrlNode.append_attribute("id").set_value(curId++);
  ToXML(data.mUctrlParam, ctrlNode);
  pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
  charNode.append_attribute("name").set_value("mUcharParam");
  charNode.append_attribute("type").set_value("cUcharacterCreateParam");
  charNode.append_attribute("id").set_value(curId++);
  ToXML(data.mUcharParam, charNode);
  NewPrimitive(node, "mPosition", data.mPosition);
  NewPrimitive(node, "mAngle", data.mAngle);

  for (uint32 id = 0; uint32 u : data.mUserData) {
    char name[0x20]{};
    snprintf(name, sizeof(name), "mUserData[%u]", id++);
    NewPrimitive(node, name, u);
  }
}

void ToXML(const HmSetData &data, pugi::xml_node node, uint32 &curId) {
  ToXML(static_cast<const ObjSetDataBase &>(data), node, curId);
  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mRightArm");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mRightArm, charNode);
  }

  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mLeftArm");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mLeftArm, charNode);
  }

  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mReserve");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mReserve, charNode);
  }
}

void ToXML(const VsSetData &data, pugi::xml_node node, uint32 &curId) {
  ToXML(static_cast<const ObjSetDataBase &>(data), node, curId);
  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mRightArm");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mRightArm, charNode);
  }

  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mLeftArm");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mLeftArm, charNode);
  }

  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mRightShoulder");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mRightShoulder, charNode);
  }

  {
    pugi::xml_node charNode = NewProperty(MtPropertyType::class_, node);
    charNode.append_attribute("name").set_value("mLeftShoulder");
    charNode.append_attribute("type").set_value("cUcharacterCreateParam");
    charNode.append_attribute("id").set_value(curId++);
    ToXML(data.mLeftShoulder, charNode);
  }
}

struct revil::OSFImpl {
  std::vector<HmSetData> hmSetData;
  std::vector<VsSetData> vsSetData;
  std::vector<ObjSetDataBase> akSetData;
  std::vector<ObjSetDataBase> omSetData;
  std::vector<ObjSetDataBase> wpSetData;
  ControlData controlData[128];

  void Load(BinReaderRef rd) {
    ObjSetHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("OSF")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    if (hdr.version != 0x20060626) {
      throw es::InvalidVersionError(hdr.version);
    }

    rd.ReadContainer(hmSetData, hdr.numHmSets);
    rd.ReadContainer(vsSetData, hdr.numVsSets);
    rd.ReadContainer(akSetData, hdr.numAkSets);
    rd.ReadContainer(omSetData, hdr.numOmSets);
    rd.ReadContainer(wpSetData, hdr.numWpSets);
    rd.SetRelativeOrigin(rd.Tell(), false);
    uint32 offsets[128];
    rd.Read(offsets);
    ControlData *curControl = controlData;

    for (uint32 o : offsets) {
      rd.Seek(o * 4);
      rd.Read(*curControl++);
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node setNode = NewProperty(MtPropertyType::class_, node);
    setNode.append_attribute("type").set_value("rObjSetXml");
    setNode.append_attribute("id").set_value(curId++);

    pugi::xml_node hmSetNode = NewArray(MtPropertyType::class_, setNode,
                                        "mHmSetData", hmSetData.size());

    for (auto &s : hmSetData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, hmSetNode);
      curNode.append_attribute("type").set_value("cHmSetData");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode, curId);
    }

    pugi::xml_node vsSetNode = NewArray(MtPropertyType::class_, setNode,
                                        "mVsSetData", vsSetData.size());

    for (auto &s : vsSetData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, vsSetNode);
      curNode.append_attribute("type").set_value("cVsSetData");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode, curId);
    }

    pugi::xml_node akSetNode = NewArray(MtPropertyType::class_, setNode,
                                        "maAkSetData", akSetData.size());

    for (auto &s : akSetData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, akSetNode);
      curNode.append_attribute("type").set_value("cAkSetData");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode, curId);
    }

    pugi::xml_node omSetNode = NewArray(MtPropertyType::class_, setNode,
                                        "mOmSetData", omSetData.size());

    for (auto &s : omSetData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, omSetNode);
      curNode.append_attribute("type").set_value("cOmSetData");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode, curId);
    }

    pugi::xml_node wpSetNode = NewArray(MtPropertyType::class_, setNode,
                                        "mWpSetData", wpSetData.size());

    for (auto &s : wpSetData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, wpSetNode);
      curNode.append_attribute("type").set_value("cWpSetData");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode, curId);
    }

    pugi::xml_node controlNode =
        NewArray(MtPropertyType::class_, setNode, "mControlData", 128);

    for (auto &s : controlData) {
      pugi::xml_node curNode = NewProperty(MtPropertyType::class_, controlNode);
      curNode.append_attribute("type").set_value("cControlData");
      curNode.append_attribute("id").set_value(curId++);
      NewPrimitive(curNode, "mNumber", s.mNumber);
      NewPrimitive(curNode, "mInterval", s.mInterval);
    }
  }
};

namespace revil {
OSF::OSF() : pi(std::make_unique<OSFImpl>()) {}
OSF::~OSF() = default;

void OSF::Load(BinReaderRef_e rd) { pi->Load(rd); }

void OSF::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
