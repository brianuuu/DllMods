#include "RankRunAnimation.h"

#include "Configuration.h"
#include <Hedgehog.h>

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

const char* volatile const RunResult = "RunResult";
const char* volatile const RunResultLoop = "RunResultLoop";
HOOK(void*, __cdecl, InitializeSonicAnimationList, 0x1272490)
{
    void* pResult = originalInitializeSonicAnimationList();
    {
        CAnimationStateSet* pList = (CAnimationStateSet*)0x15E8D40;

        CAnimationStateInfo* pEntries = new CAnimationStateInfo[pList->m_Count + 2];
        std::copy(pList->m_pEntries, pList->m_pEntries + pList->m_Count, pEntries);

        pEntries[pList->m_Count].m_Name = RunResult;
        pEntries[pList->m_Count].m_FileName = "sn_result_run";
        pEntries[pList->m_Count].m_Speed = 1.0f;
        pEntries[pList->m_Count].m_PlaybackType = 1;
        pEntries[pList->m_Count].field10 = 0;
        pEntries[pList->m_Count].field14 = -1.0f;
        pEntries[pList->m_Count].field18 = -1.0f;
        pEntries[pList->m_Count].field1C = 0;
        pEntries[pList->m_Count].field20 = -1;
        pEntries[pList->m_Count].field24 = -1;
        pEntries[pList->m_Count].field28 = -1;
        pEntries[pList->m_Count].field2C = -1;

        pEntries[pList->m_Count + 1].m_Name = RunResultLoop;
        pEntries[pList->m_Count + 1].m_FileName = "sn_result_run_loop";
        pEntries[pList->m_Count + 1].m_Speed = 1.0f;
        pEntries[pList->m_Count + 1].m_PlaybackType = 0;
        pEntries[pList->m_Count + 1].field10 = 0;
        pEntries[pList->m_Count + 1].field14 = -1.0f;
        pEntries[pList->m_Count + 1].field18 = -1.0f;
        pEntries[pList->m_Count + 1].field1C = 0;
        pEntries[pList->m_Count + 1].field20 = -1;
        pEntries[pList->m_Count + 1].field24 = -1;
        pEntries[pList->m_Count + 1].field28 = -1;
        pEntries[pList->m_Count + 1].field2C = -1;

        WRITE_MEMORY(&pList->m_pEntries, void*, pEntries);
        WRITE_MEMORY(&pList->m_Count, size_t, pList->m_Count + 2);
    }

    return pResult;
}

FUNCTION_PTR(void*, __stdcall, fpCreateAnimationState, 0xCDFA20, void* This, boost::shared_ptr<void>& spAnimationState, const Hedgehog::Base::CSharedString& name, const Hedgehog::Base::CSharedString& name2);
HOOK(void, __fastcall, CSonicCreateAnimationStates, 0xE1B6C0, void* This, void* Edx, void* A2, void* A3)
{
    boost::shared_ptr<void> spAnimationState;
    fpCreateAnimationState(A2, spAnimationState, RunResult, RunResult);
    fpCreateAnimationState(A2, spAnimationState, RunResultLoop, RunResultLoop);

    originalCSonicCreateAnimationStates(This, Edx, A2, A3);
}

