#include "RankQuote.h"
#include "RankRunAnimation.h"

uint32_t RankQuote::m_rank = 0;
uint32_t RankQuote::m_rankSfxID = 1010002;
bool RankQuote::m_playRankVoice = false;

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

uint32_t asmChangeRankVoiceReturnAddress = 0x11D237E;
void __declspec(naked) asmChangeRankVoice()
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
        mov     RankQuote::m_playRankVoice, 1  // enable voice
        push    [0x1693E80]         // offset aChangerank

        jmp     [asmSkipRankPerfectReturnAddress]
    }
}

uint32_t asmSkipRankReturnAddress = 0x10B8DAD;
void __declspec(naked) asmSkipRank()
{
    __asm
    {
        mov     RankQuote::m_playRankVoice, 1  // enable voice
        push    [0x1693E64]         // offset aRank_1

        jmp     [asmSkipRankReturnAddress]
    }
}

uint32_t asmRankPerfectReturnAddress = 0x10B77AD;
void __declspec(naked) asmRankPerfect()
{
    __asm
    {
        mov     RankQuote::m_playRankVoice, 1  // enable voice
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

        mov     RankQuote::m_playRankVoice, 1  // enable voice

        jump:
        push    [0x1693E64]         // offset aRank_1
        jmp     [asmRankReturnAddress]
    }
}

HOOK(void, __fastcall, MsgChangeResultState2, 0xE692C0, void* This, void* Edx, uint32_t a2)
{
    uint32_t const state = *(uint32_t*)(a2 + 16);
    if (state == 3 
    && !RankRunAnimation::checkCanPlayRunAnimation()
    && !Common::CheckPlayerSuperForm())
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

    // Grab rank
    WRITE_JUMP(0xE27BBC, asmGetRank);

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
