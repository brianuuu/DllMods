///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Recreating 123 Jump from 06
/*----------------------------------------------------------*/

#pragma once
class JumpChain : public Sonic::CObjectBase, public Sonic::CSetObjectListener
{
public:
	BB_SET_OBJECT_MAKE("JumpChain");
	static void registerObject();

	virtual ~JumpChain() override;

	enum class State
	{
		Idle,
		Cling,
		Launch,
		LaunchFar,
		SquatEnd,
	};

private:
	struct Data
	{
		Sonic::CParamTargetList* m_TargetList;
		float m_LaunchSpeed;
		float m_SquatEndSpeed; // >0: mach speed
		float m_FailOutOfControl;
		bool m_AutoStart;
	} m_Data;

private:
	int m_state;
	float m_timer;
	uint32_t m_playerID;
	int m_targetIndex;

	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;
	boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spSonicControlNode;

public:
	void InitializeEditParam(Sonic::CEditParam& in_rEditParam) override;
	bool SetAddColliders(const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase) override;
	void SetUpdateParallel(const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo) override;
	bool ProcessMessage(Hedgehog::Universe::Message& message, bool flag) override;

private:
	void StartCling();
	void ClingToTarget();
	bool CanAutoJump();
};

