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

using pair_type = std::pair<es::string_view, es::string_view>;

static const std::map<uint32, pair_type> extensions{
    {0x0026E7FF, {"ccl", "rChainCol"}},                         //
    {0x0086B80F, {"plexp", "rPlExp"}},                          //
    {0x00FDA99B, {"ntr", "rArmorTableNpc"}},                    //
    {0x01F28535, {"bds2", "rDoorScene"}},                       //
    {0x02358E1A, {"spkg", "rShaderPackage"}},                   //
    {0x02373BA7, {"spn", "rStagePlaceName"}},                   //
    {0x0253F147, {"hit", "rHit"}},                              //
    {0x02833703, {"efs", "rEffectStrip"}},                      //
    {0x0315E81F, {"sds", "rSoundDirectionalSet"}},              //
    {0x039D71F2, {"rvt", "rSoundReverbTable"}},                 //
    {0x03FAE282, {"efa", "rEffectAnimation"}},                  //
    {0x0437BCF2, {"grw", "rGrassWind"}},                        //
    {0x044BB32E, {"ubcell", "rUBCell"}},                        //
    {0x04B4BE62, {"tmd", "rTalkMotion"}},                       //
    {0x04E9EBE9, {"cspp", "rCutScrParam"}},                     //
    {0x0525AEE2, {"wfp", "rWeatherFogParam"}},                  //
    {0x0532063F, {"ast", "rSoundAst"}},                         //
    {0x053C8864, {"mps", "rPartsSelector"}},                    //
    {0x059BC928, {"loc2", "rArrange"}},                         //
    {0x05A36D08, {"qif", "rQuestInfinity"}},                    //
    {0x069A1911, {"olp", "rOutlineParamList"}},                 //
    {0x07437CCE, {"ase", "rSoundAttributeSe"}},                 //
    {0x079B5F3E, {"pci", "rAIPresetCharaInfo"}},                //
    {0x07D3088E, {"mdi.xml", "rModelInfo"}},                    //
    {0x07D5909F, {"stq", "rSoundStreamRequest"}},               //
    {0x07F768AF, {"gii", "rGUIIconInfo"}},                      //
    {0x08EF36C1, {"modlayout.xml", "rModelLayout"}},            //
    {0x094973CF, {"scs", "rSoundCurveSet"}},                    //
    {0x09D775FC, {"efcc", "rEffectColorCorrect"}},              //
    {0x0A74682F, {"rnp", "rRomNoraPawn"}},                      //
    {0x0B0B8495, {"smadd.xml", "rSMAdd"}},                      //
    {0x0B2FACE7, {"bscn", "rSceneInfo"}},                       //
    {0x0BCF07BD, {"pcos", "rPlayerCostume"}},                   //
    {0x0C4FCAE4, {"PlDefendParam", "rPlDefendParam"}},          //
    {0x0D06BE6B, {"tmn", "rTargetMotion"}},                     //
    {0x0D3BE7B5, {"sbl", "rSceneBoxLayout"}},                   //
    {0x0DADAB62, {"oba", "rObjAdj"}},                           //
    {0x0DE13237, {"pai", "rThinkPlanAI"}},                      //
    {0x0E16DFBA, {"cit", "rItemCurseCnv"}},                     //
    {0x0ECD7DF4, {"scs", "rSoundCurveSet"}},                    //
    {0x0F9F3E69, {"pmtt", "rPlayerMotionTimeTable"}},           //
    {0x1041BD9E, {"mod", "rModel"}},                            //
    {0x10C460E6, {"msg", "rMessage"}},                          //
    {0x11AFA688, {"sdl.xml", "rScheduler"}},                    //
    {0x11C35522, {"gr2", "rGrass2"}},                           //
    {0x11C82587, {"oba", "rObjAdj"}},                           //
    {0x12191BA1, {"epv", "rEffectProvider"}},                   //
    {0x124597FE, {"xml", "rEventTimeSchedule"}},                //
    {0x12688D38, {"pjp", "rPlJumpParam"}},                      //
    {0x139EE51D, {"lmt", "rMotionList"}},                       //
    {0x15302EF4, {"lot", "rLayout"}},                           //
    {0x157388D3, {"itl", "rItemList"}},                         //
    {0x15773620, {"nmr", "rArmorModelNpc"}},                    //
    {0x15D782FB, {"sbkr", "rSoundBank"}},                       //
    {0x167DBBFF, {"stq", "rSoundStreamRequest"}},               //
    {0x169B7213, {"irp", "rItemRandParam"}},                    //
    {0x1823137D, {"mlm", "rMapLndMarkPos"}},                    //
    {0x19054795, {"nnl", "rNpcLedgerList"}},                    //
    {0x191F0AC9, {"fur", "rFurTable::FurTable"}},               //
    {0x199C56C0, {"ocl", "rObjCollision"}},                     //
    {0x1AEB54D1, {"hvl.xml", "rHavokVertexLayout"}},            //
    {0x1B520B68, {"zon", "rZone"}},                             //
    {0x1BA81D3C, {"nck", "rNeck"}},                             //
    {0x1BCC4966, {"srq", "rSoundRequest"}},                     //
    {0x1C2B501F, {"atr", "rArmorTable"}},                       //
    {0x1D35AF2B, {"hit", "rHit"}},                              //
    {0x1DC202EA, {"stef", "rSceneEffect"}},                     //
    {0x1EFB1B67, {"adh", "rAdhesion"}},                         //
    {0x1F9FA62C, {"rbl2", "rRumble"}},                          //
    {0x20208A05, {"mtg", "rModelMontage"}},                     //
    {0x2052D67E, {"sn2", "rAISensorExt"}},                      //
    {0x20FA758C, {"gnd", "rFlashFontConfig"}},                  //
    {0x2993EA18, {"gnd", "rFlashFontConfig"}},                  //
    {0x21034C90, {"arc", "rArchive"}},                          //
    {0x215896C2, {"statusparam", "rStatusParam"}},              //
    {0x2282360D, {"jex", "rJointEx"}},                          //
    {0x22948394, {"gui", "rGUI"}},                              //
    {0x22B2A2A2, {"PlNeckPos", "rPlNeckPos"}},                  //
    {0x232E228C, {"rev_win", "rReverb"}},                       //
    {0x234D7104, {"cdf", "rCloth"}},                            //
    {0x241F5DEB, {"tex", "rTexture"}},                          //
    {0x242BB29A, {"gmd", "rGUIMessage"}},                       //
    {0x2447D742, {"idm", "sId::rIdMapList"}},                   //
    {0x257D2F7C, {"swm", "rSwingModel"}},                       //
    {0x25E98D8C, {"rclp", "rRoomCutList"}},                     //
    {0x264087B8, {"fca", "rFacialAnimation"}},                  //
    {0x26C299D0, {"hvd.xml", "rHavokVehicleData"}},             //
    {0x271D08FE, {"ssq", "rSoundSequenceSe"}},                  //
    {0x272B80EA, {"prp", "rPropParam"}},                        //
    {0x2749C8A8, {"mrl", "rMaterial"}},                         //
    {0x276DE8B7, {"e2d", "rEffect2D"}},                         //
    {0x27CE98F6, {"rtex", "rRenderTargetTexture"}},             //
    {0x29948FBA, {"srd", "rSoundRandom"}},                      //
    {0x2A37242D, {"gpl", "rLayoutGroupParamList"}},             //
    {0x2A51D160, {"ems", "rEnemyLayout"}},                      //
    {0x2B303957, {"gop", "rAIGoalPlanning"}},                   //
    {0x2B399B97, {"evc2", "rEventCollision"}},                  //
    {0x2B40AE8F, {"equ", "rSoundEQ"}},                          //
    {0x2B93C4AD, {"route", "rRouteNode"}},                      //
    {0x2CC70A66, {"grd", "rFlashDef"}},                         //
    {0x2CE309AB, {"joblvl", "rPlJobLevel"}},                    //
    {0x2D12E086, {"srd", "rSoundRandom"}},                      //
    {0x2D462600, {"gfd", "rGUIFont"}},                          //
    {0x2D6455B1, {"tde", "rTexDetailEdit"}},                    //
    {0x2DC54131, {"cdf", "rCloth"}},                            //
    {0x2E47C723, {"seg", "rSoundSeg"}},                         //
    {0x30ED4060, {"pth", "rPath"}},                             //
    {0x30FC745F, {"smx", "rSoundSubMixer"}},                    //
    {0x31B81AA5, {"qr", "rQuestReward"}},                       //
    {0x32231BD1, {"esd", "rEffectSetData"}},                    //
    {0x325AACA5, {"shl", "rShlParamList"}},                     //
    {0x338C1FEC, {"asl", "rAreaSetLayout"}},                    //
    {0x33AE5307, {"spc", "rSoundPackage"}},                     //
    {0x33B21191, {"esp", "rEmShaderParam"}},                    //
    {0x33B68E3E, {"xml", "rItemLayout"}},                       //
    {0x340F49F9, {"sds", "rSoundDirectionalSet"}},              //
    {0x34A8C353, {"sprmap", "rSprLayout"}},                     //
    {0x354284E7, {"lvl", "rPlLevel"}},                          //
    {0x358012E8, {"vib", "rVibration"}},                        //
    {0x36E29465, {"hkx", "rHkCollision"}},                      //
    {0x3821B94D, {"sngw", "rSoundSourceMusic"}},                //
    {0x3900DAD0, {"sbc", "rCollision"}},                        //
    {0x3991981E, {"msd", "rMirrorSetData"}},                    //
    {0x39A0D1D6, {"sms", "rSoundSubMixerSet"}},                 //
    {0x39C52040, {"lcm", "rCameraList"}},                       //
    {0x3A947AC1, {"cql", "rCameraQuakeList"}},                  //
    {0x3B350990, {"qsp", "rQuestSudden"}},                      //
    {0x3C318636, {"wao", "rWeaponAttachOffset"}},               //
    {0x3C3D0C05, {"hkx", "rHkCollision"}},                      //
    {0x3CAD8076, {"tex", "rTexture"}},                          //
    {0x3CBBFD41, {"ucp2", "rUpCutPuzzle"}},                     //
    {0x3D007115, {"wed", "rSoundWed"}},                         //
    {0x3D97AD80, {"amr", "rArmorModel"}},                       //
    {0x3E356F93, {"stc", "rStarCatalog"}},                      //
    {0x3E363245, {"chn", "rChain"}},                            //
    {0x3E394A0E, {"npcfsmbrn", "rFSMBrain"}},                   //
    {0x3EE10653, {"wad", "rWeskerAttackData"}},                 //
    {0x3FB52996, {"imx", "rItemMix"}},                          //
    {0x4046F1E1, {"ajp", "rAdjustParam"}},                      //
    {0x405FF76E, {"bct2", "rBgChangeTable"}},                   //
    {0x4126B31B, {"npcmac", "rNMMachine"}},                     //
    {0x430B4FF4, {"ptl", "rPathList"}},                         //
    {0x437662FC, {"oml", "rOmList"}},                           //
    {0x44C8AC26, {"mloc", "rMapLocate"}},                       //
    {0x44E79B6E, {"sdl", "rScheduler"}},                        //
    {0x4509FA80, {"itemlv", "rItemLevelParam"}},                //
    {0x456B6180, {"cnsshake", "rCnsShake"}},                    //
    {0x46810940, {"egv", "rSoundEngineValue"}},                 //
    {0x472022DF, {"AIPlActParam", "rAIPlayerActionParameter"}}, //
    {0x482B5B95, {"esl", "rEffectSetData"}},                    //
    {0x48459606, {"seq", "rSoundSeq"}},                         //
    {0x48538FFD, {"ist", "rItemSetTbl"}},                       //
    {0x4857FD94, {"rev_win", "rReverb"}},                       //
    {0x49B5A885, {"ssc", "rSoundSimpleCurve"}},                 //
    {0x4A31FCD8, {"scoop.xml", "rScoopList"}},                  //
    {0x4B704CC0, {"mia", "rMagicItemActParamList"}},            //
    {0x4C0DB839, {"sdl", "rScheduler"}},                        //
    {0x4CA26828, {"mse", "rSoundMotionSe"}},                    //
    {0x4CDF60E9, {"msg", "rMessage"}},                          //
    {0x4D894D5D, {"cmi", "sCubeMapInterp::rCubeMapInterp"}},    //
    {0x4E2FEF36, {"mtg", "rModelMontage"}},                     //
    {0x4E32817C, {"bfx", "rShader"}},                           //
    {0x4E397417, {"ean", "rEffectAnim"}},                       //
    {0x4EF19843, {"nav", "rNavigationMesh"}},                   //
    {0x4F16B7AB, {"hri", "rHkRBInfoList"}},                     //
    {0x4F6FFDDC, {"fcp", "rFacialPattern"}},                    //
    {0x4FB35A95, {"aor", "rArmorPartsOff"}},                    //
    {0x50F3D713, {"skl", "rSkillList"}},                        //
    {0x5175C242, {"geo2", "rGeometry2"}},                       //
    {0x51FC779F, {"sbc", "rCollision"}},                        //
    {0x5204D557, {"shp", "rShopList"}},                         //
    {0x522F7A3D, {"fcp", "rFacialPattern"}},                    //
    {0x528770DF, {"efs", "rEffectStrip"}},                      //
    {0x531EA143, {"uct2", "rUpCutObjTable"}},                   //
    {0x535D969F, {"ctc", "rCnsTinyChain"}},                     //
    {0x538120DE, {"eng", "rSoundEngine"}},                      //
    {0x5435D27B, {"hkm", "rHkMaterial"}},                       //
    {0x543E41DE, {"mrk", "rMarkerLayout"}},                     //
    {0x54503672, {"ahl", "rAreaHitLayout"}},                    //
    {0x55A8FB34, {"anm", "rSprAnm"}},                           //
    {0x57BAE388, {"hcl", "rHavokConstraintLayout"}},            //
    {0x5802B3FF, {"ahc", "rAHCamera"}},                         //
    {0x585831AA, {"pos", "rPos"}},                              //
    {0x586995B1, {"snd", "rSoundSnd"}},                         //
    {0x58A15856, {"mod", "rModel"}},                            //
    {0x59D80140, {"ablparam", "rPlAbilityParam"}},              //
    {0x5A3CED86, {"rrd", "rSoundRrd"}},                         //
    {0x5A45BA9C, {"xml", "rMapLink"}},                          //
    {0x5A7FEA62, {"ik", "rCnsIK"}},                             //
    {0x5B334013, {"bap", "rBowActParamList"}},                  //
    {0x5E3DC9F3, {"lge", "rGridEnvLight"}},                     //
    {0x5E4C723C, {"nls", "rNulls"}},                            //
    {0x5E640ACC, {"itm", "rItemAngle"}},                        //
    {0x5EA7A3E9, {"sky", "rSky"}},                              //
    {0x5EF1FB52, {"lcm", "rCameraList"}},                       //
    {0x5F36B659, {"way", "rAIWayPoint"}},                       //
    {0x5F84F7C4, {"mem.wmv", "rMovie"}},                        //
    {0x5F88B715, {"epd", "rEditPatternData"}},                  //
    {0x60524FBB, {"shw", "rShape"}},                            //
    {0x60DD1B16, {"lsp", "rLayoutSpr"}},                        //
    {0x6186627D, {"wep", "rWeatherEffectParam"}},               //
    {0x628DFB41, {"gr2s", "rGrass2Setting"}},                   //
    {0x63747AA7, {"rpi", "rRomPawnInfo"}},                      //
    {0x63B524A7, {"ltg", "rLockOnTarget"}},                     //
    {0x64387FF1, {"qlv", "rEquipLvUp"}},                        //
    {0x64BFE66D, {"bta", "rBtnAct"}},                           //
    {0x652071B0, {"rsl", "rScrList"}},                          //
    {0x66B45610, {"fsm", "rAIFSM"}},                            //
    {0x671F21DA, {"stp", "rStartPos"}},                         //
    {0x68E301A1, {"uce2", "rUpCutElevator"}},                   //
    {0x69A5C538, {"dwm", "rDeformWeightMap"}},                  //
    {0x6C1D2073, {"srq", "rSoundRequest"}},                     //
    {0x6C3B4904, {"hlc", "rHavokLinkCollisionLayout"}},         //
    {0x6D0115ED, {"prt", "rAIPriorityThink"}},                  //
    {0x6D5AE854, {"efl", "rEffectList"}},                       //
    {0x6EE70EFF, {"pcf", "rAIPathConsecutive"}},                //
    {0x6F302481, {"plw", "rPlWeight"}},                         //
    {0x6F62D575, {"tde", "rTexDetailEdit"}},                    //
    {0x714EC77C, {"bes2", "rEventScript"}},                     //
    {0x71D6A0D4, {"osf", "rObjSet"}},                           //
    {0x724DF879, {"xsew", "rSoundSourceMSADPCM"}},              //
    {0x73850D05, {"arc", "rArchive"}},                          //
    {0x7470D7E9, {"tex2", "r2Texture"}},                        //
    {0x754B82B4, {"ahs", "rAreaHitShape"}},                     //
    {0x758B2EB7, {"cef", "rCommonEffect"}},                     //
    {0x75D21272, {"cdp", "rSoundCdp"}},                         //
    {0x76820D81, {"lmt", "rMotionList"}},                       //
    {0x76DE35F6, {"rpn", "rRomPawnName"}},                      //
    {0x7808EA10, {"rtex", "rRenderTargetTexture"}},             //
    {0x7817FFA5, {"fbik_human", "rFullbodyIKHuman"}},           //
    {0x7A038F4C, {"rev_win", "rReverb"}},                       //
    {0x7AA81CAB, {"eap", "rAIEnemyActionParameter"}},           //
    {0x7D1530C2, {"sngw", "rSoundSourceMusic"}},                //
    {0x7E1C8D43, {"pcs", "rAIPresetStudy"}},                    //
    {0x7E33A16C, {"spc", "rSoundPackage"}},                     //
    {0x7E4152FF, {"stg", "rAISensorTargetGroup"}},              //
    {0xC230157D, {"at3", "rSoundSourceMusic"}},                 //
    {0x5F12110E, {"wpt", "rWeskerParamTable"}},                 //
    {0x176C3F95, {"los", "rLotSelector"}},                      //
    {0x557ECC08, {"aef", "rAHEffect"}},                         //
    {0x2C4666D1, {"srh", "rScrHiding"}},                        // (RE5: smh)
    {0x50F9DB3E, {"bfx", "rShader"}},                           //
    {0x017A550D, {"lom", "rLotSelectorMove"}},                  //
    {0x38F66FC3, {"seg", "rSoundSeGenerator"}},                 //
    {0x018735A6, {"hkm", "rHkMaterial"}},                       //
    {0x2C14D261, {"ahl", "rAreaHitLayout"}},                    //
    {0x2E590330, {"rsl", "rScrList"}},                          //
    {0x552E1B82, {"asl", "rAreaSetLayout"}},                    //
    {0x6A5CDD23, {"occ", "rOccluder"}},                         //
    {0x5CC5C1E2, {"ips", "rInitPosSet"}},                       //
    {0x1C775347, {"sbl", "rSceneBox2"}},                        //
    {0x20A81BF0, {"pef", "rAIPositionEvaluationFuncList"}},     //
    {0x56CF93D4, {"lnv", "rAINavigationMeshList"}},             //
    {0x55EF9710, {"hlc", "rHavokLinkCollisionLayout"}},         //
    {0x7494F854, {"hcl", "rHavokConstraintLayout"}},            //
    {0x37B53731, {"ibl", "rIntelligenceBox"}},                  //
    {0x62A68441, {"ttb", "rThinkTable"}},                       //
    {0x27D81C81, {"fes", "rFieldSet"}},                         //
    {0x2739B57C, {"grs", "rGrass"}},                            //
    {0x50F3721F, {"thk", "rThinkScript"}},                      //
    {0x745DAB77, {"pvb", "rPlanetVibration"}},                  //
    {0x753B3A8C, {"hvd", "rHavokVehicleData"}},                 //
    {0x1C635F38, {"msl", "rMaterialSkipList"}},                 //
    {0x2350E584, {"obc", "rCollisionObj"}},                     //
    {0x75967AD6, {"dsc", "rDynamicSbc"}},                       //
    {0x31AC0B5C, {"swc", "rScrCollisionArea"}},                 //
    {0x3FDA9B90, {"gmd", "rFlashMessage"}},                     //
    {0x40D14033, {"gfc", "rFlashConfig"}},                      //
    {0x1AE50150, {"vts", "rVertices"}},                         //
    {0x2A9FBCEC, {"hmg", "rHmGenerator"}},                      //
    {0x56CF8411, {"abe", "rAreaBarrierEdit"}},                  //
    {0x5AF4E4FE, {"mot", "rMotion"}},                           //
    {0x711BD05E, {"osf", "rObjSet"}},                           //
    {0x755BDD19, {"lpeq", "rPlanetEQ"}},                        //
    {0x4323D83A, {"stex", "rSceneTexture"}},                    //
    {0x67015AE5, {"wb", "rWaterBoundary"}},                     //
    {0x312607A4, {"bll", "rBlacklist"}},                        //
    {0x089BEF2C, {"sap", "rAIStageActionParameter"}},           //
    {0x52DBDCD6, {"rdd", "rRagdoll"}},                          //
    {0x12C3BFA7, {"cpl", "rCameraParamList"}},                  //
    {0x133917BA, {"mss", "rMsgSet"}},                           //
    {0x7DA64808, {"qmk", "rQuestMarker"}},                      //
    {0x72821C38, {"stm", "rPlStamina"}},                        //
    {0x0737E28B, {"rst", "rRegionStatus"}},                     //
    {0x2A4F96A8, {"rbd", "rRigidBody"}},                        //
    {0x6FE1EA15, {"spl", "rSoundPhysicsList"}},                 //
    {0x1EB3767C, {"spr", "rSoundPhysicsRigidBody"}},            //
    {0x2B0670A5, {"map", "rMagicActParamList"}},                //
    {0x32E2B13B, {"rdp", "rEditPawn"}},                         //
    {0x14428EAE, {"gce", "rCharacterEditPreset"}},              //
    {0x07B8BCDE, {"fca", "rFacialAnimation"}},                  //
    {0x31EDC625, {"spj", "rSoundPhysicsJoint"}},                //
    {0x3BBA4E33, {"qct", "rQuestCtrlTbl"}},                     //
    {0x48C0AF2D, {"msl", "rMsgSerial"}},                        //
    {0x534BF1A0, {"ddfcv", "rDDFCurve"}},                       //
    {0x619D23DF, {"shp", "rShopList"}},                         //
    {0x6DB9FA5F, {"cmc", "rCmcMsgConfig"}},                     //
    {0x7BEC319A, {"sps", "rSoundPhysicsSoftBody"}},             //
    {0x65B275E5, {"sce", "rScenario"}},                         //
    {0x0022FA09, {"hpe", "rHumanPartsEdit"}},                   //
    {0x4E44FB6D, {"fpe", "rFacePartsEdit"}},                    //
    {0x36019854, {"bed", "rBodyEdit"}},                         //
    {0x5A61A7C8, {"fed", "rFaceEdit"}},                         //
    {0x60BB6A09, {"hed", "rHumanEdit"}},                        //
    {0x14EA8095, {"cos", "rCnsOffsetSet"}},                     //
    {0x15155F8A, {"smh", "rSMHiding"}},                         //
    {0x1AADF7B7, {"cms", "rCameraMotionSdl"}},                  //
    {0x296BD0A6, {"hgm", "rHitGeometry"}},                      //
    {0x33046CD5, {"qcm", "rCameraQFPS"}},                       //
    {0x35BDD173, {"poa", "rPosAdjust"}},                        //
    {0x4B92D51A, {"llk", "rLightLinker"}},                      //
    {0x6E45FABB, {"atk", "rAttackParam"}},                      //
    {0x19F6EFCE, {"sep", "rSoundEnemyParam"}},                  //
    {0x1ED12F1B, {"glp", "rGoalPos"}},                          //
    {0x266E8A91, {"lku", "rLinkUnit"}},                         //
    {0x285A13D9, {"vzo", "rFxZone"}},                           //
    {0x25B4A6B9, {"bgm", "rSoundBGMControl"}},                  //
    {0x3B764DD4, {"sstr", "rSoundStreamTransition"}},           //
    {0x45E867D7, {"mll", "rMotionListList"}},                   //
    {0x4B768796, {"scn", "rSoundCondition"}},                   //
    {0x58819BC8, {"sso", "rSoundSmOcclusion"}},                 //
    {0x5FB399F4, {"bssq", "rBioSoundSequenceSe"}},              //
    {0x622FA3C9, {"ewy", "rAIWayPointExpand"}},                 //
    {0x6A9197ED, {"sst", "rSoundStreamStructure"}},             //
    {0x2C2DE8CA, {"adh", "rAdh"}},                              //
    {0x3B5C7FD3, {"ida", "rIdAnim"}},                           //
    {0x45F753E8, {"igs", "rInGameSound"}},                      //
    {0x6BB4ED5E, {"ich", "rLch"}},                              //
    {0x601E64CD, {"szs", "rSoundZoneSwitch"}},                  //
    {0x02A80E1F, {"lrd", "rLinkRagdoll"}},                      //
    {0x0A4280D9, {"shd", "rSoundHitData"}},                     //
    {0x1FA8B594, {"ahs", "rAreaHit"}},                          //
    {0x245133D9, {"cmr", "rCameraRail"}},                       //
    {0x2F4E7041, {"sgt", "rSoundGunTool"}},                     //
    {0x46FB08BA, {"bmt", "rBioModelMontage"}},                  //
    {0x54DC440A, {"msl", "rMotionSequenceList"}},               //
    {0x54E2D1FF, {"rpd", "rPadData"}},                          //
    {0x19A59A91, {"llk", "sLightLinker::rLightLinker"}},        //
    {0x5898749C, {"bbm", "rBioBgmMap"}},                        //
};

static decltype(extensions) extensionsPS3{
    {0x7A038F4C, {"rev_ps3", "rReverb"}},       //
    {0x4857FD94, {"rev_ps3", "rReverb"}},       //
    {0x232E228C, {"rev_ps3", "rReverb"}},       //
    {0x3821B94D, {"at3", "rSoundSourceMusic"}}, //
    {0x7D1530C2, {"at3", "rSoundSourceMusic"}}, //
};

auto GetPair(uint32 hash, revil::Platform platform) {
  auto Find = [hash](auto &reg) {
    auto found = reg.find(hash);

    if (!es::IsEnd(reg, found)) {
      return &(*found);
    }

    return (const decltype(extensions)::value_type *)(nullptr);
  };

  if (platform == revil::Platform::PS3) {
    auto retVal = Find(extensionsPS3);

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

namespace revil {
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
}; // namespace revil
