#include "TrickJumper.h"
#include "AnimationSetPatcher.h"

BB_SET_OBJECT_MAKE_HOOK(TrickJumper)

std::map<int, std::map<int, float>> const c_uiAppearSamples =
{
    { 10, {
        { 25, 0.701f },
        { 35, 0.701f },
        { 45, 0.701f },
        { 55, 0.701f },
        { 65, 0.701f },
        { 75, 0.701f },
        { 85, 0.701f }
     } },

    { 20, {
        { 25, 0.701f },
        { 35, 0.701f },
        { 45, 0.701f },
        { 55, 0.751f },
        { 65, 0.817f },
        { 75, 0.851f },
        { 85, 0.868f },
    } },

    { 30, {
        { 25, 0.701f },
        { 35, 0.784f },
        { 45, 0.918f },
        { 55, 1.001f },
        { 65, 1.068f },
        { 75, 1.118f },
        { 85, 1.151f },
    } },

    { 40, {
        { 25, 0.784f },
        { 35, 0.951f },
        { 45, 1.101f },
        { 55, 1.218f },
        { 65, 1.335f },
        { 75, 1.401f },
        { 85, 1.418f },
    } },

    { 50, {
        { 25, 0.901f },
        { 35, 1.118f },
        { 45, 1.301f },
        { 55, 1.485f },
        { 65, 1.585f },
        { 75, 1.668f },
        { 85, 1.718f },
    } },

    { 60, {
        { 25, 1.034f },
        { 35, 1.285f },
        { 45, 1.501f },
        { 55, 1.685f },
        { 65, 1.852f },
        { 75, 1.952f },
        { 85, 1.985f },
    } },
};

float const c_qteUiButtonXSpacing = 115.0f;
float const c_qteUiButtonYSpacing = 75.0f;
float const c_qteUiButtonSpamXPos = 584.0f;
float const c_qteUiButtonYPos = 360.0f;
float const c_qteUiSlowTimeScale = 0.075f;
float const c_qteUiSlowTimeTime = 0.6f; // how long it takes to slow time (linear)
float const c_qteUiSlowTimeFixed = 0.17f; // in-game time when time is fully slowed

boost::shared_ptr<Sonic::CGameObject> m_spTrickJumperUI;
class CTrickJumperUI : public Sonic::CGameObject
{
private:
	TrickJumper::Data m_data;
    float m_lifeTime;
    float m_uiAppearTime;

    hh::math::CVector m_direction;
    hh::math::CVector m_impulsePos;

	Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcQTE;
	boost::shared_ptr<Sonic::CGameObjectCSD> m_spQTE;

	enum ButtonType { A, B, X, Y, LB, RB, COUNT };
	struct Button
	{
		Button() : m_type(ButtonType::X) {}

		ButtonType m_type;
		Chao::CSD::RCPtr<Chao::CSD::CScene> m_scene;
		Chao::CSD::RCPtr<Chao::CSD::CScene> m_effect;
	};

	struct Sequence
	{
		Sequence() : m_time(5.0f), m_spamCount(0) {}

		float m_time;
		std::vector<Button> m_buttons;

		size_t m_spamCount;
		Chao::CSD::RCPtr<Chao::CSD::CScene> m_boss;

		Chao::CSD::RCPtr<Chao::CSD::CScene> m_bg;
		Chao::CSD::RCPtr<Chao::CSD::CScene> m_timer;
	};

	Chao::CSD::RCPtr<Chao::CSD::CScene> m_txt1; // nice
	Chao::CSD::RCPtr<Chao::CSD::CScene> m_txt2; // great
	Chao::CSD::RCPtr<Chao::CSD::CScene> m_txt3; // cool
	Chao::CSD::RCPtr<Chao::CSD::CScene> m_txt4; // you failed
	size_t m_txtID;

	std::deque<Sequence> m_sequences;
	size_t m_sequenceID;
	size_t m_buttonID;

