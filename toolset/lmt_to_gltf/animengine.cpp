#include "animengine.hpp"
#include "glm/gtx/quaternion.hpp"
#include "revil/mot.hpp"
#include "spike/gltf.hpp"

void WalkTree(AnimEngine &eng, GLTF &main, gltf::Node &glNode, int32 parent) {
  auto found = glNode.name.find(':');
  AnimNode *aNode = nullptr;
  int32 animNodeId = -1;

  if (glNode.name.ends_with("_s")) {
    return;
  }

  if (found != std::string::npos) {
    animNodeId = std::atol(glNode.name.data() + found + 1);
  }

  if (!eng.nodes.contains(animNodeId)) {
    AnimNode anNode;
    anNode.glNodeIndex = std::distance(main.nodes.data(), &glNode);
    aNode = &eng.nodes.emplace(animNodeId, anNode).first->second;
  }

  aNode = &eng.nodes.at(animNodeId);

  memcpy((void *)&aNode->refPosition, glNode.translation.data(), 12);
  memcpy((void *)&aNode->refRotation, glNode.rotation.data(), 16);
  aNode->magnitude = aNode->refPosition.Length();
  aNode->parentAnimNode = parent;

  if (parent != animNodeId) {
    eng.nodes.at(parent).children.push_back(animNodeId);
  }

  parent = animNodeId;
  std::string sName = glNode.name + "_s";

  for (auto childId : glNode.children) {
    if (main.nodes.at(childId).name == sName) {
      aNode->glScaleNodeIndex = childId;
      break;
    }

    WalkTree(eng, main, main.nodes.at(childId), parent);
  }
}

void LinkNodes(AnimEngine &eng, GLTF &main) {
  for (auto &node : main.nodes) {
    if (node.name == "reference") {
      WalkTree(eng, main, node, -1);
      return;
    }
  }
}

void InheritScales(AnimEngine &eng, size_t animNode) {
  auto &aNode = eng.nodes.at(animNode);

  if (aNode.parentAnimNode >= 0) {
    auto &aParentNode = eng.nodes.at(aNode.parentAnimNode);

    if (!aParentNode.scales.empty()) {
      const size_t numFrames = aParentNode.scales.size();

      if (aNode.scales.empty()) {
        aNode.scales = aParentNode.scales;
      } else {
        for (size_t f = 0; f < numFrames; f++) {
          aNode.scales.at(f) *= aParentNode.scales.at(f);
        }
      }

      if (aNode.positions.empty()) {
        aNode.positions.insert(aNode.positions.begin(), numFrames,
                               aNode.refPosition);
      }

      for (size_t f = 0; f < numFrames; f++) {
        aNode.positions.at(f) *= aParentNode.scales.at(f);
      }
    }
  }

  for (auto &child : aNode.children) {
    InheritScales(eng, child);
  }
}

void MarkHierarchy(AnimEngine &eng, Hierarchy &marks, size_t endNode) {
  if (auto parentIndex = eng.nodes.at(endNode).parentAnimNode;
      parentIndex >= 0) {
    marks.emplace(parentIndex);
    MarkHierarchy(eng, marks, parentIndex);
  }
}

Vector4A16 TransformPoint(Vector4A16 q, Vector4A16 point) {
  glm::quat q_(q.w, q.x, q.y, q.z);
  glm::vec3 p_(point.x, point.y, point.z);
  auto res = q_ * p_;

  return Vector4A16(res.x, res.y, res.z, 0);
}

Vector4A16 Multiply(Vector4A16 child, Vector4A16 parent) {
  glm::quat p_(parent.w, parent.x, parent.y, parent.z);
  glm::quat c_(child.w, child.x, child.y, child.z);
  auto res = p_ * c_;

  return Vector4A16(res.x, res.y, res.z, res.w);
}

Vector4A16 LookAt(Vector4A16 center, Vector4A16 eye) {
  glm::vec3 center_(center.x, center.y, center.z);
  glm::vec3 eye_(eye.x, eye.y, eye.z);
  glm::quat res(glm::lookAtRH(eye_, center_, glm::vec3(0, 0, -1)));

  return Vector4A16(res.x, res.y, res.z, res.w);
}

Vector4A16 LookAt(Vector4A16 dir) {
  glm::vec3 dir_(dir.x, dir.y, dir.z);
  glm::quat res(glm::quatLookAt(dir_, glm::vec3(0, 0, 1)));

  return Vector4A16(res.x, res.y, res.z, res.w);
}

