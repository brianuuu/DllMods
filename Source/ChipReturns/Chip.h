#pragma once
enum StartMode
{
	Stand,
	CrouchingStartFront,
	INVALID
};

struct MsgNotifyObjectEvent
{
	INSERT_PADDING(0x10);
	uint32_t event;
};

class Chip
{
public:
	static void applyPatches();

	// Setdata injection
	static bool generateChipSetdata(std::set<uint32_t> const& usedSetObjectIDs, std::string& chipSetdataStr);
	static void addChipRankObjectPhysics(std::string& chipSetdataStr, std::string const& type, uint32_t const setObjectID);
	static std::string getChipRankAnimationName(uint32_t rank);
	static std::string getRotationStrFromYaw(float yaw);

	// Start animation related members
	static StartMode m_startMode;
	static Eigen::Vector3f m_sonicStartPos;

	// Eigen to setdata
	static std::string to_string(Eigen::Vector3f v);
	static std::string to_string(Eigen::Quaternionf q);

	// Chip objects
	enum ChipState
	{
		Follow,
		IdleA,
		IdleB,
		IdleMove,
		Move,
		MoveIdle,
		Result,
		None,
	};

	enum ChipEyeState
	{
		Stop = 0,
		Sonic_S,
		Sonic_A,
		Sonic_B,
		Sonic_C,
		Sonic_D,
		Sonic_E,
		Evil_S,
		Evil_A,
		Evil_B,
		Evil_C,
		Evil_D,
		Evil_E,
	};

	// Eye uv-anim
	struct ChipEye
	{
		uint32_t gap0[1];
		uint32_t* pData;
		uint32_t gap2[3];
		float frameTime;
	};
	static ChipEye* m_chipEyeL;
	static ChipEye* m_chipEyeR;
	static std::vector<float> m_chipEyeEndTimes;
	static void setChipEyeState(ChipEyeState state);

	struct ChipResult
	{
		uint32_t* m_pResult;
		std::map<std::string, uint32_t*> m_pResultList;
		ChipState m_state;
		bool m_teleported;
		int m_rank;

		ChipResult()
		{
			reset();
		}

		void reset()
		{
			m_pResult = nullptr;
			m_pResultList.clear();
			m_state = ChipState::None;
			m_teleported = false;
			m_rank = -1; // -1: not started
		}

		void advanceResult();
	};
	static ChipResult m_chipResult;

	struct ChipFollow
	{
		// Objects
		uint32_t* m_pObject;

		// State
		ChipState m_state;
		float m_frameTime;

		// Idle
		float m_idleTimer;

		// Random movement
		float m_newPosTimer;
		Eigen::Vector3f m_randPosAdd;
		Eigen::Vector3f m_randPosTarget;

		// Actual transform
		Eigen::Vector3f m_position;
		Eigen::Quaternionf m_rotation;

		// Transform add-on
		bool m_delayStart;
		bool m_goalLeave;
		float m_goalLeaveYAdd;

		ChipFollow() 
		{
			reset();
		}

		void reset()
		{
			m_pObject = nullptr;

			m_state = ChipState::Follow;
			m_frameTime = FLT_MAX;

			m_idleTimer = 0.0f;

			m_newPosTimer = 2.0f;
			m_randPosAdd = Eigen::Vector3f(0.7f, 1.2f, -0.7f);
			m_randPosTarget = Eigen::Vector3f(0.7f, 1.2f, -0.7f);

			m_delayStart = false;
			m_goalLeave = false;
		}

		void advanceFollowSonic(float dt);
	};
	static ChipFollow m_chipFollow;
	static float updateGetFrameTime(void* pObject, float setFrameTime = -1.0f);

	// Transformation
	static bool getSonicTransform(Eigen::Vector3f& position, Eigen::Quaternionf& rotation);
	static float getSonicSpeedSquared();
	static void teleportTo(void* objPtr, Eigen::Vector3f position, Eigen::Quaternionf rotation);
};