    enum State
    {
        S_SlowTime,
        S_Intro,
        S_Input,
        S_Outro,
        S_Outro2,
        S_Finished,
    } m_state;

public:
	CTrickJumperUI(TrickJumper::Data const& data, hh::math::CVector const& direction)
		: m_data(data)
        , m_lifeTime(0.0f)
        , m_direction(direction)
        , m_txtID(3u)
        , m_sequenceID(0u)
        , m_buttonID(0u)
        , m_state(State::S_SlowTime)
	{
        int minSpeed = 10;
        for (int i = 50; i >= 10; i -= 10)
        {
            if (i <= (int)m_data.m_Speed[0])
            {
                minSpeed = i;
                break;
            }
        }

        int maxSpeed = 60;
        for (int i = 20; i <= 60; i += 10)
        {
            if (i > (int)m_data.m_Speed[0])
            {
                maxSpeed = i;
                break;
            }
        }

        int minPitch = 25;
        for (int i = 75; i >= 25; i -= 10)
        {
            if (i <= (int)m_data.m_Pitch[0])
            {
                minPitch = i;
                break;
            }
        }

        int maxPitch = 85;
        for (int i = 35; i <= 85; i += 10)
        {
            if (i > (int)m_data.m_Pitch[0])
            {
                maxPitch = i;
                break;
            }
        }

        float const minSpeedF = (float)minSpeed;
        float const maxSpeedF = (float)maxSpeed;
        float const minPitchF = (float)minPitch;
        float const maxPitchF = (float)maxPitch;

        float const x1y1 = c_uiAppearSamples.at(minSpeed).at(minPitch);
        float const x2y1 = c_uiAppearSamples.at(maxSpeed).at(minPitch);
        float const x1y2 = c_uiAppearSamples.at(minSpeed).at(maxPitch);
        float const x2y2 = c_uiAppearSamples.at(maxSpeed).at(maxPitch);

        float const speedProp = (m_data.m_Speed[0] - minSpeedF) / (maxSpeedF - minSpeedF);
        float const pitchProp = (m_data.m_Pitch[0] - minPitchF) / (maxPitchF - minPitchF);

        float const xy1 = x1y1 + (x2y1 - x1y1) * speedProp;
        float const xy2 = x1y2 + (x2y2 - x1y2) * speedProp;
        m_uiAppearTime = xy1 + (xy2 - xy1) * pitchProp;
	}