/*
Vector4A16 TransformPoint(Vector4A16 q, Vector4A16 point) {
  Vector4A16 qvec = q * Vector4A16(1, 1, 1, 0);
  Vector4A16 uv = qvec.Cross(point);
  Vector4A16 uuv = qvec.Cross(uv);

  return point + ((uv * q.x) + uuv) * 2;
}

Vector4A16 Multiply(Vector4A16 parent, Vector4A16 child) {
  Vector4A16 tier0 = Vector4A16(child.x, child.y, child.z, child.w) * parent.w;
  Vector4A16 tier1 = Vector4A16(child.w, child.w, child.w, -child.x) *
                     Vector4A16(parent.x, parent.y, parent.z, parent.x);
  Vector4A16 tier2 = Vector4A16(child.z, child.x, child.y, -child.y) *
                     Vector4A16(parent.y, parent.z, parent.x, parent.y);
  Vector4A16 tier3 = Vector4A16(child.y, child.z, child.x, child.z) *
                     Vector4A16(parent.z, parent.x, parent.y, parent.z);

  return tier0 + tier1 + tier2 - tier3;
}*/

void GlobalResample(AnimEngine &eng, size_t nodeId) {
  auto &aNode = eng.nodes.at(nodeId);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);

  aNode.globalPositions = parentNode.globalPositions;
  aNode.globalRotations.resize(eng.numSamples);

  if (aNode.positions.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.refPosition);
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.positions.at(s));
    }
  }

  if (aNode.rotations.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) = Pack(Multiply(
          Unpack(parentNode.globalRotations.at(s)), aNode.refRotation));
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) =
          Pack(Multiply(Unpack(parentNode.globalRotations.at(s)),
                        Unpack(aNode.rotations.at(s))));
    }
  }
}

void MakeGlobalFrames(AnimEngine &eng, Hierarchy &marks, size_t nodeId) {
  GlobalResample(eng, nodeId);
  auto &aNode = eng.nodes.at(nodeId);

  for (auto child : aNode.children) {
    if (marks.contains(child)) {
      MakeGlobalFrames(eng, marks, child);
    }
  }
}

void MakeGlobalFrames(AnimEngine &eng, Hierarchy &marks) {
  auto &refNode = eng.nodes.at(size_t(-1));

  if (refNode.rotations.empty()) {
    refNode.rotations.insert(refNode.rotations.begin(), eng.numSamples,
                             SVector4(0, 0, 0, 0x7fff));
  }

  if (refNode.positions.empty()) {
    refNode.positions.resize(eng.numSamples);
  }

  refNode.globalPositions = refNode.positions;
  refNode.globalRotations = refNode.rotations;

  for (auto child : refNode.children) {
    if (marks.contains(child)) {
      MakeGlobalFrames(eng, marks, child);
    }
  }
}

// Apply node's local transform to different parent
void RelativeResample(AnimEngine &eng, int32 bone, int32 parentBone) {
  auto &aNode = eng.nodes.at(bone);
  auto &parentNode = eng.nodes.at(parentBone);
  auto &efNode = bone > 1 && eng.nodes.contains(-bone) ? eng.nodes.at(-bone)
                                                       : eng.nodes.at(bone);

  aNode.globalPositions = parentNode.globalPositions;
  aNode.globalRotations.resize(eng.numSamples);

  if (efNode.positions.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), efNode.refPosition);
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), efNode.positions.at(s));
    }
  }

  if (efNode.rotations.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) = Pack(Multiply(
          efNode.refRotation, Unpack(parentNode.globalRotations.at(s))));
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) =
          Pack(Multiply(Unpack(efNode.rotations.at(s)),
                        Unpack(parentNode.globalRotations.at(s))));
    }
  }
}

// Apply node's local transform to different parent
void InverseRelativeResample(AnimEngine &eng, int32 bone, int32 parentBone) {
  auto &aNode = eng.nodes.at(bone);
  auto &parentNode = eng.nodes.at(parentBone);

  aNode.globalPositions = parentNode.globalPositions;
  aNode.globalRotations.resize(eng.numSamples);

  if (aNode.positions.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) -= TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.refPosition);
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) -= TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.positions.at(s));
    }
  }

  if (aNode.rotations.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) = Pack(Multiply(
          aNode.refRotation, Unpack(parentNode.globalRotations.at(s))));
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) =
          Pack(Multiply(Unpack(aNode.rotations.at(s)),
                        Unpack(parentNode.globalRotations.at(s))));
    }
  }
}

