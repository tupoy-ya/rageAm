/*
		THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
					http://dev-c.com
				(C) Alexander Blade 2015
*/

#pragma once

#include <windows.h>

constexpr int NOW_PERIOD = 100, MAX_DOWN = 5000; // ms
constexpr int KEYS_SIZE = 255;

inline struct {
	DWORD time;
	BOOL isWithAlt;
	BOOL wasDownBefore;
	BOOL isUpNow;
} keyStates[KEYS_SIZE];

// parameters are the same as with aru's ScriptHook for IV
inline void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	if (key < KEYS_SIZE)
	{
		keyStates[key].time = GetTickCount();
		keyStates[key].isWithAlt = isWithAlt;
		keyStates[key].wasDownBefore = wasDownBefore;
		keyStates[key].isUpNow = isUpNow;
	}
}

inline bool IsKeyDown(DWORD key)
{
	return (key < KEYS_SIZE) ? ((GetTickCount() < keyStates[key].time + MAX_DOWN) && !keyStates[key].isUpNow) : false;

}

inline void ResetKeyState(DWORD key)
{
	if (key < KEYS_SIZE)
		memset(&keyStates[key], 0, sizeof(keyStates[0]));
}

inline bool IsKeyJustUp(DWORD key, bool exclusive = true)
{
	bool b = (key < KEYS_SIZE) ? (GetTickCount() < keyStates[key].time + NOW_PERIOD && keyStates[key].isUpNow) : false;
	if (b && exclusive)
		ResetKeyState(key);
	return b;
}
