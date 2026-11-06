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
    # OoT's cutscene camera types (HyruleCutscene.h), letterbox, HUD visibility and camera setting calls.
    (r"\bCutsceneCameraAngle\b", "CutsceneCameraAngleOot"),
    (r"\bCutsceneCameraMove\b", "CutsceneCameraMoveOot"),
    (r"\bLetterbox_SetSizeTarget\(", "ShrinkWindow_Letterbox_SetSizeTarget("),
    (r"\bInterface_ChangeHudVisibilityMode\(", "Interface_SetHudVisibility("),
    (r"\bHUD_VISIBILITY_NOTHING_ALT\b", "HUD_VISIBILITY_NONE_ALT"),
    (r"\bprevHudVisibilityMode\b", "prevHudVisibility"),
    (r"\bCamera_RequestSetting\(", "Camera_ChangeSetting("),
    # SoH's options read as their default, which is vanilla's.
    (r"\bCVarGetInteger\(CVAR_\w+\(\"[^\"]*\"\), (-?\w+)\)", r"\1"),
    (r"\bCVarGetColor24\(CVAR_\w+\(\"[^\"]*\"\), ((?:\(Color_RGB8\))?\{[^{}]*\}|[^()]+)\)", r"\1"),
    (r"\bCVarGetColor\(CVAR_\w+\(\"[^\"]*\"\), ((?:\(Color_RGBA8\))?\{[^{}]*\}|[^()]+)\)", r"\1"),
    (r"\bCVarGetFloat\(CVAR_\w+\(\"[^\"]*\"\), ([^()]+)\)", r"\1"),
    # OoT's audio and interface calls under MM's names.
    (r"\bfunc_800F4414\(", "Audio_PlaySfx_AtPosWithTimer("),
    (r"\bfunc_800F44EC\(", "Audio_SetSfxTimerLerpInterval("),
    (r"\bAudio_QueueSeqCmd\(0x1 << 28 \| SEQ_PLAYER_FANFARE << 24 \| 0xFF\);", "SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_FANFARE, 0);"),
    (r"\bInterface_SetDoAction\(", "Interface_SetAButtonDoAction("),
    (r"\byDistToWater\b", "depthInWater"),
    (r"\bobjBankIndex\b", "objectSlot"),
    (r"\btargetMode\b", "attentionRangeType"),
    (r"\btargetArrowOffset\b", "lockOnArrowOffset"),
    (r"\bisTargeted\b", "isLockedOn"),
    (r"\bTEXT_STATE_DONE_FADING\b", "TEXT_STATE_FADING"),
    (r"\bTEXT_STATE_DONE_HAS_NEXT\b", "TEXT_STATE_NEXT"),
    (r"\bActor_ProcessTalkRequest\((&?[\w>.-]+), play\)", r"Actor_TalkOfferAccepted(\1, &play->state)"),
    (r"\bSEGMENTED_TO_VIRTUAL\(", "Lib_SegmentedToVirtual("),
    (r"\bQuake_Add\(((?:[^,()]|\([^()]*\))+), (\d)\)", r"Quake_Request(\1, QUAKE_TYPE_\2)"),
    (r"\bQuake_SetQuakeValues\(", "Quake_SetPerturbations("),
    (r"\bWaterBox_GetSurfaceImpl\(", "BgCheck_GetWaterSurfaceNoBgId("),
    # OoT's WaterBox_GetSurface1 adds Zora's Domain's own water box (not in its collision) to the scene's.
    (r"\bWaterBox_GetSurface1\(", "BgCheck_GetWaterSurfaceNoBgId("),
    (r"\bFlags_GetTempClear\(", "Flags_GetClearTemp("),
    (r"\bFlags_SetTempClear\(", "Flags_SetClearTemp("),
    (r"\bPlay_CameraSetFov\(", "Play_SetCameraFov("),
    (r"\bInterface_SetTimer\(", "Interface_StartTimer(TIMER_ID_MINIGAME_1, "),
    (r"\bfunc_800C0DB4\(", "Play_IsUnderwater("),
    (r"\bfunc_800A9F6C\(", "Rumble_Request("),
    (r"\bfunc_800AA148\(\)", "Rumble_ControllerOneHasRumblePak()"),
    (r"\bAudio_PlaySoundRandom\(", "Audio_PlaySfx_Randomized("),
    (r"\bAudio_SetSfxBanksMute\(", "AudioSfx_MuteBanks("),
    # OoT's ocarina games (z_message.c's ports; docs/SONGS.md).
    (r"\bMessage_UpdateOcarinaGame\(", "Message_UpdateOcarinaGameOot("),
    (r"\bgSaveContext\.ocarinaGameRoundNum\b", "gOcarinaGameRoundNumOot"),
    (r"\bMSGMODE_(MEMORY_GAME|FROGS)_(\w+)\b(?!_Oot)", r"MSGMODE_\1_\2_Oot"),
    (r"\bAudio_IsSfxPlaying\(", "AudioSfx_IsPlaying("),
    (r"\bOCARINA_INSTRUMENT_FLUTE\b", "OCARINA_INSTRUMENT_WHISTLING_FLUTE"),
    # OoT's scarecrow songs (docs/SONGS.md W3: one spawn song, MM's): the long song's flag is MM's unused oca_rec_flag.
    (r"\bgSaveContext\.scarecrowLongSongSet\b", "gSaveContext.save.saveInfo.unk_F40"),
    (r"\bgSaveContext\.scarecrowSpawnSongSet\b", "gSaveContext.save.saveInfo.scarecrowSpawnSongSet"),
    (r"\bmsgCtx\.lastOcaNoteIdx\b", "msgCtx.ocarinaButtonIndex"),
    (r"(->|\.)lastOcaNoteIdx\b", r"\1ocarinaButtonIndex"),
    (r"\bENEMY_RANDOMIZER_OFF\s*==\s*ENEMY_RANDOMIZER_OFF\b", "true"),
    # OoT's switch to the nature ambience (the Skull Kids' ocarina game) and back: MM's scene music plays on.
    (r"^([ \t]*)func_800F5BF0\([^;]*\);", r"\1// PORT: OoT switches to the nature ambience here; the scene's music plays on."),
    (r"^([ \t]*)func_800F5C2C\(\);", r"\1// PORT: OoT brings its music back here."),
    (r"\bLOG_HUNGUP_THREAD\(\);", "(void)0; // OoT's debug hang"),  # the same seven banks in both games
    (r"\bGfx_SetupDL_20NoCD\(", "Gfx_SetupDL20_NoCD("),
    (r"\bgMtxClear\b", "gIdentityMtx"),
    # OoT's SkinMatrix_SetRotateYXZ(x, y, z) is MM's SkinMatrix_SetRotateYRP(yaw, roll, pitch), argument for argument.
    (r"\bSkinMatrix_SetTranslateRotateYXZScale\(", "SkinMatrix_SetScaleRotateYRPTranslate("),
    (r"\bfunc_800A63CC\(", "Skin_DrawImpl("),
    (r"\bEffectSsDFire_SpawnFixedScale\(", "EffectSsDFire_SpawnFixedScaleOot("),
    (r"\bEffectSsGMagma(2?)_Spawn\(", r"EffectSsGMagma\1_SpawnOot("),
    (r"\bMatrix_MtxFToZYXRotS\(", "Matrix_MtxFToZYXRot("),
    # OoT Player's fishing line fields and left-hand matrix, as MM's leftover En_Fishing names them.
    (r"\bplayer->unk_858\b", "player->unk_B08"),
    (r"\bplayer->unk_85C\b", "player->unk_B0C"),
    (r"\bplayer->mf_9E0\b", "player->leftHandMf"),
    (r"\binterfaceCtx\.unk_260\b", "interfaceCtx.fishingHUDControl"),
    (r"\bfunc_800433A4\(", "DynaPolyActor_TransformCarriedActor("),
    (r"\bfunc_8003ECA8\(", "DynaPoly_DisableCeilingCollision("),
    (r"\bfunc_8005B198\(\)", "Hyrule_GetAttentionCategoryOot()"),
    (r"\bEffectSsHitMark_", "EffectSsHitmark_"),
    (r"\bAudio_PlaySoundTransposed\(", "Audio_PlaySoundTransposedOot("),
    (r"\bAudio_PlayWindmillBgm\(", "Audio_PlayWindmillBgmOot("),
    (r"\bAudio_StopSfxById\(", "AudioSfx_StopById("),
    # OoT's actor helpers, as MM names them or as HyruleActor.c ports them.
    (r"\bfunc_8002EABC\(", "Hilite_DrawOpa("),
    (r"\bfunc_8002EB44\(", "Hilite_DrawXlu("),
    (r"\bfunc_8002843C\(", "EffectSsDust_SpawnDrawFlag2Oot("),
    (r"\bfunc_80035844\(", "Math_Vec3fDiffToRotOot("),
    (r"\bActor_GetCollidedExplosive\(", "Actor_GetCollidedExplosiveOot("),
    (r"\bActor_SetTextWithPrefix\(", "Actor_SetTextWithPrefixOot("),
    (r"\bEffectSsDeadDd_Spawn\(", "EffectSsDeadDd_SpawnOot("),
    (r"\bEffectSsDeadDs_SpawnStationary\(", "EffectSsDeadDs_SpawnStationaryOot("),
    (r"\bEffectSsFCircle_Spawn\(", "EffectSsFCircle_SpawnOot("),
    (r"\bGfx_SetupDL_60NoCDXlu\(", "Gfx_SetupDL60_XluNoCD("),
    (r"\bfunc_80094E78\(", "Gfx_TexScrollOot("),
    (r"\bPlay_TriggerRespawn\(", "Hyrule_TriggerRespawnOot("),
    (r"\bIS_BOSS_RUSH\b", "false"),
    # OoT's Player fields MM keeps under other names.
    (r"\b(\w*[pP]layer\w*)->unk_450\b", r"\1->unk_3A0"),
    (r"\bPlayer_HasMirrorShieldSetToDraw\(", "Player_HasMirrorShieldSetToDrawOot("),
    (r"\bplayer->unk_860\b", "player->unk_B28"),  # the burning stick's timer
    # OoT's knockback request (MM's func_800B8D10 fills the same fields).
    (r"\bplayer->knockbackDamage\b", "player->unk_B74"),
    (r"\bplayer->knockbackType\b", "player->unk_B75"),
    (r"\bplayer->knockbackRot\b", "player->unk_B76"),
    (r"\bplayer->knockbackSpeed\b", "player->unk_B78"),
    (r"\bplayer->knockbackYVelocity\b", "player->unk_B7C"),
    (r"\bgMtxFClear\b", "gIdentityMtxF"),
    (r"\bfunc_8005ACFC\(", "Camera_SetStateFlag("),
    (r"\bfunc_8005AD1C\(", "Camera_UnsetStateFlag("),
    (r"\bfunc_800F483C\(", "Audio_SetBgmMainVolumeOot("),
    (r"\bfunc_800F4190\(", "Audio_PlaySfx_AtPos("),
    (r"\bfunc_80093C80\(", "Gfx_SetupDL25_OpaRoomOot("),
    (r"\bfunc_80074CE8\(", "Environment_ChangeLightSetting("),
    (r"\bfunc_80041EA4\(", "SurfaceType_GetFloorProperty("),
    (r"\bfunc_8003EE6C\(", "DynaPoly_UpdateBgActorTransforms("),
    (r"\bfunc_800355E4\(", "Actor_IsHitByHammerForwardOot("),
    (r"\bfunc_8003435C\(", "func_800BCC68("),  # an Xlu highlight under the scene's light
    (r"\bfunc_8003424C\(", "CollisionCheck_SpawnShieldParticlesMetal("),
    (r"\bfunc_8002DD6C\(", "func_800B7118("),
    (r"\bfunc_8002DD78\(", "func_800B7128("),
    # MM's z_effect_soft_sprite_old_init.c and z_eff_ss_dead.c keep OoT's helpers in OoT's order.
    (r"\bfunc_8002865C\(", "func_800B11A0("),
    (r"\bfunc_800286CC\(", "func_800B1210("),
    (r"\bfunc_8002873C\(", "func_800B1280("),
    (r"\bfunc_800287AC\(", "func_800B12F0("),
    (r"\bfunc_80028990\(", "func_800B14D4("),
    (r"\bfunc_80028A54\(", "func_800B1598("),
    (r"\bfunc_80026230\(", "func_800AE2A0("),
    (r"\bfunc_80026400\(", "func_800AE434("),
    (r"\bfunc_80026608\(", "func_800AE5A0("),
    (r"\bfunc_80026690\(", "func_800AE5E4("),
    (r"\bfunc_80026860\(", "func_800AE778("),
    (r"\bfunc_80026A6C\(", "func_800AE8EC("),
    (r"\bPlay_CameraSetAtEyeUp\(", "Play_SetCameraAtEyeUp("),
    (r"\bLinkAnimation_Update\(", "PlayerAnimation_Update("),
    (r"\bAnimationContext_SetCopyTrue\(", "AnimTaskQueue_AddCopyUsingMap("),
    (r"\bActor_SetPlayerKnockback\(", "func_800B8D10("),
    (r"\bActor_SwapHookshotAttachment\(", "Actor_SwapHookshotAttachmentOot("),
    (r"\bMath_StepUntilS\(", "Math_StepUntilSOot("),
    (r"\bAudio_PlaySoundIncreasinglyTransposed\(", "Audio_PlaySoundIncreasinglyTransposedOot("),
    (r"\bAudio_ResetIncreasingTranspose\(", "Audio_ResetIncreasingTransposeOot("),
    (r"\bSUBCAM_ACTIVE\b", "CAM_ID_NONE"),
    # MM's shock effect tracks Goht's limbs where OoT's tracks Phantom Ganon's; this one stays where it is spawned.
    (r"\bFHGFLASH_SHOCK_PG\b", "FHGFLASH_SHOCK_NO_ACTOR"),
    (r"\bPLAYER_BOOTS_(IRON|HOVER)\b", r"PLAYER_BOOTS_\1_Oot"),
    (r"\bResourceGetDataByName\(", "(u8*)ResourceMgr_LoadTexOrDListByName("),
    (r"(\.|->)childCamIdx\b", r"\1childCamId"),
    # OoT's Biggoron's Sword flag is 3Ship's owned Biggoron's Sword (HyruleInventory.c gives it with the item).
    (r"^([ \t]*)gSaveContext\.bgsFlag = \w+;", r"\1// PORT: giving ITEM_SWORD_BGS_Oot records Biggoron's Sword as owned (OoT's bgsFlag)."),
    (r"\bgSaveContext\.bgsFlag\b", "CHECK_OWNED_EQUIP_OOT(EQUIP_TYPE_SWORD, EQUIP_OWNED_SWORD_BIGGORON)"),
    (r"\bEnvironment_GetBgsDayCount\(", "Environment_GetBgsDayCountOot("),
    # OoT's custom lens flare is MM's first (z_kankyo.c), with OoT's scale, intensity and screen fill as MM's.
    (r"\bgCustomLensFlareOn\b", "gCustomLensFlare1On"),
    (r"\bgCustomLensFlarePos\b", "gCustomLensFlare1Pos"),
    (r"\bgLensFlareScale\b", "D_801F4E44"),
    (r"\bgLensFlareColorIntensity\b", "D_801F4E48"),
    (r"\bgLensFlareScreenFillAlpha\b", "D_801F4E4C"),
    (r"\bgMorphaTransposeTable\b", "gMorphaTransposeTableOot"),
    (r"\bEnvironment_ClearBgsDayCount\(", "Environment_ClearBgsDayCountOot("),
    # The flag macros, whatever their argument.
    (r"\b(GET|SET|CLEAR)_(ITEMGETINF|INFTABLE|EVENTCHKINF)\((?=(?:ITEMGETINF|INFTABLE|EVENTCHKINF)_\w+\()", r"\1_\2_OOT("),
    (r"\bplayer->unk_6A8\b", "player->autoLockOnActor"),
    (r"\bplayer->unk_A73\b", "player->unk_D57"),  # set to 4 when Link fires an arrow or throws the boomerang
    (r"\benvCtx\.unk_D6\b", "envCtx.lightBlendRateOverride"),
    (r"(\.|->)dyna\.unk_154\b", r"\1dyna.unk14C"),
    # OoT's collider elements name their element info `info`, MM's `elem`.
    (r"(\.|->)info\.(?=(?:at|ac|oc)\w*|elemType|bumper|toucher)", r"\1elem."),
    (r"(\b\w*[cC]ollider\w*(?:\[[^\]]*\])?|\belements\[[^\]]*\])\.info\b", r"\1.elem"),
    (r"\bplay->isPlayerDroppingFish\(play\)", "Player_IsDroppingFishOot(play)"),
    (r"\bEffectSsFhgFlash_SpawnLightBall\(", "EffectSsFhgFlash_SpawnLightBallOot("),
    (r"\bAudio_StopSfxByPos\(", "AudioSfx_StopByPos("),
    (r"\bAudio_QueueSeqCmd\(", "AudioSeq_QueueSeqCmd("),
    (r"\bActor_UpdateVelocityXYZ\(", "Actor_UpdateVelocityWithoutGravity("),
    (r"\bRADF_TO_BINANG\(", "RAD_TO_BINANG("),
    (r"\bplay->activeCamera\b", "play->activeCamId"),
    (r"\bPlay_TriggerVoidOut\(", "func_80169FDC("),
    (r"\bfunc_800C08AC\(", "func_80169AFC("),  # return to the main camera
    (r"\bMATRIX_CHECKFLOATS\(([^()]*)\)", r"\1"),
    (r"\bInterface_SetSubTimer\(", "Interface_StartTimer(TIMER_ID_MINIGAME_2, "),
    # OoT's manual cutscenes run in OoT's cutscene context, which MM's mirrors (HyruleCutscene.c).
    (r"\bfunc_80064520\(play, &play->csCtx\)", "Cutscene_StartManualOot(play)"),
    (r"\bfunc_80064534\(play, &play->csCtx\)", "Cutscene_StopManualOot(play)"),
    (r"\bOnePointCutscene_AttentionSetSfx\(", "OnePointCutscene_AttentionSetSfxOot("),
    (r"\bCollisionCheck_GetSwordDamage\(", "CollisionCheck_GetSwordDamageOot("),
    (r"\bPlayer_GetStrength\(\)", "Player_GetStrengthOot()"),
    (r"\bgSaveContext\.inventory\.gsTokens\b", "gSaveContext.save.saveInfo.inventoryOot.gsTokens"),
    # OoT's Vec2f has x and y where MM's has x and z.
    (r"\bVec2f\b", "Vec2fOot"),
    # OoT's curve skeletons are MM's under other names.
    (r"\bSkelAnimeCurve\b", "SkelCurve"),
    (r"\bSkelCurveLimbList\b", "CurveSkeletonHeader"),
    (r"\bTransformUpdateIndex\b", "CurveAnimationHeader"),
    (r"(\.|->)animCurFrame\b", r"\1curFrame"),
    # SoH's names for vanilla texts.
    (r"\bTEXT_GS_(NO_)?FREEZE\b", "TEXT_OOT(0xB4)"),
    # OoT's "play for an actor": the actor asks (NEAR_OCARINA_ACTOR) and Player answers when Link takes out the
    # ocarina (ATTEMPT_PLAY_FOR_ACTOR); MM's actor offers itself and is accepted (HyruleOcarina.c). The actor is
    # first in `this`.
    (r"\b\w+->stateFlags2 \|= PLAYER_STATE2_NEAR_OCARINA_ACTOR;", "Hyrule_OfferOcarinaOot(play, (Actor*)this);"),
    (r"\b\w+->stateFlags2 & PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR\b", "Hyrule_OcarinaAttemptedOot(play, (Actor*)this)"),
    (r"\b\w+->stateFlags2 &= ~PLAYER_STATE2_ATTEMPT_PLAY_FOR_ACTOR;",
     "// PORT: MM takes an accepted ocarina offer back by itself (OoT's ATTEMPT_PLAY_FOR_ACTOR)."),
    # OoT's collectibles MM lacks, as Item00_FromOot maps them.
    (r"\bITEM00_SHIELD_(DEKU|HYLIAN)\b", "ITEM00_SHIELD_HERO"),
    (r"\bITEM00_TUNIC_(ZORA|GORON)\b", "ITEM00_NOTHING"),
    (r"\bITEM00_MAGIC_SMALL\b", "ITEM00_MAGIC_JAR_SMALL"),
    (r"\bITEM00_MAGIC_LARGE\b", "ITEM00_MAGIC_JAR_BIG"),
    (r"\bITEM00_ARROWS_(SINGLE|SMALL|MEDIUM)\b", "ITEM00_ARROWS_10"),
    (r"\bITEM00_ARROWS_LARGE\b", "ITEM00_ARROWS_30"),
    (r"\bITEM00_BOMBS_SPECIAL\b", "ITEM00_BOMBS_A"),
    (r"\bAudio_SetFastTempoForTimedMinigame\(", "Audio_SetFastTempoForTimedMinigameOot("),
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
    (r"\b(actor|thisx|rideActor|playerx)(\.|->)speedXZ\b", r"\1\2speed"),
    (r"->linearVelocity\b", "->speedXZ"),  # Player's
    (r"\byDistToPlayer\b", "playerHeightRel"),
    # MM's Tatl hints do not cover OoT enemies: init chains keep their shape with no hint.
    (r"\bICHAIN_S8\(naviEnemyId, (\w+), (ICHAIN_\w+)\),",
     r"ICHAIN_U8(hintId, TATL_HINT_ID_NONE, \2), // PORT: OoT's Navi enemy info \1"),
    (r"^([ \t]*)(?:this->actor\.|thisx->)naviEnemyId = ([^;]+);(?: //)?", r"\1// PORT: OoT gives Navi enemy info \2;"),
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
    (r"\bPLAYER_STATE2_DIVING\b", "PLAYER_STATE2_800"),
    (r"\bACTORCAT_EXPLOSIVE\b", "ACTORCAT_EXPLOSIVES"),
    (r"\bPLAYER_STATE1_IN_CUTSCENE\b", "PLAYER_STATE1_20000000"),
    (r"\bPLAYER_STATE2_DISABLE_DRAW\b", "PLAYER_STATE2_20000000"),
    # SoH's colour scheme option (the A button's colour) reads as N64's.
    (r"\bCOLORSCHEME_N64 == COLORSCHEME_GAMECUBE\b", "false"),
    (r"\bPLAYER_STATE1_HANGING_OFF_LEDGE\b", "PLAYER_STATE1_2000"),
    (r"\bPLAYER_STATE1_CLIMBING_LEDGE\b", "PLAYER_STATE1_4000"),
    (r"\bPLAYER_STATE1_JUMPING\b", "PLAYER_STATE1_40000"),
    (r"\bPLAYER_STATE1_FREEFALL\b", "PLAYER_STATE1_80000"),
    # OoT's sword combo counters follow meleeWeaponState, as MM's do; the age property 0x92 is Link's voice offset.
    (r"->unk_844\b", "->unk_ADC"),
    (r"->unk_845\b", "->unk_ADD"),
    (r"(ageProperties->)unk_92\b", r"\1voiceSfxIdOffset"),
    (r"\bPlayerGrounded\((\w+)\)", r"(\1->actor.bgCheckFlags & BGCHECKFLAG_GROUND)"),
    (r"\bPLAYER_STATE1_GETTING_ITEM\b", "PLAYER_STATE1_400"),
    (r"\bPLAYER_STATE1_FIRST_PERSON\b", "PLAYER_STATE1_100000"),
    (r"\bPLAYER_STATE2_NAVI_ACTIVE\b", "PLAYER_STATE2_100000"),
    (r"\bPLAYER_STATE2_NAVI_ALERT\b", "PLAYER_STATE2_200000"),
    # OoT's TargetContext is MM's Attention, field for field; Navi's fields are Tatl's.
    (r"\bactorCtx\.targetCtx\.naviRefPos\b", "actorCtx.attention.tatlHoverPos"),
    (r"\bactorCtx\.targetCtx\.targetCenterPos\b", "actorCtx.attention.reticlePos"),
    (r"\bactorCtx\.targetCtx\.naviInner\b", "actorCtx.attention.tatlInnerColor"),
    (r"\bactorCtx\.targetCtx\.naviOuter\b", "actorCtx.attention.tatlOuterColor"),
    (r"\bactorCtx\.targetCtx\.arrowPointedActor\b", "actorCtx.attention.tatlHoverActor"),
    (r"\bactorCtx\.targetCtx\.targetedActor\b", "actorCtx.attention.reticleActor"),
    (r"\bactorCtx\.targetCtx\.unk_40\b", "actorCtx.attention.tatlMoveProgressFactor"),
    (r"\bactorCtx\.targetCtx\.bgmEnemy\b", "actorCtx.attention.bgmEnemy"),
    # Navi's C-up text is MM's Player's Tatl text.
    (r"->naviTextId\b", "->tatlTextId"),
    # Navi is Player's fairy actor while she is Link's (hyrule/README.md, Cutscenes).
    (r"->naviActor\b", "->tatlActor"),
    # SoH keeps Navi's hint timer in the save; 3Ship does not save it yet.
    (r"\bgSaveContext\.naviTimer\b", "gNaviTimerOot"),
    (r"\bfunc_800F4524\(", "Audio_PlaySfx_AtPosWithReverb("),
    (r"\bfunc_800F3F3C\(", "Hyrule_PlayCutsceneEffectsSeqOot("),
    (r"\bfunc_8002873C\(", "func_800B1280("),
    (r"\bMATRIX_TOMTX\(", "Matrix_ToMtx("),
    (r"\bAnimationContext_SetMoveActor\(", "AnimTaskQueue_AddActorMovement("),
    # OoT's footstep sfx type is an offset from NA_SE_PL_WALK_GROUND, as MM's is from its own table.
    (r"\bSurfaceType_GetSfx\(", "SurfaceType_GetSfxOffset("),
    # Cutscene data: OoT's terminator destinations (HyruleCutscene.h).
    (r"CS_TERMINATOR\(([A-Z0-9_]+?)(?:_Oot)?,", r"CS_TERMINATOR(\1_Oot,"),
    # OoT's cutscene context through a local pointer is OoT's (HyruleCutscene.h).
    (r"\bCutsceneContext\* (\w+) = &play->csCtx;", r"CutsceneContextOot* \1 = &gCsCtxOot;"),
    # OoT's boots, owned in the OoT inventory (HyruleInventory.h).
    (r"\bMagic_Fill\(play\)", "Magic_Add(play, MAGIC_FILL_TO_CAPACITY)"),
    # OoT's scene camera type (YREG(15)) and prerendered viewpoint (HyruleScenes.h).
    (r"\bYREG\(15\)", "Hyrule_GetCameraMovementOot()"),
    (r"\bPlay_CheckViewpoint\(play, ", "Hyrule_CheckViewpointOot("),
    (r"\bPlay_SetViewpoint\(", "Hyrule_SetViewpoint("),
    (r"\bPlay_SetShopBrowsingViewpoint\(", "Hyrule_SetShopBrowsingViewpoint("),
    (r"\bPHYSICAL_TO_VIRTUAL\(", "OS_K0_TO_PHYSICAL("),
    (r"\bActor_Find\(", "Actor_FindOot("),
    (r"\bfunc_800849EC\(play\)", "Hyrule_GiveGiantsKnife()"),
    (r"\bGfx_SetupDL_39Overlay\(", "Gfx_SetupDL39_Overlay("),
    (r"\b(GET|SET|CLEAR)_(ITEMGETINF|INFTABLE|EVENTCHKINF)\((\w+)\)", r"\1_\2_OOT(\3)"),
    (r"\bVIRTUAL_TO_PHYSICAL\(", "OS_K0_TO_PHYSICAL("),
    (r"\bGraph_Alloc\(", "GRAPH_ALLOC("),
    # Matrix modes written as numbers (MTXMODE_NEW 0, MTXMODE_APPLY 1).
    (r"\bMatrix_Rotate([XYZ])\(([^;]*),\s*1\);", r"Matrix_Rotate\1(\2, MTXMODE_APPLY);"),
    (r"\bMatrix_Rotate([XYZ])\(([^;]*),\s*0\);", r"Matrix_Rotate\1(\2, MTXMODE_NEW);"),
    (r"\bMatrix_RotateX\(([^;]*),\s*MTXMODE_APPLY\);", r"Matrix_RotateXFApply(\1);"),
    (r"\bMatrix_RotateX\(([^;]*),\s*MTXMODE_NEW\);", r"Matrix_RotateXFNew(\1);"),
    (r"\bMatrix_RotateY\(([^;]*),\s*(MTXMODE_\w+)\);", r"Matrix_RotateYF(\1, \2);"),
    (r"\bMatrix_RotateZ\(([^;]*),\s*(MTXMODE_\w+)\);", r"Matrix_RotateZF(\1, \2);"),
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
    # OoT's Object_Spawn loads an object into the next slot: MM's Object_SpawnPersistent.
    (r"\bObject_Spawn\(", "Object_SpawnPersistent("),
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
    (r"\bGfx_SetupDL_(\d+)\(", r"Gfx_SetupDL\1("),
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
    (r"\bgSaveContext\.(health|healthCapacity|magic|magicLevel|isMagicAcquired|isDoubleMagicAcquired)\b",
     r"gSaveContext.save.saveInfo.playerData.\1"),
    (r"\bgSaveContext\.swordHealth\b", "gSaveContext.save.saveInfo.inventoryOot.knifeHealth"),
    (r"\bUPG_NUTS\b", "UPG_DEKU_NUTS"),
    (r"\bgSaveContext\.isDoubleDefenseAcquired\b", "gSaveContext.save.saveInfo.playerData.doubleDefense"),
    (r"\bgSaveContext\.inventory\.defenseHearts\b", "gSaveContext.save.saveInfo.inventory.defenseHearts"),
    # OoT's small keys, by OoT's map index (HyruleInventory.c).
    (r"\bgSaveContext\.inventory\.dungeonKeys\[gSaveContext\.mapIndex\]",
     "gSaveContext.save.saveInfo.inventoryOot.dungeonKeys[Inventory_GetDungeonIndexOot(play)]"),
    (r"\bactorCtx\.flags\.chest\b", "actorCtx.sceneFlags.chest"),
    (r"\bCHECK_DUNGEON_ITEM\((\w+), gSaveContext\.mapIndex\)",
     r"CHECK_DUNGEON_ITEM_OOT(\1, Inventory_GetDungeonIndexOot(play))"),
    (r"\bplay->transiActorCtx\b", "play->transitionActors"),
    (r"\bcv\.slidingDoorBgCamIndex\b", "cv.doorBgCamIndex"),
    (r"\bDOORLOCK_NORMAL_SPIRIT\b", "DOORLOCK_2"),
    (r"\bDUNGEON_KEY_BOSS\b", "DUNGEON_BOSS_KEY"),
    # A door that offers itself to Player queues MM's door cutscene, which MM's Player waits for, as MM's doors do.
    (r"^([ \t]*)(\w+)->doorActor = ([^;\n]+);[ \t]*$",
     r"\1\2->doorActor = \3;\n\1// 3Ship [Hyrule] MM's Player opens a door only once the door has queued the door "
     r"cutscene, as MM's doors\n\1// do here.\n\1func_80122F28(\2);"),
    # MM's chest fanfare is OoT's (both 0x2B); OoT's sequence ids cannot carry the sequence arguments it is played with.
    (r"\bNA_BGM_OPEN_TRE_BOX\b", "NA_BGM_OPEN_CHEST"),
    (r"\benvCtx\.unk_BF\b", "envCtx.lightSettingOverride"),
    # OoT's indoor light fields, MM's light settings (OoT's layout runs 4 bytes shorter there).
    (r"\benvCtx(\.|->)unk_BD\b", r"envCtx\1lightSetting"),
    (r"\benvCtx(\.|->)unk_BE\b", r"envCtx\1prevLightSetting"),
    (r"\benvCtx(\.|->)unk_D8\b", r"envCtx\1lightBlend"),
    (r"\benvCtx(\.|->)unk_DC\b", r"envCtx\1lightBlendOverride"),
    (r"\bMAX_HEALTH\b", "(20 * 0x10)"),
    (r"\bSUBCAM_NONE\b", "CAM_ID_NONE"),
    # OoT's camera settings that the exporter maps to MM's (OotToMm.cpp).
    (r"\bCAM_SET_CHU_BOWLING\b", "CAM_SET_FIXED1"),
    (r"\bSUBCAM_FREE\b", "SUB_CAM_ID_DONE"),
    (r"\bWALL_FLAG_CRAWLSPACE\b", "(WALL_FLAG_4 | WALL_FLAG_5)"),
    # Bombs are MM's En_Bom.
    (r"\bBOMB_BODY\b", "BOMB_TYPE_BODY"),
    (r"\bBOMB_EXPLOSION\b", "BOMB_TYPE_EXPLOSION"),
    (r'#include "overlays/effects/ovl_Effect_Ss_HitMark/z_eff_ss_hitmark\.h"',
     '#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"'),
    (r"\bUPG_STICKS\b", "UPG_DEKU_STICKS"),
    (r"\bCAM_STAT_(\w+)", r"CAM_STATUS_\1"),
    (r"\bPlay_CameraSetAtEye\(", "Play_SetCameraAtEye("),
    (r"\bfunc_8005B1A4\(", "Camera_SetFinishedFlag("),
    # OoT's skin draw without an override limb draw (horses).
    (r"\bfunc_800A6330\(", "func_80138228("),
    # OoT's item camera settings, by their camera data names ("ITEM0", "ITEM2").
    (r"\bCAM_SET_SLOW_CHEST_CS\b", "CAM_SET_ITEM0"),
    (r"\bCAM_SET_TURN_AROUND\b", "CAM_SET_ITEM2"),
    # OoT's minigame states and timer. MM keeps OoT's shooting gallery status (the B button's ammo plus one) as
    # bButtonAmmoPlusOne, and its Bombchu Bowling status as Honey and Darling's unk_1887D; MM's timers count
    # centiseconds.
    (r"\bplay->shootingGalleryStatus\b", "play->bButtonAmmoPlusOne"),
    (r"\bfunc_8008EF44\(", "Player_SetBButtonAmmo("),
    (r"\bplay->bombchuBowlingStatus\b", "play->unk_1887D"),
    (r"\bgSaveContext\.timerState\b", "gSaveContext.timerStates[TIMER_ID_MINIGAME_1]"),
    # OoT's second timer (the trade items' and other timers) is MM's second minigame timer.
    (r"\bgSaveContext\.subTimerState\b", "gSaveContext.timerStates[TIMER_ID_MINIGAME_2]"),
    # Written, OoT's timer seconds are MM's minigame timer in centiseconds (En_Ma3 reads them through a pointer).
    (r"^[ \t]*s16\* timerSecondsPtr;[^\n]*\n", ""),
    (r"^[ \t]*timerSecondsPtr = &gSaveContext\.timerSeconds;\n", ""),
    (r"\*timerSecondsPtr\b", "gSaveContext.timerSeconds"),
    (r"\bgSaveContext\.(sub)?[tT]imerSeconds = ([^;]+);",
     lambda m: "gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_%d] = SECONDS_TO_TIMER(%s);" % (2 if m.group(1) else 1, m.group(2))),
    (r"\bgSaveContext\.subTimerSeconds\b", "((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_2] / 100))"),
    (r"\bSUBTIMER_STATE_OFF\b", "TIMER_STATE_OFF"),
    (r"\bTIMER_STATE_UP_FREEZE\b", "TIMER_STATE_STOP"),
    (r"\bgSaveContext\.timerSeconds\b", "((s32)(gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] / 100))"),
    (r"\bITEM00_RUPEE_ORANGE\b", "ITEM00_RUPEE_HUGE"),
    (r"\bplay->nextEntranceIndex = gSaveContext\.entranceIndex;", "play->nextEntrance = gSaveContext.save.entrance;"),
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
    # OoT's circle transitions are not MM's (whose circle does not open again here): fades of the same color and speed,
    # as for OoT's entrances (gen_oot_scenes.py).
    (r"\bTRANS_TYPE_CIRCLE\(\s*\w+\s*,\s*TCC_(BLACK|WHITE)\s*,\s*TCS_(FAST|SLOW)\s*\)", r"TRANS_TYPE_FADE_\1_\2"),
    # OoT's specular highlight setup for opaque and translucent display lists.
    (r"\bfunc_8002EBCC\b", "func_800B8050"),
    (r"\bfunc_8002ED80\b", "func_800B8118"),
    (r"(atDmgInfo\.dmgFlags & )(0x[0-9A-Fa-f]+)\b", r"\1DMG_FROM_OOT(\2)"),
    (r"\b(?:Player_SetCsActionWithHaltedActors|func_8002DF54)\(", "Player_SetCsActionWithHaltedActorsOot("),
    (r"\bplay->startPlayerCutscene\(play, ", "Player_TryCsActionOot(play, "),
    (r"\bplay->linkAgeOnLoad\b", "gLinkAgeOnLoadOot"),
    (r"\bEnBoom\b", "EnBoomOot"),  # OoT's boomerang (hyrule/overlays/actors/ovl_En_Boom), apart from MM's Zora fins
    (r"\.skyboxOffset\b", ".quakeOffset"),  # OoT's Camera field at the same offset, the quake's eye offset
    (r"\b(play|playState)->csCtx\.(npcActions|frames|state|segment|linkAction|unk_1[8AB])\b", r"gCsCtxOot.\2"),
    (r"(\w+) = &play->csCtx;", r"\1 = &gCsCtxOot;"),
    (r"\bCutsceneContext\* (\w+);", r"CutsceneContextOot* \1;"),
    (r"->toucherFlags\b", "->atElemFlags"),
    (r"->bumperFlags\b", "->acElemFlags"),
    (r"^([ \t]*)Audio_(?:Toggle|Update)MalonSinging\([^;]*\);",
     r"\1// PORT: OoT's Malon singing in Lon Lon Ranch's music is not ported."),
    (r"\bPLAYER_IA_SWORD_MASTER\b", "PLAYER_IA_SWORD_GILDED"),  # the Master Sword is the Gilded Sword's action
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
    (r"\bACTOR_EN_BOM_Oot\b", "ACTOR_EN_BOM"),
    (r"\bACTOR_EN_HORSE_Oot\b", "ACTOR_EN_HORSE"),  # Epona is MM's horse actor in both worlds (docs/EPONA.md)
    # OoT's ocarina effects are MM's actors of the same names (MM's Oceff_Wipe4 is the same swirl, for other songs).
    (r"\bACTOR_OCEFF_(SPOT|WIPE|WIPE2|WIPE3|WIPE4|STORM)_Oot\b", r"ACTOR_OCEFF_\1"),
    # Collectibles are MM's En_Item00 (OoT-only types through Item00_FromOot), arrows MM's En_Arrow, whose thrown Deku
    # Nut stands in for OoT's cutscene nut.
    (r"\bACTOR_EN_ITEM00_Oot\b", "ACTOR_EN_ITEM00"),
    (r"\bACTOR_EN_ARROW_Oot\b", "ACTOR_EN_ARROW"),
    (r"\bARROW_CS_NUT\b", "ARROW_TYPE_DEKU_NUT"),
    (r"\b(OBJECT_(?!GAMEPLAY_KEEP\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\bOBJECT_GAMEPLAY_KEEP\b", "GAMEPLAY_KEEP"),
    (r"\b(SCENE_[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    (r"\b(ITEM_(?!NONE\b)[A-Z0-9_]+)\b(?!_Oot)", r"\1_Oot"),
    # MM's empty bottle: MM keeps the bottles OoT and MM share.
    (r"\bPlayer_UpdateBottleHeld\((\w+), ([^,]+), ITEM_BOTTLE_Oot, PLAYER_IA_BOTTLE\)",
     r"Player_UpdateBottleHeld(\1, \2, ITEM_BOTTLE, PLAYER_IA_BOTTLE_EMPTY)"),
]


