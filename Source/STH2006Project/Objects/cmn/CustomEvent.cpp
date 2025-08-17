#include "CustomEvent.h"

BB_SET_OBJECT_MAKE_HOOK(CustomEvent)
void CustomEvent::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(CustomEvent);
}

void CustomEvent::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	// Handle loading changed values
}

void CustomEvent::AddParameterBank
(
	const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank
)
{
}

void CustomEvent::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{

}

bool CustomEvent::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		if (message.Is<Sonic::Message::MsgNotifyObjectEvent>())
		{
			auto& msg = static_cast<Sonic::Message::MsgNotifyObjectEvent&>(message);
			switch (msg.m_Event)
			{
			case 0:
			{
				// Shadow start teleport begin
				S06DE_API::ToggleStartTeleport(true);
				break;
			}
			case 1:
			{
				// Shadow start teleport end
				S06DE_API::ToggleStartTeleport(false);
				break;
			}
			}
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}