	~CTrickJumperUI()
	{
        ResetTime();

        if (m_spQTE)
        {
            m_spQTE->SendMessage(m_spQTE->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
            m_spQTE = nullptr;
        }

        // clean up
        for (Sequence& sequence : m_sequences)
        {
            for (Button& button : sequence.m_buttons)
            {
                Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), button.m_effect);
                Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), button.m_scene);
            }
            Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), sequence.m_bg);
            Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), sequence.m_timer);
            Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), sequence.m_boss);
        }
        Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), m_txt1);
        Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), m_txt2);
        Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), m_txt3);
        Chao::CSD::CProject::DestroyScene(m_rcQTE.Get(), m_txt4);
        m_rcQTE = nullptr;
	}

	void AddCallback
	(
		const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
		Sonic::CGameDocument* pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
	) override
	{
        // Update unit 1 is unaffected by time slowing down
        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("1", this);

        for (int i = 0; i < 3; i++)
        {
            if (m_data.m_TrickCount[i] == 0 || m_data.m_TrickTime[i] == 0.0f)
            {
                break;
            }

            // create sequence
            CreateSequence(m_data.m_TrickCount[i], m_data.m_TrickTime[i]);
        }

        // wrong params, default to something
        if (m_sequences.empty())
        {
            // create a completely random sequence
            size_t sequenceCount = 1;// (rand() % 3) + 1;
            while (m_sequences.size() < sequenceCount)
            {
                size_t buttonCount = (rand() % 3) + 3; // [3-5] buttons
                CreateSequence(buttonCount, 3.0f);
            }
        }

#if _DEBUG
        for (uint32_t s = 0; s < m_sequences.size(); s++)
        {
            Sequence const& sequence = m_sequences[s];
            char const* buttonNames[] = { "A", "B", "X", "Y", "LB", "RB" };
            std::string str;
            for (uint32_t i = 0; i < sequence.m_buttons.size(); i++)
            {
                if (i > 0) str += ",";
                str += buttonNames[sequence.m_buttons[i].m_type];
            }

            printf("[QTE] Sequence %u: %.2fs [%s]\n", s, sequence.m_time, str.c_str());
        }
#endif

        // initialize ui
        Sonic::CCsdDatabaseWrapper wrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
        auto spCsdProject = wrapper.GetCsdProject("ui_qte_swa");
        m_rcQTE = spCsdProject->m_rcProject;

        m_txt1 = m_rcQTE->CreateScene("qte_txt_1");
        m_txt1->SetHideFlag(true);
        m_txt2 = m_rcQTE->CreateScene("qte_txt_2");
        m_txt2->SetHideFlag(true);
        m_txt3 = m_rcQTE->CreateScene("qte_txt_3");
        m_txt3->SetHideFlag(true);
        m_txt4 = m_rcQTE->CreateScene("qte_txt_4");
        m_txt4->SetHideFlag(true);

        float timeTotal = 0.0f;
        for (Sequence& sequence : m_sequences)
        {
            timeTotal += sequence.m_time;

            sequence.m_bg = m_rcQTE->CreateScene("m_bg");
            sequence.m_bg->SetHideFlag(true);
            sequence.m_timer = m_rcQTE->CreateScene("m_timer");
            sequence.m_timer->SetHideFlag(true);

            // spam mode
            sequence.m_boss = m_rcQTE->CreateScene("m_boss");
            SetSpamAmountText(sequence.m_boss, sequence.m_spamCount);
            sequence.m_boss->SetHideFlag(true);

            // utterly useless code to handle more than 10 buttons with multiple rows
            size_t rowCount = ((sequence.m_buttons.size() - 1) / 10) + 1;
            std::vector<size_t> columnCounts(rowCount, sequence.m_buttons.size() / rowCount);
            for (size_t i = 0; i < sequence.m_buttons.size() % rowCount; i++)
            {
                // distribute the remaining buttons to bottom rows
                columnCounts[columnCounts.size() - 1 - i]++;
            }

            size_t row = 0;
            size_t column = 0;
            float yPos = c_qteUiButtonYPos - (rowCount - 1) * c_qteUiButtonYSpacing;
            float xPos = 0.0f;

            for (size_t b = 0; b < sequence.m_buttons.size(); b++)
            {
                if (column == 0)
                {
                    if (sequence.m_spamCount > 0)
                    {
                        xPos = c_qteUiButtonSpamXPos;
                    }
                    else
                    {
                        xPos = 640.0f - (columnCounts[row] - 1) * c_qteUiButtonXSpacing * 0.5f;
                    }
                }

                Button& button = sequence.m_buttons[b];
                switch (button.m_type)
                {
                case ButtonType::A:
                    button.m_scene = m_rcQTE->CreateScene("btn_1");
                    break;
                case ButtonType::B:
                    button.m_scene = m_rcQTE->CreateScene("btn_1");
                    button.m_scene->GetNode("img")->SetPatternIndex(1);
                    break;
                case ButtonType::X:
                    button.m_scene = m_rcQTE->CreateScene("btn_1");
                    button.m_scene->GetNode("img")->SetPatternIndex(2);
                    break;
                case ButtonType::Y:
                    button.m_scene = m_rcQTE->CreateScene("btn_1");
                    button.m_scene->GetNode("img")->SetPatternIndex(3);
                    break;
                case ButtonType::LB:
                    button.m_scene = m_rcQTE->CreateScene("btn_2");
                    break;
                case ButtonType::RB:
                    button.m_scene = m_rcQTE->CreateScene("btn_2");
                    button.m_scene->GetNode("img")->SetPatternIndex(1);
                    button.m_scene->GetNode("bg")->SetScale(-1.0f, 1.0f);
                    button.m_scene->GetNode("bg")->SetPosition(2.35f, 0.0f);
                    break;
                }
                button.m_scene->SetHideFlag(true);
                button.m_scene->GetNode("position")->SetPosition(xPos, yPos);
                button.m_effect = m_rcQTE->CreateScene("qte_multi_effect");
                button.m_effect->SetHideFlag(true);
                button.m_effect->GetNode("position")->SetPosition(xPos, yPos);

                xPos += c_qteUiButtonXSpacing;
                column++;
                if (column >= columnCounts[row] && row < rowCount - 1)
                {
                    // next row
                    row++;
                    column = 0;
                    yPos += c_qteUiButtonYSpacing;
                }
            }
        }

        // adjust the appear time base on how long the QTE is, samples were taken at 2s
        m_uiAppearTime -= (timeTotal - 2.0f) * c_qteUiSlowTimeScale;

        m_spQTE = boost::make_shared<Sonic::CGameObjectCSD>(m_rcQTE, 0.5f, "HUD_B2", false);
        Sonic::CGameDocument::GetInstance()->AddGameObject(m_spQTE, "main", this);

        // make sure Sonic doesn't receive input
        auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
        context->StateFlag(eStateFlag_OutOfControl)++;

        // change animation
        Common::SonicContextChangeAnimation("JumpSpring");
	}

    void CreateSequence(uint32_t buttonCount, float trickTime)
    {
        Sequence sequence;
        sequence.m_time = trickTime;
        for (uint32_t i = 0; i < buttonCount; i++)
        {
            Button button;
            button.m_type = (ButtonType)(rand() % ButtonType::COUNT);
            sequence.m_buttons.push_back(button);
        }
        m_sequences.push_back(sequence);
    }

    void CreateSpamSequence(ButtonType type, uint32_t spamCount, float trickTime)
    {
        Sequence sequence;
        sequence.m_spamCount = spamCount;
        sequence.m_time = trickTime;

        Button button;
        button.m_type = type;
        sequence.m_buttons.push_back(button);

        m_sequences.push_back(sequence);
    }

	bool ProcessMessage
	(
		Hedgehog::Universe::Message& message,
		bool flag
	) override
	{
		if (flag)
		{
			if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
	         || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
			{
				Kill();
				return true;
			}
		}

		return Sonic::CGameObject::ProcessMessage(message, flag);
	}

	void UpdateParallel
	(
		const Hedgehog::Universe::SUpdateInfo& updateInfo
	) override
	{
        m_lifeTime += updateInfo.DeltaTime;
        static SharedPtrTypeless soundHandle;
        static SharedPtrTypeless voiceHandle;

        switch (m_state)
        {
        case S_SlowTime:
        {
            if (SlowTime() && m_lifeTime >= m_uiAppearTime)
            {
                PlayIntroAnim();
                m_state = S_Intro;

                auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
                float const gravity = -context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_Gravity);
                hh::math::CVector vel = context->m_Velocity;
                hh::math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;

                // simulate when Sonic will end up when QTE time out
                float simDuration = 0.0f;
                for (Sequence const& sequence : m_sequences)
                {
                    simDuration += sequence.m_time;
                }
                simDuration *= c_qteUiSlowTimeScale;

                float simTime = 0.0f;
                float constexpr frameRate = 1.0f / 60.0f;
                while (simTime < simDuration)
                {
                    hh::math::CVector const velPrev = vel;
                    vel += Hedgehog::Math::CVector::UnitY() * gravity * frameRate;
                    hh::math::CVector const posPrev = pos;
                    pos += (velPrev + vel) * 0.5f * frameRate;

                    simTime += frameRate;
                }

                m_impulsePos = pos;
            }
            break;
        }
        case S_Intro:
        {
            Sequence const& sequence = m_sequences[m_sequenceID];
            if (sequence.m_bg->m_MotionDisableFlag)
            {
                PlayMotion(sequence.m_timer, "Timer_Anim", 100.0f / (60.0f * sequence.m_time));
                m_state = S_Input;
            }
            break;
        }
        case S_Input:
        {
            Sequence& sequence = m_sequences[m_sequenceID];

            // check for any tapped buttons
            std::vector<Sonic::EKeyState> const buttons =
            {
                Sonic::eKeyState_A,
                Sonic::eKeyState_B,
                Sonic::eKeyState_X,
                Sonic::eKeyState_Y,
                Sonic::eKeyState_LeftBumper,
                Sonic::eKeyState_RightBumper,
            };
            Sonic::EKeyState tapped = Sonic::eKeyState_None;
            for (Sonic::EKeyState const& button : buttons)
            {
                if (Common::fIsButtonTapped(button))
                {
                    tapped = button;
                    break;
                }
            }

            bool failed = false;
            if (tapped != Sonic::eKeyState_None)
            {
                Button const& button = sequence.m_buttons[m_buttonID];
                switch (button.m_type)
                {
                case ButtonType::A: failed = (tapped != Sonic::eKeyState_A); break;
                case ButtonType::B: failed = (tapped != Sonic::eKeyState_B); break;
                case ButtonType::X: failed = (tapped != Sonic::eKeyState_X); break;
                case ButtonType::Y: failed = (tapped != Sonic::eKeyState_Y); break;
                case ButtonType::LB: failed = (tapped != Sonic::eKeyState_LeftBumper); break;
                case ButtonType::RB: failed = (tapped != Sonic::eKeyState_RightBumper); break;
                }

                if (!failed)
                {
                    if (sequence.m_spamCount > 1)
                    {
                        sequence.m_spamCount--;
                        SetSpamAmountText(sequence.m_boss, sequence.m_spamCount);
                        break;
                    }

                    // correct input
                    PlayMotion(button.m_scene, "Effect_Anim");
                    PlayMotion(button.m_effect, "Effect_Anim");
                    m_buttonID++;

                    // next sequence
                    if (m_buttonID >= sequence.m_buttons.size())
                    {
                        Common::SonicContextPlaySound(soundHandle, 3000812995, 0);

                        sequence.m_bg->SetHideFlag(true);
                        sequence.m_timer->SetHideFlag(true);
                        sequence.m_boss->SetHideFlag(true);

                        float additionalScore = 1000.0f * sequence.m_time * (1.0f - sequence.m_timer->m_MotionFrame * 0.01f) * (float)m_data.m_Difficulty;
                        int score = max(0, m_data.m_Score + (int)additionalScore);
                        ScoreGenerationsAPI::AddScore(score);
                        UnleashedHUD_API::AddTrickScore(score);

                        m_sequenceID++;
                        if (m_sequenceID >= m_sequences.size())
                        {
                            // we are done!
                            if (sequence.m_timer->m_MotionFrame <= 50)
                            {
                                m_txtID = 2;
                                PlayMotion(m_txt3, "Intro_Anim");
                            }
                            else if (sequence.m_timer->m_MotionFrame <= 75)
                            {
                                m_txtID = 1;
                                PlayMotion(m_txt2, "Intro_Anim");
                            }
                            else
                            {
                                m_txtID = 0;
                                PlayMotion(m_txt1, "Intro_Anim");
                            }

                            // this makes a 0.5s not to accept MsgApplyImpulse if launched in air...? sub_E2BA00
                            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
                            context->StateFlag(eStateFlag_NoLandOutOfControl) = 0;

                            // pitch it up, scale it
                            hh::math::CVector pitchAxis = m_direction.cross(hh::math::CVector::UnitY());
                            hh::math::CVector impulse = Eigen::AngleAxisf(m_data.m_Pitch[1] * DEG_TO_RAD, pitchAxis) * m_direction * m_data.m_Speed[1];

                            //printf("[QTE] Dir = {%.2f, %.2f, %.2f}, Pos = {%.2f, %.2f, %.2f}\n", DEBUG_VECTOR3(m_direction), DEBUG_VECTOR3(m_uiAppearPos));

                            // Apply success impulse
                            alignas(16) MsgApplyImpulse message {};
                            message.m_position = m_impulsePos;
                            message.m_impulse = impulse;
                            message.m_impulseType = ImpulseType::JumpBoard;
                            message.m_outOfControl = m_data.m_OutOfControl[1];
                            message.m_notRelative = true;
                            message.m_snapPosition = true;
                            message.m_pathInterpolate = false;
                            message.m_alwaysMinusOne = -1.0f;
                            Common::ApplyPlayerApplyImpulse(message);
                            
                            const char* volatile const* trickAnim = AnimationSetPatcher::TrickSG;
                            bool const isUnleashedSonic = context->m_pPlayer->m_spCharacterModel->GetNode("SonicRoot") != nullptr;
                            if (isUnleashedSonic)
                            {
                                // Unleashed Sonic
                                trickAnim = AnimationSetPatcher::TrickSWA;
                            }
                            else if (context->m_pPlayer->m_spCharacterModel->GetNode("EvilRoot") != nullptr)
                            {
                                // TODO: Werehog
                            }

                            int const randomIndex = rand() % 7;
                            Common::SonicContextChangeAnimation(trickAnim[randomIndex]);
                            Common::SonicContextPlayVoice(voiceHandle, 3002013, 20);

                            // play pfx, trick_B plays nothing
                            if (randomIndex != 1)
                            {
                                static SharedPtrTypeless pfxHandle;
                                std::string effectName = "ef_cmn_trickjump_C";
                                std::string boneName = isUnleashedSonic ? "Tail1" : "Tail";
                                switch (randomIndex)
                                {
                                case 0:
                                    effectName = "ef_cmn_trickjump_A";
                                    break;
                                case 2:
                                    effectName = "ef_cmn_trickjump_C";
                                    break;
                                case 3:
                                    effectName = "ef_cmn_trickjump_D";
                                    break;
                                case 4:
                                    effectName = "ef_cmn_trickjump_E";
                                    boneName = "Index3_R";
                                    break;
                                case 5:
                                    effectName = "ef_cmn_trickjump_F";
                                    boneName = "Index3_L";
                                    break;
                                case 6:
                                    effectName = "ef_cmn_trickjump_G";
                                    break;
                                }

                                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode(boneName.c_str());
                                if (attachBone != nullptr)
                                {
                                    // play on specific bone
                                    Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, &attachBone, effectName.c_str(), 1);
                                }
                                else
                                {
                                    // default Sonic's middle pos
                                    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
                                    Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, effectName.c_str(), 1);
                                }
                            }

                            m_state = S_Outro;
                            break;
                        }

                        m_buttonID = 0;
                        PlayIntroAnim();
                        m_state = S_Intro;
                    }
                    else
                    {
                        Common::SonicContextPlaySound(soundHandle, 3000812987, 0);
                    }
                    break;
                }
            }

            // you fucked up
            if (sequence.m_timer->m_MotionDisableFlag || failed)
            {
                for (Button const& button : sequence.m_buttons)
                {
                    button.m_scene->SetHideFlag(true);
                }
                sequence.m_bg->SetHideFlag(true);
                sequence.m_timer->SetHideFlag(true);
                sequence.m_boss->SetHideFlag(true);

                m_txtID = 3;
                PlayMotion(m_txt4, "Intro_Anim");

                Common::SonicContextChangeAnimation("Fall");
                Common::SonicContextPlayVoice(soundHandle, 3002002, 10);
                Common::SonicContextPlaySound(soundHandle, 3000812996, 0);

                m_state = S_Outro;
                break;
            }

            break;
        }
        case S_Outro:
        {
            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
            context->StateFlag(eStateFlag_OutOfControl)--;
            ResetTime();

            m_state = S_Outro2;
            break;
        }
        case S_Outro2:
        {
            Chao::CSD::RCPtr<Chao::CSD::CScene> const& txt = GetTxtScene();
            if (txt->m_MotionDisableFlag)
            {
                PlayMotion(txt, m_txtID == 3 ? "Fnish_Anim" : "Finish_Anim");
                m_state = S_Finished;
            }
            break;
        }
        case S_Finished:
        {
            Chao::CSD::RCPtr<Chao::CSD::CScene> const& txt = GetTxtScene();
            if (txt->m_MotionDisableFlag)
            {
                Kill();
            }
            break;
        }
        }
	}

    bool* GetTimeScaleEnabled()
    {
        return (bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19C });
    }

    float* GetTimeScale()
    {
        return (float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A0 });
    }

    bool SlowTime()
    {
        *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19C }) = true;

        float const timeBeforeSlowDown = m_uiAppearTime - c_qteUiSlowTimeTime - c_qteUiSlowTimeFixed;
        float const prop = (m_lifeTime - timeBeforeSlowDown) / c_qteUiSlowTimeTime;
        
        if (prop <= 0.0f)
        {
            *GetTimeScale() = 1.0f;
            return false;
        }
        else if (prop >= 1.0f)
        {
            *GetTimeScale() = c_qteUiSlowTimeScale;
            return true;
        }
        else
        {
            *GetTimeScale() = 1.0f - (1.0f - c_qteUiSlowTimeScale) * prop;
            return false;
        }
    }

    void ResetTime()
    {
        if (*GetTimeScaleEnabled())
        {
            *GetTimeScaleEnabled() = false;
            *GetTimeScale() = 1.0f;
        }
    }

    void PlayIntroAnim()
    {
        Sequence const& sequence = m_sequences[m_sequenceID];
        if (sequence.m_buttons.size() <= 3)
        {
            PlayMotion(sequence.m_bg, "Size_Anim_3");
            PlayMotion(sequence.m_timer, "Size_Anim_3");
        }
        else if (sequence.m_buttons.size() == 4)
        {
            PlayMotion(sequence.m_bg, "Size_Anim_4");
            PlayMotion(sequence.m_timer, "Size_Anim_4");
        }
        else if (sequence.m_buttons.size() >= 5)
        {
            PlayMotion(sequence.m_bg, "Size_Anim_5");
            PlayMotion(sequence.m_timer, "Size_Anim_5");
        }

        PlayMotion(sequence.m_bg, "Intro_Anim");
        PlayMotion(sequence.m_timer, "Intro_Anim");
        for (Button const& button : sequence.m_buttons)
        {
            PlayMotion(button.m_scene, "Intro_Anim");
        }

        if (sequence.m_spamCount > 0)
        {
            if (m_sequenceID == 0)
            {
                static SharedPtrTypeless soundHandle;
                Common::SonicContextPlaySound(soundHandle, 3000812999, 0);
            }
            PlayMotion(sequence.m_boss, "Intro_Anim");
        }
    }

    void SetSpamAmountText(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, size_t spamCount)
    {
        if (!scene) return;
        std::string text = std::to_string(spamCount);
        if (spamCount < 10)
        {
            text = "0" + text;
        }
        scene->GetNode("num")->SetText(text.c_str());
        scene->Update();
    }

    void PlayMotion(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, char const* motion, float speed = 1.0f, bool loop = false)
    {
        if (!scene) return;
        scene->SetHideFlag(false);
        scene->SetMotion(motion);
        scene->m_MotionDisableFlag = false;
        scene->m_MotionFrame = 0.0f;
        scene->m_MotionSpeed = speed;
        scene->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
        scene->Update();
    }

    Chao::CSD::RCPtr<Chao::CSD::CScene> const& GetTxtScene()
    {
        switch (m_txtID)
        {
        case 0: return m_txt1;
        case 1: return m_txt2;
        case 2: return m_txt3;
        default: return m_txt4;
        }
    }

    void Kill()
    {
        //printf("[QTE] Killed\n");
        m_spTrickJumperUI = nullptr;
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

void TrickJumper::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
	in_rEditParam.CreateParamFloat(&m_Data.m_OutOfControl[0], "FirstOutOfControl");
	in_rEditParam.CreateParamFloat(&m_Data.m_Pitch[0], "FirstPitch");
	in_rEditParam.CreateParamFloat(&m_Data.m_Speed[0], "FirstSpeed");

	in_rEditParam.CreateParamFloat(&m_Data.m_OutOfControl[1], "SecondOutOfControl");
	in_rEditParam.CreateParamFloat(&m_Data.m_Pitch[1], "SecondPitch");
	in_rEditParam.CreateParamFloat(&m_Data.m_Speed[1], "SecondSpeed");

	in_rEditParam.CreateParamInt(&m_Data.m_TrickCount[0], "TrickCount1");
	in_rEditParam.CreateParamInt(&m_Data.m_TrickCount[1], "TrickCount2");
	in_rEditParam.CreateParamInt(&m_Data.m_TrickCount[2], "TrickCount3");
	in_rEditParam.CreateParamFloat(&m_Data.m_TrickTime[0], "TrickTime1");
	in_rEditParam.CreateParamFloat(&m_Data.m_TrickTime[1], "TrickTime2");
	in_rEditParam.CreateParamFloat(&m_Data.m_TrickTime[2], "TrickTime3");

	in_rEditParam.CreateParamInt(&m_Data.m_Difficulty, "m_Difficulty");
	in_rEditParam.CreateParamInt(&m_Data.m_Score, "m_Score");
	in_rEditParam.CreateParamBool(&m_Data.m_IsSideView, "IsSideView");
}

