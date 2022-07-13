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

FUNCTION_PTR(bool*, __thiscall, CSingleElementChangeMaterial, 0x701CC0, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* from, boost::shared_ptr<hh::mr::CMaterialData>& to);
FUNCTION_PTR(bool*, __thiscall, CSingleElementResetMaterial, 0x701830, Hedgehog::Mirage::CSingleElement* singleElement, hh::mr::CMaterialData* mat);

HOOK(void, __fastcall, CSonicStatePluginBoostBegin, 0x1117A20, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCSonicStatePluginBoostBegin(This);

    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    auto const& model = context->m_pPlayer->m_spCharacterModel;

    hh::mr::CMirageDatabaseWrapper wrapper(Sonic::CGameDocument::GetInstance()->m_pMember->m_spDatabase.get());
    boost::shared_ptr<hh::mr::CMaterialData> m1 = wrapper.GetMaterialData("chr_Silver_HD_body");
    boost::shared_ptr<hh::mr::CMaterialData> m2 = wrapper.GetMaterialData("chr_Silver_HD_cloth");
    boost::shared_ptr<hh::mr::CMaterialData> m3 = wrapper.GetMaterialData("chr_Silver_HD_fur1");
    boost::shared_ptr<hh::mr::CMaterialData> m4 = wrapper.GetMaterialData("chr_Silver_HD_fur2");
    boost::shared_ptr<hh::mr::CMaterialData> m5 = wrapper.GetMaterialData("chr_Silver_HD_fur3");
    boost::shared_ptr<hh::mr::CMaterialData> m6 = wrapper.GetMaterialData("chr_Silver_HD_fur4");
    boost::shared_ptr<hh::mr::CMaterialData> m7 = wrapper.GetMaterialData("chr_Silver_HD_fur5");
    boost::shared_ptr<hh::mr::CMaterialData> m8 = wrapper.GetMaterialData("chr_Silver_HD_fur6");
    boost::shared_ptr<hh::mr::CMaterialData> m9 = wrapper.GetMaterialData("chr_Silver_HD_fur7");
    boost::shared_ptr<hh::mr::CMaterialData> m10 = wrapper.GetMaterialData("chr_Silver_HD_fur8");
    boost::shared_ptr<hh::mr::CMaterialData> m11 = wrapper.GetMaterialData("chr_Silver_HD_mouth");
    boost::shared_ptr<hh::mr::CMaterialData> m12 = wrapper.GetMaterialData("chr_Silver_HD_shoe");
    boost::shared_ptr<hh::mr::CMaterialData> m13 = wrapper.GetMaterialData("chr_SilverHD_nose");
    boost::shared_ptr<hh::mr::CMaterialData> m14 = wrapper.GetMaterialData("chr_SilverHD_ring");
    //boost::shared_ptr<hh::mr::CMaterialData> m15 = wrapper.GetMaterialData("sonic_gm_eyeL");
    //boost::shared_ptr<hh::mr::CMaterialData> m16 = wrapper.GetMaterialData("sonic_gm_eyeR");
    boost::shared_ptr<hh::mr::CMaterialData> mt1 = wrapper.GetMaterialData("chr_Silver_boost_body");
    boost::shared_ptr<hh::mr::CMaterialData> mt2 = wrapper.GetMaterialData("chr_Silver_boost_cloth");
    boost::shared_ptr<hh::mr::CMaterialData> mt3 = wrapper.GetMaterialData("chr_Silver_boost_fur1");
    boost::shared_ptr<hh::mr::CMaterialData> mt4 = wrapper.GetMaterialData("chr_Silver_boost_fur2");
    boost::shared_ptr<hh::mr::CMaterialData> mt5 = wrapper.GetMaterialData("chr_Silver_boost_fur3");
    boost::shared_ptr<hh::mr::CMaterialData> mt6 = wrapper.GetMaterialData("chr_Silver_boost_fur4");
    boost::shared_ptr<hh::mr::CMaterialData> mt7 = wrapper.GetMaterialData("chr_Silver_boost_fur5");
    boost::shared_ptr<hh::mr::CMaterialData> mt8 = wrapper.GetMaterialData("chr_Silver_boost_fur6");
    boost::shared_ptr<hh::mr::CMaterialData> mt9 = wrapper.GetMaterialData("chr_Silver_boost_fur7");
    boost::shared_ptr<hh::mr::CMaterialData> mt10 = wrapper.GetMaterialData("chr_Silver_boost_fur8");
    boost::shared_ptr<hh::mr::CMaterialData> mt11 = wrapper.GetMaterialData("chr_Silver_boost_mouth");
    boost::shared_ptr<hh::mr::CMaterialData> mt12 = wrapper.GetMaterialData("chr_Silver_boost_shoe");
    boost::shared_ptr<hh::mr::CMaterialData> mt13 = wrapper.GetMaterialData("chr_Silver_boost_nose");
    boost::shared_ptr<hh::mr::CMaterialData> mt14 = wrapper.GetMaterialData("chr_Silver_boost_ring");
    //boost::shared_ptr<hh::mr::CMaterialData> mt15 = wrapper.GetMaterialData("chr_Silver_boost_eyeL");
    //boost::shared_ptr<hh::mr::CMaterialData> mt16 = wrapper.GetMaterialData("chr_Silver_boost_eyeR");
    CSingleElementChangeMaterial(model.get(), m1.get(), mt1);
    CSingleElementChangeMaterial(model.get(), m2.get(), mt2);
    CSingleElementChangeMaterial(model.get(), m3.get(), mt3);
    CSingleElementChangeMaterial(model.get(), m4.get(), mt4);
    CSingleElementChangeMaterial(model.get(), m5.get(), mt5);
    CSingleElementChangeMaterial(model.get(), m6.get(), mt6);
    CSingleElementChangeMaterial(model.get(), m7.get(), mt7);
    CSingleElementChangeMaterial(model.get(), m8.get(), mt8);
    CSingleElementChangeMaterial(model.get(), m9.get(), mt9);
    CSingleElementChangeMaterial(model.get(), m10.get(), mt10);
    CSingleElementChangeMaterial(model.get(), m11.get(), mt11);
    CSingleElementChangeMaterial(model.get(), m12.get(), mt12);
    CSingleElementChangeMaterial(model.get(), m13.get(), mt13);
    CSingleElementChangeMaterial(model.get(), m14.get(), mt14);
    //CSingleElementChangeMaterial(model.get(), m15.get(), mt15);
    //CSingleElementChangeMaterial(model.get(), m16.get(), mt16);
}

