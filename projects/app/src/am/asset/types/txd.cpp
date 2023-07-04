#include "txd.h"

#include "am/file/iterator.h"
#include "am/graphics/texture/imageformat.h"
#include "am/graphics/texture/surface.h"
#include "am/string/string.h"
#include "am/system/enum.h"
#include "am/task/worker.h"
#include "helpers/format.h"
#include "rage/atl/set.h"

void rageam::asset::TextureOptions::GetCompressionOptions(texture::CompressionOptions& outOptions) const
{
	outOptions.MipMaps = GenerateMips;
	outOptions.MaxResolution = MaxSize;
	outOptions.Format = Format;
	outOptions.MipFilter = MipFilter;
	outOptions.ResizeFilter = ResizeFilter;
	outOptions.Quality = Quality;
}

bool rageam::asset::Texture::Deserialize(const xml::Element& xml)
{
	XML_CHILD_GET_A(xml, Options.MaxSize);
	XML_CHILD_GET_A(xml, Options.GenerateMips);
	XML_CHILD_GET_ENUM_A(xml, Options.Format);
	XML_CHILD_GET_ENUM_A(xml, Options.MipFilter);
	XML_CHILD_GET_ENUM_A(xml, Options.ResizeFilter);
	XML_CHILD_GET_ENUM_A(xml, Options.Quality);

	return true;
}

bool rageam::asset::Texture::Serialize(xml::Element& xml) const
{
	XML_INSERT_CHILD_A(xml, Options.MaxSize);
	XML_INSERT_CHILD_A(xml, Options.GenerateMips);
	XML_INSERT_CHILD_ENUM_A(xml, Options.Format);
	XML_INSERT_CHILD_ENUM_A(xml, Options.MipFilter);
	XML_INSERT_CHILD_ENUM_A(xml, Options.ResizeFilter);
	XML_INSERT_CHILD_ENUM_A(xml, Options.Quality);

	return true;
}

bool rageam::asset::TxdAsset::CompileToGame(rage::pgDictionary<rage::grcTextureDX11>* ppOutGameFormat)
{
	ReportProgress(L"- Compressing textures", 0);

	u32 textureCount = m_Textures.GetNumUsedSlots();

	// Step 1: Compress all textures in parallel threads
	struct CompressJob
	{
		char* PixelData;
		texture::CompressedInfo Info;
		const Texture* Texture;
	};

	Tasks compressTasks;
	rage::atArray<CompressJob> compressJobs;

	compressTasks.Reserve(textureCount);
	compressJobs.Resize(textureCount);

	// For calculating % of completion
	std::atomic_int doneTextures = 0;

	u32 index = 0;
	for (const Texture& texture : m_Textures)
	{
		// Take pointer so it can be used in lambda
		const Texture* pTexture = &texture;

		ConstWString fmt = pTexture->IsPreCompressed ? L"[TxdAsset] Load %ls" : L"[TxdAsset] Compress %ls";

		compressTasks.Emplace(BackgroundWorker::Run(
			[this, pTexture, &doneTextures, textureCount, &compressJobs, index]
			{
				file::WPath texturePath = pTexture->GetFullPath();

				CompressJob& job = compressJobs[index];
				job.Texture = pTexture;

				texture::Surface surface;
				if (!surface.LoadToRam(texturePath, &job.PixelData))
					return false;

				job.Info = surface.GetInfo();

				// Accumulate and report progress
				if (CompileCallback)
				{
					++doneTextures;

					double progress = static_cast<double>(doneTextures) / textureCount;

					char sizeText[32];
					FormatBytes(sizeText, 32, job.Info.Size);

					CompileCallback(String::FormatTemp(L"Texture %i/%u: %hs -> %hs",
						doneTextures.load(), textureCount, pTexture->Name, sizeText), progress);
				}

				return true;
			}, fmt, pTexture->GetFileName()));
		index++;
	}

	// Step 2: Wait compression tasks to finish
	if (!BackgroundWorker::WaitFor(compressTasks))
		return false;

	// Step 3: Create grcTexture's
	rage::pgTextureDictionary& txd = *ppOutGameFormat;
	for (CompressJob& job : compressJobs)
	{
		texture::CompressedInfo& info = job.Info;

		ConstString textureName = job.Texture->Name;

		rage::grcTextureDX11* texture = txd.Construct(
			textureName, info.Width, info.Height, 1, info.MipCount, info.Format, job.PixelData);
		texture->SetName(textureName);
	}

	return true;
}

