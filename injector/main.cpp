#include <string>
#include <Windows.h>
#include <tlHelp32.h>
#include <tchar.h>
#include <psapi.h>
#include <format>
#include <ctime>

/*
 * KooroshRZ
 * https://github.com/KooroshRZ/Windows-DLL-Injector
 */

DWORD GetGtaProcessId();
HMODULE GetGtaModuleHandle(const std::string& name);

DWORD gtaPid;
HANDLE hGta;

DWORD GetGtaProcessId()
{
	PROCESSENTRY32 PE32{};
	PE32.dwSize = sizeof PE32;

	const HANDLE hSnapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS,
		0);
	DWORD PID = 0;

	BOOL hasProcess = Process32First(hSnapshot, &PE32);
	while (hasProcess)
	{
		if (!strcmp("GTA5.exe", PE32.szExeFile))
		{
			PID = PE32.th32ProcessID;
			printf("GTA5.exe PID: %lu\n", PID);
			break;
		}

		hasProcess = Process32Next(hSnapshot, &PE32);
	}
	CloseHandle(hSnapshot);

	return PID;
}

HMODULE GetGtaModuleHandle(const std::string& name)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	if (EnumProcessModules(hGta, hMods, sizeof(hMods), &cbNeeded))
	{
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.

			if (GetModuleFileNameEx(hGta, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				std::string moduleName = szModName;
				if (moduleName.find(name) != -1)
				{
					return hMods[i];
				}
			}
		}
	}
	return nullptr;
}

void InjectDll(const std::string& path)
{
	const char* pathStr = path.c_str();

	LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	if (!LoadLibAddr) {
		printf("Could note locate real address of LoadLibraryA!\n");
		printf("LastError : 0X%x\n", GetLastError());
		system("PAUSE");
		return;
	}

	printf("LoadLibraryA is located at real address: 0X%p\n", (void*)LoadLibAddr);

	LPVOID pDllPath = VirtualAllocEx(hGta, 0, strlen(pathStr), MEM_COMMIT, PAGE_READWRITE);

	if (!pDllPath) {
		printf("Could not allocate Memory in target process\n");
		printf("LastError : 0X%x\n", GetLastError());
		system("PAUSE");
		return;
	}

	printf("Dll path memory allocated at: 0X%p\n", (void*)pDllPath);

	BOOL Written = WriteProcessMemory(hGta, pDllPath, (LPVOID)pathStr, strlen(pathStr), NULL);

	if (!Written) {
		printf("Could not write into the allocated memory\n");
		printf("LastError : 0X%x\n", GetLastError());
		system("PAUSE");
		return;
	}

	printf("Dll path memory was written at address : 0x%p\n", (void*)pDllPath);

	HANDLE hThread = CreateRemoteThread(hGta, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, pDllPath, 0, NULL);

	if (!hThread) {
		printf("Could not open Thread with CreatRemoteThread API\n");
		printf("LastError : 0X%x\n", GetLastError());
		system("PAUSE");
		return;
	}

	printf("Thread started with CreateRemoteThread\n");

	CloseHandle(hThread);
}

bool EjectDll(const std::string& path)
{
	const HMODULE hModule = GetGtaModuleHandle(path);

	if (!hModule)
		return false;

	CreateRemoteThread(
		hGta, 0, 0,
		(PTHREAD_START_ROUTINE)FreeLibraryAndExitThread,
		hModule, 0, 0);

	return true;
}

void GetPid()
{
	gtaPid = GetGtaProcessId();
}

bool VerifyPid()
{
	if (gtaPid == 0)
	{
		printf("GTA5.exe is not running.\n");
		return false;
	}
	return true;
}

bool GetAndVerifyPid()
{
	GetPid();
	return VerifyPid();
}

void OpenProcess()
{
	hGta = OpenProcess(
		PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE |
		PROCESS_VM_READ,
		FALSE,
		gtaPid);
}

int main(int argc, char* argv[])
{
	printf("Rage Injector\n");

	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];

		printf("Argument [%i]: %s\n", i, argv[i]);

		if (arg == "-w")
		{
			printf("Waiting for GTA5.exe\n");
			while (gtaPid == 0)
			{
				Sleep(100);
				GetPid();
			}

			// Put micro delay to wait until process initializes
			Sleep(200);

			if (GetAndVerifyPid())
			{
				OpenProcess();
				InjectDll(argv[++i]);
			}
			break;
		}

		if (arg == "-i")
		{
			if (GetAndVerifyPid())
			{
				OpenProcess();
				InjectDll(argv[++i]);
			}
			break;
		}

		if (arg == "-e")
		{
			if (GetAndVerifyPid())
			{
				OpenProcess();
				if (EjectDll(argv[++i]))
				{
					printf("%s unloaded\n", argv[i]);
				}
				else
				{
					printf("%s was not loaded. Skipping.\n", argv[i]);
				}
			}
			break;
		}

		if (arg == "-fe") // Force Eject
		{
			if (GetAndVerifyPid())
			{
				OpenProcess();

				// Eject until module is not found in game
				// That's part of genius global (per rage am)
				// exception handler that really messes up things
				int count = 0;
				const char* name = argv[++i];
				while (EjectDll(name))
				{
					printf("Failed to unload... Retrying\n");

					count++;
					Sleep(100);
				}

				if (count == 0)
					printf("%s was not loaded.\n", argv[i]);
				else
					printf("%s was unloaded after %i attempts.\n", argv[i], count);
			}
			break;
		}

		// boost stack trace win dbg library it uses don't close .pdb file
		// handle, making it impossible to compile project
		// it still can be renamed or moved, so rename to random
		// and mark as to be removed
		if (arg == "-pdb")
		{
			srand(_time32(nullptr));

			std::string name = argv[++i];

			std::string newName = name.substr(0, name.rfind('/'));
			newName += std::format("/temp_{}.pdb", rand());

			printf("Renaming pdb file to: %s\n", name.c_str());

			if (MoveFileEx(name.c_str(), newName.c_str(), 0) == 0)
			{
				printf(".pdb renamed successfully.\n");
			}
			else
			{
				printf("Failed to rename pcb. Last error code: %lu.\n", GetLastError());
			}

			// This should delete file after reboot
			MoveFileEx(newName.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
	}

	printf("Exiting rageInjector.\n");
	CloseHandle(hGta);
}
