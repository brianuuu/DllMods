/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Generic object without using ObjectPhysics
/*----------------------------------------------------------*/

#pragma once
class GenericObject : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("GenericObject");
	static void registerObject();

private:
	struct Data
	{
		float m_CullingRange = 20.0f;
		Sonic::CParamTypeList* m_Name = nullptr;
		bool m_HasCollision = true;
		bool m_HasDamage = false;
	} m_Data;

private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
};

