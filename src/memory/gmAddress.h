#pragma once
#include <Windows.h>

namespace gm
{
	/**
	 * \brief Provides easy way to work with game addresses.
	 * \n All methods are null pointer safe, but it's still
	 * reasonable to check address with MayBeValid() function
	 * before de-referencing.
	 */
	struct gmAddress
	{
	private:
		uintptr_t m_address;

	public:
		explicit gmAddress(uintptr_t address)
		{
			m_address = address;
		}

		uintptr_t GetAddress() const
		{
			return m_address;
		}

		/**
		 * \brief Gets whether address can be read safely.
		 * \return Boolean indicating whether address is read-safe or not.
		 */
		bool MayBeValid() const;

		/**
		 * \brief Looks up for relative call offset
		 * e.g. (mov rdx, [gta5.exe+offset]) on current address and casts it to required type.
		 * \return T instance with offset address if success, otherwise nullptr.
		 */
		template<typename T>
		T CastRef() const
		{
			if (!MayBeValid())
				return nullptr;

			return GetRef().Cast<T>();
		}

		/**
		 * \brief Gets current address with given offset and casts it to required type.
		 * \tparam T Type to cast in.
		 * \param offset Offset relative to current address, can be negative.
		 * \return T instance on address with offset added if address is valid, otherwise nullptr.
		 */
		template<typename T>
		T CastAt(int offset)
		{
			if (!MayBeValid())
				return nullptr;

			return GetAt(offset).Cast<T>();
		}

		/**
		 * \brief Casts current address to required type.
		 * \tparam T Type to cast in.
		 * \return Current address casted to T if address is valid, otherwise nullptr.
		 */
		template<typename T>
		T Cast()
		{
			if (!MayBeValid())
				return nullptr;

			return reinterpret_cast<T>(m_address);
		}

		/**
		 * \brief Looks up for relative call offset
		 * e.g. (mov [gta5.exe+offset], 0x0) on current address.
		 * \return gmAddress instance with offset address if success, otherwise
		 * gmAddress instance with nullptr address.
		 */
		gmAddress GetRef() const;

		/**
		 * \brief Gets address on given offset to current address.
		 * \param offset Offset to add to current address.
		 * \return gmAddress instance with added offset to address if address is valid,
		 * otherwise address will null.
		 */
		gmAddress GetAt(int offset) const;

		operator uintptr_t() const { return m_address; }
	};
}
