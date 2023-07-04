#include "gameasset.h"

bool rageam::asset::AssetBase::LoadConfig()
{
	static Logger logger("asset_config");
	LoggerScoped scopedLogger(logger);

	const file::WPath& configPath = GetConfigPath();

	// If config file doesn't exist - create default one
	if (!IsFileExists(configPath))
	{
		Refresh();
		return AM_VERIFY(SaveConfig(), "Unable to save just created config");
	}

	xml::Document doc;
	if (!AM_VERIFY(doc.LoadFromFile(configPath), L"Failed to load xml file %ls", configPath.GetCStr()))
		return false;

	// Document must have root element
	xml::nElement nRoot = doc.GetRoot();
	if (!AM_VERIFY(nRoot.HasValue(),
		L"Root element is missing, XML file is invalid %ls", configPath.GetCStr()))
		return false;

	// Root element name must match to asset-specific name constant
	xml::Element root = nRoot.GetValue();
	ConstWString documentRootName = String::ToWideTemp(root.GetName());
	ConstWString expectedRootName = GetXmlName();
	if (!AM_VERIFY(String::Equals(documentRootName, expectedRootName),
		L"Root element name doesn't match (file: %ls, actual: %ls), XML file is invalid %ls",
		documentRootName, expectedRootName, configPath.GetCStr()))
		return false;

	// Try to retrieve version
	u32 version;
	if (!AM_VERIFY(root.TryGetAttribute("Version", version),
		L"Format version is not specified, XML file is invalid %ls", configPath.GetCStr()))
		return false;

	// Compare file format version to current one
	if (!AM_VERIFY(version == GetFormatVersion(),
		L"Format version doesn't match (file: %u, actual: %u) %ls", version, GetFormatVersion(), configPath.GetCStr()))
		return false;

	// Try to deserialize xml to config
	if (!AM_VERIFY(Deserialize(root), L"Failed to deserialize config for %ls", GetDirectoryPath().GetCStr()))
		return false;

	// Actualize config
	Refresh();

	return true;
}

bool rageam::asset::AssetBase::SaveConfig() const
{
	const file::WPath& configPath = GetConfigPath();

	// Create root element with asset-specific name, (for txd its TextureDictionary). It will help to detect errors in xml file faster.
	xml::Document doc;
	doc.CreateNew(String::ToAnsiTemp(GetXmlName()));

	// Add asset format version
	xml::Element root = doc.GetRoot().GetValue();
	root.AddAttribute("Version", GetFormatVersion());

	// Write config to xml
	Serialize(root);

	return AM_VERIFY(doc.SaveToFile(configPath), L"GameAsset::SaveConfig() -> Failed to save config %ls", configPath.GetCStr());
}