bool TrickJumper::SetAddRenderables
(
	Sonic::CGameDocument* in_pGameDocument, 
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
	const char* assetName = m_Data.m_IsSideView ? "cmn_obj_ms_trickpanelL2_000" : "cmn_obj_ms_trickpanelL4_000";
	hh::mr::CMirageDatabaseWrapper wrapper(in_spDatabase.get());
	boost::shared_ptr<hh::mr::CModelData> spModelData = wrapper.GetModelData(assetName, 0);
	m_spSpawnedModel = boost::make_shared<hh::mr::CSingleElement>(spModelData);
	m_spSpawnedModel->BindMatrixNode(m_spMatrixNodeTransform);
	Sonic::CGameObject::AddRenderable("Object", m_spSpawnedModel, true);

    m_spEffectMotionAll = boost::make_shared<hh::mot::CSingleElementEffectMotionAll>();
    m_spSpawnedModel->BindEffect(m_spEffectMotionAll);
    boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> texCoordAnimData;

    FUNCTION_PTR(void, __thiscall, fpGetTexCoordAnimData, 0x7597E0, 
        hh::mot::CMotionDatabaseWrapper const& wrapper,
        boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData>& texCoordAnimData, 
        hh::base::CSharedString const& name, 
        uint32_t flag
    );

    FUNCTION_PTR(void, __thiscall, fpCreateUVAnim, 0x7537E0,
        Hedgehog::Motion::CSingleElementEffectMotionAll* This,
        boost::shared_ptr<hh::mr::CModelData> const& modelData,
        boost::shared_ptr<Hedgehog::Motion::CTexcoordAnimationData> const& texCoordAnimData
    );

    hh::mot::CMotionDatabaseWrapper motWrapper(in_spDatabase.get());
    fpGetTexCoordAnimData(motWrapper, texCoordAnimData, "panelbelt-0000", 0);
    fpCreateUVAnim(m_spEffectMotionAll.get(), spModelData, texCoordAnimData);
    fpGetTexCoordAnimData(motWrapper, texCoordAnimData, "jumpboard_arrow-0000", 0);
    fpCreateUVAnim(m_spEffectMotionAll.get(), spModelData, texCoordAnimData);

	return true;
}

