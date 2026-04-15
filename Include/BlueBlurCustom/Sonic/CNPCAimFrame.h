#pragma once

namespace Sonic
{
	class CNPCAimFrame
	{
	public:
		BB_INSERT_PADDING(0xF0) {};
	};
	
	class CNPCAimFrameAxis : public CNPCAimFrame
	{
	public:
		Hedgehog::Math::CVector varF0;
	};
	
	BB_ASSERT_SIZEOF(CNPCAimFrame, 0xF0);
	BB_ASSERT_SIZEOF(CNPCAimFrameAxis, 0x100);
	
	struct CNPCAimFrameAxisData
	{
		uint16_t m_boneRotate;
		uint16_t m_boneTarget;
		float m_maxAngle;
		BB_INSERT_PADDING(0x8);
		Hedgehog::Math::CVector m_aimAxis;
		Hedgehog::Math::CVector m_unknownVector;
	};
	
	static void* fCNPCAimFrameAxisCreate
	(
		CNPCAimFrameAxis* pThis,
		CNPCAimFrameAxisData* pData,
		Hedgehog::Animation::CAnimationPose* pAnimPose,
		float slerpRate
	)
	{
		static void* const pCNPCAimFrameAxisCreate = (void*)0x1190120;
		__asm
		{
			push    slerpRate
			mov     ecx, pAnimPose
			mov     edi, pData
			mov     eax, pThis
			call	[pCNPCAimFrameAxisCreate]
		}
	}
	
	static void fCNPCAimFrameAxisPrepare
	(
		CNPCAimFrameAxis* pThis
	)
	{
		static void* const pCNPCAimFrameAxisPrepare = (void*)0x10FFB10;
		__asm
		{
			mov     edi, pThis
			call	[pCNPCAimFrameAxisPrepare]
		}
	}
	
	static void fCNPCAimFrameAxisAdvance
	(
		CNPCAimFrameAxis* pThis,
		Hedgehog::Math::CVector* pTarget,
		bool valid,
		float slerpRate
	)
	{
		static void* const pCNPCAimFrameAxisAdvance = (void*)0x10FF620;
		__asm
		{
			push    slerpRate
			push    valid
			push    pTarget
			mov     eax, pThis
			call	[pCNPCAimFrameAxisAdvance]
		}
	}
}