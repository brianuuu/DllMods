#include "EnemyShock.h"

std::recursive_mutex EnemyShock::m_mutex;
std::map<uint32_t, EnemyShock::ShockData> EnemyShock::m_shockData;

class CObjShock : public Sonic::CGameObject3D
{
private:
    uint32_t m_TargetID = 0u;
    Hedgehog::Math::CVector m_Position;
    bool m_IsCrisisCityEnemy;

    Sonic::CGlitterPlayer* m_pGlitterPlayer = nullptr;
    uint32_t m_pfxID = 0u;

public:
    CObjShock
    (
        uint32_t _TargetID,
        Hedgehog::Math::CVector const& _Position,
        bool _IsCrisisCityEnemy
    )
        : m_TargetID(_TargetID)
        , m_Position(_Position)
        , m_IsCrisisCityEnemy(_IsCrisisCityEnemy)
    {

    }

    ~CObjShock()
    {
        if (m_pGlitterPlayer)
        {
            delete m_pGlitterPlayer;
        }
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CGameObject3D::AddCallback(worldHolder, pGameDocument, spDatabase);
        m_pGlitterPlayer = Sonic::CGlitterPlayer::Make(pGameDocument);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        // Set initial transform
        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();

        // play pfx
        m_pfxID = m_pGlitterPlayer->PlayContinuous(m_pMember->m_pGameDocument, m_spMatrixNodeTransform, m_IsCrisisCityEnemy ? "ef_ch_sns_yh1_damage_shock2" : "ef_ch_sns_yh1_damage_shock", 1.0f);
    }

    void KillCallback() override
    {
        if (m_pfxID)
        {
            m_pGlitterPlayer->StopByID(m_pfxID, true);
            m_pfxID = 0u;
        }
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
        }

        return Sonic::CGameObject3D::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        if (m_ActorID)
        {
            // get current target position
            hh::math::CVector targetPosition = hh::math::CVector::Zero();
            SendMessageImm(m_TargetID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
            if (!targetPosition.isZero())
            {
                SendMessageImm(m_TargetID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                m_Position = targetPosition;

                m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
                m_spMatrixNodeTransform->NotifyChanged();
            }
        }
    }
};

void HandleEnemyAddShock(hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message)
{
    auto& msg = static_cast<Sonic::Message::MsgNotifyShockWave&>(message);
    uint32_t pCEnemyBase = ((uint32_t)This - 0x28);

    // some enemies requiire changing state
    bool isIn3D = *(bool*)(pCEnemyBase + 0x120);
    switch (*(uint32_t*)pCEnemyBase)
    {
    case 0x16FAD14: // CEnemyBiter
    {
        Common::NPCAnimationChangeState((void*)pCEnemyBase, isIn3D ? "FireEndFV" : "FireEndSV");
        Common::EnemyChangeState((void*)pCEnemyBase, "BackStep");
        break;
    } 
    case 0x16F70BC: // CEnemySpinner
    {
        bool isDischarging = *(bool*)(pCEnemyBase + 0x239);
        if (isDischarging)
        {
            // stops discharging forever
            *(float*)(pCEnemyBase + 0x228) = 0.0f;
            return;
        }
        break;
    }
    }

    uint32_t pAimTargetParams = *(uint32_t*)(pCEnemyBase + 0x11C);
    bool isCrisisCityEnemy = *(bool*)(pAimTargetParams + 0x28);

    SharedPtrTypeless soundHandle;
    Common::ObjectPlaySound((void*)pCEnemyBase, isCrisisCityEnemy ? 5001007 : 5001001, soundHandle);
    *(bool*)(pCEnemyBase + 0x16E) = true;

    std::lock_guard<std::recursive_mutex> guard(EnemyShock::m_mutex);
    EnemyShock::ShockData& shockData = EnemyShock::m_shockData[pCEnemyBase];
    shockData.m_Timer = msg.m_Duration;
    if (!shockData.m_Sound)
    {
        Common::ObjectPlaySound((void*)pCEnemyBase, 2002097, shockData.m_Sound);

        Sonic::CObjectBase* pObjectBase = (Sonic::CObjectBase*)pCEnemyBase;
        hh::math::CVector targetPosition = hh::math::CVector::Zero();
        pObjectBase->SendMessageImm(pObjectBase->m_ActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
        if (!targetPosition.isZero())
        {
            shockData.m_Effect = boost::make_shared<CObjShock>(pObjectBase->m_ActorID, targetPosition, isCrisisCityEnemy);
            pObjectBase->m_pMember->m_pGameDocument->AddGameObject(shockData.m_Effect);
        }
    }
}

void HandleEnemyRemoveShock(uint32_t pCEnemyBase)
{
    *(bool*)(pCEnemyBase + 0x16E) = false;

    std::lock_guard<std::recursive_mutex> guard(EnemyShock::m_mutex);
    auto iter = EnemyShock::m_shockData.find(pCEnemyBase);
    if (iter != EnemyShock::m_shockData.end())
    {
        EnemyShock::ShockData& shockData = iter->second;
        if (shockData.m_Sound)  shockData.m_Sound.reset();
        if (shockData.m_Effect) shockData.m_Effect->Kill();
        
        EnemyShock::m_shockData.erase(iter);
    }
}

void HandleEnemyAdvanceShock(uint32_t pCEnemyBase, float dt)
{
    std::lock_guard<std::recursive_mutex> guard(EnemyShock::m_mutex);

    auto iter = EnemyShock::m_shockData.find(pCEnemyBase);
    if (iter != EnemyShock::m_shockData.end())
    {
        EnemyShock::ShockData& shockData = iter->second;
        shockData.m_Timer -= dt;
        if (shockData.m_Timer <= 0.0f)
        {
            HandleEnemyRemoveShock(pCEnemyBase);
        }
    }
    else
    {
        HandleEnemyRemoveShock(pCEnemyBase);
    }
}

HOOK(int, __fastcall, EnemyShock_CEnemyBaseUpdateParallel, 0xBDEC80, uint32_t pCEnemyBase, void* Edx, Hedgehog::Universe::SUpdateInfo& updateInfo)
{
    // don't advance anything while shocked
    bool* pIsFrozen = (bool*)(pCEnemyBase + 0x16E);
    if (*pIsFrozen)
    {
        HandleEnemyAdvanceShock(pCEnemyBase, updateInfo.DeltaTime);
        return 0;
    }

    return originalEnemyShock_CEnemyBaseUpdateParallel(pCEnemyBase, Edx, updateInfo);
}

HOOK(int, __fastcall, EnemyShock_CEnemyBaseDeathCallback, 0xBDEBE0, uint32_t pCEnemyBase, void* Edx, void* a2)
{
    bool* pIsFrozen = (bool*)(pCEnemyBase + 0x16E);
    if (*pIsFrozen)
    {
        HandleEnemyRemoveShock(pCEnemyBase);
    }

    return originalEnemyShock_CEnemyBaseDeathCallback(pCEnemyBase, Edx, a2);
}

#define HOOK_ENEMY_PROCESS_MESSAGE(enemyName, address) \
    HOOK(bool, __fastcall, enemyName##_ProcessMessage, address, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag) \
    { \
        if (flag && message.Is<Sonic::Message::MsgNotifyShockWave>()) \
        { \
            HandleEnemyAddShock(This, Edx, message); \
            return true; \
        } \
        if (flag && message.Is<Sonic::Message::MsgDamage>()) \
        { \
            HandleEnemyRemoveShock((uint32_t)This - 0x28); \
        } \
        return original##enemyName##_ProcessMessage(This, Edx, message, flag); \
    }

HOOK_ENEMY_PROCESS_MESSAGE(CEnemyEChaserSV, 0xB76390)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyAeroCannon, 0xB7F1A0)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyBeetle, 0xBA6510) // also CEnemyMonoBeetle and CEnemyGunBeetle
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyEggRobo, 0xBB0220)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyGrabber, 0xBC3B30)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyBatabata, 0xBD7E90)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyBeeton, 0xBDC180)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyELauncher, 0xB82B40)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyCrawler, 0xB99E10)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyGunHunter, 0xBAA600)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyCopSpeeder, 0xBBA6B0)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyMotora, 0xBC7350)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyGanigani, 0xBCB8D0)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyLander, 0xBCF740)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyEFighter, 0xBD4DD0)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyNal, 0xB9EA10)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemySpinner, 0xBBDA30)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyPawnBase, 0xB958D0) // include all CEnemyPawn
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyTaker, 0xBA3250)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyBiter, 0xB869B0)

