import sys
import os.path as pt
sys.path.append(pt.dirname(pt.abspath(__file__)) + '/../')
from lmt import *

FloatFrame = NamedType('FloatFrame', 16, 4)
FloatTrackComponentRemap = NamedType('FloatTrackComponentRemap', 1, 1)

FloatTrack = ClassData('FloatTrack')
FloatTrack.members = [
    ClassMember('componentRemaps', InlineArray(FloatTrackComponentRemap, 4)),
    ClassMember('numFloats', TYPES.uint32),
    ClassMember('frames', Pointer(FloatFrame)),
]

FloatTracks = ClassData('FloatTracks')
FloatTracks.members = [
    ClassMember('groups', InlineArray(FloatTrack, 4)),
]

if __name__ == "__main__":
    CLASSES = [FloatTrack, FloatTracks,]
    print("// This file has been automatically generated. Do not modify.")
    print('#include "lmt.inl"')
    dump_classes(CLASSES)
