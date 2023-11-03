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
#include "spike/reflect/reflector.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/type/pointer.hpp"
#include "spike/type/vectors.hpp"
#include <array>
#include <cassert>

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
};

struct SDLEntry {
  SDLType type;
  UsageType usageType;
  uint16 numFrames;
  uint32 parentOrSlot;
  es::PointerX64<char> name;
  uint32 hash;
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

// Adds attributes
void ToXML(SDLFrame &frame, pugi::xml_node node) {
  node.append_attribute("frame").set_value(frame->Get<SDLFrame::Frame>());
  node.append_attribute("frameFlags").set_value(frame->Get<SDLFrame::Flags>());
}

constexpr uint32 SDL_ID = CompileFourCC("SDL");

class revil::SDLImpl {
public:
  std::string buffer;
  void ToXML(pugi::xml_node node) {
    auto root = node.append_child("class");
    auto hdr = reinterpret_cast<SDLHeader *>(buffer.data());

    node.append_attribute("type").set_value("rScheduler");
    ::ToXML(hdr->maxFrame, root.append_child("maxFrame"));
    assert(hdr->baseTrack == nullptr);

    auto entries = root.append_child("entries");
    entries.append_attribute("count").set_value(hdr->numTracks);
    std::vector<pugi::xml_node> nodes;

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
        break;

      default:
        xEntry = entries.append_child(typeName);
        xEntry.append_attribute("entrySlot").set_value(entry.parentOrSlot);
        break;
      }

      nodes.emplace_back(xEntry);

      xEntry.append_attribute("name").set_value(entry.name);
      xEntry.append_attribute("type").set_value(uint8(entry.usageType));
      // xEntry.append_attribute("id").set_value(i);

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

      if (entry.hash) {
        SetClassName(xEntry, entry.hash);
      }

      if (entry.numFrames > 0) {
        SDLFrame *frames = entry.frames;

        for (auto f = 0; f < entry.numFrames; f++) {
          auto frame = frames[f];
          auto xFrame = xEntry.append_child("frame");
          ::ToXML(frame, xFrame);

          switch (entry.type) {
          case SDLType::Int32:
            xFrame.append_attribute("value").set_value(
                reinterpret_cast<int32 *>(static_cast<char *>(entry.data))[f]);
            break;
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
            xFrame.append_attribute("index").set_value(
                reinterpret_cast<uint32 *>(static_cast<char *>(entry.data))[f]);
            break;

          case SDLType::ResourceInstance: {
            SetClassName(xFrame, *reinterpret_cast<es::PointerX64<uint32> *>(
                                     static_cast<char *>(entry.data))[f]);
            xFrame.append_attribute("path").set_value(
                reinterpret_cast<es::PointerX64<char> *>(
                    static_cast<char *>(entry.data))[f] +
                4);

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
      entry.name.FixupRelative(hdr->strings);

      if (entry.type == SDLType::ResourceInstance ||
          entry.type == SDLType::String) {
        for (auto f = 0; f < entry.numFrames; f++) {
          reinterpret_cast<es::PointerX64<char> *>(
              static_cast<char *>(entry.data))[f]
              .FixupRelative(hdr->strings);
        }
      }
    }
  }
};

SDL::SDL() : pi(std::make_unique<SDLImpl>()) {}
SDL::~SDL() = default;

void SDL::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SDL::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
