/*  Revil Format Library
    Copyright(C) 2020-2021 Lukas Cone

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

#include "revil/hashreg.hpp"
#include <map>

using revil::Platform;

static const std::map<uint32, es::string_view> classNames{
    {0x04657ED7, "sprMapHead"},
    {0x0474F424, "rAreaSetLayout::LayoutInfo"},
    {0x26C9176E, "rAreaSetLayout::LayoutInfo"},
    {0x0B3A7230, "sprMap"},
    {0x1270F9FE, "nodeData"},
    {0x14E137DD, "rScrList::ScrListTable"},
    {0x1838FCE7, "rReverb::cReverbData"},
    {0x1E7A21E4, "rSoundSeg::LayoutInfo"},
    {0x354344D5, "nodeHead"},
    {0x35DDDCA1, "nodeLink"},
    {0x3AC30C7D, "rSceneBoxLayout::LayoutInfo"},
    {0x5BC3A2CC, "rHavokLinkCollisionLayout::LayoutInfo"},
    {0x4CABC92A, "rHavokLinkCollisionLayout::LayoutInfo"},
    {0x60420840, "rHavokConstraintLayout::LayoutInfo"},
    {0x4194CF9B, "rHavokConstraintLayout::LayoutInfo"},
    {0x63A8DDBA, "rAreaHitLayout::LayoutInfo"},
    {0x60F7B3FC, "rAreaHitLayout::LayoutInfo"},
    {0x6EBD2A98, "rScrList"},
    {0x4CFD5420, "rTexDetailEdit::DetailParam"},
    {0x71134D41, "MtHeapArray"},
    {0x25B52337, "rEffectProvider::EffectParam"},
    {0x1E66D162, "rEffectProvider::EffectSet"},
    {0x3AEBFBEA, "rEffectProvider::EffectList"},
    {0x3251DD5F, "rEffectProvider::INFO_EFFECT"},
    {0x6FD8D8C7, "rEffectProvider::INFO_E2D"},
    {0x602EBA26, "rEffectProvider::EffectMotSyncParam"},
    {0x0C3494F4, "rEffectProvider::EffectLODParam"},
    {0x4B03CE4C, "rEffectProvider::INFO_BASE"}, // ::INFO_PROV ??
    {0x0BC4A3C3, "rHmGenerator::HmGeneratorInfo"},
    {0x50DAA58B, "rHmGenerator::EquipWeaponInfo"},
    {0x5E9A0BCA, "rCnsIK::JointInfo"},
    {0x1AD90F64, "rPlanetEQ::cEQData"},
    {0x11A59288, "rReverb::cSoundReverbData"},
    {0x7821C482, "rReverb::cReverbParam"},
    {0x5CECEBB6, "rSoundMotionSe::cSoundMotionSeData"},
    {0x5EB6FE4E, "rSoundSequenceSe::SequenceSe"},
    {0x7A99C43F, "rSoundSequenceSe::SequenceSe::Command"},
    {0x5F735F08, "rSoundSubMixer::Param"},
    {0x45674FD1, "cAIPositionEvaluationFuncList"},
    {0x072DF97A, "cAIPositionEvaluationFunc"},
    {0x24C99855, "cPositionFilterDoughnutAroundPosTerritory"},
    {0x3EBF164E, "rThinkPlanAI::cItemPlanInfo"},
    {0x74987697, "rThinkPlanAI::cU32"},
    {0x378EECF0, "rAreaBarrierEdit::Work"},
    {0x50835382, "rInitPosSet::InitPosSetInfo"},
    {0x6B967852, "rScrList::ScrListTable"},
    {0x18868106, "cAIFSMCluster"},
    {0x5035D8FD, "cAIFSMNode"},
    {0x63E6A949, "cAIFSMLink"},
    {0x609DB540, "cAIFSMProcessContainer"},
    {0x1836BA1E, "cPositionEvaluationDangerRange"},
    {0x5C539DD4, "cPositionEvaluationDangerMessageRange"},
    {0x53E26270, "cPositionEvaluationDistToPos"},
    {0x57DABC2A, "cTargetVisibilityHigh"},
    {0x7EED1022, "cTargetVisibilityLow"},
    {0x2A6D2CAD, "cPositionEvaluationTargetRange"},
    {0x62C06191, "cPositionEvaluationTargetGoodDistans"},
    {0x4F61724C, "rFieldSet::FieldSetWork"},
    {0x15DE6AC5, "cPositionEvaluationTargetVisibility"},
    {0x16021C59, "cPositionFilterDoughnutAroundPos"},
    {0x01662DD0, "cPositionEvaluationPlayerVisibility"},
    {0x66BB432F, "rFieldSet::E2DSetWork"},
    {0x0704302E, "cAIPlanFilterTargetExist"},
    {0x026A2DE7, "cAIPlanMoveAction"},
    {0x29B16567, "cAIPlanFilterMessagePickUp"},
    {0x3D1A2BA6, "cAIPlanFilterPartyLevel"},
    {0x0B9D355C, "cAIPlanFilterDistans"},
    {0x6DF787F5, "cAIPlanEvaluateDistans"},
    {0x765A8CDB, "cAIPlanMoveSendMessage"},
    {0x5DEBFFAF, "cAIPlanMovePickMessage"},
    {0x546EF519, "cAIPlanMoveDamage"},
    {0x22AB6E3F, "cAIPlanMoveTrace"}, // cAIPlanMoveAvoidLOF ??
    {0x7CDD207B, "cAIPlanMoveTime"},
    {0x2ED25315, "cAIPlanMoveDistance"},
    {0x5BF97E48, "cAIPlanMoveAk64Action"},
    {0x15444559, "cAIPlanMoveTargetExist"},
    {0x09D48FD0, "cAIPlanEvaluateAttackRemain"}, //...DamageRemain ??
    {0x4E2D5670, "cAIPlanMoveSendCommand"},
    {0x41184FCC, "cAIPlanEvaluateDamageRemain"}, //...AttackRemain ??
    {0x2605B481, "cAIPlanFilterFSM"},
    {0x3E661257, "cAIPlanFilterCharIsXXX"},
    {0x384E3085, "cAIPlanMoveShot"},
    {0x32097A12, "cAIPlanMoveWaitPos"},
    {0x6564DEC0, "cAIPlanEvaluateMessagePickUp"},
    {0x4FAFA8D1, "cAIPlanMoveHmAction"},
    {0x66D9AF9F, "cAIPlanFilterTargetLifeMaxOverCk"},
    {0x3A326398, "cAIPlanEvaluateAIParam"},
    {0x2E71A0D2, "cAIPlanReturnX"},
    {0x617C113F, "cAIPlanFilterPowerGroup"},
    {0x1A7ACA56, "cAIPlanMoveHm40VoiceReq"},
    {0x56F3846A, "cAIPlanFilterGK"},
    {0x00A5315B, "cAIPlanFilterWeapon"},
    {0x35A0E27E, "cAIPlanFilterTeamCommand"},
    {0x2988A5D1, "cAIPlanMoveSetCursorMode"},
    {0x7922BB3F, "cAIPlanFilterUniqueIdRand"},
    {0x364E93AB, "cAIPlanFilterDir"},
    {0x5C58D131, "cAIPlanFilterAIParam"},
    {0x59295D08, "cAIPlanFilterArea"},
    {0x7E1DA162, "cAIPlanMoveActionFreeJump"},
    {0x36F10E99, "cAIPlanFilterHm40Type"},
    {0x308E2D6E, "cAIPlanFilterCharStatus"},
    {0x6906B3E1, "cAIPlanFilterTracePos"},
    {0x7DF274C1, "cAIPlanEvaluateCursorThrough"},
    {0x54D0B905, "cAIPlanFilterDevelop"},
    {0x4A5FCDB3, "cFSMOrder::FlagParameter"},
    {0x5AAE7F38, "cFSMOrder::PositionParameter"},
    {0x785E6622, "rAIConditionTree"},
    {0x1C328079, "rAIConditionTreeNode"},
    {0x56BB1759, "rAIConditionTree::TreeInfo"},
    {0x3D9510A8, "rAIConditionTree::VariableNode"},
    {0x3E1F9629, "rAIConditionTree::VariableNode::VariableInfo"},
    {0x2C29825D, "cAIDEnum"},
    {0x3280309B, "rAIConditionTree::ConstS32Node"},
    {0x788FCAED, "cFSMCharacterBase::MotionParameter"},
    {0x57EEA524, "cFSMOrder::BoolParameter"},
    {0x5907152A, "cFSMFighter::AIMovePositionParameter"},
    {0x4A2C530F, "cFSMOrder::TimerParameter"},
    {0x01464B23, "cFSMOrder::AngleParameter"},
    {0x79F101EC, "cFSMCharacterBase::PosAngleFlagParameter"},
    {0x6D64388A, "cFSMOrder::SchedulerParameter"},
    {0x47EF9888, "cFSMOrder::ReqEventExecParameter"},
    {0x53EF5D93, "cFSMCharacterBase::ScaleSetParameter"},
    {0x305FDEB3, "cFSMOrder::S32Parameter"},
    {0x4609617D, "cFSMCharacterBase::EffectSetParameter"},
    {0x4433B9D5, "cFSMCharacterBase::AttackPlayerKeepParameter"},
    {0x24E4445F, "cFSMOrder::F32Parameter"},
    {0x53C56047, "cFSMHM::ActionSetParameter"},
    {0x45387FF1, "cFSMCharacterBase::TargetSetParameter"},
    {0x39C1CAE5, "cFSMCharacterBase::ShotSetParameter"},
    {0x076683F2, "rAIConditionTree::ConstF32Node"},
    {0x14A4355A, "cFSMOrder::SchedulerCloseParameter"},
    {0x095A4DBF, "cFSMOrder::SubMixerParameter"},
    {0x4F530D8B, "cFSMFighter::AIPlayerPursuitParameter"},
    {0x3651D96A, "cFSMOrder::ShellParameter"},
    {0x0ADFDD5A, "cFSMCharacterBase::MoveScrollActiveParameter"},
    {0x29AB48DA, "cFSMVS::ActionSetParameter"},
    {0x0B7AA143, "cFSMCharacterBase::PartsDispSetParameter"},
    {0x0B92FDF1, "cFSMCharacterBase::ShotReqSetParameter"},
    {0x0D32E37E, "cFSMFighter::AIDataPostUpParameter"},
    {0x0F389C6D, "cFSMOM::ScrCollisionSetParameter"},
    {0x0FEFA610, "cFSMOM::TrainHitSetParameter"},
    {0x118B4EBB, "cFSMFighter::AITraceMoveParameter"},
    {0x146FBE15, "cFSMOrder::BaseRespawnPointSetParameter"},
    {0x1BB1BAB0, "cFSMOrder::CameraMotionSetParameter"},
    {0x1EBF971C, "cFSMCharacterBase::CharListParentOfsParameter"},
    {0x24325FF6, "cFSMCharacterBase::ShotTargetSetZParameter"},
    {0x2CA71A90, "cFSMOrder::AreaHitSetParameter"},
    {0x2D6D5145, "cFSMArea::UvScrollParameter"},
    {0x2F0741B8, "cFSMVS::CannonShotSetParameter"},
    {0x30471086, "cFSMVS::CannonTargetSetParameter"},
    {0x310FB3B0, "cFSMOrder::CallRequestSetParameter"},
    {0x32D8555C, "cFSMArea::WarPointSetParameter"},
    {0x38B20D54, "cFSMCharacterBase::UserWorkSetParameter"},
    {0x3BB265E4, "cFSMCharacterBase::ActSetParameter"},
    {0x446BD5E4, "cFSMCharacterBase::ShotTargetSetParameter"},
    {0x4AD6AEEC, "cFSMCharacterBase::WepMotionParameter"},
    {0x51C75708, "cFSMFighter::AIPlayerSupportParameter"},
    {0x5302004F, "cFSMOM::BoatHitSetParameter"},
    {0x53AFD2FB, "cFSMOrder::StatusFlagParameter"},
    {0x56600843, "cFSMCharacterBase::InitPosSetParameter"},
    {0x5943F174, "cFSMCharacterBase::CharListParentParameter"},
    {0x59561754, "cFSMCharacterBase::CalcPartsDamageSetParameter"},
    {0x5F0AD3BD, "cFSMVS::AreaMoveAttackParameter"},
    {0x64092131, "cFSMCharacterBase::RequestSeParameter"},
    {0x6472931D, "cFSMOrder::CameraVibSetParameter"},
    {0x64C7F78E, "cFSMCharacterBase::ObjAdjustEntryParameter"},
    {0x6D99A3BF, "cFSMOrder::CounterParameter"},
    {0x6DC84078, "cFSMArea::MissionCycleSetParameter"},
    {0x755AF279, "cFSMHM::MovePositionParameter"},
    {0x75ED317D, "cFSMCharacterBase::EffectSynchroSetParameter"},
    {0x77C63BD5, "cFSMVS::MovePositionParameter"},
    {0x7991B00F, "cFSMCharacterBase::CompulsionMoveParameter"},
    {0x79E1ED4B, "cFSMOrder::RandomGetParameter"},
    {0x7D358A0C, "cFSMCharacterBase::BlendMotionParameter"},
    {0x7D73040B, "cFSMCharacterBase::EffectSetResParameter"},
    {0x7FEE673F, "cFSMOM::BoatRideModeSetParameter"},
    {0x7FFC4F5B, "cFSMFighter::CharParamSetParameter"},
    {0x4D9FF01D, "cFSMCharacterBase::TransLucenceParameter"},
    {0x0C1599A1, "cFSMHM::DamageActionSetParameter"},
    {0x044C045B, "cFSMCharacterBase::CheckPosSetParameter"},
    {0x4B3EABE1, "cFSMFighter::AIPermitSetParameter"},
    {0x170904BF, "rEffectProvider::E2DGroup"},
    {0x4858746A, "rEffectProvider::E2DMaterial"},
    {0x7D995D73, "AIPlanFilterTargetDir"},
    {0x0F7A1BA5, "cAIPlanFilterIsAction"},

    /*
    cAIPlanFilterObject
    cAIPlanFilterTrue
    cAIPlanFilterSnekingLevel
    cAIPlanFilterSeExist
    cAIPlanFilterEpisode
    cAIPlanFilterTargetDir
    cAIPlanFilterReload
    cAIPlanFilterIsAction
    cAIPlanFilterNetRecieveDataPost
    cAIPlanFilterIsPlayerAuto
    cAIPlanFilterYattaEnd
    cAIPlanFilterWpChange
    cAIPlanFilterDataPostBOT
    cAIPlanFilterGetAkEgg
    cAIPlanFilterGetWp
    cAIPlanFilterReleaseWp
    cAIPlanFilterHelpEnergy
    cAIPlanFilterRunToTargetBot
    cAIPlanFilterRunToReaderBot
    cAIPlanFilterDevelop
    cAIPlanFilterDefenceAkEgg
    cAIPlanFilterAvoidLOF

    cAIPlanEvaluateObject
    cAIPlanEvaluateDefault
    cAIPlanEvaluateDangerArea
    cAIPlanEvaluateReload
    cAIPlanEvaluateGetWp
    cAIPlanEvaluateReleaseWp
    cAIPlanEvaluateDataPostOn
    cAIPlanEvaluateHelpFriendHarmo
    cAIPlanEvaluateDangerousTarget
    cAIPlanEvaluateWpChange
    cAIPlanEvaluateAvoidLOF

    cAIPlanMoveObject
    cAIPlanMoveAvoidLOF
    cAIPlanMoveSuitWayPoint
    cAIPlanMoveTargetPosSetDir
    cAIPlanMoveHarmoControl
    cAIPlanMoveAreaInfoRefresh
    cAIPlanMoveShotGranade
    cAIPlanMoveFSMCommandSet
    cAIPlanMoveJumpControl
    cAIPlanMoveShotEnergy*/
};

using pair_type = std::pair<es::string_view, es::string_view>;

std::pair<uint32 const, pair_type> GetPair(uint32 hash, Platform platform);

namespace revil {
es::string_view GetClassName(uint32 hash, Platform platform) {
  auto pair = GetPair(hash, platform);

  if (pair.first) {
    return pair.second.second;
  }

  auto found = classNames.find(hash);

  if (!es::IsEnd(classNames, found)) {
    return found->second;
  }

  return {};
}
} // namespace revil
