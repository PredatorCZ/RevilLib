#include "revil/xfs.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/bitfield.hpp"
#include "datas/matrix44.hpp"
#include "datas/reflector.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/vectors_simd.hpp"
#include "pugixml.hpp"
#include "revil/hashreg.hpp"
#include <algorithm>
#include <deque>
#include <vector>

// #define XFS_DEBUG

/*
lp pc: 4
lp ps3: 5
lp2 pc: 8
*/

using namespace revil;

struct XFSClassMember;

MAKE_ENUM(ENUMSCOPE(class XFSType
                    : uint8, XFSType), //
          EMEMBER(invalid_),           //
          EMEMBER(class_),             //
          EMEMBER(classref_),          //
          EMEMBER(bool_),              //
          EMEMBER(u8_),                //
          EMEMBER(u16_),               //
          EMEMBER(u32_),               //
          EMEMBER(u64_),               //
          EMEMBER(s8_),                //
          EMEMBER(s16_),               //
          EMEMBER(s32_),               //
          EMEMBER(s64_),               //
          EMEMBER(f32_),               //
          EMEMBERVAL(string_, 14),     //
          EMEMBER(color_),             //
          EMEMBER(point_),             //
          EMEMBER(size_),              //
          EMEMBER(rect_),              // 8+ rectangle?
          EMEMBER(_matrix_),           //
          EMEMBER(vector4_),           //
          EMEMBER(_vector4_),          // colour
          EMEMBERVAL(string2_, 32),    //
          EMEMBERVAL(vector3_, 35),    //
          EMEMBERVAL(_resource_, 0x80) // 8+, custom?
);

struct XFSSizeAndFlag {
  using Size = BitMemberDecl<0, 15>;
  using Unk = BitMemberDecl<1, 1>;
  using type = BitFieldType<uint16, Size, Unk>;
  type data;

  const type *operator->() const { return &data; }
};

struct XFSClassInfo {
  using NumMembers = BitMemberDecl<0, 15>;
  using Unk = BitMemberDecl<1, 17>;
  using type = BitFieldType<uint32, NumMembers, Unk>;
  type data;

  const type *operator->() const { return &data; }
};

struct XFSMeta {
  using Active = BitMemberDecl<0, 1>;
  using LayoutIndex = BitMemberDecl<1, 15>;
  using MetaIndex = BitMemberDecl<2, 16>;
  using type = BitFieldType<uint32, Active, LayoutIndex, MetaIndex>;
  type data;

  const type *operator->() const { return &data; }
};

struct XFSHeaderBase {
  uint32 id;
  uint16 version;
  uint16 unk; // class version?

  void SwapEndian() {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(unk);
  }
};

struct XFSHeaderV1 : XFSHeaderBase {
  uint32 numLayouts;
  uint32 dataStart;

  void SwapEndian() {
    FByteswapper(numLayouts);
    FByteswapper(dataStart);
  }
};

struct XFSHeaderV2 : XFSHeaderBase {
  uint64 unk0; // num members/strings?
  uint32 numLayouts;
  uint32 dataStart;
};

template <class PadType> struct XFSClassMemberRaw {
  std::string memberName;
  XFSType type;
  uint8 flags; // alignment flags??
  XFSSizeAndFlag memberSize;
  PadType null[4];

  void Read(BinReaderRef_e rd) {
    uint32 memNameOffset;
    rd.Read(memNameOffset);
    rd.Read(type);
    rd.Read(flags);
    rd.Read(memberSize.data);
    rd.Read(null);
    rd.Push();
    rd.Seek(memNameOffset);
    rd.ReadString(memberName);
    rd.Pop();
  }
};

