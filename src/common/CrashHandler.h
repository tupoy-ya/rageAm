#pragma once

#include <strstream>
#include "backward.hpp"
#include <cstdio>
#include <tlHelp32.h>
#include "commctrl.h"
#include "imgui_rage/ImGuiConfigRage.h"

class CrashHandler
{
#define AM_TRACE_DEFAULT_PATH "rageAm/Trace.txt"
#define AM_TRACE_DEFAULT_PATH_W L"rageAm/Trace.txt"

	static inline std::unique_ptr<backward::SignalHandling> g_SignalHandler;
	static inline bool m_IgnoreNextSehException = false; // For ImGui

	static HWND GetGtaWindow(DWORD dwProcessId)
	{
		HWND hCurWnd = nullptr;
		do
		{
			hCurWnd = FindWindowEx(nullptr, hCurWnd, nullptr, nullptr);
			DWORD checkProcessID = 0;
			GetWindowThreadProcessId(hCurWnd, &checkProcessID);
			if (checkProcessID == dwProcessId)
			{
				char name[19];
				GetWindowTextA(hCurWnd, name, 19);
				if (strcmp(name, "Grand Theft Auto V") == 0)
					return hCurWnd;
			}
		} while (hCurWnd != nullptr);
		return nullptr;
	}
public:
	NOINLINE static void StacktraceToFile(const backward::StackTrace& stack)
	{
		backward::Printer printer;
		std::stringstream ssTrace;

		printer.print(stack, ssTrace);

		std::string trace = ssTrace.str();
		std::ofstream g_ExceptionStream(AM_TRACE_DEFAULT_PATH);
		g_ExceptionStream << trace.c_str();
		g_ExceptionStream.close();
	}

	NOINLINE static void StacktraceToDialog(const backward::StackTrace& stack, bool isFatal)
	{
		backward::Printer printer;
		printer.snippet = false;
		printer.snippet_only_first = true;
		printer.exception = false;

		std::stringstream ssTrace;
		std::stringstream ssFrame;

		printer.print(stack, ssTrace);
		printer.print_main_frame(stack, ssFrame);

		DWORD exceptionCode = stack.except_code();

		std::string fileTrace = ssTrace.str();
		std::string mainFrame = ssFrame.str();
		std::string errorName = exceptionCode == -1 ? stack.except_name() : backward::GetExceptionName(exceptionCode);
		std::wstring header = std::format(L"{} {} {:X}",
			std::wstring(mainFrame.begin(), mainFrame.end()),
			std::wstring(errorName.begin(), errorName.end()),
			exceptionCode);
		std::wstring expandedText(fileTrace.begin(), fileTrace.end());
		std::wstring footerText = std::format(L"Full stack trace was saved to: {}", AM_TRACE_DEFAULT_PATH_W);

		TASKDIALOGCONFIG config = { sizeof(TASKDIALOGCONFIG) };
		config.hwndParent = GetGtaWindow(GetProcessId(GetCurrentProcess()));
		config.pszWindowTitle = L"An exception occurred in rageAm";
		config.pszContent = isFatal ? L"Fatal exception. GTAV will be closed." : L"GTAV will continue execution but rageAm will be stopped.";
		config.pszCollapsedControlText = L"Show exception stack trace";
		config.pszExpandedControlText = L"Hide exception stack trace";
		config.pszExpandedInformation = expandedText.c_str();
		config.pszMainInstruction = header.c_str();
		config.dwFlags = TDF_EXPANDED_BY_DEFAULT;
		config.dwCommonButtons = TDCBF_OK_BUTTON;
		config.pszMainIcon = MAKEINTRESOURCEW(-7); // TD_SHIELD_ERROR_ICON
		config.pszFooterIcon = TD_INFORMATION_ICON;
		config.pszFooter = footerText.c_str();
		config.cxWidth = 290;

		TaskDialogIndirect(&config, nullptr, nullptr, nullptr);
	}

