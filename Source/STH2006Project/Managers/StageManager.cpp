#include "StageManager.h"

#include "Configuration.h"
#include "Managers/MissionManager.h"
#include "Managers/ParamManager.h"
#include "Managers/ScoreManager.h"
#include "Objects/cmn/Explosion.h"
#include "System/Application.h"
#include "UI/LoadingUI.h"
#include "UI/UIContext.h"

HOOK(void, __fastcall, StageManager_CGameplayFlowStage_CStateTitle, 0xCF8F40, void* This)
{
    UIContext::clearDraw();
    originalStageManager_CGameplayFlowStage_CStateTitle(This);
}

//---------------------------------------------------
// Kingdom Valley sfx
//---------------------------------------------------
HOOK(int32_t*, __fastcall, StageManager_MsgHitGrindPath, 0xE25680, void* This, void* Edx, uint32_t a2)
{
    // If at Kingdom Valley, change sfx to wind
    // There are normal rails too, when x > 210
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::CheckCurrentStage("euc200") 
     && Common::GetPlayerTransform(playerPosition, playerRotation)
     && playerPosition.x() < 210.0f)
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 0);
        WRITE_MEMORY(0xE4FC82, uint32_t, 4042004);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 0);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 4042005);
    }
    else
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 1);
        WRITE_MEMORY(0xE4FC82, uint32_t, 2002038);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 1);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 2002037);
    }

    return originalStageManager_MsgHitGrindPath(This, Edx, a2);
}

HOOK(int, __fastcall, StageManager_CObjSpringSFX, 0x1038DA0, void* This)
{
    // If at Kingdom Valley, change sfx to rope
    if (Common::CheckCurrentStage("euc200"))
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 8000);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 8000);
    }
    else
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 4001015);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 4001015);
    }

    return originalStageManager_CObjSpringSFX(This);
}

//---------------------------------------------------
// Wall Jump
//---------------------------------------------------
bool StageManager::m_wallJumpStart = false;
float StageManager::m_wallJumpTime = 0.0f;
float const c_wallJumpSpinTime = 0.6f;
HOOK(bool, __fastcall, StageManager_CSonicStateFallAdvance, 0x1118C50, void* This)
{
    bool result = originalStageManager_CSonicStateFallAdvance(This);

    if (StageManager::m_wallJumpStart)
    {
        Common::SonicContextChangeAnimation("SpinAttack");
        StageManager::m_wallJumpStart = false;
        StageManager::m_wallJumpTime = c_wallJumpSpinTime;
    }
    else if (StageManager::m_wallJumpTime > 0.0f)
    {
        StageManager::m_wallJumpTime -= Application::getDeltaTime();
        if (StageManager::m_wallJumpTime <= 0.0f)
        {
            Common::SonicContextChangeAnimation("FallFast");
        }
    }

    return result;
}

HOOK(int, __fastcall, StageManager_CSonicStateFallEnd, 0x1118F20, void* This)
{
    StageManager::m_wallJumpStart = false;
    StageManager::m_wallJumpTime = 0.0f;
    return originalStageManager_CSonicStateFallEnd(This);
}

void __declspec(naked) getIsWallJump()
{
    static uint32_t returnAddress = 0xE6D5AF;
    __asm
    {
        push    esi
        push    ebx
        lea     ecx, [esi + 30h]
        call    StageManager::getIsWallJumpImpl
        pop     ebx
        pop     esi

        push    [0x15F4FE8] // Walk
        jmp     [returnAddress]
    }
}

void __fastcall StageManager::getIsWallJumpImpl(float* outOfControl)
{
    if (*outOfControl < 0.0f)
    {
        m_wallJumpStart = true;
        *outOfControl = -*outOfControl;
    }
    else
    {
        m_wallJumpStart = false;
    }
}

