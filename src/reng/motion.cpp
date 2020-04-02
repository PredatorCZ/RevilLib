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

#include "motion.hpp"

int REMotion::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  bones.Fixup(masterBuffer);
  bones->ptr.Fixup(masterBuffer);
  tracks.Fixup(masterBuffer);
  unkOffset02.Fixup(masterBuffer);
  animationName.Fixup(masterBuffer);

  for (int b = 0; b < numBones; b++)
    bones->ptr[b].Fixup(masterBuffer);

  for (int b = 0; b < numTracks; b++)
    tracks[b].Fixup(masterBuffer);

  return 0;
}

int REMotionTrack::Fixup(char *masterBuffer) {
  curves.Fixup(masterBuffer);

  int numUsedCurves = 0;

  for (int t = 0; t < 3; t++)
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

void REMotionAsset::FrameRate(uint fps) {
  throw std::logic_error("Unsupported call!");
}

void REMotionCurveWorker::GetValue(uni::PRSCurve &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionCurveWorker::GetValue(esMatrix44 &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionCurveWorker::GetValue(float &output, float time) const {
  throw std::logic_error("Unsupported call!");
}

void REMotionCurveWorker::GetValue(Vector4A16 &output, float time) const {
  if (time < 0.0f) {
    time = 0.0f;
  }

  float delta = time * 60.f;
  uint frame = static_cast<uint>(delta);
  delta -= frame;
  uint foundFrameID = 0;

  for (; foundFrameID < numFrames; foundFrameID++) {
    uint cFrame = controller->GetFrame(foundFrameID);

    if (cFrame == frame) {
      break;
    } else if (cFrame > frame) {
      foundFrameID--;
      break;
    }
  }

  if (foundFrameID + 1 == numFrames) {
    delta = 0.0f;
  }

  controller->Evaluate(foundFrameID, output);

  if (delta > FLT_EPSILON) {
    Vector4A16 nextValue;
    controller->Evaluate(foundFrameID + 1, nextValue);
    output = output + (nextValue - output) * delta;
  }
}

REMotionTrackWorker::REMotionTrackWorker(REMotionTrack *tck) {
  size_t curCurve = 0;
  boneHash = tck->boneHash;

  if (tck->usedCurves[REMotionTrack::TrackType_Position]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller = std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Position;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }

  if (tck->usedCurves[REMotionTrack::TrackType_Rotation]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller = std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Rotation;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }

  if (tck->usedCurves[REMotionTrack::TrackType_Scale]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller = std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Scale;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }
}

void REMotionAsset::Build() {
  auto &data = Get();
  const int numTracks = Get().numTracks;

  for (int t = 0; t < numTracks; t++) {
    storage.emplace_back(std::unique_ptr<REMotionTrackWorker>(
        new REMotionTrackWorker(Get().tracks.operator->() + t)));
  }
}

int REMotionAsset::Fixup() {
  int retVal = Get().Fixup();
  Build();

  return retVal;
}