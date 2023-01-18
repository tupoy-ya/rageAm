#pragma once
#include "gmAddress.h"
#include "gmHelper.h"
#include "gmScannable.h"
#include "unionCast.h"

namespace gm
{
	/**
	 * \brief Represents a template wrapper for function address.
	 * \n Purpose is to simplify declaration of game functions.
	 *	\n Uses __fastcall calling convention.
	 * \n __fastcall is a function call convention that requires first 4 arguments to be put in
	 * RCX, RDX, R8, R9 registers, and remaining on top of the stack.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template <typename TReturn, typename... Args>
	class gmFunc : public gmAddress
	{
	public:
		using gmAddress::gmAddress;

		gmFunc(const char* name, const char* pattern) : gmAddress(Scan(name, pattern)) {}
		gmFunc(gmAddress onScan()) : gmAddress(onScan()) { }

		virtual TReturn operator()(Args ... args)
		{
			return reinterpret_cast<TReturn(__fastcall*)(Args ...)>(Addr)(args...);
		}
	};

	/**
	 * \brief Represents a template wrapper for function hook by pattern.
	 * \n Address - address of function that needs to be 'replaced'.
	 * \n Detour - function that will be called instead of original.
	 * \n Original - pointer to function that will be set to address where 'original' function is now located;
	 * To be called from Detour.
	 */
	class gmFuncHook : gmScannable
	{
	public:
		template<typename TOriginal>
		gmFuncHook(const gmAddress& addr, TOriginal detour, gmAddress* orig = nullptr)
		{
			LPVOID* ppOriginal = orig ? (LPVOID*)orig->GetAddressPtr() : nullptr;
			g_Hook.SetHook(addr, gm::CastAny(detour), ppOriginal);
		}

		template<typename TOriginal>
		gmFuncHook(const char* name, const char* pattern, TOriginal detour, gmAddress* orig = nullptr)
			: gmFuncHook(Scan(name, pattern), detour, orig) {}
	};
}
