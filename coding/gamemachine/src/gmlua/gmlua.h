﻿#ifndef __GMLUA_H__
#define __GMLUA_H__
#include "common.h"
BEGIN_NS

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

enum class GMLuaStates
{
	OK = LUA_OK,
	ERRRUN = LUA_ERRRUN,
	ERRMEM = LUA_ERRMEM,
	ERRERR = LUA_ERRERR,
	ERRGCMM = LUA_ERRGCMM,
};

GM_INTERFACE(GMLuaExceptionHandler)
{
	virtual void onException(GMLuaStates state, const char* msg) = 0;
};

GM_PRIVATE_OBJECT(GMLua)
{
	lua_State* luaState = nullptr;
	GMLuaExceptionHandler* exceptionHandler = nullptr;
	GMuint* ref = nullptr;
};

enum class GMLuaVariableType
{
	Number,
	String,
	Boolean,
};

struct GMLuaVariable
{
	GMLuaVariableType type;
	GMString valString;
	union
	{
		double valDouble;
		bool valBoolean;
	};

	GMLuaVariable()
		: type(GMLuaVariableType::Number)
		, valDouble(0)
	{
	}

	GMLuaVariable(double d)
		: type(GMLuaVariableType::Number)
		, valDouble(d)
	{
	}

	GMLuaVariable(GMint d)
		: type(GMLuaVariableType::Number)
		, valDouble(d)
	{
	}

	GMLuaVariable(GMfloat d)
		: type(GMLuaVariableType::Number)
		, valDouble(d)
	{
	}

	GMLuaVariable(bool b)
		: type(GMLuaVariableType::Boolean)
		, valBoolean(b)
	{
	}

	GMLuaVariable(GMString& str)
		: type(GMLuaVariableType::String)
		, valString(str)
	{
	}
};

class GMLua : public GMObject
{
	DECLARE_PRIVATE(GMLua)

public:
	GMLua();
	~GMLua();

	GMLua(const GMLua& lua);
	GMLua(GMLua&& lua) noexcept;
	GMLua& operator= (const GMLua& state);
	GMLua& operator= (GMLua&& state) noexcept;

public:
	GMLuaStates loadFile(const char* file);
	GMLuaStates loadBuffer(const GMBuffer& buffer);

	void setGlobal(const char* name, const GMLuaVariable& var);
	void call(const char* functionName, const std::initializer_list<GMLuaVariable>& args);
	void call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet);

public:
	template <size_t _size> void call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable(&returns)[_size])
	{
		call(functionName, args);
		for (GMint i = 0; i < _size; i++)
		{
			returns[i] = pop();
		}
	}

private:
	void callExceptionHandler(GMLuaStates state, const char* msg);
	void push(const GMLuaVariable& var);
	GMLuaVariable pop();
};

END_NS
#endif