template <class PtrType> struct XFSClassMemberV2 {
  std::string memberName;
  XFSType type;
  uint8 flags; // alignment flags??
  uint16 memberSize;
  PtrType null[8];

  void Read(BinReaderRef_e rd) {
    PtrType memNameOffset;
    rd.Read(memNameOffset);
    rd.Read(type);
    rd.Read(flags);
    rd.Read(memberSize);
    if constexpr (sizeof(PtrType) == 8) {
      rd.Skip(4);
    }
    rd.Read(null);
    rd.Push();
    rd.Seek(memNameOffset);
    rd.ReadString(memberName);
    rd.Pop();
  }
};

template <class PadType> struct XFSClass {
  uint32 hash;
  XFSClassInfo info;
  std::vector<XFSClassMemberRaw<PadType>> members;

  void Read(BinReaderRef_e rd) {
    rd.Read(hash);
    rd.Read(info.data);
    rd.ReadContainer(members, info->Get<XFSClassInfo::NumMembers>());
  }
};

template <class PtrType> struct XFSClassV2 {
  uint32 hash;
  std::vector<XFSClassMemberV2<PtrType>> members;

  void Read(BinReaderRef_e rd) {
    rd.Read(hash);
    if constexpr (sizeof(PtrType) == 8) {
      rd.Skip(4);
    }
    rd.ReadContainer<PtrType>(members);
  }
};

struct XFSClassMember {
  std::string name;
  XFSType type;
  uint8 flags;
  uint16 size;

  XFSClassMember() = default;
  template <class pad_type>
  XFSClassMember(XFSClassMemberRaw<pad_type> &&raw)
      : name(std::move(raw.memberName)), type(raw.type), flags(raw.flags),
        size(raw.memberSize->template Get<XFSSizeAndFlag::Size>()) {
    if (raw.memberSize->template Get<XFSSizeAndFlag::Unk>()) {
      throw std::runtime_error("Some bullshit");
    }
  }

  template <class pad_type>
  XFSClassMember(XFSClassMemberV2<pad_type> &&raw)
      : name(std::move(raw.memberName)), type(raw.type), flags(raw.flags),
        size(raw.memberSize) {}
};

REFLECT(CLASS(XFSClassMember), MEMBER(name), MEMBER(type), MEMBER(flags));

struct XFSClassDesc {
  uint32 hash;
  std::string_view className;
  std::vector<XFSClassMember> members;

  XFSClassDesc() = default;
  template <class pad_type>
  XFSClassDesc(XFSClass<pad_type> &&raw) : hash(raw.hash) {
    members.reserve(raw.members.size());

    std::transform(std::make_move_iterator(raw.members.begin()),
                   std::make_move_iterator(raw.members.end()),
                   std::back_inserter(members),
                   [](auto &&item) { return std::move(item); });
  }

  template <class PtrType>
  XFSClassDesc(XFSClassV2<PtrType> &&raw) : hash(raw.hash) {
    members.reserve(raw.members.size());

    std::transform(std::make_move_iterator(raw.members.begin()),
                   std::make_move_iterator(raw.members.end()),
                   std::back_inserter(members),
                   [](auto &&item) { return std::move(item); });
  }

  void ToXML(pugi::xml_node node) const;
};

void XFSClassDesc::ToXML(pugi::xml_node node) const {
  auto cNode = node.append_child("class");

  if (className.empty()) {
    char buffer[0x10]{};
    snprintf(buffer, sizeof(buffer), "%X", hash);
    cNode.append_attribute("hash").set_value(buffer);
  } else {
    cNode.append_attribute("name").set_value(className.data());
  }

  for (auto &m : members) {
    ReflectorWrap<const XFSClassMember> refl(m);
    auto mNode = cNode.append_child("member");
    ReflectorXMLUtil::SaveV2a(refl, mNode,
                              {ReflectorXMLUtil::Flags_StringAsAttribute});
  }
}

struct XFSDataResource {
  std::string type;
  std::string file;

  void Read(BinReaderRef_e rd) {
    uint8 numStrings;
    rd.Read(numStrings); // ctype?

    if (numStrings != 2) {
      throw std::logic_error("Unexpected number!");
    }

    rd.ReadString(type); // rtype?
    rd.ReadString(file); // path?
  }
};

