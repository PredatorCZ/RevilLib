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

#include "animation.hpp"
#include "bone_track.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/except.hpp"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "event.hpp"
#include "float_track.hpp"

#define _xml_comp_offset(_node)                                                \
  (_node.name() - (*_node.root().children().begin()).name() + 25)

#define XML_WARNING(node, ...)                                                 \
  printwarning("[LMT XML: " << _xml_comp_offset(node) << "] " __VA_ARGS__);

void FloatFrame::Save(pugi::xml_node node) const {
  pugi::xml_node currentNode = node.append_child("track");
  const size_t numComponents = NumComponents();
  std::string resVal;

  for (size_t c = 0; c < numComponents; c++) {
    resVal.append(std::to_string(value[c]) + ", ");
  }

  if (resVal.size()) {
    resVal.pop_back();
    resVal.pop_back();
    currentNode.append_buffer(resVal.c_str(), resVal.size());
  }

  currentNode.append_attribute("frame").set_value(Frame());
}

void FloatFrame::Load(pugi::xml_node node) {
  pugi::xml_attribute cFrame = node.attribute("frame");

  if (cFrame.empty()) {
    throw XMLMissingNodeAttributeException("frame", node);
  }

  Frame(cFrame.as_int());

  if (node.text().empty()) {
    throw XMLEmptyData(node);
  }

  size_t numItems = 0;
  std::string nodeText = node.text().get();
  nodeText.push_back('\0');
  std::string outputText;

  for (auto &c : nodeText) {
    if ((c >= '0' && c <= '9') || c == '.') {
      outputText.push_back(c);
    } else if (c == ',' || c == '\0') {
      value[numItems++] = std::stof(outputText);
      outputText.clear();

      if (numItems > 2) {
        break;
      }
    }
  }

  NumComponents(numItems);
}

void LMTFloatTrack_internal::Save(pugi::xml_node node, bool) const {
  pugi::xml_node evGroupsNode = node.append_child("floatTracks");

  for (size_t g = 0; g < GetNumGroups(); g++) {
    pugi::xml_node evGroupNode = evGroupsNode.append_child("floatTrack");

    ReflectToXML(evGroupNode, g);

    pugi::xml_node rangesNode = evGroupNode.append_child("tracks");
    size_t numEvents = GetGroupTrackCount(g);

    const FloatFrame *cEvents = GetFrames(g);
    const FloatFrame *cEventsEnd = cEvents + numEvents;

    for (; cEvents != cEventsEnd; cEvents++) {
      cEvents->Save(rangesNode);
    }
  }
}

void LMTFloatTrack_internal::Load(pugi::xml_node floatTracksNode) {
  auto floatTrackNodes = XMLCollectChildren(floatTracksNode, "floatTrack");
  const size_t numGroups = GetNumGroups();

  if (floatTrackNodes.size() != numGroups) {
    XML_WARNING(
        floatTracksNode,
        "<floatTrack/> count is different than built-in count. Expected: "
            << numGroups << ", got: " << floatTrackNodes.size())
  }

  for (size_t e = 0; e < numGroups; e++) {
    pugi::xml_node flTrackNode = floatTrackNodes[e];

    ReflectFromXML(flTrackNode, e);

    pugi::xml_node rangesNode = flTrackNode.child("tracks");

    if (rangesNode.empty()) {
      throw XMLMissingNodeException("tracks", flTrackNode);
    }

    auto rangeNodes = XMLCollectChildren(rangesNode, "track");

    if (!rangeNodes.size()) {
      continue;
    }

    SetNumFrames(e, rangeNodes.size());

    FloatFrame *cFrames = GetFrames(e);
    FloatFrame *const cFramesEnd = cFrames + rangeNodes.size();
    size_t curEventTrigger = 0;

    for (; cFrames != cFramesEnd; cFrames++, curEventTrigger++) {
      cFrames->Load(rangeNodes[curEventTrigger]);
    }
  }
}

void AnimEvent::Save(pugi::xml_node node) const {
  size_t numItems = 0;
  pugi::xml_node currentNode = node.append_child("event");

  if (runEventBit) {
    std::string resVal;
    using TriggerType = decltype(runEventBit);
    const size_t numTriggers = sizeof(TriggerType) * 8;

    for (size_t b = 0; b < numTriggers; b++) {
      if (runEventBit & (TriggerType(1) << b)) {
        resVal += std::to_string(b) + ", ";
        numItems++;
      }
    }

    resVal.pop_back();
    resVal.pop_back();
    currentNode.append_buffer(resVal.c_str(), resVal.size());
  }

  currentNode.append_attribute("additiveFrames").set_value(numFrames);
}