def _generic_renames():
    """OoT names MM lacks, by value: Player state flags by their bit (MM names bits by value), and camera settings by
    the exporter's own OoT-to-MM table (OTRExporter's OotToMm.cpp), so actors' settings match their scenes'."""
    soh = os.path.join(HERE, "..", "..", "..", "Shipwright", "soh", "include")
    mm = os.path.join(HERE, "..", "..", "mm", "include")
    out = []
    mm_flags = set(re.findall(r"#define (PLAYER_STATE[123]_\w+)", open(os.path.join(mm, "z64player.h")).read()))
    for name, n, bit in re.findall(r"#define (PLAYER_STATE([123])_\w+) \(1u? << (\d+)\)",
                                   open(os.path.join(soh, "z64player.h")).read()):
        mm_name = "PLAYER_STATE%s_%X" % (n, 1 << int(bit))
        if name not in mm_flags and mm_name in mm_flags:
            out.append((r"\b%s\b" % name, mm_name))
    oot_sets = re.findall(r"/\* (0x[0-9A-F]+) \*/ (CAM_SET_\w+)", open(os.path.join(soh, "z64camera.h")).read())
    mm_sets = dict((int(i, 16), n) for i, n in re.findall(r"/\* (0x[0-9A-F]+) \*/ (CAM_SET_\w+)",
                                                         open(os.path.join(mm, "z64camera.h")).read()))
    mm_names = set(mm_sets.values())
    table = dict((int(i, 16), int(m, 16)) for i, m in re.findall(
        r"/\* (0x[0-9A-F]+) [\w ()]+\*/ (0x[0-9A-F]+),",
        open(os.path.join(HERE, "..", "..", "OTRExporter", "OTRExporter", "OotToMm.cpp")).read()))
    for i, name in oot_sets:
        i = int(i, 16)
        if name not in mm_names and i in table and table[i] in mm_sets:
            out.append((r"\b%s\b" % name, mm_sets[table[i]]))
    # OoT's damage flags by name, as the OoT mask they stand for (DMG_FROM_OOT, as for OoT's masks in code).
    defs = dict(re.findall(r"#define (DMG_\w+) +\(([^)]*)\)", open(os.path.join(soh, "z64collision_check.h")).read()))
    values = {}

    def value(name):
        if name not in values:
            expr = defs[name]
            m = re.fullmatch(r"\s*1 << (0x[0-9A-Fa-f]+|\d+)\s*", expr)
            values[name] = (1 << int(m.group(1), 0)) if m else 0
            if not m:
                for part in expr.split("|"):
                    values[name] |= value(part.strip())
        return values[name]

    for name in defs:
        if name.startswith(("DMG_ENTRY", "DMG_TABLE", "DMG_FROM")):
            continue
        try:
            out.append((r"\b%s\b(?!\()" % name, "DMG_FROM_OOT(0x%08X)" % value(name)))
        except (KeyError, AttributeError):
            pass
    return out


