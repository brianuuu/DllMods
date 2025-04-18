#include "QTEJumpBoard.h"
#include "AnimationSetPatcher.h"
#include "TrickJumper.h"

QTEJumpBoard::Data QTEJumpBoard::m_data;

float const c_qteButtonXSpacing = 115.0f;
float const c_qteButtonYSpacing = 75.0f;
float const c_qteButtonSpamXPos = 584.0f;
float const c_qteButtonYPos = 360.0f;
float const c_qteSlowTimeScale = 0.075f;
float const c_qteSlowTimeTime = 0.6f;
float const c_qteSlowTimeFixed = 0.17f;
float const c_qteAppearTime = 0.8f;
float const c_qteSimRate = 1.0f / 60.0f;
float const c_qteBaseScore = 3000.0f;
float const c_qteDifficultyTimes[] =
{
    5.0f,
    4.5f,
    4.0f,
    3.5f,
    3.0f,
    2.5f,
    2.0f,
    1.5f,
    1.3f,
    1.1f,
};

float const c_qteSpamDifficultyTimes[] =
{
    15.0f,
    14.0f,
    13.0f,
    12.0f,
    11.0f,
    10.0f,
    7.5f,
    5.0f,
    3.5f,
    2.0f,
};

class CQTEButtonSequence : public Sonic::CGameObject
{
    Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcQTE;
    boost::shared_ptr<Sonic::CGameObjectCSD> m_spQTE;

    QTEJumpBoard::Data m_data;
    float m_lifeTime;
    bool m_isOutOfControl;

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
    CQTEButtonSequence(QTEJumpBoard::Data const& data)
        : m_data(data)
        , m_lifeTime(0.0f)
        , m_isOutOfControl(false)
        , m_txtID(3u)
        , m_sequenceID(0u)
        , m_buttonID(0u)
        , m_state(State::S_SlowTime)
    {
    }

    ~CQTEButtonSequence()
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

        size_t mode = (m_data.m_sizeType % 10) / 2;
        uint32_t number = m_data.m_sizeType / 10;
        if (mode == 1)
        {
            // Button spam mode
            while (number > 100 && m_sequences.size() < 2)
            {
                // 1st digit is difficulty
                size_t difficulty = number % 10;
                number /= 10;

                // 2th digit is button type
                ButtonType type = (ButtonType)(number % 10 % ButtonType::COUNT);
                number /= 10;

                // last 2 digits is spam count
                size_t spamCount = number % 100;
                spamCount = max(1, spamCount);
                number /= 100;

                // create sequence
                CreateSpamSequence(difficulty, type, spamCount);
            }
        }
        else
        {
            // extract button sequence data from m_data.m_sizeType
            while (number > 10 && m_sequences.size() < 3)
            {
                // first digit is difficulty
                size_t difficulty = number % 10;

                // 2nd digit is how many buttons
                size_t buttonCount = (number / 10) % 10;
                buttonCount = max(1, buttonCount);

                // create sequence
                CreateSequence(difficulty, buttonCount);

                // read next two digits
                number /= 100;
            }
        }

