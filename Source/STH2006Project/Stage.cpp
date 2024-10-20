#include "Stage.h"
#include "Application.h"
#include "Configuration.h"
#include "UIContext.h"
#include "ParamManager.h"
#include "LoadingUI.h"
#include "MissionManager.h"
#include "ScoreManager.h"

HOOK(void, __fastcall, Stage_CGameplayFlowStage_CStateTitle, 0xCF8F40, void* This)
{
    UIContext::clearDraw();
    originalStage_CGameplayFlowStage_CStateTitle(This);
}

//---------------------------------------------------
// Kingdom Valley sfx
//---------------------------------------------------
HOOK(int32_t*, __fastcall, Stage_MsgHitGrindPath, 0xE25680, void* This, void* Edx, uint32_t a2)
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

    return originalStage_MsgHitGrindPath(This, Edx, a2);
}

HOOK(int, __fastcall, Stage_CObjSpringSFX, 0x1038DA0, void* This)
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

    return originalStage_CObjSpringSFX(This);
}

//---------------------------------------------------
// Wall Jump
//---------------------------------------------------
bool Stage::m_wallJumpStart = false;
float Stage::m_wallJumpTime = 0.0f;
float const c_wallJumpSpinTime = 0.6f;
HOOK(bool, __fastcall, Stage_CSonicStateFallAdvance, 0x1118C50, void* This)
{
    bool result = originalStage_CSonicStateFallAdvance(This);

    if (Stage::m_wallJumpStart)
    {
        Common::SonicContextChangeAnimation("SpinAttack");
        Stage::m_wallJumpStart = false;
        Stage::m_wallJumpTime = c_wallJumpSpinTime;
    }
    else if (Stage::m_wallJumpTime > 0.0f)
    {
        Stage::m_wallJumpTime -= Application::getDeltaTime();
        if (Stage::m_wallJumpTime <= 0.0f)
        {
            Common::SonicContextChangeAnimation("FallFast");
        }
    }

    return result;
}

HOOK(int, __fastcall, Stage_CSonicStateFallEnd, 0x1118F20, void* This)
{
    Stage::m_wallJumpStart = false;
    Stage::m_wallJumpTime = 0.0f;
    return originalStage_CSonicStateFallEnd(This);
}

void __declspec(naked) getIsWallJump()
{
    static uint32_t returnAddress = 0xE6D5AF;
    __asm
    {
        push    esi
        push    ebx
        lea     ecx, [esi + 30h]
        call    Stage::getIsWallJumpImpl
        pop     ebx
        pop     esi

        push    [0x15F4FE8] // Walk
        jmp     [returnAddress]
    }
}

void __fastcall Stage::getIsWallJumpImpl(float* outOfControl)
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
bool Stage::m_waterRunning = false; 
static SharedPtrTypeless waterSoundHandle;
HOOK(char, __stdcall, Stage_CSonicStateGrounded, 0xDFF660, int* a1, bool a2)
{
    if (Common::CheckCurrentStage("ghz200"))
    {
        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (flags->KeepRunning && flags->OnWater)
        {
            // Initial start, play sfx
            if (!Stage::m_waterRunning)
            {
                Common::SonicContextPlaySound(waterSoundHandle, 2002059, 1);
            }

            // Change animation
            Stage::m_waterRunning = true;
            if (!message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Sliding");
            }
        }
        else
        {
            // Auto-run finished
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
            if (message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Walk");
            }
        }
    }

    return originalStage_CSonicStateGrounded(a1, a2);
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

HOOK(void, __fastcall, Stage_SonicChangeAnimation, 0xE74CC0, Sonic::Player::CPlayerSpeedContext* context, int a2, Hedgehog::Base::CSharedString const& name)
{
    if (context == *pModernSonicContext && !context->m_Field168 && Stage::m_waterRunning)
    {
        // if still water running, do not use walk animation (boost)
        if (strcmp(name.c_str(), "Walk") == 0)
        {
            originalStage_SonicChangeAnimation(context, a2, "Sliding");
            return;
        }

        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        if (message.IsAnimation("Sliding"))
        {
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
        }
    }

    originalStage_SonicChangeAnimation(context, a2, name);
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
HOOK(void, __fastcall, Stage_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
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

    originalStage_MsgNotifyObjectEvent(This, Edx, message);
}

//---------------------------------------------------
// Result music
//---------------------------------------------------
const char* SNG19_JNG_STH = "SNG19_JNG_STH";
HOOK(int, __fastcall, Stage_SNG19_JNG_1, 0xCFF440, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xCFF44E, char*, SNG19_JNG_STH);
    return originalStage_SNG19_JNG_1(This, Edx, a2);
}

HOOK(void, __fastcall, Stage_SNG19_JNG_2, 0xD00F70, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xD01A06, char*, SNG19_JNG_STH);
    originalStage_SNG19_JNG_2(This, Edx, a2);
}

