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
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "event.hpp"
#include "float_track.hpp"
#include "pugixml.hpp"
#include <algorithm>

#define _xml_comp_offset(_node)                                                \
  (_node.name() - (*_node.root().children().begin()).name() + 25)

#define XML_ERROR(node, ...)                                                   \
  printerror("[LMT XML: " << _xml_comp_offset(node) << "] " __VA_ARGS__);

#define XML_WARNING(node, ...)                                                 \
  printwarning("[LMT XML: " << _xml_comp_offset(node) << "] " __VA_ARGS__);

pugi::xml_node FloatFrame::ToXML(pugi::xml_node &node) const {
  pugi::xml_node currentNode = node.append_child("track");
  const uint32 numComponents = NumComponents();
  std::string resVal;

  for (uint32 c = 0; c < numComponents; c++)
    resVal.append(std::to_string(value[c]) + ", ");

  if (resVal.size()) {
    resVal.pop_back();
    resVal.pop_back();
    currentNode.append_buffer(resVal.c_str(), resVal.size());
  }

  currentNode.append_attribute("frame").set_value(Frame());
  currentNode.append_attribute("numItems").set_value(numComponents);

  return currentNode;
}

int FloatFrame::FromXML(pugi::xml_node &node) {
  pugi::xml_attribute cFrame = node.attribute("frame");

  if (cFrame.empty()) {
    XML_ERROR(node, "Missing <track frame=/>");
    return 1;
  }

  Frame(cFrame.as_int());

  if (node.text().empty()) {
    XML_ERROR(node, "Missing data for float track.");
    return 1;
  }

  pugi::xml_attribute numItemsAttr = node.attribute("numItems");

  uint32 numItems = 0;
  std::string nodeText = node.text().get();
  nodeText.push_back('\0');
  std::string outputText;

  for (auto &c : nodeText) {
    if ((c >= '0' && c <= '9') || c == '.')
      outputText.push_back(c);
    else if (c == ',' || c == '\0') {
      value[numItems] = static_cast<float>(atof(outputText.c_str()));
      outputText.clear();
      numItems++;
    }
  }

  NumComponents(numItems);

  if (!numItemsAttr.empty() && numItemsAttr.as_int() != numItems) {
    XML_WARNING(node, "<track numItems=/> differs from actual count.");
  }

  return 0;
}

int LMTFloatTrack_internal::ToXML(pugi::xml_node &node, bool standAlone) const {
  pugi::xml_node evGroupsNode = node.append_child("floatTracks");
  evGroupsNode.append_attribute("numItems").set_value(GetNumGroups());

  for (uint32 g = 0; g < GetNumGroups(); g++) {
    pugi::xml_node evGroupNode = evGroupsNode.append_child("floatTrack");

    _ToXML(evGroupNode, g);

    pugi::xml_node rangesNode = evGroupNode.append_child("tracks");

    uint32 numEvents = GetGroupTrackCount(g);

    rangesNode.append_attribute("numItems").set_value(numEvents);

    const FloatFrame *cEvents = GetFrames(g);
    const FloatFrame *const cEventsEnd = cEvents + numEvents;

    for (; cEvents != cEventsEnd; cEvents++)
      cEvents->ToXML(rangesNode);
  }

  return 0;
}

