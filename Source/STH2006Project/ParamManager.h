/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: 
/*----------------------------------------------------------*/

#pragma once

struct RefCountObject
{
	void* vtable;
	size_t refCount;
};

struct ParamBase : RefCountObject
{
	static const size_t VTABLE_BOOL = 0x16E58E4;
	static const size_t VTABLE_FLOAT = 0x16E5CDC;
	static const size_t VTABLE_INT = 0x16E5E14;
	static const size_t VTABLE_LONG = 0x16E5DAC;
	static const size_t VTABLE_TYPE_LIST = 0x16E5C74;
	static const size_t VTABLE_ULONG = 0x16E5D44;

	int8_t field8;
	const char* m_name;
	int8_t field10;
};

struct ParamValueFuncData : RefCountObject
{
	float* m_pValue;
	float m_value;
	int32_t field10;
	int32_t field14;
	void (*m_updater)();
	int32_t field1C;
	void* field20;
	int32_t field24;
	void* field28;
	void* field2C;
	const char* m_description;
	int32_t field34;

	void update()
	{
		if (!field10) return;
		((int(**)(void*, int32_t))(field10 & ~1))[1](&m_updater, true);
	}
};

struct ParamValue : ParamBase
{
	ParamValueFuncData* m_funcData;
	float m_defaultValue;

	void restore()
	{
		*m_funcData->m_pValue = m_defaultValue;
		m_funcData->update();
	}
};

class ParamManager
{
private:
	static std::map< ParamValue**, const char*> m_paramList;

public:
	static void addParam(ParamValue** data, const char* name);
	static void __fastcall getParam(ParamValue* ptr);
	static void restoreParam();
	static void applyPatches();
};