Vector4A16 DeltaRotation(Vector4A16 u, Vector4A16 v) {
  u.w = 0;
  v.w = 0;
  const auto temp = _mm_sqrt_ss((u.DotV(u) * v.DotV(v))._data);
  const float normUV = _mm_cvtss_f32(temp);
  const float realPart = normUV + u.Dot(v);
  Vector4A16 w;

  if (realPart < 1.e-6f * normUV) {
    /* If u and v are exactly opposite, rotate 180 degrees
     * around an arbitrary orthogonal axis. Axis normalisation
     * can happen later, when we normalise the quaternion. */
    w = abs(u.x) > abs(u.z) ? Vector4A16(-u.y, u.x, 0, 0)
                            : Vector4A16(0, -u.z, u.y, 0);
  } else {
    /* Otherwise, build quaternion the standard way. */
    w = u.Cross(v);
    w.w = realPart;
  }

  return w.Normalized();
}

struct IkConstraint {
  Vector4A16 minThetas;
  Vector4A16 maxThetas;
};
/*
Vector4A16 ApplyConstraints(Vector4A16 q, const IkConstraint &constraint) {
  Vector4A16 retQ{0, 0, 0, 1};

  for (uint32 i = 0; i < 3; i++) {
    const float theta = std::atan2(q[i], q.w);
    const float gt =
        (theta > constraint.maxThetas[i]) * constraint.maxThetas[i];
    const float lt =
        (theta < constraint.minThetas[i]) * constraint.minThetas[i];
    const float aTheta = gt + lt;
    const float rTheta = (aTheta == 0) * theta + (aTheta != 0) * aTheta;
    Vector4A16 rQ;
    rQ[i] = std::sin(rTheta);
    rQ.w = std::cos(rTheta);

    retQ = Multiply(retQ, rQ);
  }

  return retQ.Normalized();
}*/

Vector4A16 ApplyConstraints(Vector4A16 q, const IkConstraint &constraint) {
  const glm::vec3 decomposed = glm::eulerAngles(glm::quat(q.w, q.x, q.y, q.z));
  glm::vec3 retEuler{};

  for (uint32 i = 0; i < 3; i++) {
    glm::vec3 component{};
    component[i] = decomposed[i];
    glm::quat qt(component);
    float qtc = qt[i + 1];

    // There is a big problem with -1 == 1 in euler space, fucking modulo again

    if (qt.w < 0) {
      qtc *= -1;
    }

    /*if (qtc > 0 && q[i] < 0) {
      qtc *= -1;
    }*/

    const float useClampGT = (qtc > constraint.maxThetas[i]);
    const float useClampLT = (qtc < constraint.minThetas[i]);
    const float useClamp = useClampGT + useClampLT;

    const float gt = useClampGT * constraint.maxThetas[i];
    const float lt = useClampLT * constraint.minThetas[i];
    const float rTheta = (useClamp == 0) * qtc + (useClamp != 0) * (gt + lt);

    Vector4A16 rqt;
    rqt[i] = rTheta;
    rqt.QComputeElement();

    const glm::vec3 decomposedC =
        glm::eulerAngles(glm::quat(rqt.w, rqt.x, rqt.y, rqt.z));
    retEuler[i] = decomposedC[i];
  }

  glm::quat retQuat(retEuler);

  return {retQuat.x, retQuat.y, retQuat.z, retQuat.w};
}

Vector4A16 ApplyConstraints2(Vector4A16 q, const IkConstraint &constraint) {
  glm::vec3 rete;

  for (uint32 i = 0; i < 3; i++) {
    Vector4A16 cqti;
    cqti[i] = constraint.minThetas[i];
    cqti.QComputeElement();

    Vector4A16 cqta;
    cqta[i] = constraint.maxThetas[i];
    cqta.QComputeElement();

    auto ia = glm::degrees(
        glm::eulerAngles(glm::quat(cqti.w, cqti.x, cqti.y, cqti.z)));
    auto aa = glm::degrees(
        glm::eulerAngles(glm::quat(cqta.w, cqta.x, cqta.y, cqta.z)));
    auto qa = glm::degrees(glm::eulerAngles(glm::quat(q.w, q.x, q.y, q.z)));

    if (qa[i] > aa[i]) {
      rete[i] = aa[i];
    } else if (qa[i] < ia[i]) {
      rete[i] = ia[i];
    } else {
      rete[i] = qa[i];
    }
  }

  glm::quat rqt(glm::radians(rete));

  return Vector4A16(rqt.x, rqt.y, rqt.z, rqt.w);
}

