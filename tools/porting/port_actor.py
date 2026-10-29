#!/usr/bin/env python3
"""Copy a SoH actor into hyrule/overlays/actors/ and apply the mechanical renames of hyrule/README.md.

    python3 tools/porting/port_actor.py ../Shipwright/soh/src/overlays/actors/ovl_En_Ko

What it does (each a plain text rewrite):
- ActorInit becomes an ActorProfile named <Name>_Oot_Profile, with _Oot actor and object ids; OoT's
  gameplay_keep becomes MM's GAMEPLAY_KEEP, which is always loaded.
- Actor, object, item and get-item ids elsewhere get the _Oot suffix, and SoH asset includes point at
  assets/oot/.
- OoT sound effects MM has under another name at the same id take MM's name, damage tables become
  DMG_TABLE_FROM_OOT, and Navi's enemy info (which MM's Tatl hints lack) is left out with a PORT: note.
- Actor_UpdateBgCheckInfo flags with OoT's floor-and-water check (4) gain MM's water check (0x400).
- Collider, init chain and actor field names change to MM's (COL_MATERIAL_, ATELEM_, cullingVolume*,
  terminalVelocity, speed, depthInWater, objectSlot, attentionRangeType, lockOnArrowOffset...).
- OoT flags use HyruleFlags.h, OoT text ids in returns, cases and textId assignments get TEXT_OOT, and
  OoT age checks use HyrulePort.h.
- Debug prints are removed, SoH's GameInteractor_Should(VB_..., cond) hooks become (cond), ifs on SoH's
  enhancements keep vanilla's branch, randomizer-only blocks go, and OoT's
  matrix, allocation, segment and default-sound calls become MM's.
- Exits by OoT entrance name go through Hyrule_ExitFromOot, Player cutscene actions through
  Player_SetCsActionWithHaltedActorsOot (MM numbers them differently), and raw damage-flag tests through
  DMG_FROM_OOT. Prototypes in the actor's header of functions the .c makes static are left out.
- OoT's song, staff action and ocarina mode names go through hyrule/HyruleOcarina.h (modes are read and set in
  OoT's values). OoT's Player plays for an actor that asks (PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR); MM's waits for
  an actor to offer itself (Actor_OfferOcarinaInteraction), which needs a hand port. SoH's cvars and stats, and anything
  else that needs a decision, are listed at the end for review.
- Functions become static.

Includes of OoT actors that are ported (or named in PORTING=ovl_A,ovl_B) use a relative path, since MM may have an
actor of the same name. Data files that the actor #includes (such as z_en_kanban_gfx.c) need renaming by hand, since every .c
file under hyrule/ is compiled.

The result needs a build and a review: semantic differences (Player cutscene actions, sound effects,
damage masks, drop tables) are not handled, and the report lists what to check.
"""
import argparse
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_ROOT = os.path.join(HERE, "..", "..", "hyrule", "overlays", "actors")