HOOK(void, __fastcall, CSonicStatePluginBoostEnd, 0x1117900, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCSonicStatePluginBoostEnd(This);

    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    auto const& model = context->m_pPlayer->m_spCharacterModel;

    hh::mr::CMirageDatabaseWrapper wrapper(Sonic::CGameDocument::GetInstance()->m_pMember->m_spDatabase.get());
    boost::shared_ptr<hh::mr::CMaterialData> m1 = wrapper.GetMaterialData("chr_Silver_HD_body");
    boost::shared_ptr<hh::mr::CMaterialData> m2 = wrapper.GetMaterialData("chr_Silver_HD_cloth");
    boost::shared_ptr<hh::mr::CMaterialData> m3 = wrapper.GetMaterialData("chr_Silver_HD_fur1");
    boost::shared_ptr<hh::mr::CMaterialData> m4 = wrapper.GetMaterialData("chr_Silver_HD_fur2");
    boost::shared_ptr<hh::mr::CMaterialData> m5 = wrapper.GetMaterialData("chr_Silver_HD_fur3");
    boost::shared_ptr<hh::mr::CMaterialData> m6 = wrapper.GetMaterialData("chr_Silver_HD_fur4");
    boost::shared_ptr<hh::mr::CMaterialData> m7 = wrapper.GetMaterialData("chr_Silver_HD_fur5");
    boost::shared_ptr<hh::mr::CMaterialData> m8 = wrapper.GetMaterialData("chr_Silver_HD_fur6");
    boost::shared_ptr<hh::mr::CMaterialData> m9 = wrapper.GetMaterialData("chr_Silver_HD_fur7");
    boost::shared_ptr<hh::mr::CMaterialData> m10 = wrapper.GetMaterialData("chr_Silver_HD_fur8");
    boost::shared_ptr<hh::mr::CMaterialData> m11 = wrapper.GetMaterialData("chr_Silver_HD_mouth");
    boost::shared_ptr<hh::mr::CMaterialData> m12 = wrapper.GetMaterialData("chr_Silver_HD_shoe");
    boost::shared_ptr<hh::mr::CMaterialData> m13 = wrapper.GetMaterialData("chr_SilverHD_nose");
    boost::shared_ptr<hh::mr::CMaterialData> m14 = wrapper.GetMaterialData("chr_SilverHD_ring");
    //boost::shared_ptr<hh::mr::CMaterialData> m15 = wrapper.GetMaterialData("sonic_gm_eyeL");
    //boost::shared_ptr<hh::mr::CMaterialData> m16 = wrapper.GetMaterialData("sonic_gm_eyeR");
    CSingleElementResetMaterial(model.get(), m1.get());
    CSingleElementResetMaterial(model.get(), m2.get());
    CSingleElementResetMaterial(model.get(), m3.get());
    CSingleElementResetMaterial(model.get(), m4.get());
    CSingleElementResetMaterial(model.get(), m5.get());
    CSingleElementResetMaterial(model.get(), m6.get());
    CSingleElementResetMaterial(model.get(), m7.get());
    CSingleElementResetMaterial(model.get(), m8.get());
    CSingleElementResetMaterial(model.get(), m9.get());
    CSingleElementResetMaterial(model.get(), m10.get());
    CSingleElementResetMaterial(model.get(), m11.get());
    CSingleElementResetMaterial(model.get(), m12.get());
    CSingleElementResetMaterial(model.get(), m13.get());
    CSingleElementResetMaterial(model.get(), m14.get());
    //CSingleElementResetMaterial(model.get(), m15.get());
    //CSingleElementResetMaterial(model.get(), m16.get());
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

    // Swap material
    INSTALL_HOOK(CSonicStatePluginBoostBegin);
    INSTALL_HOOK(CSonicStatePluginBoostEnd);
}