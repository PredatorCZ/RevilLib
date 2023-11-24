/*  Revil Format Library
    Copyright(C) 2021-2023 Lukas Cone

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

#define ES_COPYABLE_POINTER

#include "revil/sdl.hpp"
#include "pugixml.hpp"
#include "revil/hashreg.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
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

MAKE_ENUM(ENUMSCOPE(class SDLType1
                    : uint8, SDLType1),
          EMEMBERVAL(RootNode, 1),        //
          EMEMBER(ClassNode),             //
          EMEMBER(SpecialClassNode),      //
          EMEMBERVAL(ClassMemberNode, 5), //
          EMEMBER(Int32),                 //
          EMEMBER(Vector4),               //
          EMEMBER(Float),                 //
          EMEMBER(Bool),                  //
          EMEMBER(NodeIndex),             //
          EMEMBER(ResourceInstance),      //
          EMEMBERVAL(BitFlags, 13),       //
          // UNUSED BELOW
          EMEMBERVAL(String, 128), //
          EMEMBER(Unit),           //
          EMEMBER(Curve)           //

);

MAKE_ENUM(ENUMSCOPE(class SDLType2
                    : uint8, SDLType2),
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
          EMEMBER(Curve),                   //
          // UNUSED BELOW
          EMEMBERVAL(BitFlags, 128) //
);

enum class UsageType : uint8 {
  None,
  ClassMember,
  NodeRef,
  Boolean,
  Integer = 6,
  Decimal = 12,
  String = 14,
  Color,
  Vector3 = 20,
  Quaternion = 22,
  Vector2 = 34,
  Curve = 57,
  ResourcePath = 128,
};

struct SDLFrame {
  using Frame = BitMemberDecl<0, 24>;
  using Flags = BitMemberDecl<1, 8>;
  using type = BitFieldType<uint32, Frame, Flags>;
  type data;

  const type *operator->() const { return &data; }
  type *operator->() { return &data; }
};

struct SDLHeaderBase {
  uint32 id;
  uint16 version;
  uint16 numTracks;
};

struct SDLEntryV1 {
  SDLType1 type;
  UsageType usageType;
  uint16 numFrames;
  uint32 parentOrSlot;
  es::PointerX86<char> name;
  uint32 hashOrArrayIndex;
  es::PointerX86<SDLFrame> frames;
  es::PointerX86<char> data;
};

struct SDLHeaderV1 : SDLHeaderBase {
  SDLFrame maxFrame;
  uint32 baseTrack;
  es::PointerX86<char> strings;
  SDLEntryV1 entries[];
};

struct SDLEntryV2_x64 {
  SDLType2 type;
  UsageType usageType;
  uint16 numFrames;
  uint32 parentOrSlot;
  es::PointerX64<char> name;
  uint32 hashOrArrayIndex;
  uint32 unk2;
  uint64 unk3;
  es::PointerX64<SDLFrame> frames;
  es::PointerX64<char> data;
};

struct SDLHeaderV2_x64 : SDLHeaderBase {
  uint32 unk0;
  SDLFrame maxFrame;
  uint32 baseTrack;
  es::PointerX64<char> strings;
  SDLEntryV2_x64 entries[];
};

struct SDLHeaderV2_x86 : SDLHeaderBase {
  uint32 unk0;
  SDLFrame maxFrame;
  uint32 baseTrack;
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
  FByteswapper(item.unk0);
}

template <> void FByteswapper(SDLHeaderV2_x64 &, bool) {
  // Not implemented
}

template <class C> void SwapData(C &entry, bool way = false) {
  size_t numBlocks = 0;
  using EnumType = decltype(entry.type);

  switch (entry.type) {
  case EnumType::Float:
  case EnumType::Int32:
  case EnumType::NodeIndex:
  case EnumType::ResourceInstance:
  case EnumType::String:
  case EnumType::Unit:
  case EnumType::BitFlags:
    numBlocks = 1;
    break;

  case EnumType::Vector4:
    numBlocks = 4;
    break;
  case EnumType::Curve:
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

pugi::xml_node XMLChild(pugi::xml_node parentNode, const char *nodeName) {
  auto node = parentNode.child(nodeName);

  if (node.empty()) {
    throw std::runtime_error(
        "Cannot find child node: " + std::string(nodeName) +
        " for node: " + node.name());
  }

  return node;
}

void ToXML(SDLFrame &frame, pugi::xml_node node) {
  node.append_attribute("frame").set_value(frame->Get<SDLFrame::Frame>());
  node.append_attribute("frameFlags").set_value(frame->Get<SDLFrame::Flags>());
}

void FromXML(SDLFrame &frame, pugi::xml_node node) {
  frame->Set<SDLFrame::Frame>(FromXMLAttr<uint32>(node, "frame"));
  frame->Set<SDLFrame::Flags>(FromXMLAttr<uint32>(node, "frameFlags"));
}

struct PaddingRange {
  uint32 offset;
  uint32 size;
};

SDLType2 GetSDLTypeV2(pugi::xml_node node) {
  static const auto refEnum = GetReflectedEnum<SDLType2>();

  for (size_t i = 0; i < refEnum->numMembers; ++i) {
    if (std::string_view(refEnum->names[i]) == node.name()) {
      return SDLType2(refEnum->values[i]);
    }
  }

  throw std::runtime_error(std::string("Unknown node type: ") + node.name());

  return SDLType2::RootNode;
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
  std::vector<SDLEntryV2_x64> items;
  std::map<std::string_view, uint32> classNodes;
  std::vector<NodeRef> nodeRefs;
  bool firstFrame = true;

  void WriteValues(pugi::xml_node entries, size_t parentIndex) {
    auto SetString = [this](auto &ptr, auto str) {
      if (auto found = strings.find(str); found != strings.end()) {
        ptr = reinterpret_cast<char *>(found->second);
      } else {
        const size_t newId = strings.size();
        strings.emplace(str, newId);
        ptr = reinterpret_cast<char *>(newId);
      }
    };

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
          ::FromXML(frames.emplace_back(), frame);
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

      switch (entry.type) {
      case SDLType2::Float:
        WriteValues(float());
        break;
      case SDLType2::Vector4:
        for (auto frame : node.children("frame")) {
          dataWr.Write(FromXMLAttr<float>(frame, "x"));
          dataWr.Write(FromXMLAttr<float>(frame, "y"));
          dataWr.Write(FromXMLAttr<float>(frame, "z"));
          dataWr.Write(FromXMLAttr<float>(frame, "w"));
        }
        break;
      case SDLType2::Int32:
      case SDLType2::Unit:
        WriteValues(int32());
        break;
      case SDLType2::NodeIndex: {
        auto &nodeRef = nodeRefs.emplace_back();
        nodeRef.offset = dataWr.Tell();

        for (auto frame : node.children("frame")) {
          dataWr.Write<uint32>(0);
          nodeRef.nodeNames.emplace_back(
              FromXMLAttr<const char *>(frame, "nodeName"));
        }
        break;
      }
      case SDLType2::Bool:
        WriteValues(bool());
        dataWr.ApplyPadding(4);
        break;
      case SDLType2::ResourceInstance:
        for (auto frame : node.children("frame")) {
          if (frame.attribute("path").empty()) {
            dataWr.Write<uintptr_t>(0);
            continue;
          }

          const uint32 cHash = ResourceType(frame);
          std::string wString(sizeof(cHash), '-');
          wString.append(FromXMLAttr<const char *>(frame, "path"));
          memcpy(wString.data(), &cHash, sizeof(cHash));
          const char *ptr;
          SetString(ptr, wString);
          stringPointers.emplace_back(StringPointer{
              .offset = uint32(dataWr.Tell()),
              .stringId = uint32(reinterpret_cast<uintptr_t>(ptr)),
          });
          dataWr.Write(ptr);
        }
        break;
      case SDLType2::String:
        for (auto frame : node.children("frame")) {
          const char *ptr;
          SetString(ptr, FromXMLAttr<const char *>(frame, "value"));
          stringPointers.emplace_back(StringPointer{
              .offset = uint32(dataWr.Tell()),
              .stringId = uint32(reinterpret_cast<uintptr_t>(ptr)),
          });
          dataWr.Write(ptr);
        }
        break;
      case SDLType2::Curve:
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
      entry.type = GetSDLTypeV2(c);
      entry.usageType = UsageType(FromXMLAttr<uint32>(c, "type"));
      auto nodeName = FromXMLAttr<const char *>(c, "name");
      SetString(entry.name, nodeName);

      switch (entry.type) {
      case SDLType2::RootNode:
        classNodes.emplace(nodeName, items.size());
        items.emplace_back(entry);
        WriteValues(c, 0);
        break;

      case SDLType2::ClassNode:
        entry.hashOrArrayIndex = ResourceType(c);
        entry.parentOrSlot = FromXMLAttr<uint32>(c, "entrySlot");
        classNodes.emplace(nodeName, items.size());
        items.emplace_back(entry);
        WriteValues(c, items.size() - 1);
        break;

      case SDLType2::ClassMemberNode:
        entry.hashOrArrayIndex = FromXMLAttr<uint32>(c, "arrayIndex");
        entry.parentOrSlot = parentIndex;
        items.emplace_back(entry);
        WriteValues(c, items.size() - 1);
        break;

      default:
        entry.hashOrArrayIndex = FromXMLAttr<uint32>(c, "arrayIndex");
        entry.parentOrSlot = parentIndex;
        DoValues(c, entry);
        items.emplace_back(entry);
        break;
      }
    }
  }
  /*
    void WriteFrames(pugi::xml_node entries) {
      auto DoFrames = [&](pugi::xml_node &node) {
        std::vector<SDLFrame> frames;

        for (auto frame : node.children("frame")) {
          ::FromXML(frames.emplace_back(), frame);
        }

        const size_t reqArea = frames.size() * sizeof(SDLFrame);

        if (reqArea > 12) {
          const uint32 retOffset = dataWr.Tell();
          dataWr.WriteContainer(frames);
          return retOffset;
        }

        for (auto it = paddingRanges.begin(); it != paddingRanges.end(); it++) {
          if (it->size == reqArea) {
            dataWr.Push();
            dataWr.Seek(it->offset);
            dataWr.WriteContainer(frames);
            dataWr.Pop();
            const uint32 retOffset = it->offset;
            paddingRanges.erase(it);
            return retOffset;
          }
        }

        auto bestAreaIt = paddingRanges.begin();
        uint32 minSize = 16;

        for (auto it = bestAreaIt; it != paddingRanges.end(); it++) {
          if (it->size > reqArea) {
            if (it->size < minSize) {
              bestAreaIt = it;
              minSize = it->size;
            }
          }
        }

        if (minSize < 16) {
          dataWr.Push();
          dataWr.Seek(bestAreaIt->offset);
          dataWr.WriteContainer(frames);
          dataWr.Pop();
          const uint32 retOffset = bestAreaIt->offset;
          bestAreaIt->offset += reqArea;
          bestAreaIt->size -= reqArea;
          return retOffset;
        }

        const uint32 retOffset = dataWr.Tell();
        dataWr.WriteContainer(frames);
        return retOffset;
      };

      for (auto c : entries.children()) {
        SDLType type = GetSDLType(c);

        switch (type) {
        case SDLType::RootNode:
          break;
        case SDLType::ArrayEntry:
        case SDLType::ClassNode:
          WriteFrames(c);
          break;

        default:
          frameOffsetStack.emplace_back(DoFrames(c));
          break;
        }
      }
    }*/
};

