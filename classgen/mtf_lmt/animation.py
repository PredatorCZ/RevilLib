import sys
import os.path as pt
sys.path.append(pt.dirname(pt.abspath(__file__)) + '/../')
from event import *
from float_track import FloatTracks

AnimV2Flags = NamedType('es::Flags<AnimV2Flags>', 4, 4)

Animation = ClassData('Animation')
Animation.members = [
    ClassMember('tracks', Pointer(TYPES.char)),
    ClassMember('numTracks', TYPES.uint32),
    ClassMember('numFrames', TYPES.uint32),
    ClassMember('loopFrame', TYPES.int32),
    ClassMember('endFrameAdditiveScenePosition', TYPES.Vector4A16),
    ClassMember('events', AnimationEvent),
]
Animation.patches = [
    ClassPatch('LMT40', ClassPatchType.insert_after, 'endFrameAdditiveScenePosition', ClassMember('endFrameAdditiveSceneRotation', TYPES.Vector4A16)),
    ClassPatch('LMT66', ClassPatchType.insert_after, 'endFrameAdditiveSceneRotation', ClassMember('flags', AnimV2Flags),
                        ClassPatchType.replace, ClassMember('events', Pointer(AnimationEvent)),
                        ClassPatchType.append, ClassMember('floats', Pointer(FloatTracks))),
    ClassPatch('LMT92', ClassPatchType.delete, 'floats', ClassPatchType.insert_after, 'flags', ClassMember('nullPtr', InlineArray(Pointer(TYPES.char), 2))),
]

if __name__ == "__main__":
    CLASSES = [Animation,]
    print("// This file has been automatically generated. Do not modify.")
    print('#include "event.inl"')
    print('#include "float_track.inl"')
    dump_classes(CLASSES)
