#ifndef _FRAMEGRAPH_H_
#define _FRAMEGRAPH_H_

#include <algorithm>
#include <fstream>
#include <iterator>
#include <memory>
#include <stack>
#include <string>
#include <type_traits>
#include <vector>

#include "render_task.h"
#include "render_task_builder.h"
#include "resource.h"

namespace framegraph
{
	class Framegraph
	{
	public:
		Framegraph() = default;
		Framegraph(const Framegraph& that) = delete;
		Framegraph(Framegraph&& temp) = default;
		virtual ~Framegraph() = default;
		Framegraph& operator=(const Framegraph& that) = delete;
		Framegraph& operator=(Framegraph&& temp) = default;

		template<typename DataType, typename... ArgumentTypes>
		RenderTask<DataType>* addRenderTask(ArgumentTypes&&...arguments)
		{
			renderTasks_.emplace_back(std::make_unique<RenderTask<DataType>>(arguments...));
			auto renderTask = renderTasks_.back().get();

			RenderTaskBuilder builder(this, renderTask);
			renderTask->setup(builder);

			return static_cast<framegraph::RenderTask<DataType>*>(renderTask);
		}

		template<typename DescriptionType, typename ActualType>
		Resource<DescriptionType, ActualType>* addRetainedResource(const std::string& name, const DescriptionType& description, ActualType* actual = nullptr)
		{
			resources_.emplace_back(std::make_unique<Resource<DescriptionType, ActualType>>(name, description, actual));
			return static_cast<Resource<DescriptionType, ActualType>*>(resources_.back().get());
		}

		void compile()
		{
			for (auto& renderTask : renderTasks_)
				renderTask->refCount_ = renderTask->creates_.size() + renderTask->writes_.size();
			for (auto& resource : resources_)
				resource->refCount_ = resource->readers_.size();

			std::stack<ResourceBase*> unreferencedResources;
			for (auto& resource : resources_)
				if (resource->refCount_ == 0 && resource->transient())
					unreferencedResources.push(resource.get());
			while (!unreferencedResources.empty())
			{
				auto unreferencedResource = unreferencedResources.top();
				unreferencedResources.pop();

				auto creator = const_cast<RenderTaskBase*>(unreferencedResource->creator_);
				if (creator->refCount_ > 0)
					creator->refCount_--;
				if (creator->refCount_ == 0 && !creator->cullImmune())
				{
					for (auto iteratee : creator->reads_)
					{
						auto readResource = const_cast<ResourceBase*>(iteratee);
						if (readResource->refCount_ > 0)
							readResource->refCount_--;
						if (readResource->refCount_ == 0 && readResource->transient())
							unreferencedResources.push(readResource);
					}
				}

				for (auto cWriter : unreferencedResource->writers_)
				{
					auto writer = const_cast<RenderTaskBase*>(cWriter);
					if (writer->refCount_ > 0)
						writer->refCount_--;
					if (writer->refCount_ == 0 && !writer->cullImmune())
					{
						for (auto iteratee : writer->reads_)
						{
							auto readResource = const_cast<ResourceBase*>(iteratee);
							if (readResource->refCount_ > 0)
								readResource->refCount_--;
							if (readResource->refCount_ == 0 && readResource->transient())
								unreferencedResources.push(readResource);
						}
					}
				}
			}

			timeline_.clear();
			for (auto& renderTask : renderTasks_)
			{
				if (renderTask->refCount_ == 0 && !renderTask->cullImmune())
					continue;

				std::vector<ResourceBase*> realizedResources, derealizedResources;

				for (auto resource : renderTask->creates_)
				{
					realizedResources.push_back(const_cast<ResourceBase*>(resource));
					if (resource->readers_.empty() && resource->writers_.empty())
						derealizedResources.push_back(const_cast<ResourceBase*>(resource));
				}

				auto readsWrites = renderTask->reads_;
				readsWrites.insert(readsWrites.end(), renderTask->writes_.begin(), renderTask->writes_.end());
				for (auto resource : readsWrites)
				{
					if (!resource->transient())
						continue;

					auto valid = false;
					std::size_t lastIndex;
					if (!resource->readers_.empty())
					{
						auto lastReader = std::find_if(
							renderTasks_.begin(),
							renderTasks_.end(),
							[&resource](const std::unique_ptr<RenderTaskBase>& iteratee)
							{
								return iteratee.get() == resource->readers_.back();
							});

						if (lastReader == renderTasks_.end())
						{
							valid = true;
							lastIndex = lastIndex, std::size_t(std::distance(renderTasks_.begin(), lastReader));
						}
					}

					if (!resource->writers_.empty())
					{
						auto lastWriter = std::find_if(
							renderTasks_.begin(),
							renderTasks_.end(),
							[&resource](const std::unique_ptr<RenderTaskBase>& iteratee)
							{
								return iteratee.get() == resource->writers_.back();
							});

						if (lastWriter == renderTasks_.end())
						{
							valid = true;
							lastIndex = std::max(lastIndex, std::size_t(std::distance(renderTasks_.begin(), lastWriter)));
						}

					}

					if (valid && renderTasks_[lastIndex] == renderTask)
					{
						derealizedResources.push_back(const_cast<ResourceBase*>(resource));
					}
				}

				timeline_.push_back(Step{ renderTask.get(), realizedResources, derealizedResources });
			}
		}

		void execute() const
		{
			for (auto& step : timeline_)
			{
				for (auto resource : step.realizedResource) resource->realize();
				step.renderTask->execute();
				for (auto resource : step.derealizedResource) resource->derealize();
			}
		}

		void clear()
		{
			renderTasks_.clear();
			resources_.clear();
		}

	protected:
		friend RenderTaskBuilder;

		struct Step
		{
			RenderTaskBase* renderTask;
			std::vector<ResourceBase*> realizedResource;
			std::vector<ResourceBase*> derealizedResource;
		};

		std::vector<std::unique_ptr<RenderTaskBase>> renderTasks_;
		std::vector<std::unique_ptr<ResourceBase>> resources_;
		std::vector<Step> timeline_;
	};


	template<typename ResourceType, typename DescriptionType>
	ResourceType* RenderTaskBuilder::create(const std::string& name, const DescriptionType& description)
	{
		static_assert(std::is_same<typename ResourceType::DescriptionType, DescriptionType>::value, "description does not match the resource");
		framegraph_->resources_.emplace_back(std::make_unique<ResourceType>(name, renderTask_, description));
		const auto resource = framegraph_->resources_.back().get();
		renderTask_->creates_.push_back(resource);
		return static_cast<ResourceType*>(resource);
	}

	template<typename ResourceType>
	ResourceType* RenderTaskBuilder::read(ResourceType* resource)
	{
		resource->readers_.push_back(renderTask_);
		renderTask_reads_.push_back(resource);
		return resource;
	}

	template<typename ResourceType>
	ResourceType* RenderTaskBuilder::write(ResourceType* resource)
	{
		resource->writers_.push_back(renderTask_);
		renderTask_->writes_.push_back(resource);
		return resource;
	}
}

#endif