void EnemyShock::applyPatches()
{
    // Make sure 0x16E is set to 0
    WRITE_MEMORY(0xBDF841, uint8_t, 0x89);

    INSTALL_HOOK(EnemyShock_CEnemyBaseUpdateParallel);
    INSTALL_HOOK(EnemyShock_CEnemyBaseDeathCallback);

    INSTALL_HOOK(CEnemyEChaserSV_ProcessMessage);
    INSTALL_HOOK(CEnemyAeroCannon_ProcessMessage);
    INSTALL_HOOK(CEnemyBeetle_ProcessMessage);
    INSTALL_HOOK(CEnemyEggRobo_ProcessMessage);
    INSTALL_HOOK(CEnemyGrabber_ProcessMessage);
    INSTALL_HOOK(CEnemyBatabata_ProcessMessage);
    INSTALL_HOOK(CEnemyBeeton_ProcessMessage);
    INSTALL_HOOK(CEnemyELauncher_ProcessMessage);
    INSTALL_HOOK(CEnemyCrawler_ProcessMessage);
    INSTALL_HOOK(CEnemyGunHunter_ProcessMessage);
    INSTALL_HOOK(CEnemyCopSpeeder_ProcessMessage);
    INSTALL_HOOK(CEnemyMotora_ProcessMessage);
    INSTALL_HOOK(CEnemyGanigani_ProcessMessage);
    INSTALL_HOOK(CEnemyLander_ProcessMessage);
    INSTALL_HOOK(CEnemyEFighter_ProcessMessage);
    INSTALL_HOOK(CEnemyNal_ProcessMessage);
    INSTALL_HOOK(CEnemySpinner_ProcessMessage);
    INSTALL_HOOK(CEnemyPawnBase_ProcessMessage);
    INSTALL_HOOK(CEnemyTaker_ProcessMessage);
    INSTALL_HOOK(CEnemyBiter_ProcessMessage);
}
