HOOK(void, __fastcall, CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    originalCSonicUpdate(This, Edx, dt);

    if (Common::CheckCurrentStage("pam000") ||
        Common::CheckCurrentStage("pam001") ||
        Common::CheckCurrentStage("fig000"))
    {
        return;
    }

    FUNCTION_PTR(void, __thiscall, processPlayerMsgPlayerGoal, 0xE6C2C0, void* This, void* message);
    Sonic::SPadState* padState = Sonic::CInputState::GetPadState();
    if ((padState->IsDown(Sonic::EKeyState::eKeyState_RightStick) && padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStick)) ||
        (padState->IsDown(Sonic::EKeyState::eKeyState_LeftStick) && padState->IsTapped(Sonic::EKeyState::eKeyState_RightStick)) ||
        padState->IsDown(Sonic::EKeyState::eKeyState_LeftBumper) && padState->IsDown(Sonic::EKeyState::eKeyState_RightBumper) && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
    {
        void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
        processPlayerMsgPlayerGoal(player, nullptr);
    }
}

extern "C" void __declspec(dllexport) OnFrame()
{

}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    INSTALL_HOOK(CSonicUpdate);
}