/*  Revil Format Library
    Copyright(C) 2017-2020 Lukas Cone

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

#include "motion_43.hpp"

template <> void REMotion43::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  if (!es::FixupPointers(masterBuffer, ptrStore, bones, tracks, unkOffset02,
                         animationName)) {
    return;
  }

  if (bones) {
    bones->ptr.Fixup(masterBuffer);
  }

  for (size_t b = 0; b < numBones; b++) {
    bones->ptr[b].Fixup(masterBuffer);
  }

  for (size_t b = 0; b < numTracks; b++) {
    tracks[b].Fixup(masterBuffer);
  }
}

void REMotionTrack43::Fixup(char *masterBuffer) {
  if (!es::FixupPointers(masterBuffer, ptrStore, curves)) {
    return;
  }

  uint32 numUsedCurves = 0;

  for (uint32 t = 0; t < 3; t++) {
    if (usedCurves[static_cast<TrackType>(t)]) {
      curves[numUsedCurves++].Fixup(masterBuffer);
    }
  }
}

void REMotionBone::Fixup(char *masterBuffer) {
  es::FixupPointers(masterBuffer, ptrStore, boneName, parentBoneNamePtr,
                    firstChildBoneNamePtr, lastChildBoneNamePtr);
}

void RETrackCurve43::Fixup(char *masterBuffer) {
  es::FixupPointers(masterBuffer, ptrStore, frames, controlPoints,
                    minMaxBounds);
}

// https://en.wikipedia.org/wiki/Slerp
static Vector4A16 slerp(const Vector4A16 &v0, const Vector4A16 &_v1, float t) {
  Vector4A16 v1 = _v1;
  float dot = v0.Dot(v1);

  // If the dot product is negative, slerp won't take
  // the shorter path. Fix by reversing one quaternion.
  if (dot < 0.0f) {
    v1 *= -1;
    dot *= -1;
  }

  static const float DOT_THRESHOLD = 0.9995f;
  if (dot > DOT_THRESHOLD) {
    // If the inputs are too close for comfort, linearly interpolate
    // and normalize the result.

    Vector4A16 result = v0 + (v1 - v0) * t;
    return result.Normalize();
  }

  const float theta00 = acos(dot);   // theta00 = angle between input vectors
  const float theta01 = theta00 * t; // theta01 = angle between v0 and result
  const float theta02 = sin(theta01);
  const float theta03 = 1.0f / sin(theta00);
  const float s0 = cos(theta01) - dot * theta02 * theta03;
  const float s1 = theta02 * theta03;

  return (v0 * s0) + (v1 * s1);
}

void REMotionTrackWorker::GetValue(Vector4A16 &output, float time) const {
  if (!controller) {
    return;
  }

  if (time <= 0.0f || numFrames == 1) {
    controller->Evaluate(0, output);
    return;
  }

  float frameDelta = time * 60.f;
  auto span = controller->GetSpan(frameDelta);

  if (span.offset >= numFrames) {
    controller->Evaluate(numFrames - 1, output);
    return;
  }

  const float fFrameBegin = static_cast<float>(span.first);
  const float fFrameEnd = static_cast<float>(span.second);

  if (span.first == span.second) {
    frameDelta = 0.f;
  } else {
    frameDelta = (fFrameBegin - frameDelta) / (fFrameBegin - fFrameEnd);
  }

  controller->Evaluate(span.offset - 1, output);

  if (frameDelta > FLT_EPSILON) {
    Vector4A16 nextValue;
    controller->Evaluate(span.offset, nextValue);

    if (cType == TrackType_e::Rotation) {
      output = slerp(output, nextValue, frameDelta);
    } else {
      output = output + (nextValue - output) * frameDelta;
    }
  }
}

void REMotion43Asset::Build() {
  const uint32 numTracks = Get().numTracks;

  for (uint32 t = 0; t < numTracks; t++) {
    auto tck = Get().tracks.operator->() + t;
    size_t curCurve = 0;

    if (tck->usedCurves[REMotionTrack43::TrackType_Position]) {
      REMotionTrackWorker wk;
      auto data = &tck->curves[curCurve++];
      wk.controller = data->GetController();
      wk.cType = REMotionTrackWorker::Position;
      wk.boneHash = tck->boneHash;
      wk.numFrames = data->numFrames;
      storage.emplace_back(std::move(wk));
    }

    if (tck->usedCurves[REMotionTrack43::TrackType_Rotation]) {
      REMotionTrackWorker wk;
      auto data = &tck->curves[curCurve++];
      wk.controller = data->GetController();
      wk.cType = REMotionTrackWorker::Rotation;
      wk.boneHash = tck->boneHash;
      wk.numFrames = data->numFrames;
      storage.emplace_back(std::move(wk));
    }

    if (tck->usedCurves[REMotionTrack43::TrackType_Scale]) {
      REMotionTrackWorker wk;
      auto data = &tck->curves[curCurve++];
      wk.controller = data->GetController();
      wk.cType = REMotionTrackWorker::Scale;
      wk.boneHash = tck->boneHash;
      wk.numFrames = data->numFrames;
      storage.emplace_back(std::move(wk));
    }
  }
}

void REMotion43Asset::Fixup() {
  Get().Fixup();
  Build();
}

namespace revil {
template <> ES_EXPORT uni::Element<const uni::Motion> REAsset::As<>() const {
  auto val = i->AsMotion();
  return {static_cast<const uni::Motion *>(val.release()), false};
}

template <> ES_EXPORT uni::MotionsConst REAsset::As<>() const {
  auto val = i->AsMotions();
  return {static_cast<typename uni::MotionsConst::pointer>(val.release()),
          false};
}
} // namespace revil
