#pragma once

namespace gm
{
	/**
	 * \brief Allow to cast any type to any time. Extreme hack.
	 * \tparam TIn Type to cast from.
	 * \tparam TOut Type to cast to.
	 * \param in Value to cast.
	 * \return Casted value.
	 */
	template<typename TIn, typename TOut>
	TOut UnionCast(TIn in)
	{
		union cast
		{
			TIn im;
			TOut out;
		};
		cast c{ in };
		return c.out;
	}

	/**
	 * \brief Casts any time to LPVOID. Used to get address of member function.
	 * \tparam TIn Type to cast from.
	 * \param in Value to cast.
	 * \return Value casted to LPVOID.
	 */
	template<typename TIn>
	LPVOID CastLPVOID(TIn in)
	{
		return UnionCast<TIn, LPVOID>(in);
	}
}