struct XFSData {
  union TypeData {
    bool asBool;
    int8 asInt8;
    uint8 asUInt8;
    int16 asInt16;
    uint16 asUInt16;
    int32 asInt32;
    uint32 asUInt32;
    int64 asInt64;
    uint64 asUInt64;
    Vector2 asVector2;
    Vector asVector3;
    Vector4A16 asVector4;
    IVector4A16 asIVector4;
    IVector2 asIVector2;
    UIVector2 asUIVector2;
    IVector asIVector3;
    void *asPointer;
    UCVector4 asColor;
    float asFloat;
    double asDouble;
    char raw[sizeof(Vector4A16)];

    TypeData() { memset(raw, 0, sizeof(raw)); }
  };

  template <class type> type *AllocArray(size_t numItems) {
    const size_t allocSize = sizeof(type) * numItems;
    mustFree = Free_Free;
    auto value = malloc(allocSize);
    data.asPointer = value;
    return static_cast<type *>(value);
  }
  template <class type> type *AllocClass() {
    mustFree = Free_DeleteSingle;
    auto value = new type();
    data.asPointer = value;
    return value;
  }
  template <class type> type *AllocClasses(size_t numItems) {
    mustFree = Free_DeleteArray;
    auto value = new type[numItems]();
    data.asPointer = value;
    return value;
  }
  void SetString(std::string_view sw) {
    if (sw.size() < sizeof(data.raw)) {
      memcpy(data.raw, sw.data(), sw.size());
      stringInRaw = true;
    } else {
      memcpy(AllocArray<char>(sw.size() + 1), sw.data(), sw.size() + 1);
    }
  }

  const char *AsString() const {
    return stringInRaw ? data.raw : static_cast<const char *>(data.asPointer);
  }

  XFSData(XFSData &&o)
      : rtti(o.rtti), numItems(o.numItems), stringInRaw(o.stringInRaw),
        mustFree(o.mustFree), data(o.data) {
    o.mustFree = Free_None;
  }
  XFSData() = default;
  XFSData(const XFSData &) = delete;

  ~XFSData() {
    switch (mustFree) {
    case Free_Free:
      free(data.asPointer);
      break;
    case Free_DeleteSingle: {
      switch (rtti->type) {
      case XFSType::_resource_:
        delete static_cast<XFSDataResource *>(data.asPointer);
        break;
      case XFSType::_matrix_:
        delete static_cast<es::Matrix44 *>(data.asPointer);
        break;

      default:
        break;
      }
      break;
    }
    case Free_DeleteArray: {
      switch (rtti->type) {
      case XFSType::_resource_:
        delete[] static_cast<XFSDataResource *>(data.asPointer);
        break;
      case XFSType::_matrix_:
        delete[] static_cast<es::Matrix44 *>(data.asPointer);
        break;

      default:
        break;
      }
      break;
    }

    default:
      break;
    }
  }

  XFSClassMember *rtti = nullptr;
  uint32 numItems = 0;

private:
  bool stringInRaw = false;

  enum FreeType : uint8 {
    Free_None,
    Free_Free,
    Free_DeleteSingle,
    Free_DeleteArray,
  };
  FreeType mustFree = Free_None;

public:
  TypeData data;
};

struct XFSClassData {
  std::vector<XFSData> members;
  XFSClassDesc *rtti = nullptr;
};

class revil::XFSImpl {
public:
  std::vector<XFSClassDesc> rtti;
  std::deque<XFSClassData> dataStore;
  XFSClassData *root;

  template <class PtrType>
  void ReadData(BinReaderRef_e rd, XFSClassData **root = nullptr);
  void ToXML(const XFSClassData &item, pugi::xml_node node);
  void ToXML(pugi::xml_node node);
  void RTTIToXML(pugi::xml_node node);
  void Load(BinReaderRef_e rd);
};

