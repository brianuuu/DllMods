/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Vulkan object used by enemies
/*----------------------------------------------------------*/

#pragma once
class EnemyBullet : public Sonic::CObjectBase
{
private:
	hh::mr::CTransform m_startTrans;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;

	uint32_t m_owner = 0;
	uint32_t m_frames = 0;
	float m_lifetime = 0.0f;

public:
	EnemyBullet(uint32_t owner, hh::mr::CTransform const& startTrans) : m_owner(owner), m_startTrans(startTrans) {}

	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

private:
};