void Constraint(AnimEngine &eng, int32 bone, const IkConstraint &constraint) {
  auto &aNode = eng.nodes.at(bone);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);
  auto &parentNode2 = eng.nodes.at(parentNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 parentNodePos = parentNode.globalPositions.at(s);
    Vector4A16 &nodePos = aNode.globalPositions.at(s);
    Vector4A16 ogDir = aNode.refPosition;

    /*if (lookAtNode.positions.size() > 0) {
      ogDir = lookAtNode.positions.at(s);
    }*/

    // Parent joint global rotation
    Vector4A16 globalRotation = DeltaRotation(ogDir, nodePos);
    // Parent joint local rotation
    const Vector4A16 parentGlobalRotation =
        Unpack(parentNode2.globalRotations.at(s));
    // Vector4A16 localRotation =
    //     Multiply(globalRotation, parentGlobalRotation.QConjugate());
    //  Parent joint local rotation constraint
    Vector4A16 constrainedLocalRotation =
        ApplyConstraints(globalRotation, constraint);
    // Parent joint global rotation
    Vector4A16 constrainedGlobalRotation =
        Multiply(parentGlobalRotation, constrainedLocalRotation);

    // End joint global position
    nodePos = TransformPoint(constrainedGlobalRotation, ogDir);
    nodePos += parentNodePos;

    parentNode.globalRotations.at(s) = Pack(constrainedGlobalRotation);
  }
}

void FixupNodeMagnitudeForward(AnimEngine &eng, int32 bone,
                               const IkConstraint &constraint) {
  auto &aNode = eng.nodes.at(bone);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 &nodePos = aNode.globalPositions.at(s);
    Vector4A16 parentNodePos = parentNode.globalPositions.at(s);
    nodePos = (nodePos - parentNodePos).Normalized() * aNode.magnitude;
    nodePos += parentNodePos;
  }

  Constraint(eng, bone, constraint);
}

void FixupNodeMagnitudeBackward(AnimEngine &eng, int32 bone,
                                const IkConstraint &constraint) {
  auto &parentNode = eng.nodes.at(bone);
  auto &aNode = eng.nodes.at(parentNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 &nodePos = aNode.globalPositions.at(s);
    Vector4A16 parentNodePos = parentNode.globalPositions.at(s);
    nodePos = parentNodePos +
              (nodePos - parentNodePos).Normalized() * parentNode.magnitude;
  }

  Constraint(eng, aNode.parentAnimNode, constraint);
}

// unused
Vector4A16 BuildConstraintQuat(Vector4A16 value) {
  Vector4A16 thetas;

  for (uint32 i = 0; i < 3; i++) {
    Vector4A16 qt;
    qt[i] = value[i];
    qt.QComputeElement();
    thetas[i] = std::atan2(value[i], qt.w);
  }

  return thetas;
}

IkConstraint BuildConstraintQuat(Vector4A16 min, Vector4A16 max) {
  Vector4A16 minThetas(min);
  Vector4A16 maxThetas(max);

  return {minThetas, maxThetas};
}

using PoseRotation = Vector4A16 (*)(int32 boneId);
Vector4A16 DefaultRotation(int32) { return {0, 0, 0, 1}; }

struct IkChainDescript {
  int16 base = -1;
  int16 controlBase = -1;
  uint8 numLinks = 2;
  uint8 type = 0;
  Vector4A16 effectorDirection;
  PoseRotation chainPoseRotations[3]{DefaultRotation, DefaultRotation,
                                     DefaultRotation};
};

struct IkChainDescript2 {
  int16 base = -1;
  int16 controlBase = -1;
  uint8 numLinks = 2;
  uint8 type = 1;
  Vector4A16 effectorDirection;
  PoseRotation chainPoseRotations[3]{DefaultRotation, DefaultRotation,
                                     DefaultRotation};
  bool useConstraints[3] = {0, 0, 0};
  IkConstraint constraints[3]{};
};

template <int x, int y, int z> Vector4A16 R(int32) {
  static Vector4A16 retVal = (Vector4A16{x, y, z, 0} / 1000).QComputeElement();
  return retVal;
}

IkChainDescript2 IK_DR_RIGHT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-370, 99, -239>,
        R<500, 0, 0>,
    },
    .useConstraints{true, true, true},
    .constraints{
        BuildConstraintQuat({-0.7, -0.4, -0.45, 0}, {0.9, 0.09, 0.05, 0}),
        BuildConstraintQuat({0, -0.15, 0, 0}, {0.95, 0.15, 0, 0}),
        BuildConstraintQuat({-0.25, 0, 0, 0}, {0.35, 0, 0.2, 0}),
    },
};