RENAMES += _generic_renames()
# Lines that are SoH-only and dropped when they stand alone.
DROP_LINE = re.compile(r"^\s*#include \"(vt\.h|soh_assets\.h|soh/[^\"]+|overlays/effects/ovl_Effect_Ss_Dead_Sound/[^\"]+)\"\s*$|"
                       r"^\s*#include <libultraship/bridge/[^>]+>\s*$|"
                       # OoT's actors declare Object_Spawn themselves; MM's is in z64scene.h.
                       r"^\s*(static )?s32 Object_Spawn\(ObjectContext\* \w+, s16 \w+\);\s*$|"
                       # SoH's randomizer item entries.
                       r"^\s*(/\*[^*]*\*/ )?GetItemEntry getItemEntry( = \(GetItemEntry\)(GI_NONE|GET_ITEM_NONE))?;\s*$|"
                       r"^\s*this->getItemEntry = \(GetItemEntry\)(GI_NONE|GET_ITEM_NONE);\s*$|"
                       r"^\s*\w+\.trailType = TRAIL_TYPE_\w+;\s*$|"
                       r"^\s*Flags_SetRandomizerInf\([^;]*\);\s*$|"
                       # SoH's record of a sale for its randomizer.
                       r"^\s*gSaveContext\.ship\.pendingSale\w* = [^;]*;\s*$|"
                       r"^\s*GetItemEntry \w+ = ItemTable_Retrieve\([^;]*\);\s*$|"
                       # SoH's interpolation and texture-blending hints.
                       r"^\s*(FrameInterpolation_DontInterpolateChild|Gfx_RegisterBlendedTexture)\([^;]*\);\s*$")
