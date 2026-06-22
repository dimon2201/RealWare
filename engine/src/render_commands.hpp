// render_commands.hpp

#pragma once

#include <future>
#include "types.hpp"

namespace triton
{
	class cTexture;

	enum class ERenderCommand
	{
		NONE = 0,
		RESIZE_RENDER_TARGETS,
		CLEAR,
		DRAW,
		CREATE_BUFFER,
		WRITE_BUFFER,
		DESTROY_BUFFER
	};

	class CRenderCommand
	{
	protected:
		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
		types::cpuword _argE = 0;
		types::cpuword _argF = 0;
		types::cpuword _argG = 0;
		types::cpuword _argH = 0;

	public:
		virtual ~CRenderCommand() = default;

		virtual void Execute(cContext* context) = 0;
	};

	class CCreateTextureCommand final : public CRenderCommand
	{
		std::promise<cTexture*> _promise = {};

	public:
		~CCreateTextureCommand() override = default;

		void Execute(cContext* context) override;
		std::future<cTexture*> GetFuture();
	};
}