void AnimEvent::Load(pugi::xml_node node) {
  pugi::xml_attribute addFrames = node.attribute("additiveFrames");

  if (addFrames.empty()) {
    throw XMLMissingNodeException("additiveFrames", node);
  }

  numFrames = addFrames.as_int();

  if (node.text().empty()) {
    return;
  }

  uint32 currentID = 0;
  uint32 currentChar = 0;
  std::string nodeText = node.text().get();
  nodeText.push_back('\0');

  for (auto &c : nodeText) {
    if (c >= '0' && c <= '9') {
      currentID |= static_cast<uint32>(c) << ((3 - currentChar) * 8);
      currentChar++;
    } else if (c == ',' || c == '\0') {
      FByteswapper(currentID);
      runEventBit |= 1 << atoi(reinterpret_cast<char *>(&currentID)); // oh no
      currentID = 0;
      currentChar = 0;
    }
  }
}

void LMTAnimationEventV1_Internal::Save(pugi::xml_node node, bool) const {
  pugi::xml_node evGroupsNode = node.append_child("eventGroups");

  for (uint32 g = 0; g < GetNumGroups(); g++) {
    pugi::xml_node evGroupNode = evGroupsNode.append_child("eventGroup");

    ReflectToXML(evGroupNode, g);

    pugi::xml_node rangesNode = evGroupNode.append_child("events");
    const EventsCollection &cEvents = GetEvents(g);

    for (auto &e : cEvents) {
      e.Save(rangesNode);
    }
  }
}

void LMTAnimationEventV1_Internal::Load(pugi::xml_node node) {
  pugi::xml_node eventGroupsNode = node.child("eventGroups");

  if (eventGroupsNode.empty()) {
    throw XMLMissingNodeException("eventGroups", node);
  }

  auto eventGroupNodes = XMLCollectChildren(eventGroupsNode, "eventGroup");
  const uint32 numGroups = GetNumGroups();

  if (eventGroupNodes.size() != numGroups) {
    XML_WARNING(
        eventGroupsNode,
        "<eventGroup/> count is different than built-in count. Expected: "
            << numGroups << ", got: " << eventGroupNodes.size())
  }

  for (uint32 e = 0; e < numGroups; e++) {
    pugi::xml_node evGroupNode = eventGroupNodes[e];

    ReflectFromXML(evGroupNode, e);

    pugi::xml_node rangesNode = evGroupNode.child("events");

    if (rangesNode.empty()) {
      throw XMLMissingNodeException("events", evGroupNode);
    }

    auto rangeNodes = XMLCollectChildren(rangesNode, "event");

    if (rangeNodes.empty()) {
      throw XMLMissingNodeException("event", evGroupNode);
    }

    SetNumEvents(e, static_cast<uint32>(rangeNodes.size()));

    EventsCollection &cEvents = GetEvents(e);
    uint32 curEventTrigger = 0;

    for (auto &e : cEvents) {
      e.Load(rangeNodes[curEventTrigger++]);
    }
  }

  return;
}

void AnimEventFrameV2::Save(pugi::xml_node node) const {
  ReflectorWrap<const AnimEventFrameV2> reflFrame(this);
  ReflectorXMLUtil::Save(reflFrame, node);

  pugi::xml_node dNode = node.append_child("data");
  std::string obuff;
  obuff += '[';

  for (uint32 d = 0; d < 3; d++) {
    if (dataType == EventFrameV2DataType::Float) {
      obuff.append(std::to_string(fdata[d])) += ", ";
    } else {
      obuff.append(std::to_string(idata[d])) += ", ";
    }
  }

  obuff.pop_back();
  obuff.pop_back();
  obuff += ']';

  dNode.append_buffer(obuff.c_str(), obuff.size());
}

