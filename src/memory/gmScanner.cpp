#include "gmScanner.h"
#include "pattern.h"

void gm::gmScanner::Load()
{
	std::ifstream fs(m_addrStorageName);

	if (!fs.is_open())
	{
		g_Log.LogT("gmScanner::Load() -> .bin doesn't exists");
		return;
	}
	g_Log.LogT("gmScanner::Load() -> Loading cache");

	int8_t ver;
	fs >> ver;
	if (ver != SCANNER_STORE_FORMAT_VER)
	{
		g_Log.LogT("gmScanner::Load() -> Storage format version mismatch: {} vs {}, storage will be cleaned up",
			ver, SCANNER_STORE_FORMAT_VER);
		return;
	}

	int numLoaded = 0;
	std::string name;
	uintptr_t address;
	while (fs >> address)
	{
		std::getline(fs, name, '\n');

		g_Log.LogD("gmScanner::Load() -> {}, {:X}", name, address);

		m_addrStorage.emplace(name, address);

		numLoaded++;
	}
	g_Log.LogT("gmScanner::Load() -> Loaded {} entries", numLoaded);

	fs.close();

	if (numLoaded != 0)
	{
#ifdef GM_SCANNER_USE_STORAGE
		ValidateStorage();
#endif
	}
}

void gm::gmScanner::Save() const
{
	g_Log.LogT("gmScanner::Save()");

	EnsureDataFoldersExist();

	std::ofstream fs(m_addrStorageName, std::ofstream::trunc);

	fs << SCANNER_STORE_FORMAT_VER;
	for (auto& entry : m_addrStorage)
	{
		fs << entry.second << entry.first << "\n";
	}

	fs.close();
}

void gm::gmScanner::ValidateStorage()
{
	g_Log.LogT("gmScanner::ValidateStorage()");

	// TODO: Additionally compare game version

	// We can check if addresses shifted by getting some constant,
	// one of them is "Grand Theft Auto V" passed in GameInitialize function

	// TODO: This actually gets scanned only on second run, because on first storage doesn't exists
	// This m_address will be stored so we'll know if its outdated or not
	auto result = ScanPattern("InitializeGame",
		"48 89 5C 24 08 48 89 7C 24 18 55 48 8D AC 24 B0 FE FF FF 48 81 EC 50 02 00 00 48");

	char* gtaStr = result.GetAt(0x132 + 0x3).CastRef<char*>();

	if (gtaStr != nullptr)
	{
		// Simply check first and last characters (reading whole string may be actually danger if pointer is invalid)

		if (gtaStr[0] == 'G' && gtaStr[17] == 'V')
		{
			g_Log.LogT("gmScanner::ValidateStorage() -> Storage validated successfully. Welcome to {}!", gtaStr);
			return;
		}
	}

	g_Log.LogE("gmScanner::ValidateStorage() -> Addresses are outdated, storage will be cleaned up");

	ClearStorage();
}

void gm::gmScanner::ClearStorage()
{
	m_addrStorage.clear();

	std::ofstream fs(m_addrStorageName, std::ofstream::trunc);
	fs.close();
}

gm::gmScanner::gmScanner()
{
	g_Log.LogT("gmScanner::gmScanner()");

	Load();
}

gm::gmScanner::~gmScanner()
{
	g_Log.LogT("~gmScanner()");

	Save();
}

uintptr_t gm::gmScanner::GetAddressFromStorage(const char* name) const
{
	if (!m_addrStorage.contains(name))
		return 0;

	return m_addrStorage.at(name);
}

gm::gmAddress gm::gmScanner::ScanPatternModule(const char* name, const char* module, const std::string& pattern)
{
	uintptr_t address;

	if (name[0] != '\0')
	{
#ifdef GM_SCANNER_USE_STORAGE
		address = GetAddressFromStorage(name);

		if (address != 0)
		{
			g_Log.LogT("gmScanner::ScanPattern({}): Found in storage - {:X}", name, address);
			return { address };
		}
#endif
	}

	auto startTime = std::chrono::high_resolution_clock::now();
	address = FindPattern(module, pattern);
	auto endTime = std::chrono::high_resolution_clock::now();

	if (address == 0)
	{
		g_Log.LogE("gmScanner::ScanPattern({}): 0x0", name);
		return { 0 };
	}

#ifdef _DEBUG
	double time = std::chrono::duration<double, std::milli>(endTime - startTime).count();
	g_Log.LogD("gmScanner::ScanPattern({}): {:X} took {}ms", name, address, time);
#else
	g_Log.LogT("gmScanner::ScanPattern({}): {:X}", name, address);
#endif

	if (name[0] != '\0')
		m_addrStorage.emplace(name, address);

	return { address };
}

gm::gmAddress gm::gmScanner::ScanPattern(const char* name, const std::string& pattern)
{
	return ScanPatternModule(name, nullptr, pattern);
}
