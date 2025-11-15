#include "SoleannaCar.h"

#include "System/Application.h"

BB_SET_OBJECT_MAKE_HOOK(SoleannaCar)
void SoleannaCar::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(SoleannaCar);
}

SoleannaCar::~SoleannaCar()
{
	if (m_Data.m_PathName)
	{
		m_Data.m_PathName->Release();
	}
}

char const* SoleannaCar::c_carNames[(int)Type::COUNT] =
{
	"twn_car_compact",
	"twn_car_sedan",
	"twn_car_wagon",
};

float SoleannaCar::c_carWheelRadius[(int)Type::COUNT] =
{
	0.28f, 0.33f, 0.33f
};

void SoleannaCar::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamInt(&m_Data.m_Type, "Type");
	in_rEditParam.CreateParamInt(&m_Data.m_Color, "Color");

	uint32_t dummy = 0u;
	char const* pathName = "PathName";
	m_Data.m_PathName = Sonic::CParamTypeList::Create(&dummy, pathName);
	m_Data.m_PathName->m_pMember->m_DefaultValue = 1;
	m_Data.m_PathName->m_pMember->m_pFuncData->m_ValueMax = 1;
	if (m_Data.m_PathName)
	{
		m_Data.m_PathName->AddRef();
	}
	in_rEditParam.CreateParamBase(m_Data.m_PathName, pathName);

	in_rEditParam.CreateParamFloat(&m_Data.m_PathStartProp, "PathStartProp");
	in_rEditParam.CreateParamFloat(&m_Data.m_Speed, "Speed");
}

bool SoleannaCar::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	if (m_Data.m_Type == 0)
	{
		m_type = (Type)(rand() % (int)Type::COUNT);
	}
	else
	{
		m_type = (Type)(m_Data.m_Type - 1);
	}
	
	if (m_Data.m_Color == 0)
	{
		m_color = (Color)(rand() % (int)Color::COUNT);
	}
	else
	{
		m_color = (Color)(m_Data.m_Color - 1);
	}
	
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());

	// base
	boost::shared_ptr<hh::mr::CModelData> spModelBaseData = wrapper.GetModelData(c_carNames[(int)m_type], 0);
	m_spModel = boost::make_shared<hh::mr::CSingleElement>(spModelBaseData);
	m_spModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spModel, m_pMember->m_CastShadow);

	// wheels
	boost::shared_ptr<hh::mr::CModelData> spModelWheelLData = wrapper.GetModelData((c_carNames[(int)m_type] + std::string("_wheelL")).c_str(), 0);
	boost::shared_ptr<hh::mr::CModelData> spModelWheelRData = wrapper.GetModelData((c_carNames[(int)m_type] + std::string("_wheelR")).c_str(), 0);
	m_spNodeWheelFL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelFR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelBL = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelBR = boost::make_shared<Sonic::CMatrixNodeTransform>();
	m_spNodeWheelFL->SetParent(m_spModel->GetNode("Wheel_F_L").get());
	m_spNodeWheelFR->SetParent(m_spModel->GetNode("Wheel_F_R").get());
	m_spNodeWheelBL->SetParent(m_spModel->GetNode("Wheel_B_L").get());
	m_spNodeWheelBR->SetParent(m_spModel->GetNode("Wheel_B_R").get());
	m_spModelWheelFL = boost::make_shared<hh::mr::CSingleElement>(spModelWheelLData);
	m_spModelWheelFR = boost::make_shared<hh::mr::CSingleElement>(spModelWheelRData);
	m_spModelWheelBL = boost::make_shared<hh::mr::CSingleElement>(spModelWheelLData);
	m_spModelWheelBR = boost::make_shared<hh::mr::CSingleElement>(spModelWheelRData);
	m_spModelWheelFL->BindMatrixNode(m_spNodeWheelFL);
	m_spModelWheelFR->BindMatrixNode(m_spNodeWheelFR);
	m_spModelWheelBL->BindMatrixNode(m_spNodeWheelBL);
	m_spModelWheelBR->BindMatrixNode(m_spNodeWheelBR);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelFL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelFR, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelBL, m_pMember->m_CastShadow);
	Sonic::CGameObject::AddRenderable("Object", m_spModelWheelBR, m_pMember->m_CastShadow);

	// mat-anim
	m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
	m_spModel->BindEffect(m_spEffectMotionAll);

	FUNCTION_PTR(void, __thiscall, fpGetMaterialAnimData, 0x759720,
		hh::mot::CMotionDatabaseWrapper const& wrapper,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData>&materialAnimData,
		hh::base::CSharedString const& name,
		uint32_t flag
	);

	FUNCTION_PTR(void, __thiscall, fpCreateMatAnim, 0x753910,
		Hedgehog::Motion::CSingleElementEffectMotionAll * This,
		boost::shared_ptr<hh::mr::CModelData> const& modelData,
		boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> const& materialAnimData
	);

	hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
	boost::shared_ptr<Hedgehog::Motion::CMaterialAnimationData> materialAnimData;
	fpGetMaterialAnimData(motWrapper, materialAnimData, (c_carNames[(int)m_type] + std::string("_color")).c_str(), 0);
	fpCreateMatAnim(m_spEffectMotionAll.get(), spModelBaseData, materialAnimData);

	FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
	fpUpdateMotionAll(m_spEffectMotionAll.get(), (float)m_color);

	SetCullingRange(0.0f);

	return true;
}

