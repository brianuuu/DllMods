#include "TrickJumper.h"
#include "AnimationSetPatcher.h"

BB_SET_OBJECT_MAKE_HOOK(TrickJumper)

float TrickJumper::m_xAspectOffset = 0.0f;
float TrickJumper::m_yAspectOffset = 0.0f;

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
    bool m_isOutOfControl;

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
    CTrickJumperUI
    (
        TrickJumper::Data const& data,
        hh::math::CVector const& direction,
        hh::math::CVector const& impulsePos,
        float const& uiAppearTime
    )
        : m_data(data)
        , m_lifeTime(0.0f)
        , m_uiAppearTime(uiAppearTime)
        , m_isOutOfControl(false)
        , m_direction(direction)
        , m_impulsePos(impulsePos)
        , m_txtID(3u)
        , m_sequenceID(0u)
        , m_buttonID(0u)
        , m_state(State::S_SlowTime)
	{
	}

	~CTrickJumperUI()
	{
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

        if (m_isOutOfControl)
        {
            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
            if (context)
            {
                context->StateFlag(eStateFlag_OutOfControl)--;
                m_isOutOfControl = false;
            }
        }
	}

	void AddCallback
	(
		const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
		Sonic::CGameDocument* pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
	) override
	{
        TrickJumper::CalculateAspectOffsets();

        // Update unit 1 is unaffected by time slowing down
        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("1", this);

        for (int i = 0; i < 3; i++)
        {
            if (m_data.m_TrickCount[i] <= 0 || m_data.m_TrickTime[i] == 0.0f)
            {
                break;
            }

            // create sequence
            if (m_data.m_TrickTime[i] < 0.0f)
            {
                CreateSpamSequence((ButtonType)(m_data.m_TrickCount[i] / 100), m_data.m_TrickCount[i] % 100, abs(m_data.m_TrickTime[i]));
            }
            else
            {
                CreateSequence(m_data.m_TrickCount[i], m_data.m_TrickTime[i]);
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

        for (Sequence& sequence : m_sequences)
        {
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
                    button.m_scene->GetNode("bg")->SetPosition(TrickJumper::m_xAspectOffset * 0.5f + 2.35f, TrickJumper::m_yAspectOffset * 0.5f);
                    break;
                }
                button.m_scene->SetHideFlag(true);
                button.m_scene->GetNode("position")->SetPosition(TrickJumper::m_xAspectOffset * 0.5f + xPos, TrickJumper::m_yAspectOffset * 0.5f + yPos);
                button.m_effect = m_rcQTE->CreateScene("qte_multi_effect");
                button.m_effect->SetHideFlag(true);
                button.m_effect->GetNode("position")->SetPosition(TrickJumper::m_xAspectOffset * 0.5f + xPos, TrickJumper::m_yAspectOffset * 0.5f + yPos);

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

        m_spQTE = boost::make_shared<Sonic::CGameObjectCSD>(m_rcQTE, 0.5f, "HUD_B2", false);
        Sonic::CGameDocument::GetInstance()->AddGameObject(m_spQTE, "main", this);

        // make sure Sonic doesn't receive input
        auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
        context->StateFlag(eStateFlag_OutOfControl)++;
        m_isOutOfControl = true;

        // change animation
        Common::SonicContextChangeAnimation(AnimationSetPatcher::TrickJumpStart);
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

                        float additionalScore = 1000.0f * sequence.m_time * (1.0f - sequence.m_timer->m_MotionFrame * 0.01f) * (float)max(1, m_data.m_Difficulty);
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
                            hh::math::CVector pitchAxis = m_direction.cross(hh::math::CVector::UnitY()).normalized();
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
            m_isOutOfControl = false;
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

    bool SlowTime()
    {
        float const timeBeforeSlowDown = m_uiAppearTime - c_qteUiSlowTimeTime - c_qteUiSlowTimeFixed;
        float const prop = (m_lifeTime - timeBeforeSlowDown) / c_qteUiSlowTimeTime;
        
        if (prop <= 0.0f)
        {
            return false;
        }
        else if (prop >= 1.0f)
        {
            SendSlowTimeMessage(c_qteUiSlowTimeScale, true);
            return true;
        }
        else
        {
            SendSlowTimeMessage(1.0f - (1.0f - c_qteUiSlowTimeScale) * prop, true);
            return false;
        }
    }

    void ResetTime()
    {
        SendSlowTimeMessage(1.0f, false);
    }

    void SendSlowTimeMessage(float scale, bool isSlowed)
    {
        size_t cGameplayFlowActor = *(uint32_t*)(*(uint32_t*)(*(uint32_t*)((*(uint32_t*)0x1E66B34) + 4) + 52) + 96 + 0x2C);
        SendMessage(cGameplayFlowActor, boost::make_shared<Sonic::Message::MsgChangeGameSpeed>(scale, isSlowed ? 0 : 1));
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
        ResetTime();
    }
};

void TrickJumper::InitializeEditParam
(
	Sonic::CEditParam& in_rEditParam
)
{
    m_statChecked = false;
    m_uvAnimUpdate = 0.0f;

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

    // some params were missing in some level in Unleashed
    m_Data.m_Difficulty = 1;
    m_Data.m_Score = 3000.0f;

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
    m_spNodeRigidBody->m_Transform.SetPosition(hh::math::CVector(0.0f, m_Data.m_IsSideView ? -0.05f : -0.2f, 0.0f));
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
    FUNCTION_PTR(void, __thiscall, fpUpdateMotionAll, 0x752F00, Hedgehog::Motion::CSingleElementEffectMotionAll * This, float dt);
    FUNCTION_PTR(void, __thiscall, fpUpdateCTexcoordMotion, 0x7570E0, Hedgehog::Motion::CTexcoordMotion * This, float dt);

    // belt
    fpUpdateCTexcoordMotion(&m_spEffectMotionAll->m_TexcoordMotionList[0], in_rUpdateInfo.DeltaTime);

    // arrows
    float constexpr frameRate = 1.0f / 60.0f;
    m_uvAnimUpdate += in_rUpdateInfo.DeltaTime;
    while (m_uvAnimUpdate >= frameRate)
    {
        m_uvAnimUpdate -= frameRate;
        fpUpdateCTexcoordMotion(&m_spEffectMotionAll->m_TexcoordMotionList[1], frameRate);
    }
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
            hh::math::CVector pitchAxis = dir.cross(hh::math::CVector::UnitY()).normalized();
            hh::math::CVector impulse = Eigen::AngleAxisf(m_Data.m_Pitch[0] * DEG_TO_RAD, pitchAxis) * dir * m_Data.m_Speed[0];
            hh::math::CVector position = m_spMatrixNodeTransform->m_Transform.m_Position + hh::math::CVector(0.0f, m_Data.m_IsSideView ? 0.5f : 1.0f, 0.0f);

			// apply impulse to Soniic
			alignas(16) MsgApplyImpulse message {};
			message.m_position = position;
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
            
            // Calculate where the peak position is
            if (!m_statChecked)
            {
                m_statChecked = true;

                // Validate parameters
                bool valid = true;
                for (int i = 0; i < 3; i++)
                {
                    if (i == 0)
                    {
                        // first slot must be valid
                        valid &= m_Data.m_TrickCount[i] > 0 && m_Data.m_TrickTime[i] != 0.0f;
                    }
                    else if (m_Data.m_TrickCount[i] != 0 && m_Data.m_TrickTime[i] != 0.0f)
                    {
                        // 2nd & 3rd slot
                        valid &= m_Data.m_TrickCount[i] > 0 && m_Data.m_TrickTime[i] != 0.0f;
                    }

                    // button spamming
                    if (m_Data.m_TrickTime[i] < 0.0f)
                    {
                        valid &= m_Data.m_TrickCount[i] <= 599 && (m_Data.m_TrickCount[i] % 100 != 0);
                    }
                }

                if (!valid)
                {
                    // make random sequence
                    m_Data.m_TrickCount[0] = (rand() % 3) + 3; // [3-5] buttons
                    m_Data.m_TrickCount[1] = 0;
                    m_Data.m_TrickCount[2] = 0;

                    m_Data.m_TrickTime[0] = 3.0f;
                    m_Data.m_TrickTime[1] = 0.0f;
                    m_Data.m_TrickTime[2] = 0.0f;
                }

                auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
                float const gravity = -context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_Gravity);
                hh::math::CVector vel = impulse;
                hh::math::CVector pos = position;

                float simTime = 0.0f;
                float constexpr frameRate = 1.0f / 30.0f;
                while (true)
                {
                    hh::math::CVector const velPrev = vel;
                    vel += Hedgehog::Math::CVector::UnitY() * gravity * frameRate;
                    hh::math::CVector const posPrev = pos;
                    pos += vel * frameRate;

                    if (pos.y() < posPrev.y())
                    {
                        // always launch at peak of arc
                        m_arcPeakPosition = posPrev;
                        break;
                    }

                    simTime += frameRate;
                }

                float trickTime = 0.0f;
                for (int i = 0; i < 3; i++)
                {
                    if (m_Data.m_TrickCount[i] <= 0 || m_Data.m_TrickTime[i] == 0.0f)
                    {
                        break;
                    }

                    trickTime += abs(m_Data.m_TrickTime[i]);
                }

                m_uiAppearTime = max(0.701f, simTime - trickTime * c_qteUiSlowTimeScale + 0.467f);
            }

			// kill any QTE that still exist
            if (m_spTrickJumperUI)
            {
                ((CTrickJumperUI*)m_spTrickJumperUI.get())->Kill();
            }

			m_spTrickJumperUI = boost::make_shared<CTrickJumperUI>(m_Data, dir, m_arcPeakPosition, m_uiAppearTime);
			Sonic::CGameDocument::GetInstance()->AddGameObject(m_spTrickJumperUI);

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

void TrickJumper::CalculateAspectOffsets()
{
    if (*(size_t*)0x6F23C6 != 0x75D8C0D9) // Widescreen Support
    {
        const float aspect = (float)*(size_t*)0x1DFDDDC / (float)*(size_t*)0x1DFDDE0;

        if (aspect * 9.0f > 16.0f)
        {
            m_xAspectOffset = 720.0f * aspect - 1280.0f;
            m_yAspectOffset = 0.0f;
        }
        else
        {
            m_xAspectOffset = 0.0f;
            m_yAspectOffset = 1280.0f / aspect - 720.0f;
        }
    }
    else
    {
        m_xAspectOffset = 0.0f;
        m_yAspectOffset = 0.0f;
    }
}