#define STH2006_RUN_STAGE_COUNT 4
const char* sth2006RunStageIDs[STH2006_RUN_STAGE_COUNT] = { "ghz200", "sph200", "ssh200", "euc200" };
bool checkCanPlayRunAnimation()
{
    // Not Modern Sonic
    void* const pModernSonicContext = *(void**)0x1E5E2F8;
    if (!pModernSonicContext) return false;

    // Currently in super form
    uint32_t superSonicAddress = (uint32_t)(pModernSonicContext) + 0x1A0;
    if (*(void**)superSonicAddress) return false;

    char* currentStageID = (char*)0x01E774D4;
    if (Configuration::m_run == RunResultType::STH2006)
    {
        // Only enable for STH2006 Project stages
        for (int i = 0; i < STH2006_RUN_STAGE_COUNT; i++)
        {
            if (strcmp(currentStageID, sth2006RunStageIDs[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

    if (Configuration::m_run == RunResultType::Custom)
    {
        // Only enable for custom defined stages
        for (string const& stage : Configuration::m_runStages)
        {
            if (strcmp(currentStageID, stage.c_str()) == 0)
            {
                return true;
            }
        }
        return false;
    }

    // RunResultType::EnableAll
    return true;
}

FUNCTION_PTR(void, __thiscall, CSonicContextChangeAnimation, 0xE74CC0, void* This, const Hedgehog::Base::CSharedString& name);
HOOK(void, __fastcall, MsgChangeResultState, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (checkCanPlayRunAnimation())
    {
        if (state == 1)
        {
            void* const pModernSonicContext = *(void**)0x1E5E2F8;
            CSonicContextChangeAnimation(pModernSonicContext, RunResult);
        }
        else if (state == 3)
        {
            // Skip the normal rank animation
            return;
        }
    }

    originalMsgChangeResultState(This, Edx, a2);
}

float flt_15C8614 = -1.0f;
uint32_t sub_CE0600 = 0xCE0600;
uint32_t sub_CDFB40 = 0xCDFB40;
uint32_t sub_662010 = 0x662010;
uint32_t stringConstructor = 0x6621A0;
uint32_t stringDestructor = 0x661550;
uint32_t addTransitionReturnAddress = 0xE22C61;
void __declspec(naked) addTransition()
{
    __asm
    {
        // Unknown function, still works without but do it anyway
        sub     esp, 4
        push    RunResult
        lea     ecx, [esp + 4]
        call    [stringConstructor]
        lea     ecx, [esp]
        mov     ecx, ebx
        call    [sub_CE0600]
        lea     ecx, [esp]
        call    [stringDestructor]
        add     esp, 4

        // Unknown function, still works without but do it anyway
        sub     esp, 4
        push    RunResultLoop
        lea     ecx, [esp + 4]
        call    [stringConstructor]
        lea     ecx, [esp]
        mov     ecx, ebx
        call    [sub_CE0600]
        lea     ecx, [esp]
        call    [stringDestructor]
        add     esp, 4

        // Construct strings
        sub     esp, 8
        push    RunResult
        lea     ecx, [esp + 8]
        call[stringConstructor]
        push    RunResultLoop
        lea     ecx, [esp + 4]
        call[stringConstructor]

        // Creates transition for RunResult
        lea     eax, [esp + 4] // RunResult
        push    eax
        push    ebx
        call    [sub_CDFB40]
        mov     eax, [eax]
        movss   xmm0, flt_15C8614
        lea     ecx, [esp] // RunResultLoop
        push    ecx
        lea     ecx, [eax + 0x88]
        mov     byte ptr [eax + 0x90], 1
        movss   dword ptr [eax + 0x8C], xmm0
        call    [sub_662010]

        // Destruct strings
        lea     ecx, [esp + 4] // RunResult
        call    [stringDestructor]
        lea     ecx, [esp] // RunResultLoop
        call    [stringDestructor]
        add     esp, 8

        // Resume original
        push    [0x015F8B5C]    // offset aCatchrocket
        jmp     [addTransitionReturnAddress]
    }
}

void RankRunAnimation::applyPatches()
{
    if (Configuration::m_model != ModelType::Sonic) return;
    if (Configuration::m_run == RunResultType::Disable) return;

    // Add run goal animations to the animation list
    INSTALL_HOOK(InitializeSonicAnimationList);
    INSTALL_HOOK(CSonicCreateAnimationStates);

    // Add transition asm for RunResult->RunResultLoop
    WRITE_JUMP(0xE22C5C, addTransition);

    // Play animation when screen faded to white
    // Ignore original change animation during result screen
    INSTALL_HOOK(MsgChangeResultState);
}