void AnimEventFrameV2::Load(pugi::xml_node node) {
  ReflectorWrap<AnimEventFrameV2> reflFrame(this);
  ReflectorXMLUtil::Load(reflFrame, node);

  std::string valueCopy = node.child("data").text().as_string();
  size_t firstBrace = valueCopy.find_first_of('[');

  if (firstBrace == valueCopy.npos) {
    firstBrace = 0;
  } else {
    firstBrace++;
  }

  valueCopy = valueCopy.substr(firstBrace, valueCopy.find(']', firstBrace));

  std::replace(valueCopy.begin(), valueCopy.end(), '[', ' ');
  std::replace(valueCopy.begin(), valueCopy.end(), ']', ' ');

  size_t currentItem = 0;
  size_t lastItem = 0;

  for (size_t i = 0; i < valueCopy.size() + 1; i++) {
    if (valueCopy[i] == ',' || valueCopy[i] == '\0') {

      if (dataType == EventFrameV2DataType::Float) {
        fdata[currentItem++] =
            std::strtof(valueCopy.data() + lastItem, nullptr);
      } else {
        idata[currentItem++] = atoi(valueCopy.data() + lastItem);
      }

      lastItem = i + 1;
    }
  }
}

void LMTAnimationEventV2Event::Load(pugi::xml_node node) {
  pugi::xml_attribute eHashAttr = node.attribute("hash");
  pugi::xml_attribute eNumFrames = node.attribute("numFrames");

  if (eHashAttr.empty()) {
    throw XMLMissingNodeAttributeException("hash", node);
  }

  SetHash(eHashAttr.as_uint());

  auto eventFramesNodes = XMLCollectChildren(node, "frame");

  if (!eNumFrames.empty() && eNumFrames.as_uint() != eventFramesNodes.size()) {
    XML_WARNING(node,
                "<event numFrames=/> differs from actual <frame/> count.");
  }

  frames.resize(eventFramesNodes.size());

  for (size_t f = 0; f < eventFramesNodes.size(); f++) {
    frames[f].Load(eventFramesNodes[f]);
  }
}

void LMTAnimationEventV2Group::Load(pugi::xml_node node) {
  pugi::xml_attribute cHashAttr = node.attribute("hash");
  pugi::xml_attribute cNumEvents = node.attribute("numEvents");

  if (cHashAttr.empty()) {
    throw XMLMissingNodeAttributeException("hash", node);
  }

  SetHash(cHashAttr.as_uint());

  auto eventNodes = XMLCollectChildren(node, "event");

  if (!cNumEvents.empty() && cNumEvents.as_uint() != eventNodes.size()) {
    XML_WARNING(node,
                "<eventGroup numEvents=/> differs from actual <event/> count.");
  }

  for (auto &e : eventNodes) {
    events.emplace_back(LMTAnimationEventV2Event::Create());
    events.back()->Load(e);
  }
}

void LMTAnimationEventV2_Internal::Save(pugi::xml_node node, bool) const {
  pugi::xml_node evGroupsNode = node.append_child("eventGroups");
  evGroupsNode.append_attribute("hash").set_value(GetHash());

  for (auto &g : groups) {
    pugi::xml_node evGroupNode = evGroupsNode.append_child("eventGroup");
    evGroupNode.append_attribute("numEvents").set_value(g->events.size());
    evGroupNode.append_attribute("hash").set_value(g->GetHash());

    for (auto &e : g->events) {
      pugi::xml_node evNode = evGroupNode.append_child("event");
      evNode.append_attribute("numFrames").set_value(e->frames.size());
      evNode.append_attribute("hash").set_value(e->GetHash());

      for (auto &f : e->frames) {
        pugi::xml_node frNode = evNode.append_child("frame");
        f.Save(frNode);
      }
    }
  }
}

void LMTAnimationEventV2_Internal::Load(pugi::xml_node node) {

  pugi::xml_node eventGroupsNode = node.child("eventGroups");

  if (eventGroupsNode.empty()) {
    throw XMLMissingNodeException("eventGroups", node);
  }

  pugi::xml_attribute hashAttr = eventGroupsNode.attribute("hash");

  if (hashAttr.empty()) {
    throw XMLMissingNodeAttributeException("hash", eventGroupsNode);
  }

  SetHash(hashAttr.as_uint());

  auto eventGroupNodes = XMLCollectChildren(eventGroupsNode, "eventGroup");

  for (auto &g : eventGroupNodes) {
    groups.push_back(LMTAnimationEventV2Group::Create());
    groups.back()->Load(g);
  }
}

