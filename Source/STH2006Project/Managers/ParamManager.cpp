#include "ParamManager.h"

std::map< ParamValue**, const char*> ParamManager::m_paramList;

void ParamManager::addParam(ParamValue** data, const char* name)
{
	m_paramList[data] = name;
}

void __fastcall ParamManager::getParam(ParamValue* ptr)
{
	for (auto& iter : m_paramList)
	{
		if (strcmp(ptr->m_name, iter.second) == 0)
		{
			*iter.first = ptr;
			break;
		}
	}
}

void ParamManager::restoreParam()
{
	for (auto& iter : m_paramList)
	{
		if (*iter.first)
		{
			(*iter.first)->restore();
		}
	}
}

HOOK(int, __fastcall, ParamManager_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	ParamManager::restoreParam();
	return originalParamManager_MsgRestartStage(This, Edx, message);
}

void __declspec(naked) ParamManager_GetFloat()
{
	static uint32_t sub_660190 = 0x660190;
	static uint32_t returnAddress = 0x59089E;
	__asm
	{
		call	[sub_660190]

		mov		ecx, edi
		call	ParamManager::getParam

		jmp		[returnAddress]
	}
}

void ParamManager::applyPatches()
{
	WRITE_JUMP(0x590899, ParamManager_GetFloat);
	INSTALL_HOOK(ParamManager_MsgRestartStage);
}