XFS::XFS() : pi(std::make_unique<XFSImpl>()) {}
XFS::~XFS() = default;

void XFS::Load(BinReaderRef_e rd) { pi->Load(rd); }

void XFS::ToXML(pugi::xml_node node) const { pi->ToXML(node); }

void XFS::RTTIToXML(pugi::xml_node node) const { pi->RTTIToXML(node); }

template <class PtrType>
void XFSImpl::ReadData(BinReaderRef_e rd, XFSClassData **root) {
  XFSMeta meta;
  rd.Read(meta.data);

  if (!meta->Get<XFSMeta::Active>()) {
    return;
  }

  PtrType chunkSize;
  const size_t strBegin = rd.Tell();
  rd.Read(chunkSize);

  auto &&desc = rtti.at(meta->Get<XFSMeta::LayoutIndex>());
  XFSClassData classData;
  classData.rtti = &desc;

  for (auto &d : desc.members) {
    XFSData cType;
    cType.rtti = &d;
    rd.Read(cType.numItems);

    if (cType.numItems == 1) {
      switch (d.type) {
      case XFSType::bool_:
      case XFSType::s8_:
      case XFSType::u8_:
        rd.Read(cType.data.asUInt8);
        break;
      case XFSType::s16_:
      case XFSType::u16_:
        rd.Read(cType.data.asUInt16);
        break;
      case XFSType::f32_:
      case XFSType::s32_:
      case XFSType::u32_:
        rd.Read(cType.data.asUInt32);
        break;
      case XFSType::s64_:
      case XFSType::u64_:
        rd.Read(cType.data.asUInt64);
        break;
      case XFSType::point_:
      case XFSType::size_:
        rd.Read(cType.data.asVector2);
        break;
      case XFSType::vector3_:
        rd.Read(cType.data.asVector3);
        break;
      case XFSType::vector4_:
      case XFSType::_vector4_:
        rd.Read(cType.data.asVector4);
        break;
      case XFSType::rect_:
        rd.Read(cType.data.asIVector4);
        break;
      case XFSType::color_:
        rd.Read(cType.data.asColor);
        break;
      case XFSType::string_:
      case XFSType::string2_: {
        std::string temp;
        rd.ReadString(temp);
        cType.SetString(temp);
        break;
      }
      case XFSType::_matrix_:
        rd.Read(*cType.AllocClass<es::Matrix44>());
        break;
      case XFSType::class_:
      case XFSType::classref_:
        ReadData<PtrType>(
            rd, reinterpret_cast<XFSClassData **>(&cType.data.asPointer));
        break;
      case XFSType::_resource_:
        rd.Read(*cType.AllocClass<XFSDataResource>());
        break;
      default:
        throw std::runtime_error("Undefined type at: " +
                                 std::to_string(rd.Tell()));
      }
    } else {
      switch (d.type) {
      case XFSType::bool_:
      case XFSType::s8_:
      case XFSType::u8_: {
        char *adata = cType.AllocArray<char>(cType.numItems);
        rd.ReadBuffer(adata, cType.numItems);
        break;
      }
      case XFSType::s16_:
      case XFSType::u16_: {
        uint16 *adata = cType.AllocArray<uint16>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::f32_:
      case XFSType::s32_:
      case XFSType::u32_: {
        uint32 *adata = cType.AllocArray<uint32>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::s64_:
      case XFSType::u64_: {
        uint64 *adata = cType.AllocArray<uint64>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::point_:
      case XFSType::size_: {
        Vector2 *adata = cType.AllocArray<Vector2>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::vector3_: {
        Vector2 *adata = cType.AllocArray<Vector2>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::vector4_:
      case XFSType::_vector4_: {
        Vector4A16 *adata = cType.AllocArray<Vector4A16>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::color_: {
        const size_t alocSize = cType.numItems * sizeof(UCVector4);
        char *adata = cType.AllocArray<char>(alocSize);
        rd.ReadBuffer(adata, alocSize);
        break;
      }
      case XFSType::string_: {
        throw std::runtime_error("Array string!");
      }
      case XFSType::_matrix_: {
        es::Matrix44 *adata = cType.AllocClasses<es::Matrix44>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          rd.Read(*adata++);
        }
        break;
      }
      case XFSType::class_:
      case XFSType::classref_: {
        auto adata = cType.AllocArray<XFSClassData *>(cType.numItems);
        for (size_t i = 0; i < cType.numItems; i++) {
          ReadData<PtrType>(rd, adata++);
        }
        break;
      }
      default:
        throw std::runtime_error("Undefined type at: " +
                                 std::to_string(rd.Tell()));
      }
    }

    classData.members.emplace_back(std::move(cType));
  }

  dataStore.emplace_back(std::move(classData));

  if (rd.Tell() != strBegin + chunkSize) {
    throw std::runtime_error("Chunk size mismatch!");
  }

  if (root) {
    *root = &dataStore.back();
  }
}

void XMLSetType(const XFSClassData &item, pugi::xml_node node) {
  auto attr = node.append_attribute("type");

  if (item.rtti->className.empty()) {
    char buffer[0x10];
    snprintf(buffer, sizeof(buffer), "h:%X", item.rtti->hash);
    attr.set_value(buffer);
    return;
  }

  attr.set_value(item.rtti->className.data());
}

void XFSImpl::RTTIToXML(pugi::xml_node node) {
  for (auto &c : rtti) {
    c.ToXML(node);
  }
}

void XFSImpl::ToXML(const XFSClassData &item, pugi::xml_node node) {
  static const auto refEnum = GetReflectedEnum<XFSType>();

  for (auto &m : item.members) {
    auto name = [&] {
      const size_t numEns = refEnum->numMembers;

      for (size_t i = 0; i < numEns; i++) {
        if (refEnum->values[i] == static_cast<uint64>(m.rtti->type)) {
          return refEnum->names[i];
        }
      }

      return "__UNREGISTERED__";
    }();

    if (m.numItems > 1) {
      auto cNode = node.append_child("array");
      cNode.append_attribute("name").set_value(m.rtti->name.data());
      cNode.append_attribute("type").set_value(name);
      cNode.append_attribute("count").set_value(m.numItems);

      switch (m.rtti->type) {
      case XFSType::class_:
      case XFSType::classref_: {
        auto adata =
            reinterpret_cast<const XFSClassData *const *>(m.data.asPointer);
        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          auto found = std::find_if(
              dataStore.begin(), dataStore.end(),
              [adata, i](auto &value) { return &value == adata[i]; });

          if (!es::IsEnd(dataStore, found)) {
            XMLSetType(*found, aNode);
            ToXML(*found, aNode);
          }
        }
        break;
      }
      case XFSType::u8_: {
        auto adata = reinterpret_cast<const uint8 *>(m.data.asPointer);

        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          aNode.append_attribute("value").set_value(adata[i]);
        }
        break;
      }
      case XFSType::s8_: {
        auto adata = reinterpret_cast<const int8 *>(m.data.asPointer);

        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          aNode.append_attribute("value").set_value(adata[i]);
        }
        break;
      }
      case XFSType::s32_: {
        auto adata = reinterpret_cast<const int32 *>(m.data.asPointer);

        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          aNode.append_attribute("value").set_value(adata[i]);
        }
        break;
      }
      case XFSType::u32_: {
        auto adata = reinterpret_cast<const uint32 *>(m.data.asPointer);

        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          aNode.append_attribute("value").set_value(adata[i]);
        }
        break;
      }
      case XFSType::f32_: {
        auto adata = reinterpret_cast<const float *>(m.data.asPointer);

        for (size_t i = 0; i < m.numItems; i++) {
          auto aNode = cNode.append_child(name);
          aNode.append_attribute("value").set_value(adata[i]);
        }
        break;
      }
      default:
        throw std::runtime_error("Unhandled xml array type");
      }
    } else if (m.numItems == 1) {
      auto cNode = node.append_child(name);
      cNode.append_attribute("name").set_value(m.rtti->name.data());
      auto value = cNode.append_attribute("value");

      switch (m.rtti->type) {
      case XFSType::bool_:
        value.set_value(m.data.asBool);
        break;
      case XFSType::s8_:
        value.set_value(m.data.asInt8);
        break;
      case XFSType::s16_:
        value.set_value(m.data.asInt16);
        break;
      case XFSType::s32_:
        value.set_value(m.data.asInt32);
        break;
      case XFSType::s64_:
        value.set_value(m.data.asInt64);
        break;
      case XFSType::u8_:
        value.set_value(m.data.asUInt8);
        break;
      case XFSType::u16_:
        value.set_value(m.data.asUInt16);
        break;
      case XFSType::u32_:
        value.set_value(m.data.asUInt32);
        break;
      case XFSType::u64_:
        value.set_value(m.data.asUInt64);
        break;
      case XFSType::string_:
      case XFSType::string2_:
        value.set_value(m.AsString());
        break;
      case XFSType::color_:
        value.set_name("r");
        value.set_value(m.data.asColor.X);
        cNode.append_attribute("g").set_value(m.data.asColor.Y);
        cNode.append_attribute("b").set_value(m.data.asColor.Z);
        cNode.append_attribute("a").set_value(m.data.asColor.W);
        break;
      case XFSType::f32_:
        value.set_value(m.data.asFloat);
        break;
      case XFSType::point_:
        value.set_name("x");
        value.set_value(m.data.asIVector2.X);
        cNode.append_attribute("y").set_value(m.data.asIVector2.Y);
        break;
      case XFSType::size_:
        value.set_name("w");
        value.set_value(m.data.asUIVector2.X);
        cNode.append_attribute("h").set_value(m.data.asUIVector2.Y);
        break;
      case XFSType::vector3_:
        value.set_name("x");
        value.set_value(m.data.asVector3.X);
        cNode.append_attribute("y").set_value(m.data.asVector3.Y);
        cNode.append_attribute("z").set_value(m.data.asVector3.Z);
        break;
      case XFSType::vector4_:
      case XFSType::_vector4_:
        value.set_name("x");
        value.set_value(m.data.asVector4.X);
        cNode.append_attribute("y").set_value(m.data.asVector4.Y);
        cNode.append_attribute("z").set_value(m.data.asVector4.Z);
        cNode.append_attribute("w").set_value(m.data.asVector4.W);
        break;
      case XFSType::rect_:
        value.set_name("x0");
        value.set_value(m.data.asIVector4.X);
        cNode.append_attribute("y0").set_value(m.data.asIVector4.Y);
        cNode.append_attribute("x1").set_value(m.data.asIVector4.Z);
        cNode.append_attribute("y1").set_value(m.data.asIVector4.W);
        break;
      case XFSType::class_:
      case XFSType::classref_: {
        auto found =
            std::find_if(dataStore.begin(), dataStore.end(), [&m](auto &value) {
              return &value == m.data.asPointer;
            });
        cNode.remove_attribute(value);

        if (!es::IsEnd(dataStore, found)) {
          XMLSetType(*found, cNode);
          ToXML(*found, cNode);
        }
        break;
      }
      case XFSType::_resource_: {
        auto adata = static_cast<const XFSDataResource *>(m.data.asPointer);
        value.set_name("type");
        value.set_value(adata->type.data());
        cNode.append_attribute("value").set_value(adata->file.data());
        break;
      }
      default:
        throw std::runtime_error("Unhandled xml type");
      }
    }
  }
}

