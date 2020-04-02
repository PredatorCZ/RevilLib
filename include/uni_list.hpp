#pragma once
#include "uni_virtual_iterator.hpp"

namespace uni {
template <class C> class List {
public:
  typedef C *pointer_type;
  typedef C value_type;

  virtual size_t Size() const = 0;
  virtual pointer_type At(size_t id) const = 0;

  typedef VirtualIterator<List, &List::Size, pointer_type, &List::At>
      iterator_type;

  iterator_type begin() const { return iterator_type(this, 0); }
  iterator_type end() const { return iterator_type(this); }

  pointer_type operator[](size_t id) { return At(id); }
};
} // namespace uni