bool SoleannaCar::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	// Rigid body
	char const* rigidBodyName = c_carNames[(int)m_type];
	AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x1E0AFF4, m_spMatrixNodeTransform, in_spDatabase);

	return true;
}

void SoleannaCar::AddCallback
(
	const Hedgehog::Base::THolder<Sonic::CWorld>& in_rWorldHolder,
	Sonic::CGameDocument* in_pGameDocument,
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	Sonic::CObjectBase::AddCallback(in_rWorldHolder, in_pGameDocument, in_spDatabase);

	m_wheelFLPos = m_spNodeWheelFL->GetWorldMatrix().translation();
	m_wheelFRPos = m_spNodeWheelFR->GetWorldMatrix().translation();
	m_wheelBLPos = m_spNodeWheelBL->GetWorldMatrix().translation();
	m_wheelBRPos = m_spNodeWheelBR->GetWorldMatrix().translation();

	if (!m_Data.m_PathName->m_pMember->m_DefaultValueName.empty())
	{
		bool const valid = PathManager::parsePathXml(m_path, false, (Application::getModDirString() + "Assets\\Stage\\" + m_Data.m_PathName->m_pMember->m_DefaultValueName.c_str() + ".path.xml").c_str()) == tinyxml2::XML_SUCCESS;
		if (!valid || m_path.empty())
		{
			MessageBox(NULL, L"Failed to parse Glider path", NULL, MB_ICONERROR);
			Kill();
			return;
		}

		m_followData.m_yawOnly = false;
		m_followData.m_speed = m_Data.m_Speed;
		m_followData.m_loop = true;
		m_followData.m_pPathData = &m_path[0];

		m_followData.m_rotation = Eigen::Quaternionf::Identity();
		m_followData.m_position = m_path[0].m_knots[0].m_point;

		if (m_Data.m_PathStartProp > 0.0f)
		{
			PathManager::followSetProp(m_followData, m_Data.m_PathStartProp);
		}

		m_spMatrixNodeTransform->m_Transform.SetPosition(m_followData.m_position);
		m_spMatrixNodeTransform->NotifyChanged();
	}
}

void SoleannaCar::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
	if (!m_Data.m_PathName->m_pMember->m_DefaultValueName.empty())
	{
		PathManager::followAdvance(m_followData, in_rUpdateInfo.DeltaTime);

		m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(m_followData.m_rotation, m_followData.m_position);
		m_spMatrixNodeTransform->NotifyChanged();
	}

	if (m_Data.m_Speed != 0.0f)
	{
		// wheels
		auto fnWheelSpin = [this](Sonic::CMatrixNodeTransform* transform, hh::math::CVector& pos)
		{
			hh::math::CVector const newPos = transform->GetWorldMatrix().translation();
			if ((newPos - pos).isApprox(hh::math::CVector::Zero()))
			{
				return;
			}

			float const dist = (newPos - pos).norm();
			float const wheelRadius = c_carWheelRadius[(int)m_type];
			float const theta = dist / wheelRadius;
			hh::math::CQuaternion const newRotation = Eigen::AngleAxisf(theta, hh::math::CVector::UnitX()) * transform->m_Transform.m_Rotation;
			transform->m_Transform.SetRotation(newRotation);
			transform->NotifyChanged();
			pos = newPos;
		};

		fnWheelSpin(m_spNodeWheelFL.get(), m_wheelFLPos);
		fnWheelSpin(m_spNodeWheelFR.get(), m_wheelFRPos);
		fnWheelSpin(m_spNodeWheelBL.get(), m_wheelBLPos);
		fnWheelSpin(m_spNodeWheelBR.get(), m_wheelBRPos);
	}
}

bool SoleannaCar::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)
{
	if (flag)
	{
		
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}