TEXT_ID = re.compile(r"\b(0x[1-7][0-9A-Fa-f]{3})\b")
# Lines that give text ids: returns, cases, textId assignments, text calls and a GetTextId's `ret`. A statement that
# goes on to the next lines (a multi-line ternary) is converted to its end.
TEXT_LINE = re.compile(r"\breturn\b|\bcase\b|textId\b|\bret = |[Tt]ext\w* = |SetText\w*\(|SetTalkAction\(|"
                       r"Message_(?:Start|Continue)Textbox\(|TextIds?\[\w*\] = ")
# An array of text ids, whose short values are OoT's texts below 0x1000 too.
TEXT_ID_ARRAY = re.compile(r"TextIds?\[\w*\] = \{")
# OoT's texts below 0x1000 (item texts, shop texts), where a value can only be a text id.
SHORT_TEXT_ID = re.compile(r"(Message_(?:Start|Continue)Textbox\(\s*\w+,\s*|[Tt]extId = )(0x[0-9A-Fa-f]{1,3})\b")
PROFILE = re.compile(
    r"const ActorInit (\w+)_InitVars = \{\s*(\w+),\s*(\w+),\s*(\w+),\s*(\w+),\s*sizeof\((\w+)\),\s*"
    r"(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),\s*(?:\(ActorFunc\))?(\w+),"
    r"(?:\s*(?:\(ActorResetFunc\))?(\w+),?)?\s*\};")


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


