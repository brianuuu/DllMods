/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Handles special events in STH2006
/*----------------------------------------------------------*/

#pragma once
class CustomEvent : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("CustomEvent");
	static void registerObject();

	virtual ~CustomEvent() override;

private:
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddParameterBank(const Hedgehog::Base::CRefPtr<Sonic::CParameterBank>& in_rParameterBank) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

	bool m_disableBoost = false;
	bool m_chaosBoostCanLevelDown = true;
	bool m_chaosBoostMatchMaxLevel = false;
	uint32_t m_chaosBoostMaxLevel = 3;

	void SetDisableBoost(bool disable);
};

