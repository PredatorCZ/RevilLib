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

template <> void ProcessClass(RETrackCurve78 &item, ProcessFlags flags) {
  es::FixupPointers(flags.base, *flags.ptrStore, item.frames,
                    item.controlPoints, item.minMaxBounds);
}

template <> void ProcessClass(REMotionTrack78 &item, ProcessFlags flags) {
  if (!es::FixupPointers(flags.base, *flags.ptrStore, item.curves)) {
    return;
  }

  uint32 numUsedCurves = 0;

  for (uint32 t = 0; t < 3; t++) {
    if (item.usedCurves[static_cast<REMotionTrack43::TrackType>(t)]) {
      ProcessClass(item.curves[numUsedCurves++], flags);
    }
  }
}

template <> void ProcessClass(REMotion78 &item, ProcessFlags flags) {
  flags.base = reinterpret_cast<char *>(&item);
  if (!es::FixupPointers(flags.base, *flags.ptrStore, item.tracks,
                         item.unkOffset02, item.animationName)) {
    return;
  }

  for (size_t b = 0; b < item.numTracks; b++) {
    ProcessClass(item.tracks[b], flags);
  }
}

void REMotion78Asset::Build() {
  const size_t numTracks = Get().numTracks;

  for (size_t t = 0; t < numTracks; t++) {
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

void REMotion78Asset::Fixup(std::vector<void *> &ptrStore) {
  ProcessFlags flags;
  flags.ptrStore = &ptrStore;
  ProcessClass(Get(), flags);
  Build();
}