# OoT's equipment (CHECK_OWNED_EQUIP with EQUIP_INV_*) is InventoryOot.equipmentOwned (HyruleInventory.h).
EQUIP_TYPES_OOT = {"SWORD": "0", "SHIELD": "1", "TUNIC": "2", "BOOTS": "EQUIP_TYPE_BOOTS_Oot"}
EQUIP_OWNED_OOT = {
    "SWORD_KOKIRI": "EQUIP_OWNED_SWORD_KOKIRI", "SWORD_MASTER": "EQUIP_OWNED_SWORD_MASTER",
    "SWORD_BIGGORON": "EQUIP_OWNED_SWORD_BIGGORON", "SWORD_BROKENGIANTKNIFE": "EQUIP_OWNED_SWORD_KNIFE_BROKEN",
    "SHIELD_DEKU": "EQUIP_OWNED_SHIELD_DEKU", "SHIELD_HYLIAN": "EQUIP_OWNED_SHIELD_HYLIAN",
    "SHIELD_MIRROR": "EQUIP_OWNED_SHIELD_MIRROR_OOT",
    "TUNIC_KOKIRI": "EQUIP_OWNED_KOKIRI", "TUNIC_GORON": "EQUIP_OWNED_GORON_IRON", "TUNIC_ZORA": "EQUIP_OWNED_ZORA_HOVER",
    "BOOTS_KOKIRI": "EQUIP_OWNED_KOKIRI", "BOOTS_IRON": "EQUIP_OWNED_GORON_IRON", "BOOTS_HOVER": "EQUIP_OWNED_ZORA_HOVER",
}


