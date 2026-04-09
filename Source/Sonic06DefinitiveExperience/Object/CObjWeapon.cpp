#include "CObjWeapon.h"

std::vector<CObjWeapon::WeaponData> const c_data =
{
	// EggPawnGun
	{
		"weapon_eggpawngun"
	},
};

CObjWeapon::CObjWeapon
(
	boost::shared_ptr<hh::mr::CMatrixNode> parent, 
	Type type
)
: m_spNodeParent(parent)
, m_type(type)
, m_pData(&c_data[(int)type])
{
	
}

void CObjWeapon::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, 
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CGameObject3D::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);
	Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
	in_pGameDocument->AddUpdateUnit("0", this);

	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(m_pData->m_modelName.c_str(), 0);
	m_spNodeModel = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeModel->SetParent(m_spNodeParent.get());
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spNodeModel);
	AddRenderable("Object", m_spModel, true);
}

void CObjWeapon::UpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{

}