//---------------------------------------------------
// Water Running
//---------------------------------------------------
bool StageManager::m_waterRunning = false; 
static SharedPtrTypeless waterSoundHandle;
HOOK(char, __stdcall, StageManager_CSonicStateGrounded, 0xDFF660, int* a1, bool a2)
{
    if (Common::CheckCurrentStage("ghz200"))
    {
        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (flags->KeepRunning && flags->OnWater)
        {
            // Initial start, play sfx
            if (!StageManager::m_waterRunning)
            {
                Common::SonicContextPlaySound(waterSoundHandle, 2002059, 1);
            }

            // Change animation
            StageManager::m_waterRunning = true;
            if (!message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Sliding");
            }
        }
        else
        {
            // Auto-run finished
            StageManager::m_waterRunning = false;
            waterSoundHandle.reset();
            if (message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Walk");
            }
        }
    }

    return originalStageManager_CSonicStateGrounded(a1, a2);
}

void __declspec(naked) playWaterPfx()
{
    static uint32_t successAddress = 0x11DD1B9;
    static uint32_t returnAddress = 0x11DD240;
    __asm
    {
        mov     edx, [ecx + 4]

        // check boost
        cmp     byte ptr[edx + 10h], 0
        jnz     jump

        // check auto-run
        cmp     byte ptr[edx + 2Dh], 0
        jnz     jump

        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

HOOK(void, __fastcall, StageManager_SonicChangeAnimation, 0xE74CC0, Sonic::Player::CPlayerSpeedContext* context, int a2, Hedgehog::Base::CSharedString const& name)
{
    if (context == *pModernSonicContext && !context->m_Field168 && StageManager::m_waterRunning)
    {
        // if still water running, do not use walk animation (boost)
        if (strcmp(name.c_str(), "Walk") == 0)
        {
            originalStageManager_SonicChangeAnimation(context, a2, "Sliding");
            return;
        }

        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        if (message.IsAnimation("Sliding"))
        {
            StageManager::m_waterRunning = false;
            waterSoundHandle.reset();
        }
    }

    originalStageManager_SonicChangeAnimation(context, a2, name);
}

//---------------------------------------------------
// Object Physics dummy event
//---------------------------------------------------
ParamValue* m_LightSpeedDashStartCollisionFovy = nullptr;
ParamValue* m_LightSpeedDashStartCollisionFar = nullptr;
ParamValue* m_LightSpeedDashCollisionFovy = nullptr;
ParamValue* m_LightSpeedDashCollisionFar = nullptr;
ParamValue* m_LightSpeedDashMinVelocity = nullptr;
ParamValue* m_LightSpeedDashMinVelocity3D = nullptr;
ParamValue* m_LightSpeedDashMaxVelocity = nullptr;
ParamValue* m_LightSpeedDashMaxVelocity3D = nullptr;
HOOK(void, __fastcall, StageManager_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
    switch (message.m_Event)
    {
    // Change light speed dash param
    case 1001:
    {
        *m_LightSpeedDashStartCollisionFovy->m_funcData->m_pValue = 80.0f;
        m_LightSpeedDashStartCollisionFovy->m_funcData->update();

        *m_LightSpeedDashStartCollisionFar->m_funcData->m_pValue = 10.0f;
        m_LightSpeedDashStartCollisionFar->m_funcData->update();

        *m_LightSpeedDashCollisionFovy->m_funcData->m_pValue = 80.0f;
        m_LightSpeedDashCollisionFovy->m_funcData->update();

        *m_LightSpeedDashCollisionFar->m_funcData->m_pValue = 10.0f;
        m_LightSpeedDashCollisionFar->m_funcData->update();

        float speed = 80.0f;
        if (Common::CheckCurrentStage("ghz200")) speed = 100.0f;
        else if (Common::CheckCurrentStage("euc200")) speed = 65.0f;
        *m_LightSpeedDashMinVelocity->m_funcData->m_pValue = speed;
        m_LightSpeedDashMinVelocity->m_funcData->update();
        *m_LightSpeedDashMinVelocity3D->m_funcData->m_pValue = speed;
        m_LightSpeedDashMinVelocity3D->m_funcData->update();

        *m_LightSpeedDashMaxVelocity->m_funcData->m_pValue = 100.0f;
        m_LightSpeedDashMaxVelocity->m_funcData->update();
        *m_LightSpeedDashMaxVelocity3D->m_funcData->m_pValue = 100.0f;
        m_LightSpeedDashMaxVelocity3D->m_funcData->update();

        return;
    }
    case 300:
    {
        // Mute
        Common::PlayStageMusic("City_Escape_Generic2", 1.5f);
        return;
    }
    case 301:
    {
        // Soleanna Town
        Common::PlayStageMusic("Speed_Highway_Generic1", 0.0f);
        return;
    }
    case 302:
    {
        // New City
        Common::PlayStageMusic("Speed_Highway_Generic2", 0.0f);
        return;
    }
    case 303:
    {
        // Soleanna Forest
        Common::PlayStageMusic("Speed_Highway_Generic3", 0.0f);
        return;
    }
    }

    originalStageManager_MsgNotifyObjectEvent(This, Edx, message);
}

//---------------------------------------------------
// Result music
//---------------------------------------------------
const char* SNG19_JNG_STH = "SNG19_JNG_STH";
HOOK(int, __fastcall, StageManager_SNG19_JNG_1, 0xCFF440, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xCFF44E, char*, SNG19_JNG_STH);
    return originalStageManager_SNG19_JNG_1(This, Edx, a2);
}

HOOK(void, __fastcall, StageManager_SNG19_JNG_2, 0xD00F70, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xD01A06, char*, SNG19_JNG_STH);
    originalStageManager_SNG19_JNG_2(This, Edx, a2);
}

