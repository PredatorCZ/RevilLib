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
#include "ext_dd.hpp"
#include "ext_dr.hpp"
#include "ext_lp.hpp"
#include "ext_lp2.hpp"
#include "ext_mh3.hpp"
#include "ext_mh4.hpp"
#include "ext_mhg.hpp"
#include "ext_mhs.hpp"
#include "ext_pwaadd.hpp"
#include "ext_pwaasoj.hpp"
#include "ext_re0.hpp"
#include "ext_re5.hpp"
#include "ext_re6.hpp"
#include <map>

using pair_type = std::pair<es::string_view, es::string_view>;

static const std::map<uint32, pair_type> extensions{
    {0x000692F5, {"amskl", "rAmuletSkillData"}},                 //
    {0x0022FA09, {"hpe", "rHumanPartsEdit"}},                    //
    {0x0026E7FF, {"ccl", "rChainCol"}},                          //
    {0x0086B80F, {"plexp", "rPlExp"}},                           //
    {0x00AD4093, {"w12d", "rWeapon12BaseData"}},                 //
    {0x00E0BB1C, {"qdl", "rQuestLink"}},                         //
    {0x00FDA99B, {"ntr", "rArmorTableNpc"}},                     //
    {0x01337721, {"tstd", "rTalkSelectData"}},                   //
    {0x017A550D, {"lom", "rLotSelectorMove"}},                   //
    {0x017CA1B2, {"ased", "rArmorSeData"}},                      //
    {0x018735A6, {"hkm", "rHkMaterial"}},                        //
    {0x01F28535, {"bds2", "rDoorScene"}},                        //
    {0x02009C23, {"mkr", "rMarkerData"}},                        //
    {0x02178810, {"gar", "rGroundAdjustment"}},                  //
    {0x02358E1A, {"spkg", "rShaderPackage"}},                    //
    {0x02373BA7, {"spn", "rStagePlaceName"}},                    //
    {0x0253F147, {"hit", "rHit"}},                               //
    {0x02833703, {"efs", "rEffectStrip"}},                       //
    {0x02929600, {"qdm", "rQuestDaily"}},                        //
    {0x02A80E1F, {"lrd", "rLinkRagdoll"}},                       //
    {0x02DD067F, {"kod", "rKowareObjData"}},                     //
    {0x02F3A8C4, {"w10d", "rWeapon10LevelData"}},                //
    {0x0315E81F, {"sds", "rSoundDirectionalSet"}},               //
    {0x039D71F2, {"rvt", "rSoundReverbTable"}},                  //
    {0x03FAE282, {"efa", "rEffectAnimation"}},                   //
    {0x041925D7, {"mvp", "rMonNyanVillagePoint"}},               //
    {0x0430075D, {"nis", "rNpcInitScript"}},                     //
    {0x0437BCF2, {"grw", "rGrassWind"}},                         //
    {0x044BB32E, {"ubcell", "rUBCell"}},                         //
    {0x049F01BD, {"wgi", "rWeaponGimmickInfo"}},                 //
    {0x04B4BE62, {"tmd", "rTalkMotion"}},                        //
    {0x04E9EBE9, {"cspp", "rCutScrParam"}},                      //
    {0x05249C19, {"sfcbd", "rSoundFSMCommandBgmData"}},          //
    {0x0525AEE2, {"wfp", "rWeatherFogParam"}},                   //
    {0x0532063F, {"ast", "rSoundAst"}},                          //
    {0x05332520, {"jnj", "rJinjyaData"}},                        //
    {0x053C8864, {"mps", "rPartsSelector"}},                     //
    {0x056CCDBC, {"tuto", "rTutorialFlowData"}},                 //
    {0x057C5F1D, {"fgd", "rFieldGateData"}},                     //
    {0x058ACE42, {"acn", "rAreaConnect"}},                       //
    {0x059BC928, {"loc2", "rArrange"}},                          //
    {0x059CD219, {"mmk", "rMapMarker"}},                         //
    {0x05A36D08, {"qif", "rQuestInfinity"}},                     //
    {0x05C89DEE, {"dptra", "rDungeonPassRareTbl"}},              //
    {0x05D0CAD3, {"fas", "rFieldAISetAct"}},                     //
    {0x060678F9, {"w12d", "rWeapon12LevelData"}},                //
    {0x0620FD81, {"oec", "rOtEquipCreate"}},                     //
    {0x065375D5, {"sis", "rStageInfoSet"}},                      //
    {0x069A1911, {"olp", "rOutlineParamList"}},                  //
    {0x069D4A3E, {"tdvs", "rTalkDemoViewSpriteData"}},           //
    {0x06C7327B, {"fmt", "rFueMusicData"}},                      //
    {0x06E58760, {"wet", "rWeatherData"}},                       //
    {0x0707698C, {"fas", "rFieldAISetKind"}},                    //
    {0x0737E28B, {"rst", "rRegionStatus"}},                      //
    {0x07437CCE, {"ase", "rSoundAttributeSe"}},                  //
    {0x079B5F3E, {"pci", "rAIPresetCharaInfo"}},                 //
    {0x07B8BCDE, {"fca", "rFacialAnimation"}},                   //
    {0x07D3088E, {"mdi.xml", "rModelInfo"}},                     //
    {0x07D5909F, {"stq", "rSoundStreamRequest"}},                //
    {0x07F768AF, {"gii", "rGUIIconInfo"}},                       //
    {0x086AEE8E, {"swp", "rShadowParamNative"}},                 //
    {0x089BEF2C, {"sap", "rAIStageActionParameter"}},            //
    {0x08EF36C1, {"modlayout.xml", "rModelLayout"}},             //
    {0x08F105E6, {"dcm", "rDemoCamera"}},                        //
    {0x0947F3C8, {"areapatrol", "rAreaPatrolData"}},             //
    {0x094973CF, {"scs", "rSoundCurveSet"}},                     //
    {0x0990B835, {"sad", "rSoundArmorData"}},                    //
    {0x09D775FC, {"efcc", "rEffectColorCorrect"}},               //
    {0x0A0E48D4, {"emd", "rEnemyData"}},                         //
    {0x0A164982, {"gmt", "rGeneMatrixTable"}},                   // table??
    {0x0A3FE3F4, {"fssd", "rFieldSchedulerSetData"}},            //
    {0x0A4280D9, {"shd", "rSoundHitData"}},                      //
    {0x0A74682F, {"rnp", "rRomNoraPawn"}},                       //
    {0x0A7DB17E, {"deco", "rDecoData"}},                         //
    {0x0AAF2DB2, {"rtl", "rTexLayout"}},                         //
    {0x0AB54515, {"bui", "rBui"}},                               //
    {0x0AF34367, {"dtt", "rDungeonTowerTbl"}},                   //
    {0x0B0B8495, {"smadd.xml", "rSMAdd"}},                       //
    {0x0B1808BE, {"w14d", "rWeapon14LevelData"}},                //
    {0x0B2FACE7, {"bscn", "rSceneInfo"}},                        //
    {0x0B452BD2, {"fed", "rFieldEnemyPathData"}},                //
    {0x0B842B37, {"mrt", "rMonsterRankTable"}},                  //
    {0x0BCF07BD, {"pcos", "rPlayerCostume"}},                    //
    {0x0C0FAB06, {"otml", "rOtMessageLot"}},                     //
    {0x0C41C74D, {"lpm", "rLayoutParameter"}},                   //
    {0x0C4FCAE4, {"PlDefendParam", "rPlDefendParam"}},           //
    {0x0C6016B9, {"htd", "rHitData"}},                           //
    {0x0C6C4DBF, {"kcr", "rKitchenListSkillRandom"}},            //
    {0x0C6D4399, {"sid", "rSetItemData"}},                       //
    {0x0C7468EA, {"kcm", "rKitchenListMenu"}},                   //
    {0x0D06BE6B, {"tmn", "rTargetMotion"}},                      //
    {0x0D3BE7B5, {"sbl", "rSceneBoxLayout"}},                    //
    {0x0DADAB62, {"oba", "rObjAdj"}},                            //
    {0x0DD0E47A, {"AngleLimit", "rAngleLimitData"}},             //
    {0x0DE13237, {"pai", "rThinkPlanAI"}},                       //
    {0x0E16DFBA, {"cit", "rItemCurseCnv"}},                      //
    {0x0ECD7DF4, {"scs", "rSoundCurveSet"}},                     //
    {0x0F86C052, {"oskl", "rOtSkill"}},                          //
    {0x0F9F3E69, {"pmtt", "rPlayerMotionTimeTable"}},            //
    {0x1041BD9E, {"mod", "rModel"}},                             //
    {0x10C460E6, {"msg", "rMessage"}},                           //
    {0x10CEDAEA, {"pcl", "rPieceCreateList"}},                   //
    {0x10EBE843, {"mss", "rFestaSoundSequence"}},                //
    {0x117D7CD0, {"mcm", "rMonNyanCommonMaterial"}},             //
    {0x11AFA688, {"sdl.xml", "rScheduler"}},                     //
    {0x11B6FD48, {"tdmact", "rTalkDemoActorData"}},              //
    {0x11C35522, {"gr2", "rGrass2"}},                            //
    {0x11C82587, {"oba", "rObjAdj"}},                            //
    {0x11DE9EF6, {"isd", "rInsectData"}},                        //
    {0x12191BA1, {"epv", "rEffectProvider"}},                    //
    {0x122CA603, {"fnbd", "rFieldNestBossData"}},                //
    {0x124597FE, {"xml", "rEventTimeSchedule"}},                 //
    {0x125F7431, {"ssd", "rSoundSOMData"}},                      //
    {0x12688D38, {"pjp", "rPlJumpParam"}},                       //
    {0x12A794D8, {"ain", "rAinpc"}},                             //
    {0x12C3BFA7, {"cpl", "rCameraParamList"}},                   //
    {0x133917BA, {"mss", "rMsgSet"}},                            //
    {0x134F74D8, {"fpd", "rFieldHuntingData"}},                  //
    {0x136EBC7F, {"sisd", "rSoundIngredientSeData"}},            //
    {0x139EE51D, {"lmt", "rMotionList"}},                        //
    {0x141C243A, {"insectabirity", "rInsectAbirity"}},           //
    {0x141C421D, {"stdsc", "rSoundTalkDemoSeControl"}},          //
    {0x1431ED71, {"esc", "rEggSpecialColorData"}},               //
    {0x14428EAE, {"gce", "rCharacterEditPreset"}},               //
    {0x14483CFB, {"bplt", "rBattlePlayerTbl"}},                  //
    {0x1479E09D, {"ftd", "rFieldTagData"}},                      //
    {0x148B6F89, {"mef", "rMhMotionEffect"}},                    //
    {0x148FA98A, {"gcd", "rGatherCommentData"}},                 //
    {0x14B5C8E6, {"sbkr", "rLtSoundBank"}},                      //
    {0x14EA8095, {"cos", "rCnsOffsetSet"}},                      //
    {0x15155F8A, {"smh", "rSMHiding"}},                          //
    {0x15302EF4, {"lot", "rLayout"}},                            //
    {0x156A8085, {"fmi", "rFueMusicInfData"}},                   //
    {0x157388D3, {"itl", "rItemList"}},                          //
    {0x15773620, {"nmr", "rArmorModelNpc"}},                     //
    {0x15D782FB, {"sbkr", "rSoundBank"}},                        //
    {0x15E98D21, {"areaseldat", "rAreaSelectData"}},             //
    {0x15FCE6C6, {"wpd", "rWeaponProcessData"}},                 //
    {0x160329F8, {"rem", "rGuestRemData"}},                      //
    {0x162E07FB, {"lacd", "rArchiveLayoutCmnData"}},             //
    {0x164F60FF, {"nld", "rNpcLocateData"}},                     //
    {0x1657A2D0, {"sptl", "rSoundPronounceList"}},               //
    {0x167DBBFF, {"stq", "rSoundStreamRequest"}},                //
    {0x169B7213, {"irp", "rItemRandParam"}},                     //
    {0x176C3F95, {"los", "rLotSelector"}},                       //
    {0x176FADAB, {"scm", "rStageCamera"}},                       //
    {0x17D654D0, {"w13d", "rWeapon13BaseData"}},                 //
    {0x1800EB37, {"emyure", "rEmSizeYureTbl"}},                  //
    {0x1816EB57, {"bwpt", "rBattleWeaponTbl"}},                  //
    {0x1823137D, {"mlm", "rMapLndMarkPos"}},                     //
    {0x18A9225C, {"pma", "rPlayerManyAttacks"}},                 //
    {0x18C80D7A, {"fntl", "rFieldNpcNekoTaxiList"}},             //
    {0x18D25FEC, {"ccinfo", "rConditionChangeInfo"}},            //
    {0x19054795, {"nnl", "rNpcLedgerList"}},                     //
    {0x191F0AC9, {"fur", "rFurTable::FurTable"}},                //
    {0x19278D07, {"skst", "rSwkbdMessageStyleTable"}},           //
    {0x193175EA, {"gfx", "rGeneFix"}},                           //
    {0x194BE415, {"chr", "rCharacter"}},                         //
    {0x197E4D7A, {"maptime", "rMapTimeData"}},                   //
    {0x199C56C0, {"ocl", "rObjCollision"}},                      //
    {0x19A59A91, {"llk", "sLightLinker::rLightLinker"}},         //
    {0x19C994C4, {"mex", "rMonNyanExp"}},                        //
    {0x19F2AB31, {"tpil", "rTradePointItemList"}},               //
    {0x19F6EFCE, {"sep", "rSoundEnemyParam"}},                   //
    {0x1A032CE2, {"dmem", "rDemoEnemy"}},                        //
    {0x1A273E8B, {"npcSd", "rNpcSubData"}},                      //
    {0x1A2B1A25, {"deep", "rDungeonDeepTbl"}},                   //
    {0x1A3E9CBA, {"ntd", "rNpcTalkData"}},                       //
    {0x1AADF7B7, {"cms", "rCameraMotionSdl"}},                   //
    {0x1AC7B52D, {"bdd", "rBodyData"}},                          //
    {0x1AE50150, {"vts", "rVertices"}},                          //
    {0x1AEB54D1, {"hvl.xml", "rHavokVertexLayout"}},             //
    {0x1B520B68, {"zon", "rZone"}},                              //
    {0x1BA81D3C, {"nck", "rNeck"}},                              //
    {0x1BBC291E, {"dtp", "rEnemyDtBaseParts"}},                  //
    {0x1BBFD18E, {"quest", "rQuestData"}},                       //
    {0x1BCC4966, {"srq", "rSoundRequest"}},                      //
    {0x1C2B501F, {"atr", "rArmorTable"}},                        //
    {0x1C635F38, {"msl", "rMaterialSkipList"}},                  //
    {0x1C755227, {"w04d", "rWeapon04BaseData"}},                 //
    {0x1C775347, {"sbl", "rSceneBox2"}},                         //
    {0x1D35AF2B, {"hit", "rHit"}},                               //
    {0x1D8CC9B7, {"fol", "rFieldObjectList"}},                   //
    {0x1DC202EA, {"stef", "rSceneEffect"}},                      //
    {0x1E01F870, {"poom", "rPreOcclusionOneModel"}},             //
    {0x1E329EFC, {"rlt", "rRelationData"}},                      //
    {0x1EB12C38, {"sep", "rShellEffectParam"}},                  //
    {0x1EB3767C, {"spr", "rSoundPhysicsRigidBody"}},             //
    {0x1ED12F1B, {"glp", "rGoalPos"}},                           //
    {0x1EE1ED64, {"arealinkdat", "rAreaLinkData"}},              //
    {0x1EFB1B67, {"adh", "rAdhesion"}},                          //
    {0x1F149AEC, {"mdd", "rEmDouMouKaData"}},                    //
    {0x1F8BAD22, {"snd", "rSoundNpcData"}},                      //
    {0x1F9FA62C, {"rbl2", "rRumble"}},                           //
    {0x1FA3FF7A, {"nasl", "rNpcAirouSetResourceLogDataNative"}}, //
    {0x1FA8B594, {"ahs", "rAreaHit"}},                           //
    {0x20208A05, {"mtg", "rModelMontage"}},                      //
    {0x2052D67E, {"sn2", "rAISensorExt"}},                       //
    {0x2074FB1F, {"srcd", "rStaffRollCutData"}},                 //
    {0x20A81BF0, {"pef", "rAIPositionEvaluationFuncList"}},      //
    {0x20E0A843, {"dptev", "rDungeonPassEventTbl"}},             //
    {0x20ED9750, {"pep", "rProofEffectParamScript"}},            //
    {0x20FA758C, {"gnd", "rFlashFontConfig"}},                   //
    {0x21034C90, {"arc", "rArchive"}},                           //
    {0x2141D3A9, {"wpp", "rWeaponParam"}},                       //
    {0x215896C2, {"statusparam", "rStatusParam"}},               //
    {0x215E5305, {"vjr", "rVirtualJoint"}},                      //
    {0x21684FE4, {"clc", "rColorLinkColor"}},                    //
    {0x21A16C7D, {"w07d", "rWeapon07LevelData"}},                //
    {0x21F33E01, {"amslt", "rAmuletSlotData"}},                  //
    {0x2229D051, {"dcd", "rDecoCreateData"}},                    //
    {0x2282360D, {"jex", "rJointEx"}},                           //
    {0x228736BF, {"fsosl", "rFieldSoundOrnamentSeList"}},        //
    {0x22948394, {"gui", "rGUI"}},                               //
    {0x22B2A2A2, {"PlNeckPos", "rPlNeckPos"}},                   //
    {0x2325C7A0, {"mectd", "rMonsterEnumConversionTable"}},      //
    {0x232E228C, {"rev_win", "rReverb"}},                        //
    {0x233A2C4D, {"plweplist", "rPlayerWeaponList"}},            //
    {0x233A9F13, {"kcg", "rKitchenListGrillItem"}},              //
    {0x234D7104, {"cdf", "rCloth"}},                             //
    {0x2350E584, {"obc", "rCollisionObj"}},                      //
    {0x236C1AAB, {"lad", "rArchiveLayoutData"}},                 //
    {0x237E6120, {"kc1", "rKitchenListSuccessTable1"}},          //
    {0x238D25D2, {"w07m", "rWeapon07MsgData"}},                  //
    {0x23AC90FB, {"sod", "rSoundObjectData"}},                   //
    {0x23C997E4, {"bft", "rBuddyFlagTable"}},                    //
    {0x24080FFF, {"fht", "rFreeHuntData"}},                      //
    {0x241F5DEB, {"tex", "rTexture"}},                           //
    {0x242BB29A, {"gmd", "rGUIMessage"}},                        //
    {0x2447D742, {"idm", "sId::rIdMapList"}},                    //
    {0x245133D9, {"cmr", "rCameraRail"}},                        //
    {0x24F0A487, {"kcs", "rKitchenListSkillSet"}},               //
    {0x24F56BC0, {"smd", "rSoundMonsterData"}},                  //
    {0x252BD342, {"ebcd", "rEquipBaseColorData"}},               //
    {0x25334DDE, {"trdl", "rTradeDeliveryList"}},                //
    {0x254309C9, {"psl", "rProofEffectMotSequenceList"}},        //
    {0x2543B93E, {"ses", "rFestaSoundEmitter"}},                 //
    {0x254A0337, {"rvl", "rVramLoadList"}},                      //
    {0x2553701D, {"sem", "rSetEmMain"}},                         //
    {0x2555081D, {"stb", "rStoryTalkBalloon"}},                  //
    {0x257D2F7C, {"swm", "rSwingModel"}},                        //
    {0x25A60BC4, {"ssjje", "rSansaijijiExchange"}},              //
    {0x25B4A6B9, {"bgm", "rSoundBGMControl"}},                   //
    {0x25E98D8C, {"rclp", "rRoomCutList"}},                      //
    {0x25F86EE2, {"w07d", "rWeapon07BaseData"}},                 //
    {0x25FD693F, {"ipl", "rItemPopList"}},                       //
    {0x2618DE3F, {"srq", "rLtSoundRequest"}},                    //
    {0x264087B8, {"fca", "rFacialAnimation"}},                   //
    {0x2662A59D, {"bcmr", "rBattleCommonResource"}},             //
    {0x266E8A91, {"lku", "rLinkUnit"}},                          //
    {0x26BEC21C, {"qdp", "rQuestPlus"}},                         //
    {0x26C299D0, {"hvd.xml", "rHavokVehicleData"}},              //
    {0x271D08FE, {"ssq", "rSoundSequenceSe"}},                   //
    {0x272B80EA, {"prp", "rPropParam"}},                         //
    {0x2739B57C, {"grs", "rGrass"}},                             //
    {0x2749C8A8, {"mrl", "rMaterial"}},                          //
    {0x2754877D, {"trll", "rTradeLotList"}},                     //
    {0x276DE8B7, {"e2d", "rEffect2D"}},                          //
    {0x27C72B28, {"w03m", "rWeapon03MsgData"}},                  //
    {0x27CE98F6, {"rtex", "rRenderTargetTexture"}},              //
    {0x27D81C81, {"fes", "rFieldSet"}},                          //
    {0x284ACC07, {"w03d", "rWeapon03LevelData"}},                //
    {0x284FCD6E, {"ebc", "rEggBaseColorData"}},                  //
    {0x285A13D9, {"vzo", "rFxZone"}},                            //
    {0x28BAE197, {"sed", "rStEpiData"}},                         //
    {0x28C32975, {"tdms", "rTalkDemoScript"}},                   //
    {0x29482CCB, {"w00m", "rWeapon00MsgData"}},                  //
    {0x294A5E8D, {"hta", "rHunterArtsData"}},                    //
    {0x2957DDCD, {"fppnr", "rFldPlParam_NR"}},                   //
    {0x296BD0A6, {"hgm", "rHitGeometry"}},                       //
    {0x29751294, {"tril", "rTradeItemList"}},                    //
    {0x2993EA18, {"gnd", "rFlashFontConfig"}},                   //
    {0x29948FBA, {"srd", "rSoundRandom"}},                       //
    {0x29A5C1D1, {"rdp", "rRideParamNative"}},                   //
    {0x2A03657D, {"slw02", "rEquipShopListW02"}},                //
    {0x2A06CC14, {"tdmfc", "rTalkDemoFaceData"}},                //
    {0x2A110586, {"slw11", "rEquipShopListW11"}},                //
    {0x2A25AF1B, {"bnt", "rBattleNavirouTable"}},                //
    {0x2A37242D, {"gpl", "rLayoutGroupParamList"}},              //
    {0x2A4F96A8, {"rbd", "rRigidBody"}},                         //
    {0x2A51D160, {"ems", "rEnemyLayout"}},                       //
    {0x2A68813F, {"mlc", "rMonNyanLotCommon"}},                  //
    {0x2A8800EE, {"sai", "rStageAreaInfo"}},                     //
    {0x2A9FBCEC, {"hmg", "rHmGenerator"}},                       //
    {0x2AC7B904, {"dpttr", "rDungeonPassRoomMaxTbl"}},           //
    {0x2B0670A5, {"map", "rMagicActParamList"}},                 //
    {0x2B303957, {"gop", "rAIGoalPlanning"}},                    //
    {0x2B399B97, {"evc2", "rEventCollision"}},                   //
    {0x2B40AE8F, {"equ", "rSoundEQ"}},                           //
    {0x2B93C4AD, {"route", "rRouteNode"}},                       //
    {0x2BCFB893, {"otil", "rOtIniLot"}},                         //
    {0x2C14D261, {"ahl", "rAreaHitLayout"}},                     //
    {0x2C2DE8CA, {"adh", "rAdh"}},                               //
    {0x2C4666D1, {"srh", "rScrHiding"}},                         // (RE5: smh)
    {0x2C59EEA9, {"sksg", "rSwkbdSubGroup"}},                    //
    {0x2C97686F, {"mhd", "rMovieHitData"}},                      //
    {0x2CBF1C3A, {"w01d", "rWeapon01LevelData"}},                //
    {0x2CC70A66, {"grd", "rFlashDef"}},                          //
    {0x2CE309AB, {"joblvl", "rPlJobLevel"}},                     //
    {0x2D022231, {"w04m", "rWeapon04MsgData"}},                  //
    {0x2D0B48AC, {"sbsdef", "rSoundBattleStageDefine"}},         //
    {0x2D12E086, {"srd", "rSoundRandom"}},                       //
    {0x2D462600, {"gfd", "rGUIFont"}},                           //
    {0x2D4CF80A, {"cli", "rColorLinkInfo"}},                     //
    {0x2D6455B1, {"tde", "rTexDetailEdit"}},                     //
    {0x2D6EA164, {"slw06", "rEquipShopListW06"}},                //
    {0x2DC54131, {"cdf", "rCloth"}},                             //
    {0x2DECBCA6, {"fpd", "rFieldPredatorData"}},                 //
    {0x2E47C723, {"seg", "rSoundSeg"}},                          //
    {0x2E590330, {"rsl", "rScrList"}},                           //
    {0x2E5B6815, {"w10d", "rWeapon10BaseData"}},                 //
    {0x2E897056, {"ear", "rEnemyAreaRoute"}},                    //
    {0x2EC99875, {"fned", "rFieldNestEggData"}},                 //
    {0x2ECFC102, {"fppar", "rFldPlParam_AR"}},                   //
    {0x2EF9EEAF, {"fnd", "rFieldNestData"}},                     //
    {0x2F1C6767, {"saou", "rOtSupportActionOtUnique"}},          //
    {0x2F4E7041, {"sgt", "rSoundGunTool"}},                      //
    {0x2FC0C6C5, {"tucyl", "rTutorialCylinderData"}},            //
    {0x2FE088C0, {"dpd", "rDollPartsDisp"}},                     //
    {0x300DB281, {"igf", "rInsectGrowFeed"}},                    //
    {0x306945DB, {"ghlt", "rGatherLevelTable"}},                 //
    {0x3077D00E, {"dmd", "rDemoData"}},                          //
    {0x30991F46, {"frl", "rFestaResourceList"}},                 //
    {0x30BC3F6B, {"w13m", "rWeapon13MsgData"}},                  //
    {0x30ED4060, {"pth", "rPath"}},                              //
    {0x30FC745F, {"smx", "rSoundSubMixer"}},                     //
    {0x31095696, {"fmpd", "rFieldMotionPackageData"}},           //
    {0x310C90B8, {"fbd", "rFieldPlayerMotionData"}},             //
    {0x312607A4, {"bll", "rBlacklist"}},                         //
    {0x31798063, {"nhap", "rNestHappening"}},                    //
    {0x31AC0B5C, {"swc", "rScrCollisionArea"}},                  //
    {0x31B81AA5, {"qr", "rQuestReward"}},                        //
    {0x31EDC625, {"spj", "rSoundPhysicsJoint"}},                 //
    {0x32231BD1, {"esd", "rEffectSetData"}},                     //
    {0x325774D5, {"fppwr", "rFldPlParam_WR"}},                   //
    {0x325AACA5, {"shl", "rShlParamList"}},                      //
    {0x327E327E, {"abd", "rArmorBuildData"}},                    //
    {0x32837AA1, {"w06d", "rWeapon06BaseData"}},                 //
    {0x32CA92F8, {"esl", "rEmSetList"}},                         //
    {0x32E2B13B, {"rdp", "rEditPawn"}},                          //
    {0x33046CD5, {"qcm", "rCameraQFPS"}},                        //
    {0x330A34C7, {"slw01", "rEquipShopListW01"}},                //
    {0x3318543C, {"slw12", "rEquipShopListW12"}},                //
    {0x332CF371, {"ssjjp", "rSansaijijiPresent"}},               //
    {0x336D826C, {"ext", "rSkillTable"}},                        //
    {0x338C1FEC, {"asl", "rAreaSetLayout"}},                     //
    {0x33A84E14, {"hgi", "rHagi"}},                              //
    {0x33AE5307, {"spc", "rSoundPackage"}},                      //
    {0x33B21191, {"esp", "rEmShaderParam"}},                     //
    {0x33B68E3E, {"xml", "rItemLayout"}},                        //
    {0x340F49F9, {"sds", "rSoundDirectionalSet"}},               //
    {0x3443F314, {"iaf", "rInsectAttrFeed"}},                    //
    {0x344FD684, {"dqt", "rDungeonQuestData"}},                  //
    {0x3464995E, {"ots", "rOtodokeSetList"}},                    //
    {0x3488527B, {"npcMd", "rNpcMoveData"}},                     //
    {0x34886F87, {"plt", "rPlayerTitle"}},                       //
    {0x34A8C353, {"sprmap", "rSprLayout"}},                      //
    {0x34BDFC5B, {"fas", "rFieldAISet"}},                        //
    {0x3516C3D2, {"lfd", "rLayoutFont"}},                        //
    {0x354284E7, {"lvl", "rPlLevel"}},                           //
    {0x358012E8, {"vib", "rVibration"}},                         //
    {0x35BDD173, {"poa", "rPosAdjust"}},                         //
    {0x36019854, {"bed", "rBodyEdit"}},                          //
    {0x360737E0, {"w09m", "rWeapon09MsgData"}},                  //
    {0x3654EBA0, {"sce", "rCallingEncountData"}},                //
    {0x368E9519, {"bgsd", "rBowgunShellData"}},                  //
    {0x36C909FD, {"mre", "rMonNyanRewardEnemy"}},                //
    {0x36C92C9A, {"fed", "rFieldEnemyPlanData"}},                //
    {0x36E29465, {"hkx", "rHkCollision"}},                       //
    {0x370AD68B, {"tdd", "rTalkDemoDialogData"}},                //
    {0x3718238F, {"fgi", "rFieldGridInfo"}},                     //
    {0x3718B9C7, {"dptch", "rDungeonPassChallengeTbl"}},         //
    {0x3756EE15, {"ptex", "rLtProceduralTexture"}},              //
    {0x3759B207, {"sbsd", "rSoundBattleStageData"}},             //
    {0x37B53731, {"ibl", "rIntelligenceBox"}},                   //
    {0x3821B94D, {"sngw", "rSoundSourceMusic"}},                 //
    {0x38534E81, {"isa", "rInsectAttr"}},                        //
    {0x3876D6FA, {"npsl", "rNpcSetResourceLogDataNative"}},      //
    {0x38F66FC3, {"seg", "rSoundSeGenerator"}},                  //
    {0x3900DAD0, {"sbc", "rCollision"}},                         //
    {0x39207C56, {"w11d", "rWeapon11BaseData"}},                 //
    {0x3991981E, {"msd", "rMirrorSetData"}},                     //
    {0x39A0D1D6, {"sms", "rSoundSubMixerSet"}},                  //
    {0x39C52040, {"lcm", "rCameraList"}},                        //
    {0x39C8DC68, {"skd", "rSkillData"}},                         //
    {0x3A244568, {"nhapp", "rNestHappeningProb"}},               //
    {0x3A298CCF, {"btat", "rBattleAtk"}},                        //
    {0x3A6A5A4D, {"stq", "rLtSoundStreamRequest"}},              //
    {0x3A77309A, {"kc2", "rKitchenListSuccessTable2"}},          //
    {0x3A793672, {"w14m", "rWeapon14MsgData"}},                  //
    {0x3A873D57, {"fesd", "rFieldEnemySetData"}},                //
    {0x3A8B6D04, {"epd", "rEggPatternData"}},                    //
    {0x3A947AC1, {"cql", "rCameraQuakeList"}},                   //
    {0x3AABBA02, {"emc", "rEnemyCmd"}},                          //
    {0x3AF73507, {"emt", "rEnemyTurnPos"}},                      //
    {0x3B04F5A1, {"ape", "rAcPlayerEquip"}},                     //
    {0x3B350990, {"qsp", "rQuestSudden"}},                       //
    {0x3B5C7FD3, {"ida", "rIdAnim"}},                            //
    {0x3B764DD4, {"sstr", "rSoundStreamTransition"}},            //
    {0x3BBA4E33, {"qct", "rQuestCtrlTbl"}},                      //
    {0x3BFEDD61, {"emm", "rEnemyMapPos"}},                       //
    {0x3C285CC6, {"otd", "rOtTensionData"}},                     //
    {0x3C318636, {"wao", "rWeaponAttachOffset"}},                //
    {0x3C3D0C05, {"hkx", "rHkCollision"}},                       //
    {0x3C4BD0ED, {"rcd", "rReactionCommentData"}},               //
    {0x3C51E03C, {"ard", "rArmorResistData"}},                   //
    {0x3C56A1F1, {"ssdc", "rSoundShortDemoControl"}},            //
    {0x3C6F8994, {"fcr", "rFieldCamera"}},                       //
    {0x3CAD8076, {"tex", "rTexture"}},                           //
    {0x3CBBFD41, {"ucp2", "rUpCutPuzzle"}},                      //
    {0x3D007115, {"wed", "rSoundWed"}},                          //
    {0x3D8BBBAC, {"etd", "rEnemyTuneData"}},                     //
    {0x3D97AD80, {"amr", "rArmorModel"}},                        //
    {0x3DD1BCF5, {"slw09", "rEquipShopListW09"}},                //
    {0x3E06712D, {"cnd", "rConditionNameData"}},                 //
    {0x3E2A4D8E, {"amlt", "rAmuletData"}},                       //
    {0x3E333888, {"w10m", "rWeapon10MsgData"}},                  //
    {0x3E356F93, {"stc", "rStarCatalog"}},                       //
    {0x3E363245, {"chn", "rChain"}},                             //
    {0x3E394A0E, {"npcfsmbrn", "rFSMBrain"}},                    //
    {0x3E4BFC33, {"gtb", "rGeneTableTable"}},                    // table??
    {0x3EE10653, {"wad", "rWeskerAttackData"}},                  //
    {0x3F13DE3C, {"ots", "rResearchReinforce"}},                 //
    {0x3F1513D0, {"fbd", "rFieldBuddyMotionData"}},              //
    {0x3F53ECC9, {"mpd", "rMonsterPartsDisp"}},                  //
    {0x3F685CCE, {"w09d", "rWeapon09LevelData"}},                //
    {0x3FA5AD6A, {"ict", "rItemCategoryTypeData"}},              //
    {0x3FB52996, {"imx", "rItemMix"}},                           //
    {0x3FDA9B90, {"gmd", "rFlashMessage"}},                      //
    {0x4042405A, {"fssl", "rFieldSoundIngredientSeList"}},       //
    {0x4046F1E1, {"ajp", "rAdjustParam"}},                       //
    {0x40596F49, {"stdc", "rSoundTalkDemoControl"}},             //
    {0x405FF76E, {"bct2", "rBgChangeTable"}},                    //
    {0x40D14033, {"gfc", "rFlashConfig"}},                       //
    {0x4126B31B, {"npcmac", "rNMMachine"}},                      //
    {0x4199032B, {"w00d", "rWeapon00BaseData"}},                 //
    {0x41E33404, {"lan", "rLayoutAnime"}},                       //
    {0x42354DC6, {"wcd", "rWeaponCreateData"}},                  //
    {0x43057C7C, {"alc", "rAlchemyData"}},                       //
    {0x43062A33, {"npcId", "rNpcBaseData_ID"}},                  //
    {0x430B4FF4, {"ptl", "rPathList"}},                          //
    {0x430F0258, {"tcil", "rTradeCoinTicketList"}},              //
    {0x4323D83A, {"stex", "rSceneTexture"}},                     //
    {0x433B8D7E, {"tdmpos", "rTalkDemoPoseData"}},               //
    {0x4360C048, {"slw04", "rEquipShopListW04"}},                //
    {0x437662FC, {"oml", "rOmList"}},                            //
    {0x437D7704, {"w00d", "rWeapon00LevelData"}},                //
    {0x440D0451, {"slw00", "rEquipShopListW00"}},                //
    {0x441F64AA, {"slw13", "rEquipShopListW13"}},                //
    {0x44A96149, {"acs", "rAccessorySkill"}},                    //
    {0x44C8AC26, {"mloc", "rMapLocate"}},                        //
    {0x44DC7515, {"ssdsc", "rSoundShortDemoSeControl"}},         //
    {0x44E79B6E, {"sdl", "rScheduler"}},                         //
    {0x44EE724E, {"scd", "rStChapData"}},                        //
    {0x4509FA80, {"itemlv", "rItemLevelParam"}},                 //
    {0x450A16E3, {"pntpos", "rPointPos"}},                       //
    {0x4541367C, {"pts", "rFestaPelTiedSe"}},                    //
    {0x456B6180, {"cnsshake", "rCnsShake"}},                     //
    {0x458DE878, {"sad", "rSpActData"}},                         //
    {0x459AFA94, {"dgf", "rDungeonFieldData"}},                  //
    {0x45CFC491, {"skf", "rSkillFlag"}},                         //
    {0x45E867D7, {"mll", "rMotionListList"}},                    //
    {0x45F753E8, {"igs", "rInGameSound"}},                       //
    {0x4601A201, {"bnm", "rBattleNavirouMes"}},                  //
    {0x46810940, {"egv", "rSoundEngineValue"}},                  //
    {0x46AADC49, {"swd", "rSoundWeaponData"}},                   //
    {0x46FB08BA, {"bmt", "rBioModelMontage"}},                   //
    {0x47165A39, {"fnmd", "rFieldNpcMotion"}},                   //
    {0x472022DF, {"AIPlActParam", "rAIPlayerActionParameter"}},  //
    {0x4788A739, {"w02d", "rWeapon02LevelData"}},                //
    {0x482B5B95, {"esl", "rEffectSetData"}},                     //
    {0x482DDCBE, {"eml", "rEnemyLandingPos"}},                   //
    {0x48459606, {"seq", "rSoundSeq"}},                          //
    {0x48538FFD, {"ist", "rItemSetTbl"}},                        //
    {0x4857FD94, {"rev_win", "rReverb"}},                        //
    {0x486B233A, {"sdtl", "rStageData"}},                        //
    {0x488DAB8D, {"acr", "rAccessoryRare"}},                     //
    {0x48920641, {"dpt", "rDungeonPopTableSet"}},                //
    {0x48AE1387, {"tdmeff", "rTalkDemoEffectData"}},             //
    {0x48B16938, {"isl", "rInsectLevel"}},                       //
    {0x48B239C0, {"sddc", "rSoundDramaDemoControl"}},            //
    {0x48C0AF2D, {"msl", "rMsgSerial"}},                         //
    {0x48DFD78B, {"msgm", "rMessageData"}},                      //
    {0x48E8AC29, {"dtt", "rEnemyDtTune"}},                       //
    {0x496F8F22, {"fup", "rFreeUseParam"}},                      //
    {0x49B5A885, {"ssc", "rSoundSimpleCurve"}},                  //
    {0x4A31FCD8, {"scoop.xml", "rScoopList"}},                   //
    {0x4A32252D, {"sfcsd", "rSoundFSMCommandSeData"}},           //
    {0x4A4b677C, {"rev_ctr", "rLtSoundReverb"}},                 //
    {0x4A91DDB9, {"mle", "rMonNyanLotEnemy"}},                   //
    {0x4A96D77E, {"w04d", "rWeapon04LevelData"}},                //
    {0x4AA69872, {"shell", "rShell"}},                           //
    {0x4AD68C63, {"slw08", "rEquipShopListW08"}},                //
    {0x4B4C2BD3, {"plcmdtbllist", "rHitDataPlayer"}},            //
    {0x4B51E836, {"olvl", "rOtLevel"}},                          //
    {0x4B704CC0, {"mia", "rMagicItemActParamList"}},             //
    {0x4B768796, {"scn", "rSoundCondition"}},                    //
    {0x4B92D51A, {"llk", "rLightLinker"}},                       //
    {0x4BF84F6F, {"arealinkdat", "rAreaLinkData"}},              //
    {0x4C0DB839, {"sdl", "rScheduler"}},                         //
    {0x4C2446BF, {"nasl", "rPoogieSetResourceLogDataNative"}},   //
    {0x4C3942E3, {"skt", "rSkillTypeData"}},                     //
    {0x4CA26828, {"mse", "rSoundMotionSe"}},                     //
    {0x4CDF60E9, {"msg", "rMessage"}},                           //
    {0x4D70000C, {"kc3", "rKitchenListSuccessTable3"}},          //
    {0x4D87AEBA, {"bef", "rBattleEnemyFile"}},                   //
    {0x4D894D5D, {"cmi", "sCubeMapInterp::rCubeMapInterp"}},     //
    {0x4DEE8265, {"ems", "rEnemyStatus"}},                       //
    {0x4DEF5367, {"bdy", "rBuddyPathData"}},                     //
    {0x4E2EF008, {"hdp", "rHitDataPlayer"}},                     //
    {0x4E2FEF36, {"mtg", "rModelMontage"}},                      //
    {0x4E32817C, {"bfx", "rShader"}},                            //
    {0x4E397417, {"ean", "rEffectAnim"}},                        //
    {0x4E399FFE, {"spkg", "rEmulationShaderPackage"}},           //
    {0x4E44FB6D, {"fpe", "rFacePartsEdit"}},                     //
    {0x4E630743, {"w06d", "rWeapon06LevelData"}},                //
    {0x4EB68CEE, {"hds", "rHitDataShell"}},                      //
    {0x4ED7C110, {"sqd", "rSubQuestData"}},                      //
    {0x4EF19843, {"nav", "rNavigationMesh"}},                    //
    {0x4EF8A3D0, {"arp", "rArmorParam"}},                        //
    {0x4F1544F5, {"npd", "rNavirouPartsDisp"}},                  //
    {0x4F16B7AB, {"hri", "rHkRBInfoList"}},                      //
    {0x4F6FFDDC, {"fcp", "rFacialPattern"}},                     //
    {0x4FB35A95, {"aor", "rArmorPartsOff"}},                     //
    {0x5001FCC8, {"bstr", "rBattleStageResource"}},              //
    {0x50AA37F0, {"w08d", "rWeapon08LevelData"}},                //
    {0x50F3721F, {"thk", "rThinkScript"}},                       //
    {0x50F3D713, {"skl", "rSkillList"}},                         //
    {0x50F9DB3E, {"bfx", "rShader"}},                            //
    {0x5139901D, {"plbasecmd", "rPlBaseCmd"}},                   //
    {0x5175C242, {"geo2", "rGeometry2"}},                        //
    {0x51FC779F, {"sbc", "rCollision"}},                         //
    {0x5204D557, {"shp", "rShopList"}},                          //
    {0x522F7A3D, {"fcp", "rFacialPattern"}},                     //
    {0x525BBF16, {"esq", "rEffectSequence"}},                    //
    {0x52776AB1, {"ips", "rItemPopSet"}},                        //
    {0x528770DF, {"efs", "rEffectStrip"}},                       //
    {0x52DBDCD6, {"rdd", "rRagdoll"}},                           //
    {0x52F032FA, {"cndp", "rConditionPriorityData"}},            //
    {0x531EA143, {"uct2", "rUpCutObjTable"}},                    //
    {0x534BF1A0, {"ddfcv", "rDDFCurve"}},                        //
    {0x535D969F, {"ctc", "rCnsTinyChain"}},                      //
    {0x5376F652, {"tdmmot", "rTalkDemoMotionData"}},             //
    {0x538120DE, {"eng", "rSoundEngine"}},                       //
    {0x5400AB57, {"blyr", "rBattleLayoutResource"}},             //
    {0x542767EC, {"cskd", "rCatSkillData"}},                     //
    {0x5435D27B, {"hkm", "rHkMaterial"}},                        //
    {0x543E41DE, {"mrk", "rMarkerLayout"}},                      //
    {0x54503672, {"ahl", "rAreaHitLayout"}},                     //
    {0x5450C517, {"sdm", "rShortDemoData"}},                     //
    {0x54539AEE, {"sup", "rSupplyList"}},                        //??
    {0x54AE0DF9, {"dmlp", "rDemoListeningPoint"}},               //
    {0x54DC440A, {"msl", "rMotionSequenceList"}},                //
    {0x54DD639F, {"pvi", "rPartsVisibleInfo"}},                  //
    {0x54E2D1FF, {"rpd", "rPadData"}},                           //
    {0x550D05AA, {"sat", "rSelectAnsText"}},                     //
    {0x552E1B82, {"asl", "rAreaSetLayout"}},                     //
    {0x557ECC08, {"aef", "rAHEffect"}},                          //
    {0x55A8FB34, {"anm", "rSprAnm"}},                            //
    {0x55C30579, {"stdrd", "rSoundTalkDemoRequestData"}},        //
    {0x55EF9710, {"hlc", "rHavokLinkCollisionLayout"}},          //
    {0x55FCA0F5, {"tlil", "rTradeLimitedItemList"}},             //
    {0x562C6C5D, {"fosd", "rFieldOrnamentSetData"}},             //
    {0x5653C1B0, {"hts", "rHitSize"}},                           //
    {0x5699B3FE, {"dgp", "rDungeonPartsData"}},                  //
    {0x56CF8411, {"abe", "rAreaBarrierEdit"}},                   //
    {0x56CF93D4, {"lnv", "rAINavigationMeshList"}},              //
    {0x56D892D0, {"areaeatdat", "rAreaEatData"}},                //
    {0x56E21768, {"w01d", "rWeapon01BaseData"}},                 //
    {0x57BAE388, {"hcl", "rHavokConstraintLayout"}},             //
    {0x57D1CECD, {"dmfd", "rDemoFade"}},                         //
    {0x57DC1ED1, {"lfp", "rLensFlare"}},                         //
    {0x5802B3FF, {"ahc", "rAHCamera"}},                          //
    {0x58072136, {"sfsa", "rSeFsAse"}},                          //
    {0x5812AC77, {"gpd", "rGeyserPointData"}},                   //
    {0x583F70B0, {"rdb", "rEnemyResidentDtBase"}},               //
    {0x5842F0B0, {"pts", "rSoundPelTiedSe"}},                    //
    {0x585831AA, {"pos", "rPos"}},                               //
    {0x586995B1, {"snd", "rSoundSnd"}},                          //
    {0x58819BC8, {"sso", "rSoundSmOcclusion"}},                  //
    {0x5898749C, {"bbm", "rBioBgmMap"}},                         //
    {0x58A15856, {"mod", "rModel"}},                             //
    {0x58BC1C29, {"ext", "rGuestQuestData"}},                    //
    {0x597812F5, {"sbmd", "rSoundBgmMonsterData"}},              //
    {0x59D80140, {"ablparam", "rPlAbilityParam"}},               //
    {0x59D9C3DA, {"dtb", "rEnemyDtBase"}},                       //
    {0x5A2E573A, {"por", "rPreOcclusion"}},                      //
    {0x5A3CED86, {"rrd", "rSoundRrd"}},                          //
    {0x5A45BA9C, {"xml", "rMapLink"}},                           //
    {0x5A525C16, {"pel", "rProofEffectList"}},                   //
    {0x5A61A7C8, {"fed", "rFaceEdit"}},                          //
    {0x5A6991F2, {"slw07", "rEquipShopListW07"}},                //
    {0x5A7A72DE, {"lsnl", "rLimitedShopNpcList"}},               //
    {0x5A7BF109, {"slw14", "rEquipShopListW14"}},                //
    {0x5A7FEA62, {"ik", "rCnsIK"}},                              //
    {0x5ADC692C, {"emsizetbl", "rEmSizeCalcTblElement"}},        //
    {0x5AF4E4FE, {"mot", "rMotion"}},                            //
    {0x5B0DD78A, {"otp", "rOtTrainParam"}},                      //
    {0x5B334013, {"bap", "rBowActParamList"}},                   //
    {0x5B3C302D, {"rem", "rRem"}},                               //
    {0x5B8E6BF3, {"itp", "rItemPreData"}},                       //
    {0x5CA6DB93, {"sla00", "rEquipShopListA00"}},                //
    {0x5CC5C1E2, {"ips", "rInitPosSet"}},                        //
    {0x5CF33CD6, {"rnd", "rRiderNoteData"}},                     //
    {0x5D0455EB, {"slw03", "rEquipShopListW03"}},                //
    {0x5D163510, {"slw10", "rEquipShopListW10"}},                //
    {0x5D2E4199, {"ane", "rAcNyanterEquip"}},                    //
    {0x5D4F7DBF, {"sosd", "rSoundOrnamentSeData"}},              //
    {0x5E1A909B, {"ged", "rGeneEdit"}},                          //
    {0x5E3DC9F3, {"lge", "rGridEnvLight"}},                      //
    {0x5E4C723C, {"nls", "rNulls"}},                             //
    {0x5E5B44C8, {"sls", "rShopListSale"}},                      //
    {0x5E640ACC, {"itm", "rItemAngle"}},                         //
    {0x5EA6AA68, {"oxpb", "rOtQuestExpBias"}},                   //
    {0x5EA7A3E9, {"sky", "rSky"}},                               //
    {0x5EF1FB52, {"lcm", "rCameraList"}},                        //
    {0x5F12110E, {"wpt", "rWeskerParamTable"}},                  //
    {0x5F36B659, {"way", "rAIWayPoint"}},                        //
    {0x5F6A387F, {"cms", "rCommonScript"}},                      //
    {0x5F84F7C4, {"mem.wmv", "rMovie"}},                         //
    {0x5F88B715, {"epd", "rEditPatternData"}},                   //
    {0x5F975A38, {"epd", "rEggPartsData"}},                      //
    {0x5FB399F4, {"bssq", "rBioSoundSequenceSe"}},               //
    {0x601E64CD, {"szs", "rSoundZoneSwitch"}},                   //
    {0x60524FBB, {"shw", "rShape"}},                             //
    {0x60604467, {"btatef", "rBattleAtkEffect"}},                //
    {0x60869A71, {"evt", "rEventActorTbl"}},                     //
    {0x60BB6A09, {"hed", "rHumanEdit"}},                         //
    {0x60DD1B16, {"lsp", "rLayoutSpr"}},                         //
    {0x60EEAE69, {"sfd", "rSoundFootstepData"}},                 //
    {0x61382649, {"fisd", "rFieldIngredientSetData"}},           //
    {0x617B0C47, {"dspw", "rSoundSourceDSPADPCM"}},              //
    {0x6186627D, {"wep", "rWeatherEffectParam"}},                //
    {0x619D23DF, {"shp", "rShopList"}},                          //
    {0x61C203A4, {"fms", "rFueMusicScData"}},                    //
    {0x61CF79AF, {"qsg", "rQuestGroup"}},                        //
    {0x62220853, {"vfp", "rVillageFirstPos"}},                   // MovePos?
    {0x622FA3C9, {"ewy", "rAIWayPointExpand"}},                  //
    {0x62440501, {"lmd", "rLayoutMessage"}},                     //
    {0x626BC4D6, {"mrs", "rMonNyanRewardSecret"}},               //
    {0x628DFB41, {"gr2s", "rGrass2Setting"}},                    //
    {0x62A68441, {"ttb", "rThinkTable"}},                        //
    {0x630ED7BB, {"nan", "rEnemyNandoData"}},                    //
    {0x63747AA7, {"rpi", "rRomPawnInfo"}},                       //
    {0x63B524A7, {"ltg", "rLockOnTarget"}},                      //
    {0x63F2D70D, {"apd", "rArmorProcessData"}},                  //
    {0x63F62424, {"ipt", "rItemPreTypeData"}},                   //
    {0x6409628D, {"stdsrd", "rSoundTalkDemoRequestSeData"}},     //
    {0x64387FF1, {"qlv", "rEquipLvUp"}},                         //
    {0x64844D84, {"mri", "rMonNyanReward"}},                     //
    {0x64A7CFA9, {"fng", "rFinger"}},                            //
    {0x64BBEAA6, {"d.l", "rFieldDestMarkerList"}},               //
    {0x64BFE66D, {"bta", "rBtnAct"}},                            //
    {0x652071B0, {"rsl", "rScrList"}},                           //
    {0x65B275E5, {"sce", "rScenario"}},                          //
    {0x65E22C55, {"w01m", "rWeapon01MsgData"}},                  //
    {0x6622AB2D, {"smkd", "rSoundMonsterKizunaData"}},           //
    {0x663D73B2, {"tdmd", "rTalkDemoData"}},                     //
    {0x66B45610, {"fsm", "rAIFSM"}},                             //
    {0x66C89ED2, {"plpartsdisp", "rPlayerPartsDisp"}},           //
    {0x66D7CD8A, {"mai", "rMonNyanAdventItem"}},                 //
    {0x67015AE5, {"wb", "rWaterBoundary"}},                      //
    {0x67195A2E, {"mca", "rLtSoundSourceADPCM"}},                //
    {0x671F21DA, {"stp", "rStartPos"}},                          //
    {0x677EDC52, {"dptrm", "rDungeonPassTreasureTbl"}},          //
    {0x681FA774, {"owp", "rOtWeaponData"}},                      //
    {0x682B1925, {"lfx", "rLtShader"}},                          //
    {0x68CD2933, {"ses", "rMHSoundEmitter"}},                    //
    {0x68E301A1, {"uce2", "rUpCutElevator"}},                    //
    {0x694348E3, {"h2d", "rHit2D"}},                             //
    {0x699AC631, {"npcMdl", "rNpcBaseData_Mdl"}},                //
    {0x69A5C538, {"dwm", "rDeformWeightMap"}},                   //
    {0x69B525DF, {"otpt", "rOtPointTable"}},                     //
    {0x69C413C7, {"w13d", "rWeapon13LevelData"}},                //
    {0x6A1670F0, {"fld", "rFloorLvData"}},                       //
    {0x6A28E7C5, {"gstd", "rGatherSetTableData"}},               //
    {0x6A5CDD23, {"occ", "rOccluder"}},                          //
    {0x6A6AFD9E, {"fld", "rFieldData"}},                         //
    {0x6A9197ED, {"sst", "rSoundStreamStructure"}},              //
    {0x6ABA51B0, {"wofb", "rWeaponOfsForBodyNative"}},           //
    {0x6B41A2F9, {"scd", "rStageCameraData"}},                   //
    {0x6B6D2BB6, {"w02m", "rWeapon02MsgData"}},                  //
    {0x6BB4ED5E, {"ich", "rLch"}},                               //
    {0x6BEEFB2A, {"nstera", "rNestEggReviewA"}},                 //
    {0x6C1D2073, {"srq", "rSoundRequest"}},                      //
    {0x6C3B4904, {"hlc", "rHavokLinkCollisionLayout"}},          //
    {0x6C980E22, {"des", "rDungeonEnemySet"}},                   //
    {0x6CACB310, {"mcd", "rMiniChatData"}},                      //
    {0x6CC8C051, {"acd", "rAccessoryData"}},                     //
    {0x6D0115ED, {"prt", "rAIPriorityThink"}},                   //
    {0x6D31C3FA, {"w11d", "rWeapon11LevelData"}},                //
    {0x6D3AB570, {"w09d", "rWeapon09BaseData"}},                 //
    {0x6D5AAA39, {"npcBd", "rNpcBaseData"}},                     //
    {0x6D5AE854, {"efl", "rEffectList"}},                        //
    {0x6D81CFDD, {"oar", "rOtArmorData"}},                       //
    {0x6D964D19, {"kca", "rKitchenListSkillAlcohol"}},           //
    {0x6D96778B, {"cli", "rCameraListInfo"}},                    //
    {0x6DB9FA5F, {"cmc", "rCmcMsgConfig"}},                      //
    {0x6E171A6E, {"mss", "rMHSoundSequence"}},                   //
    {0x6E45FABB, {"atk", "rAttackParam"}},                       //
    {0x6E5FB1CC, {"std", "rStoryData"}},                         //
    {0x6E6A25E0, {"mrd", "rMonsterRaceData"}},                   //
    {0x6E765E35, {"insectessenceskill", "rInsectEssenceSkill"}}, //
    {0x6E972F76, {"kad", "rKireajiData"}},                       //
    {0x6EC8125C, {"raps", "rRapidshotData"}},                    //
    {0x6EE70EFF, {"pcf", "rAIPathConsecutive"}},                 //
    {0x6F0CD860, {"brsb", "rBattleResultBonus"}},                //
    {0x6F27254C, {"w06m", "rWeapon06MsgData"}},                  //
    {0x6F302481, {"plw", "rPlWeight"}},                          //
    {0x6F56B442, {"opl", "rOtParamLot"}},                        //
    {0x6F62D575, {"tde", "rTexDetailEdit"}},                     //
    {0x6F6F2BAD, {"w02d", "rWeapon02BaseData"}},                 //
    {0x6FCC7AD4, {"pec", "rProofEffectColorControl"}},           //
    {0x6FE1EA15, {"spl", "rSoundPhysicsList"}},                  //
    {0x7039F76F, {"oxpv", "rOtQuestExpValue"}},                  //
    {0x705A17BE, {"emg", "rEnemyGroundMove"}},                   //
    {0x7063542F, {"bdy", "rBuddyPlanData"}},                     //
    {0x70709F3F, {"ctl", "rCoinTradeList"}},                     //
    {0x708E0028, {"lanl", "rLayoutAnimeList"}},                  //
    {0x70BB64BA, {"hde", "rHitDataEnemy"}},                      //
    {0x70C56D5E, {"skmt", "rSwkbdMessageTable"}},                //
    {0x70FDFB5B, {"ext", "rEggTableData"}},                      //
    {0x710688E2, {"squs", "rSquatshotData"}},                    //
    {0x711BD05E, {"osf", "rObjSet"}},                            //
    {0x714EC77C, {"bes2", "rEventScript"}},                      //
    {0x7190B007, {"nstip", "rNestInitProb"}},                    //
    {0x71D6A0D4, {"osf", "rObjSet"}},                            //
    {0x720EF393, {"fmd", "rFieldMotionData"}},                   //
    {0x724DF879, {"xsew", "rSoundSourceMSADPCM"}},               //
    {0x72821C38, {"stm", "rPlStamina"}},                         //
    {0x7284DAF5, {"list", "rQuestList"}},                        //
    {0x72993816, {"w11m", "rWeapon11MsgData"}},                  //
    {0x72E7AA90, {"nsterb", "rNestEggReviewB"}},                 //
    {0x7370D1FC, {"fslm", "rMHFSMList"}},                        //
    {0x737234F5, {"acd", "rArmorColorData"}},                    //
    {0x73850D05, {"arc", "rArchive"}},                           //
    {0x73B73919, {"w14d", "rWeapon14BaseData"}},                 //
    {0x745DAB77, {"pvb", "rPlanetVibration"}},                   //
    {0x7470D7E9, {"tex2", "r2Texture"}},                         //
    {0x7494F854, {"hcl", "rHavokConstraintLayout"}},             //
    {0x74A22486, {"spval", "rSupportGaugeValue"}},               //
    {0x74A4D35F, {"dge", "rDungeonEventData"}},                  //
    {0x74B583F5, {"fpgl", "rPoogieList"}},                       //
    {0x74EA83B0, {"fsesl", "rFieldSoundEffectSeList"}},          //
    {0x7501E3C1, {"ard", "rArmorData"}},                         //
    {0x751ACAA0, {"eepd", "rEncntEnemyParty"}},                  //
    {0x751AFF22, {"tams", "rTameshotData"}},                     //
    {0x7522DD13, {"acrd", "rArmorCreateData"}},                  //
    {0x7534679E, {"mea", "rModelEasyAnime"}},                    //
    {0x753B3A8C, {"hvd", "rHavokVehicleData"}},                  //
    {0x754B82B4, {"ahs", "rAreaHitShape"}},                      //
    {0x755BDD19, {"lpeq", "rPlanetEQ"}},                         //
    {0x758B2EB7, {"cef", "rCommonEffect"}},                      //
    {0x75967AD6, {"dsc", "rDynamicSbc"}},                        //
    {0x759C9F51, {"bes", "rBattleEnemySet"}},                    //
    {0x75D21272, {"cdp", "rSoundCdp"}},                          //
    {0x75E48071, {"glt", "rGeneLotting"}},                       //
    {0x766F00AC, {"bnt", "rBattleNpcTbl"}},                      //
    {0x76820D81, {"lmt", "rMotionList"}},                        //
    {0x76A1D9A2, {"isp", "rInsectParam"}},                       //
    {0x76DE35F6, {"rpn", "rRomPawnName"}},                       //
    {0x77044C04, {"vibpl", "rVibrationPlayerList"}},             //
    {0x77A69893, {"bnu", "rBattleNavirouUnique"}},               //
    {0x77D70343, {"atr", "rGuestEffectiveAttr"}},                //
    {0x7808EA10, {"rtex", "rRenderTargetTexture"}},              //
    {0x78143FEE, {"w03d", "rWeapon03BaseData"}},                 //
    {0x7817FFA5, {"fbik_human", "rFullbodyIKHuman"}},            //
    {0x78956684, {"fppgr", "rFldPlParam_GR"}},                   //
    {0x7896B60A, {"asd", "rArmorSeriesData"}},                   //
    {0x790203B0, {"angryprm", "rAngryParam"}},                   //
    {0x79C47B59, {"mca", "rSoundSourceADPCM"}},                  //
    {0x79FF11C9, {"olos", "rOtLotOwnSupport"}},                  //
    {0x7A038F4C, {"rev_win", "rReverb"}},                        //
    {0x7A23F10F, {"mdl", "rMedalDataList"}},                     //
    {0x7A395CB7, {"sab", "rOtSupportActionBase"}},               //
    {0x7A41A133, {"w08d", "rWeapon08BaseData"}},                 //
    {0x7AA81CAB, {"eap", "rAIEnemyActionParameter"}},            //
    {0x7AAD377E, {"w08m", "rWeapon08MsgData"}},                  //
    {0x7AF36DC1, {"nstmsg", "rNestMessage"}},                    //
    {0x7B54B600, {"atd", "rActivityData"}},                      //
    {0x7B572569, {"olsk", "rOtLotOwnSkill"}},                    //
    {0x7BEA3086, {"cfl", "rLtSoundCategoryFilter"}},             //
    {0x7BEC319A, {"sps", "rSoundPhysicsSoftBody"}},              //
    {0x7C067B17, {"wpd", "rWeaponData"}},                        //
    {0x7C163FF5, {"w12m", "rWeapon12MsgData"}},                  //
    {0x7C409434, {"bnf", "rBattleNaviFsm"}},                     //
    {0x7C4883A8, {"itm", "rItemData"}},                          //
    {0x7C559D35, {"sddsc", "rSoundDramaDemoSeControl"}},         //
    {0x7C5E6060, {"osa", "rOtSpecialAction"}},                   //
    {0x7CD0E77E, {"mpm", "rMonsterPartsManager"}},               //
    {0x7D025838, {"mla", "rMonNyanLotAdvent"}},                  //
    {0x7D1530C2, {"sngw", "rSoundSourceMusic"}},                 //
    {0x7DA64808, {"qmk", "rQuestMarker"}},                       //
    {0x7DAAFFDC, {"samd", "rSoundArchiveData"}},                 //
    {0x7DD2AFBE, {"mqsd", "rMainQuestData"}},                    //
    {0x7DD2F109, {"ssed", "rSubStEpiData"}},                     //
    {0x7E1C8D43, {"pcs", "rAIPresetStudy"}},                     //
    {0x7E33A16C, {"spc", "rSoundPackage"}},                      //
    {0x7E4152FF, {"stg", "rAISensorTargetGroup"}},               //
    {0x7EF03563, {"cfid", "rChestItemTableData"}},               //
    {0x7F2E2EE0, {"areaacttbl", "rAreaActTblData"}},             //
    {0x7F416CE5, {"mcn", "rMonNyanCirclePattern"}},              //
    {0xC230157D, {"at3", "rSoundSourceMusic"}},                  //
};