IkChainDescript2 IK_DR_LEFT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-370, -99, 239>,
        R<500, 0, 0>,
    },
    .useConstraints{true, true, true},
    .constraints{
        BuildConstraintQuat({-0.7, -0.09, -0.05, 0}, {0.9, 0.4, 0.45, 0}),
        BuildConstraintQuat({0, -0.15, 0, 0}, {0.95, 0.15, 0, 0}),
        BuildConstraintQuat({-0.25, 0, -0.2, 0}, {0.35, 0, 0, 0}),
    },
};

IkChainDescript IK_EFFECTOR{};

IkChainDescript IK_DR_LEFT_ARM{
    .effectorDirection{1, 0, 0, 0},
    .chainPoseRotations{
        R<0, 0, -500>,
        R<0, -500, 0>,
    },
};

IkChainDescript IK_DR_RIGHT_ARM{
    .effectorDirection{-1, 0, 0, 0},
    .chainPoseRotations{
        R<0, 0, 500>,
        R<0, 500, 0>,
    },
};

IkChainDescript *IK_DR[]{
    /**/     //
    nullptr, // 0 none
    nullptr,
    &IK_EFFECTOR, // 2 effector
    nullptr,
    nullptr,
    &IK_DR_RIGHT_ARM, // 5 hand
    &IK_DR_LEFT_ARM,  // 6 hand
    reinterpret_cast<IkChainDescript *>(&IK_DR_RIGHT_LEG),
    reinterpret_cast<IkChainDescript *>(&IK_DR_LEFT_LEG),
};

IkChainDescript IK_LP_LEG{
    .effectorDirection{0, -1, 0, 0},
};

IkChainDescript IK_LP_VS04_LEG{
    .numLinks = 3,
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-383, 0, 0>,
        R<259, 0, 0>,
        R<259, 0, 0>,
    },
};

IkChainDescript IK_LP_VS00_LEFT_LEG{
    .numLinks = 3,
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-384, -65, -73>,
        R<383, 0, 0>,
        R<259, 0, 0>,
    },
};

IkChainDescript IK_LP_VS00_RIGHT_LEG{
    .numLinks = 3,
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-384, 65, 73>,
        R<383, 0, 0>,
        R<259, 0, 0>,
    },
};

IkChainDescript IK_LP_VS00_RIGHT_ARM{
    .effectorDirection{0, 0, 1, 0},
    .chainPoseRotations{
        R<0, 0, 0>,
        R<0, 707, 0>,
    },
};

IkChainDescript IK_LP_VS00_LEFT_ARM{
    .effectorDirection{0, 0, 1, 0},
    .chainPoseRotations{
        R<0, 0, 0>,
        R<0, -707, 0>,
    },
};

IkChainDescript IK_LP_RIGHT_LEG_AKC0{
    .effectorDirection{-1, 0, 0, 0},
    .chainPoseRotations{
        R<0, 0, 259>,
        R<0, 0, 574>,
    },
};

IkChainDescript IK_LP_LEFT_LEG_AKC0{
    .effectorDirection{1, 0, 0, 0},
    .chainPoseRotations{
        R<0, 0, -259>,
        R<0, 0, -574>,
    },
};

IkChainDescript IK_LP_AK0B_RIGHT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 12:
            // vs03 back leg
            return {0.370, 0.099, 0.233, 0.892};
          default:
            // ak0b back leg, case 4 collides
            // return {0.785, 0.211, -0.366, 0.453};
            return {0.5, 0, 0, 0.866};
          }
        },
        R<-707, 0, 0>,
    },
};

IkChainDescript IK_LP_AK0B_LEFT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 16:
            // vs03 back leg
            return {0.370, -0.099, -0.233, 0.892};
          case 7:
            // ak0b back leg
            return {0.785, -0.211, 0.366, 0.453};
          default:
            return {0.5, 0, 0, 0.866};
          }
        },
        R<-707, 0, 0>,
    },
};

IkChainDescript IK_LP_AK09_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 2:
          case 6:
          case 10:
          case 14:
            // ak13
            return {0.259, 0, 0, 0.966};
          default:
            return {-0.906, 0, 0, 0.423};
          }
        },
        R<819, 0, 0>,
    },
};

IkChainDescript IK_LP_AK00_LEFT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 5:
            // ak00 back leg
            return {0, 0, 0.259, 0.966};
          case 7:
            // vs03 front leg
            return {-0.383, 0, 0, 0.924};
          default:
            return {-0.933, 0.250, 0.250, 0.067};
          }
        },
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 6:
            // ak00 back leg
            return {-0.924, 0, 0, -0.383};
          default:
            return {0.574, 0, 0, 0.819};
          }
        },
    },
};