int LMTFloatTrack_internal::FromXML(pugi::xml_node &floatTracksNode) {
  auto floatTrackIter = floatTracksNode.children("floatTrack");
  std::vector<pugi::xml_node> floatTrackNodes(floatTrackIter.begin(),
                                              floatTrackIter.end());
  pugi::xml_attribute numItemsAttr = floatTracksNode.attribute("numItems");

  if (!numItemsAttr.empty() &&
      numItemsAttr.as_int() != floatTrackNodes.size()) {
    XML_WARNING(
        floatTracksNode,
        "<floatTracks numItems=/> differs from actual <floatTrack/> count.")
  }

  const uint32 numGroups = GetNumGroups();

  if (floatTrackNodes.size() != numGroups) {
    XML_WARNING(
        floatTracksNode,
        "<floatTrack/> count is different than built-in count. Expected: "
            << numGroups << ", got: " << floatTrackNodes.size())
  }

  for (uint32 e = 0; e < numGroups; e++) {
    pugi::xml_node flTrackNode = floatTrackNodes[e];

    _FromXML(flTrackNode, e);

    pugi::xml_node rangesNode = flTrackNode.child("tracks");

    if (rangesNode.empty()) {
      XML_ERROR(flTrackNode,
                "Couldn't find any <tracks/> nodes for floatTrack.")
      return 1;
    }

    numItemsAttr = rangesNode.attribute("numItems");

    auto rangeIter = rangesNode.children("track");
    std::vector<pugi::xml_node> rangeNodes(rangeIter.begin(), rangeIter.end());

    if (!rangeNodes.size())
      continue;

    if (!numItemsAttr.empty() && numItemsAttr.as_int() != rangeNodes.size()) {
      XML_WARNING(flTrackNode,
                  "<tracks numItems=/> differs from actual <track/> count.")
    }

    SetNumFrames(e, static_cast<uint32>(rangeNodes.size()));

    FloatFrame *cFrames = GetFrames(e);
    FloatFrame *const cFramesEnd = cFrames + rangeNodes.size();

    uint32 curEventTrigger = 0;

    for (; cFrames != cFramesEnd; cFrames++, curEventTrigger++)
      cFrames->FromXML(rangeNodes[curEventTrigger]);
  }
  return 0;
}

pugi::xml_node AnimEvent::ToXML(pugi::xml_node &node) const {
  uint32 numItems = 0;
  pugi::xml_node currentNode = node.append_child("event");

  if (runEventBit) {
    std::string resVal;

    for (uint32 b = 0; b < 32; b++)
      if (runEventBit & (1 << b)) {
        resVal += std::to_string(b) + ", ";
        numItems++;
      }

    resVal.pop_back();
    resVal.pop_back();

    currentNode.append_buffer(resVal.c_str(), resVal.size());
  }

  currentNode.append_attribute("additiveFrames").set_value(numFrames);
  currentNode.append_attribute("numItems").set_value(numItems);

  return currentNode;
}

int AnimEvent::FromXML(pugi::xml_node &node) {
  pugi::xml_attribute addFrames = node.attribute("additiveFrames");

  if (addFrames.empty()) {
    XML_ERROR(node, "Missing <event additiveFrames=/>");
    return 1;
  }

  numFrames = addFrames.as_int();

  if (node.text().empty())
    return 0;

  pugi::xml_attribute numItemsAttr = node.attribute("numItems");

  uint32 currentID = 0;
  uint32 currentChar = 0;
  uint32 numItems = 0;
  std::string nodeText = node.text().get();
  nodeText.push_back('\0');

  for (auto &c : nodeText) {
    if (c >= '0' && c <= '9') {
      currentID |= static_cast<uint32>(c) << ((3 - currentChar) * 8);
      currentChar++;
    } else if (c == ',' || c == '\0') {
      FByteswapper(currentID);
      runEventBit |= 1 << atoi(reinterpret_cast<char *>(&currentID));
      currentID = 0;
      currentChar = 0;
      numItems++;
    }
  }

  if (!numItemsAttr.empty() && numItemsAttr.as_int() != numItems) {
    XML_WARNING(node, "<event numItems=/> differs from actual count.");
  }

  return 0;
}

int LMTAnimationEventV1_Internal::ToXML(pugi::xml_node &node,
                                        bool standAlone) const {
  pugi::xml_node evGroupsNode = node.append_child("eventGroups");
  evGroupsNode.append_attribute("numItems").set_value(GetNumGroups());

  for (uint32 g = 0; g < GetNumGroups(); g++) {
    pugi::xml_node evGroupNode = evGroupsNode.append_child("eventGroup");

    _ToXML(evGroupNode, g);

    pugi::xml_node rangesNode = evGroupNode.append_child("events");

    uint32 numEvents = GetGroupEventCount(g);

    rangesNode.append_attribute("numItems").set_value(numEvents);

    const EventsCollection &cEvents = GetEvents(g);

    for (auto &e : cEvents)
      e.ToXML(rangesNode);
  }

  return 0;
}