def convert_equipment(text):
    return re.sub(r"\bCHECK_OWNED_EQUIP(?:_ALT)?\(EQUIP_TYPE_(\w+), EQUIP_INV_(\w+)\)",
                  lambda m: f"CHECK_OWNED_EQUIP_OOT({EQUIP_TYPES_OOT[m.group(1)]}, {EQUIP_OWNED_OOT[m.group(2)]})", text)


def oot_sequences(soh_root):
    """OoT's music ids by name, from SoH's sequence.h."""
    text = open(os.path.join(soh_root, "include", "sequence.h")).read()
    return {name: int(value, 16) for name, value in re.findall(r"#define (NA_BGM_\w+) (0x[0-9A-Fa-f]+)", text)}


SEQUENCES = {}
# MM's music ids by name (mm/include/sequence.h).
MM_SEQUENCES = set(re.findall(r"#define (NA_BGM_\w+) ", open(os.path.join(HERE, "..", "..", "mm", "include", "sequence.h")).read()))


def convert_entrances(text):
    """OoT exits by entrance name become MM entrances through HyruleScenes.h (an exit into a scene that is not
    loaded yet leads back to the spawn Link came in at), and checks of the entrance Link came in at compare MM's."""
    text = re.sub(r"play->nextEntranceIndex = (ENTR_\w+);",
                  lambda m: f"play->nextEntrance = Hyrule_ExitFromOot(0x{ENTRANCES[m.group(1)]:03X}); // OoT's {m.group(1)}",
                  text)
    text = re.sub(r"gSaveContext\.entranceIndex ([!=]=)\s+(ENTR_\w+)",
                  lambda m: f"gSaveContext.save.entrance {m.group(1)} Hyrule_EntranceFromOot(0x{ENTRANCES[m.group(2)]:03X} /* {m.group(2)} */)",
                  text)
    # Exits taken from a variable (a table of OoT entrance indices): the remaining OoT names become their indices.
    text = re.sub(r"play->nextEntranceIndex = ([^;]+);", r"play->nextEntrance = Hyrule_ExitFromOot(\1);", text)
    return re.sub(r"\b(ENTR_\w+)\b",
                  lambda m: f"0x{ENTRANCES[m.group(1)]:03X} /* {m.group(1)} */" if m.group(1) in ENTRANCES else m.group(0),
                  text)


