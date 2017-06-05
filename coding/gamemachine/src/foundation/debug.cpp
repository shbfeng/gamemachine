﻿#include "stdafx.h"
#include "debug.h"

Hooks& Hooks::instance()
{
	static Hooks hooks;
	return hooks;
}

void Hooks::invoke0(const std::string& identifier)
{
	auto hooks = instance().m_hooks0[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)();
	}
}

void Hooks::install(const std::string& identifier, Hook0 hook)
{
	auto& hooks = instance().m_hooks0[identifier];
	hooks.insert(hook);
}

void Hooks::invoke1(const std::string& identifier, void* arg1)
{
	auto hooks = instance().m_hooks1[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)(arg1);
	}
}

void Hooks::install(const std::string& identifier, Hook1 hook)
{
	auto& hooks = instance().m_hooks1[identifier];
	hooks.insert(hook);
}

void Hooks::invoke2(const std::string& identifier, void* arg1, void* arg2)
{
	auto hooks = instance().m_hooks2[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)(arg1, arg2);
	}
}

void Hooks::install(const std::string& identifier, Hook2 hook)
{
	auto& hooks = instance().m_hooks2[identifier];
	hooks.insert(hook);
}