HOOK(void, __fastcall, Stage_CStateGoalFadeIn, 0xCFD2D0, void* This)
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

    originalStage_CStateGoalFadeIn(This);
}

//---------------------------------------------------
// Perfect Chaos
//---------------------------------------------------
void Stage_CBossPerfectChaosFinalHitSfxImpl()
{
    static SharedPtrTypeless soundHandle;
    Common::PlaySoundStatic(soundHandle, 5552007);
}

void __declspec(naked) Stage_CBossPerfectChaosFinalHitSfx()
{
    static uint32_t returnAddress = 0xC0FFC5;
    static uint32_t sub_C0F580 = 0xC0F580;
    __asm
    {
        push	eax
        call	Stage_CBossPerfectChaosFinalHitSfxImpl
        pop     eax

        // original function
        call    [sub_C0F580]
        jmp     [returnAddress]
    }
}

HOOK(void, __fastcall, Stage_CBossPerfectChaosCStateDefeated, 0x5D20A0, int This)
{
    bool wasMovieStarted = *(bool*)(This + 40);
    bool wasMovieEnded = *(bool*)(This + 41);

    originalStage_CBossPerfectChaosCStateDefeated(This);

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
HOOK(void, __fastcall, Stage_CTutorialImpl, 0xD24440, int This, void* Edx, int a2)
{
    if (*(uint32_t*)(This + 176) == 3)
    {
        Common::PlayStageMusic("City_Escape_Generic2", 1.5f);
    }
    originalStage_CTutorialImpl(This, Edx, a2);
}

//---------------------------------------------------
// Enemy hitting ObjectPhysics
//---------------------------------------------------
HOOK(void, __fastcall, Stage_CObjectPhysics_MsgDamage, 0xEA50B0, uint32_t* This, void* Edx, Sonic::Message::MsgDamage& message)
{
    if (message.m_DamageType == *(uint32_t*)0x1E0BE34) // DamageID_NoAttack
    {
        // allow enemy damage object physics
        message.m_DamageType = *(uint32_t*)0x1E0BE2C; // DamageID_Normal
    }

    originalStage_CObjectPhysics_MsgDamage(This, Edx, message);
}

HOOK(void, __fastcall, Stage_CEnemyGunHunter_MsgHitEventCollision, 0xBA7580, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyGunHunter_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyELauncher_MsgHitEventCollision, 0xB7FD80, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyELauncher_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyMotora_MsgHitEventCollision, 0xBC4DF0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyMotora_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyGanigani_MsgHitEventCollision, 0xBC88D0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyGanigani_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyLander_MsgHitEventCollision, 0xBCC9E0, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyLander_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyTaker_MsgHitEventCollision, 0xB9F770, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyTaker_MsgHitEventCollision(This, Edx, message);
    }
}

HOOK(void, __fastcall, Stage_CEnemyBiter_MsgHitEventCollision, 0xB83A20, Sonic::CObjectBase* This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        originalStage_CEnemyBiter_MsgHitEventCollision(This, Edx, message);
    }
}

//---------------------------------------------------
// Bombbox Explosion
//---------------------------------------------------
float const cExplosion_radius = 5.0f;
float const cExplosion_duration = 0.3f;
float const cExplosion_velocityObjPhy = 20.0f;
float const cExplosion_velocityEnemy = 10.0f;
class CObjExplosion : public Sonic::CGameObject3D
{
private:
    float m_Radius;
    float m_LifeTime;
    Hedgehog::Math::CVector m_Position;
    boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
    CObjExplosion
    (
        float _Radius,
        Hedgehog::Math::CVector const& _Position
    )
        : m_Radius(_Radius)
        , m_Position(_Position)
        , m_LifeTime(0.0f)
    {

    }

