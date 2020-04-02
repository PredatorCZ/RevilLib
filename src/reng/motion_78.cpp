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

#include "motion_78.hpp"

int REMotion78::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  tracks.Fixup(masterBuffer);
  unkOffset02.Fixup(masterBuffer);
  animationName.Fixup(masterBuffer);

  for (int b = 0; b < numTracks; b++)
    tracks[b].Fixup(masterBuffer);

  return 0;
}

int REMotionTrack78::Fixup(char *masterBuffer) {
  curves.Fixup(masterBuffer);

  int numUsedCurves = 0;

  for (int t = 0; t < 3; t++)
    if (usedCurves[static_cast<REMotionTrack::TrackType>(t)])
      curves[numUsedCurves++].Fixup(masterBuffer);

  return 0;
}

int RETrackCurve78::Fixup(char *masterBuffer) {
  frames.Fixup(masterBuffer);
  controlPoints.Fixup(masterBuffer);
  minMaxBounds.Fixup(masterBuffer);

  return 0;
}

REMotionTrackWorker::REMotionTrackWorker(REMotionTrack78 *tck) {
  size_t curCurve = 0;
  boneHash = tck->boneHash;

  if (tck->usedCurves[REMotionTrack::TrackType_Position]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller =
        std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Position;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }

  if (tck->usedCurves[REMotionTrack::TrackType_Rotation]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller =
        std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Rotation;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }

  if (tck->usedCurves[REMotionTrack::TrackType_Scale]) {
    storage.emplace_back(
        std::unique_ptr<REMotionCurveWorker>(new REMotionCurveWorker()));
    REMotionCurveWorker *wk = std::prev(storage.end())->get();
    auto data = &tck->curves[curCurve++];
    wk->controller =
        std::unique_ptr<RETrackController>(data->GetController());
    wk->cType = REMotionCurveWorker::Scale;
    wk->boneHash = tck->boneHash;
    wk->numFrames = data->numFrames;
  }
}

void REMotion78Asset::Build() {
  const int numTracks = Get().numTracks;

  for (int t = 0; t < numTracks; t++) {
    storage.emplace_back(std::unique_ptr<REMotionTrackWorker>(
        new REMotionTrackWorker(Get().tracks.operator->() + t)));
  }
}

int REMotion78Asset::Fixup() {
  int retVal = Get().Fixup();
  Build();

  return retVal;
}