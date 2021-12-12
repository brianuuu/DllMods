#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
// Parameters:
//
//   float4 g_BlurRate;
//   float4 g_ViewportSize;
//   sampler2D sampColor;
//   sampler2D sampVelocityMap;
//   sampler2D sampZBuffer;
//
//
// Registers:
//
//   Name            Reg   Size
//   --------------- ----- ----
//   g_ViewportSize  c24      1
//   g_BlurRate      c150     1
//   sampColor       s0       1
//   sampVelocityMap s1       1
//   sampZBuffer     s2       1
//

    ps_3_0
    def c0, 1, 0, 0.00392156886, -2
    def c1, 255, -0.00999999978, 0.125, 0
    defi i0, 255, 0, 0, 0
    dcl_texcoord v0.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    mul r0, c0.xxyy, v0.xyxx
    texldl r1, r0, s2
    texldl r2, r0, s1
    mad r1.yz, r2.xyww, c0.z, r2.xxzw
    mad r1.yz, r1, -c0.w, -c0.x
    mul r2.xy, r1.yzzw, c24
    dp2add r1.w, r2, r2, c0.y
    rsq r1.w, r1.w
    rcp r1.w, r1.w
    frc r2.x, -r1.w
    add r1.w, r1.w, r2.x
    min r2.x, r1.w, c1.x
    rcp r1.w, r2.x
    mul r2.yz, r1.w, r1
    texldl r0, r0, s0
    mov r3.zw, c0.y
    mov r4.xyz, r0
    mov r4.w, c0.x
    mov r0.w, c0.x
    rep i0
      mov r1.w, r2.x
      break_lt r1.w, r0.w
      mad r3.xy, r2.yzzw, r0.w, v0
      texldl r5, r3.xyww, s0
      texldl r6, r3, s2
      add r1.w, r1.x, -r6.x
      add r1.w, r1.w, c1.y
      add r5.xyz, r4, r5
      add r5.w, r4.w, c0.x
      cmp r4, r1.w, r4, r5
      add r0.w, r0.w, c0.x
    endrep
    rcp r0.x, r4.w
    mul oC0.xyz, r0.x, r4
    mov r0.y, c0.y
    dp2add r0.x, r1_abs.yzzw, c24, r0.y
    mul_sat r0.x, r0.x, c1.z
    mul r0.x, r0.x, c150.x
    add_sat r0.y, r4.w, -c0.x
    mul oC0.w, r0.y, r0.x

// approximately 52 instruction slots used (10 texture, 42 arithmetic)
#endif

