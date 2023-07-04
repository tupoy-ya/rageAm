//
// File: element.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include <cstdarg>

#include "tinyxml2.h"
#include "common/types.h"
#include "helpers/macro.h"
#include "am/system/asserts.h"
#include "am/system/enum.h"
#include "am/system/nullable.h"
#include "am/xml/xmlutils.h"
#include "rage/atl/string.h"
#include "rage/math/vector3.h"

namespace rageam::xml
{
#define XML_ASSERT(expr) AM_ASSERT((expr) == tinyxml2::XML_SUCCESS, #expr);
#define XML_VERIFY(expr) AM_VERIFY((expr) == tinyxml2::XML_SUCCESS, #expr);
#define XML_OK(expr) (expr) == tinyxml2::XML_SUCCESS

	// NOTE: Following parser macro-helpers are made to use inside a function that returns bool!

	// Attempts to get value
#define XML_GET(element, out)													\
	if(!(element).TryGetValue<decltype(out)>((out)))							\
	{																			\
		AM_ERRF("Failed to get XML value");										\
		(element).PrintLastError();												\
		return false;															\
	}																			\
	MACRO_END

	// Attempts to get child value
#define XML_CHILD_GET(element, out, name)										\
	if(!(element).TryGetChildValue<decltype(out)>((name), (out)))				\
	{																			\
		AM_ERRF("Failed to get child XML value for %s", (name));				\
		(element).PrintLastError();												\
		return false;															\
	}																			\
	MACRO_END

	// Attempts to get child enum value
#define XML_CHILD_GET_ENUM(element, out, name)									\
	if(!(element).TryGetChildEnum<decltype(out)>((name), (out)))				\
	{																			\
		AM_ERRF("Failed to get child XML enum value for %s", (name));			\
		(element).PrintLastError();												\
		return false;															\
	}																			\
	MACRO_END

	// Attempts to get attribute
#define XML_GET_ATTR(element, out, name)										\
	if(!(element).TryGetAttribute<decltype(out)>((name), (out)))				\
	{																			\
		AM_ERRF("Failed to get XML element %s", (name));						\
		(element).PrintLastError();												\
		return false;															\
	}																			\
	MACRO_END

	// W/ auto get name

#define XML_CHILD_GET_A(element, out) XML_CHILD_GET(element, out, GET_FIELD_NAME(out))
#define XML_CHILD_GET_ENUM_A(element, out) XML_CHILD_GET_ENUM(element, out, GET_FIELD_NAME(out))
#define XML_GET_ATTR_A(element, out) XML_GET_ATTR(element, out, GET_FIELD_NAME(out))

#define XML_INSERT_CHILD_A(element, in) (element).Insert(GET_FIELD_NAME(in), (in));
#define XML_INSERT_CHILD_ENUM_A(element, in) (element).Insert(GET_FIELD_NAME(in), rageam::Enum::GetName((in)));

	class Element;
	class ElementChildIterator;

	// Nullable Element
	using nElement = Nullable<Element>;

	/**
	 * \brief XML Element represents named tag inside document.
	 * \n For example <Element>picture.png</Element>
	 */
	class Element
	{
		static constexpr u32 TEMP_BUFFER_SIZE = 256;

		using TErrorHandler = void(ConstString error);

		static inline rage::atString sm_LastError = nullptr;

		tinyxml2::XMLElement* m_Element;

		PRINTF_ATTR(2, 3) ConstString FormatTemp(ConstString fmt, ...) const;

		template<typename TValue>
		ConstString FormatValue(TValue value);
	public:
		Element(tinyxml2::XMLElement* element)
		{
			m_Element = element;
		}

		Element(const Element& other)
		{
			m_Element = other.m_Element;
		}

		ConstString GetLastError() const { return sm_LastError; }

		void PrintLastError() const
		{
			AM_ERRF("Last Error: %s, Line: %u", GetLastError(), m_Element->GetLineNum());
		}

		/**
		 * \brief Gets text of current tag element.
		 * \n <Name/>
		 */
		ConstString GetName() const { return m_Element->Name(); }

		/**
		 * \brief Sets text of current tag element.
		 * \n <Name/>
		 */
		void SetName(ConstString newName) const { m_Element->SetName(newName); }

		/**
		 * \brief Inserts new child with given name in the end.
		 * \n <Name/>
		 */
		Element Insert(ConstString name) const
		{
			return { m_Element->InsertNewChildElement(name) };
		}

		/**
		 * \brief Inserts new child with given name and value in the end.
		 * \n <Name>Value</Name>
		 */
		template<typename TValue>
		Element Insert(ConstString name, TValue value)
		{
			Element result = Insert(name);
			result.SetValue(value);
			return result;
		}

