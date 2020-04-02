#pragma once
#include "datas/Matrix44.hpp"
#include "uni_list.hpp"

namespace uni {

struct PRSCurve {
  Vector4A16 position;
  Vector4A16 rotation;
  Vector4A16 scale;
};

class MotionCurve {
public:
  enum CurveType_e {
    Position,
    Rotation,
    Scale,
    Matrix,
    PositionRotationScale,
    SingleFloat
  };
  virtual CurveType_e CurveType() const = 0;
  virtual size_t BoneIndex() const = 0;
  virtual void GetValue(PRSCurve &output, float time) const = 0;
  virtual void GetValue(esMatrix44 &output, float time) const = 0;
  virtual void GetValue(Vector4A16 &output, float time) const = 0;
  virtual void GetValue(float &output, float time) const = 0;
};

typedef List<MotionCurve> MotionCurves;

// Each track can contains multiple curves, one for Rotation, one for Position,
// etc. 
class MotionTrack {
public:
  virtual const MotionCurves &Curves() const = 0;
  virtual size_t Index() const = 0;

  MotionCurves::iterator_type begin() const { return Curves().begin(); }
  MotionCurves::iterator_type end() const { return Curves().end(); }
};

typedef List<MotionTrack> MotionTracks;

class Motion {
public:
  virtual std::string Name() const = 0;
  virtual void FrameRate(uint fps) = 0;
  virtual uint FrameRate() const = 0;
  virtual float Duration() const = 0;
  virtual const MotionTracks &Tracks() const = 0;

  MotionTracks::iterator_type begin() const { return Tracks().begin(); }
  MotionTracks::iterator_type end() const { return Tracks().end(); }
};
} // namespace uni