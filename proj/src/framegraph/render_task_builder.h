#ifndef _FRAMEGRAPH_RENDER_TASK_BUILDER_H_
#define _FRAMEGRAPH_RENDER_TASK_BUILDER_H_

#include <string>

namespace framegraph
{
	class RenderTaskBuilder
	{
	public:
		explicit RenderTaskBuilder(class Framegraph* framegraph, class RenderTaskBase* renderTask)
		{

		}

		RenderTaskBuilder(const RenderTaskBuilder& that) = default;
		RenderTaskBuilder(RenderTaskBuilder&& temp) = default;
		virtual ~RenderTaskBuilder() = default;
		RenderTaskBuilder& operator=(const RenderTaskBuilder& that) = default;
		RenderTaskBuilder& operator=(RenderTaskBuilder&& temp) = default;

		template<typename ResourceType, typename DescriptionType>
		ResourceType* create(const std::string& name, const DescriptionType& description);
		template<typename ResourceType>
		ResourceType* read(ResourceType* resource);
		template<typename ResourceType>
		ResourceType* write(ResourceType* resource);

	protected:
		class Framegraph* framegraph_;
		class RenderTaskBase* renderTask_;
	};
}

#endif