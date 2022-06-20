extern "C" void __declspec(dllexport) OnFrame()
{

}

void playRailSwitchVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002019, 30);
}

void playHangOnVoice() // also used on trick panel
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002020, 30);
}

void playJumpRampVoice() // boost only
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002021, 30);
}

void playHomingAttackVoice()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlayVoice(soundHandle, 3002029, 20);
}

void playTrickAnnouncer(int trickCount, bool isFinish)
{
	static SharedPtrTypeless soundHandle;
	if (isFinish)
	{
		Common::PlaySoundStatic(soundHandle, 3002022); // Amazing
	}
	else
	{
		switch (trickCount)
		{
		case 1:		Common::PlaySoundStatic(soundHandle, 3002026); break; // Good
		case 2:		Common::PlaySoundStatic(soundHandle, 3002025); break; // Great
		case 3:		Common::PlaySoundStatic(soundHandle, 3002024); break; // Awesome
		default:	Common::PlaySoundStatic(soundHandle, 3002023); break; // Outstanding
		}
	}
}

void playSuperSonicAnnouncer()
{
	static SharedPtrTypeless soundHandle;
	Common::PlaySoundStatic(soundHandle, 3002027);
}

void playGameOverAnnouncer()
{
	static SharedPtrTypeless soundHandle;
	Common::PlaySoundStatic(soundHandle, 3002028);
}

HOOK(void, __fastcall, CSonicStateGrindJumpSideBegin, 0x124A1E0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	originalCSonicStateGrindJumpSideBegin(This);
	if (Common::GetSonicStateFlags()->GrindSideJump)
	{
		playRailSwitchVoice();
	}
}

HOOK(int, __fastcall, CSonicStateHomingAttackBegin, 0x1232040, hh::fnd::CStateMachineBase::CStateBase* This)
{
	auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
	if (context->m_HomingAttackTargetActorID)
	{
		playHomingAttackVoice();
	}
	return originalCSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, CTrickSimpleAdvance, 0xE4B3F0, uint32_t* This)
{
	uint32_t trickCountPrev = This[35];
	uint32_t statePrev = This[24];
	originalCTrickSimpleAdvance(This);

	uint32_t trickCount = This[35]; 
	if (This[24] == 3 && statePrev != 3)
	{
		// Trick finish
		playTrickAnnouncer(trickCount, true);
	}
	else if (trickCount > trickCountPrev)
	{
		// Performed trick
		playTrickAnnouncer(trickCount, false);
	}
}

HOOK(void, __stdcall, CSonicContextStartHangOn, 0xE5F530, Sonic::Player::CPlayerSpeedContext* context, hh::fnd::Message& message)
{
	size_t* pHangOnActorID = (size_t*)((uint32_t)context + 0x7D8);
	bool hangOnPrev = *pHangOnActorID;

	originalCSonicContextStartHangOn(context, message);

	if (!hangOnPrev && *pHangOnActorID)
	{
		playHangOnVoice();
	}
}

HOOK(void, __fastcall, CObjAdlibTrickJumpStart, 0x1014FB0, void* This, void* Edx, hh::fnd::Message& message)
{
	originalCObjAdlibTrickJumpStart(This, Edx, message);

	auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
	if (message.m_SenderActorID == context->m_pPlayer->m_ActorID)
	{
		playHangOnVoice();
	}
}

HOOK(void, __fastcall, CSonicContextMsgApplyImpulse, 0xE6CFA0, Sonic::Player::CPlayerSpeedContext* context, void* Edx, uint32_t message)
{
	originalCSonicContextMsgApplyImpulse(context, Edx, message);

	switch (*(uint32_t*)(message + 0x38))
	{
	case 7: // JumpBoardSpecial
		playJumpRampVoice();
		break;
	}
}

HOOK(bool*, __fastcall, CSonicStatePluginSuperSonicBegin, 0x11D6840, hh::fnd::CStateMachineBase::CStateBase* This)
{
	playSuperSonicAnnouncer();
	return originalCSonicStatePluginSuperSonicBegin(This);
}

HOOK(int, __fastcall, CStateDisplayGameOverBegin, 0xCFDDF0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	playGameOverAnnouncer();
	return originalCStateDisplayGameOverBegin(This);
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	INSTALL_HOOK(CSonicStateGrindJumpSideBegin);
	INSTALL_HOOK(CSonicStateHomingAttackBegin);
	INSTALL_HOOK(CTrickSimpleAdvance);
	INSTALL_HOOK(CSonicContextStartHangOn);
	INSTALL_HOOK(CObjAdlibTrickJumpStart);
	INSTALL_HOOK(CSonicContextMsgApplyImpulse);
	INSTALL_HOOK(CSonicStatePluginSuperSonicBegin);
	INSTALL_HOOK(CStateDisplayGameOverBegin);
}