RENAMES = [
    (r"\bCOLTYPE_", "COL_MATERIAL_"),
    (r"\bELEMTYPE_", "ELEM_MATERIAL_"),
    (r"\bTOUCH_ON\b", "ATELEM_ON"),
    (r"\bTOUCH_NONE\b", "ATELEM_NONE"),
    (r"\bTOUCH_NEAREST\b", "ATELEM_NEAREST"),
    (r"\bEffect_Delete\b", "Effect_Destroy"),
    (r"\bActor_SetProjectileSpeed\b", "Actor_SetSpeeds"),
    (r"\bMSGMODE_OCARINA_CORRECT_PLAYBACK\b", "MSGMODE_DISPLAY_SONG_PLAYED"),
    (r"\bTOUCH_SFX_", "ATELEM_SFX_"),
    (r"\bBUMP_ON\b", "ACELEM_ON"),
    (r"\bBUMP_NONE\b", "ACELEM_NONE"),
    (r"\bBUMP_HOOKABLE\b", "ACELEM_HOOKABLE"),
    (r"\buncullZoneForward\b", "cullingVolumeDistance"),
    (r"\buncullZoneScale\b", "cullingVolumeScale"),
    (r"\buncullZoneDownward\b", "cullingVolumeDownward"),
    (r"\bminVelocityY\b", "terminalVelocity"),
    (r"\bactor\.speedXZ\b", "actor.speed"),
    (r"\byDistToWater\b", "depthInWater"),
    (r"\bobjBankIndex\b", "objectSlot"),
    (r"\btargetMode\b", "attentionRangeType"),
    (r"\btargetArrowOffset\b", "lockOnArrowOffset"),
    (r"\bisTargeted\b", "isLockedOn"),
    (r"\bTEXT_STATE_DONE_FADING\b", "TEXT_STATE_FADING"),
    (r"\bTEXT_STATE_DONE_HAS_NEXT\b", "TEXT_STATE_NEXT"),
    (r"\bActor_ProcessTalkRequest\((&?[\w>.-]+), play\)", r"Actor_TalkOfferAccepted(\1, &play->state)"),
    (r"\bSEGMENTED_TO_VIRTUAL\(", "Lib_SegmentedToVirtual("),
    (r"\bQuake_Add\(([^,]+), (\d)\)", r"Quake_Request(\1, QUAKE_TYPE_\2)"),
    (r"\bQuake_SetQuakeValues\(", "Quake_SetPerturbations("),
    (r"\bWaterBox_GetSurfaceImpl\(", "BgCheck_GetWaterSurfaceNoBgId("),
    (r"\bActor_UpdateVelocityXZGravity\(", "Actor_UpdateVelocityWithGravity("),
    (r"\bMatrix_RotateAxis\(", "Matrix_RotateAxisF("),
    (r"\bMatrix_MtxFToYXZRotS\(", "Matrix_MtxFToYXZRot("),
    (r"\bQuake_SetCountdown\(", "Quake_SetDuration("),
    (r"\bfunc_80033480\(", "func_800BBFB0("),
    (r"\bActor_SetPlayerKnockbackLarge\(", "func_800B8D50("),
    (r"\bActor_SetPlayerKnockbackLargeNoDamage\(", "func_800B8D98("),
    (r"\bSfx_PlaySfxCentered\(", "Audio_PlaySfx("),
    # OoT's miniboss music start and stop.
    (r"\bfunc_800F5ACC\(", "Audio_PlayBgm_StorePrevBgm("),
    (r"\bfunc_800F5B58\(", "Audio_RestorePrevBgm("),
    (r"\bDamageTable_Get\(", "DamageTable_Get_Oot("),
    (r"\bIS_DAY\b", "IS_DAY_OOT"),
    (r"\bGET_GS_FLAGS\(", "GET_GS_FLAGS_OOT("),
    (r"\bSET_GS_FLAGS\(", "SET_GS_FLAGS_OOT("),
    (r"\bSkelAnime_DrawSkeletonOpa\(", "SkelAnime_DrawSkeletonOpaOot("),
    (r"\bDEGF_TO_RADF\(", "DEG_TO_RAD("),
    (r"\bIS_NIGHT\b", "IS_NIGHT_OOT"),
    (r"\bactorCtx\.unk_02\b", "actorCtx.unk2"),
    (r"\bPLAYER_STATE1_CLIMBING_LADDER\b", "PLAYER_STATE1_200000"),
    (r"\b(elements\[[^]]+\])\.info\b", r"\1.base"),
    (r"\b(\w*[cC]ollider|col\w*|cyl|hitbox)(\[[^]]*\])?(\.|->)info\b", r"\1\2\3elem"),
    (r"\bColliderInfo\b", "ColliderElement"),
    (r"\bBUMP_(\w+)", r"ACELEM_\1"),
    (r"\bTOUCH_(\w+)", r"ATELEM_\1"),
    (r"\.elemType\b", ".elemMaterial"),
    (r"\.colType\b", ".colMaterial"),
    (r"\b(actor|thisx|rideActor)(\.|->)speedXZ\b", r"\1\2speed"),
    (r"->linearVelocity\b", "->speedXZ"),  # Player's
    (r"\byDistToPlayer\b", "playerHeightRel"),
    # MM's Tatl hints do not cover OoT enemies: init chains keep their shape with no hint.
    (r"\bICHAIN_S8\(naviEnemyId, (\w+), (ICHAIN_\w+)\),",
     r"ICHAIN_U8(hintId, TATL_HINT_ID_NONE, \2), // PORT: OoT's Navi enemy info \1"),
    (r"^([ \t]*)(?:this->actor\.|thisx->)naviEnemyId = (\w+);(?: //)?", r"\1// PORT: OoT gives Navi enemy info \2;"),
    (r"^([ \t]*)(?:this->actor\.|thisx->)naviEnemyId \+= (\w+);", r"\1// PORT: OoT gives Navi the enemy info \2 further on."),
    (r"\bActor_SetDropFlag(JntSph)?\(", r"Actor_SetDropFlag\1_Oot("),
    (r"\bEffectSsDeadDb_Spawn\(", "EffectSsDeadDb_Spawn_Oot("),
    (r"\bPLAYER_SHIELD_(DEKU|HYLIAN|MIRROR)\b", r"PLAYER_SHIELD_\1_Oot"),
    # OoT's floating-collectible bit (no gravity) is MM's 0x10000; 0x4000 is part of MM's collectible flag.
    (r"(\bItem_DropCollectible2\([^;]*?)\b0x4000\b", r"\g<1>0x10000"),
    (r"\bITEM00_NUTS\b", "ITEM00_DEKU_NUTS_1"),
    (r"\bITEM00_STICK\b", "ITEM00_DEKU_STICK"),
    (r"\bITEM00_HEART\b", "ITEM00_RECOVERY_HEART"),
    (r"\bITEM00_SEEDS\b", "ITEM00_DEKU_SEEDS_Oot"),
    (r"\b(GI_(?!NONE\b|MAX\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(GID_(?!NONE\b|MAX\b|OOT_BASE\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"(\.|->)toucher\.", r"\1atDmgInfo."),
    (r"(\.|->)bumper\.", r"\1acDmgInfo."),
    (r"\.toucherFlags\b", ".atElemFlags"),
    (r"\.bumperFlags\b", ".acElemFlags"),
    (r"\bacHitInfo\b", "acHitElem"),
    (r"\batHitInfo\b", "atHitElem"),
    (r"\bPlayer_PlaySfx\(&GET_PLAYER\(play\)->actor, ", "Player_PlaySfx(GET_PLAYER(play), "),
    (r"\bPLAYER_STATE1_IN_ITEM_CS\b", "PLAYER_STATE1_10000000"),
    (r"\bPLAYER_STATE1_DAMAGED\b", "PLAYER_STATE1_4000000"),
    (r"\bPLAYER_STATE1_IN_WATER\b", "PLAYER_STATE1_8000000"),
    (r"\bPLAYER_STATE1_ON_HORSE\b", "PLAYER_STATE1_800000"),
    (r"\bPLAYER_STATE1_FLOOR_DISABLED\b", "PLAYER_STATE1_80000000"),
    (r"\bPLAYER_STATE2_GRABBED_BY_ENEMY\b", "PLAYER_STATE2_80"),
    (r"\bACTORCAT_EXPLOSIVE\b", "ACTORCAT_EXPLOSIVES"),
    (r"\bPLAYER_STATE1_IN_CUTSCENE\b", "PLAYER_STATE1_20000000"),
    (r"\bVIRTUAL_TO_PHYSICAL\(", "OS_K0_TO_PHYSICAL("),
    (r"\bGraph_Alloc\(", "GRAPH_ALLOC("),
    (r"\bMatrix_RotateX\(([^;]*), MTXMODE_APPLY\);", r"Matrix_RotateXFApply(\1);"),
    (r"\bMatrix_RotateX\(([^;]*), MTXMODE_NEW\);", r"Matrix_RotateXFNew(\1);"),
    (r"\bMatrix_RotateY\(([^;]*), (MTXMODE_\w+)\);", r"Matrix_RotateYF(\1, \2);"),
    (r"\bMatrix_RotateZ\(([^;]*), (MTXMODE_\w+)\);", r"Matrix_RotateZF(\1, \2);"),
    # In any order of the three flags.
    (r"\bgSPMatrix\((POLY_\w+_DISP\+\+), MATRIX_NEWMTX\(([^)]*)\),\s*"
     r"(?=[^;]*G_MTX_LOAD)(?=[^;]*G_MTX_MODELVIEW)(?:G_MTX_(?:NOPUSH|LOAD|MODELVIEW)(?: \| )?){2,3}\);",
     r"MATRIX_FINALIZE_AND_LOAD(\1, \2);"),
    (r"\bAudio_PlaySfxGeneral\((\w+), &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,\s*&gSfxDefaultFreqAndVolScale, "
     r"&gSfxDefaultReverb\);", r"Audio_PlaySfx(\1);"),
    (r"\bAudio_PlaySfxGeneral\((\w+), (&?[\w>.-]+), 4, &gSfxDefaultFreqAndVolScale,\s*&gSfxDefaultFreqAndVolScale, "
     r"&gSfxDefaultReverb\);", r"Audio_PlaySfx_AtPos(\2, \1);"),
    (r"\bAudio_PlaySfxGeneral\(", "AudioSfx_PlaySfx("),
    (r"\bSfx_PlaySfxAtPos\(", "Audio_PlaySfx_AtPos("),
    (r"\bSfx_PlaySfxCentered2\(", "Audio_PlaySfx_2("),
    (r"\bAudio_PlaySoundRiver\(", "Audio_PlaySfx_River("),
    (r"\bAudio_PlaySoundWaterfall\(", "Audio_PlaySfx_Waterfall("),
    (r"\bACTOR_FLAG_IGNORE_POINTLIGHTS\b", "ACTOR_FLAG_IGNORE_LEGACY_POINT_LIGHTS"),
    (r"\bgSaveContext\.cutsceneIndex\b", "gSaveContext.save.cutsceneIndex"),
    # OoT's WaterBox corner and surface are MM's minPos.
    (r"(\.|->)xMin\b", r"\1minPos.x"),
    (r"(\.|->)ySurface\b", r"\1minPos.y"),
    (r"(\.|->)zMin\b", r"\1minPos.z"),
    (r"\bfunc_8003EBF8\(", "DynaPoly_DisableCollision("),
    (r"\bfunc_8003EC50\(", "DynaPoly_EnableCollision("),
    (r"\bfunc_800F436C\(", "Audio_PlaySfx_AtPosWithFreq("),
    # SoH's savestate field lists.
    (r"#define \w+_SHIP_SAVESTATE_FIELDS\(F\)(?:[^\n]*\\\n)*[^\n]*\n\s*", ""),
    (r"SHIP_SAVESTATE_DEFINE\([^)]*\)\n\s*", ""),
    (r"\bview\.lookAt\b", "view.at"),
    # OoT's roomCtx.unk_74 (data cutscenes and actors pass to the scene draw config) is MM's unk7A.
    (r"\broomCtx\.unk_74\b", "roomCtx.unk7A"),
    # OoT's and MM's EnvironmentContext share their layout under other names.
    (r"\benvCtx\.unk_17\b", "envCtx.skyboxConfig"),
    (r"\benvCtx\.unk_18\b", "envCtx.changeSkyboxNextConfig"),
    (r"\benvCtx\.unk_19\b", "envCtx.changeSkyboxState"),
    (r"\benvCtx\.unk_1A\b", "envCtx.changeSkyboxTimer"),
    (r"\benvCtx\.indoors\b", "envCtx.lightMode"),
    (r"\benvCtx\.unk_1F\b", "envCtx.lightConfig"),
    (r"\benvCtx\.unk_20\b", "envCtx.changeLightNextConfig"),
    (r"\benvCtx\.unk_21\b", "envCtx.changeLightEnabled"),
    (r"\benvCtx\.unk_22\b", "envCtx.changeLightTimer"),
    (r"\benvCtx\.unk_24\b", "envCtx.changeDuration"),
    (r"\benvCtx\.gloomySkyMode\b", "envCtx.stormRequest"),
    (r"\benvCtx\.unk_DE\b", "envCtx.stormState"),
    (r"\benvCtx\.lightningMode\b", "envCtx.lightningState"),
    (r"\benvCtx\.unk_EE\b", "envCtx.precipitation"),
    (r"\benvCtx\.adjAmbientColor\b", "envCtx.adjLightSettings.ambientColor"),
    (r"\benvCtx\.adjLight1Color\b", "envCtx.adjLightSettings.light1Color"),
    (r"\benvCtx\.adjFogColor\b", "envCtx.adjLightSettings.fogColor"),
    (r"\benvCtx\.adjFogNear\b", "envCtx.adjLightSettings.fogNear"),
    (r"\benvCtx\.adjFogFar\b", "envCtx.adjLightSettings.zFar"),
    (r"\bdyna\.unk_150\b", "dyna.pushForce"),
    (r"\bdyna\.unk_158\b", "dyna.yRotation"),
    (r"\bPLAYER_STATE2_MOVING_DYNAPOLY\b", "PLAYER_STATE2_10"),
    (r"\b(actorLists\[[^\]]+\])\.head\b", r"\1.first"),
    (r"\bActor_MoveXYZ\(", "Actor_MoveWithoutGravity("),
    (r"\bActor_SetPlayerKnockbackSmallNoDamage\(", "func_800B8E1C("),
    (r"\bActor_SetPlayerKnockbackSmall\(", "func_800B8DD4("),
    (r"\bfunc_800287AC\(", "func_800B12F0("),
    (r"\bfunc_8002DEEC\(", "func_800B7200("),
    (r"\bfunc_8002F5C4\(", "func_800B8C20("),
    (r"\bfunc_80033C30\(", "func_800BC620("),
    (r"\bfunc_800435D8\(", "DynaPolyActor_ValidateMove("),
    (r"\bObject_GetIndex\(", "Object_GetSlot("),
    (r"\bobjectCtx\.status\[", "objectCtx.slots["),
    # OoT's DynaPolyMoveFlag has MM's bits: 1 carries actors on top, 2 turns them.
    (r"\bDPM_UNK3\b", "DYNA_TRANSFORM_POS | DYNA_TRANSFORM_ROT_Y"),
    (r"\bDPM_PLAYER\b", "DYNA_TRANSFORM_POS"),
    (r"\bDPM_ENEMY\b", "DYNA_TRANSFORM_ROT_Y"),
    (r"\bDPM_UNK\b", "0"),
    # OoT's Floor5 (with play) is MM's Floor5_2, and OoT's Floor4 is MM's Floor5; in that order.
    (r"\bBgCheck_EntityRaycastFloor5\(", "BgCheck_EntityRaycastFloor5_2("),
    (r"\bBgCheck_EntityRaycastFloor4\(", "BgCheck_EntityRaycastFloor5("),
    (r"\bActor_MoveXZGravity\(", "Actor_MoveWithGravity("),
    (r"\bAudio_PlayActorSound2\(", "Actor_PlaySfx("),
    (r"\bGfx_SetupDL_(\d+)(Opa|Xlu)(\d?)\(", r"Gfx_SetupDL\1_\2\3("),
    # OoT's damage flash fog (z_eff_ss_dead.c) and floor-aligned matrix, under MM's names.
    (r"\bfunc_80026230\(", "func_800AE2A0("),
    (r"\bfunc_80026400\(", "func_800AE434("),
    (r"\bfunc_80026608\(", "func_800AE5A0("),
    (r"\bfunc_80038A28\(", "func_800C0094("),
    (r"\bAnimation_ChangeByInfo\(", "Actor_ChangeAnimationByInfo("),
    (r"\bACTOR_FLAG_GRASS_DESTROYED\b", "ACTOR_FLAG_800"),
    (r"\bplay->sceneNum\b", "play->sceneId"),
    (r"\bLINK_AGE_IN_YEARS\b", "LINK_AGE_IN_YEARS_OOT"),
    (r"\bLINK_IS_ADULT\b", "LINK_IS_ADULT_OOT"),
    (r"\bLINK_IS_CHILD\b", "LINK_IS_CHILD_OOT"),
    (r"\bCHECK_QUEST_ITEM\((QUEST_\w+)\)", r"CHECK_QUEST_ITEM_OOT(\1_Oot)"),
    (r"\bINV_CONTENT\(", "INV_CONTENT_OOT("),
    # OoT's exchange items (HyrulePlayer.h): MM's Player keeps item actions; offers are wrapped in convert().
    (r"\bActor_GetPlayerExchangeItemId\(", "Actor_GetPlayerExchangeItemIdOot("),
    (r"\bplayer->exchangeItemId = ([^;]+);", r"player->exchangeItemAction = Hyrule_ExchangeItemToActionOot(\1);"),
    (r"\bGET_ITEM_NONE\b", "GI_NONE"),
    (r"\bgSaveContext\.(eventChkInf|itemGetInf|infTable)\[", r"gSaveContext.save.saveInfo.\1Oot["),
    (r"\bgSaveContext\.rupees\b", "gSaveContext.save.saveInfo.playerData.rupees"),
    (r"\bCAM_STAT_(\w+)", r"CAM_STATUS_\1"),
    (r"\bPlay_CameraSetAtEye\(", "Play_SetCameraAtEye("),
    (r"\bfunc_8005B1A4\(", "Camera_SetFinishedFlag("),
    (r"\bDEGTORAD\(", "DEG_TO_RAD("),
    (r"\bPlayer_UnsetMask\(", "Player_UnsetMaskOot("),
    (r"\bPlayer_GetMask\(", "Player_GetMaskOot("),
    (r"\bText_GetFaceReaction\(", "Text_GetFaceReactionOot("),
    (r"\bEffectSsKiraKira_", "EffectSsKirakira_"),
    # OoT's songs on MM's ocarina (hyrule/HyruleOcarina.h): song and action names map to MM's, and modes, which MM
    # numbers differently, are read and set in OoT's values.
    (r"\bOCARINA_SONG_([A-Z_]+)\b(?!_Oot)", r"OCARINA_SONG_\1_Oot"),
    (r"\bOCARINA_ACTION_([A-Z0-9_]+)\b(?!_Oot)", r"OCARINA_ACTION_\1_Oot"),
    (r"\bOCARINA_MODE_(0[0-9A-F])\b", r"OCARINA_MODE_\1_Oot"),
    (r"\b(\w+)->msgCtx\.ocarinaMode = ([^;]+);", r"Hyrule_SetOcarinaModeOot(\1, \2);"),
    (r"\b(\w+)->msgCtx\.ocarinaMode\b", r"Hyrule_GetOcarinaModeOot(\1)"),
    (r"\b(?:func_8010BD58|Message_StartOcarina)\(", "Message_StartOcarinaOot("),
    (r"\bmsgCtx\.unk_E3F2\b", "msgCtx.lastPlayedSong"),  # OoT's "chk_ocarina_no"
    # OoT's cutscenes play with OoT's interpreter (hyrule/HyruleCutscene.h), in OoT's own context and types.
    (r"\bplay->csCtx\.(npcActions|frames|state|segment|linkAction|unk_1[8AB])\b", r"gCsCtxOot.\1"),
    (r"\bgSaveContext\.cutsceneTrigger = (\d);", r"Hyrule_TriggerCutsceneOot(\1);"),
    (r"\bCutscene_SetSegment\(play, ([^;]+)\);", r"gCsCtxOot.segment = \1;"),
    (r"\b(CS_STATE_[A-Z_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\bCS_CMD_(CONTINUE|STOP)\b", r"CS_CMD_\1_Oot"),
    (r"\bFlags_(Get|Set|Unset)Env\(", r"Flags_\1EnvOot("),
    (r"\bfunc_800BB2B4\(", "Hyrule_CsSplineStepOot("),
    (r"\bgSaveContext\.linkAge\b", "LINK_AGE_OOT"),
    # OoT's math helpers that fill an out-parameter return their value in MM.
    (r"\bVecSph\b", "VecGeo"),
    (r"\bOLib_VecSphGeoToVec3f\(&(\w+), ([^;]+)\);", r"\1 = OLib_VecGeoToVec3f(\2);"),
    (r"\bOLib_Vec3fToVecSphGeo\(&(\w+), ([^;]+)\);", r"\1 = OLib_Vec3fToVecGeo(\2);"),
    (r"\bActor_GetWorld\(&(\w+), ([^;]+)\);", r"\1 = Actor_GetWorld(\2);"),
    (r"\bCsCmdActor(?:Cue|Action)\b", "CsCmdActorCueOot"),
    (r"\bCutsceneCameraPoint\b", "CutsceneCameraPointOot"),
    (r'#include "z64cutscene_commands\.h"', '#include "HyruleCutsceneCommands.h"'),
    (r"\b(?<!OOT_)CS_(?!CMD_|STATE_)([A-Z0-9_]+)\(", r"OOT_CS_\1("),
    (r"\bfunc_8010BD88\(", "Message_StartOcarinaNoWarpOot("),
    # OoT's clock is MM's.
    (r"\bgSaveContext\.dayTime\b", "gSaveContext.save.time"),
    (r"\bgTimeSpeed\b", "R_TIME_SPEED"),
    (r"\bSkelAnime_DrawSkeleton2\(", "SkelAnime_DrawSkeleton2Oot("),
    (r"\bD_80116280\b", "D_80116280_Oot"),
    (r"\bSEG_ADDR\(1, 0\)", "D_01000000_TO_SEGMENTED"),
    (r"\bAudio_StopSfxByPosAndId\(", "AudioSfx_StopByPosAndId("),
    (r"\bBEANS_BOUGHT\b", "BEANS_BOUGHT_OOT"),
    (r"\bHIGH_SCORE\((HS_\w+)\)", r"HIGH_SCORE_OOT(\1_Oot)"),
    # MM's circle transition has no starburst, colour or speed parameters.
    (r"\bTRANS_TYPE_CIRCLE\([^)]*\)", "TRANS_TYPE_CIRCLE"),
    # OoT's specular highlight setup for opaque and translucent display lists.
    (r"\bfunc_8002EBCC\(", "func_800B8050("),
    (r"\bfunc_8002ED80\(", "func_800B8118("),
    (r"(atDmgInfo\.dmgFlags & )(0x[0-9A-Fa-f]+)\b", r"\1DMG_FROM_OOT(\2)"),
    (r"\b(?:Player_SetCsActionWithHaltedActors|func_8002DF54)\(", "Player_SetCsActionWithHaltedActorsOot("),
    (r"\bPLAYER_MASK_([A-Z_]+)\b(?!_Oot)", r"PLAYER_MASK_\1_Oot"),
    (r"\bgSaveContext\.eventInf\[", "gSaveContext.eventInfOot["),
    (r"\bCUR_EQUIP_VALUE\(", "GET_CUR_EQUIP_VALUE("),
    (r"\bGET_EVENTCHKINF_CARPENTERS_FREE_ALL\(\)", "GET_EVENTCHKINF_CARPENTERS_FREE_ALL_OOT()"),
    (r"\bFlags_GetEventChkInf\((\w+)\)", r"GET_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_SetEventChkInf\((\w+)\)", r"SET_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_UnsetEventChkInf\((\w+)\)", r"CLEAR_EVENTCHKINF_OOT(\1_Oot)"),
    (r"\bFlags_GetInfTable\((\w+)\)", r"GET_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_SetInfTable\((\w+)\)", r"SET_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_UnsetInfTable\((\w+)\)", r"CLEAR_INFTABLE_OOT(\1_Oot)"),
    (r"\bFlags_GetItemGetInf\((\w+)\)", r"GET_ITEMGETINF_OOT(\1_Oot)"),
    (r"\bFlags_SetItemGetInf\((\w+)\)", r"SET_ITEMGETINF_OOT(\1_Oot)"),
    (r"\bFlags_GetEventInf\((\w+)\)", r"GET_EVENTINF_OOT(\1_Oot)"),
    (r"\bFlags_SetEventInf\((\w+)\)", r"SET_EVENTINF_OOT(\1_Oot)"),
    (r"\b((?:EVENTCHKINF|INFTABLE|ITEMGETINF|EVENTINF)_[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(ACTOR_(?!FLAG_)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(OBJECT_(?!GAMEPLAY_KEEP\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\bOBJECT_GAMEPLAY_KEEP\b", "GAMEPLAY_KEEP"),
    (r"\b(SCENE_[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(ITEM_(?!NONE\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
]
# Lines that are SoH-only and dropped when they stand alone.
DROP_LINE = re.compile(r"^\s*#include \"(vt\.h|soh/[^\"]+|overlays/effects/ovl_Effect_Ss_Dead_Sound/[^\"]+)\"\s*$|"
                       r"^\s*#include <libultraship/bridge/[^>]+>\s*$|"
                       # SoH's randomizer item entries.
                       r"^\s*(/\*[^*]*\*/ )?GetItemEntry getItemEntry( = \(GetItemEntry\)(GI_NONE|GET_ITEM_NONE))?;\s*$|"
                       r"^\s*this->getItemEntry = \(GetItemEntry\)(GI_NONE|GET_ITEM_NONE);\s*$")
TEXT_ID = re.compile(r"\b(0x[1-7][0-9A-Fa-f]{3})\b")
# Lines that give text ids: returns, cases, textId assignments, text calls and a GetTextId's `ret`. A statement that
# goes on to the next lines (a multi-line ternary) is converted to its end.
TEXT_LINE = re.compile(r"\breturn\b|\bcase\b|textId\b|\bret = |[Tt]ext\w* = |SetText\w*\(|SetTalkAction\(|"
                       r"Message_(?:Start|Continue)Textbox\(")
PROFILE = re.compile(
    r"const ActorInit (\w+)_InitVars = \{\s*(\w+),\s*(\w+),\s*(\w+),\s*(\w+),\s*sizeof\((\w+)\),\s*"
    r"(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),"
    r"\s*(?:\(ActorResetFunc\))?(\w+),?\s*\};")


def sfx_renames(soh_root):
    """OoT sound effects MM has under another name, by id (MM renamed many: OoT's NA_SE_EN_DEKU_JR_MOUTH is
    MM's NA_SE_EN_MIZUBABA1_MOUTH). Names MM has are kept, as they are MM's sound of the same name."""
    define = re.compile(r"#define (NA_SE_\w+) (0x[0-9A-Fa-f]+)")
    mm = dict(define.findall(open(os.path.join(HERE, "..", "..", "mm", "include", "sfx.h")).read()))
    by_id = {}
    for name, value in mm.items():
        by_id.setdefault(int(value, 16), name)
    oot = define.findall(open(os.path.join(soh_root, "include", "sfx.h")).read())
    return {name: by_id[int(value, 16)] for name, value in oot if name not in mm and int(value, 16) in by_id}


SFX_RENAMES = {}


def oot_entrances(soh_root):
    """OoT's entrance indexes by name, from SoH's entrance_table.h."""
    table = open(os.path.join(soh_root, "include", "tables", "entrance_table.h")).read()
    return {name: int(value, 16) for value, name in re.findall(r"/\* (0x[0-9A-Fa-f]+) \*/ DEFINE_ENTRANCE\((ENTR_\w+),", table)}


ENTRANCES = {}


def convert_entrances(text):
    """OoT exits by entrance name become MM entrances through HyruleScenes.h (an exit into a scene that is not
    loaded yet leads back to the spawn Link came in at), and checks of the entrance Link came in at compare MM's."""
    text = re.sub(r"play->nextEntranceIndex = (ENTR_\w+);",
                  lambda m: f"play->nextEntrance = Hyrule_ExitFromOot(0x{ENTRANCES[m.group(1)]:03X}); // OoT's {m.group(1)}",
                  text)
    return re.sub(r"gSaveContext\.entranceIndex ([!=]=) (ENTR_\w+)",
                  lambda m: f"gSaveContext.save.entrance {m.group(1)} Hyrule_EntranceFromOot(0x{ENTRANCES[m.group(2)]:03X} /* {m.group(2)} */)",
                  text)


def strip_rando_ifs(text):
    """SoH's randomizer-only blocks without an else (if (IS_RANDO) { ... }, no nested braces) are left out, as are
    randomizer-only terms of conditions and SoH's GameInteractor event calls."""
    text = re.sub(r"\s*&&\s*!\(?IS_RANDO\)?", "", text)
    text = re.sub(r"\n[ \t]*GameInteractor_Execute\w*\([^;]*\);", "", text)
    return re.sub(r"\n[ \t]*if \(IS_RANDO\) \{\n[^{}]*?\n[ \t]*\}(?=\n)(?![ \t]*else)", "", text)


def PORTED_ACTOR(ovl):
    """Whether an OoT actor is ported (or being ported) under hyrule/overlays/actors."""
    return os.path.isdir(os.path.join(OUT_ROOT, ovl)) or ovl in PORTING


PORTING = set()


def profile(m):
    name, actor, cat, flags, obj, size, init, destroy, update, draw, reset = m.groups()
    lines = [f"ActorProfile {name}_Oot_Profile = {{"]
    for v in (actor, cat, flags, obj, f"sizeof({size})", init, destroy, update, draw):
        lines.append(f"    /**/ {v},")
    if reset != "NULL":
        lines.append(f"    /**/ {reset},")
    lines.append("};")
    return "\n".join(lines)


def strip_game_interactor(text):
    """GameInteractor_Should(VB_X, cond[, ...]) is SoH's hook around vanilla behavior: keep (cond)."""
    out, i = [], 0
    while True:
        j = text.find("GameInteractor_Should(", i)
        if j < 0:
            out.append(text[i:])
            return "".join(out)
        out.append(text[i:j])
        k = j + len("GameInteractor_Should(")
        depth, args, start = 1, [], k
        while depth:
            c = text[k]
            if c in "([{":
                depth += 1
            elif c in ")]}":
                depth -= 1
            elif c == "," and depth == 1:
                args.append(text[start:k])
                start = k + 1
            k += 1
        args.append(text[start:k - 1])
        out.append("(" + args[1].strip() + ")" if len(args) > 1 else text[j:k])
        i = k


CVAR_OFF = r"CVarGetInteger\(CVAR_ENHANCEMENT\(\"[A-Za-z.]+\"\), 0\)"


def _block_end(text, i):
    """Index just past the brace block that opens at text[i] ('{')."""
    depth = 0
    for k in range(i, len(text)):
        if text[k] == "{":
            depth += 1
        elif text[k] == "}":
            depth -= 1
            if depth == 0:
                return k + 1
    raise ValueError("unbalanced braces")


def _dedent(block, indent):
    lines = block.split("\n")
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return "\n".join(l[4:] if l.startswith(indent + "    ") else l for l in lines) + "\n"


def strip_enhancement_ifs(text):
    """SoH enhancements are off by default: an if whose condition is one (or its negation) keeps the branch
    vanilla takes, and a `&& !enhancement` clause goes. The comments right above such an if go with it."""
    text = re.sub(r"\s*&&\s*!" + CVAR_OFF, "", text)
    head = re.compile(r"\n((?:[ \t]*//[^\n]*\n)*)([ \t]*)if \((!?)" + CVAR_OFF + r"\) \{")
    while True:
        m = head.search(text)
        if not m:
            return text
        indent, negated = m.group(2), m.group(3) == "!"
        open_brace = m.end() - 1
        end = _block_end(text, open_brace)
        then_block = text[open_brace + 1:end - 1]
        else_block = None
        rest = re.match(r" else \{", text[end:])
        if rest:
            else_open = end + rest.end() - 1
            else_end = _block_end(text, else_open)
            else_block = text[else_open + 1:else_end - 1]
            end = else_end
        kept = then_block if negated else else_block
        replacement = "\n" + (_dedent(kept, indent) if kept and kept.strip() else "")
        if text[end:end + 1] == "\n":  # the kept branch brings its own
            end += 1
        text = text[:m.start()] + replacement + text[end:]


def wrap_exchange_offers(text):
    """The exchange item an OoT actor offers to take becomes the item action MM's Actor_OfferTalkExchange takes."""
    out, i = [], 0
    for m in re.finditer(r"\bActor_OfferTalkExchange(?:EquiCylinder)?\(", text):
        if m.start() < i:
            continue
        depth, j, last_comma = 1, m.end(), None
        while depth:
            c = text[j]
            if c in "([":
                depth += 1
            elif c in ")]":
                depth -= 1
            elif c == "," and depth == 1:
                last_comma = j
            j += 1
        arg = text[last_comma + 1:j - 1].strip()
        out.append(text[i:last_comma + 1] + f" Hyrule_ExchangeItemToActionOot({arg}))")
        i = j
    return "".join(out) + text[i:]


def convert(text, header):
    out = []
    in_text = False
    for line in text.splitlines():
        if line.strip() == '#include "soh/frame_interpolation.h"':
            line = '#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"'
        elif DROP_LINE.match(line):
            continue
        if (in_text or TEXT_LINE.search(line)) and not line.lstrip().startswith("//"):
            line = TEXT_ID.sub(r"TEXT_OOT(\1)", line)
            in_text = not re.search(r"[;:{}]\s*(//.*)?$", line)
        out.append(line)
    text = "\n".join(out) + "\n"
    text = re.sub(r'#include "objects/(\w+)/(\w+)\.h"', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include <objects/(\w+)/(\w+)\.h>', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include "overlays/(ovl_\w+)/(\w+)\.h"', r'#include "assets/oot/overlays/\1/\2.h"', text)
    # Another ported OoT actor's header: by relative path, since MM may have an actor of the same name.
    text = re.sub(r'#include "overlays/actors/(ovl_\w+)/(\w+\.h)"',
                  lambda m: f'#include "../{m.group(1)}/{m.group(2)}"'
                  if PORTED_ACTOR(m.group(1)) else m.group(0), text)
    # One-line debug prints.
    text = re.sub(r"\n[ \t]*// \"[^\n]*\"(?=\n[ \t]*osSyncPrintf)", "", text)
    text = re.sub(r"\n[ \t]*osSyncPrintf\((?:[^;]|\n)*?\);", "", text)
    text = re.sub(r"\n[ \t]*LOG_(?:NUM|STRING|HEX|ADDRESS|FLOAT)\((?:[^;]|\n)*?\);", "", text)
    for pattern, repl in RENAMES:
        text = re.sub(pattern, repl, text, flags=re.M)
    text = re.sub(r"\bNA_SE_\w+\b", lambda m: SFX_RENAMES.get(m.group(0), m.group(0)), text)
    text = re.sub(r"\bMATRIX_NEWMTX\(", "Matrix_Finalize(", text)
    text = wrap_exchange_offers(text)
    # OoT damage tables (32 entries) become MM's 64 with DMG_TABLE_FROM_OOT.
    text = re.sub(r"(DamageTable \w+ = )\{\n((?:[ \t]*(?:/\*[^*]*\*/ )?DMG_ENTRY\([^)]*\),\n){32})\};",
                  lambda m: m.group(1) + "DMG_TABLE_FROM_OOT(\n" + m.group(2).rstrip(",\n") + "\n);", text)
    text = strip_game_interactor(text)
    text = strip_rando_ifs(text)
    text = convert_entrances(text)
    text = strip_enhancement_ifs(text)
    # OoT's background check flag 4 covers floor and water; MM checks water only with 0x400.
    text = re.sub(r"(Actor_UpdateBgCheckInfo\((?:[^;()]|\([^()]*\))*, )(0x[0-9A-Fa-f]+|\d+)\)",
                  lambda m: m.group(0) if not (int(m.group(2), 0) & 4) else
                  f"{m.group(1)}0x{int(m.group(2), 0) | 0x400:X})", text)
    # Collider damage masks (the first field of an element's AT and AC info) take MM's bits too.
    text = re.sub(r"\{ (0x[0-9A-Fa-f]{8}), (0x[0-9A-Fa-f]{2}), (0x[0-9A-Fa-f]{2}) \}",
                  lambda m: m.group(0) if int(m.group(1), 16) == 0 else
                  f"{{ DMG_FROM_OOT({m.group(1)}), {m.group(2)}, {m.group(3)} }}", text)
    if not header:
        text = PROFILE.sub(profile, text)
        text = re.sub(r"^(void|s32|u16|s16|f32|u8|u32) (\w+_\w+\()", r"static \1 \2", text, flags=re.M)
        # After the actor's own header, which comes first.
        extra = '#include "HyruleFlags.h"\n#include "HyrulePort.h"\n#include "HyruleActor.h"\n'
        extra += '#include "z64quake.h"\n' if "Quake_" in text else ""
        extra += '#include "z64rumble.h"\n' if "Rumble_" in text else ""
        extra += '#include "HyrulePlayer.h"\n' if re.search(r"\bPLAYER_SHIELD_\w+_Oot\b", text) else ""
        extra += '#include "HyruleScenes.h"\n' if "Hyrule_E" in text else ""
        extra += '#include "z64horse.h"\n' if "Horse_" in text else ""
        extra += '#include "HyruleOcarina.h"\n' if re.search(r"OCARINA_\w+_Oot|OcarinaOot\(|StartOcarina\w*Oot", text) else ""
        extra += '#include "HyruleCutscene.h"\n' if re.search(r"gCsCtxOot|CutsceneOot\(|CS_STATE_\w+_Oot|CsCmdActorCueOot", text) else ""
        # SoH's soh/ResourceManagerHelpers.h, dropped above; without it C assumes int and truncates pointers.
        extra += '#include "2s2h/BenPort.h"\n' if "ResourceMgr_" in text else ""
        text = re.sub(r'(#include "[^"]+"\n)', lambda m: m.group(1) + extra, text, count=1)
    else:
        text = text.replace("#include <libultraship/libultra.h>\n", "")
        text = re.sub(r"#ifndef (\w+)_H\n#define \1_H", r"#ifndef \1_OOT_H\n#define \1_OOT_H", text, count=1)
        if "BodyBreak" in text:  # OoT's, in HyruleActor.h
            text = text.replace('#include "global.h"\n', '#include "global.h"\n#include "HyruleActor.h"\n', 1)
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("soh_actor_dir", help="SoH's soh/src/overlays/actors/ovl_<Name> directory")
    args = parser.parse_args()
    src = os.path.normpath(args.soh_actor_dir)
    SFX_RENAMES.update(sfx_renames(os.path.join(src, "..", "..", "..", "..")))
    ENTRANCES.update(oot_entrances(os.path.join(src, "..", "..", "..", "..")))
    PORTING.update(a.strip() for a in os.environ.get("PORTING", "").split(",") if a.strip())
    out = os.path.join(OUT_ROOT, os.path.basename(src))
    os.makedirs(out, exist_ok=True)
    review = re.compile(r"GameInteractor|CVar|IS_RANDO|Randomizer|gSaveContext\.ship|Sfx_Play|Audio_Play|"
                        r"Player_SetCsAction|func_8002DF54|dmgFlags|"
                        r"Item_Drop|ITEM00_|MATRIX_NEWMTX|#include \"overlays/|\.table\[|ENTR_|Horse_")
    statics = set()
    for name in sorted(os.listdir(src), key=lambda n: n.endswith(".h")):
        if not name.endswith((".c", ".h")):
            continue
        with open(os.path.join(src, name)) as f:
            text = f.read()
        result = convert(text, name.endswith(".h"))
        if name.endswith(".c"):
            statics.update(re.findall(r"^static [^\n(=]*?\b(\w+)\(", result, flags=re.M))
        else:
            result = re.sub(r"^(?!typedef)[A-Za-z_][^\n(;]*?\b(\w+)\([^;{]*\);\n",
                            lambda m: "" if m.group(1) in statics else m.group(0), result, flags=re.M)
        with open(os.path.join(out, name), "w") as f:
            f.write(result)
        for i, line in enumerate(result.splitlines(), 1):
            if review.search(line):
                print(f"{os.path.join(out, name)}:{i}: review: {line.strip()[:110]}")
    print(f"ported into {os.path.normpath(out)}")


if __name__ == "__main__":
    main()
