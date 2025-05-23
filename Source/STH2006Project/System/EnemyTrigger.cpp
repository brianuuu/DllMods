#include "EnemyTrigger.h"

#include "Managers/ScoreManager.h"

void __declspec(naked) EnemyTrigger_SendEnemyEventTrigger()
{
    static uint32_t fpCHolderBaseDtor = 0x65FC40;
    static uint32_t fpEventTrigger = 0xD5ED00;
    static uint32_t returnAddress = 0xBE067A;
    __asm
    {
        call    [fpCHolderBaseDtor]
        push    4
        push    esi
        call    [fpEventTrigger]
        jmp     [returnAddress]
    }
}

void EnemyTrigger_HandleEnemyMsgNotifyObjectEvent(hh::fnd::CMessageActor* This, hh::fnd::Message& message)
{
    auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
    if (msg.m_Event == 12)
    {
        // event damage always kills enemy, set HP to 0
        uint32_t pCEnemyBase = (uint32_t)This - 0x28;
        *(uint8_t*)(pCEnemyBase + 0x16F) = 0u;

        ScoreManager::addEnemyChain((uint32_t*)pCEnemyBase);
        This->SendMessage(This->m_ActorID, boost::make_shared<Sonic::Message::MsgDamage>
            (
                *(uint32_t*)0x1E0BE30, hh::math::CVector::Zero(), hh::math::CVector::Zero()
            )
        );
    }
}

bool EnemyTrigger_HandleEnemyMsgDamage(hh::fnd::CMessageActor* This, hh::fnd::Message& message)
{
    // Fix gens hitting FakeDead enemy multiple times
    uint32_t pCEnemyBase = (uint32_t)This - 0x28;
    Hedgehog::Universe::CTinyStateMachineBase* stateMachine = (Hedgehog::Universe::CTinyStateMachineBase*)(pCEnemyBase + 0x12C);
    auto const& stateName = stateMachine->m_spCurrentState->GetName();
    return stateName == "ReviveWait" || stateName == "FakeDead";
}

#define HOOK_ENEMY_PROCESS_MESSAGE(enemyName, address) \
    HOOK(bool, __fastcall, EnemyTrigger_##enemyName##_ProcessMessage, address, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag) \
    { \
        if (flag && message.Is<Sonic::Message::MsgNotifyObjectEvent>()) \
        { \
            EnemyTrigger_HandleEnemyMsgNotifyObjectEvent(This, message); \
        } \
        if (flag && message.Is<Sonic::Message::MsgDamage>()) \
        { \
            if (EnemyTrigger_HandleEnemyMsgDamage(This, message)) return true; \
        } \
        return originalEnemyTrigger_##enemyName##_ProcessMessage(This, Edx, message, flag); \
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

HOOK(bool, __fastcall, EnemyTrigger_CEnemyBase_ProcessMessage, 0xBE0790, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    if (flag && message.Is<Sonic::Message::MsgGetEnemyType>())
    {
        auto& msg = static_cast<Sonic::Message::MsgGetEnemyType&>(message);
        *msg.m_pType = 1;
        return true;
    }

    return originalEnemyTrigger_CEnemyBase_ProcessMessage(This, Edx, message, flag);
}

void EnemyTrigger::applyPatches()
{
    // Allow all enemies to send Trigger 4, call from 0xD5F9B0 (using function from CObjectPhysics)
    WRITE_MEMORY(0x16FC414 + 0x2C, uint32_t, 0xEA2940); // CEnemyEChaserSV
    WRITE_MEMORY(0x16FB62C + 0x2C, uint32_t, 0xEA2940); // CEnemyAeroCannon
    WRITE_MEMORY(0x16F868C + 0x2C, uint32_t, 0xEA2940); // CEnemyBeetle
    WRITE_MEMORY(0x16F87CC + 0x2C, uint32_t, 0xEA2940); // CEnemyMonoBeetle
    WRITE_MEMORY(0x16F890C + 0x2C, uint32_t, 0xEA2940); // CEnemyGunBeetle
    WRITE_MEMORY(0x16F7C9C + 0x2C, uint32_t, 0xEA2940); // CEnemyEggRobo
    WRITE_MEMORY(0x16F6B5C + 0x2C, uint32_t, 0xEA2940); // CEnemyGrabber
    WRITE_MEMORY(0x16F561C + 0x2C, uint32_t, 0xEA2940); // CEnemyBatabata
    WRITE_MEMORY(0x16F517C + 0x2C, uint32_t, 0xEA2940); // CEnemyBeeton
    WRITE_MEMORY(0x16FB1FC + 0x2C, uint32_t, 0xEA2940); // CEnemyELauncher
    WRITE_MEMORY(0x16F95CC + 0x2C, uint32_t, 0xEA2940); // CEnemyCrawler
    WRITE_MEMORY(0x16F82FC + 0x2C, uint32_t, 0xEA2940); // CEnemyGunHunter
    WRITE_MEMORY(0x16F755C + 0x2C, uint32_t, 0xEA2940); // CEnemyCopSpeeder
    WRITE_MEMORY(0x16F67C4 + 0x2C, uint32_t, 0xEA2940); // CEnemyMotora
    WRITE_MEMORY(0x16F62B4 + 0x2C, uint32_t, 0xEA2940); // CEnemyGanigani
    WRITE_MEMORY(0x16F5F64 + 0x2C, uint32_t, 0xEA2940); // CEnemyLander
    WRITE_MEMORY(0x16F593C + 0x2C, uint32_t, 0xEA2940); // CEnemyEFighter
    WRITE_MEMORY(0x16F912C + 0x2C, uint32_t, 0xEA2940); // CEnemyNal
    WRITE_MEMORY(0x16F70BC + 0x2C, uint32_t, 0xEA2940); // CEnemySpinner
    WRITE_MEMORY(0x16FA5F4 + 0x2C, uint32_t, 0xEA2940); // CEnemyPawnPla
    WRITE_MEMORY(0x16F9E8C + 0x2C, uint32_t, 0xEA2940); // CEnemyPawnGun
    WRITE_MEMORY(0x16F9A1C + 0x2C, uint32_t, 0xEA2940); // CEnemyPawnBase
    WRITE_MEMORY(0x16FA104 + 0x2C, uint32_t, 0xEA2940); // CEnemyPawnLance
    WRITE_MEMORY(0x16FA37C + 0x2C, uint32_t, 0xEA2940); // CEnemyPawnNormal
    WRITE_MEMORY(0x16F8C54 + 0x2C, uint32_t, 0xEA2940); // CEnemyTaker
    WRITE_MEMORY(0x16FAD14 + 0x2C, uint32_t, 0xEA2940); // CEnemyBiter

    // Send EventTrigger when enemy dies (when CObjChaosEnergy spawns)
    WRITE_JUMP(0xBE0675, EnemyTrigger_SendEnemyEventTrigger);

    // Handle MsgNotifyObjectEvent
    INSTALL_HOOK(EnemyTrigger_CEnemyEChaserSV_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyAeroCannon_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyBeetle_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyEggRobo_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyGrabber_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyBatabata_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyBeeton_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyELauncher_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyCrawler_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyGunHunter_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyCopSpeeder_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyMotora_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyGanigani_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyLander_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyEFighter_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyNal_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemySpinner_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyPawnBase_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyTaker_ProcessMessage);
    INSTALL_HOOK(EnemyTrigger_CEnemyBiter_ProcessMessage);

    // Handle MsgGetEnemyType
    INSTALL_HOOK(EnemyTrigger_CEnemyBase_ProcessMessage);
}