void rageam::asset::TxdAsset::Refresh()
{
	// Scan existing textures
	rage::atSet<rage::atWideString> imageFiles;
	rage::atSet<u32> imageNames; // To check if there's two textures with same name and different extension
	file::FindData entry;
	file::Iterator it(GetDirectoryPath() / L"*.*");
	while (it.Next())
	{
		it.GetCurrent(entry);

		ConstWString extension = file::GetExtension<wchar_t>(entry.Path);
		if (!texture::IsImageFormat(extension))
			continue;

		rage::atWideString fileName = file::GetFileName<wchar_t>(entry.Path);

		// Check if we already parsed image file with such name (but different extension)
		u32 nameHash = rage::joaat(entry.Path.GetFileNameWithoutExtension());
		if (imageNames.Contains(nameHash))
		{
			AM_WARNINGF(
				L"TxdAsset::Refresh() -> Found 2 image files with the same name but different extension, ignoring last occurence: %ls",
				fileName.GetCStr());
			continue;
		}

		// Verify that texture name has no non-ascii symbols because they can't be converted into const char* and user will have issues later
		bool validName = true;
		for (wchar_t c : fileName)
		{
			if (static_cast<u16>(c) <= UINT8_MAX)
				continue;

			AM_WARNINGF(
				L"TxdAsset::Refresh() -> Found non ASCII symbol in %ls (%lc)! Name cannot be converted safely, skipping!",
				fileName.GetCStr(), c);

			validName = false;
			break;
		}

		if (!validName)
			continue;

		imageNames.Insert(nameHash);
		imageFiles.Emplace(std::move(fileName));
	}

	// Find and remove image files that were deleted
	rage::atArray<u32> imagesToRemove; // Store temp array because we can't alter collection (m_Textures) we're iterating
	for (const Texture& texture : m_Textures)
	{
		u32 key = texture.GetHashKey();

		// Image is not in directory, file was removed
		if (!imageFiles.ContainsAt(key))
		{
			imagesToRemove.Add(key);
			AM_TRACEF(L"TxdAsset::Refresh() -> Image file %ls was removed.", texture.GetFileName());
		}
		// Both config and image file are present, don't do anything to it
		else
		{
			imageFiles.RemoveAt(key);
		}
	}

	// Now we can safely clean up removes images
	for (u32 key : imagesToRemove)
		m_Textures.RemoveAt(key);

	// Add new images (that didn't exist before)
	for (const rage::atWideString& imageName : imageFiles)
	{
		m_Textures.ConstructAt(joaat(imageName), this, imageName);
	}
}

bool rageam::asset::TxdAsset::Serialize(xml::Element& xml) const
{
	for (const Texture& texture : m_Textures)
	{
		xml::Element xitem = xml.Insert("Item");
		xitem.AddAttribute("Name", String::ToUtf8Temp(texture.GetFileName()));

		if (!texture.Serialize(xitem))
			return false;
	}

	return true;
}

bool rageam::asset::TxdAsset::Deserialize(const xml::Element& xml)
{
	xml::ElementChildIterator childIt(xml, "Item");
	while (childIt != xml.end())
	{
		const xml::Element& xitem = childIt.Current();

		ConstString fileName;
		XML_GET_ATTR(xitem, fileName, "Name");

		ConstWString texturePath = String::ToWideTemp(fileName);
		u32 hashKey = rage::joaat(texturePath); // AssetSource::GetHashKey() is file name hash

		Texture& texture = m_Textures.ConstructAt(hashKey, this, texturePath);
		texture.Deserialize(xitem);

		++childIt;
	}
	return true;
}
