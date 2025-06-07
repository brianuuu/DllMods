/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Explosion effect for cmn_bombbox
/*----------------------------------------------------------*/

#pragma once
class Explosion : public Sonic::CObjectBase
{
private:
	float m_radius = 1.0f;
	float m_lifeTime = 0.0f;
	Hedgehog::Math::CVector m_position;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
	Explosion(float radius, Hedgehog::Math::CVector const& position) : m_radius(radius) , m_position(position) {}

	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

	static float GetDefaultRadius();
};