HOOK(void, __fastcall, StageManager_CStateGoalFadeIn, 0xCFD2D0, void* This)
{
    static const char* Result_Town = "Result_Town";
    static const char* Result = (char*)0x15B38F0;

    if (Common::IsCurrentStageMission() && !MissionManager::m_missionAsStage)
    {
        WRITE_MEMORY(0xCFD3C9, char*, Result_Town);
    }
    else
    {
        WRITE_MEMORY(0xCFD3C9, char*, Result);
    }

    // Music length
    static double length = 7.831;
    WRITE_MEMORY(0xCFD562, double*, &length);

    // Always use Result1
    WRITE_MEMORY(0xCFD4E5, uint8_t, 0xEB);

    originalStageManager_CStateGoalFadeIn(This);
}

//---------------------------------------------------
// Perfect Chaos
//---------------------------------------------------
void StageManager_CBossPerfectChaosFinalHitSfxImpl()
{
    static SharedPtrTypeless soundHandle;
    Common::PlaySoundStatic(soundHandle, 5552007);
}

void __declspec(naked) StageManager_CBossPerfectChaosFinalHitSfx()
{
    static uint32_t returnAddress = 0xC0FFC5;
    static uint32_t sub_C0F580 = 0xC0F580;
    __asm
    {
        push	eax
        call	StageManager_CBossPerfectChaosFinalHitSfxImpl
        pop     eax

        // original function
        call    [sub_C0F580]
        jmp     [returnAddress]
    }
}

HOOK(void, __fastcall, StageManager_CBossPerfectChaosCStateDefeated, 0x5D20A0, int This)
{
    bool wasMovieStarted = *(bool*)(This + 40);
    bool wasMovieEnded = *(bool*)(This + 41);

    originalStageManager_CBossPerfectChaosCStateDefeated(This);

    if (!wasMovieStarted && *(bool*)(This + 40))
    {
        Common::PlayStageMusic("Dummy", 0.0f);
        LoadingUI::startNowLoading(6.8f);
    }

    if (!wasMovieEnded && *(bool*)(This + 41))
    {
        // Movie ended by player skipping
        LoadingUI::startNowLoading();
    }
}

//---------------------------------------------------
// HUD Music
//---------------------------------------------------
HOOK(void, __fastcall, StageManager_CTutorialImpl, 0xD24440, int This, void* Edx, int a2)
{
    if (*(uint32_t*)(This + 176) == 3)
    {
        Common::PlayStageMusic("City_Escape_Generic2", 1.5f);
    }
    originalStageManager_CTutorialImpl(This, Edx, a2);
}

