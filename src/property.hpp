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

#pragma once
#include "pugixml.hpp"
#include "revil/types.hpp"
#include "spike/reflect/reflector.hpp"

MAKE_ENUM(ENUMSCOPE(class MtPropertyType : uint8, MtPropertyType), //
          EMEMBER(invalid),                                        //
          EMEMBERNAME(class_, "class"),                            //
          EMEMBER(classref),                                       //
          EMEMBERNAME(bool_, "bool"),                              //
          EMEMBERNAME(u8_, "u8"),                                  //
          EMEMBERNAME(u16_, "u16"),                                //
          EMEMBERNAME(u32_, "u32"),                                //
          EMEMBERNAME(u64_, "u64"),                                //
          EMEMBERNAME(s8_, "s8"),                                  //
          EMEMBERNAME(s16_, "s16"),                                //
          EMEMBERNAME(s32_, "s32"),                                //
          EMEMBERNAME(s64_, "s64"),                                //
          EMEMBERNAME(f32_, "f32"),                                //
          EMEMBERNAME(f64_, "f64"),                                //
          EMEMBERNAME(string_, "string"),                          //
          EMEMBER(color),                                          //
          EMEMBER(point),                                          //
          EMEMBER(size),                                           //
          EMEMBER(rect),                                           //
          EMEMBER(matrix44),                                       //
          EMEMBER(vector3),                                        //
          EMEMBER(vector4),                                        //
          EMEMBER(quaternion),                                     //
          EMEMBER(property),                                       //
          EMEMBER(event),                                          //
          EMEMBER(group),                                          //
          EMEMBER(pagebegin),                                      //
          EMEMBER(pageend),                                        //
          EMEMBER(event32),                                        //
          EMEMBER(array),                                          //
          EMEMBER(propertylist),                                   //
          EMEMBER(groupend),                                       //
          EMEMBER(cstring),                                        //
          EMEMBER(time),                                           //
          EMEMBER(float2),                                         //
          EMEMBER(float3),                                         //
          EMEMBER(float4),                                         //
          EMEMBER(float3x3),                                       //
          EMEMBER(float4x3),                                       //
          EMEMBER(float4x4),                                       //
          EMEMBER(easecurve),                                      //
          EMEMBER(line),                                           //
          EMEMBER(linesegnemt),                                    //
          EMEMBER(ray),                                            //
          EMEMBER(plane),                                          //
          EMEMBER(sphere),                                         //
          EMEMBER(capsule),                                        //
          EMEMBER(aabb),                                           //
          EMEMBER(obb),                                            //
          EMEMBER(cylinder),                                       //
          EMEMBER(triangle),                                       //
          EMEMBER(cone),                                           //
          EMEMBER(torus),                                          //
          EMEMBER(ellipsoid),                                      // ellpsoid
          EMEMBER(range),                                          //
          EMEMBER(rangef),                                         //
          EMEMBER(rangeu16),                                       //
          EMEMBER(hermitecurve),                                   //
          EMEMBER(enumlist),                                       //
          EMEMBER(float3x4),                                       //
          EMEMBER(linesegment4),                                   //
          EMEMBER(aabb4),                                          //
          EMEMBER(oscillator),                                     //
          EMEMBER(variable),                                       //
          EMEMBER(vector2),                                        //
          EMEMBER(matrix33),                                       //
          EMEMBER(rect3d_xz),                                      //
          EMEMBER(rect3d),                                         //
          EMEMBER(rect3d_collision),                               //
          EMEMBER(plane_xz),                                       //
          EMEMBER(ray_y),                                          //
          EMEMBER(pointf),                                         //
          EMEMBER(sizef),                                          //
          EMEMBER(rectf),                                          //
          EMEMBER(event64),                                        //
          EMEMBERVAL(custom, 0x80)                                 //
);

inline const char *PropType(MtPropertyType type) {
  static const auto refEnum = GetReflectedEnum<MtPropertyType>();
  return [&] {
    const size_t numEns = refEnum->numMembers;

    for (size_t i = 0; i < numEns; i++) {
      if (refEnum->values[i] == static_cast<uint64>(type)) {
        return refEnum->names[i];
      }
    }

    return "__UNREGISTERED__";
  }();
}