int LMTAnimationEventV1_Internal::FromXML(pugi::xml_node &node) {
  pugi::xml_node eventGroupsNode = node.child("eventGroups");

  if (eventGroupsNode.empty()) {
    XML_ERROR(node, "Couldn't find <eventGroups/> for animation.");
    return 1;
  }

  auto eventGroupIter = eventGroupsNode.children("eventGroup");
  std::vector<pugi::xml_node> eventGroupNodes(eventGroupIter.begin(),
                                              eventGroupIter.end());
  pugi::xml_attribute numItemsAttr = eventGroupsNode.attribute("numItems");

  if (!numItemsAttr.empty() &&
      numItemsAttr.as_int() != eventGroupNodes.size()) {
    XML_WARNING(
        eventGroupsNode,
        "<eventGroups numItems=/> differs from actual <eventGroup/> count.");
  }

  const uint32 numGroups = GetNumGroups();

  if (eventGroupNodes.size() != numGroups) {
    XML_WARNING(
        eventGroupsNode,
        "<eventGroup/> count is different than built-in count. Expected: "
            << numGroups << ", got: " << eventGroupNodes.size())
  }

  for (uint32 e = 0; e < numGroups; e++) {
    pugi::xml_node evGroupNode = eventGroupNodes[e];

    _FromXML(evGroupNode, e);

    pugi::xml_node rangesNode = evGroupNode.child("events");

    if (rangesNode.empty()) {
      XML_ERROR(evGroupNode,
                "Couldn't find any <events/> nodes for eventGroup.");
      return 1;
    }

    numItemsAttr = rangesNode.attribute("numItems");

    auto rangeIter = rangesNode.children("event");
    std::vector<pugi::xml_node> rangeNodes(rangeIter.begin(), rangeIter.end());

    if (!rangeNodes.size()) {
      XML_ERROR(evGroupNode, "Couldn't find any <event/> nodes for events.");
      return 1;
    }

    if (!numItemsAttr.empty() && numItemsAttr.as_int() != rangeNodes.size()) {
      XML_WARNING(
          evGroupNode,
          "Animation <events numItems=/> differs from actual <event/> count.")
    }

    SetNumEvents(e, static_cast<uint32>(rangeNodes.size()));

    EventsCollection &cEvents = GetEvents(e);

    uint32 curEventTrigger = 0;

    for (auto &e : cEvents)
      e.FromXML(rangeNodes[curEventTrigger++]);
  }

  return 0;
}

int AnimEventFrameV2::ToXML(pugi::xml_node &node) const {
  ReflectorWrapConst<AnimEventFrameV2> reflFrame(this);
  ReflectorXMLUtil::Save(reflFrame, node);

  pugi::xml_node dNode = node.append_child("data");
  std::string obuff;
  obuff += '[';

  for (uint32 d = 0; d < 3; d++)
    if (dataType == EventFrameV2DataType::Float)
      obuff.append(std::to_string(fdata[d])) += ", ";
    else
      obuff.append(std::to_string(idata[d])) += ", ";

  obuff.pop_back();
  obuff.pop_back();
  obuff += ']';

  dNode.append_buffer(obuff.c_str(), obuff.size());

  return 0;
}

int AnimEventFrameV2::FromXML(pugi::xml_node &node) {
  ReflectorWrap<AnimEventFrameV2> reflFrame(this);
  ReflectorXMLUtil::Load(reflFrame, node);

  std::string valueCopy = node.child("data").text().as_string();
  size_t firstBrace = valueCopy.find_first_of('[');

  if (firstBrace == valueCopy.npos)
    firstBrace = 0;
  else
    firstBrace++;

  valueCopy = valueCopy.substr(firstBrace, valueCopy.find(']', firstBrace));

  std::replace(valueCopy.begin(), valueCopy.end(), '[', ' ');
  std::replace(valueCopy.begin(), valueCopy.end(), ']', ' ');

  size_t currentItem = 0;
  size_t lastItem = 0;

  for (size_t i = 0; i < valueCopy.size() + 1; i++) {
    if (valueCopy[i] == ',' || valueCopy[i] == '\0') {

      if (dataType == EventFrameV2DataType::Float)
        fdata[currentItem++] =
            static_cast<float>(atof(valueCopy.data() + lastItem));
      else
        idata[currentItem++] = atoi(valueCopy.data() + lastItem);

      lastItem = i + 1;
    }
  }

  return 0;
}

