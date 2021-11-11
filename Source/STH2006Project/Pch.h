#pragma once

#define WIN32_LEAN_AND_MEAN

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
#include <unordered_set>
#include <deque>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

#include "tinyxml2.h"

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
#include <Hedgehog.h>
#include <Sonic.h>

#define DEBUG_DRAW_TEXT_DLL_IMPORT
#include <DebugDrawText.h>

#include <Common.h>
#include <ScoreGenerationsAPI.h>
