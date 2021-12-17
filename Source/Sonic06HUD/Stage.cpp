#include "Stage.h"
#include "Application.h"
#include "Configuration.h"
#include "UIContext.h"

//---------------------------------------------------
// Fade transitions
//---------------------------------------------------
void __fastcall SetLoadingFadeIn(float startAlpha, float* a2)
{
    uint8_t* color = (uint8_t*)((uint32_t)a2 + 21);
    if (color[0] == 0 && color[1] == 0 && color[2] == 0)
    {
        a2[6] = startAlpha; // start alpha

        if (a2[6] > 0.0f)
        {
            a2[4] = 0.2f; // transition time

            // Unlock player control
            (*(uint32_t**)0x1E66B40)[2] = 0;
        }
    }
}

void __fastcall SetLoadingFadeOut(float* a2)
{
    uint8_t* color = (uint8_t*)((uint32_t)a2 + 21);
    if (color[0] == 0 && color[1] == 0 && color[2] == 0)
    {
        a2[4] = 0.0f; // transition time
        a2[6] = 1.0f; // start alpha
    }
}

HOOK(float*, __fastcall, MsgFadeInFxp, 0x10CEC90, void* This, void* Edx, float* a2)
{
    SetLoadingFadeIn(((float*)This)[24], a2);
    return originalMsgFadeInFxp(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeOutFxp, 0x10CEDB0, void* This, void* Edx, float* a2)
{
    SetLoadingFadeOut(a2);
    return originalMsgFadeOutFxp(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeInMtfx, 0x57B290, void* This, void* Edx, float* a2)
{
    uint32_t actualThis = (uint32_t)This + 48 * *(uint32_t*)((uint32_t)a2 + 28) + 2064;
    SetLoadingFadeIn(((float*)actualThis)[1], a2);
    return originalMsgFadeInMtfx(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeOutMtfx, 0x57B270, void* This, void* Edx, float* a2)
{
    SetLoadingFadeOut(a2);
    return originalMsgFadeOutMtfx(This, Edx, a2);
}

//---------------------------------------------------
// 3 digit milliseconds correction
//---------------------------------------------------
void __declspec(naked) Stage_GetCurrentMillisecond()
{
    static uint32_t returnAddress = 0x10B3847;
	__asm
	{
		imul    edx, 3E8h
		sub     ecx, edx
		jmp     [returnAddress]
	}
}

void __fastcall SetMissionTime(char* buffer, int totalSeconds)
{
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    sprintf(buffer, "%02d:%02d.", minutes, seconds);
}

void __declspec(naked) Stage_GetMissionCurrentTime()
{
    static uint32_t returnAddress = 0x124F098;
	__asm
	{
        mov     edx, esi
        call    SetMissionTime
        pop     esi
        sub     esp, 0Ch
        jmp     [returnAddress]
	}
}

void __declspec(naked) Stage_GetMissionNextRankMaxTime()
{
    static uint32_t returnAddress = 0x10B16D5;
	__asm
	{
        mov	    esi, 99
        mov     edx, 59
        mov     ecx, 999
        jmp     [returnAddress]
	}
}

uint32_t __fastcall CreateRandomThirdDigit(uint32_t milliseconds)
{
    return milliseconds * 10 + rand() % 10;
}

void __declspec(naked) Stage_GetMissionNextRankRandomDigit()
{
    static uint32_t returnAddress = 0x10B16F9;
	__asm
	{
        push    eax
        push    edx
        push    esi
        call    CreateRandomThirdDigit
        mov     ecx, eax
        pop     esi
        pop     edx
        pop     eax

        push    ecx
        push    edx
        push    esi
        lea     ecx, [esp + 3Ch]
        jmp     [returnAddress]
	}
}

//---------------------------------------------------
// Checkpoint
//---------------------------------------------------
std::string Stage::m_lapTimeStr;
float Stage::m_checkpointTimer = 0.0f;
HOOK(void, __fastcall, Stage_MsgNotifyLapTimeHud, 0x1097640, void* This, void* Edx, uint32_t a2)
{
    float lapTime = *(float*)(a2 + 20);
    int minute = (int)(lapTime / 60.0f);
    int seconds = (int)(lapTime - 60.0f * (float)minute);
    int milliseconds = (int)(lapTime * 1000.0f) % 1000;

    char buffer[20];
    sprintf(buffer, "%02d'%02d\"%03d", minute, seconds, milliseconds);

    Stage::m_lapTimeStr = std::string(buffer);
    Stage::m_checkpointTimer = 3.0f;

    return;
}

//---------------------------------------------------
// HUD Update
//---------------------------------------------------
HOOK(void, __fastcall, Stage_CHudSonicStageUpdate, 0x1098A50, void* This, void* Edx, float* dt)
{
    // Always clamp boost to 100
    *Common::GetPlayerBoost() = min(*Common::GetPlayerBoost(), 100.0f);

    originalStage_CHudSonicStageUpdate(This, Edx, dt);
}

//---------------------------------------------------
// Boost Particle
//---------------------------------------------------
HOOK(void, __fastcall, Stage_MsgGetHudPosition, 0x1096790, void* This, void* Edx, MsgGetHudPosition* message)
{
    if (message->m_type == 0)
    {
        Eigen::Vector3f sonicPosition;
        Eigen::Quaternionf sonicRotation;
        if (Common::GetPlayerTransform(sonicPosition, sonicRotation))
        {
            sonicPosition += sonicRotation * (Eigen::Vector3f::UnitY() * 0.5f); // half Sonic's height
            message->m_position = sonicPosition;
            return;
        }
    }

    originalStage_MsgGetHudPosition(This, Edx, message);
}

void Stage::applyPatches()
{
    // Install hooks to improve fading transitions.
    INSTALL_HOOK(MsgFadeInFxp);
    INSTALL_HOOK(MsgFadeOutFxp);
    INSTALL_HOOK(MsgFadeInMtfx);
    INSTALL_HOOK(MsgFadeOutMtfx);

    // Prevent timer getting reset twice after fade in is completed
    WRITE_MEMORY(0xCFDD8F, uint8_t, 0xEB);

    // Use single digit for life counter.
    WRITE_STRING(0x168E8C4, "%d");

    // Calculate correct 3 digit milliseconds for stage time
    WRITE_MEMORY(0x10B37BF, double*, (double*)0x1703970); // *100 -> *1000
    WRITE_MEMORY(0x10B37F7, uint32_t, 9162597); // 9162597/2^39 ~= 1/60000
    WRITE_MEMORY(0x10B3830, int32_t, -60000); // sec = total_sec + min * (-60000) = total_sec % 60000
    WRITE_MEMORY(0x10B3837, uint32_t, 137438954); // 137438954/2^37 ~= 1/1000
    WRITE_JUMP(0x10B3842, Stage_GetCurrentMillisecond);
    WRITE_MEMORY(0x10B3823, uint32_t, 999); // maximum milliseconds
    WRITE_MEMORY(0x1098D75, uint32_t, 0x168E8E0); // %03d

    // Change mission countdown time to have MM:SS.xxx format
    WRITE_JUMP(0x124F08D, Stage_GetMissionCurrentTime);

    // Set mission count down have 3 digit millisecond
    WRITE_MEMORY(0x124F0B1, double*, (double*)0x1703970); // *100 -> *1000
    WRITE_MEMORY(0x124F0BF, uint32_t, 0x168E8E0); // %03d

    // Display a random 3rd digit for next rank time in mission
    WRITE_STRING(0x1693474, "%02d:%02d.%03d");
    WRITE_JUMP(0x10B16CB, Stage_GetMissionNextRankMaxTime);
    WRITE_JUMP(0x10B16F2, Stage_GetMissionNextRankRandomDigit);

    // Other instances using sub_10B37B0
    WRITE_STRING(0x1689274, "--:--.---");
    WRITE_STRING(0x1689280, "%02d:%02d.%03d");
    WRITE_STRING(0x1689290, "--:--.---");
    WRITE_STRING(0x168929C, "%02d:%02d.%03d");
    WRITE_STRING(0x168CD0C, "99:59.999");
    WRITE_STRING(0x168CD18, "%02d:%02d.%03d");
    WRITE_STRING(0x16941C0, "%02d:%02d.%03d");
    WRITE_MEMORY(0x117BF38, uint32_t, 0x168E8E0); // %03d 
    WRITE_MEMORY(0x11CCCC2, uint32_t, 0x168E8E0); // %03d

    // Set laptime always display current time
    WRITE_MEMORY(0x1032FD1, uint8_t, 0xEB);
    WRITE_MEMORY(0x1033040, uint8_t, 0xEB);
    WRITE_NOP(0x10330A0, 6);
    WRITE_JUMP(0x103312C, (void*)0x103313E);
    WRITE_MEMORY(0x10978E4, double*, (double*)0x1703970); // *100 -> *1000
    INSTALL_HOOK(Stage_MsgNotifyLapTimeHud);

    // Install HUD update hook
    INSTALL_HOOK(Stage_CHudSonicStageUpdate);

    // Make boost particles goes to Sonic
    INSTALL_HOOK(Stage_MsgGetHudPosition);

    // TODO: Remove Life + 1 UI
}

void Stage::draw()
{
    // At loading screen, clear all
    if (Common::IsAtLoadingScreen())
    {
        m_checkpointTimer = 0.0f;
        return;
    }

    if (m_checkpointTimer > 0.0f)
    {
        static bool visible = true;
        ImGui::Begin("Checkpoint", &visible, UIContext::m_hudFlags);
        {
            ImVec2 size = ImGui::CalcTextSize(m_lapTimeStr.c_str());
            ImGui::Text(m_lapTimeStr.c_str());
            ImGui::SetWindowFocus();
            ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.5f - size.x * 0.5f, (float)*BACKBUFFER_HEIGHT * 0.882f - size.y * 0.5f));
        }
        ImGui::End();

        m_checkpointTimer -= Application::getHudDeltaTime();
    }
}