static decltype(extensions) extensionsPS3{
    {0x7A038F4C, {"rev_ps3", "rReverb"}},       //
    {0x4857FD94, {"rev_ps3", "rReverb"}},       //
    {0x232E228C, {"rev_ps3", "rReverb"}},       //
    {0x3821B94D, {"at3", "rSoundSourceMusic"}}, //
    {0x7D1530C2, {"at3", "rSoundSourceMusic"}}, //
};

static decltype(extensions) extensionsCAFE{
    {0x232E228C, {"revr_cafe", "rSoundReverb"}}, //
};

static decltype(extensions) extensionsN3DS{
    {0x15302EF4, {"lyt", "rLayout"}},           // mh4, mhs
    {0x232E228C, {"revr_ctr", "rSoundReverb"}}, // mhg
    {0x5204D557, {"slt", "rShopList"}},         // mhg
    {0x3FB52996, {"mix", "rItemMix"}},          // mhs
    {0x2B40AE8F, {"equr", "rSoundEQ"}},         // mhs
};

static decltype(extensions) extensionsNSW{
    {0x232E228C, {"revr", "rSoundReverb"}},        // mhg
    {0x79C47B59, {"adpcm", "rSoundSourceADPCM"}},  // mhg
    {0x1BCC4966, {"srqr", "rSoundRequest"}},       // mhg
    {0x167DBBFF, {"stqr", "rSoundStreamRequest"}}, // mhg
};