bool TrickJumper::SetAddColliders
(
	const boost::shared_ptr<Hedgehog::Database::CDatabase>& in_spDatabase
)
{
    // Rigid body
    m_spNodeRigidBody = boost::make_shared<Sonic::CMatrixNodeTransform>();
    m_spNodeRigidBody->m_Transform.SetPosition(hh::math::CVector::Zero());
    m_spNodeRigidBody->NotifyChanged();
    m_spNodeRigidBody->SetParent(m_spMatrixNodeTransform.get());

    char const* rigidBodyName = m_Data.m_IsSideView ? "cmn_obj_trickpanel30M_HD" : "cmn_obj_trickpanel30L_HD";
    AddRigidBody(m_spRigidBody, rigidBodyName, rigidBodyName, *(int*)0x01E0AFF4, m_spNodeRigidBody, in_spDatabase);

    // Event collision
    float constexpr angle = 0.5f;
	m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
    m_spNodeEventCollision->m_Transform.SetRotationAndPosition
    (
        hh::math::CQuaternion(cos(angle * 0.5f), sin(angle * 0.5f), 0.0f, 0.0f),
        hh::math::CVector(0.0f, m_Data.m_IsSideView ? 0.8f : 1.6f, 0.0f)
    );
	m_spNodeEventCollision->NotifyChanged();
	m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

	hk2010_2_0::hkpBoxShape* shapeEventTrigger1 = new hk2010_2_0::hkpBoxShape(m_Data.m_IsSideView ? hh::math::CVector(0.85f, 0.1f, 1.2f) : hh::math::CVector(1.9f, 0.2f, 3.0f));
	AddEventCollision("Object", shapeEventTrigger1, *reinterpret_cast<int*>(0x01E0AFD8), true, m_spNodeEventCollision);

	return true;
}

