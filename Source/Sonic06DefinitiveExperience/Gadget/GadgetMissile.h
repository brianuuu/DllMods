/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Gadget_Missile object from 06
/*----------------------------------------------------------*/

#pragma once
class GadgetMissile : public Sonic::CObjectBase
{
private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEffect;

	uint32_t m_owner = 0;
	std::vector<uint32_t> m_targets;
	float m_lifetime = 0.0f;
	hh::math::CVector m_velocity = hh::math::CVector::Zero();

public:
	GadgetMissile(uint32_t owner, hh::mr::CTransform const& startTrans);

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

private:
};