	NOINLINE static void ShowErrorDialog(PCWSTR header, PCWSTR content, PCWSTR expandedContent)
	{
		TASKDIALOGCONFIG config = { sizeof(TASKDIALOGCONFIG) };
		config.hwndParent = GetGtaWindow(GetProcessId(GetCurrentProcess()));
		config.pszWindowTitle = L"An exception occurred in rageAm";
		config.pszContent = content;
		config.pszCollapsedControlText = L"Show details";
		config.pszExpandedControlText = L"Hide details";
		config.pszExpandedInformation = expandedContent;
		config.pszMainInstruction = header;
		config.dwFlags = TDF_EXPANDED_BY_DEFAULT;
		config.dwCommonButtons = TDCBF_OK_BUTTON;
		config.pszMainIcon = MAKEINTRESOURCEW(-7); // TD_SHIELD_ERROR_ICON
		config.pszFooterIcon = TD_INFORMATION_ICON;
		config.cxWidth = 300;

		TaskDialogIndirect(&config, nullptr, nullptr, nullptr);
	}

	NOINLINE static int HandleSEH(const _EXCEPTION_POINTERS* ex, int skip = 0, bool isFatal = false)
	{
		if (m_IgnoreNextSehException)
		{
			m_IgnoreNextSehException = false;
			return EXCEPTION_EXECUTE_HANDLER;
		}

		if (ex->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
			return backward::SignalHandling::HandleDebugBreak(ex, 1);

		PCONTEXT exContext = ex->ContextRecord;
		DWORD exCode = ex->ExceptionRecord->ExceptionCode;
		PVOID exAddr = ex->ExceptionRecord->ExceptionAddress;

		backward::StackTrace stack;
		stack.load(7 + skip, exContext, exAddr, exCode);

		StacktraceToFile(stack);
		StacktraceToDialog(stack, isFatal);

		return EXCEPTION_EXECUTE_HANDLER;
	}

	// Don't call it! It's reserved for ImGui assert failed override.
	NOINLINE static void ShowImGuiError(const char* msg)
	{
		char buffer[256];
		sprintf_s(buffer, 256, "ImGui Assert Failed: %s", msg);

		backward::StackTrace stack;
		stack.load(4, nullptr, nullptr);
		stack.set_error_name(buffer);

		StacktraceToFile(stack);
		StacktraceToDialog(stack, false);

		// See explanation in ImGuiConfigRage.h
		m_IgnoreNextSehException = true;
	}

	CrashHandler()
	{
#ifdef USE_UNHANDLED_CRASH_HANDLER
		AM_TRACE("CrashHandler()");

		CreateDefaultFolders();

		g_SignalHandler = std::make_unique<backward::SignalHandling>();
#endif
	}

	~CrashHandler()
	{
#ifdef USE_UNHANDLED_CRASH_HANDLER
		AM_TRACE("~CrashHandler()");
		g_SignalHandler.reset();
#endif
	}

	template<typename TFunc, typename ... Args>
	static bool ExecuteSafe(TFunc action, Args ... args)
	{
		__try { action(args...); }
		__except (HandleSEH(GetExceptionInformation())) { return false; }
		return true;
	}
};

inline void backward::SignalHandling::HandleStacktrace(
	int skip, const _EXCEPTION_POINTERS* exInfo, const char* exName)
{
	if (exInfo)
	{
		CrashHandler::HandleSEH(exInfo, skip, true);
		return;
	}

	StackTrace stack;

	stack.load(skip, nullptr, nullptr);
	stack.set_error_name(exName);

	CrashHandler::StacktraceToFile(stack);
	CrashHandler::StacktraceToDialog(stack, true);
}

void ImGuiCrashHandler::ImGuiHandleAssert(const char* msg)
{
	CrashHandler::ShowImGuiError(msg);
}

inline CrashHandler g_CrashHandler;
