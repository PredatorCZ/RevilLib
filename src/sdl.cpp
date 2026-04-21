/*  Revil Format Library
    Copyright(C) 2021-2026 Lukas Cone

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

#include "spike/crypto/crc32.hpp"
#define ES_COPYABLE_POINTER

#include "property.hpp"
#include "pugixml.hpp"
#include "revil/hashreg.hpp"
#include "revil/sdl.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/type/pointer.hpp"
#include "spike/type/vectors.hpp"
#include "spike/util/endian.hpp"
#include <array>
#include <cassert>
#include <sstream>

using namespace revil;

struct XFSClassMember;

MAKE_ENUM(ENUMSCOPE(class SDLTrackType1 : uint8, SDLTrackType1),
          EMEMBER(invalid_),          //
          EMEMBER(RootTrack),         //
          EMEMBER(UnitTrack),         //
          EMEMBER(SystemTrack),       //
          EMEMBER(Separator),         //
          EMEMBER(ParamTrack),        //
          EMEMBER(IntTrack),          //
          EMEMBER(VectorTrack),       //
          EMEMBER(FloatTrack),        //
          EMEMBER(BoolTrack),         //
          EMEMBER(RefTrack),          //
          EMEMBER(ResourceTrack),     //
          EMEMBER(StringTrack),       //
          EMEMBER(EventTrack),        //
          EMEMBER(HermiteCurveTrack), //
          // UNUSED BELOW
          EMEMBERVAL(LongTrack, 128), //
          EMEMBER(DoubleTrack),       //
          EMEMBER(MatrixTrack)        //

);

MAKE_ENUM(ENUMSCOPE(class SDLTrackType2 : uint8, SDLTrackType2),
          EMEMBER(invalid_),         //
          EMEMBER(RootTrack),        //
          EMEMBER(UnitTrack),        //
          EMEMBER(SystemTrack),      //
          EMEMBER(Separator),        //
          EMEMBER(ParamTrack),       //
          EMEMBER(IntTrack),         //
          EMEMBER(LongTrack),        //
          EMEMBER(VectorTrack),      //
          EMEMBER(FloatTrack),       //
          EMEMBER(DoubleTrack),      //
          EMEMBER(BoolTrack),        //
          EMEMBER(RefTrack),         //
          EMEMBER(ResourceTrack),    //
          EMEMBER(StringTrack),      //
          EMEMBER(EventTrack),       //
          EMEMBER(MatrixTrack),      //
          EMEMBER(HermiteCurveTrack) //
);

MAKE_ENUM(ENUMSCOPE(class SDLType2Legacy : uint8, SDLType2Legacy),
          EMEMBERVAL(RootNode, 1),          //
          EMEMBER(ClassNode),               //
          EMEMBERVAL(ClassMemberNode, 5),   //
          EMEMBER(Int32),                   //
          EMEMBERVAL(Vector4, 8),           //
          EMEMBER(Float),                   //
          EMEMBERVAL(Bool, 11),             //
          EMEMBER(NodeIndex),               //
          EMEMBERVAL(ResourceInstance, 13), //
          EMEMBER(String),                  //
          EMEMBER(Unit),                    //
          EMEMBER(Curve)                    //
);

struct MtHermiteCurve {
  float x[8];
  float y[8];
};

struct SDLFrame {
  using Frame = BitMemberDecl<0, 24>;
  using Mode = BitMemberDecl<1, 8>;
  using type = BitFieldType<uint32, Frame, Mode>;
  type data;

  const type *operator->() const { return &data; }
  type *operator->() { return &data; }
};

struct SDLHeaderBase {
  uint32 id;
  SDLVersion version;
  uint16 numTracks;
};

struct SDLEntryV1 {
  SDLTrackType1 trackType;
  MtPropertyType propertyType;
  uint16 numFrames;
  uint32 parentOrSlot;
  es::PointerX86<char> name;
  uint32 hashOrArrayIndex;
  es::PointerX86<SDLFrame> frames;
  es::PointerX86<char> data;
};

struct SDLHeaderV1 : SDLHeaderBase {
  SDLFrame maxFrame;
  uint32 baseTrack = 0;
  es::PointerX86<char> strings;
  SDLEntryV1 entries[];
};

struct SDLEntryV2_x64 {
  SDLTrackType2 trackType;
  MtPropertyType propertyType;
  uint16 numFrames;
  uint32 parentOrSlot;
  es::PointerX64<char> name;
  uint32 hashOrArrayIndex;
  uint64 unitGroup;
  es::PointerX64<SDLFrame> frames;
  es::PointerX64<char> data;
};

struct SDLHeaderV2_x64 : SDLHeaderBase {
  uint32 crc;
  SDLFrame maxFrame;
  uint32 baseTrack = 0;
  es::PointerX64<char> strings;
  SDLEntryV2_x64 entries[];
};

struct SDLHeaderV2_x86 : SDLHeaderBase {
  uint32 crc;
  SDLFrame maxFrame;
  uint32 baseTrack = 0;
  es::PointerX86<char> strings;
  SDLEntryV1 entries[];
};

static_assert(sizeof(SDLHeaderV2_x64) == 32);
static_assert(sizeof(SDLHeaderV2_x86) == 24);

template <> void FByteswapper(SDLEntryV1 &item, bool) {
  FByteswapper(item.parentOrSlot);
  FByteswapper(item.numFrames);
  FByteswapper(item.name);
  FByteswapper(item.hashOrArrayIndex);
  FByteswapper(item.frames);
  FByteswapper(item.data);
}

template <> void FByteswapper(SDLEntryV2_x64 &, bool) {
  // Not implemented
}

template <> void FByteswapper(SDLHeaderBase &item, bool) {
  FByteswapper(item.numTracks);
  FByteswapper(item.version);
}

template <> void FByteswapper(SDLHeaderV1 &item, bool way) {
  FByteswapper(static_cast<SDLHeaderBase &>(item));
  FByteswapper(item.baseTrack);
  FByteswapper(item.maxFrame.data, way);
  FByteswapper(item.strings);
}

template <> void FByteswapper(SDLHeaderV2_x86 &item, bool way) {
  FByteswapper(static_cast<SDLHeaderBase &>(item));
  FByteswapper(item.baseTrack);
  FByteswapper(item.maxFrame.data, way);
  FByteswapper(item.strings);
  FByteswapper(item.crc);
}

template <> void FByteswapper(SDLHeaderV2_x64 &, bool) {
  // Not implemented
}

template <class C> void SwapData(C &entry, bool way = false) {
  size_t numBlocks = 0;
  using EnumType = decltype(entry.trackType);

  switch (entry.trackType) {
  case EnumType::FloatTrack:
  case EnumType::IntTrack:
  case EnumType::RefTrack:
  case EnumType::ResourceTrack:
  case EnumType::StringTrack:
  case EnumType::EventTrack:
    numBlocks = 1;
    break;

  case EnumType::VectorTrack:
    numBlocks = 4;
    break;
  case EnumType::MatrixTrack:
  case EnumType::HermiteCurveTrack:
    numBlocks = 16;
    break;

  default:
    break;
  }

  const size_t numSwaps = numBlocks * entry.numFrames;
  char *dataRaw = entry.data;
  uint32 *data = reinterpret_cast<uint32 *>(dataRaw);

  for (size_t i = 0; i < numSwaps; i++) {
    FByteswapper(data[i]);
  }

  SDLFrame *frames = entry.frames;

  for (size_t i = 0; i < numSwaps; i++) {
    FByteswapper(frames[i].data, way);
  }
}

template <class C> C FromXMLAttr(pugi::xml_node node, const char *attrName) {
  auto attr = node.attribute(attrName);

  if (attr.empty()) {
    throw std::runtime_error("Cannot find attribute: " + std::string(attrName) +
                             " for node: " + node.name());
  }

  if constexpr (std::is_same_v<C, int32>) {
    return attr.as_int();
  } else if constexpr (std::is_same_v<C, uint32>) {
    return attr.as_uint();
  } else if constexpr (std::is_same_v<C, bool>) {
    return attr.as_bool();
  } else if constexpr (std::is_same_v<C, float>) {
    return attr.as_float();
  } else if constexpr (std::is_same_v<C, double>) {
    return attr.as_double();
  } else if constexpr (std::is_same_v<C, int64>) {
    return attr.as_llong();
  } else if constexpr (std::is_same_v<C, uint64>) {
    return attr.as_ullong();
  } else if constexpr (std::is_same_v<C, const char *>) {
    return attr.as_string();
  }
}

pugi::xml_node GetXMLNode(pugi::xml_node node, MtPropertyType type,
                          const char *propName) {
  auto subNode = node.find_child_by_attribute(PropType(type), "name", propName);

  if (subNode.empty()) {
    throw std::runtime_error(
        "Cannot find sub-node: <" + std::string(PropType(type)) + " name=\"" +
        std::string(propName) + "\" ...> for node: " + node.name());
  }

  return subNode;
}

template <class C>
C FromXMLProp(pugi::xml_node node, MtPropertyType type, const char *propName,
              const char *valueName = "value") {
  auto subNode = GetXMLNode(node, type, propName);
  auto attr = subNode.attribute(valueName);

  if (attr.empty()) {
    throw std::runtime_error(std::string("Cannot find " +
                                         std::string(valueName) +
                                         " attribute for node: ") +
                             subNode.name());
  }

  if constexpr (std::is_same_v<C, int32>) {
    return attr.as_int();
  } else if constexpr (std::is_same_v<C, uint32>) {
    return attr.as_uint();
  } else if constexpr (std::is_same_v<C, bool>) {
    return attr.as_bool();
  } else if constexpr (std::is_same_v<C, float>) {
    return attr.as_float();
  } else if constexpr (std::is_same_v<C, double>) {
    return attr.as_double();
  } else if constexpr (std::is_same_v<C, int64>) {
    return attr.as_llong();
  } else if constexpr (std::is_same_v<C, uint64>) {
    return attr.as_ullong();
  } else if constexpr (std::is_same_v<C, const char *>) {
    return attr.as_string();
  }
}

pugi::xml_node XMLChild(pugi::xml_node parentNode, const char *nodeName) {
  auto node = parentNode.child(nodeName);

  if (node.empty()) {
    throw std::runtime_error(
        "Cannot find child node: " + std::string(nodeName) +
        " for node: " + node.name());
  }

  return node;
}

void FromXMLLegacy(SDLFrame &frame, pugi::xml_node node) {
  frame->Set<SDLFrame::Frame>(FromXMLAttr<uint32>(node, "frame"));
  frame->Set<SDLFrame::Mode>(FromXMLAttr<uint32>(node, "frameFlags"));
}

SDLTrackType1 GetTrackTypeV1(std::string_view className) {
  static const auto refEnum = GetReflectedEnum<SDLTrackType1>();
  className.remove_prefix(className.find_last_of(':') + 1);

  for (size_t i = 0; i < refEnum->numMembers; ++i) {
    if (std::string_view(refEnum->names[i]) == className) {
      return SDLTrackType1(refEnum->values[i]);
    }
  }

  throw std::runtime_error("Unknown node type: " + std::string(className));

  return SDLTrackType1::RootTrack;
};

MtPropertyType GetPropertyType(std::string_view typeName) {
  static const auto refEnum = GetReflectedEnum<MtPropertyType>();

  for (size_t i = 0; i < refEnum->numMembers; ++i) {
    if (std::string_view(refEnum->names[i]) == typeName) {
      return MtPropertyType(refEnum->values[i]);
    }
  }

  throw std::runtime_error("Unknown property type: " + std::string(typeName));

  return MtPropertyType::invalid;
};

uint32 GetClassHashV1(std::string_view name) {
  if (name.starts_with("0x")) {
    name.remove_prefix(2);
    return strtoul(name.data(), nullptr, 16);
  } else {
    return MTHashV1(name);
  }
}

uint32 GetClassHashV2(std::string_view name) {
  if (name.starts_with("0x")) {
    name.remove_prefix(2);
    return strtoul(name.data(), nullptr, 16);
  } else {
    return MTHashV2(name);
  }
}

void FromXML(SDLEntryV1 &item, pugi::xml_node node,
             uint32 (*GetClassHash)(std::string_view name)) {
  std::string_view className = FromXMLAttr<const char *>(node, "type");
  item.trackType = GetTrackTypeV1(className);

  if (item.trackType >= SDLTrackType1::ParamTrack) {
    const char *propType =
        FromXMLProp<const char *>(node, MtPropertyType::string_, "mPropType");
    item.propertyType = GetPropertyType(propType);
    if (item.trackType > SDLTrackType1::ParamTrack) {
      item.numFrames = GetXMLNode(node, MtPropertyType::array, "mpKey")
                           .attribute("count")
                           .as_uint();
    }
  } else if (item.trackType == SDLTrackType1::UnitTrack) {
    std::string_view unitType =
        FromXMLProp<const char *>(node, MtPropertyType::string_, "UnitType");
    item.hashOrArrayIndex = GetClassHash(unitType);
    item.parentOrSlot =
        FromXMLProp<int32>(node, MtPropertyType::s32_, "MoveLine");
  } else if (item.trackType == SDLTrackType1::SystemTrack) {
    std::string_view systemType =
        FromXMLProp<const char *>(node, MtPropertyType::string_, "SystemType");
    item.hashOrArrayIndex = GetClassHash(systemType);
  }
}

void FromXML(SDLFrame &item, pugi::xml_node node) {
  item->Set<SDLFrame::Frame>(
      FromXMLProp<uint32>(node, MtPropertyType::u32_, "mFrame"));
  item->Set<SDLFrame::Mode>(
      FromXMLProp<uint32>(node, MtPropertyType::u32_, "mMode"));
}

struct PaddingRange {
  uint32 offset;
  uint32 size;
};

SDLTrackType2 GetSDLTypeV2Legacy(pugi::xml_node node) {
  static const auto refEnum = GetReflectedEnum<SDLType2Legacy>();

  for (size_t i = 0; i < refEnum->numMembers; ++i) {
    if (std::string_view(refEnum->names[i]) == node.name()) {
      return SDLTrackType2(refEnum->values[i]);
    }
  }

  throw std::runtime_error(std::string("Unknown node type: ") + node.name());

  return SDLTrackType2::RootTrack;
};

struct NodeRef {
  uint32 offset;
  std::vector<std::string_view> nodeNames;
};

struct StringPointer {
  uint32 offset;
  uint32 stringId;
};

struct DataBuilder {
  std::vector<PaddingRange> paddingRanges;
  std::vector<StringPointer> stringPointers;
  std::map<std::string, size_t> strings;
  std::stringstream sstr;
  BinWritterRef dataWr{sstr};
  std::vector<SDLEntryV1> itemsV1;
  std::vector<SDLEntryV2_x64> itemsV2;
  std::map<std::string_view, uint32> classNodes;
  std::vector<NodeRef> nodeRefs;
  bool firstFrame = true;
  uint32 baseTrack = 0;
  uint32 (*GetClassHash)(std::string_view name) = GetClassHashV1;

  uint32 GetString(auto str) {
    if (auto found = strings.find(str); found != strings.end()) {
      return found->second;
    } else {
      const size_t newId = strings.size();
      strings.emplace(str, newId);
      return newId;
    }
  }

  void SetString(auto &ptr, auto str) {
    if (auto found = strings.find(str); found != strings.end()) {
      ptr.Reset(found->second);
    } else {
      const size_t newId = strings.size();
      strings.emplace(str, newId);
      ptr.Reset(newId);
    }
  }

  void ProcessNodeTree(pugi::xml_node node, size_t parentIndex) {
    uint32 newParentIndex = itemsV1.size();
    SDLEntryV1 &entry = itemsV1.emplace_back();
    FromXML(entry, node, GetClassHash);
    const char *nodeName = nullptr;

    if (entry.trackType >= SDLTrackType1::ParamTrack) {
      entry.parentOrSlot = parentIndex;
      nodeName =
          FromXMLProp<const char *>(node, MtPropertyType::string_, "mPropName");
      SetString(entry.name, nodeName);

      if (nodeName == std::string_view("mCut")) {
        if (const uint32 hash = itemsV1.at(parentIndex).hashOrArrayIndex;
            hash == MTHashV1("uMotionCamera") ||
            hash == MTHashV2("uMotionCamera")) {
          baseTrack = newParentIndex;
        }
      }
    } else {
      nodeName = FromXMLProp<const char *>(node, MtPropertyType::string_,
                                           "mTrackName");
      SetString(entry.name, nodeName);
    }

    auto children = GetXMLNode(node, MtPropertyType::array, "Track");

    switch (entry.trackType) {
    case SDLTrackType1::RootTrack:
      newParentIndex = 0;
      [[fallthrough]];
    case SDLTrackType1::UnitTrack:
    case SDLTrackType1::SystemTrack:
    case SDLTrackType1::Separator:
      classNodes.emplace(nodeName, newParentIndex);
      [[fallthrough]];
    case SDLTrackType1::ParamTrack:

      for (auto &c : children) {
        ProcessNodeTree(c, newParentIndex);
      }
      return;

    default:
      break;
    }

    auto keys = GetXMLNode(node, MtPropertyType::array, "mpKey").children();

    auto SaveKeys = [&] {
      std::vector<SDLFrame> keyData;

      for (auto k : keys) {
        auto &nKey = keyData.emplace_back();
        FromXML(nKey, k);
      }

      size_t reqArea = keyData.size() * sizeof(SDLFrame);

      for (auto it = paddingRanges.begin(); it != paddingRanges.end(); it++) {
        if ((it->offset % alignof(SDLFrame)) == 0 && it->size >= reqArea) {
          dataWr.Push();
          dataWr.Seek(it->offset);
          entry.frames.Reset(it->offset);
          dataWr.WriteContainer(keyData);
          dataWr.Pop();

          if (it->size == reqArea) {
            paddingRanges.erase(it);
          } else {
            it->offset += reqArea;
            it->size -= reqArea;
          }
          return;
        }
      }

      dataWr.ApplyPadding(4);
      entry.frames.Reset(dataWr.Tell());
      dataWr.WriteContainer(keyData);
    };

    auto WriteValues = [&](auto value) {
      using value_type = decltype(value);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<value_type>(key, entry.propertyType, "mValue"));
      }
    };

    auto AlignData = [&](uint32 align) {
      const size_t padding = GetPadding(dataWr.Tell(), align);

      if (padding > 0) {
        paddingRanges.emplace_back(PaddingRange{
            .offset = uint32(dataWr.Tell()),
            .size = uint32(padding),
        });
        dataWr.Skip(padding);
      }

      entry.data.Reset(dataWr.Tell());
    };

    SaveKeys();

    switch (entry.propertyType) {
    case MtPropertyType::u8_:
    case MtPropertyType::u16_:
    case MtPropertyType::u32_:
    case MtPropertyType::event32:
    case MtPropertyType::event:
      AlignData(4);
      WriteValues(uint32{});
      break;
    case MtPropertyType::s8_:
    case MtPropertyType::s16_:
    case MtPropertyType::s32_:
      AlignData(4);
      WriteValues(int32{});
      break;
    case MtPropertyType::f32_:
      AlignData(4);
      WriteValues(float{});
      break;
    case MtPropertyType::bool_:
      AlignData(1);
      WriteValues(bool{});
      break;
    case MtPropertyType::classref: {
      AlignData(4);
      auto &nodeRef = nodeRefs.emplace_back();
      nodeRef.offset = dataWr.Tell();

      for (auto key : keys) {
        dataWr.Write<uint32>(0);
        nodeRef.nodeNames.emplace_back(
            FromXMLProp<const char *>(key, MtPropertyType::string_, "mValue"));
      }
      break;
    }
    case MtPropertyType::vector2:
    case MtPropertyType::float2: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y"));
        dataWr.Skip(8);
      }
      break;
    }
    case MtPropertyType::rangef: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "s"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "r"));
        dataWr.Skip(8);
      }
      break;
    }
    case MtPropertyType::easecurve: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "p1"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "p2"));
        dataWr.Skip(8);
      }
      break;
    }
    case MtPropertyType::vector3:
    case MtPropertyType::float3: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "z"));
        dataWr.Skip(4);
      }
      break;
    }
    case MtPropertyType::vector4:
    case MtPropertyType::float4:
    case MtPropertyType::quaternion: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "z"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "w"));
      }
      break;
    }
    case MtPropertyType::color: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "r"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "g"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "b"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "a"));
      }
      break;
    }
    case MtPropertyType::custom: {
      AlignData(4);
      for (auto key : keys) {
        std::string_view cType = FromXMLProp<const char *>(
            key, MtPropertyType::custom, "mValue", "ctype");

        if (cType != "resource") {
          throw es::RuntimeError("Invalid frame value ctype for custom "
                                 "property, resource expected");
        }

        std::string_view rType = FromXMLProp<const char *>(
            key, MtPropertyType::custom, "mValue", "rtype");
        const uint32 classHash = GetClassHash(rType);
        std::string_view path = FromXMLProp<const char *>(
            key, MtPropertyType::custom, "mValue", "path");

        if (path.empty()) {
          dataWr.Skip(sizeof(entry.data));
          continue;
        }

        std::string wString(sizeof(classHash), '-');
        wString.append(path);
        memcpy(wString.data(), &classHash, sizeof(classHash));
        stringPointers.emplace_back(StringPointer{
            .offset = uint32(dataWr.Tell()),
            .stringId = GetString(wString),
        });
        dataWr.Skip(sizeof(entry.data));
      }

      break;
    }

    case MtPropertyType::hermitecurve: {
      AlignData(16);
      for (auto key : keys) {
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x0"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x1"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x2"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x3"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x4"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x5"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x6"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "x7"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y0"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y1"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y2"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y3"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y4"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y5"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y6"));
        dataWr.Write(
            FromXMLProp<float>(key, entry.propertyType, "mValue", "y7"));
      }
      break;
    }
    case MtPropertyType::string_: {
      AlignData(4);
      for (auto key : keys) {
        std::string value =
            FromXMLProp<const char *>(key, MtPropertyType::string_, "mValue");

        if (value.empty()) {
          dataWr.Skip(sizeof(entry.data));
          continue;
        }

        stringPointers.emplace_back(StringPointer{
            .offset = uint32(dataWr.Tell()),
            .stringId = GetString(value),
        });
        dataWr.Skip(sizeof(entry.data));
      }

      break;
    }

    default:
      throw es::RuntimeError("Unhandled property type");
      break;
    }
  }

  void WriteValuesLegacy(pugi::xml_node entries, size_t parentIndex) {
    auto ResourceType = [](pugi::xml_node &node) -> uint32 {
      if (auto rType = node.attribute("resourceType"); !rType.empty()) {
        return MTHashV2(rType.as_string());
        // TODO validate class
      }

      auto hashText = FromXMLAttr<const char *>(node, "resourceHash");

      return strtoul(hashText, nullptr, 16);
    };

    auto DoValues = [&](pugi::xml_node &node, SDLEntryV2_x64 &entry) {
      auto SaveFrames = [&] {
        std::vector<SDLFrame> frames;

        for (auto frame : node.children("frame")) {
          ::FromXMLLegacy(frames.emplace_back(), frame);
        }

        entry.numFrames = frames.size();
        size_t reqArea = frames.size() * sizeof(SDLFrame);

        for (auto it = paddingRanges.begin(); it != paddingRanges.end(); it++) {
          if (it->size >= reqArea) {
            dataWr.Push();
            dataWr.Seek(it->offset);
            entry.frames = reinterpret_cast<SDLFrame *>(dataWr.Tell());
            dataWr.WriteContainer(frames);
            dataWr.Pop();

            if (it->size == reqArea) {
              paddingRanges.erase(it);
            } else {
              it->offset += reqArea;
              it->size -= reqArea;
            }
            return;
          }
        }

        entry.frames = reinterpret_cast<SDLFrame *>(dataWr.Tell());
        dataWr.WriteContainer(frames);
      };

      auto WriteValues = [&](auto value, const char *attrName = "value") {
        using value_type = decltype(value);
        for (auto frame : node.children("frame")) {
          dataWr.Write(FromXMLAttr<value_type>(frame, attrName));
        }
      };

      if (!firstFrame) {
        SaveFrames();
      }

      const size_t padding = GetPadding(dataWr.Tell(), 16);

      if (padding > 0) {
        paddingRanges.emplace_back(PaddingRange{
            .offset = uint32(dataWr.Tell()),
            .size = uint32(padding),
        });
        dataWr.Skip(padding);
      }

      entry.data = reinterpret_cast<char *>(dataWr.Tell());

      switch (entry.trackType) {
      case SDLTrackType2::FloatTrack:
        WriteValues(float());
        break;
      case SDLTrackType2::VectorTrack:
        for (auto frame : node.children("frame")) {
          dataWr.Write(FromXMLAttr<float>(frame, "x"));
          dataWr.Write(FromXMLAttr<float>(frame, "y"));
          dataWr.Write(FromXMLAttr<float>(frame, "z"));
          dataWr.Write(FromXMLAttr<float>(frame, "w"));
        }
        break;
      case SDLTrackType2::IntTrack:
      case SDLTrackType2::EventTrack:
        WriteValues(int32());
        break;
      case SDLTrackType2::RefTrack: {
        auto &nodeRef = nodeRefs.emplace_back();
        nodeRef.offset = dataWr.Tell();

        for (auto frame : node.children("frame")) {
          dataWr.Write<uint32>(0);
          nodeRef.nodeNames.emplace_back(
              FromXMLAttr<const char *>(frame, "nodeName"));
        }
        break;
      }
      case SDLTrackType2::BoolTrack:
        WriteValues(bool());
        dataWr.ApplyPadding(4);
        break;
      case SDLTrackType2::ResourceTrack:
        for (auto frame : node.children("frame")) {
          if (frame.attribute("path").empty()) {
            dataWr.Write<uintptr_t>(0);
            continue;
          }

          const uint32 cHash = ResourceType(frame);
          std::string wString(sizeof(cHash), '-');
          wString.append(FromXMLAttr<const char *>(frame, "path"));
          memcpy(wString.data(), &cHash, sizeof(cHash));
          es::PointerX86<char> ptr;
          SetString(ptr, wString);
          stringPointers.emplace_back(StringPointer{
              .offset = uint32(dataWr.Tell()),
              .stringId = reinterpret_cast<uint32 &>(ptr),
          });
          dataWr.Write(ptr);
        }
        break;
      case SDLTrackType2::StringTrack:
        for (auto frame : node.children("frame")) {
          es::PointerX86<char> ptr;
          SetString(ptr, FromXMLAttr<const char *>(frame, "value"));
          stringPointers.emplace_back(StringPointer{
              .offset = uint32(dataWr.Tell()),
              .stringId = reinterpret_cast<uint32 &>(ptr),
          });
          dataWr.Write(ptr);
        }
        break;
      case SDLTrackType2::MatrixTrack:
        for (auto frame : node.children("frame")) {
          for (size_t i = 0; i < 16; i++) {
            auto aName = "e" + std::to_string(i);
            dataWr.Write(FromXMLAttr<float>(frame, aName.c_str()));
          }
        }
        break;

      default:
        break;
      }

      if (firstFrame) {
        firstFrame = false;
        SaveFrames();
      }
    };

    for (auto c : entries.children()) {
      SDLEntryV2_x64 entry{};
      entry.trackType = GetSDLTypeV2Legacy(c);
      entry.propertyType = MtPropertyType(FromXMLAttr<uint32>(c, "type"));
      auto nodeName = FromXMLAttr<const char *>(c, "name");
      SetString(entry.name, nodeName);

      switch (entry.trackType) {
      case SDLTrackType2::RootTrack:
        classNodes.emplace(nodeName, itemsV2.size());
        itemsV2.emplace_back(entry);
        WriteValuesLegacy(c, 0);
        break;

      case SDLTrackType2::UnitTrack:
        entry.hashOrArrayIndex = ResourceType(c);
        entry.parentOrSlot = FromXMLAttr<uint32>(c, "entrySlot");
        classNodes.emplace(nodeName, itemsV2.size());
        itemsV2.emplace_back(entry);
        WriteValuesLegacy(c, itemsV2.size() - 1);
        break;

      case SDLTrackType2::ParamTrack:
        entry.hashOrArrayIndex = FromXMLAttr<uint32>(c, "arrayIndex");
        entry.parentOrSlot = parentIndex;
        itemsV2.emplace_back(entry);
        WriteValuesLegacy(c, itemsV2.size() - 1);
        break;

      default:
        entry.hashOrArrayIndex = FromXMLAttr<uint32>(c, "arrayIndex");
        entry.parentOrSlot = parentIndex;
        DoValues(c, entry);
        itemsV2.emplace_back(entry);
        break;
      }
    }
  }
};

constexpr uint32 SDL_ID = CompileFourCC("SDL");
constexpr uint32 SDL_ID_BE = CompileFourCC("\0LDS");

void SDLFromXml(pugi::xml_node rootNode, SDLFrame &maxFrame,
                DataBuilder &dataBuilder) {
  auto classNode = XMLChild(rootNode, "class");
  const char *typeName = FromXMLAttr<const char *>(classNode, "type");

  if (std::string_view("rScheduler") == typeName) {
    ::FromXMLLegacy(maxFrame, XMLChild(classNode, "maxFrame"));
    auto entries = XMLChild(classNode, "entries");
    dataBuilder.WriteValuesLegacy(entries, 0);
  } else if (std::string_view("rSchedulerXml") == typeName) {
    auto rootTrack =
        GetXMLNode(classNode, MtPropertyType::class_, "mRootTrack");
    maxFrame->Set<SDLFrame::Frame>(
        FromXMLProp<uint32>(rootTrack, MtPropertyType::u32_, "mFrameMax"));
    maxFrame->Set<SDLFrame::Mode>(
        FromXMLProp<bool>(rootTrack, MtPropertyType::bool_, "mFloorFrame"));
    dataBuilder.ProcessNodeTree(rootTrack, -1);
  } else {
    throw es::RuntimeError("Invalid class type, expected rScheduler");
  }
}

template <class HeaderType>
void SDLFromXMLV2(BinWritterRef wr, pugi::xml_node rootNode,
                  SDLVersion version) {
  HeaderType hdr;
  hdr.id = SDL_ID;
  hdr.version = version;
  hdr.crc = 0xE2316427;
  DataBuilder dataBuilder;
  SDLFromXml(rootNode, hdr.maxFrame, dataBuilder);

  hdr.baseTrack = dataBuilder.baseTrack;
  hdr.numTracks = dataBuilder.itemsV2.size();
  wr.Write(hdr);
  wr.WriteContainer(dataBuilder.itemsV2);
  const size_t dataOffset = wr.Tell();
  std::string dataBuffer(std::move(dataBuilder.sstr).str());
  wr.WriteContainer(dataBuffer);
  es::Dispose(dataBuffer);
  uintptr_t stringBegin = wr.Tell();
  hdr.strings = reinterpret_cast<char *>(stringBegin);
  std::vector<uintptr_t> stringOffsets;
  std::vector<std::string_view> strings;
  strings.resize(dataBuilder.strings.size());

  for (auto &[str, id] : dataBuilder.strings) {
    strings.at(id) = str.c_str();
  }

  for (size_t i = 0; i < strings.size(); i++) {
    stringOffsets.emplace_back(wr.Tell() - stringBegin);
    wr.WriteT(strings.at(i));
  }

  for (auto &e : dataBuilder.itemsV2) {
    e.name = reinterpret_cast<char *>(
        stringOffsets.at(reinterpret_cast<uintptr_t &>(e.name)));

    if (e.numFrames > 0) {
      e.data.FixupRelative(reinterpret_cast<char *>(dataOffset));
      e.frames.FixupRelative(reinterpret_cast<char *>(dataOffset));
    }
  }

  for (auto p : dataBuilder.stringPointers) {
    wr.Seek(dataOffset + p.offset);
    wr.Write(stringOffsets.at(p.stringId));
  }

  for (auto r : dataBuilder.nodeRefs) {
    wr.Seek(dataOffset + r.offset);

    for (auto &n : r.nodeNames) {
      wr.Write(dataBuilder.classNodes.at(n));
    }
  }

  wr.Seek(0);
  wr.Write(hdr);
  wr.WriteContainer(dataBuilder.itemsV2);
}

template <class HeaderType>
void SDLFromXMLV1(BinWritterRef wr, pugi::xml_node rootNode,
                  SDLVersion version) {
  HeaderType hdr;
  hdr.id = SDL_ID;
  hdr.version = version;
  DataBuilder dataBuilder;

  if (hdr.version > SDLVersion::V_16) {
    dataBuilder.GetClassHash = GetClassHashV2;
  }

  SDLFromXml(rootNode, hdr.maxFrame, dataBuilder);

  hdr.baseTrack = dataBuilder.baseTrack;
  hdr.numTracks = dataBuilder.itemsV1.size();
  wr.Write(hdr);
  wr.WriteContainer(dataBuilder.itemsV1);
  wr.ApplyPadding();
  const size_t dataOffset = wr.Tell();
  std::string dataBuffer(std::move(dataBuilder.sstr).str());
  wr.WriteContainer(dataBuffer);
  es::Dispose(dataBuffer);
  uint32 stringBegin = wr.Tell();
  hdr.strings.Reset(stringBegin);
  std::vector<uint32> stringOffsets;
  std::vector<std::string_view> strings;
  strings.resize(dataBuilder.strings.size());

  for (auto &[str, id] : dataBuilder.strings) {
    strings.at(id) = str;
  }

  for (size_t i = 0; i < strings.size(); i++) {
    stringOffsets.emplace_back(wr.Tell() - stringBegin);
    wr.WriteT(strings.at(i));
  }

  for (auto &e : dataBuilder.itemsV1) {
    e.name.Reset(stringOffsets.at(reinterpret_cast<uint32 &>(e.name)));

    if (e.numFrames > 0) {
      e.data.Reset(dataOffset + e.data.RawValue());
      e.frames.Reset(dataOffset + e.frames.RawValue());
    }
  }

  for (auto p : dataBuilder.stringPointers) {
    wr.Seek(dataOffset + p.offset);
    wr.Write(stringOffsets.at(p.stringId));
  }

  for (auto r : dataBuilder.nodeRefs) {
    wr.Seek(dataOffset + r.offset);

    for (auto &n : r.nodeNames) {
      wr.Write(dataBuilder.classNodes.at(n));
    }
  }

  wr.Seek(0);
  wr.Write(hdr);
  wr.WriteContainer(dataBuilder.itemsV1);
}

void revil::SDLFromXML(BinWritterRef wr, pugi::xml_node rootNode,
                       SDLVersion version) {
  if (version == SDLVersion::V_16) {
    SDLFromXMLV2<SDLHeaderV2_x64>(wr, rootNode, version);
  } else if (version < SDLVersion::V_20) {
    SDLFromXMLV1<SDLHeaderV1>(wr, rootNode, version);
  } else {
    SDLFromXMLV1<SDLHeaderV2_x86>(wr, rootNode, version);
  }
}

template <class EnumType>
pugi::xml_node NewTrack(EnumType type, pugi::xml_node &parent) {
  static const auto refEnum = GetReflectedEnum<EnumType>();
  std::string typeName = [&] {
    const size_t numEns = refEnum->numMembers;

    for (size_t i = 0; i < numEns; i++) {
      if (refEnum->values[i] == static_cast<uint64>(type)) {
        return refEnum->names[i];
      }
    }

    return "__UNREGISTERED__";
  }();

  pugi::xml_node node;

  switch (type) {
  case EnumType::RootTrack:
    node = parent.append_child("class");
    node.append_attribute("name").set_value("mRootTrack");
    break;

  default:
    node = parent.append_child("classref");
    break;
  }

  typeName = "rSchedulerXml::" + typeName;
  node.append_attribute("type").set_value(typeName.c_str());
  return node;
};

pugi::xml_node SetupTrack(pugi::xml_node &node, const char *name, uint32 id,
                          uint16 numFrames, MtPropertyType propType,
                          uint32 classHash, int type) {
  node.append_attribute("id").set_value(id);

  if (type < 5) {
    pugi::xml_node trackName = NewProperty(MtPropertyType::string_, node);
    trackName.append_attribute("name").set_value("mTrackName");
    trackName.append_attribute("value").set_value(name);
  }

  pugi::xml_node trackTrack = NewProperty(MtPropertyType::array, node);
  trackTrack.append_attribute("name").set_value("Track");
  trackTrack.append_attribute("type").set_value("classref");
  trackTrack.append_attribute("count").set_value(0);

  if (type < 2) {
    return {};
  }

  pugi::xml_node trackType = NewProperty(MtPropertyType::string_, node);

  auto SetClassName = [classHash, name, type](pugi::xml_node &node) {
    auto clName = GetClassName(classHash);

    if (clName.empty()) {
      char buffer[0x10]{};
      snprintf(buffer, sizeof(buffer), "0x%X", classHash);
      node.append_attribute("value").set_value(buffer);
      PrintError(type, " ", buffer, " Unknown class: ", name);
    } else {
      std::string resNme(clName);
      // PrintInfo(clName);
      node.append_attribute("value").set_value(resNme.c_str());
    }
  };

  switch (type) {
  case 2:
    trackType.append_attribute("name").set_value("UnitType");
    SetClassName(trackType);
    break;
  case 3:
    trackType.append_attribute("name").set_value("SystemType");
    SetClassName(trackType);
    break;
  case 4:
    node.remove_child(trackType);
    break;

  default: {
    trackType.append_attribute("name").set_value("mPropName");
    trackType.append_attribute("value").set_value(name);

    pugi::xml_node propNode = NewProperty(MtPropertyType::string_, node);
    propNode.append_attribute("name").set_value("mPropType");
    propNode.append_attribute("value").set_value(PropType(propType));

    if (type > 5) {
      pugi::xml_node keys = NewProperty(MtPropertyType::array, node);
      keys.append_attribute("name").set_value("mpKey");
      keys.append_attribute("type").set_value("classref");
      keys.append_attribute("count").set_value(numFrames);

      return keys;
    }
  }
  }

  return {};
}

auto MakePair(std::string_view name) {
  static uint32 prev = 0xFFFFFFFF;
  prev = ~crc32b(~prev, name.data(), name.size());
  return std::make_pair(prev, name);
}

static const std::map<uint32, std::string_view> LINE_NAMES{
    MakePair("Line0"),  MakePair("Line1"),     MakePair("Line2"),
    MakePair("Line3"),  MakePair("Line4"),     MakePair("Line5"),
    MakePair("Line6"),  MakePair("Line7"),     MakePair("Line8"),
    MakePair("Line9"),  MakePair("Line10"),    MakePair("Line11"),
    MakePair("Line12"), MakePair("Line13"),    MakePair("Line14"),
    MakePair("Line15"), MakePair("Line16"),    MakePair("Line17"),
    MakePair("Line18"), MakePair("Line19"),    MakePair("Line20"),
    MakePair("Line21"), MakePair("Line22"),    MakePair("Line23"),
    MakePair("Line24"), MakePair("Line25"),    MakePair("Line26"),
    MakePair("Line27"), MakePair("Line28"),    MakePair("Line29"),
    MakePair("Line30"), MakePair("Line31"),    MakePair("Line32"),
    MakePair("Line33"), MakePair("Line34"),    MakePair("Line35"),
    MakePair("Line36"), MakePair("Line37"),    MakePair("Line38"),
    MakePair("Line39"), MakePair("Line40"),    MakePair("Line41"),
    MakePair("Line42"), MakePair("Line43"),    MakePair("Line44"),
    MakePair("Line45"), MakePair("Line46"),    MakePair("Line47"),
    MakePair("Line48"), MakePair("Line49"),    MakePair("Line50"),
    MakePair("Line51"), MakePair("Line52"),    MakePair("Line53"),
    MakePair("Line54"), MakePair("Line55"),    MakePair("Line56"),
    MakePair("Line57"), MakePair("Line58"),    MakePair("Line59"),
    MakePair("Line60"), MakePair("Line61"),    MakePair("Line62"),
    MakePair("Line63"), MakePair("Undefined"),
};

template <class HdrType> void ToXML(HdrType *hdr, pugi::xml_node root) {
  using EntryType = std::decay_t<decltype(hdr->entries[0])>;
  using EnumType = decltype(EntryType::trackType);
  using PtrTypeChar = decltype(hdr->strings);
  using PtrTypeUint =
      std::conditional_t<sizeof(PtrTypeChar) == 8, es::PointerX64<uint32>,
                         es::PointerX86<uint32>>;

  if constexpr (std::is_same_v<HdrType, SDLHeaderV2_x86>) {
    // pugi::xml_node lineName = NewProperty(MtPropertyType::string_, root);
    // lineName.append_attribute("name").set_value("mLineName");
    // lineName.append_attribute("value").set_value(
    //     LINE_NAMES.at(hdr->crc).data());
    //assert(hdr->crc == 3843825908);
  }

  std::vector<pugi::xml_node> nodes;
  uint32 curId = 1;

  for (size_t i = 0; i < hdr->numTracks; i++) {
    auto &entry = hdr->entries[i];
    pugi::xml_node trackNode;

    switch (entry.trackType) {
    case EnumType::RootTrack: {
      trackNode = NewTrack(entry.trackType, root);

      pugi::xml_node frameMax = NewProperty(MtPropertyType::u32_, trackNode);
      frameMax.append_attribute("name").set_value("mFrameMax");
      frameMax.append_attribute("value").set_value(
          hdr->maxFrame->template Get<SDLFrame::Frame>());

      pugi::xml_node floorFrame = NewProperty(MtPropertyType::bool_, trackNode);
      floorFrame.append_attribute("name").set_value("mFloorFrame");
      floorFrame.append_attribute("value").set_value(
          bool(hdr->maxFrame->template Get<SDLFrame::Mode>()));

      break;
    }
    case EnumType::UnitTrack: {
      pugi::xml_node parent =
          nodes.front().find_child_by_attribute("array", "name", "Track");
      pugi::xml_attribute countAttr = parent.attribute("count");
      countAttr.set_value(countAttr.as_uint(0) + 1);
      trackNode = NewTrack(entry.trackType, parent);
      pugi::xml_node moveLine = NewProperty(MtPropertyType::s32_, trackNode);
      moveLine.append_attribute("name").set_value("MoveLine");
      moveLine.append_attribute("value").set_value(entry.parentOrSlot);
      break;
    }
    case EnumType::Separator:
    case EnumType::SystemTrack: {
      pugi::xml_node parent =
          nodes.front().find_child_by_attribute("array", "name", "Track");
      pugi::xml_attribute countAttr = parent.attribute("count");
      countAttr.set_value(countAttr.as_uint(0) + 1);
      trackNode = NewTrack(entry.trackType, parent);
      break;
    }
    default: {
      pugi::xml_node parent =
          nodes.at(entry.parentOrSlot)
              .find_child_by_attribute("array", "name", "Track");
      pugi::xml_attribute countAttr = parent.attribute("count");
      countAttr.set_value(countAttr.as_uint(0) + 1);
      trackNode = NewTrack(entry.trackType, parent);
      break;
    }
    }

    pugi::xml_node keys = SetupTrack(
        trackNode, entry.name, curId++, entry.numFrames, entry.propertyType,
        entry.hashOrArrayIndex, int(entry.trackType));

    if (entry.trackType == EnumType::ResourceTrack) {
      // for some reason it's hermitecurve
      trackNode.find_child_by_attribute("string", "name", "mPropType")
          .attribute("value")
          .set_value("custom");
    } else if (entry.trackType == EnumType::RefTrack && entry.propertyType == MtPropertyType::invalid) {
      // RE5: sometimes is invalid
      trackNode.find_child_by_attribute("string", "name", "mPropType")
          .attribute("value")
          .set_value("classref");
    } else if (entry.trackType == EnumType::BoolTrack && entry.propertyType == MtPropertyType::invalid) {
      // LP2: sometimes is invalid
      entry.propertyType = MtPropertyType::bool_;
      trackNode.find_child_by_attribute("string", "name", "mPropType")
          .attribute("value")
          .set_value("bool");
    }

    // trackNode.append_attribute("nodeid").set_value(nodes.size());
    nodes.emplace_back(trackNode);

    SDLFrame *frames = entry.frames;
    std::string keyTypeName(trackNode.attribute("type").as_string());
    keyTypeName.append("::Key");

    for (uint16 f = 0; f < entry.numFrames; f++) {
      SDLFrame frame = frames[f];

      pugi::xml_node keyNode = NewProperty(MtPropertyType::classref, keys);
      keyNode.append_attribute("type").set_value(keyTypeName.c_str());
      keyNode.append_attribute("id").set_value(curId++);

      pugi::xml_node frameNode = NewProperty(MtPropertyType::u32_, keyNode);
      frameNode.append_attribute("name").set_value("mFrame");
      frameNode.append_attribute("value").set_value(
          frame->Get<SDLFrame::Frame>());

      pugi::xml_node modeNode = NewProperty(MtPropertyType::u32_, keyNode);
      modeNode.append_attribute("name").set_value("mMode");
      modeNode.append_attribute("value").set_value(
          frame->Get<SDLFrame::Mode>());

      pugi::xml_node valueNode = NewProperty(entry.propertyType, keyNode);
      valueNode.append_attribute("name").set_value("mValue");

      switch (entry.trackType) {
      case EnumType::IntTrack: {
        const int32 value =
            reinterpret_cast<int32 *>(static_cast<char *>(entry.data))[f];

        switch (entry.propertyType) {
        case MtPropertyType::s32_:
        case MtPropertyType::s16_:
        case MtPropertyType::s8_:
          valueNode.append_attribute("value").set_value(value);
          break;
        case MtPropertyType::u32_:
        case MtPropertyType::u16_:
        case MtPropertyType::u8_:
          valueNode.append_attribute("value").set_value(uint32(value));
          break;

        case MtPropertyType::bool_:
          valueNode.append_attribute("value").set_value(bool(value));
          break;

        default:
          PrintError("Unknown property type ", PropType(entry.propertyType),
                     " for track type ", int(entry.trackType));
          break;
        }

        break;
      }

      case EnumType::VectorTrack: {
        auto &value =
            reinterpret_cast<Vector4 *>(static_cast<char *>(entry.data))[f];

        switch (entry.propertyType) {
        case MtPropertyType::vector4:
        case MtPropertyType::quaternion:
        case MtPropertyType::float4:
          valueNode.append_attribute("x").set_value(value.x);
          valueNode.append_attribute("y").set_value(value.y);
          valueNode.append_attribute("z").set_value(value.z);
          valueNode.append_attribute("w").set_value(value.w);
          break;
        case MtPropertyType::vector3:
        case MtPropertyType::float3:
          valueNode.append_attribute("x").set_value(value.x);
          valueNode.append_attribute("y").set_value(value.y);
          valueNode.append_attribute("z").set_value(value.z);
          break;
        case MtPropertyType::color:
          valueNode.append_attribute("r").set_value(value.x);
          valueNode.append_attribute("g").set_value(value.y);
          valueNode.append_attribute("b").set_value(value.z);
          valueNode.append_attribute("a").set_value(value.w);
          break;
        case MtPropertyType::easecurve:
          valueNode.append_attribute("p1").set_value(value.x);
          valueNode.append_attribute("p2").set_value(value.y);
          break;
        case MtPropertyType::float2:
        case MtPropertyType::vector2:
          valueNode.append_attribute("x").set_value(value.x);
          valueNode.append_attribute("y").set_value(value.y);
          break;
        case MtPropertyType::rangef:
          valueNode.append_attribute("s").set_value(value.x);
          valueNode.append_attribute("r").set_value(value.y);
          break;
        default:
          PrintError("Unknown property type ", PropType(entry.propertyType),
                     " for track type ", int(entry.trackType));
          break;
        }
        break;

        break;
      }

      case EnumType::FloatTrack: {
        const float value =
            reinterpret_cast<float *>(static_cast<char *>(entry.data))[f];

        switch (entry.propertyType) {
        case MtPropertyType::f32_:
          valueNode.append_attribute("value").set_value(value);
          break;

        default:
          PrintError("Unknown property type ", PropType(entry.propertyType),
                     " for track type ", int(entry.trackType));
          break;
        }

        break;
      }

      case EnumType::BoolTrack: {
        const bool value =
            reinterpret_cast<bool *>(static_cast<char *>(entry.data))[f];

        switch (entry.propertyType) {
        case MtPropertyType::bool_:
          valueNode.append_attribute("value").set_value(value);
          break;

        default:
          PrintError("Unknown property type ", PropType(entry.propertyType),
                     " for track type ", int(entry.trackType));
          break;
        }

        break;
      }

      case EnumType::ResourceTrack: {
        auto &dataPtr =
            reinterpret_cast<PtrTypeUint *>(static_cast<char *>(entry.data))[f];
        valueNode.append_attribute("ctype").set_value("resource");
        valueNode.set_name("custom");

        if (dataPtr) {
          auto clName = GetClassName(*dataPtr);

          if (clName.empty()) {
            char buffer[0x10]{};
            snprintf(buffer, sizeof(buffer), "0x%X", *dataPtr);
            valueNode.append_attribute("rtype").set_value(buffer);
            PrintError("Unknown resource class: ", *dataPtr);
          } else {
            std::string resNme(clName);
            valueNode.append_attribute("rtype").set_value(resNme.c_str());
          }

          valueNode.append_attribute("path").set_value(
              reinterpret_cast<const char *>(dataPtr.operator->() + 1));
        } else {
          valueNode.append_attribute("rtype").set_value("null");
          valueNode.append_attribute("path");
        }
        break;
      }

      case EnumType::RefTrack: {
        const char *value =
            hdr->entries[reinterpret_cast<uint32 *>(
                             static_cast<char *>(entry.data))[f]]
                .name;
        valueNode.append_attribute("value").set_value(value);
        valueNode.set_name("string");
        break;
      }

      case EnumType::EventTrack: {
        const uint32 value =
            reinterpret_cast<uint32 *>(static_cast<char *>(entry.data))[f];
        valueNode.append_attribute("value").set_value(value);
        break;
      }

      case EnumType::HermiteCurveTrack: {
        auto &value = reinterpret_cast<MtHermiteCurve *>(
            static_cast<char *>(entry.data))[f];
        assert(entry.propertyType == MtPropertyType::hermitecurve);

        valueNode.append_attribute("x0").set_value(value.x[0]);
        valueNode.append_attribute("x1").set_value(value.x[1]);
        valueNode.append_attribute("x2").set_value(value.x[2]);
        valueNode.append_attribute("x3").set_value(value.x[3]);
        valueNode.append_attribute("x4").set_value(value.x[4]);
        valueNode.append_attribute("x5").set_value(value.x[5]);
        valueNode.append_attribute("x6").set_value(value.x[6]);
        valueNode.append_attribute("x7").set_value(value.x[7]);
        valueNode.append_attribute("y0").set_value(value.y[0]);
        valueNode.append_attribute("y1").set_value(value.y[1]);
        valueNode.append_attribute("y2").set_value(value.y[2]);
        valueNode.append_attribute("y3").set_value(value.y[3]);
        valueNode.append_attribute("y4").set_value(value.y[4]);
        valueNode.append_attribute("y5").set_value(value.y[5]);
        valueNode.append_attribute("y6").set_value(value.y[6]);
        valueNode.append_attribute("y7").set_value(value.y[7]);
        break;
      }

      case EnumType::StringTrack: {
        auto &dataPtr =
            reinterpret_cast<PtrTypeChar *>(static_cast<char *>(entry.data))[f];
        auto attr = valueNode.append_attribute("value");

        if (dataPtr) {
          attr.set_value(dataPtr.operator->());
        }
        break;
      }

      default: {
        // const char *data = entry.data;
        PrintError("Unknown track type ", int(entry.trackType));
        break;
      }
      }
    }
  }
}

class revil::SDLImpl {
public:
  std::string buffer;

  bool IsX86() const {
    auto hdr = reinterpret_cast<const SDLHeaderV2_x86 *>(buffer.data());
    // Member strings overlaps with padding after baseTrack
    // Big endian are always x86
    // There are no MTF V1 x64 schedulers

    return int(hdr->version) < 0x10 || hdr->id == SDL_ID_BE || hdr->strings;
  }

  void ToXML(pugi::xml_node node) {
    pugi::xml_node schedNode = NewProperty(MtPropertyType::class_, node);
    schedNode.append_attribute("name").set_value("Scheduler");
    schedNode.append_attribute("type").set_value("rSchedulerXml");
    schedNode.append_attribute("id").set_value("0");
    auto hdrBase = reinterpret_cast<SDLHeaderBase *>(buffer.data());

    if (!(int(hdrBase->version) == 0x10 && !IsX86()) &&
        hdrBase->version < SDLVersion::V_20) {
      auto hdr = reinterpret_cast<SDLHeaderV1 *>(buffer.data());
      ::ToXML(hdr, schedNode);
    } else {
      if (IsX86()) {
        auto hdr = reinterpret_cast<SDLHeaderV2_x86 *>(buffer.data());
        ::ToXML(hdr, schedNode);
      } else {
        auto hdrx64 = reinterpret_cast<SDLHeaderV2_x64 *>(buffer.data());
        ::ToXML(hdrx64, schedNode);
      }
    }
  }
  void Load(BinReaderRef_e rd) {
    uint32 id;
    rd.Read(id);
    rd.Seek(0);

    if (id == SDL_ID_BE) {
      rd.SwapEndian(true);
    } else if (id != SDL_ID) {
      throw es::InvalidHeaderError(id);
    }

    rd.ReadContainer(buffer, rd.GetSize());

    auto hdrBase = reinterpret_cast<SDLHeaderBase *>(buffer.data());

    if (!(int(hdrBase->version) == 0x10 && !IsX86()) &&
        hdrBase->version < SDLVersion::V_20) {
      auto hdr = reinterpret_cast<SDLHeaderV1 *>(buffer.data());
      hdr->strings.Fixup(buffer.data());

      for (size_t i = 0; i < hdr->numTracks; i++) {
        auto &entry = hdr->entries[i];
        es::FixupPointers(buffer.data(), entry.data, entry.frames);
        // This can be misleading, since null is allowed only for root nodes
        entry.name.FixupRelative(hdr->strings);

        if (entry.trackType == SDLTrackType1::ResourceTrack ||
            entry.trackType == SDLTrackType1::StringTrack) {
          for (auto f = 0; f < entry.numFrames; f++) {
            reinterpret_cast<es::PointerX86<char> *>(
                static_cast<char *>(entry.data))[f]
                .Fixup(hdr->strings);
          }
        }
      }
    } else {
      auto FixupStuff = [&](auto *hdr) {
        const bool shouldSwap = hdr->id == SDL_ID_BE;
        using PtrTypeChar = decltype(hdr->strings);

        if (shouldSwap) {
          FByteswapper(*hdr);
        }

        hdr->strings.Fixup(buffer.data());

        for (size_t i = 0; i < hdr->numTracks; i++) {
          auto &entry = hdr->entries[i];
          if (shouldSwap) {
            FByteswapper(entry);
          }

          es::FixupPointers(buffer.data(), entry.data, entry.frames);
          // This can be misleading, since null is allowed only for root nodes
          entry.name.FixupRelative(hdr->strings);

          if (shouldSwap) {
            SwapData(entry);
          }

          using EnumType = decltype(entry.trackType);

          if (entry.trackType == EnumType::ResourceTrack ||
              entry.trackType == EnumType::StringTrack) {
            for (auto f = 0; f < entry.numFrames; f++) {
              reinterpret_cast<PtrTypeChar *>(
                  static_cast<char *>(entry.data))[f]
                  .Fixup(hdr->strings);
            }
          }
        }
      };

      if (IsX86()) {
        auto hdr = reinterpret_cast<SDLHeaderV2_x86 *>(buffer.data());
        FixupStuff(hdr);
      } else {
        auto hdr = reinterpret_cast<SDLHeaderV2_x64 *>(buffer.data());
        FixupStuff(hdr);
      }
    }
  }
};

SDL::SDL() : pi(std::make_unique<SDLImpl>()) {}
SDL::~SDL() = default;

void SDL::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SDL::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
