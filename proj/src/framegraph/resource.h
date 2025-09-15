#ifndef _FRAMEGRAPH_RESOURCE_H_
#define _FRAMEGRAPH_RESOURCE_H_



#include <memory>
#include <string>
#include <variant>

#include "resource_base.h"
#include "realize.h"

namespace fg
{
	template<typename DescriptionType_, typename ActualType_>
	class Resource : public ResourceBase
	{
	public:
		using DescriptionType = DescriptionType_;
		using ActualType = ActualType_;

		explicit Resource(const std::string& name, const class RenderTaskBase* creator, const DescriptionType& description, ActualType* actual = nullptr) :
			ResourceBase(name, creator),
			description_(description),
			actual_(actual)
		{
		}

		explicit Resource(const std::string& name, const DescriptionType& description, ActualType* actual = nullptr) :
			ResourceBase(name, nullptr),
			description_(description),
			actual_(actual)
		{
			if (!actual) actual_ = framegraph::realize<DescriptionType, ActualType>(description);
		}

		Resource(const Resource& that) = delete;
		Resource(Resource&& temp) = default;
		~Resource() = default;
		Resource& operator=(const Resource& that) = delete;
		Resource& operator=(Resource&& temp) = default;


		const DescriptionType& description() const
		{
			return description_;
		}

		ActualType* actual() const
		{
			return std::holds_alternative<std::unique_ptr<ActualType>>(actual_) ? std::get<std::unique_ptr<ActualType>>(actual_).get() : std::get<ActualType*>(actual_);
		}


	protected:
		void realize() override
		{
			if (transient()) std::get<std::unique_ptr<ActualType>>(actual_) = framegraph::realize<DescriptionType, ActualType>(description_);
		}

		void derealize() override
		{
			if (transient()) std::get<std::unique_ptr<ActualType>>(actual_).reset();
		}

		DescriptionType description_;
		std::variant<std::unique_ptr<ActualType>, ActualType*> actual_;
	};
}

#endif