		/**
		 * \brief Inserts Vector3 with given name in the end.
		 * \n <Name X="0.0" Y="0.0" Z="0.0"/>
		 */
		void InsertVector3(ConstString name, const rage::Vector3& vec) const
		{
			Element result = Insert(name);
			result.AddAttribute("X", vec.X);
			result.AddAttribute("Y", vec.Y);
			result.AddAttribute("Z", vec.Z);
		}

		/**
		 * \brief Retrieves Vector3 from this element.
		 * \n Element must be in format of <Name X="0.0" Y="0.0" Z="0.0"/>
		 */
		rage::Vector3 GetVector3() const
		{
			float x = GetAttribute<float>("X");
			float y = GetAttribute<float>("Y");
			float z = GetAttribute<float>("Z");
			return { x, y, z };
		}

		/**
		 * \brief Gets first child with given name, there must be at least one child.
		 * \remarks Passing null as name will return any child.
		 */
		nElement GetChild(ConstString name) const
		{
			tinyxml2::XMLElement* element = m_Element->FirstChildElement(name);
			if (!element)
				return nElement::Null();
			return nElement(element);
		}

		/**
		 * \brief Gets first child text with given name, there must be at least one child.
		 * \remarks Passing null as name will return any child.
		 */
		template<typename TValue = ConstString>
		TValue GetChildValue(ConstString name) const
		{
			return GetChild(name)->GetValue<TValue>();
		}

		/**
		 * \brief Gets first child text with given name, there must be at least one child.
		 * \remarks Passing null as name will return any child.
		 */
		template<typename TEnum>
		TEnum GetChildEnum(ConstString name) const
		{
			return GetChild(name).GetValue().GetEnum<TEnum>();
		}

		/**
		 * \brief Gets first child text with given name.
		 * \remarks Passing null as name will return any child.
		 */
		template<typename TValue = ConstString>
		bool TryGetChildValue(ConstString name, TValue& value) const
		{
			nElement nChild = GetChild(name);
			if (!nChild.HasValue())
			{
				sm_LastError = FormatTemp("Element %s has no inner child %s", GetName(), name);
				return false;
			}

			Element child = nChild.GetValue();
			return child.TryGetValue<TValue>(value);
		}

		/**
		 * \brief Gets first child enum with given name.
		 * \remarks Passing null as name will return any child.
		 */
		template<typename TEnum>
		bool TryGetChildEnum(ConstString name, TEnum& value) const
		{
			nElement nChild = GetChild(name);
			if (!nChild.HasValue())
			{
				sm_LastError = FormatTemp("Element %s has no inner child %s", GetName(), name);
				return false;
			}

			Element child = nChild.GetValue();
			if (!child.TryGetEnum<TEnum>(value))
			{
				sm_LastError = FormatTemp(
					"Element %s has incorrect enum value %s in enum %s", name, child.GetValue(), Enum::GetName<TEnum>());
				return false;
			}
			return true;
		}

		/**
		 * \brief Get next sibling element, additionally name can be specified.
		 * \n If there's two such elements:
		 * \n <Item Name="Adder"/>
		 * \n <Item Name="Deluxo"/>
		 * \n And current element is Adder, next sibling element is Deluxo.
		 * \remarks If there's no sibling element, assert is triggered. Use HasNext();
		 */
		nElement GetNext(ConstString name = nullptr) const
		{
			tinyxml2::XMLElement* element = m_Element->NextSiblingElement(name);
			if (!element)
				return nElement::Null();
			return nElement(element);
		}

		/**
		 * \brief Sets tag inner value.
		 * \n <Element>Text Will Go Here</Element>
		 */
		template<typename TValue>
		void SetValue(TValue value)
		{
			m_Element->SetText(FormatValue(value));
		}

		/**
		 * \brief Gets tag inner value.
		 * \n <Element>Text From Here</Element>
		 */
		template<typename TValue = ConstString>
		TValue GetValue() const;

		/**
		 * \brief Tries to get value and returns whether attempt was successful.
		 */
		template<typename TValue = ConstString>
		bool TryGetValue(TValue& value) const;

		/**
		 * \brief Parses enum value from tag text.
		 * \remarks If not successful, assertion is thrown.
		 */
		template<typename TEnum>
		TEnum GetEnum()
		{
			return Enum::Parse<TEnum>(GetValue<ConstString>());
		}

		/**
		 * \brief Tries to get enum value and returns whether attempt was successful.
		 */
		template<typename TEnum>
		bool TryGetEnum(TEnum& value)
		{
			return Enum::TryParse<TEnum>(GetValue<ConstString>(), value);
		}

		/**
		 * \brief Adds attribute inside this element tag.
		 * \n <Element Name="Value"/>, where Name and Value correspond to function parameters.
		 */
		template<typename TValue>
		void AddAttribute(ConstString name, TValue value)
		{
			m_Element->SetAttribute(name, FormatValue(value));
		}