    ~CObjExplosion()
    {
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CGameObject3D::AddCallback(worldHolder, pGameDocument, spDatabase);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        // set initial position
        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();

        // set up collision with enemy
        m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
        m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.0f, 0.0f));
        m_spNodeEventCollision->NotifyChanged();
        m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

        hk2010_2_0::hkpSphereShape* shapeEventTrigger1 = new hk2010_2_0::hkpSphereShape(m_Radius);
        AddEventCollision("Damage", shapeEventTrigger1, *reinterpret_cast<int*>(0x1E0AF84), true, m_spNodeEventCollision);
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }

            if (std::strstr(message.GetType(), "MsgHitEventCollision") != nullptr)
            {
                uint32_t enemyType = 0u;
                SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetEnemyType>(&enemyType));

                auto* senderMessageActor = m_pMessageManager->GetMessageActor(message.m_SenderActorID);
                uint32_t* senderActor = (uint32_t*)((uint32_t)senderMessageActor - 0x28);
                bool isObjectPhysics = *(uint32_t*)senderMessageActor == 0x16CF58C;

                hh::math::CVector targetPosition = hh::math::CVector::Identity();
                if (enemyType > 0)
                {
                    // try to get center position from lock-on for enemy
                    SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                }
                else if (isObjectPhysics)
                {
                    // get dynamic position for object physics
                    Common::fObjectPhysicsDynamicPosition(senderActor, targetPosition);
                }
                else
                {
                    SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
                }

                // apply damage
                if (!targetPosition.isIdentity())
                {
                    SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
                        (
                            *(uint32_t*)0x1E0BE34, // DamageID_NoAttack
                            m_Position, 
                            (targetPosition - m_Position) * (enemyType > 0 ? cExplosion_velocityEnemy : cExplosion_velocityObjPhy)
                        ));
                
                    if (enemyType > 0)
                    {
                        ScoreManager::addEnemyChain(senderActor);
                    }
                }

                return true;
            }
        }

        return Sonic::CGameObject3D::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        // time out
        m_LifeTime += updateInfo.DeltaTime;
        if (m_LifeTime >= cExplosion_duration)
        {
            Kill();
        }
    }

    void Kill()
    {
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

HOOK(void, __stdcall, Stage_PhysicsReward, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
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
                
                auto spExplosioin = boost::make_shared<CObjExplosion>(cExplosion_radius, pos);
                Sonic::CGameDocument::GetInstance()->AddGameObject(spExplosioin);
            }
        }
    }

    originalStage_PhysicsReward(This, a2, a3, a4);
}

void Stage::applyPatches()
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
    INSTALL_HOOK(Stage_CGameplayFlowStage_CStateTitle);
    
    //---------------------------------------------------
    // Kingdom Valley sfx
    //---------------------------------------------------
    // Play robe sfx in Kingdom Valley
    INSTALL_HOOK(Stage_CObjSpringSFX);

    // Play wind rail sfx for Kingdom Valley
    INSTALL_HOOK(Stage_MsgHitGrindPath);

    //---------------------------------------------------
    // Wall Jump
    //---------------------------------------------------
    // Do SpinAttack animation for walljumps (required negative out of control time)
    WRITE_JUMP(0xE6D5AA, getIsWallJump);
    INSTALL_HOOK(Stage_CSonicStateFallAdvance);
    INSTALL_HOOK(Stage_CSonicStateFallEnd);

    //---------------------------------------------------
    // Water Running
    //---------------------------------------------------
    // Do slide animation on water running in Wave Ocean
    INSTALL_HOOK(Stage_CSonicStateGrounded);
    INSTALL_HOOK(Stage_SonicChangeAnimation);
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
    INSTALL_HOOK(Stage_MsgNotifyObjectEvent);

    //---------------------------------------------------
    // Result music
    //---------------------------------------------------
    // Use custom SNG19_JNG, adjust round clear length
    INSTALL_HOOK(Stage_SNG19_JNG_1);
    INSTALL_HOOK(Stage_SNG19_JNG_2);
    INSTALL_HOOK(Stage_CStateGoalFadeIn);

    //---------------------------------------------------
    // Perfect Chaos
    //---------------------------------------------------
    // Iblis final hit sfx & event movie
    WRITE_JUMP(0xC0FFC0, Stage_CBossPerfectChaosFinalHitSfx);
    WRITE_STRING(0x1587DD8, "ev704");
    INSTALL_HOOK(Stage_CBossPerfectChaosCStateDefeated);

    //---------------------------------------------------
    // HUD Music
    //---------------------------------------------------
    // Tutorial stop music
    INSTALL_HOOK(Stage_CTutorialImpl);

    // Shop don't change music
    WRITE_JUMP(0xD34984, (void*)0xD349E2);
    WRITE_JUMP(0xD32D4C, (void*)0xD32D8E);

    //---------------------------------------------------
    // Enemy hitting ObjectPhysics
    //---------------------------------------------------
    INSTALL_HOOK(Stage_CObjectPhysics_MsgDamage);
    INSTALL_HOOK(Stage_CEnemyGunHunter_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyELauncher_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyMotora_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyGanigani_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyLander_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyTaker_MsgHitEventCollision);
    INSTALL_HOOK(Stage_CEnemyBiter_MsgHitEventCollision);

    // Always allow MsgCheckPermissionAttack
    WRITE_MEMORY(0xBDE62E, uint8_t, 0xEB);
    WRITE_NOP(0xBDE681, 2);

    //---------------------------------------------------
    // Bombbox Explosion
    //---------------------------------------------------
    INSTALL_HOOK(Stage_PhysicsReward);
}
