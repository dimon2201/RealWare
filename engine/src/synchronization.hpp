// render_frame.hpp

#pragma once

#include <condition_variable>
#include <vector>
#include <optional>
#include <mutex>
#include <tuple>
#include "object.hpp"
#include "context.hpp"
#include "thread_guard.hpp"
#include "application.hpp"
#include "types.hpp"

namespace triton
{
	class CWindow;

	enum class ERenderCommand
	{
		NONE = 0,
		RESIZE_RENDER_TARGETS,
		RESIZE_RENDER_TARGET_COLORS,
		RESIZE_RENDER_TARGET_DEPTH,
		RESIZE_TEXTURE,
		CLEAR,
		DRAW,
		CREATE_BUFFER,
		BIND_BUFFER,
		WRITE_BUFFER,
		DESTROY_BUFFER,
		CREATE_TEXTURE,
		WRITE_TEXTURE,
		GENERATE_TEXTURE_MIPS,
		DESTROY_TEXTURE,
		CREATE_RENDER_TARGET,
		DESTROY_RENDER_TARGET,
		CREATE_SHADER,
		DESTROY_SHADER,
		CREATE_INPUT_LAYOUT,
		BIND_INPUT_LAYOUT,
		BIND_RIGID_INPUT_LAYOUT,
		BIND_SKINNED_INPUT_LAYOUT,
		UNBIND_INPUT_LAYOUT,
		DESTROY_INPUT_LAYOUT,
		CREATE_RENDER_PASS,
		EXECUTE_RENDER_PASS,
		DESTROY_RENDER_PASS,
		CREATE_PIPELINE,
		DESTROY_PIPELINE,
		FinalizeSwapchain,
		ReleaseSwapchainResources,
		BindVertexBuffer,
		BindIndexBuffer,
		CreateBindingGroupLayout,
		DestroyBindingGroupLayout,
		CreateBindingGroup,
		DestroyBindingGroup
	};

	struct SRenderCommandArgs
	{
		SRenderCommandArgs(
			types::cpuword argA_ = 0,
			types::cpuword argB_ = 0,
			types::cpuword argC_ = 0,
			types::cpuword argD_ = 0,
			types::cpuword argE_ = 0,
			types::cpuword argF_ = 0,
			types::cpuword argG_ = 0,
			types::cpuword argH_ = 0,
			types::cpuword argI_ = 0,
			types::cpuword argJ_ = 0
		) : _argA(argA_), _argB(argB_), _argC(argC_), _argD(argD_), _argE(argE_), _argF(argF_), _argG(argG_), _argH(argH_), _argI(argI_), _argJ(argJ_) {
		}

		types::cpuword _argA = 0;
		types::cpuword _argB = 0;
		types::cpuword _argC = 0;
		types::cpuword _argD = 0;
		types::cpuword _argE = 0;
		types::cpuword _argF = 0;
		types::cpuword _argG = 0;
		types::cpuword _argH = 0;
		types::cpuword _argI = 0;
		types::cpuword _argJ = 0;
	};

	struct SRenderCommand
	{
		SRenderCommand(
			ERenderCommand command_ = ERenderCommand::NONE,
			types::cpuword argA_ = 0,
			types::cpuword argB_ = 0,
			types::cpuword argC_ = 0,
			types::cpuword argD_ = 0,
			types::cpuword argE_ = 0,
			types::cpuword argF_ = 0,
			types::cpuword argG_ = 0,
			types::cpuword argH_ = 0,
			types::cpuword argI_ = 0,
			types::cpuword argJ_ = 0
		) : _command(command_), _args(argA_, argB_, argC_, argD_, argE_, argF_, argG_, argH_, argI_, argJ_) {}

		ERenderCommand _command = ERenderCommand::NONE;
		SRenderCommandArgs _args = {};
	};

	struct SRenderCommandPack final
	{
		types::usize count = 0;
		SRenderCommand cmds[1024];
	};

	class XRenderCommandRecorder
	{
		SRenderCommandPack _renderCommandPack;

	public:
		virtual ~XRenderCommandRecorder() = default;

