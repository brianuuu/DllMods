#include "EnemyHealth.h"

#include "Objects/enemy/EnemyLander.h"
#include "Objects/enemy/EnemyMotora.h"

std::mutex EnemyHealth::m_mutex;
std::map<uint32_t, EnemyHealth::HealthData> EnemyHealth::m_healthData;
bool EnemyHealth::m_noDamage = false;

HOOK(int32_t*, __fastcall, EnemyHealth_CSonicStateHomingAttackAfterBegin, 0x1118300, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    context->StateFlag(eStateFlag_NoDamage)++;
    EnemyHealth::m_noDamage = true;

    return originalEnemyHealth_CSonicStateHomingAttackAfterBegin(This);
}

HOOK(void, __fastcall, EnemyHealth_CSonicStateHomingAttackAfterAdvance, 0x1118600, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (EnemyHealth::m_noDamage && context->m_Velocity.y() <= 15.0f) // HomingAttackAfterUpVelocity = 18
    {
        context->StateFlag(eStateFlag_NoDamage)--;
        EnemyHealth::m_noDamage = false;
    }

    originalEnemyHealth_CSonicStateHomingAttackAfterAdvance(This);
}

HOOK(void, __fastcall, EnemyHealth_CSonicStateHomingAttackAfterEnd, 0x11182F0)
{
    if (EnemyHealth::m_noDamage)
    {
        auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
        context->StateFlag(eStateFlag_NoDamage)--;
        EnemyHealth::m_noDamage = false;
    }

    originalEnemyHealth_CSonicStateHomingAttackAfterEnd();
}

float const cEnemyHealth_noDamageCooldown = 0.1f;
float const cEnemyHealth_hiddenTimer = 5.0f;

class CObjHealth : public Sonic::CGameObject
{
private:
    Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectHealth;
    boost::shared_ptr<Sonic::CGameObjectCSD> m_spHealth;
    Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneHealth;

    std::mutex m_mutex;
    uint32_t m_targetID = 0u;
    uint32_t m_attackerID = 0u;
    float m_damageCooldown = cEnemyHealth_noDamageCooldown;
    float m_hiddenTimer = cEnemyHealth_hiddenTimer;
    float m_offset = 0.0f;

    uint8_t m_maxHP = 1u;
    uint8_t m_currentHP = 1u;

public:
    CObjHealth
    (
        uint32_t _targetID,
        uint32_t _attackerID,
        float _offset,
        uint8_t _maxHP,
        uint8_t _currentHP
    )
        : m_targetID(_targetID)
        , m_attackerID(_attackerID)
        , m_offset(_offset)
        , m_maxHP(_maxHP)
        , m_currentHP(_currentHP)
    {
    }

    ~CObjHealth()
    {
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("1", this);

        Sonic::CCsdDatabaseWrapper wrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
        auto spCsdProject = wrapper.GetCsdProject("enemy_powergage");
        Common::CopyCCsdProject(spCsdProject.get(), m_projectHealth);

        if (m_projectHealth)
        {
            m_sceneHealth = m_projectHealth->CreateScene("enemy_powergage");
            if (m_sceneHealth)
            {
                m_sceneHealth->m_MotionSpeed = 0.0f;
            }

            m_spHealth = boost::make_shared<Sonic::CGameObjectCSD>(m_projectHealth, 0.5f, "HUD", false);
            Sonic::CGameDocument::GetInstance()->AddGameObject(m_spHealth, "main", this);

            UpdateHealthBar();
        }
    }

    void KillCallback() override
    {
        if (m_spHealth)
        {
            m_spHealth->SendMessage(m_spHealth->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
            m_spHealth = nullptr;
        }

        Chao::CSD::CProject::DestroyScene(m_projectHealth.Get(), m_sceneHealth);
        m_projectHealth = nullptr;
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (message.Is<Sonic::Message::MsgRestartStage>() || message.Is<Sonic::Message::MsgStageClear>())
            {
                Kill();
                return true;
            }
        }

        return Sonic::CGameObject::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        if (m_damageCooldown > 0.0f)
        {
            m_damageCooldown -= updateInfo.DeltaTime;
        }

        if (m_hiddenTimer > 0.0f)
        {
            m_hiddenTimer -= updateInfo.DeltaTime;
        }

        UpdateHealthBar();
    }

    void UpdateHealthBar()
    {
        if (!m_sceneHealth) return;

        hh::math::CVector position = hh::math::CVector::Zero();
        SendMessageImm(m_targetID, Sonic::Message::MsgGetHomingAttackPosition(&position));
        if (position.isZero())
        {
            SendMessageImm(m_targetID, Sonic::Message::MsgGetPosition(&position));
        }
;
        Eigen::Vector4f screenPosition;
        Eigen::Vector4f position4(position.x(), position.y() + m_offset, position.z(), 1.0f);
        Common::fGetScreenPosition(position4, screenPosition);

        auto spCamera = m_pMember->m_pWorld->GetCamera();
        float const dist = (position - spCamera->m_MyCamera.m_Position).norm();
        float const scale = 5.0f / dist;

        m_sceneHealth->SetHideFlag(screenPosition.z() < 0.0f || m_hiddenTimer <= 0.0f);
        m_sceneHealth->SetPosition(screenPosition.x(), screenPosition.y());
        m_sceneHealth->SetMotionFrame(100.0f * (float)m_currentHP / (float)m_maxHP);
        m_sceneHealth->SetScale(scale, scale);
        m_sceneHealth->Update();
    }