LMTAnimationEventV2Event *
LMTAnimationEventV2Event::FromXML(pugi::xml_node &node) {
  pugi::xml_attribute eHashAttr = node.attribute("hash");
  pugi::xml_attribute eNumFrames = node.attribute("numFrames");

  if (eHashAttr.empty()) {
    XML_ERROR(node, "Couldn't find hash attribute for <event>.");
    return nullptr;
  }

  LMTAnimationEventV2Event *cEvent = LMTAnimationEventV2Event::Create();
  cEvent->SetHash(eHashAttr.as_uint());

  auto eventFramesNodesIter = node.children("frame");
  std::vector<pugi::xml_node> eventFramesNodes(eventFramesNodesIter.begin(),
                                               eventFramesNodesIter.end());
  if (!eNumFrames.empty() && eNumFrames.as_int() != eventFramesNodes.size()) {
    XML_WARNING(node,
                "<event numFrames=/> differs from actual <frame/> count.");
  }

  cEvent->frames.resize(eventFramesNodes.size());

  for (size_t f = 0; f < eventFramesNodes.size(); f++)
    cEvent->frames[f].FromXML(eventFramesNodes[f]);

  return cEvent;
}

LMTAnimationEventV2Group *
LMTAnimationEventV2Group::FromXML(pugi::xml_node &node) {
  pugi::xml_attribute cHashAttr = node.attribute("hash");
  pugi::xml_attribute cNumEvents = node.attribute("numEvents");

  if (cHashAttr.empty()) {
    XML_ERROR(node, "Couldn't find hash attribute for <eventGroup>.");
    return nullptr;
  }

  LMTAnimationEventV2Group *cGroup = Create();
  cGroup->SetHash(cHashAttr.as_uint());

  auto eventNodesIter = node.children("event");
  std::vector<pugi::xml_node> eventNodes(eventNodesIter.begin(),
                                         eventNodesIter.end());
  if (!cNumEvents.empty() && cNumEvents.as_int() != eventNodes.size()) {
    XML_WARNING(node,
                "<eventGroup numEvents=/> differs from actual <event/> count.");
  }

  for (auto &e : eventNodes)
    cGroup->events.push_back(EventPtr(LMTAnimationEventV2Event::FromXML(e)));

  return cGroup;
}

int LMTAnimationEventV2_Internal::ToXML(pugi::xml_node &node,
                                        bool standAlone) const {
  pugi::xml_node evGroupsNode = node.append_child("eventGroups");
  evGroupsNode.append_attribute("numItems").set_value(GetNumGroups());
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
        f.ToXML(frNode);
      }
    }
  }

  return 0;
}

int LMTAnimationEventV2_Internal::FromXML(pugi::xml_node &node) {

  pugi::xml_node eventGroupsNode = node.child("eventGroups");

  if (eventGroupsNode.empty()) {
    XML_ERROR(node, "Couldn't find <eventGroups/> for animation.");
    return 1;
  }

  pugi::xml_attribute hashAttr = eventGroupsNode.attribute("hash");

  if (hashAttr.empty()) {
    XML_ERROR(eventGroupsNode,
              "Couldn't find hash attribute for <eventGroups>.");
    return 2;
  }

  SetHash(hashAttr.as_uint());

  auto eventGroupIter = eventGroupsNode.children("eventGroup");
  std::vector<pugi::xml_node> eventGroupNodes(eventGroupIter.begin(),
                                              eventGroupIter.end());
  pugi::xml_attribute numItemsAttr = eventGroupsNode.attribute("numItems");

  if (!numItemsAttr.empty() &&
      numItemsAttr.as_int() != eventGroupNodes.size()) {
    XML_WARNING(
        eventGroupsNode,
        "<eventGroups numItems=/> differs from actual <eventGroup/> count.");
  }

  for (auto &g : eventGroupNodes)
    groups.push_back(GroupPtr(LMTAnimationEventV2Group::FromXML(g)));

  return 0;
}