constexpr uint32 SDL_ID = CompileFourCC("SDL");
constexpr uint32 SDL_ID_BE = CompileFourCC("\0LDS");

void revil::SDLFromXML(BinWritterRef wr, pugi::xml_node rootNode) {
  auto classNode = XMLChild(rootNode, "class");

  if (std::string_view("rScheduler") !=
      FromXMLAttr<const char *>(classNode, "type")) {
    throw std::runtime_error("Invalid class type, expected rScheduler");
  }

  SDLHeaderV2_x64 hdr{};
  hdr.id = SDL_ID;
  hdr.version = 0x16;
  hdr.unk0 = 0xE2316427;
  ::FromXML(hdr.maxFrame, XMLChild(classNode, "maxFrame"));
  auto entries = XMLChild(classNode, "entries");
  DataBuilder dataBuilder;
  dataBuilder.WriteValues(entries, 0);
  hdr.numTracks = dataBuilder.items.size();

  wr.Write(hdr);
  wr.WriteContainer(dataBuilder.items);
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

  for (auto &e : dataBuilder.items) {
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
  wr.WriteContainer(dataBuilder.items);
}

template <class HdrType> void ToXML(HdrType *hdr, pugi::xml_node root) {
  using EntryType = std::decay_t<decltype(hdr->entries[0])>;
  using EnumType = decltype(EntryType::type);
  using PtrTypeChar = decltype(hdr->strings);
  using PtrTypeUint =
      std::conditional_t<sizeof(PtrTypeChar) == 8, es::PointerX64<uint32>,
                         es::PointerX86<uint32>>;

  ::ToXML(hdr->maxFrame, root.append_child("maxFrame"));

  if (hdr->baseTrack > 0) {
    auto &entry = hdr->entries[hdr->baseTrack];
    std::string xmlTrack(
        static_cast<const char *>(hdr->entries[entry.parentOrSlot].name));
    xmlTrack.append("::");
    xmlTrack.append(static_cast<const char *>(entry.name));

    root.append_attribute("baseTrack").set_value(xmlTrack.c_str());
  }

  auto entries = root.append_child("entries");
  std::vector<pugi::xml_node> nodes;
  pugi::xml_node currentRoot;

  for (size_t i = 0; i < hdr->numTracks; i++) {
    auto &entry = hdr->entries[i];

    if constexpr (std::is_same_v<HdrType, SDLHeaderV2_x64>) {
      assert(entry.unk2 == 0);
      assert(entry.unk3 == 0);
    }
    static const auto refEnum = GetReflectedEnum<EnumType>();
    auto typeName = [&] {
      const size_t numEns = refEnum->numMembers;

      for (size_t i = 0; i < numEns; i++) {
        if (refEnum->values[i] == static_cast<uint64>(entry.type)) {
          return refEnum->names[i];
        }
      }

      return "__UNREGISTERED__";
    }();

    pugi::xml_node xEntry;

    auto SetClassName = [](pugi::xml_node &node, uint32 hash) {
      auto clName = GetClassName(hash);

      if (clName.empty()) {
        char buffer[0x10]{};
        snprintf(buffer, sizeof(buffer), "%X", hash);
        node.append_attribute("resourceHash").set_value(buffer);
      } else {
        node.append_attribute("resourceType").set_value(clName.data());
      }
    };

    switch (entry.type) {
    case EnumType::Float:
    case EnumType::Vector4:
    case EnumType::Int32:
    case EnumType::ClassMemberNode:
    case EnumType::Bool:
    case EnumType::NodeIndex:
    case EnumType::ResourceInstance:
    case EnumType::String:
    case EnumType::Unit:
    case EnumType::Curve:
    case EnumType::BitFlags:
      xEntry = nodes.at(entry.parentOrSlot).append_child(typeName);
      xEntry.append_attribute("arrayIndex").set_value(entry.hashOrArrayIndex);
      break;

    default:
      xEntry = currentRoot.append_child(typeName);
      xEntry.append_attribute("entrySlot").set_value(entry.parentOrSlot);
      SetClassName(xEntry, entry.hashOrArrayIndex);
      break;
    case EnumType::RootNode:
      xEntry = currentRoot = entries.append_child(typeName);
      assert(entry.parentOrSlot == 0);
      break;
    }

    nodes.emplace_back(xEntry);

    xEntry.append_attribute("name").set_value(
        static_cast<const char *>(entry.name));
    xEntry.append_attribute("type").set_value(uint8(entry.usageType));
    //xEntry.append_attribute("id").set_value(i);

    if (entry.numFrames > 0) {
      SDLFrame *frames = entry.frames;

      for (auto f = 0; f < entry.numFrames; f++) {
        auto frame = frames[f];
        auto xFrame = xEntry.append_child("frame");
        ::ToXML(frame, xFrame);

        switch (entry.type) {
        case EnumType::Int32:
        case EnumType::Unit:
          xFrame.append_attribute("value").set_value(
              reinterpret_cast<int32 *>(static_cast<char *>(entry.data))[f]);
          break;
        case EnumType::Vector4: {
          auto &value =
              reinterpret_cast<Vector4 *>(static_cast<char *>(entry.data))[f];
          xFrame.append_attribute("x").set_value(value.x);
          xFrame.append_attribute("y").set_value(value.y);
          xFrame.append_attribute("z").set_value(value.z);
          xFrame.append_attribute("w").set_value(value.w);
          break;
        }
        case EnumType::Float:
          xFrame.append_attribute("value").set_value(
              reinterpret_cast<float *>(static_cast<char *>(entry.data))[f]);
          break;
        case EnumType::Bool:
          xFrame.append_attribute("value").set_value(
              reinterpret_cast<bool *>(static_cast<char *>(entry.data))[f]);
          break;
        case EnumType::BitFlags:
          xFrame.append_attribute("value").set_value(
              reinterpret_cast<uint32 *>(static_cast<char *>(entry.data))[f]);
          break;
        case EnumType::NodeIndex:
          xFrame.append_attribute("nodeName")
              .set_value(static_cast<const char *>(
                  hdr->entries[reinterpret_cast<uint32 *>(
                                   static_cast<char *>(entry.data))[f]]
                      .name));
          break;

        case EnumType::ResourceInstance: {
          auto &dataPtr = reinterpret_cast<PtrTypeUint *>(
              static_cast<char *>(entry.data))[f];

          if (dataPtr) {
            SetClassName(xFrame, *dataPtr);
            xFrame.append_attribute("path").set_value(
                reinterpret_cast<const char *>(dataPtr.operator->() + 1));
          }

          break;
        }

        case EnumType::Curve: {
          auto &value = reinterpret_cast<std::array<float, 16> *>(
              static_cast<char *>(entry.data))[f];
          for (size_t i = 0; i < value.size(); i++) {
            auto aName = "e" + std::to_string(i);
            xFrame.append_attribute(aName.c_str()).set_value(value[i]);
          }
          break;
        }

        case EnumType::String:
          xFrame.append_attribute("value").set_value(
              static_cast<const char *>(reinterpret_cast<PtrTypeChar *>(
                  static_cast<char *>(entry.data))[f]));

          break;

        default:
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

    return hdr->version < 0x10 || hdr->id == SDL_ID_BE || hdr->strings;
  }

  void ToXML(pugi::xml_node node) {
    auto root = node.append_child("class");
    root.append_attribute("type").set_value("rScheduler");
    auto hdrBase = reinterpret_cast<SDLHeaderBase *>(buffer.data());

    if (hdrBase->version < 0x10) {
      auto hdr = reinterpret_cast<SDLHeaderV1 *>(buffer.data());
      ::ToXML(hdr, root);
    } else {
      if (IsX86()) {
        auto hdr = reinterpret_cast<SDLHeaderV2_x86 *>(buffer.data());
        ::ToXML(hdr, root);
      } else {
        auto hdrx64 = reinterpret_cast<SDLHeaderV2_x64 *>(buffer.data());
        ::ToXML(hdrx64, root);
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

    if (hdrBase->version < 0x10) {
      auto hdr = reinterpret_cast<SDLHeaderV1 *>(buffer.data());
      hdr->strings.Fixup(buffer.data());

      for (size_t i = 0; i < hdr->numTracks; i++) {
        auto &entry = hdr->entries[i];
        es::FixupPointers(buffer.data(), entry.data, entry.frames);
        // This can be misleading, since null is allowed only for root nodes
        entry.name.FixupRelative(hdr->strings);

        if (entry.type == SDLType1::ResourceInstance ||
            entry.type == SDLType1::String) {
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

          using EnumType = decltype(entry.type);

          if (entry.type == EnumType::ResourceInstance ||
              entry.type == EnumType::String) {
            for (auto f = 0; f < entry.numFrames; f++) {
              reinterpret_cast<es::PointerX64<char> *>(
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
