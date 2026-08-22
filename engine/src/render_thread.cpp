// render_thread.cpp

#include <tracy/Tracy.hpp>
#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "graphics.hpp"
#include "graphics_backend.hpp"
#include "thread_guard.hpp"

using namespace types;

void triton::cRenderThread::ThreadFunction()
{
	std::mutex mtx;
	CThreadGuard::CaptureRenderThreadId();

	tracy::SetThreadName("Render Thread");

	IGraphicsBackend* gfxBackend = _context->GetBackend<IGraphicsBackend>();

	// Create graphics contexts for windows
	CWindow* window = _context->GetSubsystem<CEngine>()->GetApplication()->GetWindow();
	gfxBackend->CreateGraphicsContext(window->GetBackendWindow());
	gfxBackend->MakeWindowGraphicsContextCurrent(window->GetBackendWindow());

	_sync->InitRenderThread();
	
	{
		std::lock_guard<std::mutex> lg(mtx);
		std::cout << "Render thread ready and starts\n\n";
	}

	while (K_TRUE)
	{
		//_synchronization->LoopStart();
		//_renderSubsystem->NotifyMainThread();

		{
			ZoneScopedN("Wait for Produced Frame");

			_sync->WaitForProducedFrame();
		}

		boolean bDoExit = K_FALSE;
		{
			ZoneScopedN("Render Job");

			const SProducedFrameData& frame = _sync->GetProducedFrameData();

			if (frame.operation == EProducedFrameOp::Kill)
			{
				bDoExit = K_TRUE;
			}
			else if (frame.operation == EProducedFrameOp::ExecuteFull)
			{
				// Full job
				CGraphics* gfx = _context->GetSubsystem<CGraphics>();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend,
					gfx
				);

				gfx->ExecutePasses();

				Present(gfxBackend);
			}
			else if (frame.operation == EProducedFrameOp::ExecuteCommandsOnly)
			{
				// Execute render commands only
				CGraphics* gfx = _context->GetSubsystem<CGraphics>();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend,
					gfx
				);
			}
		}
		
		_sync->FinishRenderJob();

		{
			ZoneScopedN("Release Frame");

			_sync->ReleaseFrame();
		}

		if (bDoExit == K_TRUE)
			break;
	}
}

void triton::cRenderThread::ExecuteCommands(
	const SRenderCommandPack& renderCommandPack,
	IGraphicsBackend* gfxBackend,
	CGraphics* gfx
)
{
	for (usize i = 0; i < renderCommandPack.count; i++)
	{
		const SRenderCommand& cmd = renderCommandPack.cmds[i];

		switch (cmd._command)
		{
			case ERenderCommand::RESIZE_RENDER_TARGETS:
			{
				gfx->ResizeRenderTargets(cVector2(
					cmd._args._argA,
					cmd._args._argB
				));
				break;
			}
			case ERenderCommand::CLEAR:
			{
				gfxBackend->ClearColor(cVector4(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				));
				gfxBackend->ClearDepth(
					cmd._args._argE
				);
				break;
			}
			case ERenderCommand::DRAW:
			{
				gfxBackend->Draw(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);
				break;
			}
			case ERenderCommand::WRITE_BUFFER:
			{
				gfxBackend->WriteBuffer(
					*((CGPUBufferResource*)cmd._args._argA),
					cmd._args._argB,
					cmd._args._argC,
					(const u8*)cmd._args._argD
				);
				break;
			}
			case ERenderCommand::CREATE_BUFFER:
			{
				CGPUBufferResource resultBuffer = gfxBackend->CreateBuffer(
					(EGPUBufferType)cmd._args._argA,
					(u8*)cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultBuffer, sizeof(CGPUBufferResource));
				break;
			}
			case ERenderCommand::BIND_BUFFER:
			{
				CGPUBufferResource* buffer = (CGPUBufferResource*)cmd._args._argA;
				gfxBackend->BindBuffer(*buffer);
				break;
			}
			case ERenderCommand::DESTROY_BUFFER:
			{
				gfxBackend->DestroyBuffer(
					*((CGPUBufferResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_INPUT_LAYOUT:
			{
				CGPUInputLayoutResource resultIA = gfxBackend->CreateInputLayout();
				memcpy(&_sync->GetResultBuffer().data[0], &resultIA, sizeof(CGPUInputLayoutResource));
				break;
			}
			case ERenderCommand::BIND_INPUT_LAYOUT:
			{
				CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				gfxBackend->BindInputLayout(*ia);
				break;
			}
			case ERenderCommand::UNBIND_INPUT_LAYOUT:
			{
				gfxBackend->UnbindInputLayout();
				break;
			}
			case ERenderCommand::DESTROY_INPUT_LAYOUT:
			{
				CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				gfxBackend->DestroyInputLayout(*ia);
				break;
			}
			case ERenderCommand::CREATE_TEXTURE:
			{
				CGPUTextureResource resultTexture = gfxBackend->CreateTexture(
					cVector3(cmd._args._argA, cmd._args._argB, cmd._args._argC),
					(ETextureDimension)cmd._args._argD,
					(ETextureFormat)cmd._args._argE,
					(u8*)cmd._args._argF,
					cmd._args._argG
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultTexture, sizeof(CGPUTextureResource));
				break;
			}
			case ERenderCommand::WRITE_TEXTURE:
			{
				gfxBackend->WriteTexture(
					*((CGPUTextureResource*)cmd._args._argA),
					cVector3(cmd._args._argB, cmd._args._argC, cmd._args._argD),
					cVector2(cmd._args._argE, cmd._args._argF),
					(u8*)cmd._args._argG
				);
				break;
			}
			case ERenderCommand::GENERATE_TEXTURE_MIPS:
			{
				gfxBackend->GenerateTextureMips(
					*((CGPUTextureResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_RENDER_TARGET:
			{
				usize attachmentCount = cmd._args._argA;
				CGPUTextureResource* attachments = (CGPUTextureResource*)cmd._args._argB;
				std::vector<CGPUTextureResource> attachmentsVec;
				for (usize i = 0; i < attachmentCount; i++)
					attachmentsVec.push_back(attachments[i]);
				CGPURenderTargetResource resultRT = gfxBackend->CreateRenderTarget(
					attachmentsVec,
					*((CGPUTextureResource*)cmd._args._argC)
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultRT, sizeof(CGPURenderTargetResource));
				break;
			}
			case ERenderCommand::CREATE_SHADER:
			{
				CGPUShaderResource resultGPUShader = gfxBackend->CreateShader(
					(const char*)cmd._args._argA,
					(const char*)cmd._args._argB,
					(const char*)cmd._args._argC,
					(const char*)cmd._args._argD,
					cmd._args._argE,
					(const SShaderDefine*)cmd._args._argF,
					cmd._args._argG,
					(const char**)cmd._args._argH,
					cmd._args._argI,
					(const char**)cmd._args._argJ
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultGPUShader, sizeof(CGPUShaderResource));
				break;
			}
			case ERenderCommand::BIND_STATIC_INPUT_LAYOUT:
			{
				gfxBackend->BindStaticInputLayout();
				break;
			}
			case ERenderCommand::BIND_SKINNED_INPUT_LAYOUT:
			{
				gfxBackend->BindSkinnedInputLayout();
				break;
			}
		}
	}
}

void triton::cRenderThread::Present(IGraphicsBackend* gfxBackend)
{
	gfxBackend->SwapWindowBuffers(_window->GetBackendWindow());
}