void TrickJumper::SetUpdateParallel
(
	const Hedgehog::Universe::SUpdateInfo& in_rUpdateInfo
)
{
    FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll* This, float dt);
    fpUpdateMotionAll(m_spEffectMotionAll.get(), in_rUpdateInfo.DeltaTime);
}

bool TrickJumper::ProcessMessage
(
	Hedgehog::Universe::Message& message, 
	bool flag
)

{
	if (flag)
	{
		if (std::strstr(message.GetType(), "MsgHitEventCollision") != nullptr)
		{
			// get forward direction
			hh::math::CVector dir = m_spMatrixNodeTransform->m_Transform.m_Rotation * -hh::math::CVector::UnitZ();
			dir.y() = 0.0f;
			dir.normalize();

			// pitch it up, scale it
            hh::math::CVector pitchAxis = dir.cross(hh::math::CVector::UnitY());
            hh::math::CVector impulse = Eigen::AngleAxisf(m_Data.m_Pitch[0] * DEG_TO_RAD, pitchAxis) * dir * m_Data.m_Speed[0];

			// apply impulse to Soniic
			alignas(16) MsgApplyImpulse message {};
			message.m_position = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, m_Data.m_IsSideView ? 1.0f : 2.0f, 0.0f);
			message.m_impulse = impulse;
			message.m_impulseType = ImpulseType::JumpBoard;
			message.m_outOfControl = m_Data.m_OutOfControl[0];
			message.m_notRelative = true;
			message.m_snapPosition = true;
			message.m_pathInterpolate = false;
			message.m_alwaysMinusOne = -1.0f;
			Common::ApplyPlayerApplyImpulse(message);

			// play sound
			Sonic::Player::CPlayerSpeedContext::GetInstance()->PlaySound(4002023, false);

			// create QTE UI when it doesn't exist
			if (!m_spTrickJumperUI)
			{
				m_spTrickJumperUI = boost::make_shared<CTrickJumperUI>(m_Data, dir);
				Sonic::CGameDocument::GetInstance()->AddGameObject(m_spTrickJumperUI);
			}

			return true;
		}
	}

	return Sonic::CObjectBase::ProcessMessage(message, flag);
}

void TrickJumper::registerObject()
{
	BB_INSTALL_SET_OBJECT_MAKE_HOOK(TrickJumper);
}

void TrickJumper::applyPatches()
{
    // Remove message delay for MsgFinishPause, this can cause HUD to not show up
    // anymore if we pause before HUD show up when time is slowed down
    WRITE_MEMORY(0x10A1500, uint8_t, 0xD9, 0xEE, 0x90, 0x90, 0x90, 0x90);
}
