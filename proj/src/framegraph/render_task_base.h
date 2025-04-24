#ifndef _FRAMEGRAPH_RENDER_TASK_BASE_H_
#define _FRAMEGRAPH_RENDER_TASK_BASE_H_


#include <cstddef>
#include <string>
#include <vector>

namespace framegraph
{
	class RenderTaskBase
	{
	public:
		explicit RenderTaskBase(const std::string& name) :
			name_(name),
			cullImmune_(false),
			refCount_(0)
		{
		}

		RenderTaskBase(const RenderTaskBase& that) = delete;
		RenderTaskBase(RenderTaskBase&& temp) = default;
		virtual ~RenderTaskBase() = default;
		RenderTaskBase& operator=(const RenderTaskBase& that) = delete;
		RenderTaskBase& operator=(RenderTaskBase&& temp) = default;

		const std::string& name() const
		{
			return name_;
		}

		void setName(const std::string& name)
		{
			name_ = name;
		}

		bool cullImmune() const
		{
			return cullImmune_;
		}

		void setCullImmune(bool cullImmune)
		{
			cullImmune_ = cullImmune;
		}

	protected:

		friend class Framegraph;
		friend class RenderTaskBuilder;

		virtual void setup(RenderTaskBuilder& builder) = 0;
		virtual void execute() const = 0;

		std::string name_;
		bool cullImmune_;
		std::vector<const class ResourceBase*> creates_;
		std::vector<const class ResourceBase*> reads_;
		std::vector<const class ResourceBase*> writes_;
		std::size_t refCount_;
	};
}

#endif