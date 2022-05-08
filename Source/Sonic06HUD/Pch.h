#pragma once

#define WIN32_LEAN_AND_MEAN

#include <BlueBlur.h>

// Detours
#include <Windows.h>
#include <detours.h>

// std
#include <stdint.h>
#include <array>
#include <set>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <deque>

// ImGui
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <DDSTextureLoader9.h>

// LostCodeLoader
#include <LostCodeLoader.h>

// Other
#include <INIReader.h>
#include <Helpers.h>

#define DEBUG_DRAW_TEXT_DLL_IMPORT
#include <DebugDrawText.h>

#include <Common.h>
#include <ScoreGenerationsAPI.h>
#include <Sonic06DefinitiveExperienceAPI.h>
