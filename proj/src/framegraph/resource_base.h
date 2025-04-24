#ifndef _FRAMEGRAPH_RESOURCE_BASE_H_
#define _FRAMEGRAPH_RESOURCE_BASE_H_

#include <cstddef>
#include <string>
#include <vector>

namespace framegraph
{
	class ResourceBase
	{
	public:
		explicit ResourceBase(const std::string& name, const class RenderTaskBase* creator) :
			name_(name),
			creator_(creator),
			refCount_(0)
		{
			static std::size_t id = 0;
			id_ = id++;
		}

		ResourceBase(const ResourceBase& that) = delete;
		ResourceBase(ResourceBase&& temp) = default;
		virtual ~ResourceBase() = default;
		ResourceBase& operator=(const ResourceBase& that) = delete;
		ResourceBase& operator=(ResourceBase&& temp) = default;

		std::size_t id() const
		{
			return id_;
		}

		const std::string& name() const
		{
			return name_;
		}

		void setName(const std::string& name)
		{
			name_ = name;
		}

		bool transient() const
		{
			return creator_ != nullptr;
		}

	protected:
		friend class Framegraph;
		friend class RenderTaskBuilder;

		virtual void realize() = 0;
		virtual void derealize() = 0;

		std::size_t id_;
		std::string name_;
		const class RenderTaskBase* creator_;
		std::vector<class RenderTaskBase*> readers_;
		std::vector<class RenderTaskBase*> writers_;
		std::size_t refCount_;
	};
}

#endif