def strip_rando_ifs(text):
    """SoH's randomizer-only blocks without an else (if (IS_RANDO) { ... }, no nested braces) are left out, as are
    randomizer-only terms of conditions and SoH's GameInteractor event calls."""
    text = re.sub(r"\s*&&\s*!(?:\(IS_RANDO\)|IS_RANDO\b)", "", text)
    text = re.sub(r"\n[ \t]*GameInteractor_Execute\w*\([^;]*\);", "", text)
    return re.sub(r"\n[ \t]*if \(IS_RANDO\) \{\n[^{}]*?\n[ \t]*\}(?=\n)(?![ \t]*else)", "", text)


def PORTED_ACTOR(ovl):
    """Whether an OoT actor is ported (or being ported) under hyrule/overlays/actors."""
    return os.path.isdir(os.path.join(OUT_ROOT, ovl)) or ovl in PORTING


PORTING = set()


COLLIDER_ELEMENT_NAMES = set()


def fix_title_card_args(text):
    """OoT's TitleCard_InitBossName takes the PlayState (MM's the GameState); SoH adds an eighth argument."""
    out, i = [], 0
    while True:
        j = text.find("TitleCard_InitBossName(", i)
        if j < 0:
            break
        k = j + len("TitleCard_InitBossName(")
        depth, commas = 1, []
        while depth:
            c = text[k]
            depth += (c == "(") - (c == ")")
            if c == "," and depth == 1:
                commas.append(k)
            k += 1
        call = text[j:k]
        if len(commas) == 7:
            call = text[j:commas[6]] + ")"
        call = call.replace("TitleCard_InitBossName(play,", "TitleCard_InitBossName(&play->state,", 1)
        out.append(text[i:j] + call)
        i = k
    return "".join(out) + text[i:]


def drop_camera_data_last_arg(text):
    """SoH's Camera_SetCameraData has a seventh, unused argument that MM's lacks."""
    out, i = [], 0
    while True:
        j = text.find("Camera_SetCameraData(", i)
        if j < 0:
            break
        k = j + len("Camera_SetCameraData(")
        depth, commas = 1, []
        while depth:
            c = text[k]
            depth += (c == "(") - (c == ")")
            if c == "," and depth == 1:
                commas.append(k)
            k += 1
        if len(commas) == 6:
            out.append(text[i:commas[5]] + ")")
        else:
            out.append(text[i:k])
        i = k
    return "".join(out) + text[i:]


