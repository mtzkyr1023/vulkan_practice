#ifndef _FRAMEGRAPH_RENDER_TASK_H_
#define _FRAMEGRAPH_RENDER_TASK_H_



#include <functional>
#include <string>

#include "render_task_base.h"

namespace framegraph
{
	template<typename DataType_>
	class RenderTask : public RenderTaskBase
	{
	public:
		using DataType = DataType_;

		explicit RenderTask(
			const std::string& name,
			const std::function<void(DataType&, class RenderTaskBuilder&)>& setup,
			const std::function<void(DataType&)>& execute) :
			RenderTaskBase(name)
		{

		}

		RenderTask(const RenderTask& that) = delete;
		RenderTask(RenderTask&& temp) = default;
		virtual ~RenderTask() = default;
		RenderTask& operator=(const RenderTask& that) = delete;
		RenderTask& operator(RenderTask&& temp) = default;

		const DataType& data() const
		{
			return data_;
		}

	protected:
		friend class Framegraph;
		friend class RenderTaskBuilder;

		void setup(class RenderTaskBuilder& builder) override
		{
			setup_(data_, builder);
		}

		void execute() override
		{
			execute_(data_);
		}

		DatType data_;
		const std::function<void(DataType&, class RenderTaskBuilder&)> setup_;
		const std::function<void(DatType&)> execute_;
	};
}

#endif