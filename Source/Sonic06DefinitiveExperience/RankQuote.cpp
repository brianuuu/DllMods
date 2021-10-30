#include "RankQuote.h"
#include "RankRunAnimation.h"

uint32_t RankQuote::m_rank = 0;
uint32_t RankQuote::m_rankSfxID = 1010002;
bool RankQuote::m_playRankVoice = false;

void __cdecl getRankSfx()
{
    if (*pModernSonicContext && RankQuote::m_playRankVoice)
    {
        // S:40000 - D:40004
        RankQuote::m_rankSfxID = 40004 - RankQuote::m_rank;
    }
    else
    {
        // Default rank sfx ID
        RankQuote::m_rankSfxID = 1010002;
    }

    RankQuote::m_playRankVoice = false;
}

void __declspec(naked) asmChangeRankVoice()
{
    static uint32_t returnAddress = 0x11D237E;
    __asm
    {
        // eax is used after jumping back
        push    eax
        call    getRankSfx
        pop     eax

        push    RankQuote::m_rankSfxID
        jmp     [returnAddress]
    }
}

void __declspec(naked) asmSkipRankPerfect()
{
    static uint32_t returnAddress = 0x10B8D4A;
    __asm
    {
        mov     RankQuote::m_playRankVoice, 1  // enable voice
        push    [0x1693E80]         // offset aChangerank

        jmp     [returnAddress]
    }
}

void __declspec(naked) asmSkipRank()
{
    static uint32_t returnAddress = 0x10B8DAD;
    __asm
    {
        mov     RankQuote::m_playRankVoice, 1  // enable voice
        push    [0x1693E64]         // offset aRank_1

        jmp     [returnAddress]
    }
}

void __declspec(naked) asmRankPerfect()
{
    static uint32_t returnAddress = 0x10B77AD;
    __asm
    {
        mov     RankQuote::m_playRankVoice, 1  // enable voice
        push    [0x1693E80]         // offset aChangerank

        jmp     [returnAddress]
    }
}

void __declspec(naked) asmRank()
{
    static uint32_t perfectBonusFunctionAddress = 0x10B8A90;
    static uint32_t returnAddress = 0x10B785B;
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

        mov     RankQuote::m_playRankVoice, 1  // enable voice

        jump:
        push    [0x1693E64]         // offset aRank_1
        jmp     [returnAddress]
    }
}

HOOK(void, __fastcall, MsgChangeResultState2, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    RankQuote::m_rank = *(uint32_t*)(a2 + 20);
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (state == 3 
    && !RankRunAnimation::checkCanPlayRunAnimation()
    && !Common::IsPlayerSuper())
    {
        // Play character stage complete voice
        static SharedPtrTypeless soundHandle;
        Common::PlaySoundStatic(soundHandle, 3002019);
    }

    originalMsgChangeResultState2(This, Edx, a2);
}

void RankQuote::applyPatches()
{
    // Disable result animation skip
    WRITE_NOP(0x10B9C1A, 0x5);

    // Change rank sfx
    WRITE_JUMP(0x11D2379, asmChangeRankVoice);

    // Functions that triggers rank voice
    WRITE_JUMP(0x10B8D45, asmSkipRankPerfect);
    WRITE_JUMP(0x10B8DA8, asmSkipRank);
    WRITE_JUMP(0x10B77A8, asmRankPerfect);
    WRITE_JUMP(0x10B7856, asmRank);

    // Play character stage complete voice
    INSTALL_HOOK(MsgChangeResultState2);
}