void LMTTrack_internal::Load(pugi::xml_node node) {
  ReflectFromXML(node);

  if (UseTrackExtremes()) {
    ReflectorWrap<TrackMinMax> minMaxRelfl(minMax);
    ReflectorXMLUtil::Load(minMaxRelfl, node);
  }

  if (!CreateController()) {
    pugi::xml_node comChild = node.child("compression");
    throw XMLBaseException("Unknown compression type: " +
                               std::string(comChild.text().get()),
                           comChild);
  }

  pugi::xml_node dataNode = node.child("data");
  pugi::xml_attribute numDataItems = dataNode.attribute("numItems");

  if (dataNode.empty() || numDataItems.empty() || !numDataItems.as_int()) {
    return;
  }

  controller->NumFrames(numDataItems.as_int());
  std::string strBuff = dataNode.text().get();
  controller->FromString(strBuff);
}

void LMTTrack_internal::Save(pugi::xml_node node, bool standAlone) const {
  ReflectToXML(node);

  if (useMinMax) {
    ReflectorWrap<const TrackMinMax> refl(minMax);
    ReflectorXMLUtil::Save(refl, node);
  }

  pugi::xml_node dataNode = node.append_child("data");
  std::string strBuff;
  controller->ToString(strBuff, standAlone ? numIdents - 1 : numIdents);
  dataNode.append_buffer(strBuff.c_str(), strBuff.size());
  dataNode.append_attribute("numItems").set_value(controller->NumFrames());
}

void LMTAnimation_internal::Load(pugi::xml_node node) {
  ReflectFromXML(node);

  events = CreateEvents();
  events->Load(node);

  pugi::xml_node floatTracksNode = node.child("floatTracks");

  if (!floatTracksNode.empty()) {
    floatTracks = CreateFloatTracks();

    if (floatTracks) {
      floatTracks->Load(floatTracksNode);
    }
  }

  pugi::xml_node tracksNode = node.child("tracks");

  if (tracksNode.empty()) {
    throw XMLMissingNodeException("tracks", node);
  }

  auto trackNodes = XMLCollectChildren(tracksNode, "track");

  storage.reserve(trackNodes.size());

  for (auto &t : trackNodes) {
    storage.emplace_back(CreateTrack());
    storage.back()->Load(t);
  }
}

void LMTAnimation::Save(const std::string &fileName, bool asXML) const {
  if (asXML) {
    pugi::xml_document doc = {};
    Save(doc, true);
    XMLToFile(fileName, doc, {XMLFormatFlag::WriteBOM, XMLFormatFlag::Indent});
  } else {
    BinWritter wr(fileName);
    Save(wr);
  }
}

void LMTAnimation_internal::Save(pugi::xml_node node, bool standAlone) const {
  ReflectToXML(node);

  if (events) {
    events->Save(node, standAlone);
  }

  if (floatTracks) {
    floatTracks->Save(node, standAlone);
  }

  pugi::xml_node tracksNode = node.append_child("tracks");

  for (auto &t : storage) {
    pugi::xml_node trackNode = tracksNode.append_child("track");
    t->Save(trackNode, standAlone);
  }
}

void LMT::Save(pugi::xml_node node, es::string_view fileName,
               LMTExportSettings settings) const {
  pugi::xml_node master = node.append_child("LMT");
  master.append_attribute("version").set_value(static_cast<int>(Version()));
  const auto archtype =
      settings.arch == LMTArchType::Auto ? Architecture() : settings.arch;
  master.append_attribute("X64").set_value(archtype == LMTArchType::X64);

  size_t curAniID = 0;
  AFileInfo fleInf(fileName);

  for (auto &a : pi->storage) {
    pugi::xml_node cAni = master.append_child("Animation");
    cAni.append_attribute("ID").set_value(curAniID);

    if (a) {
      if (settings.type == LMTExportType::FullXML) {
        a->Save(cAni);
      } else if (settings.type == LMTExportType::LinkedXML) {
        pugi::xml_document linkAni = {};
        pugi::xml_node subAni = linkAni.append_child("Animation");
        subAni.append_attribute("version").set_value(
            static_cast<int>(Version()));
        a->Save(subAni, true);
        std::string linkedName = fleInf.GetFilename();
        linkedName += "_m" + std::to_string(curAniID) + ".mtx";
        std::string linkedFullName = fleInf.GetFolder();
        linkedFullName += linkedName;
        cAni.append_buffer(linkedName.c_str(), linkedName.size());
        XMLToFile(linkedFullName, linkAni,
                  {XMLFormatFlag::Indent, XMLFormatFlag::WriteBOM});
      } else if (settings.type == LMTExportType::BinaryLinkedXML) {
        std::string linkedName = fleInf.GetFilename();
        linkedName += "_m" + std::to_string(curAniID) + ".mti";
        std::string linkedFullName = fleInf.GetFilename();
        linkedFullName += linkedName;
        cAni.append_buffer(linkedName.c_str(), linkedName.size());
        a->Save(linkedFullName);
      }
    }
    curAniID++;
  }
}