		inline void Clear()
		{
			_renderCommandPack.count = 0;
		}

		inline void PushCommand(const SRenderCommand& cmd)
		{
			const types::usize idx = _renderCommandPack.count++;
			_renderCommandPack.cmds[idx] = cmd;
		}

		inline const SRenderCommandPack& GetRenderCommandPack()
		{
			return _renderCommandPack;
		}
	};
	
	enum class EProducedFrameOp
	{
		None,
		ExecuteCommandsOnly,
		ExecuteFull,
		Kill
	};

	struct alignas(64) SProducedFrameData
	{
		void Clear()
		{
			operation = EProducedFrameOp::None;
			renderCommandPack.count = 0;
		}

		void Apply(
			EProducedFrameOp operation_,
			const SRenderCommandPack& renderCommandPack_
		)
		{
			operation = operation_;
			renderCommandPack = renderCommandPack_;
		}

		EProducedFrameOp operation = EProducedFrameOp::None;
		SRenderCommandPack renderCommandPack;
	};

	struct alignas(64) SReleasedFrameData
	{
	};

	struct SSyncAtomics
	{
		alignas(64) std::atomic<types::usize> producedFrame = { 0 };
		alignas(64) std::atomic<types::usize> releasedFrame = { 1 };
		alignas(64) types::usize lastProducedFrame = 0;
		alignas(64) types::usize lastReleasedFrame = 0;
	};

	struct SResultBuffer final
	{
		types::u8* data = nullptr;
	};

	class XSynchronization
	{
		cContext* _context = nullptr;
		XRenderCommandRecorder* _cmdRecorder = nullptr;
		std::atomic<types::boolean> _bIsRenderThreadInitialized = { types::K_FALSE };
		SProducedFrameData _producedFrameData[2];
		SReleasedFrameData _releasedFrameData;
		types::usize _writeIndex = 1;
		std::atomic<types::usize> _readIndex = { 0 };
		SSyncAtomics _frameAtomics;
		SResultBuffer _resultBuffer;
		std::atomic<types::boolean> _bIsAlive = { types::K_TRUE };
		std::atomic<types::boolean> _bIsRenderJobFinished = { types::K_FALSE };

	public:
		explicit XSynchronization(
			cContext* context,
			XRenderCommandRecorder* cmdRecorder,
			types::usize resultBufferByteSize
		);
		~XSynchronization();

		void WaitForRenderThreadToInit();

		void WaitForReleasedFrame();

		void WaitForProducedFrame();

		void WaitForRenderJobFinish();

		void InitRenderThread();

		void ProduceFrame(
			EProducedFrameOp operation,
			const SRenderCommandPack& renderCommandPack
		);

		void ReleaseFrame();

		inline void FinishRenderJob()
		{
			_bIsRenderJobFinished.store(types::K_TRUE, std::memory_order_release);
			_bIsRenderJobFinished.notify_one();
		}

		template <typename TResult>
		TResult WaitForRenderCommandResult()
		{
			CThreadGuard::AssertMain();

			WaitForReleasedFrame();
			ProduceFrame(
				EProducedFrameOp::ExecuteCommandsOnly,
				_cmdRecorder->GetRenderCommandPack()
			);
			_cmdRecorder->Clear();
			WaitForRenderJobFinish();

			// TODO: add result buffer locking/unlocking
			// |||||||||||||||||||||||||||||||||||||||||
			// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
			TResult result = *(TResult*)&_resultBuffer.data[0];
			((TResult*)&_resultBuffer.data[0])->~TResult();

			return result;
		}

		inline const SProducedFrameData& GetProducedFrameData() const
		{
			CThreadGuard::AssertRender();

			const types::usize readIndex = _readIndex.load(std::memory_order_acquire);

			return _producedFrameData[readIndex];
		}

		inline SResultBuffer& GetResultBuffer()
		{
			return _resultBuffer;
		}

		inline types::usize GetWriteIndex() const { return _writeIndex; }

		inline const std::atomic<types::usize>& GetReadIndex() const { return _readIndex; }
	};
}