int LMTTrack_internal::FromXML(pugi::xml_node &node) {
  _FromXML(node);

  if (UseTrackExtremes()) {
    TrackMinMax *localMinMax = new TrackMinMax();
    ReflectorWrap<TrackMinMax> minMaxRelfl(localMinMax);
    pugi::xml_node minMaxNode = ReflectorXMLUtil::Load(minMaxRelfl, node);

    if (minMaxNode.empty())
      delete localMinMax;
    else
      minMax = MinMaxPtr(localMinMax);
  }

  if (!CreateController()) {
    pugi::xml_node comChild = node.child("compression");

    XML_ERROR(comChild,
              "Unknown track compression type: " << comChild.text().get());
    return 1;
  }

  pugi::xml_node dataNode = node.child("data");
  pugi::xml_attribute numDataItems = dataNode.attribute("numItems");

  if (dataNode.empty() || numDataItems.empty() || !numDataItems.as_int())
    return 0;

  controller->NumFrames(numDataItems.as_int());
  std::string strBuff = dataNode.text().get();
  controller->FromString(strBuff);

  return 0;
}

int LMTTrack_internal::ToXML(pugi::xml_node &node, bool standAlone) const {
  _ToXML(node);

  if (minMax) {
    ReflectorWrapConst<TrackMinMax> refl(minMax.get());
    ReflectorXMLUtil::Save(refl, node);
  }

  uint32 currentBufferOffset = 0;
  pugi::xml_node dataNode = node.append_child("data");
  std::string strBuff;
  controller->ToString(strBuff, standAlone ? numIdents - 1 : numIdents);
  dataNode.append_buffer(strBuff.c_str(), strBuff.size());
  dataNode.append_attribute("numItems").set_value(controller->NumFrames());

  return 0;
}

int LMTAnimation_internal::FromXML(pugi::xml_node &node) {
  _FromXML(node);

  events = CreateEvents();
  events->FromXML(node);

  pugi::xml_node floatTracksNode = node.child("floatTracks");

  if (!floatTracksNode.empty()) {
    floatTracks = CreateFloatTracks();

    if (floatTracks)
      floatTracks->FromXML(floatTracksNode);
  }

  pugi::xml_node tracksNode = node.child("tracks");

  if (tracksNode.empty()) {
    XML_ERROR(node, "Couldn't find <tracks/> for animation.")
    return 1;
  }

  auto trackNodesIter = tracksNode.children("track");
  std::vector<pugi::xml_node> trackNodes(trackNodesIter.begin(),
                                         trackNodesIter.end());
  pugi::xml_attribute numItemsAttr = tracksNode.attribute("numItems");

  if (!numItemsAttr.empty() && numItemsAttr.as_int() != trackNodes.size()) {
    XML_WARNING(node, "<tracks numItems=/> differs from actual "
                      "<track/> count.");
  }

  storage.reserve(trackNodes.size());

  for (auto &t : trackNodes) {
    storage.emplace_back(CreateTrack());
    (*(storage.end() - 1))->FromXML(t);
  }

  return 0;
}

int LMTAnimation_internal::ToXML(pugi::xml_node &node, bool standAlone) const {
  _ToXML(node);

  if (events)
    events->ToXML(node, standAlone);

  if (floatTracks)
    floatTracks->ToXML(node, standAlone);

  pugi::xml_node tracksNode = node.append_child("tracks");
  tracksNode.append_attribute("numItems").set_value(storage.size());

  for (auto &t : storage) {
    pugi::xml_node trackNode = tracksNode.append_child("track");
    t->ToXML(trackNode, standAlone);
  }

  return 0;
}

