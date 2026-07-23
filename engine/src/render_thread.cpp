// render_thread.cpp

#include <tracy/Tracy.hpp>
#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "thread_guard.hpp"
#include "dynamic_array.hpp"

using namespace types;

triton::cRenderThread::cRenderThread(
	cContext* context,
	XSynchronization* sync
) : cThread(context), _sync(sync)
{
}

triton::cRenderThread::~cRenderThread()
{
}

void triton::cRenderThread::ThreadFunction()
{
	std::mutex mtx;
	CThreadGuard::CaptureRenderThreadId();

	tracy::SetThreadName("Render Thread");

	// Create graphics contexts for windows
	cInput* inputSubsystem = _context->GetSubsystem<cInput>();
	std::vector<cInputWindow>* windows = inputSubsystem->GetWindows();
	iGraphicsContextBackend* gfxContextBackend = _context->GetBackend<iGraphicsContextBackend>();
	for (usize i = 0; i < windows->size(); i++)
	{
		gfxContextBackend->CreateGraphicsContext(windows->at(i).GetBackendWindow());
		gfxContextBackend->MakeWindowGraphicsContextCurrent(windows->at(i).GetBackendWindow());
	}

	_sync->InitRenderThread();
	
	{
		std::lock_guard<std::mutex> lg(mtx);
		std::cout << "Render thread ready and starts\n\n";
	}

	iGraphicsDrawcallBackend* gfxDrawcallBackend = _context->GetBackend<iGraphicsDrawcallBackend>();
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();

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
				cGraphics* gfx = _context->GetSubsystem<cGraphics>();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxDrawcallBackend,
					gfxResourceBackend,
					gfxPipelineBackend,
					gfx
				);

				gfx->ExecutePasses();

				Present(frame.window, gfxContextBackend);
			}
			else if (frame.operation == EProducedFrameOp::ExecuteCommandsOnly)
			{
				// Execute render commands only
				cGraphics* gfx = _context->GetSubsystem<cGraphics>();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxDrawcallBackend,
					gfxResourceBackend,
					gfxPipelineBackend,
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
	iGraphicsDrawcallBackend* drawcallBackend,
	iGraphicsResourceBackend* resourceBackend,
	iGraphicsPipelineBackend* pipelineBackend,
	cGraphics* gfx
)
{
	for (usize i = 0; i < renderCommandPack.cmds.size(); i++)
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
				drawcallBackend->ClearColor(cVector4(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				));
				drawcallBackend->ClearDepth(
					cmd._args._argE
				);
				break;
			}
			case ERenderCommand::DRAW:
			{
				drawcallBackend->Draw(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);
				break;
			}
			case ERenderCommand::WRITE_BUFFER:
			{
				resourceBackend->WriteBuffer(
					(cBuffer*)cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					(const u8*)cmd._args._argD
				);
				break;
			}
			case ERenderCommand::CREATE_BUFFER:
			{
				cBuffer* resultBuffer = resourceBackend->CreateBuffer(
					(cBuffer::eType)cmd._args._argA,
					(u8*)cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultBuffer, sizeof(cBuffer*));
				break;
			}
			case ERenderCommand::BIND_BUFFER:
			{
				cBuffer* buffer = (cBuffer*)cmd._args._argA;
				resourceBackend->BindBuffer(buffer);
				break;
			}
			case ERenderCommand::CREATE_VERTEX_ARRAY:
			{
				CGPUVertexArray resultVertexArray = pipelineBackend->CreateVertexArray();
				memcpy(&_sync->GetResultBuffer().data[0], &resultVertexArray, sizeof(CGPUVertexArray));
				break;
			}
			case ERenderCommand::BIND_VERTEX_ARRAY:
			{
				CGPUVertexArray* vertexArray = (CGPUVertexArray*)cmd._args._argA;
				pipelineBackend->BindVertexArray(*vertexArray);
				break;
			}
			case ERenderCommand::UNBIND_VERTEX_ARRAY:
			{
				pipelineBackend->UnbindVertexArray();
				break;
			}
			case ERenderCommand::DESTROY_VERTEX_ARRAY:
			{
				CGPUVertexArray* vertexArray = (CGPUVertexArray*)cmd._args._argA;
				pipelineBackend->DestroyVertexArray(*vertexArray);
				break;
			}
			case ERenderCommand::CREATE_TEXTURE:
			{
				cTexture* resultTexture = resourceBackend->CreateTexture(
					cVector3(cmd._args._argA, cmd._args._argB, cmd._args._argC),
					(cTexture::eDimension)cmd._args._argD,
					(cTexture::eFormat)cmd._args._argE,
					(u8*)cmd._args._argF,
					cmd._args._argG
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultTexture, sizeof(cTexture*));
				break;
			}
			case ERenderCommand::WRITE_TEXTURE:
			{
				resourceBackend->WriteTexture(
					(cTexture*)cmd._args._argA,
					cVector3(cmd._args._argB, cmd._args._argC, cmd._args._argD),
					cVector2(cmd._args._argE, cmd._args._argF),
					(u8*)cmd._args._argG
				);
				break;
			}
			case ERenderCommand::GENERATE_TEXTURE_MIPS:
			{
				resourceBackend->GenerateTextureMips(
					(cTexture*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::CREATE_RENDER_TARGET:
			{
				usize attachmentCount = cmd._args._argA;
				cTexture** attachments = (cTexture**)cmd._args._argB;
				std::vector<cTexture*> attachmentsVec;
				for (usize i = 0; i < attachmentCount; i++)
					attachmentsVec.push_back(attachments[i]);
				XRenderTarget* resultRT = pipelineBackend->CreateRenderTarget(
					attachmentsVec,
					(cTexture*)cmd._args._argC
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultRT, sizeof(XRenderTarget*));
				break;
			}
			case ERenderCommand::CREATE_SHADER:
			{
				CGPUShader resultGPUShader = pipelineBackend->CreateShader(
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
				memcpy(&_sync->GetResultBuffer().data[0], &resultGPUShader, sizeof(CGPUShader));
				break;
			}
			case ERenderCommand::BIND_DEFAULT_INPUT_LAYOUT:
			{
				pipelineBackend->BindDefaultInputLayout();
				break;
			}
		}
	}
}

void triton::cRenderThread::Present(const cInputWindow* window, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(window->GetBackendWindow());
}