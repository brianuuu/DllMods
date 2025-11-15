/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Cars following path
/*----------------------------------------------------------*/
#include "Managers/PathManager.h"

#pragma once
class SoleannaCar : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("SoleannaCar");
	static void registerObject();

	virtual ~SoleannaCar() override;

	enum class Type
	{
		Compact,
		Sedan,
		Wagon,
		COUNT
	};

	enum class Color
	{
		White,
		Silver,
		Grey,
		Black,
		Red,
		Brown,
		Blue,
		Green,
		COUNT,
	};

private:
	struct Data
	{
		int m_Type = 0; // 0: random
		int m_Color = 0; // 0: random
		Sonic::CParamTypeList* m_PathName = nullptr;
		float m_PathStartProp = 0.0f;
		float m_Speed = 10.0f;
	} m_Data;

private:
	boost::shared_ptr<hh::mr::CSingleElement> m_spModel;
	boost::shared_ptr<Sonic::CRigidBody> m_spRigidBody;
	boost::shared_ptr<hh::mot::CSingleElementEffectMotionAll> m_spEffectMotionAll;

	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelFL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelFR;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelBL;
	boost::shared_ptr<hh::mr::CSingleElement> m_spModelWheelBR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelFL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelFR;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelBL;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeWheelBR;
	hh::math::CVector m_wheelFLPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelFRPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelBLPos = hh::math::CVector::Zero();
	hh::math::CVector m_wheelBRPos = hh::math::CVector::Zero();

	Type m_type = Type::Compact;
	Color m_color = Color::White;

	PathDataCollection m_path;
	PathFollowData m_followData;

	static char const* c_carNames[(int)Type::COUNT];
	static float c_carWheelRadius[(int)Type::COUNT];

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddRenderables(Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void AddCallback(const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder, Sonic::CGameDocument* in_pGameDocument, const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;
};