IkChainDescript IK_LP_AK00_RIGHT_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 1:
            // ak00 back leg
            return {0, 0, -0.259, 0.966};
          case 2:
            // vs03 front leg
            return {-0.383, 0, 0, 0.924};
          default:
            return {-0.933, -0.250, -0.250, 0.067};
          }
        },
        [](int32 bone) -> Vector4A16 {
          switch (bone) {
          case 2:
            // ak00 back leg
            return {-0.924, 0, 0, -0.383};
          default:
            return {0.574, 0, 0, 0.819};
          }
        },
    },
};

IkChainDescript IK_LP2_HM_LEG{
    .effectorDirection{0, -1, 0, 0},
    .chainPoseRotations{
        R<-259, 0, 0>,
        R<500, 0, 0>,
    },
};

// Ak09, Ak15(Dongo) and Most Vs doesn't have marked LegIKTarget
enum LPTypes {
  None,
  Unk1,
  LegIKTarget,
  Ak00RightLegChain, // Vs03, Vs33, Ak16, Ak0A
  Ak00LeftLegChain,  // Vs03, Vs33, Ak16, Ak0A
  Unk5,
  Unk6,
  Ak09RightLegChain, // Ak15, Ak13, Ak6C, Ak70, Ak11, Vs06, Hm (lp1)
  Ak09LeftLegChain,  // Ak15, Ak13, Ak6C, Ak70, Ak11, Vs06, Ak0b front, Hm (lp1)
  Ak0bRightBackLegChain, // Vs01, Vs03, Vs33, Vs34
  Ak0bLeftBackLegChain,  // Vs01, Vs03, Vs33, Vs34
  Vs04RightLegChain,     // Vs05, Vs31, Vs32, Vs40
  Vs04LeftLegChain,      // Vs05, Vs31, Vs32, Vs40
  Vs00RightLegChain,     // Vs41
  Vs00LeftLegChain,      // Vs41
  Unk15,
  Unk16,
  Unk17,
  Unk18,
  Unk19,
  Unk20,
  Vs00RightArmChain, // Vs07, Vs41, Hm target parent ani_bone 9 neck
  Vs00LeftArmChain,  // Vs07, Vs41, Hm
  Unk23,
  Ak64Leg, // Ak67
  Unk25,
  Unk26,
  Unk27,
  Unk28,
  Unk29,
  Unk30,
  Unk31,
  Unk32,
  Unk33,
  Unk34,
  Unk35,
  Unk36,
  Unk37,
  Unk38,
  Unk39,
  Unk40,
  LP2HmLeg,
  AkC0LeftLeg,
  AkC0RightLeg,
  Unk44,
  Unk45,
  LP2HmRightArm,
  LP2HmLeftArm,
};

IkChainDescript *IK_LP[]{
    /**/     //
    nullptr, // 0 none
    nullptr,
    &IK_EFFECTOR, // 2 effector
    &IK_LP_AK00_RIGHT_LEG,
    &IK_LP_AK00_LEFT_LEG,
    nullptr,
    nullptr,
    &IK_LP_AK09_LEG,
    &IK_LP_AK09_LEG,
    &IK_LP_AK0B_RIGHT_LEG,
    &IK_LP_AK0B_LEFT_LEG,
    &IK_LP_VS04_LEG,
    &IK_LP_VS04_LEG,
    &IK_LP_VS00_LEFT_LEG,
    &IK_LP_VS00_RIGHT_LEG,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &IK_LP_VS00_RIGHT_ARM,
    &IK_LP_VS00_LEFT_ARM,
    nullptr,
    &IK_LP_LEG,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &IK_LP2_HM_LEG,
    &IK_LP_LEFT_LEG_AKC0,
    &IK_LP_RIGHT_LEG_AKC0,
    nullptr,
    nullptr,
    &IK_DR_RIGHT_ARM,
    &IK_DR_LEFT_ARM,
};

using LV = revil::LMTVersion;
std::map<uint16, IkChainDescripts> IK_VERSION{
    {uint16(LV::V_22), IK_DR},
    {uint16(LV::V_40), IK_LP},
    {uint16(LV::V_50), IK_LP},
    {uint16(LV::V_56), IK_LP},
};
/*
constraints:
  X  Y  Z
bone 1:
  0.9 0.09 0.05
  -0.7 -0.4 -0.45
bone 2:
  0, -0.15, 0
  0.95, 0.15, 0
bone 3:
  0.35, 0, 0
  -0.25, 0, 0.2

bone 4:
  0.9 0.4 0.45
  -0.7 -0.09 -0.05
bone 5 == bone 2

bone 6:
  0.35, 0, 0
  -0.25, 0, -0.2
*/