int LMT::ToXML(pugi::xml_node &node, const char *fileName,
               ExportSettings settings) {
  pugi::xml_node master = node.append_child("LMT");
  master.append_attribute("version").set_value(Version());
  master.append_attribute("numItems").set_value(storage.size());
  master.append_attribute("X64").set_value(GetArchitecture() == X64);

  uint32 curAniID = 0;
  AFileInfo fleInf(fileName);

  for (auto &a : storage) {
    pugi::xml_node cAni = master.append_child("Animation");
    cAni.append_attribute("ID").set_value(curAniID);

    if (a) {
      if (settings == ExportSetting_FullXML)
        a->ToXML(cAni);
      else if (settings == ExportSetting_FullXMLLinkedMotions) {
        pugi::xml_document linkAni = {};
        pugi::xml_node subAni = linkAni.append_child("Animation");
        subAni.append_attribute("version").set_value(Version());
        a->ToXML(subAni, true);
        std::string linkedName = fleInf.GetFilename();
        linkedName + "_m" + std::to_string(curAniID) + ".mtx";
        std::string linkedFullName = fleInf.GetFolder();
        linkedFullName += linkedName;
        cAni.append_buffer(linkedName.c_str(), linkedName.size());
        linkAni.save_file(linkedFullName.c_str(), "\t",
                          pugi::format_write_bom | pugi::format_indent);
      } else if (settings == ExportSetting_BinaryMotions) {
        std::string linkedName = fleInf.GetFilename();
        linkedName += "_m" + std::to_string(curAniID) + ".mti";
        std::string linkedFullName = fleInf.GetFilename();
        linkedFullName += linkedName;
        cAni.append_buffer(linkedName.c_str(), linkedName.size());
        a->Save(linkedFullName.c_str());
      }
    }
    curAniID++;
  }

  return 0;
}

int LMT::ToXML(const char *fileName, ExportSettings settings) {
  pugi::xml_document doc = {};
  ToXML(doc, fileName, settings);

  if (!doc.save_file(fileName, "\t",
                     pugi::format_write_bom | pugi::format_indent)) {
    printerror("[LMT] Couldn't save file: " << fileName);
    return 1;
  }

  return 0;
}

