#include "../src/LMTInternal.h"
#include "LMTDecode.h"
#include "datas/masterprinter.hpp"
#include <iomanip>

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
    LMTRawTrack::InputRaw rData {
        /*esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {100,105,301,1}),
        esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {2.0005,3.00078,15.000874,1}),
        esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1.0005,1.00078,3.000874,1})*/
        /*esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3e-06, 18.2245, 77.5075, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.471, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.80552, 18.2245, 83.2695, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-7.62831, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-5.8223, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3.66865, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {3.66864, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {5.82229, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {7.6283, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.80485, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.47099, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3e-06, 18.2245, 77.5075, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.471, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.80486, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-7.62831, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-5.8223, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3.66865, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {3.66864, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {5.82229, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {7.6283, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.80485, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.47099, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3e-06, 18.2245, 77.5075, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.471, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.80486, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-7.62831, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-5.8223, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3.66865, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {3.66864, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {5.82229, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {7.6283, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.80485, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.47099, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3e-06, 18.2245, 77.5075, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.471, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-9.80486, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-7.62831, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-5.8223, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3.66865, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1.26503, 18.2245, 77.6167, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {3.66864, 18.2245, 78.379, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {5.82229, 18.2245, 79.6199, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {7.6283, 18.2245, 81.0446, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.98869, 18.2245, 82.3591, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.80485, 18.2245, 83.2687, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.97779, 18.2245, 83.4794, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {9.47099, 18.2245, 82.8829, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {8.37035, 18.2245, 81.734, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {6.77487, 18.2245, 80.3277, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {4.78282, 18.2245, 78.9581, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {2.49199, 18.2245, 77.9196, 1}),
esMatrix44({1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {-3e-06, 18.2245, 77.5075, 1}),*/
    };

    //const float sampleFrac = 1.0f / 200.0f;
    //printer.AddPrinterFunction(reinterpret_cast<void*>(printf));

    /*for (int t = 0; t < 200; t++)
    {
        for (int e = 0; e < 3; e++)
            rData[1].r4[e] = rData[0].r4[e] + (rData[2].r4[e] - rData[0].r4[e]) * sampleFrac * t;
      */      
        //CreateTrackFromRaw(LMTRawTrack(rData, 0));
    //}
    
    
    

    return 0;
}