    void ApplyDamage(uint32_t attackerID, uint8_t currentHP)
    {
        m_attackerID = attackerID;
        m_currentHP = currentHP;
        m_damageCooldown = cEnemyHealth_noDamageCooldown;
        m_hiddenTimer = cEnemyHealth_hiddenTimer;
    }

    bool CanDamage(uint32_t attackerID)
    {
        return m_attackerID != attackerID || m_damageCooldown <= 0.0f;
    }
};

bool EnemyHealth_HandleMsgDamage(hh::fnd::CMessageActor* This, hh::fnd::Message& message)
{
    uint32_t pCEnemyBase = (uint32_t)This - 0x28;
    uint8_t* health = (uint8_t*)(pCEnemyBase + 0x16F);

    if (*health > 0)
    {
        std::lock_guard<std::mutex> guard(EnemyHealth::m_mutex);
        EnemyHealth::HealthData& data = EnemyHealth::m_healthData[pCEnemyBase];
        if (data.m_spHealth && !data.m_spHealth->CanDamage(message.m_SenderActorID))
        {
            return true;
        }

        if (--(*health) > 0)
        {
            if (data.m_spHealth)
            {
                data.m_spHealth->ApplyDamage(message.m_SenderActorID, *health);
            }
            else
            {
                Sonic::CObjectBase const* pObjectBase = (Sonic::CObjectBase*)pCEnemyBase;
                data.m_spHealth = boost::make_shared<CObjHealth>
                    (
                        pObjectBase->m_ActorID, 
                        message.m_SenderActorID, 
                        EnemyHealth::GetHealthOffset(pCEnemyBase), 
                        EnemyHealth::GetMaxHealth(pCEnemyBase), 
                        *health
                    );
                Sonic::CGameDocument::GetInstance()->AddGameObject(data.m_spHealth);
            }

            uint32_t const pAimTargetParams = *(uint32_t*)(pCEnemyBase + 0x11C);
            bool const isCrisisCityEnemy = *(bool*)(pAimTargetParams + 0x28);

            SharedPtrTypeless soundHandle;
            Common::ObjectPlaySound((void*)pCEnemyBase, isCrisisCityEnemy ? 5001007 : 5001001, soundHandle);

            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
            if (message.m_SenderActorID == context->m_pPlayer->m_ActorID)
            {
                *(uint32_t*)((uint32_t)context + 0x1174) = 0u; // reset last hit actor ID so hit effect plays again
                This->SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamageSuccess>(context->m_spMatrixNode->m_Transform.m_Position, true));
            }

            return true;
        }
        else
        {
            // delete health bar if there's one
            if (data.m_spHealth) data.m_spHealth->Kill();
            EnemyHealth::m_healthData.erase(pCEnemyBase);
        }
    }

    return false;
}

HOOK(void, __fastcall, EnemyHealth_CEnemyBaseAddCallback, 0xBDF720, uint32_t pCEnemyBase, void* Edx, int a2, int a3, int a4)
{
    originalEnemyHealth_CEnemyBaseAddCallback(pCEnemyBase, Edx, a2, a3, a4);
    *(uint8_t*)(pCEnemyBase + 0x16F) = EnemyHealth::GetMaxHealth(pCEnemyBase);
}

HOOK(int, __fastcall, EnemyHealth_CEnemyBaseDeathCallback, 0xBDEBE0, uint32_t pCEnemyBase, void* Edx, void* a2)
{
    std::lock_guard<std::mutex> guard(EnemyHealth::m_mutex);
    auto iter = EnemyHealth::m_healthData.find(pCEnemyBase);
    if (iter != EnemyHealth::m_healthData.end())
    {
        EnemyHealth::HealthData& healthData = iter->second;
        if (healthData.m_spHealth) healthData.m_spHealth->Kill();

        EnemyHealth::m_healthData.erase(iter);
    }

    return originalEnemyHealth_CEnemyBaseDeathCallback(pCEnemyBase, Edx, a2);
}

HOOK(void, _fastcall, EnemyHealth_CEnemyBeeton_CStateReviveWaitEnd, 0x5F0390, Hedgehog::Universe::CTinyStateMachineBase::CStateBase* stateBase)
{
    uint32_t pCEnemyBase = (uint32_t)stateBase->m_pContext;
    *(uint8_t*)(pCEnemyBase + 0x16F) = EnemyHealth::GetMaxHealth(pCEnemyBase);
    originalEnemyHealth_CEnemyBeeton_CStateReviveWaitEnd(stateBase);
}

HOOK(void, __stdcall, EnemyHealth_CEnemyEggRobo_CStateReviveBegin, 0x602AF0, uint32_t pCEnemyBase)
{
    *(uint8_t*)(pCEnemyBase + 0x16F) = EnemyHealth::GetMaxHealth(pCEnemyBase);
    originalEnemyHealth_CEnemyEggRobo_CStateReviveBegin(pCEnemyBase);
}

