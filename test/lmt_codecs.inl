#pragma once
#include "datas/unit_testing.hpp"
#include "mtf_lmt/codecs.hpp"

// 90 -180 45
static const Vector4A16 testingQuat(0.2706f, -0.2706f, -0.65328f, 0.65328f);
static const Vector4A16 testingQuatBi(0.2706f, 0.2706f, 0.65328f, 0.65328f);
static const Vector4A16 testingVector(5.5632f, 12.846931f, 368.2519f, 1.0f);
static const Vector4A16 testingVectorBi(0.5632f, 0.846931f, 0.2519f, 1.0f);

typedef std::unique_ptr<LMTTrackController> CTR;

int test_lmt_codec00() {
  Vector4A16 resultVector;
  CTR control(
      LMTTrackController::CreateCodec(TrackTypesShared::StepRotationQuat3));
  control->NumFrames(1);
  control->Devaluate(testingQuat, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00001f);
  TEST_EQUAL(testingQuat, resultVector);

  return 0;
}

int test_lmt_codec01() {
  Vector4A16 resultVector;
  CTR control =
      CTR(LMTTrackController::CreateCodec(TrackTypesShared::SingleVector3));
  control->NumFrames(1);
  control->Devaluate(testingVector, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00001f);
  TEST_EQUAL(testingVector, resultVector);

  return 0;
}

int test_lmt_codec02() {
  Vector4A16 resultVector;
  CTR control =
      CTR(LMTTrackController::CreateCodec(TrackTypesShared::LinearVector3));
  control->NumFrames(1);
  control->Devaluate(testingVector, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00001f);
  TEST_EQUAL(testingVector, resultVector);

  return 0;
}

int test_lmt_codec03() {
  Vector4A16 resultVector;
  CTR control =
      CTR(LMTTrackController::CreateCodec(TrackTypesShared::SphericalRotation));
  control->NumFrames(1);
  control->Devaluate(testingQuat, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00001f);
  TEST_EQUAL(testingQuat, resultVector);

  return 0;
}

int test_lmt_codec04() {
  Vector4A16 resultVector;
  CTR control = CTR(
      LMTTrackController::CreateCodec(TrackTypesShared::BiLinearVector3_16bit));
  control->NumFrames(1);
  control->Devaluate(testingVectorBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00001f);
  TEST_EQUAL(testingVectorBi, resultVector);

  return 0;
}

int test_lmt_codec05() {
  Vector4A16 resultVector;
  CTR control = CTR(
      LMTTrackController::CreateCodec(TrackTypesShared::BiLinearVector3_8bit));
  control->NumFrames(1);
  control->Devaluate(testingVectorBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.005f);
  TEST_EQUAL(testingVectorBi, resultVector);

  return 0;
}

int test_lmt_codec06() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::LinearRotationQuat4_14bit));
  control->NumFrames(1);
  control->Devaluate(testingQuat, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00018f);
  TEST_EQUAL(testingQuat, resultVector);

  return 0;
}

int test_lmt_codec07() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuat4_7bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.005f);
  TEST_EQUAL(testingQuatBi, resultVector);

  return 0;
}

int test_lmt_codec08() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuatXW_14bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00002f);
  TEST_EQUAL(testingQuatBi * Vector4A16(1.0f, 0.0f, 0.0f, 1.0f), resultVector);

  return 0;
}

int test_lmt_codec09() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuatYW_14bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00002f);
  TEST_EQUAL(testingQuatBi * Vector4A16(0.0f, 1.0f, 0.0f, 1.0f), resultVector);

  return 0;
}

int test_lmt_codec10() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuatZW_14bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.00002f);
  TEST_EQUAL(testingQuatBi * Vector4A16(0.0f, 0.0f, 1.0f, 1.0f), resultVector);

  return 0;
}

int test_lmt_codec11() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuat4_11bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.0005f);
  TEST_EQUAL(testingQuatBi, resultVector);

  return 0;
}

int test_lmt_codec12() {
  Vector4A16 resultVector;
  CTR control = CTR(LMTTrackController::CreateCodec(
      TrackTypesShared::BiLinearRotationQuat4_9bit));
  control->NumFrames(1);
  control->Devaluate(testingQuatBi, 0);
  control->Evaluate(resultVector, 0);

  Vector4A16::SetEpsilon(0.002f);
  TEST_EQUAL(testingQuatBi, resultVector);

  return 0;
}