void RebakeNode(AnimEngine &eng, size_t nodeId) {
  auto &eNode = eng.nodes.at(nodeId);
  auto &parentNode = eng.nodes.at(eNode.parentAnimNode);
  eNode.positions = std::move(eNode.globalPositions);

  if (eNode.globalRotations.empty()) {
    eNode.rotations.insert(eNode.rotations.begin(), eng.numSamples,
                           SVector4(0, 0, 0, 0x7fff));
  } else {
    eNode.rotations = std::move(eNode.globalRotations);
  }

  for (size_t s = 0; s < eng.numSamples; s++) {
    eNode.positions.at(s) -= parentNode.globalPositions.at(s);
    eNode.positions.at(s) =
        TransformPoint(Unpack(parentNode.globalRotations.at(s)).QConjugate(),
                       eNode.positions.at(s));

    eNode.rotations.at(s) =
        Pack(Multiply(Unpack(eNode.rotations.at(s)),
                      Unpack(parentNode.globalRotations.at(s)).QConjugate()));
  }
}
/*
void findNewAngles(AnimEngine &eng, int node) {
  auto &target = eng.nodes.at(node);
  auto &aNode = eng.nodes.at(target.parentAnimNode);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 endPosition = aNode.globalPositions.at(s);
    Vector4A16 startPosition = parentNode.globalPositions.at(s);
    Vector4A16 targetPosition = target.globalPositions.at(s);
    Vector4A16 toTarget =
        (targetPosition - startPosition) * Vector4A16(1, 1, 1, 0);
    Vector4A16 toEnd = (endPosition - startPosition) * Vector4A16(1, 1, 1, 0);
    toEnd.Normalize();
    toTarget.Normalize();

    float cosine = toEnd.Dot(toTarget);

    if (cosine < 0.99) {
      Vector4A16 crossResult = toEnd.Cross(toTarget);
      float angle = glm::angle(toTarget, toEnd);
      quat rotation = angleAxis(angle, crossResult);
      rotation = normalize(rotation);
      glm::vec3 euler = glm::eulerAngles(rotation);
      currentBone->rotateMax(euler.x, euler.y, euler.z);
    }
  }
}*/

void FabrikForward(AnimEngine &eng, int32 bone) {
  auto &aNode = eng.nodes.at(bone);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 &nodePos = aNode.globalPositions.at(s);
    Vector4A16 parentNodePos = parentNode.globalPositions.at(s);
    nodePos = parentNodePos +
              (nodePos - parentNodePos).Normalized() * aNode.magnitude;
  }
}

void FabrikBackward(AnimEngine &eng, int32 bone) {
  auto &parentNode = eng.nodes.at(bone);
  auto &aNode = eng.nodes.at(parentNode.parentAnimNode);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 &nodePos = aNode.globalPositions.at(s);
    Vector4A16 parentNodePos = parentNode.globalPositions.at(s);
    nodePos = parentNodePos +
              (nodePos - parentNodePos).Normalized() * parentNode.magnitude;
  }
}

void LookatRotation(AnimEngine &eng, int32 bone, int32 lookAtBone) {
  auto &aNode = eng.nodes.at(bone);
  auto &lookAtNode = eng.nodes.at(lookAtBone);

  for (size_t s = 0; s < eng.numSamples; s++) {
    Vector4A16 nodePos = aNode.globalPositions.at(s);
    Vector4A16 lookAtNodePos = lookAtNode.globalPositions.at(s);
    Vector4A16 solvedDir = lookAtNodePos - nodePos;

    Vector4A16 ogDir = lookAtNode.refPosition;

    /*if (lookAtNode.positions.size() > 0) {
      ogDir = lookAtNode.positions.at(s);
    }*/

    aNode.globalRotations.at(s) = Pack(DeltaRotation(ogDir, solvedDir));
  }
}

void Fabrik(AnimEngine &eng, IkChainDescript &chain) {
  RelativeResample(eng, chain.base + 2, chain.controlBase);
  InverseRelativeResample(eng, chain.base + 1, chain.base + 2);

  for (size_t i = 0; i < 32; i++) {
    FabrikBackward(eng, chain.base + 2);
    FabrikBackward(eng, chain.base + 1);
    RelativeResample(eng, chain.base, eng.nodes.at(chain.base).parentAnimNode);
    FabrikForward(eng, chain.base + 1);
    FabrikForward(eng, chain.base + 2);
    RelativeResample(eng, chain.base + 2, chain.controlBase);
  }

  LookatRotation(eng, chain.base, chain.base + 1);
  LookatRotation(eng, chain.base + 1, chain.base + 2);
}

