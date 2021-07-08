#include "RankQuote.h"

uint32_t RankQuote::m_rank = 0;
uint32_t RankQuote::m_rankSfxID = 1010002;
bool RankQuote::m_playRankSfx = false;

#if _DEBUG
void __cdecl debugRank()
{
    switch (RankQuote::m_rank)
    {
    case 4: printf("Got S rank!\n"); break;
    case 3: printf("Got A rank!\n"); break;
    case 2: printf("Got B rank!\n"); break;
    case 1: printf("Got C rank!\n"); break;
    default: printf("Got D rank!\n"); break;
    }
}
#endif

uint32_t asmGetRankReturnAddress = 0xE27BC2;
void __declspec(naked) asmGetRank()
{
    __asm
    {
        mov     eax, [esi + 0x14]
        mov     RankQuote::m_rank, eax

#if _DEBUG
        // eax is used after
        push    eax
        call    debugRank
        pop     eax
#endif

        cmp     eax, 4
        jmp     [asmGetRankReturnAddress]
    }
}

void __cdecl getRankSfx()
{
    /*
    0x1E5E2F8: Modern context
    0x1E5E304: Classic context
    0x1E5E310: Super Sonic context
    */
    void* pModernSonicContext = *(void**)0x1E5E2F8;
    if (pModernSonicContext && RankQuote::m_playRankSfx)
    {
        // S:40000 - D:40004
        RankQuote::m_rankSfxID = 40004 - RankQuote::m_rank;

        // E-Rank Generations support
        if (RankQuote::m_rank == 4 && *(uint8_t*)0x15EFE9D == 0x45)
        {
            RankQuote::m_rankSfxID = 40005;
        }
    }
    else
    {
        // Default rank sfx ID
        RankQuote::m_rankSfxID = 1010002;
    }

    RankQuote::m_playRankSfx = false;
}

uint32_t asmChangeRankVoiceReturnAddress = 0x11D237E;
void __declspec(naked) asmChangeRankSfx()
{
    __asm
    {
        // eax is used after jumping back
        push    eax
        call    getRankSfx
        pop     eax

        push    RankQuote::m_rankSfxID
        jmp     [asmChangeRankVoiceReturnAddress]
    }
}

uint32_t asmSkipRankPerfectReturnAddress = 0x10B8D4A;
void __declspec(naked) asmSkipRankPerfect()
{
    __asm
    {
        mov     RankQuote::m_playRankSfx, 1  // enable voice
        push    [0x1693E80]         // offset aChangerank

        jmp     [asmSkipRankPerfectReturnAddress]
    }
}

uint32_t asmSkipRankReturnAddress = 0x10B8DAD;
void __declspec(naked) asmSkipRank()
{
    __asm
    {
        mov     RankQuote::m_playRankSfx, 1  // enable voice
        push    [0x1693E64]         // offset aRank_1

        jmp     [asmSkipRankReturnAddress]
    }
}

uint32_t asmRankPerfectReturnAddress = 0x10B77AD;
void __declspec(naked) asmRankPerfect()
{
    __asm
    {
        mov     RankQuote::m_playRankSfx, 1  // enable voice
        push    [0x1693E80]         // offset aChangerank

        jmp     [asmRankPerfectReturnAddress]
    }
}

uint32_t perfectBonusFunctionAddress = 0x10B8A90;
uint32_t asmRankReturnAddress = 0x10B785B;
void __declspec(naked) asmRank()
{
    __asm
    {
        // Only plays when there's NO pefect bonus
        push    ecx
        push    edx
        call    [perfectBonusFunctionAddress]
        pop     edx
        pop     ecx

        test    al, al
        jnz     jump

        mov     RankQuote::m_playRankSfx, 1  // enable voice

        jump:
        push    [0x1693E64]         // offset aRank_1
        jmp     [asmRankReturnAddress]
    }
}

using SharedPtrTypeless = boost::shared_ptr<void>;
FUNCTION_PTR(void*, __thiscall, sub_75FA60, 0x75FA60, void* This, SharedPtrTypeless&, uint32_t cueId);
void PlaySound(SharedPtrTypeless& soundHandle, uint32_t cueID)
{
    uint32_t* syncObject = *(uint32_t**)0x1E79044;
    if (syncObject)
    {
        sub_75FA60((void*)syncObject[8], soundHandle, cueID);
    }
}

HOOK(void, __stdcall, ChangeAnimation, 0xCDFC80, void* A1, SharedPtrTypeless& A2, const Hedgehog::Base::CSharedString& name)
{
    static SharedPtrTypeless soundHandle;
    if (strcmp(name.m_pStr, "ResultRankS_Link") == 0)
    {
        PlaySound(soundHandle, *(uint8_t*)0x15EFE9D == 0x45 ? 40015 : 40010);
    }
    else if (strcmp(name.m_pStr, "ResultRankA_Link") == 0)
    {
        PlaySound(soundHandle, 40011);
    }
    else if (strcmp(name.m_pStr, "ResultRankB_Link") == 0)
    {
        PlaySound(soundHandle, 40012);
    }
    else if (strcmp(name.m_pStr, "ResultRankC_Link") == 0)
    {
        PlaySound(soundHandle, 40013);
    }
    else if (strcmp(name.m_pStr, "ResultRankD_Link") == 0)
    {
        PlaySound(soundHandle, 40014);
    }

    originalChangeAnimation(A1, A2, name);
}

void RankQuote::applyPatches()
{
    // Grab rank
    WRITE_JUMP(0xE27BBC, asmGetRank);

    // Change rank sfx
    WRITE_JUMP(0x11D2379, asmChangeRankSfx);

    // Functions that triggers rank sfx change
    WRITE_JUMP(0x10B8D45, asmSkipRankPerfect);
    WRITE_JUMP(0x10B8DA8, asmSkipRank);
    WRITE_JUMP(0x10B77A8, asmRankPerfect);
    WRITE_JUMP(0x10B7856, asmRank);

    // Play rank quote on certain animation state
    //INSTALL_HOOK(ChangeAnimation);
}