static const std::map<Platform, decltype(extensions) *> extensionSlots{
    {Platform::Auto, &extensions},     //
    {Platform::WinPC, &extensions},    //
    {Platform::PS3, &extensionsPS3},   //
    {Platform::CAFE, &extensionsCAFE}, //
    {Platform::N3DS, &extensionsN3DS}, //
    {Platform::NSW, &extensionsNSW},   //
};

auto GetPair(uint32 hash, Platform platform) {
  auto Find = [hash](auto &reg) {
    auto found = reg.find(hash);

    if (!es::IsEnd(reg, found)) {
      return &(*found);
    }

    return (const decltype(extensions)::value_type *)(nullptr);
  };

  if (platform != Platform::Auto) {
    auto retVal = Find(*extensionSlots.at(platform));

    if (retVal) {
      return retVal;
    }
  }

  return Find(extensions);
}

static const std::map<uint32, es::string_view> classNames{
    {0x04657ED7, "sprMapHead"},                                //
    {0x0474F424, "rAreaSetLayout::LayoutInfo"},                //
    {0x26C9176E, "rAreaSetLayout::LayoutInfo"},                //
    {0x0B3A7230, "sprMap"},                                    //
    {0x1270F9FE, "nodeData"},                                  //
    {0x14E137DD, "rScrList::ScrListTable"},                    //
    {0x1838FCE7, "rReverb::cReverbData"},                      //
    {0x1E7A21E4, "rSoundSeg::LayoutInfo"},                     //
    {0x354344D5, "nodeHead"},                                  //
    {0x35DDDCA1, "nodeLink"},                                  //
    {0x3AC30C7D, "rSceneBoxLayout::LayoutInfo"},               //
    {0x5BC3A2CC, "rHavokLinkCollisionLayout::LayoutInfo"},     //
    {0x4CABC92A, "rHavokLinkCollisionLayout::LayoutInfo"},     //
    {0x60420840, "rHavokConstraintLayout::LayoutInfo"},        //
    {0x4194CF9B, "rHavokConstraintLayout::LayoutInfo"},        //
    {0x63A8DDBA, "rAreaHitLayout::LayoutInfo"},                //
    {0x60F7B3FC, "rAreaHitLayout::LayoutInfo"},                //
    {0x6EBD2A98, "rScrList"},                                  //
    {0x4CFD5420, "rTexDetailEdit::DetailParam"},               //
    {0x71134D41, "MtHeapArray"},                               //
    {0x25B52337, "rEffectProvider::EffectParam"},              //
    {0x1E66D162, "rEffectProvider::EffectSet"},                //
    {0x3AEBFBEA, "rEffectProvider::EffectList"},               //
    {0x3251DD5F, "rEffectProvider::INFO_EFFECT"},              //
    {0x6FD8D8C7, "rEffectProvider::INFO_E2D"},                 //
    {0x602EBA26, "rEffectProvider::EffectMotSyncParam"},       //
    {0x0C3494F4, "rEffectProvider::EffectLODParam"},           //
    {0x4B03CE4C, "rEffectProvider::INFO_BASE"},                // ::INFO_PROV ??
    {0x0BC4A3C3, "rHmGenerator::HmGeneratorInfo"},             //
    {0x50DAA58B, "rHmGenerator::EquipWeaponInfo"},             //
    {0x5E9A0BCA, "rCnsIK::JointInfo"},                         //
    {0x1AD90F64, "rPlanetEQ::cEQData"},                        //
    {0x11A59288, "rReverb::cSoundReverbData"},                 //
    {0x7821C482, "rReverb::cReverbParam"},                     //
    {0x5CECEBB6, "rSoundMotionSe::cSoundMotionSeData"},        //
    {0x5EB6FE4E, "rSoundSequenceSe::SequenceSe"},              //
    {0x7A99C43F, "rSoundSequenceSe::SequenceSe::Command"},     //
    {0x5F735F08, "rSoundSubMixer::Param"},                     //
    {0x45674FD1, "cAIPositionEvaluationFuncList"},             //
    {0x072DF97A, "cAIPositionEvaluationFunc"},                 //
    {0x24C99855, "cPositionFilterDoughnutAroundPosTerritory"}, //
    {0x3EBF164E, "rThinkPlanAI::cItemPlanInfo"},               //
    {0x74987697, "rThinkPlanAI::cU32"},                        //
    {0x378EECF0, "rAreaBarrierEdit::Work"},                    //
    {0x50835382, "rInitPosSet::InitPosSetInfo"},               //
    {0x6B967852, "rScrList::ScrListTable"},                    //
    {0x18868106, "cAIFSMCluster"},                             //
    {0x5035D8FD, "cAIFSMNode"},                                //
    {0x63E6A949, "cAIFSMLink"},                                //
    {0x609DB540, "cAIFSMProcessContainer"},                    //
    {0x1836BA1E, "cPositionEvaluationDangerRange"},            //
    {0x5C539DD4, "cPositionEvaluationDangerMessageRange"},     //
    {0x53E26270, "cPositionEvaluationDistToPos"},              //
    {0x57DABC2A, "cTargetVisibilityHigh"},                     //
    {0x7EED1022, "cTargetVisibilityLow"},                      //
    {0x2A6D2CAD, "cPositionEvaluationTargetRange"},            //
    {0x62C06191, "cPositionEvaluationTargetGoodDistans"},      //
    {0x4F61724C, "rFieldSet::FieldSetWork"},                   //
    {0x15DE6AC5, "cPositionEvaluationTargetVisibility"},       //
    {0x16021C59, "cPositionFilterDoughnutAroundPos"},          //
    {0x01662DD0, "cPositionEvaluationPlayerVisibility"},       //
    {0x66BB432F, "rFieldSet::E2DSetWork"},                     //
    {0x0704302E, "cAIPlanFilterTargetExist"},                  //
    {0x026A2DE7, "cAIPlanMoveAction"},                         //
    {0x29B16567, "cAIPlanFilterMessagePickUp"},                //
    {0x3D1A2BA6, "cAIPlanFilterPartyLevel"},                   //
    {0x0B9D355C, "cAIPlanFilterDistans"},                      //
    {0x6DF787F5, "cAIPlanEvaluateDistans"},                    //
    {0x765A8CDB, "cAIPlanMoveSendMessage"},                    //
    {0x5DEBFFAF, "cAIPlanMovePickMessage"},                    //
    {0x546EF519, "cAIPlanMoveDamage"},                         //
    {0x22AB6E3F, "cAIPlanMoveTrace"},                 // cAIPlanMoveAvoidLOF ??
    {0x7CDD207B, "cAIPlanMoveTime"},                  //
    {0x2ED25315, "cAIPlanMoveDistance"},              //
    {0x5BF97E48, "cAIPlanMoveAk64Action"},            //
    {0x15444559, "cAIPlanMoveTargetExist"},           //
    {0x09D48FD0, "cAIPlanEvaluateAttackRemain"},      //...DamageRemain ??
    {0x4E2D5670, "cAIPlanMoveSendCommand"},           //
    {0x41184FCC, "cAIPlanEvaluateDamageRemain"},      //...AttackRemain ??
    {0x2605B481, "cAIPlanFilterFSM"},                 //
    {0x3E661257, "cAIPlanFilterCharIsXXX"},           //
    {0x384E3085, "cAIPlanMoveShot"},                  //
    {0x32097A12, "cAIPlanMoveWaitPos"},               //
    {0x6564DEC0, "cAIPlanEvaluateMessagePickUp"},     //
    {0x4FAFA8D1, "cAIPlanMoveHmAction"},              //
    {0x66D9AF9F, "cAIPlanFilterTargetLifeMaxOverCk"}, //
    {0x3A326398, "cAIPlanEvaluateAIParam"},           //
    {0x2E71A0D2, "cAIPlanReturnX"},                   //
    {0x617C113F, "cAIPlanFilterPowerGroup"},          //
    {0x1A7ACA56, "cAIPlanMoveHm40VoiceReq"},          //
    {0x56F3846A, "cAIPlanFilterGK"},                  //
    {0x00A5315B, "cAIPlanFilterWeapon"},              //
    {0x35A0E27E, "cAIPlanFilterTeamCommand"},         //
    {0x2988A5D1, "cAIPlanMoveSetCursorMode"},         //
    {0x7922BB3F, "cAIPlanFilterUniqueIdRand"},        //
    {0x364E93AB, "cAIPlanFilterDir"},                 //
    {0x5C58D131, "cAIPlanFilterAIParam"},             //
    {0x59295D08, "cAIPlanFilterArea"},                //
    {0x7E1DA162, "cAIPlanMoveActionFreeJump"},        //
    {0x36F10E99, "cAIPlanFilterHm40Type"},            //
    {0x308E2D6E, "cAIPlanFilterCharStatus"},          //
    {0x6906B3E1, "cAIPlanFilterTracePos"},            //
    {0x7DF274C1, "cAIPlanEvaluateCursorThrough"},     //
    {0x54D0B905, "cAIPlanFilterDevelop"},             //
    {0x4A5FCDB3, "cFSMOrder::FlagParameter"},         //
    {0x5AAE7F38, "cFSMOrder::PositionParameter"},     //
    {0x785E6622, "rAIConditionTree"},                 //
    {0x1C328079, "rAIConditionTreeNode"},             //
    {0x56BB1759, "rAIConditionTree::TreeInfo"},       //
    {0x3D9510A8, "rAIConditionTree::VariableNode"},   //
    {0x3E1F9629, "rAIConditionTree::VariableNode::VariableInfo"},   //
    {0x2C29825D, "cAIDEnum"},                                       //
    {0x3280309B, "rAIConditionTree::ConstS32Node"},                 //
    {0x788FCAED, "cFSMCharacterBase::MotionParameter"},             //
    {0x57EEA524, "cFSMOrder::BoolParameter"},                       //
    {0x5907152A, "cFSMFighter::AIMovePositionParameter"},           //
    {0x4A2C530F, "cFSMOrder::TimerParameter"},                      //
    {0x01464B23, "cFSMOrder::AngleParameter"},                      //
    {0x79F101EC, "cFSMCharacterBase::PosAngleFlagParameter"},       //
    {0x6D64388A, "cFSMOrder::SchedulerParameter"},                  //
    {0x47EF9888, "cFSMOrder::ReqEventExecParameter"},               //
    {0x53EF5D93, "cFSMCharacterBase::ScaleSetParameter"},           //
    {0x305FDEB3, "cFSMOrder::S32Parameter"},                        //
    {0x4609617D, "cFSMCharacterBase::EffectSetParameter"},          //
    {0x4433B9D5, "cFSMCharacterBase::AttackPlayerKeepParameter"},   //
    {0x24E4445F, "cFSMOrder::F32Parameter"},                        //
    {0x53C56047, "cFSMHM::ActionSetParameter"},                     //
    {0x45387FF1, "cFSMCharacterBase::TargetSetParameter"},          //
    {0x39C1CAE5, "cFSMCharacterBase::ShotSetParameter"},            //
    {0x076683F2, "rAIConditionTree::ConstF32Node"},                 //
    {0x14A4355A, "cFSMOrder::SchedulerCloseParameter"},             //
    {0x095A4DBF, "cFSMOrder::SubMixerParameter"},                   //
    {0x4F530D8B, "cFSMFighter::AIPlayerPursuitParameter"},          //
    {0x3651D96A, "cFSMOrder::ShellParameter"},                      //
    {0x0ADFDD5A, "cFSMCharacterBase::MoveScrollActiveParameter"},   //
    {0x29AB48DA, "cFSMVS::ActionSetParameter"},                     //
    {0x0B7AA143, "cFSMCharacterBase::PartsDispSetParameter"},       //
    {0x0B92FDF1, "cFSMCharacterBase::ShotReqSetParameter"},         //
    {0x0D32E37E, "cFSMFighter::AIDataPostUpParameter"},             //
    {0x0F389C6D, "cFSMOM::ScrCollisionSetParameter"},               //
    {0x0FEFA610, "cFSMOM::TrainHitSetParameter"},                   //
    {0x118B4EBB, "cFSMFighter::AITraceMoveParameter"},              //
    {0x146FBE15, "cFSMOrder::BaseRespawnPointSetParameter"},        //
    {0x1BB1BAB0, "cFSMOrder::CameraMotionSetParameter"},            //
    {0x1EBF971C, "cFSMCharacterBase::CharListParentOfsParameter"},  //
    {0x24325FF6, "cFSMCharacterBase::ShotTargetSetZParameter"},     //
    {0x2CA71A90, "cFSMOrder::AreaHitSetParameter"},                 //
    {0x2D6D5145, "cFSMArea::UvScrollParameter"},                    //
    {0x2F0741B8, "cFSMVS::CannonShotSetParameter"},                 //
    {0x30471086, "cFSMVS::CannonTargetSetParameter"},               //
    {0x310FB3B0, "cFSMOrder::CallRequestSetParameter"},             //
    {0x32D8555C, "cFSMArea::WarPointSetParameter"},                 //
    {0x38B20D54, "cFSMCharacterBase::UserWorkSetParameter"},        //
    {0x3BB265E4, "cFSMCharacterBase::ActSetParameter"},             //
    {0x446BD5E4, "cFSMCharacterBase::ShotTargetSetParameter"},      //
    {0x4AD6AEEC, "cFSMCharacterBase::WepMotionParameter"},          //
    {0x51C75708, "cFSMFighter::AIPlayerSupportParameter"},          //
    {0x5302004F, "cFSMOM::BoatHitSetParameter"},                    //
    {0x53AFD2FB, "cFSMOrder::StatusFlagParameter"},                 //
    {0x56600843, "cFSMCharacterBase::InitPosSetParameter"},         //
    {0x5943F174, "cFSMCharacterBase::CharListParentParameter"},     //
    {0x59561754, "cFSMCharacterBase::CalcPartsDamageSetParameter"}, //
    {0x5F0AD3BD, "cFSMVS::AreaMoveAttackParameter"},                //
    {0x64092131, "cFSMCharacterBase::RequestSeParameter"},          //
    {0x6472931D, "cFSMOrder::CameraVibSetParameter"},               //
    {0x64C7F78E, "cFSMCharacterBase::ObjAdjustEntryParameter"},     //
    {0x6D99A3BF, "cFSMOrder::CounterParameter"},                    //
    {0x6DC84078, "cFSMArea::MissionCycleSetParameter"},             //
    {0x755AF279, "cFSMHM::MovePositionParameter"},                  //
    {0x75ED317D, "cFSMCharacterBase::EffectSynchroSetParameter"},   //
    {0x77C63BD5, "cFSMVS::MovePositionParameter"},                  //
    {0x7991B00F, "cFSMCharacterBase::CompulsionMoveParameter"},     //
    {0x79E1ED4B, "cFSMOrder::RandomGetParameter"},                  //
    {0x7D358A0C, "cFSMCharacterBase::BlendMotionParameter"},        //
    {0x7D73040B, "cFSMCharacterBase::EffectSetResParameter"},       //
    {0x7FEE673F, "cFSMOM::BoatRideModeSetParameter"},               //
    {0x7FFC4F5B, "cFSMFighter::CharParamSetParameter"},             //
    {0x4D9FF01D, "cFSMCharacterBase::TransLucenceParameter"},       //
    {0x0C1599A1, "cFSMHM::DamageActionSetParameter"},               //
    {0x044C045B, "cFSMCharacterBase::CheckPosSetParameter"},        //
    {0x4B3EABE1, "cFSMFighter::AIPermitSetParameter"},              //
    {0x170904BF, "rEffectProvider::E2DGroup"},                      //
    {0x4858746A, "rEffectProvider::E2DMaterial"},                   //
    {0x7D995D73, "AIPlanFilterTargetDir"},                          //
    {0x0F7A1BA5, "cAIPlanFilterIsAction"},                          //

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

static const std::map<es::string_view, const MtExtensions *> invertedExtensions{
    {"ace_attorney_dual_destinies", &extPWAADD},     //
    {"ace_attorney_spirit_of_justice", &extPWAASOJ}, //
    {"biohazzard_0", &extRE0},                       //
    {"biohazzard_5", &extRE5},                       //
    {"biohazzard_6", &extRE6},                       //
    {"dd", &extDD},                                  //
    {"dead_rising", &extDR},                         //
    {"dr", &extDR},                                  //
    {"dragons_dogma", &extDD},                       //
    {"lost_planet_2", &extLP2},                      //
    {"lost_planet", &extLP},                         //
    {"lp", &extLP},                                  //
    {"lp2", &extLP2},                                //
    {"mh3", &extMH3},                                //
    {"mh4", &extMH4},                                //
    {"mhg", &extMHG},                                //
    {"mhs", &extMHS},                                //
    {"mhx", &extMHG},                                //
    {"mhxx", &extMHG},                               //
    {"monster_hunter_3", &extMH3},                   //
    {"monster_hunter_4", &extMH4},                   //
    {"monster_hunter_cross", &extMHG},               //
    {"monster_hunter_double_cross", &extMHG},        //
    {"monster_hunter_generations", &extMHG},         //
    {"monster_hunter_stories", &extMHS},             //
    {"monster_hunter_x", &extMHG},                   //
    {"monster_hunter_xx", &extMHG},                  //
    {"pwaadd", &extPWAADD},                          //
    {"pwaasoj", &extPWAASOJ},                        //
    {"re0", &extRE0},                                //
    {"re5", &extRE5},                                //
    {"re6", &extRE6},                                //
    {"resident_evil_0", &extRE0},                    //
    {"resident_evil_5", &extRE5},                    //
    {"resident_evil_6", &extRE6},                    //
};

namespace revil {
void GetTitles(TitleCallback cb) {
  for (auto &p : invertedExtensions) {
    cb(p.first);
  }
}

es::string_view GetExtension(uint32 hash, Platform platform) {
  auto pair = GetPair(hash, platform);

  if (pair) {
    return pair->second.first;
  }

  return {};
}

es::string_view GetClassName(uint32 hash, Platform platform) {
  auto pair = GetPair(hash, platform);

  if (pair) {
    return pair->second.second;
  }

  auto found = classNames.find(hash);

  if (!es::IsEnd(classNames, found)) {
    return found->second;
  }

  return {};
}

uint32 GetHash(es::string_view extension, es::string_view title,
               Platform platform) {
  auto found = invertedExtensions.find(title);

  if (es::IsEnd(invertedExtensions, found)) {
    throw std::runtime_error("Coundn't find title.");
  }

  auto foundSec = found->second;
  uint32 retVal = foundSec->GetHash(extension, platform);

  if (retVal) {
    return retVal;
  }

  // for backward compatibility, some extensions might have numerical (hashed)
  // extension (not found in main registry) if the extension has been added
  // later, just find it by hash and verify it in inverted registry
  auto cvted = strtoul(extension.data(), nullptr, 10);

  if (cvted < 0x10000) {
    return 0;
  }

  auto extTranslated = revil::GetExtension(cvted, platform);

  if (extTranslated.empty()) {
    return 0;
  }

  return foundSec->GetHash(extTranslated, platform);
}
}; // namespace revil