		/**
		 * \brief Attempts to get attribute value by name.
		 */
		template<typename TValue = ConstString>
		bool TryGetAttribute(ConstString name, TValue& outValue) const
		{
			return m_Element->QueryAttribute(name, &outValue) == tinyxml2::XML_SUCCESS;
		}

		/**
		 * \brief Gets attribute value by name, attribute must exist or assert will be triggered.
		 */
		template<typename TValue = ConstString>
		TValue GetAttribute(ConstString name) const
		{
			TValue value;
			AM_ASSERT(TryGetAttribute(name, value),
				"XmlElement::GetAttribute() -> Failed to get attribute with name %s, %s", name, m_Element->GetDocument()->ErrorStr());
			return value;
		}

		Element& operator=(const Element& other)
		{
			m_Element = other.m_Element;
			return *this;
		}

		bool operator==(const Element& other) const
		{
			return m_Element == other.m_Element;
		}

		ElementChildIterator begin() const;
		ElementChildIterator end() const;
	};

	class ElementChildIterator
	{
		nElement m_Element;
		ConstString m_ChildName;

		ElementChildIterator() // End constructor
		{
			m_Element = nElement::Null();
			m_ChildName = nullptr;
		}
	public:
		ElementChildIterator(const Element& element, ConstString childName = nullptr)
		{
			m_ChildName = childName;
			m_Element = element.GetChild(childName);
		}

		ElementChildIterator(const ElementChildIterator& other)
		{
			m_Element = other.m_Element;
			m_ChildName = other.m_ChildName;
		}

		static ElementChildIterator End() { return {}; }

		Element& Current() { return m_Element.GetValue(); }

		bool Next()
		{
			if (!m_Element.HasValue())
				return false;

			m_Element = m_Element.GetValue().GetNext(m_ChildName);
			return m_Element.HasValue();
		}

		Element& operator*() { return Current(); }

		ElementChildIterator operator++()
		{
			if (!Next())
				return End();
			return *this;
		}

		bool operator==(const ElementChildIterator& other) const
		{
			return m_Element == other.m_Element;
		}
	};

	// Type-specific implementations

	template<>
	inline ConstString Element::FormatValue(ConstString value) { return value; }
	template<>
	inline ConstString Element::FormatValue(u32 value) { return FormatTemp("%u", value); }
	template<>
	inline ConstString Element::FormatValue(s32 value) { return FormatTemp("%i", value); }
	template<>
	inline ConstString Element::FormatValue(u64 value) { return FormatTemp("%llu", value); }
	template<>
	inline ConstString Element::FormatValue(s64 value) { return FormatTemp("%lli", value); }
	template<>
	inline ConstString Element::FormatValue(float value) { return FormatTemp("%f", value); } // NOLINT(clang-diagnostic-double-promotion)
	template<>
	inline ConstString Element::FormatValue(bool value) { return value ? "True" : "False"; }

	template <>
	inline ConstString Element::GetValue() const { return m_Element->GetText(); }
	template <>
	inline u32 Element::GetValue() const { u32 value; XML_ASSERT(m_Element->QueryUnsignedText(&value)); return value; }
	template <>
	inline s32 Element::GetValue() const { s32 value; XML_ASSERT(m_Element->QueryIntText(&value)); return value; }
	template <>
	inline u64 Element::GetValue() const { u64 value; XML_ASSERT(m_Element->QueryUnsigned64Text(&value)); return value; }
	template <>
	inline s64 Element::GetValue() const { s64 value; XML_ASSERT(m_Element->QueryInt64Text(&value)); return value; }
	template <>
	inline float Element::GetValue() const { float value; XML_ASSERT(m_Element->QueryFloatText(&value)); return value; }
	template <>
	inline bool Element::GetValue() const { bool value; XML_ASSERT(m_Element->QueryBoolText(&value)); return value; }

	template <>
	inline bool Element::TryGetValue(ConstString&) const { return m_Element->GetText(); }
	template <>
	inline bool Element::TryGetValue(u32& value) const { return XML_OK(m_Element->QueryUnsignedText(&value)); }
	template <>
	inline bool Element::TryGetValue(s32& value) const { return XML_OK(m_Element->QueryIntText(&value)); }
	template <>
	inline bool Element::TryGetValue(u64& value) const { return XML_OK(m_Element->QueryUnsigned64Text(&value)); }
	template <>
	inline bool Element::TryGetValue(s64& value) const { return XML_OK(m_Element->QueryInt64Text(&value)); }
	template <>
	inline bool Element::TryGetValue(float& value) const { return XML_OK(m_Element->QueryFloatText(&value)); }
	template <>
	inline bool Element::TryGetValue(bool& value) const { return XML_OK(m_Element->QueryBoolText(&value)); }
}