const BYTE g_ps30_main[] =
{
      0,   3, 255, 255, 254, 255, 
     78,   0,  67,  84,  65,  66, 
     28,   0,   0,   0,  11,   1, 
      0,   0,   0,   3, 255, 255, 
      5,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
      4,   1,   0,   0, 128,   0, 
      0,   0,   2,   0, 150,   0, 
      1,   0,  90,   2, 140,   0, 
      0,   0,   0,   0,   0,   0, 
    156,   0,   0,   0,   2,   0, 
     24,   0,   1,   0,  98,   0, 
    140,   0,   0,   0,   0,   0, 
      0,   0, 171,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      2,   0, 184,   0,   0,   0, 
      0,   0,   0,   0, 200,   0, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   6,   0, 216,   0, 
      0,   0,   0,   0,   0,   0, 
    232,   0,   0,   0,   3,   0, 
      2,   0,   1,   0,  10,   0, 
    244,   0,   0,   0,   0,   0, 
      0,   0, 103,  95,  66, 108, 
    117, 114,  82,  97, 116, 101, 
      0, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
    103,  95,  86, 105, 101, 119, 
    112, 111, 114, 116,  83, 105, 
    122, 101,   0, 115,  97, 109, 
    112,  67, 111, 108, 111, 114, 
      0, 171, 171, 171,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 115,  97, 109, 112, 
     86, 101, 108, 111,  99, 105, 
    116, 121,  77,  97, 112,   0, 
      4,   0,  12,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 115,  97, 
    109, 112,  90,  66, 117, 102, 
    102, 101, 114,   0,   4,   0, 
     12,   0,   1,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0, 112, 115,  95,  51, 
     95,  48,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  49,  48,  46,  49, 
      0, 171,  81,   0,   0,   5, 
      0,   0,  15, 160,   0,   0, 
    128,  63,   0,   0,   0,   0, 
    129, 128, 128,  59,   0,   0, 
      0, 192,  81,   0,   0,   5, 
      1,   0,  15, 160,   0,   0, 
    127,  67,  10, 215,  35, 188, 
      0,   0,   0,  62,   0,   0, 
      0,   0,  48,   0,   0,   5, 
      0,   0,  15, 240, 255,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  31,   0,   0,   2, 
      5,   0,   0, 128,   0,   0, 
      3, 144,  31,   0,   0,   2, 
      0,   0,   0, 144,   0,   8, 
     15, 160,  31,   0,   0,   2, 
      0,   0,   0, 144,   1,   8, 
     15, 160,  31,   0,   0,   2, 
      0,   0,   0, 144,   2,   8, 
     15, 160,   5,   0,   0,   3, 
      0,   0,  15, 128,   0,   0, 
     80, 160,   0,   0,   4, 144, 
     95,   0,   0,   3,   1,   0, 
     15, 128,   0,   0, 228, 128, 
      2,   8, 228, 160,  95,   0, 
      0,   3,   2,   0,  15, 128, 
      0,   0, 228, 128,   1,   8, 
    228, 160,   4,   0,   0,   4, 
      1,   0,   6, 128,   2,   0, 
    244, 128,   0,   0, 170, 160, 
      2,   0, 224, 128,   4,   0, 
      0,   4,   1,   0,   6, 128, 
      1,   0, 228, 128,   0,   0, 
    255, 161,   0,   0,   0, 161, 
      5,   0,   0,   3,   2,   0, 
      3, 128,   1,   0, 233, 128, 
     24,   0, 228, 160,  90,   0, 
      0,   4,   1,   0,   8, 128, 
      2,   0, 228, 128,   2,   0, 
    228, 128,   0,   0,  85, 160, 
      7,   0,   0,   2,   1,   0, 
      8, 128,   1,   0, 255, 128, 
      6,   0,   0,   2,   1,   0, 
      8, 128,   1,   0, 255, 128, 
     19,   0,   0,   2,   2,   0, 
      1, 128,   1,   0, 255, 129, 
      2,   0,   0,   3,   1,   0, 
      8, 128,   1,   0, 255, 128, 
      2,   0,   0, 128,  10,   0, 
      0,   3,   2,   0,   1, 128, 
      1,   0, 255, 128,   1,   0, 
      0, 160,   6,   0,   0,   2, 
      1,   0,   8, 128,   2,   0, 
      0, 128,   5,   0,   0,   3, 
      2,   0,   6, 128,   1,   0, 
    255, 128,   1,   0, 228, 128, 
     95,   0,   0,   3,   0,   0, 
     15, 128,   0,   0, 228, 128, 
      0,   8, 228, 160,   1,   0, 
      0,   2,   3,   0,  12, 128, 
      0,   0,  85, 160,   1,   0, 
      0,   2,   4,   0,   7, 128, 
      0,   0, 228, 128,   1,   0, 
      0,   2,   4,   0,   8, 128, 
      0,   0,   0, 160,   1,   0, 
      0,   2,   0,   0,   8, 128, 
      0,   0,   0, 160,  38,   0, 
      0,   1,   0,   0, 228, 240, 
      1,   0,   0,   2,   1,   0, 
      8, 128,   2,   0,   0, 128, 
     45,   0,   4,   2,   1,   0, 
    255, 128,   0,   0, 255, 128, 
      4,   0,   0,   4,   3,   0, 
      3, 128,   2,   0, 233, 128, 
      0,   0, 255, 128,   0,   0, 
    228, 144,  95,   0,   0,   3, 
      5,   0,  15, 128,   3,   0, 
    244, 128,   0,   8, 228, 160, 
     95,   0,   0,   3,   6,   0, 
     15, 128,   3,   0, 228, 128, 
      2,   8, 228, 160,   2,   0, 
      0,   3,   1,   0,   8, 128, 
      1,   0,   0, 128,   6,   0, 
      0, 129,   2,   0,   0,   3, 
      1,   0,   8, 128,   1,   0, 
    255, 128,   1,   0,  85, 160, 
      2,   0,   0,   3,   5,   0, 
      7, 128,   4,   0, 228, 128, 
      5,   0, 228, 128,   2,   0, 
      0,   3,   5,   0,   8, 128, 
      4,   0, 255, 128,   0,   0, 
      0, 160,  88,   0,   0,   4, 
      4,   0,  15, 128,   1,   0, 
    255, 128,   4,   0, 228, 128, 
      5,   0, 228, 128,   2,   0, 
      0,   3,   0,   0,   8, 128, 
      0,   0, 255, 128,   0,   0, 
      0, 160,  39,   0,   0,   0, 
      6,   0,   0,   2,   0,   0, 
      1, 128,   4,   0, 255, 128, 
      5,   0,   0,   3,   0,   8, 
      7, 128,   0,   0,   0, 128, 
      4,   0, 228, 128,   1,   0, 
      0,   2,   0,   0,   2, 128, 
      0,   0,  85, 160,  90,   0, 
      0,   4,   0,   0,   1, 128, 
      1,   0, 233, 139,  24,   0, 
    228, 160,   0,   0,  85, 128, 
      5,   0,   0,   3,   0,   0, 
     17, 128,   0,   0,   0, 128, 
      1,   0, 170, 160,   5,   0, 
      0,   3,   0,   0,   1, 128, 
      0,   0,   0, 128, 150,   0, 
      0, 160,   2,   0,   0,   3, 
      0,   0,  18, 128,   4,   0, 
    255, 128,   0,   0,   0, 161, 
      5,   0,   0,   3,   0,   8, 
      8, 128,   0,   0,  85, 128, 
      0,   0,   0, 128, 255, 255, 
      0,   0
};
