// render_frame.hpp

#pragma once

#include <condition_variable>
#include <vector>
#include <optional>
#include <mutex>
#include <tuple>
#include "object.hpp"
#include "thread_guard.hpp"
#include "types.hpp"

namespace triton
{
	class cInputWindow;

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
		CREATE_VERTEX_ARRAY,
		BIND_VERTEX_ARRAY,
		UNBIND_VERTEX_ARRAY,
		DESTROY_VERTEX_ARRAY,
		BIND_DEFAULT_INPUT_LAYOUT
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
			ERenderCommand command_,
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

		inline SRenderCommand& SetLabel(const std::string& label)
		{
			_label = label;
			return *this;
		}

		ERenderCommand _command = ERenderCommand::NONE;
		SRenderCommandArgs _args = {};
		std::string _label;
	};

	struct SRenderCommandPack final
	{
		std::vector<SRenderCommand> cmds;
	};

	class XRenderCommandRecorder : public iObject
	{
		TRITON_OBJECT(XRenderCommandRecorder)

		SRenderCommandPack _renderCommandPack;

	public:
		explicit XRenderCommandRecorder(cContext* context) : iObject(context) {}
		virtual ~XRenderCommandRecorder() = default;

		inline void Clear()
		{
			_renderCommandPack.cmds.clear();
		}

		inline void PushCommand(const SRenderCommand& cmd)
		{
			_renderCommandPack.cmds.push_back(cmd);
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
			renderCommandPack.cmds.clear();
		}

		void Apply(
			EProducedFrameOp operation_,
			const SRenderCommandPack& renderCommandPack_,
			cInputWindow* window_
		)
		{
			operation = operation_;
			renderCommandPack = renderCommandPack_;
			window = window_;
		}

		EProducedFrameOp operation = EProducedFrameOp::None;
		SRenderCommandPack renderCommandPack;
		cInputWindow* window = nullptr;
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

	class XSynchronization : public iObject
	{
		TRITON_OBJECT(XSynchronization)

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
			XRenderCommandRecorder* cmdRecorder
		);
		~XSynchronization() override;

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

			TResult result = *(TResult*)&_resultBuffer.data[0];

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
	};
}