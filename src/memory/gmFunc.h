#pragma once
#include "gmAddress.h"
#include "gmHelper.h"

#include <functional>

namespace gm
{
	/**
	 * \brief Represents a template wrapper for function address.
	 * Purpose is to simplify declaration of game functions.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFunc
	{
	protected:
		uintptr_t m_Addr = 0;
	public:
		gmFunc(LPVOID addr)
		{
			m_Addr = reinterpret_cast<uintptr_t>(addr);
		}

		gmFunc(uintptr_t addr)
		{
			m_Addr = addr;
		}

		virtual ~gmFunc() {}

		virtual TReturn operator()(Args... args)
		{
			return reinterpret_cast<TReturn(*)(Args...)>(m_Addr)(args...);
		}

		gmAddress GetAddress() const
		{
			return m_Addr;
		}
	};

	/**
	 * \brief Represents a template wrapper for __fastcall function address.
	 * \n __fastcall is a function call convention that requires first 4 arguments to be put in
	 * RCX, RDX, R8, R9 registers, and remaining on top of the stack.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFuncFastcall : public gmFunc<TReturn, Args...>
	{
	public:
		using gmFunc<TReturn, Args...>::gmFunc;

		TReturn operator()(Args... args) override
		{
			uintptr_t addr = gmFunc<TReturn, Args...>::m_Addr;
			return reinterpret_cast<TReturn(__fastcall*)(Args...)>(addr)(args...);
		}
	};

	/**
	 * \brief Represents a template wrapper for function hook by pattern.
	 * \n Uses __fastcall convention, see gmFuncFastcall.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFuncScan : public gmFuncFastcall<TReturn, Args...>
	{
	public:
		gmFuncScan(std::string pattern) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(Scan("", pattern)) {}
		gmFuncScan(const char* name, std::string pattern) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(Scan(name, pattern)) {}
		gmFuncScan(std::function<uintptr_t()> const& onScan) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(onScan()) {}
	};

	/**
	 * \brief Represents a template wrapper for function swap by pattern.
	 * \n Swap disables game function, calling Detour instead.
	 * \n Uses __fastcall convention, see gmFuncFastcall.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFuncHook : public gmFuncFastcall<TReturn, Args...>
	{
	public:
		template<typename Detour, typename Original>
		gmFuncHook(std::string pattern, Detour detour, Original original) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(ScanAndHook("", pattern, detour, original)) {}
		template<typename Detour, typename Original>
		gmFuncHook(const char* name, std::string pattern, Detour detour, Original original) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(ScanAndHook(name, pattern, detour, original)) {}
		template<typename Detour, typename Original>
		gmFuncHook(std::function<uintptr_t()> const& onScan, Detour detour, Original original) : gmFuncFastcall<TReturn, Args...>::gmFuncFastcall(ScanAndHook(onScan(), detour, original)) {}
	};

	/**
	 * \brief Represents a template wrapper for function swap by pattern.
	 * \n Swap disables game function, calling Detour instead.
	 * \tparam TReturn Type of return value.
	 * \tparam Args Function arguments.
	 */
	template<typename TReturn, typename... Args>
	class gmFuncSwap
	{
	public:
		template<typename Detour>
		gmFuncSwap(std::string pattern, Detour detour)
		{
			ScanAndHook("", pattern, detour);
		}

		template<typename Detour>
		gmFuncSwap(const char* name, std::string pattern, Detour detour)
		{
			ScanAndHook(name, pattern, detour);
		}

		template<typename Detour>
		gmFuncSwap(std::function<uintptr_t()> const& onScan, Detour detour)
		{
			ScanAndHook(onScan(), detour);
		}
	};
}