#define HOOK_ENEMY_PROCESS_MESSAGE(enemyName, address) \
HOOK(bool, __fastcall, EnemyHealth_##enemyName##_ProcessMessage, address, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag) \
{ \
    if (flag && message.Is<Sonic::Message::MsgDamage>()) \
    { \
        if (EnemyHealth_HandleMsgDamage(This, message)) \
        { \
            return true; \
        } \
    } \
    return originalEnemyHealth_##enemyName##_ProcessMessage(This, Edx, message, flag); \
}

HOOK_ENEMY_PROCESS_MESSAGE(CEnemyBeeton, 0xBDC180)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyEggRobo, 0xBB0220)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyELauncher, 0xB82B40)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyCrawler, 0xB99E10)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyMotora, 0xBC7350)
HOOK_ENEMY_PROCESS_MESSAGE(CEnemyLander, 0xBCF740)

void EnemyHealth::applyPatches()
{
    // Disable player Damage going up in HomingAttackAfter
    INSTALL_HOOK(EnemyHealth_CSonicStateHomingAttackAfterBegin);
    INSTALL_HOOK(EnemyHealth_CSonicStateHomingAttackAfterAdvance);
    INSTALL_HOOK(EnemyHealth_CSonicStateHomingAttackAfterEnd);

    // Common hooks
    INSTALL_HOOK(EnemyHealth_CEnemyBaseAddCallback);
    INSTALL_HOOK(EnemyHealth_CEnemyBaseDeathCallback);

    // Individual enemy hooks
    INSTALL_HOOK(EnemyHealth_CEnemyBeeton_ProcessMessage);
    INSTALL_HOOK(EnemyHealth_CEnemyEggRobo_ProcessMessage);
    INSTALL_HOOK(EnemyHealth_CEnemyELauncher_ProcessMessage);
    INSTALL_HOOK(EnemyHealth_CEnemyCrawler_ProcessMessage);
    INSTALL_HOOK(EnemyHealth_CEnemyMotora_ProcessMessage);
    INSTALL_HOOK(EnemyHealth_CEnemyLander_ProcessMessage);

    // Reset health for FakeDead enemies
    INSTALL_HOOK(EnemyHealth_CEnemyBeeton_CStateReviveWaitEnd);
    INSTALL_HOOK(EnemyHealth_CEnemyEggRobo_CStateReviveBegin);

    // Replace enemy's interactionType to regular enemy lock-on
    WRITE_MEMORY(0xBAF7EC, uint32_t, 0x1E0AF24); // CEnemyEggRobo
    WRITE_MEMORY(0xBAF655, uint32_t, 0x1E0AF24); // CEnemyEggRobo
    WRITE_MEMORY(0xBAB07F, uint32_t, 0x1E0AF24); // CEnemyEggRobo revive
    WRITE_MEMORY(0xB8211B, uint32_t, 0x1E0AF24); // CEnemyELauncher
}

float EnemyHealth::GetHealthOffset(uint32_t pCEnemyBase)
{ 
    switch (*(uint32_t*)pCEnemyBase)
    {
    case 0x16F517C: // CEnemyBeeton
    {
        return 1.6f;
    }
    case 0x16F7C9C: // CEnemyEggRobo
    {
        return 1.0f;
    }
    case 0x16FB1FC: // CEnemyELauncher
    case 0x16F95CC: // CEnemyCrawler
    {
        return 2.0f;
    }
    case 0x16F5F64: // CEnemyLander
    {
        return 1.5f;
    }
    case 0x16F67C4: // CEnemyMotora
    {
        return 0.7f;
    }
    }

    return 0.0f;
}

uint32_t EnemyHealth::GetMaxHealth(uint32_t pCEnemyBase)
{
    switch (*(uint32_t*)pCEnemyBase)
    {
    case 0x16F517C: // CEnemyBeeton
    {
        return *(bool*)(pCEnemyBase + 0x17C) ? 0u : 3u;
    }
    case 0x16F7C9C: // CEnemyEggRobo
    {
        return *(bool*)(pCEnemyBase + 416) ? 0u : 2u;
    }
    case 0x16FB1FC: // CEnemyELauncher
    case 0x16F95CC: // CEnemyCrawler
    {
        return 3u;
    }
    case 0x16F5F64: // CEnemyLander
    {
        return ((EnemyLander*)pCEnemyBase)->m_isCommander ? 2u : 0u;
    }
    case 0x16F67C4: // CEnemyMotora
    {
        return ((EnemyMotora*)pCEnemyBase)->m_isChaser ? 2u : 0u;
    }
    }

    return 0u;
}

uint8_t EnemyHealth::GetHealth(uint32_t pCEnemyBase)
{
    return *(uint8_t*)(pCEnemyBase + 0x16F);
}

void EnemyHealth::SetHealth(uint32_t pCEnemyBase, uint8_t health)
{
    *(uint8_t*)(pCEnemyBase + 0x16F) = health;
}