int LMT::FromXML(pugi::xml_node &node, const char *fileName,
                 Architecture forceArchitecture) {
  auto children = node.children("LMT");
  std::vector<pugi::xml_node> mainNodes(children.begin(), children.end());

  if (!mainNodes.size()) {
    XML_ERROR(node, "Couldn't find <LMT/>.");
    return 2;
  }

  if (mainNodes.size() > 1) {
    XML_WARNING(node, "XML have too many root elements, only first processed.");
  }

  pugi::xml_node masterNode = *children.begin();

  pugi::xml_attribute versionAttr = masterNode.attribute("version");

  if (versionAttr.empty()) {
    XML_ERROR(masterNode, "Missing <LMT version=/>");
    return 2;
  }

  pugi::xml_attribute archAttr = masterNode.attribute("X64");

  if (archAttr.empty()) {
    XML_ERROR(masterNode, "Missing <LMT X64=/>");
    return 2;
  }

  props.version = versionAttr.as_int();
  props.ptrSize |= (forceArchitecture == Xundefined && archAttr.as_bool()) ||
                           forceArchitecture == X64
                       ? 8
                       : 4;

  pugi::xml_attribute numItemsAttr = masterNode.attribute("numItems");

  auto animationIter = masterNode.children("Animation");
  std::vector<pugi::xml_node> animationNodes(animationIter.begin(),
                                             animationIter.end());

  if (!numItemsAttr.empty() && numItemsAttr.as_int() != animationNodes.size()) {
    XML_WARNING(masterNode,
                "<LMT numItems=/> differs from actual <Animation/> count.");
  }

  storage.reserve(animationNodes.size());

  AFileInfo fleInf(fileName);

  for (auto &a : animationNodes) {
    pugi::xml_text nodeBuffer = a.text();

    if (nodeBuffer.empty()) {
      auto animationSubNodesIter = a.children();
      std::vector<pugi::xml_node> animationSubNodes(
          animationSubNodesIter.begin(), animationSubNodesIter.end());

      if (!animationSubNodes.size()) {
        storage.emplace_back(nullptr);
        continue;
      }

      LMTAnimation *cAni =
          LMTAnimation::Create(LMTConstructorProperties(props));
      cAni->FromXML(a);
      storage.emplace_back(cAni);
    } else {
      const char *path = nodeBuffer.get();
      std::string absolutePath = path;
      BinReader rd(absolutePath);
      bool notMTMI = false;

      if (!rd.IsValid()) {
        absolutePath = fleInf.GetFilename();
        absolutePath += path;
        rd.Open(absolutePath);
        if (!rd.IsValid()) {
          XML_ERROR(a, "Couldn't load animation: " << absolutePath.c_str());
          storage.emplace_back(nullptr);
          continue;
        }
      }

      LMTAnimation_internal *cAni = nullptr;
      int errNo = LMTAnimation_internal::Load(rd, props, cAni);

      if (errNo == 1)
        notMTMI = true;
      else if (errNo == 2) {
        printerror("[LMT] Animation is empty: " << absolutePath.c_str());
        storage.emplace_back(nullptr);
        continue;
      } else {
        printerror(
            "[LMT] Layout errors in animation: " << absolutePath.c_str());

        LMTConstructorPropertiesBase errMsg =
            reinterpret_cast<LMTConstructorPropertiesBase &>(errNo);

        if (errMsg.version != props.version) {
          printerror("[LMT] Unexpected animation version: "
                     << errMsg.version << ", expected: " << props.version);
        }

        bool expectedX64Arch = props.ptrSize == 8;
        bool haveX64Arch = errMsg.ptrSize == 8;

        if (haveX64Arch != expectedX64Arch) {
          printerror("[LMT] Unexpected animation architecture: "
                     << (haveX64Arch ? "X64" : "X86")
                     << ", expected: " << (expectedX64Arch ? "X64" : "X86"));
        }

        storage.emplace_back(nullptr);
        continue;
      }

      if (!notMTMI) {
        storage.emplace_back(cAni);
        continue;
      } else
        cAni = static_cast<LMTAnimation_internal *>(
            LMTAnimation::Create(LMTConstructorProperties(props)));

      pugi::xml_document subAnim = {};
      pugi::xml_parse_result reslt = subAnim.load_file(absolutePath.c_str());

      if (!reslt) {
        printerror("[LMT] Couldn't load animation: "
                   << absolutePath.c_str() << ", "
                   << GetReflectedEnum<XMLError>()[reslt]
                   << " at offset: " << reslt.offset);
        delete cAni;
        storage.emplace_back(nullptr);
        continue;
      }

      auto subAnimChildren = subAnim.children("Animation");
      std::vector<pugi::xml_node> subAniMainNodes(subAnimChildren.begin(),
                                                  subAnimChildren.end());

      if (!subAniMainNodes.size()) {
        XML_ERROR(subAnim, "Couldn't find <Animation/>.");
        delete cAni;
        storage.emplace_back(nullptr);
        continue;
      }

      pugi::xml_node subAniMainNode = subAniMainNodes[0];
      pugi::xml_attribute subVersionAttr = subAniMainNode.attribute("version");

      if (subVersionAttr.empty()) {
        XML_ERROR(subAnim, "Missing <Animation version=/>");
        delete cAni;
        storage.emplace_back(nullptr);
        continue;
      }

      if (subVersionAttr.as_int() != Version()) {
        XML_ERROR(subAnim, "Unexpected animation version: "
                               << subVersionAttr.as_int()
                               << ", expected: " << Version());
        delete cAni;
        storage.emplace_back(nullptr);
        continue;
      }

      cAni->FromXML(subAniMainNode);
      storage.emplace_back(cAni);
    }
  }

  return 0;
}

int LMT::FromXML(const char *fileName, Architecture forceArchitecture) {
  pugi::xml_document doc = {};
  pugi::xml_parse_result reslt = doc.load_file(fileName);

  if (!reslt) {
    printerror("[LMT] Couldn't load xml file. "
               << GetReflectedEnum<XMLError>()[reslt]
               << " at offset: " << reslt.offset);
    return 1;
  }

  return FromXML(doc, fileName, forceArchitecture);
}