inline pugi::xml_node NewProperty(MtPropertyType type, pugi::xml_node &parent) {
  return parent.append_child(PropType(type));
}

inline pugi::xml_node NewArray(MtPropertyType type, pugi::xml_node parent,
                               const char *name, size_t count = 0) {
  pugi::xml_node arrayNode = NewProperty(MtPropertyType::array, parent);
  arrayNode.append_attribute("name").set_value(name);
  arrayNode.append_attribute("type").set_value(PropType(type));
  arrayNode.append_attribute("count").set_value(count);
  return arrayNode;
}

template <class C>
pugi::xml_node NewPrimitive(pugi::xml_node parent, const char *name, C value) {
  pugi::xml_node retval;

  if constexpr (std::is_same_v<bool, C>) {
    retval = NewProperty(MtPropertyType::bool_, parent);
  } else if constexpr (std::is_integral_v<C>) {
    if constexpr (std::is_signed_v<C>) {
      if constexpr (sizeof(C) == 1) {
        retval = NewProperty(MtPropertyType::s8_, parent);
      } else if constexpr (sizeof(C) == 2) {
        retval = NewProperty(MtPropertyType::s16_, parent);
      } else if constexpr (sizeof(C) == 4) {
        retval = NewProperty(MtPropertyType::s32_, parent);
      } else if constexpr (sizeof(C) == 8) {
        retval = NewProperty(MtPropertyType::s64_, parent);
      }
    } else {
      if constexpr (sizeof(C) == 1) {
        retval = NewProperty(MtPropertyType::u8_, parent);
      } else if constexpr (sizeof(C) == 2) {
        retval = NewProperty(MtPropertyType::u16_, parent);
      } else if constexpr (sizeof(C) == 4) {
        retval = NewProperty(MtPropertyType::u32_, parent);
      } else if constexpr (sizeof(C) == 8) {
        retval = NewProperty(MtPropertyType::u64_, parent);
      }
    }
  } else if constexpr (std::is_floating_point_v<C>) {
    if constexpr (sizeof(C) == 4) {
      retval = NewProperty(MtPropertyType::f32_, parent);
    } else {
      retval = NewProperty(MtPropertyType::f64_, parent);
    }
  }

  if (name) {
    retval.append_attribute("name").set_value(name);
  }
  retval.append_attribute("value").set_value(value);

  return retval;
}

template <>
inline pugi::xml_node NewPrimitive(pugi::xml_node parent, const char *name,
                                   MtVector3 value) {
  pugi::xml_node retval = NewProperty(MtPropertyType::vector3, parent);
  retval.append_attribute("name").set_value(name);
  retval.append_attribute("x").set_value(value.x);
  retval.append_attribute("y").set_value(value.y);
  retval.append_attribute("z").set_value(value.z);

  return retval;
}

template <>
inline pugi::xml_node NewPrimitive(pugi::xml_node parent, const char *name,
                                   MtVector4 value) {
  pugi::xml_node retval = NewProperty(MtPropertyType::vector4, parent);
  retval.append_attribute("name").set_value(name);
  retval.append_attribute("x").set_value(value.x);
  retval.append_attribute("y").set_value(value.y);
  retval.append_attribute("z").set_value(value.z);
  retval.append_attribute("w").set_value(value.w);

  return retval;
}

struct uint64a4 {
  uint32 lo;
  uint32 hi;

  operator uint64() const { return lo | (uint64(hi) << 32); }
};

template <>
inline pugi::xml_node NewPrimitive(pugi::xml_node parent, const char *name,
                                   uint64a4 value) {

  return NewPrimitive(parent, name, uint64(value));
}

template <>
inline pugi::xml_node NewPrimitive(pugi::xml_node parent, const char *name,
                                   MtFloat3 value) {
  pugi::xml_node retval = NewProperty(MtPropertyType::vector3, parent);
  retval.append_attribute("name").set_value(name);
  retval.append_attribute("x").set_value(value.x);
  retval.append_attribute("y").set_value(value.y);
  retval.append_attribute("z").set_value(value.z);

  return retval;
}