//---------------------------------------------------
// Enemy hitting ObjectPhysics
//---------------------------------------------------
HOOK(void, __fastcall, StageManager_CObjectPhysics_MsgDamage, 0xEA50B0, uint32_t* This, void* Edx, Sonic::Message::MsgDamage& message)
{
    if (message.m_DamageType == *(uint32_t*)0x1E0BE34) // DamageID_NoAttack
    {
        // allow enemy damage object physics
        message.m_DamageType = *(uint32_t*)0x1E0BE2C; // DamageID_Normal
    }

    originalStageManager_CObjectPhysics_MsgDamage(This, Edx, message);
}

HOOK(void, __fastcall, StageManager_CEnemyGunHunter_MsgHitEventCollision, 0xBA7580, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyGunHunter_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyELauncher_MsgHitEventCollision, 0xB7FD80, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyELauncher_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyMotora_MsgHitEventCollision, 0xBC4DF0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyMotora_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyGanigani_MsgHitEventCollision, 0xBC88D0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyGanigani_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyLander_MsgHitEventCollision, 0xBCC9E0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyLander_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyTaker_MsgHitEventCollision, 0xB9F770, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyTaker_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, StageManager_CEnemyBiter_MsgHitEventCollision, 0xB83A20, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStageManager_CEnemyBiter_MsgHitEventCollision(This, Edx, message);
    }
}

//---------------------------------------------------
// Enemy Fixes
//---------------------------------------------------
HOOK(int*, __fastcall, StageManager_CEnemyEggRobo_SpawnBrk, 0xBAAEC0, uint32_t This)
{
    int* result = originalStageManager_CEnemyEggRobo_SpawnBrk(This);

    static Hedgehog::Base::CSharedString enm_eggroboA_brk = "enm_eggroboA_brk";
    static Hedgehog::Base::CSharedString enm_eggroboB_brk = "enm_eggroboB_brk";
    static Hedgehog::Base::CSharedString cmn_eggroboA_brk_ = "cmn_eggroboA_brk_";
    static Hedgehog::Base::CSharedString cmn_eggroboB_brk_ = "cmn_eggroboB_brk_";
    if (*(bool*)(This + 416))
    {
        WRITE_MEMORY(0x1E77828, char*, enm_eggroboB_brk.get());
        WRITE_MEMORY(0x1E7782C, char*, cmn_eggroboB_brk_.get());
    }
    else
    {
        WRITE_MEMORY(0x1E77828, char*, enm_eggroboA_brk.get());
        WRITE_MEMORY(0x1E7782C, char*, cmn_eggroboA_brk_.get());
    }

    return result;
}

void __declspec(naked) StageManager_CEnemyELauncher_HideMissile()
{
    static uint32_t returnAddress = 0xB81491;
    __asm
    {
        mov     dword ptr [esp + 184h], 0xC61C4000 // -10000y
        movss   [esp + 190h], xmm0 // original
        jmp     [returnAddress]
    }
}

void __declspec(naked) StageManager_CEnemyEggRobo_TrackBeam()
{
    static uint32_t returnAddress = 0x601CF0;
    __asm
    {
        // original
        movss   [esp + 0xB4], xmm0

        // TStateGetContext
        mov     ecx, ebx
        mov     eax, [ecx + 8]

        // use missile track values
        movss   xmm0, dword ptr [eax + 0x3E0] // homing velocity
        movss   [esp + 0xA8], xmm0
        movss   xmm0, dword ptr [eax + 0x3E4] // homing start time
        movss   [esp + 0xAC], xmm0
        movss   xmm0, ds:0x1A40984 // 1.5 homing time
        movss   [esp + 0xB0], xmm0
        movss   xmm0, dword ptr [eax + 0x3EC] // life time
        movss   dword ptr [esp + 0xB8], xmm0

        jmp     [returnAddress]
    }
}

