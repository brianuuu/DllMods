#pragma once

#include <Hedgehog/Universe/Engine/hhMessage.h>

namespace Sonic::Message
{
    class __declspec(align(4)) MsgStartExternalControl : public Hedgehog::Universe::MessageTypeSet
	{
	public:
		HH_FND_MSG_MAKE_TYPE(0x01680810);

		boost::shared_ptr<CMatrixNodeTransform> m_spControlNode;
		bool TerrainCollisionEnable;
		bool ForceGroundMaybe;
		bool ChangeToSpin;
		bool DisableSuperSonic = false;
		bool ObserveBoost = false;
		bool ObserveSpin = false;
		bool ObserveInput = false;
		bool EscapeEnabled = false;
		bool NoDamage = false;
		int ChangeCollisionFlags;
		int DamageType;
		char m_Field_2C = 1;
		char m_Field_2D = 0;
		char m_Field_2E = 1;
		char m_Field_2F = 0;

		MsgStartExternalControl(
			const boost::shared_ptr<CMatrixNodeTransform>& in_spControlNode,
			bool in_TerrainCollisionEnable, bool in_ForceGroundMaybe, bool in_ChangeToSpin, int in_ChangeCollisionFlags
		)
		: m_spControlNode(in_spControlNode),
		  TerrainCollisionEnable(in_TerrainCollisionEnable),
		  ForceGroundMaybe(in_ForceGroundMaybe),
		  ChangeToSpin(in_ChangeToSpin),
		  ChangeCollisionFlags(in_ChangeCollisionFlags),
		  DamageType(*reinterpret_cast<int*>(0x01E61B5C))
		{}

		MsgStartExternalControl(
			const boost::shared_ptr<CMatrixNodeTransform>& in_spControlNode,
			bool in_TerrainCollisionEnable, bool in_ForceGroundMaybe, bool in_ChangeToSpin
		)
		: m_spControlNode(in_spControlNode),
		  TerrainCollisionEnable(in_TerrainCollisionEnable),
		  ForceGroundMaybe(in_ForceGroundMaybe),
		  ChangeToSpin(in_ChangeToSpin),
		  ChangeCollisionFlags(0),
		  DamageType(*reinterpret_cast<int*>(0x01E61B5C))
		{}

		MsgStartExternalControl(
			const boost::shared_ptr<CMatrixNodeTransform>& in_spControlNode,
			bool in_TerrainCollisionEnable, bool in_ForceGroundMaybe
		)
		: m_spControlNode(in_spControlNode),
		  TerrainCollisionEnable(in_TerrainCollisionEnable),
		  ForceGroundMaybe(in_ForceGroundMaybe),
		  ChangeToSpin(false),
		  ChangeCollisionFlags(0),
		  DamageType(*reinterpret_cast<int*>(0x01E61B5C))
		{}

		// These aren't in the base game, but can prove useful.
		MsgStartExternalControl(
			const boost::shared_ptr<CMatrixNodeTransform>& in_spControlNode,
			bool in_TerrainCollisionEnable, bool in_ForceGroundMaybe, bool in_ChangeToSpin, int in_ChangeCollisionFlags,
			int in_DamageType
		)
		: m_spControlNode(in_spControlNode),
		  TerrainCollisionEnable(in_TerrainCollisionEnable),
		  ForceGroundMaybe(in_ForceGroundMaybe),
		  ChangeToSpin(in_ChangeToSpin),
		  ChangeCollisionFlags(in_ChangeCollisionFlags),
		  DamageType(in_DamageType)
		{}

		MsgStartExternalControl(
			const boost::shared_ptr<CMatrixNodeTransform>& in_spControlNode
		)
		: m_spControlNode(in_spControlNode),
		  TerrainCollisionEnable(true),
		  ForceGroundMaybe(false),
		  ChangeToSpin(false),
		  ChangeCollisionFlags(0),
		  DamageType(*reinterpret_cast<int*>(0x01E61B5C))
		{}
	};
}