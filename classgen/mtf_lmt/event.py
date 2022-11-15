import sys
import os.path as pt
sys.path.append(pt.dirname(pt.abspath(__file__)) + '/../')
from base import *


AnimEvent = NamedType('AnimEvent', 8, 4)
AnimEventsHeaderV2 = NamedType('AnimEventsHeaderV2', 8, 4)

AnimEvents = ClassData('AnimEvents')
AnimEvents.members = [
    ClassMember('eventRemaps', InlineArray(TYPES.uint16, 32)),
    ClassMember('numEvents', TYPES.uint32),
    ClassMember('events', Pointer(AnimEvent)),
]

AnimationEvent = ClassData('AnimationEvent')
AnimationEvent.members = [
    ClassMember('groups', InlineArray(AnimEvents, 2)),
]
AnimationEvent.patches = [
    ClassPatch('LMT56', ClassPatchType.replace, ClassMember('groups', InlineArray(AnimEvents, 4))),
    ClassPatch('LMT92', ClassPatchType.replace, ClassMember('groups', Pointer(AnimEventsHeaderV2))),
]

if __name__ == "__main__":
    CLASSES = [AnimEvents, AnimationEvent,]
    print("// This file has been automatically generated. Do not modify.")
    print('#include "lmt.inl"')
    dump_classes(CLASSES)