bool __fastcall StageManager_CEnemyShotPoint_IgnoreEnemyImpl(Hedgehog::Universe::CMessageActor* messageActor)
{
    uint32_t enemyType = 0u;
    messageActor->SendMessageSelfImm(Sonic::Message::MsgGetEnemyType(&enemyType));
    return enemyType > 0u;
}

void __declspec(naked) StageManager_CEnemyShotPoint_IgnoreEnemy()
{
    static uint32_t successAddress = 0xB6B411;
    static uint32_t returnAddress = 0xB6B485;
    __asm
    {
        call    StageManager_CEnemyShotPoint_IgnoreEnemyImpl

        test    al, al
        jnz     jump
        jmp     [successAddress]

        jump:
        jmp     [returnAddress]
    }
}

//---------------------------------------------------
// Bombbox Explosion
//---------------------------------------------------
HOOK(void, __stdcall, StageManager_PhysicsReward, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
{
    // Check if it's a breakable object
    if (!*(bool*)(This + 0x120))
    {
        if (!a4 || *(uint32_t*)(This + 0x108) == 1)
        {
            std::string name(*(char**)(This + 0x130));
            if (name == "cmn_bombbox" || name == "cmn_hurt_bombbox")
            {
                // spawn bombbox explosion
                hh::math::CVector pos = hh::math::CVector::Identity();
                Common::fObjectPhysicsDynamicPosition((void*)This, pos);
                
                auto spExplosion = boost::make_shared<Explosion>(Explosion::GetDefaultRadius(), pos);
                Sonic::CGameDocument::GetInstance()->AddGameObject(spExplosion);
            }
        }
    }

    originalStageManager_PhysicsReward(This, a2, a3, a4);
}

void StageManager::applyPatches()
{
    //---------------------------------------------------
    // General
    //---------------------------------------------------
    // Disable enter CpzPipe sfx
    WRITE_MEMORY(0x1234856, int, -1);

    // Disable result first sfx
    WRITE_MEMORY(0x11D24DA, int, -1);

    // Always use 3D Boost navigation
    WRITE_MEMORY(0x5295BB, uint8_t, 0xEB);

    // Clear UI at title screen
    INSTALL_HOOK(StageManager_CGameplayFlowStage_CStateTitle);
    
    //---------------------------------------------------
    // Kingdom Valley sfx
    //---------------------------------------------------
    // Play robe sfx in Kingdom Valley
    INSTALL_HOOK(StageManager_CObjSpringSFX);

    // Play wind rail sfx for Kingdom Valley
    INSTALL_HOOK(StageManager_MsgHitGrindPath);

    //---------------------------------------------------
    // Wall Jump
    //---------------------------------------------------
    // Do SpinAttack animation for walljumps (required negative out of control time)
    WRITE_JUMP(0xE6D5AA, getIsWallJump);
    INSTALL_HOOK(StageManager_CSonicStateFallAdvance);
    INSTALL_HOOK(StageManager_CSonicStateFallEnd);

    //---------------------------------------------------
    // Water Running
    //---------------------------------------------------
    // Do slide animation on water running in Wave Ocean
    INSTALL_HOOK(StageManager_CSonicStateGrounded);
    INSTALL_HOOK(StageManager_SonicChangeAnimation);
    WRITE_JUMP(0x11DD1AC, playWaterPfx);

    //---------------------------------------------------
    // Object Physics dummy event
    //---------------------------------------------------
    ParamManager::addParam(&m_LightSpeedDashStartCollisionFovy, "LightSpeedDashStartCollisionFovy");
    ParamManager::addParam(&m_LightSpeedDashStartCollisionFar, "LightSpeedDashStartCollisionFar");
    ParamManager::addParam(&m_LightSpeedDashCollisionFovy, "LightSpeedDashCollisionFovy");
    ParamManager::addParam(&m_LightSpeedDashCollisionFar, "LightSpeedDashCollisionFar");
    ParamManager::addParam(&m_LightSpeedDashMinVelocity, "LightSpeedDashMinVelocity");
    ParamManager::addParam(&m_LightSpeedDashMinVelocity3D, "LightSpeedDashMinVelocity3D");
    ParamManager::addParam(&m_LightSpeedDashMaxVelocity, "LightSpeedDashMaxVelocity");
    ParamManager::addParam(&m_LightSpeedDashMaxVelocity3D, "LightSpeedDashMaxVelocity3D");
    INSTALL_HOOK(StageManager_MsgNotifyObjectEvent);

    //---------------------------------------------------
    // Music
    //---------------------------------------------------
    // Use custom SNG19_JNG, adjust round clear length
    INSTALL_HOOK(StageManager_SNG19_JNG_1);
    INSTALL_HOOK(StageManager_SNG19_JNG_2);
    INSTALL_HOOK(StageManager_CStateGoalFadeIn);

    // Fix Mephiles BGM not playing first time
    WRITE_MEMORY(0xCFDCD5, uint8_t, 0xEB);

    //---------------------------------------------------
    // Perfect Chaos
    //---------------------------------------------------
    // Iblis final hit sfx & event movie
    WRITE_JUMP(0xC0FFC0, StageManager_CBossPerfectChaosFinalHitSfx);
    WRITE_STRING(0x1587DD8, "ev704");
    INSTALL_HOOK(StageManager_CBossPerfectChaosCStateDefeated);

    //---------------------------------------------------
    // HUD Music
    //---------------------------------------------------
    // Tutorial stop music
    INSTALL_HOOK(StageManager_CTutorialImpl);

    // Shop don't change music
    WRITE_JUMP(0xD34984, (void*)0xD349E2);
    WRITE_JUMP(0xD32D4C, (void*)0xD32D8E);

    //---------------------------------------------------
    // Enemy hitting ObjectPhysics
    //---------------------------------------------------
    INSTALL_HOOK(StageManager_CObjectPhysics_MsgDamage);
    INSTALL_HOOK(StageManager_CEnemyGunHunter_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyELauncher_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyMotora_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyGanigani_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyLander_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyTaker_MsgHitEventCollision);
    INSTALL_HOOK(StageManager_CEnemyBiter_MsgHitEventCollision);

    // Always allow MsgCheckPermissionAttack
    WRITE_MEMORY(0xBDE62E, uint8_t, 0xEB);
    WRITE_NOP(0xBDE681, 2);

    //---------------------------------------------------
    // Enemy Fixes
    //---------------------------------------------------
    // Fix EggRoboB using wrong brk object
    INSTALL_HOOK(StageManager_CEnemyEggRobo_SpawnBrk);

    // Hide ELauncher missile respawn
    WRITE_JUMP(0xB81488, (void*)StageManager_CEnemyELauncher_HideMissile);

    // CEnemyBeeton size
    WRITE_MEMORY(0xBDB1FE, uint32_t, 0x1574644); // body radius -> 1.0

    // ELauncher
    WRITE_MEMORY(0xB820C0, uint32_t, 0x1574644); // rigidbody radius -> 1.0
    WRITE_MEMORY(0xB8207F, uint32_t, 0x156460C); // attached rigidbody to Spine

    // Change EggRoboA beam data 
    WRITE_MEMORY(0x601C5C, uint32_t, 0x1A416CC); // 0.5 length
    WRITE_MEMORY(0x601CBF, uint32_t, 0x1A3F16C); // 12 shot velocity
    WRITE_MEMORY(0x601CD5, uint32_t, 0x156A23C); // 10 life time

    // Set EggRoboA beam to use EggRoboB missile track values
    WRITE_JUMP(0x601CE7, (void*)StageManager_CEnemyEggRobo_TrackBeam);
    WRITE_JUMP(0x601E54, (void*)0x601E7E); 

    // Enemy projectile damage all rigid body not just aimed target
    WRITE_JUMP(0xB6B408, (void*)StageManager_CEnemyShotPoint_IgnoreEnemy);

    // Gunner ignore slip damage
    WRITE_MEMORY(0xBAA40F, uint8_t, 0xEB);

    //---------------------------------------------------
    // Bombbox Explosion
    //---------------------------------------------------
    INSTALL_HOOK(StageManager_PhysicsReward);
}
