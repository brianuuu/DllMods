
extern "C" void __declspec(dllexport) OnFrame()
{

}

void IncreaseSpeed(char const* targetState)
{
	auto context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	auto stateName = context->m_pPlayer->m_StateMachine.GetCurrentState()->GetStateName();
	if (std::string(stateName.c_str()) == std::string(targetState))
	{
		Eigen::Vector3f velocity;
		Common::GetPlayerVelocity(velocity);
		velocity = (velocity.norm() + 15.0f) * velocity.normalized();
		Common::SetPlayerVelocity(velocity);
	}
}

HOOK(void, __fastcall, CSonicStateSlidingAdvance, 0x11D69A0, int This)
{
	originalCSonicStateSlidingAdvance(This);
	IncreaseSpeed("RunQuickStep");
}

HOOK(void, __fastcall, CSonicStateStompingAdvance, 0x12548C0, int This)
{
	originalCSonicStateStompingAdvance(This);
	IncreaseSpeed("Sliding");
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	INSTALL_HOOK(CSonicStateSlidingAdvance);
	INSTALL_HOOK(CSonicStateStompingAdvance);
}