def profile(m):
    name, actor, cat, flags, obj, size, init, destroy, update, draw, reset = m.groups()
    lines = [f"ActorProfile {name}_Oot_Profile = {{"]
    for v in (actor, cat, flags, obj, f"sizeof({size})", init, destroy, update, draw):
        lines.append(f"    /**/ {v},")
    if reset not in (None, "NULL"):
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
    in_text_array = False
    for line in text.splitlines():
        if line.strip() == '#include "soh/frame_interpolation.h"':
            line = '#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"'
        elif DROP_LINE.match(line):
            continue
        if (in_text_array or TEXT_ID_ARRAY.search(line)) and not line.lstrip().startswith("//"):
            # An array of text ids, to its closing brace.
            line = TEXT_ID.sub(r"TEXT_OOT(\1)", line)
            line = re.sub(r"(?<!TEXT_OOT\()\b(0x[0-9A-Fa-f]{1,4})\b", r"TEXT_OOT(\1)", line)
            in_text_array = "}" not in line
        elif (in_text or TEXT_LINE.search(line)) and not line.lstrip().startswith("//"):
            line = TEXT_ID.sub(r"TEXT_OOT(\1)", line)
            line = SHORT_TEXT_ID.sub(r"\1TEXT_OOT(\2)", line)
            in_text = not re.search(r"[;:{}]\s*(//.*)?$", line)
        out.append(line)
    text = "\n".join(out) + "\n"
    text = re.sub(r'#include "objects/(\w+)/(\w+)\.h"', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include "textures/(\w+)/(\w+)\.h"', r'#include "assets/oot/textures/\1/\2.h"', text)
    text = drop_camera_data_last_arg(text)
    text = fix_title_card_args(text)
    for elem_name in COLLIDER_ELEMENT_NAMES:
        text = re.sub(r"\b(%s(?:\[[^\]]*\])?)(\.|->)(?:info|elem)\b" % elem_name, r"\1\2base", text)
    text = re.sub(r'#include "scenes/([\w/]+)\.h"', r'#include "assets/oot/scenes/\1.h"', text)
    # SoH's few includes spelled with the assets/ prefix.
    text = re.sub(r'#include "assets/(objects|scenes)/([\w/]+)\.h"', r'#include "assets/oot/\1/\2.h"', text)
    text = re.sub(r'#include <objects/(\w+)/(\w+)\.h>', r'#include "assets/oot/objects/\1/\2.h"', text)
    text = re.sub(r'#include "overlays/(ovl_\w+)/(\w+)\.h"', r'#include "assets/oot/overlays/\1/\2.h"', text)
    # Another ported OoT actor's header: by relative path, since MM may have an actor of the same name.
    text = re.sub(r'#include "overlays/actors/(ovl_\w+)/(\w+\.h)"',
                  lambda m: f'#include "../{m.group(1)}/{m.group(2)}"'
                  if PORTED_ACTOR(m.group(1)) else m.group(0), text)
    # One-line debug prints.
    text = re.sub(r"\n[ \t]*// \"[^\n]*\"(?=\n[ \t]*osSyncPrintf)", "", text)
    # Up to the call's end: a string may hold a semicolon.
    text = re.sub(r"\n[ \t]*osSyncPrintf\((?:\"(?:[^\"\\]|\\.)*\"|[^;\"])*?\);", "", text)
    text = re.sub(r"\n[ \t]*LOG_(?:NUM|STRING|HEX|ADDRESS|FLOAT)\((?:\"(?:[^\"\\]|\\.)*\"|[^;\"])*?\);", "", text)
    for pattern, repl in RENAMES:
        text = re.sub(pattern, repl, text, flags=re.M)
    text = re.sub(r"\bNA_SE_\w+\b", lambda m: SFX_RENAMES.get(m.group(0), m.group(0)), text)
    # OoT's music plays as OoT's sequence in MM's table (SEQ_OOT).
    text = re.sub(r"\bAudio_QueueSeqCmd\((SEQ_PLAYER_\w+) << 24 \| NA_BGM_STOP\);",
                  r"SEQCMD_STOP_SEQUENCE(\1, 0);", text)
    text = re.sub(r"\bAudio_QueueSeqCmd\((SEQ_PLAYER_\w+) << 24 \| (NA_BGM_\w+)\);",
                  lambda m: f"SEQCMD_PLAY_SEQUENCE({m.group(1)}, 0, SEQ_OOT(0x{SEQUENCES[m.group(2)]:02X})); // OoT's {m.group(2)}",
                  text)
    # OoT's fanfares MM lacks play as OoT's sequence.
    text = re.sub(r"\bAudio_PlayFanfare\((NA_BGM_\w+)\);",
                  lambda m: m.group(0) if m.group(1) in MM_SEQUENCES else
                  f"Audio_PlayFanfare(SEQ_OOT(0x{SEQUENCES[m.group(1)]:02X})); // OoT's {m.group(1)}", text)
    text = re.sub(r"\bNA_BGM_\w+\b", lambda m: m.group(0) if (m.group(0) in MM_SEQUENCES or m.group(0) not in SEQUENCES)
                  else "SEQ_OOT(0x%02X)" % SEQUENCES[m.group(0)], text)
    text = convert_equipment(text)
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
        text = re.sub(r"^(void|s32|u16|s16|f32|u8|u32|s8|\w+\*) (\w+_\w+\()", r"static \1 \2", text, flags=re.M)
        # After the actor's own header, which comes first.
        extra = '#include "HyruleFlags.h"\n#include "HyrulePort.h"\n#include "HyruleActor.h"\n'
        extra += '#include "z64quake.h"\n' if "Quake_" in text else ""
        extra += '#include "z64rumble.h"\n' if "Rumble_" in text else ""
        extra += '#include "HyrulePlayer.h"\n' if re.search(r"\bPLAYER_SHIELD_\w+_Oot\b|Player_GetStrengthOot", text) else ""
        extra += '#include "HyruleScenes.h"\n' if "Hyrule_E" in text else ""
        extra += '#include "HyruleInventory.h"\n' if "Inventory_GetDungeonIndexOot" in text else ""
        extra += '#include "z64horse.h"\n' if "Horse_" in text else ""
        extra += '#include "HyruleOcarina.h"\n' if re.search(r"OCARINA_\w+_Oot|OcarinaOot\(|StartOcarina\w*Oot", text) else ""
        extra += '#include "HyruleCutscene.h"\n' if re.search(r"gCsCtxOot|CutsceneOot\(|CS_STATE_\w+_Oot|CsCmdActorCueOot|gLinkAgeOnLoadOot", text) else ""
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
    SEQUENCES.update(oot_sequences(os.path.join(src, "..", "..", "..", "..")))
    PORTING.update(a.strip() for a in os.environ.get("PORTING", "").split(",") if a.strip())
    out = os.path.join(OUT_ROOT, os.path.basename(src))
    os.makedirs(out, exist_ok=True)
    review = re.compile(r"GameInteractor|CVar|IS_RANDO|Randomizer|gSaveContext\.ship|Sfx_Play|Audio_Play|"
                        r"Player_SetCsAction|func_8002DF54|dmgFlags|"
                        r"Item_Drop|ITEM00_|MATRIX_NEWMTX|#include \"overlays/|\.table\[|ENTR_|Horse_")
    statics = set()
    # The actor's sphere-group and triangle collider elements (arrays and pointers), whose info is MM's `base`.
    COLLIDER_ELEMENT_NAMES.clear()
    COLLIDER_ELEMENT_NAMES.add("elements")  # ColliderJntSph's and ColliderTris's own member
    for name in os.listdir(src):
        if name.endswith((".c", ".h")):
            COLLIDER_ELEMENT_NAMES.update(
                re.findall(r"\bCollider(?:JntSph|Tris)Element\b\s*\*?\s*(\w+)", open(os.path.join(src, name)).read()))
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