        // we have invalid number, probably Gen's trick panel
        if (m_sequences.empty())
        {
            // create a completely random sequence
            size_t sequenceCount = 1;// (rand() % 3) + 1;
            while (m_sequences.size() < sequenceCount)
            {
                size_t difficulty = rand() % 5; // [0-4] difficulty, don't want to make it too hard
                size_t buttonCount = (rand() % 3) + 3; // [3-5] buttons
                CreateSequence(difficulty, buttonCount);
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
            float yPos = c_qteButtonYPos - (rowCount - 1) * c_qteButtonYSpacing;
            float xPos = 0.0f;

            for (size_t b = 0; b < sequence.m_buttons.size(); b++)
            {
                if (column == 0)
                {
                    if (sequence.m_spamCount > 0)
                    {
                        xPos = c_qteButtonSpamXPos;
                    }
                    else
                    {
                        xPos = 640.0f - (columnCounts[row] - 1) * c_qteButtonXSpacing * 0.5f;
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

                xPos += c_qteButtonXSpacing;
                column++;
                if (column >= columnCounts[row] && row < rowCount - 1)
                {
                    // next row
                    row++;
                    column = 0;
                    yPos += c_qteButtonYSpacing;
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

    void CreateSequence(size_t difficulty, uint32_t buttonCount)
    {
        Sequence sequence;
        sequence.m_time = c_qteDifficultyTimes[difficulty];
        for (uint32_t i = 0; i < buttonCount; i++)
        {
            Button button;
            button.m_type = (ButtonType)(rand() % ButtonType::COUNT);
            sequence.m_buttons.push_back(button);
        }
        m_sequences.push_front(sequence);
    }

    void CreateSpamSequence(size_t difficulty, ButtonType type, uint32_t spamCount)
    {
        Sequence sequence;
        sequence.m_spamCount = spamCount;
        sequence.m_time = c_qteSpamDifficultyTimes[difficulty];

        Button button;
        button.m_type = type;
        sequence.m_buttons.push_back(button);

        m_sequences.push_front(sequence);
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
            if (SlowTime() &&m_lifeTime >= c_qteAppearTime)
            {
                //printf("[QTE] Boost Land Location = {%.2f, %.2f, %.2f}\n", DEBUG_VECTOR3(m_data.m_position));
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

                        float additionalScore = 1000.0f * sequence.m_time * (1.0f - sequence.m_timer->m_MotionFrame * 0.01f);
                        int score = max(0, c_qteBaseScore + (int)additionalScore);
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
                            
                            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
                            const char* volatile const* trickAnim = AnimationSetPatcher::TrickSG;
                            bool const isUnleashedSonic = context->m_pPlayer->m_spCharacterModel->GetNode("SonicRoot") != nullptr;
                            if (isUnleashedSonic)
                            {
                                // Unleashed Sonic
                                trickAnim = AnimationSetPatcher::TrickSWA;
                            }
                            else if (context->m_pPlayer->m_spCharacterModel->GetNode("EvilRoot") != nullptr)
                            {
                                // Werehog
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
        
        float const timeBeforeSlowDown = c_qteAppearTime - c_qteSlowTimeTime - c_qteSlowTimeFixed;
        float const prop = (m_lifeTime - timeBeforeSlowDown) / c_qteSlowTimeTime;

        if (prop <= 0.0f)
        {
            *GetTimeScale() = 1.0f;
            return false;
        }
        else if (prop >= 1.0f)
        {
            *GetTimeScale() = c_qteSlowTimeScale;
            return true;
        }
        else
        {
            *GetTimeScale() = 1.0f - (1.0f - c_qteSlowTimeScale) * prop;
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
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

uint32_t tempSizeType;
void QTEJumpBoard_SaveSizeType(uint32_t const& This)
{
    uint32_t* sizeType = (uint32_t*)(This + 0x108);
    tempSizeType = *sizeType;
    *sizeType = *sizeType % 2;
}

void QTEJumpBoard_RestoreSizeType(uint32_t const& This)
{
    uint32_t* sizeType = (uint32_t*)(This + 0x108);
    *sizeType = tempSizeType;
}

HOOK(void, __fastcall, QTEJumpBoard_MsgHitEventCollision, 0x1014FB0, uint32_t This, void* Edx, hh::fnd::MessageTypeSet& message)
{
    QTEJumpBoard_SaveSizeType(This);
	originalQTEJumpBoard_MsgHitEventCollision(This, Edx, message);
    QTEJumpBoard_RestoreSizeType(This);

    if (message.m_SenderActorID == Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID)
    {
        QTEJumpBoard::GetQTEJumpBoardData(This);
    }
}

HOOK(void, __fastcall, QTEJumpBoard_MsgApplyImpulse, 0xE6CFA0, void* This, void* Edx, MsgApplyImpulse* message)
{
    originalQTEJumpBoard_MsgApplyImpulse(This, Edx, message);

    if (QTEJumpBoard::m_data.m_init)
    {
        // create QTE object
        auto object = boost::make_shared<CQTEButtonSequence>(QTEJumpBoard::m_data);
        Sonic::CGameDocument::GetInstance()->AddGameObject(object);

        QTEJumpBoard::m_data = QTEJumpBoard::Data();
    }
}

HOOK(bool, __fastcall, QTEJumpBoard_CObjAdlibTrickJumpInit1, 0x1014C00, uint32_t This, void* Edx, void* a2, void* a3)
{
    QTEJumpBoard_SaveSizeType(This);
    bool result = originalQTEJumpBoard_CObjAdlibTrickJumpInit1(This, Edx, a2, a3);
    QTEJumpBoard_RestoreSizeType(This);
    return result;
}

HOOK(bool, __fastcall, QTEJumpBoard_CObjAdlibTrickJumpInit2, 0x1015BF0, uint32_t This, void* Edx, void* a2)
{
    QTEJumpBoard_SaveSizeType(This);
    bool result = originalQTEJumpBoard_CObjAdlibTrickJumpInit2(This, Edx, a2);
    QTEJumpBoard_RestoreSizeType(This);
    return result;
}

void QTEJumpBoard::applyPatches()
{
	// Make trick ramp use JumpBaord animation
	WRITE_MEMORY(0x1014866, uint32_t, ImpulseType::JumpBoard);

    // Disable effects
    WRITE_STRING(0x166A16C, "");
    WRITE_STRING(0x166A150, "");
    WRITE_STRING(0x166A134, "");
    WRITE_STRING(0x166A118, "");

    // Main triggers
	INSTALL_HOOK(QTEJumpBoard_MsgHitEventCollision);
	INSTALL_HOOK(QTEJumpBoard_MsgApplyImpulse);

    // Fix instance using SizeType member
    INSTALL_HOOK(QTEJumpBoard_CObjAdlibTrickJumpInit1);
    INSTALL_HOOK(QTEJumpBoard_CObjAdlibTrickJumpInit2);

    // Replace with Unleashed model
    WRITE_MEMORY(0x1A47254, char*, "cmn_obj_ms_trickpanelL2_000");
    WRITE_MEMORY(0x1A47258, char*, "cmn_obj_ms_trickpanelL4_000");
    WRITE_MEMORY(0x1A47278, char*, "panelbelt-0000");
    WRITE_MEMORY(0x1A4727C, char*, "panelbelt-0000");
    WRITE_MEMORY(0x1A4726C, char*, "jumpboard_arrow-0000");
    WRITE_MEMORY(0x1A47270, char*, "jumpboard_arrow-0000");
    WRITE_MEMORY(0x1014EFD, uint32_t, 0xFF7448DF); // replace pt-anim with uv-anim
    WRITE_MEMORY(0x1014F1B, uint32_t, 0xFF73E8C1); // replace pt-anim with uv-anim
}

void QTEJumpBoard::GetQTEJumpBoardData(uint32_t ptr)
{
	m_data = Data();

    m_data.m_sizeType = *(uint32_t*)(ptr + 0x108);

	m_data.m_init = true;
}