void LMT::Save(const std::string &fileName, LMTExportSettings settings) const {
  if (settings.type == LMTExportType::FullBinary) {
    BinWritter wr(fileName);
    wr.SwapEndian(settings.swapEndian);
    Save(wr);
    return;
  }

  pugi::xml_document doc = {};
  Save(doc, fileName, settings);
  XMLToFile(fileName, doc, {XMLFormatFlag::Indent, XMLFormatFlag::WriteBOM});
}

void LMT::Load(const std::string &fileName, LMTImportOverrides overrides) {
  BinReader rd(fileName);

  try {
    Load(rd);
  } catch (const es::InvalidHeaderError &) {
    es::Dispose(rd);
    pugi::xml_document doc = XMLFromFile(fileName);
    Load(doc, fileName, overrides);
  }
}

void LMT::Load(pugi::xml_node node, es::string_view outPath,
               LMTImportOverrides overrides) {
  auto mainNodes = XMLCollectChildren(node, "LMT");

  if (mainNodes.empty()) {
    throw XMLMissingNodeException("LMT", node);
  }

  if (mainNodes.size() > 1) {
    XML_WARNING(node, "XML have too many root elements, only first processed.");
  }

  pugi::xml_node masterNode = mainNodes.front();
  pugi::xml_attribute versionAttr = masterNode.attribute("version");

  if (versionAttr.empty()) {
    throw XMLMissingNodeAttributeException("version", masterNode);
  }

  pugi::xml_attribute archAttr = masterNode.attribute("X64");

  if (archAttr.empty()) {
    throw XMLMissingNodeAttributeException("X64", masterNode);
  }

  pi->props.version = static_cast<LMTVersion>(versionAttr.as_int());

  if (overrides.arch == LMTArchType::Auto) {
    pi->props.arch = archAttr.as_bool() ? LMTArchType::X64 : LMTArchType::X86;
  } else {
    pi->props.arch = overrides.arch;
  }

  auto animationNodes = XMLCollectChildren(masterNode, "Animation");

  pi->storage.reserve(animationNodes.size());

  AFileInfo fleInf(outPath);

  for (auto &a : animationNodes) {
    pugi::xml_text nodeBuffer = a.text();

    if (nodeBuffer.empty()) {
      auto animationSubNodes = XMLCollectChildren(a);

      if (!animationSubNodes.size()) {
        pi->storage.emplace_back();
        continue;
      }

      auto cAni = LMTAnimation::Create(LMTConstructorProperties(pi->props));
      cAni->Load(a);
      pi->storage.emplace_back(uni::ToElement(cAni));
    } else {
      const char *path = nodeBuffer.get();
      std::string absolutePath = path;
      BinReader rd;

      try {
        rd.Open(absolutePath);
      } catch (const es::FileNotFoundError &) {
        absolutePath = fleInf.GetFilename();
        absolutePath += path;
        rd.Open(absolutePath);
      }

      LMTAnimation::Ptr cAni;
      try {
        cAni = LMTAnimation_internal::Load(rd, pi->props);
        pi->storage.emplace_back(uni::ToElement(cAni));
        continue;
      } catch (const es::InvalidHeaderError &) {
        cAni = LMTAnimation::Create(pi->props);
      }

      pugi::xml_document subAnim = XMLFromFile(absolutePath);
      auto subAniMainNodes = XMLCollectChildren(subAnim, "Animation");

      if (subAniMainNodes.empty()) {
        throw XMLMissingNodeException("Animation", subAnim);
      }

      pugi::xml_node subAniMainNode = subAniMainNodes[0];
      pugi::xml_attribute subVersionAttr = subAniMainNode.attribute("version");

      if (subVersionAttr.empty()) {
        throw XMLMissingNodeAttributeException("version", subAniMainNode);
      }

      const auto xmVersion = subVersionAttr.as_int();

      if (static_cast<LMTVersion>(xmVersion) != Version()) {
        throw es::InvalidVersionError(xmVersion);
      }

      cAni->Load(subAniMainNode);
      pi->storage.emplace_back(uni::ToElement(cAni));
    }
  }
}
