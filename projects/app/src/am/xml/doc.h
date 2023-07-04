//
// File: doc.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "am/system/asserts.h"
#include "tinyxml2.h"
#include "element.h"

namespace rageam::xml
{
	/**
	 * \brief XML Document.
	 */
	class Document
	{
		tinyxml2::XMLDocument m_Doc;
	public:
		tinyxml2::XMLDocument* GetLowLevelDocument() { return &m_Doc; }

		/**
		 * \brief Declares new document.
		 * \param rootName Name of root element name.
		 */
		void CreateNew(ConstString rootName)
		{
			m_Doc.InsertFirstChild(m_Doc.NewDeclaration());

			tinyxml2::XMLElement* root = m_Doc.NewElement(rootName);
			m_Doc.InsertEndChild(root);
		}

		/**
		 * \brief Loads existing document from xml file.
		 */
		bool LoadFromFile(const file::WPath& path)
		{
			FILE* file;
			if (!AM_VERIFY(_wfopen_s(&file, path.GetCStr(), L"rb") == 0, 
				L"XmlDocument::LoadFromFile() -> Failed to open file %ls", path.GetCStr()))
				return false;
			
			tinyxml2::XMLError load = m_Doc.LoadFile(file);
			fclose(file);

			AM_VERIFY(load == tinyxml2::XML_SUCCESS, "XmlDocument::LoadFromFile() -> Failed: %s", m_Doc.ErrorStr());
			return load == tinyxml2::XML_SUCCESS;
		}

		/**
		 * \brief Writes XML document to file.
		 */
		bool SaveToFile(const file::WPath& path)
		{
			FILE* file;
			if (!AM_VERIFY(_wfopen_s(&file, path.GetCStr(), L"w") == 0,
				L"XmlDocument:SaveToFile() -> Failed to open file %ls", path.GetCStr()))
				return false;

			tinyxml2::XMLError save = m_Doc.SaveFile(file);
			fclose(file);

			AM_VERIFY(save == tinyxml2::XML_SUCCESS, "XmlDocument::LoadFromFile() -> Failed: %s", m_Doc.ErrorStr());
			return save == tinyxml2::XML_SUCCESS;
		}

		/**
		 * \brief Gets root element (tag) in this document.
		 */
		nElement GetRoot()
		{
			tinyxml2::XMLElement* element = m_Doc.FirstChildElement();
			return nElement(element);
		}
	};
}