void XFSImpl::ToXML(pugi::xml_node node) {
  auto rNode = node.append_child("class");
  auto &&rootData = *root;
  XMLSetType(rootData, rNode);
  ToXML(rootData, rNode);
}

#ifdef XFS_DEBUG
std::map<uint32, XFSClassDesc> rttiStore;
#endif

static constexpr uint32 XFSID = CompileFourCC("XFS");
static constexpr uint32 XFSIDBE = CompileFourCC("\0SFX");

template <class PtrType> void Load(XFSImpl &main, BinReaderRef_e rd) {
  XFSHeaderV1 header;
  rd.Read(header);
  rd.SetRelativeOrigin(rd.Tell(), false);
  std::vector<uint32> layoutOffsets;
  std::vector<XFSClass<PtrType>> layouts;
  rd.ReadContainer(layoutOffsets, header.numLayouts);
  rd.ReadContainer(layouts, header.numLayouts);
  rd.Seek(header.dataStart);

  std::transform(std::make_move_iterator(layouts.begin()),
                 std::make_move_iterator(layouts.end()),
                 std::back_inserter(main.rtti), [](auto &&item) {
                   if (item.info->template Get<XFSClassInfo::Unk>()) {
                     throw std::runtime_error("Some bullshit");
                   }

                   return std::move(item);
                 });
}

