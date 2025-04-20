#include "NextGenObjects.h"

#include "Itembox.h"

void NextGenObjects::applyPatches()
{
    // Ring
    WRITE_STRING(0x167D7C0, "cmn_ring"); // CObjRing
    WRITE_STRING(0x167DBA8, "cmn_ring"); // CDroppedRing

    // Spring
    WRITE_STRING(0x16738D8, "cmn_spring");

    // WideSpring 
    WRITE_STRING(0x1673924, "cmn_widespring");

    // SpringAir
    WRITE_STRING(0x1673908, "cmn_springair_top");

    // DashPanel
    WRITE_STRING(0x1672FC8, "cmn_dashpanel");
    WRITE_STRING(0x1673038, "cmn_obj_dashpanel_06_belt");

    // ThornBall
    WRITE_STRING(0x166E7A8, "cmn_havokthorn");

    // JumpBoard
    WRITE_STRING(0x1672888, "cmn_jumppanel30M");
    WRITE_STRING(0x16728A0, "cmn_jumppanel30L");
    WRITE_STRING(0x16727A4, "cmn_jumppanel15S");
    WRITE_STRING(0x16727BC, "cmn_jumppanel30S");
    WRITE_STRING(0x1672970, "cmn_obj_jumppanel_06_belt");
    WRITE_STRING(0x167298C, "cmn_obj_jumppanel_06_belt");

    // AdlibTrickJump
    WRITE_STRING(0x1669EEC, "cmn_trickpanel30M");
    WRITE_STRING(0x1669F08, "cmn_trickpanel30L");
    WRITE_STRING(0x1669FEC, "cmn_obj_trickpanel06_belt");
    WRITE_STRING(0x166A008, "cmn_obj_trickpanel06_belt");

    // DirectionalThorn
    WRITE_STRING(0x166EC10, "cmn_thorn");
    WRITE_STRING(0x166EC24, "cmn_thornpanel");

    // RainbowRing, DashRing
    WRITE_STRING(0x1672338, "cmn_rainbowring");
    WRITE_STRING(0x1672350, "cmn_dashring_gens");
    WRITE_STRING(0x1672364, "cmn_dashring"); // TailsDashring
    WRITE_STRING(0x1672628, "cmn_dashring_mat001");
    WRITE_STRING(0x1672608, "cmn_dashring_mat002");
    WRITE_MEMORY(0x1A6B800, uint32_t, 4001020);

    // GoalRing
    WRITE_STRING(0x1671630, "cmn_goalring");
    WRITE_STRING(0x1671838, "cmn_goalring_rainbow");
    WRITE_STRING(0x1671854, "cmn_goalring_rainbow");

    // RedRing
    WRITE_STRING(0x161ABB0, "cmn_silvermedal"); // pam
    WRITE_STRING(0x1668F58, "cmn_silvermedal"); // stage
    WRITE_STRING(0x1669058, "ef_silvermedal"); // stage
    // TODO: pam hide, sfx

    // PointMarker
    WRITE_STRING(0x1671D28, "cmn_savepointR");
    WRITE_STRING(0x1671D70, "cmn_savepointR");
    WRITE_STRING(0x1671DA0, "save_barRp"); // effect bone
    WRITE_STRING(0x1671DC0, "cmn_savepointR"); // anim SV
    WRITE_STRING(0x1671DF8, "cmn_savepointR"); // anim FV
    WRITE_STRING(0x1671D40, "cmn_savepointL");
    WRITE_STRING(0x1671D88, "cmn_savepointL");
    WRITE_STRING(0x1671DB0, "save_barLp");  // effect bone
    WRITE_STRING(0x1671DDC, "cmn_savepointL");  // anim SV
    WRITE_STRING(0x1671E14, "cmn_savepointL");  // anim FV
    WRITE_STRING(0x1672048, "cmn_savepoint_light"); // mat-anim
    WRITE_NOP(0x1032305, 7); // SV pole point 90 degree like FV
    WRITE_JUMP(0x10323F8, (void*)0x1032646); // Disable middle laser effect
    WRITE_MEMORY(0x1032879, uint32_t, 0x1693ECC); // mat restart -> 1.0f
    WRITE_STRING(0x1672180, "ef_savepoint_pole"); // sparkle effect

    //---------------------------------------------------
    // Custom Objects
    //---------------------------------------------------
    // 06 itembox
    Itembox::applyPatches();
}
