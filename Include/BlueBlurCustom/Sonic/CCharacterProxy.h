#pragma once

namespace hk2010_2_0
{
	class hkpCachingShapePhantom;

	class hkpPhantomListener
	{
	public:
		virtual ~hkpPhantomListener() = default;
	};
	class hkpEntityListener : public hkpPhantomListener
	{

	};
}

namespace Sonic
{
	class CPhantomCallback;
	class CCharacterProxy;

	class CAabbAndCastUnit : public CPhysicsUnit
	{
	public:
		hk2010_2_0::hkReferencedObject* m_pReferencedObject48;
		hk2010_2_0::hkpCachingShapePhantom* m_pCachingShapePhantom;
		boost::shared_ptr<CPhantomCallback> m_spPhantomCallback;
	};

	constexpr int pCCharacterProxyCtor = 0x010E3EF0;
	static void __declspec(naked) fCCharacterProxyCtor()
	{
		__asm
		{
			mov eax, edx
			jmp[pCCharacterProxyCtor]
		}
	}
	static BB_FUNCTION_PTR(void, __fastcall, fpCCharacterProxyCtor, fCCharacterProxyCtor,
		CCharacterProxy* This, Hedgehog::Universe::CMessageActor* in_pMessageActor, const Hedgehog::Base::THolder<CWorld>& worldHolder,
		hk2010_2_0::hkpShape** in_ppShape, const Hedgehog::Math::CVector& in_rPosition, const Hedgehog::Math::CQuaternion& in_rRotation,
		uint32_t in_CollisionID, const Hedgehog::Math::CVector& in_rWorldCenter);

	constexpr int pCCharacterProxySetPosition = 0x010E0230;
	static void __declspec(naked) fCCharacterProxySetPosition()
	{
		__asm
		{
			mov eax, edx
			mov esi, ecx
			jmp[pCCharacterProxySetPosition]
		}
	}

	static void fnCCharacterProxySetPosition(Sonic::CCharacterProxy* This, const Hedgehog::Math::CVector& in_rPosition)
	{
		__asm
		{
			mov eax, in_rPosition
			mov esi, This
			call[pCCharacterProxySetPosition]
		}
	}

	static BB_FUNCTION_PTR(void, __fastcall, fpCCharacterProxySetPosition, fCCharacterProxySetPosition,
		Sonic::CCharacterProxy* This, const Hedgehog::Math::CVector& in_rPosition);

	inline constexpr uint32_t pCCharacterProxyUpdateShapePhantomTransform = 0x10E0670;
	inline BB_NOINLINE void fpCCharacterProxyUpdateShapePhantomTransform(Sonic::CCharacterProxy* This)
	{
		__asm
		{
			mov	eax, This
			call[pCCharacterProxyUpdateShapePhantomTransform]
		}
	}

	constexpr int pCCharacterProxyAddPhantomCallback = 0x010C1810;
	static void __declspec(naked) fCCharacterProxyAddPhantomCallback()
	{
		__asm
		{
			mov eax, ecx
			jmp[pCCharacterProxyAddPhantomCallback]
		}
	}
	static BB_FUNCTION_PTR(void, __fastcall, fpCCharacterProxyAddPhantomCallback, fCCharacterProxyAddPhantomCallback, Sonic::CCharacterProxy* This);
	
	class CCharacterProxy : public CAabbAndCastUnit, public hk2010_2_0::hkpEntityListener, public hk2010_2_0::hkpPhantomListener
	{
	public:

		CCharacterProxy(const bb_null_ctor&) : CAabbAndCastUnit(), hkpEntityListener(), hkpPhantomListener() {}
		CCharacterProxy(
			Hedgehog::Universe::CMessageActor* in_pMessageActor,
			const Hedgehog::Base::THolder<CWorld>& worldHolder,
			hk2010_2_0::hkpShape* in_pShape,
			const Hedgehog::Math::CVector& in_rPosition,
			const Hedgehog::Math::CQuaternion& in_rRotation,
			uint32_t in_CollisionID) : CCharacterProxy(bb_null_ctor{})
		{
			const Hedgehog::Math::CVector worldCenter = worldHolder->m_pMember->m_spPhysicsWorld
				? worldHolder->m_pMember->m_spPhysicsWorld->m_WorldCenter
				: Hedgehog::Math::CVector(0, 0, 0);
			fpCCharacterProxyCtor(this, in_pMessageActor, worldHolder, &in_pShape, in_rPosition, in_rRotation, in_CollisionID, worldCenter);
		}

		void SetPosition(const Hedgehog::Math::CVector& in_rPosition)
		{
			fnCCharacterProxySetPosition(this, in_rPosition);
		}

		void SetRotation(hh::math::CQuaternion in_rRotation)
		{
			BB_FUNCTION_PTR(void, __thiscall, fpQuaternionNormalize, 0x4077F0, hh::math::CQuaternion& in_rRotation);
			fpQuaternionNormalize(in_rRotation);
			BB_FUNCTION_PTR(int, __thiscall, fpCCharacterProxySetRotation, 0x8724C0, Eigen::Matrix<float, 4, 3>& in_rHKRotation, hh::math::CQuaternion const& in_rRotation);
			fpCCharacterProxySetRotation(m_hkRotation, in_rRotation);
			m_Field1D0 = in_rRotation;
			fpCCharacterProxyUpdateShapePhantomTransform(this);
		}

		void AddPhantomCallback()
		{
			fpCCharacterProxyAddPhantomCallback(this);
		}

		float var60;
		int var64;
		float m_Float68;
		float var6c;
		float var70;
		float var74;
		float var78;
		float var7c;
		float var80;
		float cosAngleThing;
		int var88;
		int var8c;
		Hedgehog::Math::CVector m_UpVector;
		float varA0;
		int varA4;
		float varA8;
		float varAc;
		int m_DamageID;
		int varB4;
		int varB8;
		int varBc;
		Hedgehog::Math::CVector varC0;
		Hedgehog::Math::CVector varD0;
		Hedgehog::Math::CVector varE0;
		int varF0;
		int varF4;
		int varF8;
		int varFc;
		int var100;
		int var104;
		int var108;
		int var10c;
		Hedgehog::Math::CVector var110;
		Hedgehog::Math::CVector m_Position;
		Hedgehog::Math::CVector m_Velocity;
		Hedgehog::Math::CVector var140;
		float m_VelocityStrengthScale;
		int var154;
		int var158;
		int var15c;
		Eigen::Matrix<float, 4, 3> m_hkRotation; // 0x160
		Eigen::Matrix<float, 4, 3> m_hkRotationDefault; // 0x190
		Hedgehog::Math::CQuaternion m_Field1C0; // default rotation?
		Hedgehog::Math::CQuaternion m_Field1D0; // new rotation?
		Hedgehog::Math::CVector m_Field1E0;
		float m_Field1F0;
		int m_Field1F4;
		int m_Field1F8;
		int m_Field1FC;
	};
	BB_ASSERT_SIZEOF(Sonic::CCharacterProxy, 0x200);
}