template <class PtrType> void LoadV2(XFSImpl &main, BinReaderRef_e rd, XFSHeaderV2 &header) {
  std::vector<PtrType> layoutOffsets;
  std::vector<XFSClassV2<PtrType>> layouts;
  rd.ReadContainer(layoutOffsets, header.numLayouts);
  rd.ReadContainer(layouts, header.numLayouts);
  rd.Seek(header.dataStart);

  std::transform(std::make_move_iterator(layouts.begin()),
                 std::make_move_iterator(layouts.end()),
                 std::back_inserter(main.rtti),
                 [](auto &&item) { return std::move(item); });
}

bool LoadV2(XFSImpl &main, BinReaderRef_e rd) {
  XFSHeaderV2 header;
  rd.Read(header);
  rd.SetRelativeOrigin(rd.Tell(), false);
  uint32 offset;
  rd.Read(offset);
  rd.Seek(0);
  const size_t expectedEndX64 = header.numLayouts * 8;

  if (offset == expectedEndX64) {
    LoadV2<uint64>(main, rd, header);
    return true;
  }

  LoadV2<uint32>(main, rd, header);
  return false;
}

void XFSImpl::Load(BinReaderRef_e rd) {
  using pt = Platform;
  XFSHeaderBase hdr;
  pt platform = pt::Win32;
  rd.Push();
  rd.Read(hdr);
  rd.Pop();

  if (hdr.id == XFSIDBE) {
    rd.SwapEndian(true);
    hdr.SwapEndian();
    platform = Platform::PS3;
  } else if (hdr.id != XFSID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  bool isX64 = false;

  if (hdr.version == 0xf || hdr.version == 0x10) {
    isX64 = ::LoadV2(*this, rd);
  } else {
    if (platform == pt::Win32) {
      ::Load<uint32>(*this, rd);
    } else if (platform == pt::PS3) {
      ::Load<uint64>(*this, rd);
    } else {
      throw std::runtime_error("Undefined platform!");
    }
  }

  for (auto &c : rtti) {
    c.className = GetClassName(c.hash, Platform::Win32);
#ifdef XFS_DEBUG
    if (c.className.empty() && !rttiStore.count(c.hash)) {
      rttiStore[c.hash] = c;
    }
#endif
  }

  if (isX64) {
    ReadData<uint64>(rd);
  } else {
    ReadData<uint32>(rd);
  }
  root = &dataStore.back();

  const size_t eof = rd.GetSize();

  if (eof != rd.Tell()) {
    throw std::runtime_error("Unexpected eof");
  }
}
