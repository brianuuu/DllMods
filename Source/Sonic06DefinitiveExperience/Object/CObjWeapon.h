/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: Weapon object used by Shadow
/*----------------------------------------------------------*/

#pragma once
class CObjWeapon : public Sonic::CGameObject3D
{
public:
	enum class Type
	{
		EggPawnGun,
	}; 
	
	struct WeaponData
	{
		std::string m_modelName;
	};

public:
	CObjWeapon(boost::shared_ptr<hh::mr::CMatrixNode> parent, Type type);

private:
	boost::shared_ptr<hh::mr::CMatrixNode> m_spNodeParent;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeModel;

	Type const m_type = Type::EggPawnGun;
	WeaponData const* m_pData = nullptr;

private:
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void UpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;

};