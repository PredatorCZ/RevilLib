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

int REMotion43::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  bones.Fixup(masterBuffer);

  if (bones) {
    bones->ptr.Fixup(masterBuffer);
  }
  tracks.Fixup(masterBuffer);
  unkOffset02.Fixup(masterBuffer);
  animationName.Fixup(masterBuffer);

  for (uint32 b = 0; b < numBones; b++)
    bones->ptr[b].Fixup(masterBuffer);

  for (uint32 b = 0; b < numTracks; b++)
    tracks[b].Fixup(masterBuffer);

  return 0;
}

int REMotionTrack43::Fixup(char *masterBuffer) {
  curves.Fixup(masterBuffer);

  uint32 numUsedCurves = 0;

  for (uint32 t = 0; t < 3; t++)
    if (usedCurves[static_cast<TrackType>(t)])
      curves[numUsedCurves++].Fixup(masterBuffer);

  return 0;
}

int REMotionBone::Fixup(char *masterBuffer) {
  boneName.Fixup(masterBuffer);
  parentBoneNamePtr.Fixup(masterBuffer);
  firstChildBoneNamePtr.Fixup(masterBuffer);
  lastChildBoneNamePtr.Fixup(masterBuffer);

  return 0;
}

int RETrackCurve::Fixup(char *masterBuffer) {
  frames.Fixup(masterBuffer);
  controlPoints.Fixup(masterBuffer);
  minMaxBounds.Fixup(masterBuffer);

  return 0;
}

void REMotion43Asset::FrameRate(uint32 fps) {
  throw std::logic_error("Unsupported call!");
}

void REMotionTrackWorker::GetValue(uni::RTSValue &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionTrackWorker::GetValue(esMatrix44 &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionTrackWorker::GetValue(float &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionTrackWorker::GetValue(Vector4A16 &output, float time) const {
  if (!controller)
    return;

  if (time <= 0.0f || numFrames == 1) {
    controller->Evaluate(0, output);
    return;
  }

  float frameDelta = time * 60.f;
  uint32 frame = static_cast<uint32>(frameDelta);
  uint32 foundFrameID = 0;
  uint32 frameBegin = 0;
  uint32 frameEnd = 0;

  for (; foundFrameID < numFrames; foundFrameID++) {
    frameBegin = frameEnd;
    frameEnd = controller->GetFrame(foundFrameID);

    if (frameEnd == frame) {
      frameBegin = frameEnd;
      break;
    } else if (frameEnd > frame) {
      foundFrameID--;
      break;
    }
  }

  if (foundFrameID >= numFrames) {
    controller->Evaluate(foundFrameID - 1, output);
    return;
  }

  const float fFrameBegin = static_cast<float>(frameBegin);
  const float fFrameEnd = static_cast<float>(frameEnd);

  frameDelta = (fFrameBegin - frameDelta) / (fFrameBegin - fFrameEnd);

  controller->Evaluate(foundFrameID, output);

  if (frameDelta > FLT_EPSILON) {
    Vector4A16 nextValue;
    controller->Evaluate(foundFrameID + 1, nextValue);
    output = output + (nextValue - output) * frameDelta;
  }
}

void REMotion43Asset::Build() {
  const uint32 numTracks = Get().numTracks;

  for (uint32 t = 0; t < numTracks; t++) {
    auto tck = Get().tracks.operator->() + t;
    size_t curCurve = 0;

    if (tck->usedCurves[REMotionTrack43::TrackType_Position]) {
      auto *wk = new REMotionTrackWorker();
      auto data = &tck->curves[curCurve++];
      wk->controller =
          std::unique_ptr<RETrackController>(data->GetController());
      wk->cType = REMotionTrackWorker::Position;
      wk->boneHash = tck->boneHash;
      wk->numFrames = data->numFrames;
      storage.emplace_back(wk);
    }

    if (tck->usedCurves[REMotionTrack43::TrackType_Rotation]) {
      auto *wk = new REMotionTrackWorker();
      auto data = &tck->curves[curCurve++];
      wk->controller =
          std::unique_ptr<RETrackController>(data->GetController());
      wk->cType = REMotionTrackWorker::Rotation;
      wk->boneHash = tck->boneHash;
      wk->numFrames = data->numFrames;
      storage.emplace_back(wk);
    }

    if (tck->usedCurves[REMotionTrack43::TrackType_Scale]) {
      auto *wk = new REMotionTrackWorker();
      auto data = &tck->curves[curCurve++];
      wk->controller =
          std::unique_ptr<RETrackController>(data->GetController());
      wk->cType = REMotionTrackWorker::Scale;
      wk->boneHash = tck->boneHash;
      wk->numFrames = data->numFrames;
      storage.emplace_back(wk);
    }
  }
}

int REMotion43Asset::Fixup() {
  int retVal = Get().Fixup();
  Build();

  return retVal;
}