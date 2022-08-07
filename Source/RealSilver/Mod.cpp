#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "SetdataPatcher.h"

extern "C" void __declspec(dllexport) OnFrame()
{

}

HOOK(void, __fastcall, CSonicStateAirBoostBegin, 0x1233380, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCSonicStateAirBoostBegin(This);

    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    Eigen::Vector3f velocity = context->m_Velocity;
    velocity.y() = 0.0f;
    Common::SetPlayerVelocity(velocity);
}

void SetBoostModelVisible(bool visible)
{
    auto const& model = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_spCharacterModel;
    model->m_spModel->m_NodeGroupModels[0]->m_Visible = !visible;
    model->m_spModel->m_NodeGroupModels[1]->m_Visible = visible;
}

HOOK(int, __fastcall, MsgRestartStage, 0xE76810, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    SetBoostModelVisible(false);
    return originalMsgRestartStage(player, Edx, message);
}

HOOK(void, __fastcall, CSonicStatePluginBoostBegin, 0x1117A20, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCSonicStatePluginBoostBegin(This);
    SetBoostModelVisible(true);
}

HOOK(void, __fastcall, CSonicStatePluginBoostEnd, 0x1117900, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCSonicStatePluginBoostEnd(This);
    SetBoostModelVisible(false);
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    std::string dir = modInfo->CurrentMod->Path;

    size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        dir.erase(pos + 1);
    }

    if (!Configuration::load(dir))
    {
        MessageBox(NULL, L"Failed to parse Config.ini", NULL, MB_ICONERROR);
    }

    if (Configuration::m_blazeSupport)
    {
        ArchiveTreePatcher::applyPatches();
        SetdataPatcher::applyPatches();
    }

    // don't disable air action after air boost
    WRITE_NOP(0x12334D0, 3);

    // don't play boost screen effect
    WRITE_MEMORY(0x12334C1, uint8_t, 0x4);

    // infinite air boost time
    WRITE_MEMORY(0x123325E, uint8_t, 0xEB);

    // Cancel y-speed on air boost
    INSTALL_HOOK(CSonicStateAirBoostBegin);

    // Swap boost model
    INSTALL_HOOK(MsgRestartStage);
    INSTALL_HOOK(CSonicStatePluginBoostBegin);
    INSTALL_HOOK(CSonicStatePluginBoostEnd);
}