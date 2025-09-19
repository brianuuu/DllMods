/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Inject new animations
/*----------------------------------------------------------*/

#pragma once
struct CAnimationStateInfo
{
    const char* m_Name;
    const char* m_FileName;
    float m_Speed;
    int32_t m_PlaybackType;
    int32_t field10;
    float field14;
    float field18;
    int32_t field1C;
    int32_t field20;
    int32_t field24;
    int32_t field28;
    int32_t field2C;
};

struct CAnimationStateSet
{
    CAnimationStateInfo* m_pEntries;
    size_t m_Count;
};

struct NewAnimationData
{
    char const* m_stateName;
    char const* m_fileName;
    float m_speed;
    bool m_isLoop;
    char const* m_destinationState;

    NewAnimationData
    (
        char const* _stateName, 
        char const* _fileName,
        float _speed,
        bool _isLoop,
        char const* _destinationState
    )
        : m_stateName(_stateName)
        , m_fileName(_fileName)
        , m_speed(_speed)
        , m_isLoop(_isLoop)
        , m_destinationState(_destinationState)
    {}
};

class AnimationSetPatcher
{
public:
    typedef std::vector<NewAnimationData> NewAnimationDataList;
    static NewAnimationDataList m_newAnimationData;
    static NewAnimationDataList m_newAnimationDataSuper;
    static void applyPatches();

    static void initializeAnimationList(CAnimationStateInfo* pEntries, size_t const count, NewAnimationDataList const& dataList);
    static void createAnimationState(void* A2, NewAnimationDataList const& dataList);

    // New animation states
    static const char* volatile const RunResult;
    static const char* volatile const RunResultLoop;
    static const char* volatile const BrakeFlip;

    static const char* volatile const SpinFall;
    static const char* volatile const SpinFallSpring;
    static const char* volatile const SpinFallLoop;

    static const char* volatile const HomingAttackLoop;

    static const char* volatile const AccelJumpLoop;
    static const char* volatile const FireTornadoLoop;
    static const char* volatile const FireTornadoEnd;

    static const char* volatile const GreenGemGround;
    static const char* volatile const GreenGemAir;
    static const char* volatile const SkyGem;
    static const char* volatile const SkyGemLoop;
    static const char* volatile const SkyGemEnd;
    static const char* volatile const FloatingBoost;

    static const char* volatile const SpinFast;
    static const char* volatile const ChaosAttack[5];
    static const char* volatile const ChaosAttackWait;
    static const char* volatile const SpinAttack[3];
    static const char* volatile const SpearWait;
    static const char* volatile const SpearWaitLoop;
    static const char* volatile const SpearShot;
    static const char* volatile const SpearShotLoop;
    static const char* volatile const SpearSuperWait;
    static const char* volatile const SpearSuperWaitLoop;
    static const char* volatile const SpearSuperShot;
    static const char* volatile const SpearSuperShotLoop;
    static const char* volatile const ChaosBoost;
    static const char* volatile const ChaosBoostLoop;
    static const char* volatile const ChaosBlastWait;
    static const char* volatile const ChaosBlastWaitLoop;
    static const char* volatile const ChaosBlast;
    static const char* volatile const ChaosBlastLoop;
};

