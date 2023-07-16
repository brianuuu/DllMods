#pragma once

#define WIN32_LEAN_AND_MEAN

#include <BlueBlur.h>
#include <BlueBlurCustom.h>

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
#include <filesystem>
#include <mutex>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

#include "tinyxml2.h"

// LostCodeLoader
#include <ModLoader.h>

// Other
#include <INIReader.h>
#include <Helpers.h>

#define DEBUG_DRAW_TEXT_DLL_IMPORT
#include <DebugDrawText.h>

#include <Common.h>
#include <StateManager.h>
#include <Sonic06HUDAPI.h>