void Ccdik(AnimEngine &eng, IkChainDescript &chain) {
  for (size_t i = 0; i < 32; i++) {
    // FixupNodeMagnitudeBackward(eng, chain.base + 2);
    // FixupNodeMagnitudeBackward(eng, chain.base + 1);

    // RelativeResample(eng, chain.base,
    // eng.nodes.at(chain.base).parentAnimNode);

    /*if (chain.useConstraints[0]) {
      FixupNodeMagnitudeForward(eng, chain.base + 1, chain.constraints[0]);
    } else {*/
    // FixupNodeMagnitudeForward(eng, chain.base + 1);
    // }

    /*if (chain.useConstraints[1]) {
      FixupNodeMagnitudeForward(eng, chain.base + 2, chain.constraints[1]);
    } else {
      FixupNodeMagnitudeForward(eng, chain.base + 2);
    }*/

    // if (i < 31) {
    // RelativeResample(eng, chain.base + 2, chain.controlBase);
    // FixupNodeMagnitudeForward(eng, chain.base + 2);
    //}
  }

  /*for (size_t i = 0; i < 32; i++) {
    FixupNodeMagnitudeBackward(eng, chain.base + 1, chain.base + 2);
    FixupNodeMagnitudeBackward(eng, chain.base, chain.base + 1);

    RelativeResample(eng, chain.base,
  eng.nodes.at(chain.base).parentAnimNode); FixupNodeMagnitudeForward(eng,
  chain.base + 1); FixupNodeMagnitudeForward(eng, chain.base + 2);
    RelativeResample(eng, chain.base + 2, chain.controlBase);
  }*/
}

void RebakeChain(AnimEngine &eng, size_t nodeId) {
  RebakeNode(eng, nodeId + 2);
  RebakeNode(eng, nodeId + 1);
  RebakeNode(eng, nodeId);
}

void MakeEffector(AnimEngine &eng, IkChainDescript &chain) {
  AnimNode &aNode = eng.nodes.at(chain.base + chain.numLinks);
  const size_t nodeId = -(chain.base + chain.numLinks);
  AnimNode &newEffector = eng.nodes[nodeId];
  newEffector.positions = std::move(aNode.positions);
  newEffector.rotations = std::move(aNode.rotations);
  newEffector.parentAnimNode = chain.controlBase;

  AnimNode &newEffectorDir = eng.nodes[nodeId - 1000];
  newEffectorDir.positions = std::move(eng.nodes.at(chain.base).positions);
  newEffectorDir.refPosition = chain.effectorDirection;
  RelativeResample(eng, nodeId, chain.controlBase);
  RebakeNode(eng, nodeId);
}

void MakeDefaultPose(AnimEngine &eng, IkChainDescript &chain) {
  for (uint8 i = 0; i < chain.numLinks; i++) {
    AnimNode &node = eng.nodes.at(chain.base + i);
    PoseRotation rotFc = chain.chainPoseRotations[i];
    if (rotFc == DefaultRotation) {
      continue;
    }
    node.rotations.clear();
    node.rotations.insert(node.rotations.end(), eng.numSamples,
                          Pack(rotFc(chain.base + i)));
  }
}

void SetupChains(AnimEngine &eng, IkChainDescripts iks) {
  std::vector<IkChainDescript> chains;
  Hierarchy marks;

  for (auto &[nodeIndex, node] : eng.nodes) {
    assert(node.boneType < iks.size());
    IkChainDescript *tChain = iks[node.boneType];

    if (tChain && tChain != &IK_EFFECTOR) {
      IkChainDescript nChain{*tChain};
      nChain.base = nodeIndex;
      chains.emplace_back(nChain);
      eng.usedIkNodes.emplace(
          eng.nodes.at(nodeIndex + nChain.numLinks).glNodeIndex);

      MarkHierarchy(eng, marks, nodeIndex + nChain.numLinks);

      if (nChain.controlBase > -1) {
        MarkHierarchy(eng, marks, nChain.controlBase);
      }
    }
  }

  if (chains.empty()) {
    return;
  }

  MakeGlobalFrames(eng, marks);

  for (auto &chain : chains) {
    MakeEffector(eng, chain);
    MakeDefaultPose(eng, chain);
    // Fabrik(eng, chain);
    // RebakeChain(eng, chain.base);
  }
}
