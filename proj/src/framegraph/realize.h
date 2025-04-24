#ifndef _FRAMEGRAPH_REALIZE_H_
#define _FRAMEGRAPH_REALIZE_H_

#include <memory>
#include <type_traits>

namespace framegraph
{
	template<typename DescriptionType, typename ActualType>
	struct MisingRealizeImplementation : std::false_type {};

	template<typename DescriptionType, typename ActualType>
	std::unique_ptr<ActualType> realize(const DescriptionType& description)
	{
		static_assert(MisingRealizeImplementation<DescriptionType, ActualType>::value, "missing realize implementation for description - type pair.");
		return nullptr;
	}
}

#endif