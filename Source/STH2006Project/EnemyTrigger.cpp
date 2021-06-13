#include "EnemyTrigger.h"

uint32_t const MsgNotifyObjectEventOffset = 0x16811C0;
FUNCTION_PTR(void*, __stdcall, fpEventTrigger, 0xD5ED00, void* This, int Event);
#define DAMAGE_EVENT_ASM(enemyName, address, damageFunctionAddress) \
    uint32_t const enemyName##DamageEventReturnAddress = address; \
    uint32_t const enemyName##JumpToDamageCall = address + 0x0D; \
    uint32_t enemyName##Damaging = 0; \
    void __declspec(naked) enemyName##DamageEvent() \
    { \
        __asm \
        { \
            /*Test if event is "MsgNotifyObjectEvent"*/ \
            __asm mov   eax, [esi] \
            __asm mov   edx, [eax + 4] \
            __asm push  [MsgNotifyObjectEventOffset] \
            __asm mov   ecx, esi \
            __asm call  edx \
            __asm test  al, al \
            __asm jz    jump \
            /*Test if event number is 12*/ \
            __asm mov   eax, [esi + 0x10] \
            __asm cmp   eax, 0x0C \
            __asm jz    jumpSuccess \
            /*Original function (checking MsgDamage)*/ \
            __asm jump: \
            __asm mov   eax, [esi] \
            __asm mov   edx, [eax + 4] \
            __asm jmp   [enemyName##DamageEventReturnAddress] \
            /*Success*/ \
            __asm jumpSuccess: \
            __asm mov   enemyName##Damaging, 1 \
            __asm jmp   [enemyName##JumpToDamageCall] \
        } \
    } \
    HOOK(void, __fastcall, enemyName##MsgDamage, damageFunctionAddress, void* This, void* Edx, uint32_t* a2) \
    { \
        /*Fire event 4*/ \
        fpEventTrigger(This, 4); \
        original##enemyName##MsgDamage(This, Edx, a2);\
    }

#define DAMAGE_EXPLODE_ASM(enemyName, failAddress, successAddress, originalASM, successASM) \
    uint32_t const enemyName##ExplodeFailAddress = failAddress; \
    uint32_t const enemyName##ExplodeSuccessAddress = successAddress; \
    extern uint32_t enemyName##Damaging; \
    void __declspec(naked) enemyName##ExplodeASM() \
    { \
        __asm \
        { \
            /*test explode*/ \
            __asm cmp     enemyName##Damaging, 1 \
            __asm je      jumpSuccess \
            /*original function*/ \
            originalASM \
            __asm jump: \
            __asm jmp     [enemyName##ExplodeFailAddress] \
            /*success*/ \
            __asm jumpSuccess: \
            successASM \
            __asm mov     enemyName##Damaging, 0 \
            __asm jmp     [enemyName##ExplodeSuccessAddress] \
        } \
    }

#define WRITE_JUMP_EVENT_TRIGGER(address, enemyName) \
    WRITE_JUMP(address, enemyName##DamageEvent); \
    INSTALL_HOOK(enemyName##MsgDamage);

// ---------------------------------------------------
// Enemies that explodes on spot
// ---------------------------------------------------
DAMAGE_EVENT_ASM(EnemyEChaserSV, 0xB763AF, 0xB76340);
DAMAGE_EVENT_ASM(EnemyAeroCannon, 0xB7F1BF, 0xB7F100);
DAMAGE_EVENT_ASM(EnemyBeetle, 0xBA652F, 0xBA6450);
DAMAGE_EVENT_ASM(EnemyEggRobo, 0xBB023F, 0xBB01B0);
DAMAGE_EVENT_ASM(EnemyGrabber, 0xBC3B4F, 0xBC39F0);
DAMAGE_EVENT_ASM(EnemyBatabata, 0xBD7EAF, 0xBD7E60);
DAMAGE_EVENT_ASM(EnemyBeeton, 0xBDC19F, 0xBDC110);

// ---------------------------------------------------
// Enemies that has animations when getting hit
// ---------------------------------------------------
DAMAGE_EVENT_ASM(EnemyELauncher, 0xB82B5F, 0xB82900);
#define ENEMY_E_LAUNCHER_ASM __asm cmp [esp+0x20-0x11], 0 __asm jz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyELauncher, 0xB82986, 0xB82AED, ENEMY_E_LAUNCHER_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyCrawler, 0xB99E2F, 0xB99B80);
#define ENEMY_CRAWLER_ASM __asm mov ebx, [ebp+0x8] __asm mov ecx, [ebx+0x10]
#define ENEMY_CRAWLER_SUCCESS_ASM __asm mov ebx, [ebp+0x8]
DAMAGE_EXPLODE_ASM(EnemyCrawler, 0xB99D1E, 0xB99D50, ENEMY_CRAWLER_ASM, ENEMY_CRAWLER_SUCCESS_ASM);

DAMAGE_EVENT_ASM(EnemyGunHunter, 0xBAA61F, 0xBAA2F0);
#define ENEMY_GUN_HUNTER_ASM __asm cmp byte ptr [esp+0x40-0x34+0x3], 0 __asm jz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyGunHunter, 0xBAA37B, 0xBAA585, ENEMY_GUN_HUNTER_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyCopSpeeder, 0xBBA6CF, 0xBBA530);
#define ENEMY_COP_SPEEDER_ASM __asm test bl, bl __asm jz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyCopSpeeder, 0xBBA59D, 0xBBA68A, ENEMY_COP_SPEEDER_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyMotora, 0xBC736F, 0x5FA2E0);
#define ENEMY_MOTORA_ASM __asm test esi, esi __asm jnz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyMotora, 0xBC74AA, 0xBC753C, ENEMY_MOTORA_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyGanigani, 0xBCB8EF, 0x5FA2E0);
#define ENEMY_GANIGANI_ASM __asm test esi, esi __asm jnz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyGanigani, 0xBCBA10, 0xBCBA9A, ENEMY_GANIGANI_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyLander, 0xBCF75F, 0xBCF5E0);
#define ENEMY_LANDER_ASM __asm test bl, bl __asm jz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyLander, 0xBCF64D, 0xBCF705, ENEMY_LANDER_ASM, __asm nop);

DAMAGE_EVENT_ASM(EnemyEFighter, 0xBD4DEF, 0xBD49E0);
#define ENEMY_E_FIGHTER_ASM __asm test esi, esi __asm jnz jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyEFighter, 0xBD4B35, 0xBD4CB7, ENEMY_E_FIGHTER_ASM, __asm nop);

// ---------------------------------------------------
// Enemies that has animations when getting hit, or need to bypass certain functions
// ---------------------------------------------------
DAMAGE_EVENT_ASM(EnemyNal, 0xB9EA2F, 0xB9E8D0);
#define ENEMY_NAL_ASM __asm test bl, bl __asm jnz jump __asm push esi __asm jmp jumpSuccess
DAMAGE_EXPLODE_ASM(EnemyNal, 0xB9E9F8, 0xB9E9D9, ENEMY_NAL_ASM, __asm push esi);
uint32_t const sub_4F8840 = 0x4F8840;
uint32_t const EnemyNalExtraASMReturnAddress = 0xB9E98C;
void __declspec(naked) EnemyNalExtraASM()
{
    __asm
    {
        cmp     EnemyNalDamaging, 1
        je      jump

        call    [sub_4F8840]

        jump:
        jmp     [EnemyNalExtraASMReturnAddress]
    }
}

DAMAGE_EVENT_ASM(EnemyTaker, 0xBA326F, 0xBA3140);
DAMAGE_EVENT_ASM(EnemyBiter, 0xB869CB, 0xB86850);
uint32_t const EnemyBiterExtraASMReturnAddress = 0xB8692D;
void __declspec(naked) EnemyBiterExtraASM()
{
    __asm
    {
        cmp     EnemyBiterDamaging, 1
        je      jump

        call    [sub_4F8840]

        jump:
        jmp     [EnemyBiterExtraASMReturnAddress]
    }
}

uint32_t const sub_4F87B0 = 0x4F87B0;
uint32_t const EnemySharedExtraASMReturnAddress = 0xBE0C01;
uint32_t const EnemySharedExtraASMSuccessAddress = 0xBE0C47;
void __declspec(naked) EnemySharedExtraASM()
{
    __asm
    {
        /*test explode*/
        cmp     EnemyTakerDamaging, 1
        je      jumpEnemyTaker

        cmp     EnemyBiterDamaging, 1
        je      jumpEnemyBiter

        /*original function*/
        mov     ecx, [edi + 0x10]
        push    ecx
        call    [sub_4F87B0]
        jmp     [EnemySharedExtraASMReturnAddress]

        /*success*/
        jumpEnemyTaker:
        add     esp, 0x10
        mov     EnemyTakerDamaging, 0
        jmp     [EnemySharedExtraASMSuccessAddress]

        jumpEnemyBiter:
        add     esp, 0x10
        mov     EnemyBiterDamaging, 0
        jmp     [EnemySharedExtraASMSuccessAddress]
    }
}

DAMAGE_EVENT_ASM(EnemySpinner, 0xBBDA4F, 0xBBD990);
uint32_t const EnemySpinnerExtraASMReturnAddress = 0xBBD9A3;
uint32_t const EnemySpinnerExtraASMSuccessAddress = 0xBBD9B9;
void __declspec(naked) EnemySpinnerExtraASM()
{
    __asm
    {
        cmp     EnemySpinnerDamaging, 1
        je      jump

        cmp     byte ptr[esi + 239h], 0
        jmp     [EnemySpinnerExtraASMReturnAddress]

        jump:
        jmp     [EnemySpinnerExtraASMSuccessAddress]
    }
}


DAMAGE_EVENT_ASM(EnemyPawn, 0xB958EF, 0xB907E0);
#define ENEMY_PAWN_ASM __asm call [sub_4F8840]
#define ENEMY_PAWN_SUCCESS_ASM __asm add esp, 0x10
DAMAGE_EXPLODE_ASM(EnemyPawn, 0xB935FB, 0xB93666, ENEMY_PAWN_ASM, ENEMY_PAWN_SUCCESS_ASM);
uint32_t const EnemyPawnExtraASMReturnAddress = 0xB95A53;
uint32_t const EnemyPawnExtraASMSuccessAddress = 0xB95AB1;
void __declspec(naked) EnemyPawnExtraASM()
{
    __asm
    {
        cmp     EnemyPawnDamaging, 1
        je      jump

        call    [sub_4F8840]
        jmp     [EnemyPawnExtraASMReturnAddress]

        jump:
        add     esp, 0x10
        jmp     [EnemyPawnExtraASMSuccessAddress]
    }
}
uint32_t const EnemyPawnExtraASM2ReturnAddress = 0xB95B3A;
uint32_t const EnemyPawnExtraASM2SuccessAddress = 0xB95CB5;
void __declspec(naked) EnemyPawnExtraASM2()
{
    __asm
    {
        cmp     EnemyPawnDamaging, 1
        je      jumpSuccess

        cmp     [esp + 0x30 -0x1D], 0
        jnz     jump
        cmp     [esp + 0x30 - 0x1E], 0
        jnz     jumpSuccess

        jump:
        jmp     [EnemyPawnExtraASM2ReturnAddress]

        jumpSuccess:
        jmp     [EnemyPawnExtraASM2SuccessAddress]
    }
}
uint32_t const EnemyPawnPLAExtraASMReturnAddress = 0xB8C602;
uint32_t const EnemyPawnPLAExtraASMSuccessAddress = 0xB8C5EE;
void __declspec(naked) EnemyPawnPLAExtraASM()
{
    __asm
    {
        // Test if event is "MsgNotifyObjectEvent"
        call    edx
        test    al, al
        jz      jump

        // Test if event number is 12
        mov     eax, [esi + 0x10]
        cmp     eax, 0x0C
        jz      jump
        jmp     [EnemyPawnPLAExtraASMSuccessAddress]
        
        // force run damage
        jump:
        jmp     [EnemyPawnPLAExtraASMReturnAddress]
    }
}

void EnemyTrigger::applyPatches()
{
    // ---------------------------------------------------
    // This allows ALL objects in TriggerList to inialize list
    // Ideally just want to do that for enemies, but works for now
    WRITE_NOP(0xD5F9CB, 0xD);

    // ---------------------------------------------------
    // Enemies that explodes on spot
    // ---------------------------------------------------
    WRITE_JUMP_EVENT_TRIGGER(0xB763AA, EnemyEChaserSV);
    WRITE_JUMP_EVENT_TRIGGER(0xB7F1BA, EnemyAeroCannon);
    WRITE_JUMP_EVENT_TRIGGER(0xBA652A, EnemyBeetle);
    WRITE_JUMP_EVENT_TRIGGER(0xBB023A, EnemyEggRobo);
    WRITE_JUMP_EVENT_TRIGGER(0xBC3B4A, EnemyGrabber);
    WRITE_JUMP_EVENT_TRIGGER(0xBD7EAA, EnemyBatabata);
    WRITE_JUMP_EVENT_TRIGGER(0xBDC19A, EnemyBeeton);

    // ---------------------------------------------------
    // Enemies that has animations when getting hit
    // ---------------------------------------------------
    WRITE_JUMP_EVENT_TRIGGER(0xB82B5A, EnemyELauncher);
    WRITE_JUMP(0xB8297B, EnemyELauncherExplodeASM);

    WRITE_JUMP_EVENT_TRIGGER(0xB99E2A, EnemyCrawler);
    WRITE_JUMP(0xB99D18, EnemyCrawlerExplodeASM);
    WRITE_NOP(0xB99D1D, 0x1);

    WRITE_JUMP_EVENT_TRIGGER(0xBAA61A, EnemyGunHunter);
    WRITE_JUMP(0xBAA370, EnemyGunHunterExplodeASM);

    WRITE_JUMP_EVENT_TRIGGER(0xBBA6CA, EnemyCopSpeeder);
    WRITE_JUMP(0xBBA595, EnemyCopSpeederExplodeASM);
    WRITE_NOP(0xBBA59A, 0x3);

    WRITE_JUMP_EVENT_TRIGGER(0xBC736A, EnemyMotora);
    WRITE_JUMP(0xBC74A2, EnemyMotoraExplodeASM);
    WRITE_NOP(0xBC74A7, 0x3);

    WRITE_JUMP_EVENT_TRIGGER(0xBCB8EA, EnemyGanigani);
    WRITE_JUMP(0xBCBA08, EnemyGaniganiExplodeASM);
    WRITE_NOP(0xBCBA0D, 0x3);

    WRITE_JUMP_EVENT_TRIGGER(0xBCF75A, EnemyLander);
    WRITE_JUMP(0xBCF645, EnemyLanderExplodeASM);
    WRITE_NOP(0xBCF64A, 0x3);

    WRITE_JUMP_EVENT_TRIGGER(0xBD4DEA, EnemyEFighter);
    WRITE_JUMP(0xBD4B2D, EnemyEFighterExplodeASM);
    WRITE_NOP(0xBD4B32, 0x3);

    // ---------------------------------------------------
    // Enemies that has animations when getting hit, or need to bypass certain functions
    // ---------------------------------------------------
    WRITE_JUMP_EVENT_TRIGGER(0xB9EA2A, EnemyNal);
    WRITE_JUMP(0xB9E9D4, EnemyNalExplodeASM);
    WRITE_JUMP(0xB9E987, EnemyNalExtraASM);

    WRITE_JUMP_EVENT_TRIGGER(0xBBDA4A, EnemySpinner);
    WRITE_JUMP(0xBBD99C, EnemySpinnerExtraASM);
    WRITE_NOP(0xBBD9A1, 0x2);

    WRITE_JUMP_EVENT_TRIGGER(0xB958EA, EnemyPawn);
    WRITE_JUMP(0xB935F6, EnemyPawnExplodeASM);
    WRITE_JUMP(0xB95A4E, EnemyPawnExtraASM);
    WRITE_JUMP(0xB95B28, EnemyPawnExtraASM2);
    WRITE_JUMP(0xB8C5E8, EnemyPawnPLAExtraASM);

    WRITE_JUMP_EVENT_TRIGGER(0xBA326A, EnemyTaker);
    WRITE_JUMP_EVENT_TRIGGER(0xB869C6, EnemyBiter);
    WRITE_JUMP(0xB86928, EnemyBiterExtraASM);
    WRITE_JUMP(0xBE0BF8, EnemySharedExtraASM);
    WRITE_NOP(0xBE0BFD, 0x4);
}
