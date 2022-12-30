#pragma once
#include "gmAddress.h"
#include "gmHelper.h"

#include <functional>

namespace gm
{
	/*
	 * What to use:
	 * - I need to call function and I know it's address: gmFunc (useful for testing)
	 * - I need to call function, but I have only pattern: gmFuncScan
	 * - I want to hook game function with detour: gmFuncHook
	 * - I want to completely replace existing game function with one I want: gmFuncSwap
	 */

	 /**
	  * \brief Represents a template wrapper for function address.
	  * \n Purpose is to simplify declaration of game functions.
	  *	\n Uses __fastcall calling convention.
	  * \n __fastcall is a function call convention that requires first 4 arguments to be put in
	  * RCX, RDX, R8, R9 registers, and remaining on top of the stack.
	  * \tparam TReturn Type of return value.
	  * \tparam Args Function arguments.
	  */
	template<typename TReturn, typename... Args>
	class gmFunc : public gmAddress
	{
	public:
		using gmAddress::gmAddress;

		virtual TReturn operator()(Args... args)
		{
			return reinterpret_cast<TReturn(__fastcall*)(Args...)>(m_Address)(args...);
		}
	};

	/**
	 * \brief Represents a template wrapper for function hook by pattern.
	 * \n Uses __fastcall convention, see gmFuncFastcall.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFuncScan : public gmFunc<TReturn, Args...>
	{
	public:
		gmFuncScan(const char* pattern) : gmFunc<TReturn, Args...>::gmFunc(Scan("", pattern)) {}
		gmFuncScan(const char* name, const char* pattern) : gmFunc<TReturn, Args...>::gmFunc(Scan(name, pattern)) {}
		gmFuncScan(std::function<uintptr_t()> const& onScan) : gmFunc<TReturn, Args...>::gmFunc(onScan()) {}
	};

	/**
	 * \brief Represents a template wrapper for function hook by pattern.
	 * \n After hook being installed, instead of game function 'Detour' will be called.
	 */
	class gmFuncHook
	{
	public:
		// gmFuncBase wrappers

		template<typename Detour>
		gmFuncHook(const char* name, const char* pattern, Detour detour, gmAddress* original)
		{
			ScanAndHook(name, pattern, detour, (LPVOID*)original->GetAddressPtr());
		}

		// Template wrappers

		template<typename Detour, typename Original>
		gmFuncHook(const char* pattern, Detour detour, Original original)
		{
			ScanAndHook("", pattern, detour, original);
		}

		template<typename Detour, typename Original>
		gmFuncHook(const char* name, const char* pattern, Detour detour, Original original)
		{
			ScanAndHook(name, pattern, detour, original);
		}

		template<typename Detour, typename Original>
		gmFuncHook(std::function<uintptr_t()> const& onScan, Detour detour, Original original)
		{
			g_Hook.SetHook(onScan(), detour, original);
		}

		template<typename Detour, typename Original>
		gmFuncHook(LPVOID address, Detour detour, Original original)
		{
			g_Hook.SetHook(address, detour, original);
		}
	};

	/**
	 * \brief Represents a template wrapper for function swap by pattern.
	 * \n Swap disables game function, calling Detour instead.
	 */
	class gmFuncSwap
	{
	public:
		template<typename Detour>
		gmFuncSwap(const char* pattern, Detour detour)
		{
			ScanAndHook("", pattern, detour);
		}

		template<typename Detour>
		gmFuncSwap(const char* name, const char* pattern, Detour detour)
		{
			ScanAndHook(name, pattern, detour);
		}

		template<typename Detour>
		gmFuncSwap(std::function<uintptr_t()> const& onScan, Detour detour)
		{
			g_Hook.SetHook(onScan(), detour);
		}
	};
}
