import sys
import os.path as pt
sys.path.append(pt.dirname(pt.abspath(__file__)) + '/../')
from base import *

TrackV1BufferTypes = NamedType('TrackV1BufferTypes', 1, 1)
TrackType_er = NamedType('TrackType_er', 1, 1)
TrackV1_5BufferTypes = NamedType('TrackV1_5BufferTypes', 1, 1)
TrackV2BufferTypes = NamedType('TrackV2BufferTypes', 1, 1)
TrackMinMax = NamedType('TrackMinMax', 32, 4)

BoneTrack = ClassData('BoneTrack')
BoneTrack.members = [
    ClassMember('compression', TrackV1BufferTypes),
    ClassMember('trackType', TrackType_er),
    ClassMember('boneType', TYPES.uint8),
    ClassMember('boneID', TYPES.uint8),
    ClassMember('weight', TYPES.float),
    ClassMember('bufferSize', TYPES.uint32),
    ClassMember('buffer', Pointer(TYPES.char)),
]
BoneTrack.patches = [
    ClassPatch('LMT40', ClassPatchType.append, ClassMember('referenceData', TYPES.Vector4)),
    ClassPatch('LMT51', ClassPatchType.replace,  ClassMember('compression', TrackV1_5BufferTypes)),
    ClassPatch('LMT56', ClassPatchType.replace,  ClassMember('compression', TrackV2BufferTypes),
                        ClassPatchType.append, ClassMember('extremes', Pointer(TrackMinMax))),
    ClassPatch('LMT92', ClassPatchType.insert_after, 'boneID', ClassMember('boneID2', TYPES.int32)),
]

if __name__ == "__main__":
    CLASSES = [BoneTrack,]
    print("// This file has been automatically generated. Do not modify.")
    print('#include "lmt.inl"')
    dump_classes(CLASSES)
