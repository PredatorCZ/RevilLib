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
#include <array>
#include <cassert>
#include <sstream>

using namespace revil;

struct XFSClassMember;

MAKE_ENUM(ENUMSCOPE(class SDLType
                    : uint8, SDLType),
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

struct SDLEntry {
  SDLType type;
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

struct SDLHeader {
  uint32 id;
  uint16 version;
  uint16 numTracks;
  uint32 unk0;
  SDLFrame maxFrame;
  es::PointerX64<char> baseTrack;
  es::PointerX64<char> strings;
  SDLEntry entries[];
};

static_assert(sizeof(SDLHeader) == 32);

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

SDLType GetSDLType(pugi::xml_node node) {
  static const auto refEnum = GetReflectedEnum<SDLType>();

  for (size_t i = 0; i < refEnum->numMembers; ++i) {
    if (std::string_view(refEnum->names[i]) == node.name()) {
      return SDLType(refEnum->values[i]);
    }
  }

  throw std::runtime_error(std::string("Unknown node type: ") + node.name());

  return SDLType::RootNode;
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
  std::vector<SDLEntry> items;
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

    auto DoValues = [&](pugi::xml_node &node, SDLEntry &entry) {
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
      case SDLType::Float:
        WriteValues(float());
        break;
      case SDLType::Vector4:
        for (auto frame : node.children("frame")) {
          dataWr.Write(FromXMLAttr<float>(frame, "x"));
          dataWr.Write(FromXMLAttr<float>(frame, "y"));
          dataWr.Write(FromXMLAttr<float>(frame, "z"));
          dataWr.Write(FromXMLAttr<float>(frame, "w"));
        }
        break;
      case SDLType::Int32:
      case SDLType::Unit:
        WriteValues(int32());
        break;
      case SDLType::NodeIndex: {
        auto &nodeRef = nodeRefs.emplace_back();
        nodeRef.offset = dataWr.Tell();

        for (auto frame : node.children("frame")) {
          dataWr.Write<uint32>(0);
          nodeRef.nodeNames.emplace_back(
              FromXMLAttr<const char *>(frame, "nodeName"));
        }
        break;
      }
      case SDLType::Bool:
        WriteValues(bool());
        dataWr.ApplyPadding(4);
        break;
      case SDLType::ResourceInstance:
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
      case SDLType::String:
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
      case SDLType::Curve:
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
      SDLEntry entry{};
      entry.type = GetSDLType(c);
      entry.usageType = UsageType(FromXMLAttr<uint32>(c, "type"));
      auto nodeName = FromXMLAttr<const char *>(c, "name");
      SetString(entry.name, nodeName);

      switch (entry.type) {
      case SDLType::RootNode:
        classNodes.emplace(nodeName, items.size());
        items.emplace_back(entry);
        WriteValues(c, 0);
        break;

      case SDLType::ClassNode:
        entry.hashOrArrayIndex = ResourceType(c);
        entry.parentOrSlot = FromXMLAttr<uint32>(c, "entrySlot");
        classNodes.emplace(nodeName, items.size());
        items.emplace_back(entry);
        WriteValues(c, items.size() - 1);
        break;

      case SDLType::ClassMemberNode:
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

void revil::SDLFromXML(BinWritterRef wr, pugi::xml_node rootNode) {
  auto classNode = XMLChild(rootNode, "class");

  if (std::string_view("rScheduler") !=
      FromXMLAttr<const char *>(classNode, "type")) {
    throw std::runtime_error("Invalid class type, expected rScheduler");
  }

  SDLHeader hdr{};
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

class revil::SDLImpl {
public:
  std::string buffer;

  void ToXML(pugi::xml_node node) {
    auto root = node.append_child("class");
    auto hdr = reinterpret_cast<SDLHeader *>(buffer.data());

    root.append_attribute("type").set_value("rScheduler");
    ::ToXML(hdr->maxFrame, root.append_child("maxFrame"));
    assert(hdr->baseTrack == nullptr);

    auto entries = root.append_child("entries");
    std::vector<pugi::xml_node> nodes;
    pugi::xml_node currentRoot;

    for (size_t i = 0; i < hdr->numTracks; i++) {
      auto &entry = hdr->entries[i];
      assert(entry.unk2 == 0);
      assert(entry.unk3 == 0);
      static const auto refEnum = GetReflectedEnum<SDLType>();
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
        auto clName = GetClassName(hash, Platform::Win32);

        if (clName.empty()) {
          char buffer[0x10]{};
          snprintf(buffer, sizeof(buffer), "%X", hash);
          node.append_attribute("resourceHash").set_value(buffer);
        } else {
          node.append_attribute("resourceType").set_value(clName.data());
        }
      };

      switch (entry.type) {
      case SDLType::Float:
      case SDLType::Vector4:
      case SDLType::Int32:
      case SDLType::ClassMemberNode:
      case SDLType::Bool:
      case SDLType::NodeIndex:
      case SDLType::ResourceInstance:
      case SDLType::String:
      case SDLType::Unit:
      case SDLType::Curve:
        xEntry = nodes.at(entry.parentOrSlot).append_child(typeName);
        xEntry.append_attribute("arrayIndex").set_value(entry.hashOrArrayIndex);
        break;

      default:
        xEntry = currentRoot.append_child(typeName);
        xEntry.append_attribute("entrySlot").set_value(entry.parentOrSlot);
        SetClassName(xEntry, entry.hashOrArrayIndex);
        break;
      case SDLType::RootNode:
        xEntry = currentRoot = entries.append_child(typeName);
        assert(entry.parentOrSlot == 0);
        break;
      }

      nodes.emplace_back(xEntry);

      xEntry.append_attribute("name").set_value(entry.name);
      xEntry.append_attribute("type").set_value(uint8(entry.usageType));

      if (entry.numFrames > 0) {
        SDLFrame *frames = entry.frames;

        for (auto f = 0; f < entry.numFrames; f++) {
          auto frame = frames[f];
          auto xFrame = xEntry.append_child("frame");
          ::ToXML(frame, xFrame);

          switch (entry.type) {
          case SDLType::Int32:
          case SDLType::Unit:
            xFrame.append_attribute("value").set_value(
                reinterpret_cast<int32 *>(static_cast<char *>(entry.data))[f]);
            break;
          case SDLType::Vector4: {
            auto &value =
                reinterpret_cast<Vector4 *>(static_cast<char *>(entry.data))[f];
            xFrame.append_attribute("x").set_value(value.x);
            xFrame.append_attribute("y").set_value(value.y);
            xFrame.append_attribute("z").set_value(value.z);
            xFrame.append_attribute("w").set_value(value.w);
            break;
          }
          case SDLType::Float:
            xFrame.append_attribute("value").set_value(
                reinterpret_cast<float *>(static_cast<char *>(entry.data))[f]);
            break;
          case SDLType::Bool:
            xFrame.append_attribute("value").set_value(
                reinterpret_cast<bool *>(static_cast<char *>(entry.data))[f]);
            break;
          case SDLType::NodeIndex:
            xFrame.append_attribute("nodeName")
                .set_value(hdr->entries[reinterpret_cast<uint32 *>(
                                            static_cast<char *>(entry.data))[f]]
                               .name);
            break;

          case SDLType::ResourceInstance: {
            auto &dataPtr = reinterpret_cast<es::PointerX64<uint32> *>(
                static_cast<char *>(entry.data))[f];

            if (dataPtr) {
              SetClassName(xFrame, *dataPtr);
              xFrame.append_attribute("path").set_value(
                  reinterpret_cast<const char *>(dataPtr.operator->() + 1));
            }

            break;
          }

          case SDLType::Curve: {
            auto &value = reinterpret_cast<std::array<float, 16> *>(
                static_cast<char *>(entry.data))[f];
            for (size_t i = 0; i < value.size(); i++) {
              auto aName = "e" + std::to_string(i);
              xFrame.append_attribute(aName.c_str()).set_value(value[i]);
            }
            break;
          }

          case SDLType::String:
            xFrame.append_attribute("value").set_value(
                reinterpret_cast<es::PointerX64<char> *>(
                    static_cast<char *>(entry.data))[f]);

            break;

          default:
            break;
          }
        }
      }
    }
  }
  void Load(BinReaderRef_e rd) {
    uint32 id;
    rd.Read(id);
    rd.Seek(0);

    if (id != SDL_ID) {
      throw es::InvalidHeaderError(id);
    }

    rd.ReadContainer(buffer, rd.GetSize());

    auto hdr = reinterpret_cast<SDLHeader *>(buffer.data());
    es::FixupPointers(buffer.data(), hdr->baseTrack, hdr->strings);

    for (size_t i = 0; i < hdr->numTracks; i++) {
      auto &entry = hdr->entries[i];
      es::FixupPointers(buffer.data(), entry.data, entry.frames);
      // This can be misleading, since null is allowed only for root nodes
      entry.name.FixupRelative(hdr->strings);

      if (entry.type == SDLType::ResourceInstance ||
          entry.type == SDLType::String) {
        for (auto f = 0; f < entry.numFrames; f++) {
          reinterpret_cast<es::PointerX64<char> *>(
              static_cast<char *>(entry.data))[f]
              .Fixup(hdr->strings);
        }
      }
    }
  }
};

SDL::SDL() : pi(std::make_unique<SDLImpl>()) {}
SDL::~SDL() = default;

void SDL::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SDL::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
