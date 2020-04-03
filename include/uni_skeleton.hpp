#pragma once
#include "datas/Matrix44.hpp"
#include "uni_list.hpp"

namespace uni {

class Bone {
public:
  virtual esMatrix44 Transform() const = 0;
  virtual const Bone *Parent() const = 0;
  // A special bone identicator, this is not a bone index within skeleton
  virtual size_t Index() const = 0;
  virtual std::string Name() const = 0;
};

typedef List<Bone> Bones;

class Skeleton {
public:
  virtual const Bones &Bones() const = 0;
  virtual std::string Name() const = 0;

  Bones::iterator_type begin() const { return Bones().begin(); }
  Bones::iterator_type end() const { return Bones().end(); }
};
} // namespace uni
