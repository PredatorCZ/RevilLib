#include "../src/LMTInternal.h"


int main()
{
    Vector4A16::SetEpsilon(0.00001);
    const Vector4A16 testingQuat(0.2706, -0.2706, -0.65328, 0.65328); // 90 -180 45
    const Vector4A16 testingQuatBi(0.2706, 0.2706, 0.65328, 0.65328);
    const Vector4A16 testingVector(5.5632, 12.846931, 368.2519, 1.0f);
    const Vector4A16 testingVectorBi(0.5632, 0.846931, 0.2519, 1.0f);
    Vector4A16 resultVector;
    
    typedef std::unique_ptr<LMTTrackController> CTR;

    CTR control(LMTTrackController::CreateCodec(TrackTypesShared::StepRotationQuat3));
    control->NumFrames(1);
    control->Devaluate(testingQuat, 0);
    control->Evaluate(resultVector, 0);

    bool pass = testingQuat == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::SingleVector3));
    control->NumFrames(1);
    control->Devaluate(testingVector, 0);
    control->Evaluate(resultVector, 0);

    pass = testingVector == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::LinearVector3));
    control->NumFrames(1);
    control->Devaluate(testingVector, 0);
    control->Evaluate(resultVector, 0);

    pass = testingVector == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::SphericalRotation));
    control->NumFrames(1);
    control->Devaluate(testingQuat, 0);
    control->Evaluate(resultVector, 0);

    pass = testingQuat == resultVector;

    if (!pass) throw; 

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearVector3_16bit));
    control->NumFrames(1);
    control->Devaluate(testingVectorBi, 0);
    control->Evaluate(resultVector, 0);

    pass = testingVectorBi == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearVector3_8bit));
    control->NumFrames(1);
    control->Devaluate(testingVectorBi, 0);
    control->Evaluate(resultVector, 0);

    Vector4A16::SetEpsilon(0.005);
    pass = testingVectorBi == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::LinearRotationQuat4_14bit));
    control->NumFrames(1);
    control->Devaluate(testingQuat, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.00018);
    pass = testingQuat == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuat4_7bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.005);
    pass = testingQuatBi == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuatXW_14bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.00002);
    pass = testingQuatBi * Vector4A16(1.0f, 0.0f, 0.0f, 1.0f) == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuatYW_14bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.00002);
    pass = testingQuatBi * Vector4A16(0.0f, 1.0f, 0.0f, 1.0f) == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuatZW_14bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.00002);
    pass = testingQuatBi * Vector4A16(0.0f, 0.0f, 1.0f, 1.0f) == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuat4_11bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.0005);
    pass = testingQuatBi == resultVector;

    if (!pass) throw;

    control = CTR(LMTTrackController::CreateCodec(TrackTypesShared::BiLinearRotationQuat4_9bit));
    control->NumFrames(1);
    control->Devaluate(testingQuatBi, 0);
    control->Evaluate(resultVector, 0);
    
    Vector4A16::SetEpsilon(0.002);
    pass = testingQuatBi == resultVector;

    if (!pass) throw;

    //HermiteVector3

    return 0;
}