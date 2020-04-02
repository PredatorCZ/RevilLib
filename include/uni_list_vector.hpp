#pragma once
#include "uni_list.hpp"
#include <memory>
#include <vector>

namespace uni {
template <class _interface_type, class _class_type>
class VectorList : public List<_interface_type> {
public:
  typedef _interface_type interface_type;
  typedef _class_type class_type;

  size_t Size() const override { return storage.size(); }
  pointer_type At(size_t id) const override {
    return dynamic_cast<pointer_type>(storage.at(id).get());
  }

  typedef VirtualIterator<List, &List::Size, pointer_type, &List::At>
      iterator_type;

  const iterator_type begin() const { return iterator_type(this, 0); }
  const iterator_type end() const { return iterator_type(this); }

  pointer_type operator[](size_t id) { return At(id); }

  std::vector<std::unique_ptr<class_type>> storage;
};
} // namespace uni