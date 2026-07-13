// render_thread.cpp

#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "input.hpp"
#include "graphics.hpp"
#include "graphics_context_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "render_subsystem.hpp"
#include "thread_guard.hpp"
#include "dynamic_array.hpp"

using namespace types;

triton::cRenderThread::cRenderThread(cContext* context, XEngineMTSynchronization* synchronization, XRenderSubsystem* renderSubsystem) : cThread(context), _synchronization(synchronization), _renderSubsystem(renderSubsystem)
{
	const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
	_resultBuffer = (u8*)_context->GetMemoryAllocator()->Allocate(caps->futureResultBufferByteSize, 64);
	_initialized.store(K_FALSE);
}

triton::cRenderThread::~cRenderThread()
{
	_context->GetMemoryAllocator()->Deallocate(_resultBuffer);
}

void triton::cRenderThread::ThreadFunction()
{
	std::mutex mtx;
	CThreadGuard::CaptureRenderThreadId();

	// Create graphics contexts for windows
	cInput* inputSubsystem = _context->GetSubsystem<cInput>();
	std::vector<cInputWindow>* windows = inputSubsystem->GetWindows();
	iGraphicsContextBackend* gfxContextBackend = _context->GetBackend<iGraphicsContextBackend>();
	for (usize i = 0; i < windows->size(); i++)
	{
		gfxContextBackend->CreateGraphicsContext(windows->at(i).GetBackendWindow());
		gfxContextBackend->MakeWindowGraphicsContextCurrent(windows->at(i).GetBackendWindow());
	}

	_initialized.store(K_TRUE);
	_renderSubsystem->NotifyMainThread(); // TODO: move XRenderSubsystem::NotifyMainThread() to cRenderThread
	
	{
		std::lock_guard<std::mutex> lg(mtx);
		std::cout << "Render thread waits for main thread to initialize\n\n";
	}
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

		EFrameState mainThreadSignal = _synchronization->WaitForProducedFrame(_cv);

		if (!_synchronization->IsAlive())
			break;

		const CRenderFrame* renderFrame = _synchronization->AcquireProducedFrame(mainThreadSignal);

		if (mainThreadSignal == EFrameState::EXECUTE_FULL)
		{
			// Full job
			cGraphics* gfx = _context->GetSubsystem<cGraphics>();

			// Core events
			//MakeContextCurrent(renderFrame, gfxContextBackend);
			ExecuteCommands(renderFrame, gfxDrawcallBackend, gfxResourceBackend, gfxPipelineBackend, gfx);
			
			// Execute render passes
			gfx->ExecutePasses();

			Present(renderFrame, gfxContextBackend);
		}
		else if (mainThreadSignal == EFrameState::EXECUTE_COMMANDS)
		{
			// Execute render commands only
			cGraphics* gfx = _context->GetSubsystem<cGraphics>();
			//MakeContextCurrent(renderFrame, gfxContextBackend);
			ExecuteCommands(renderFrame, gfxDrawcallBackend, gfxResourceBackend, gfxPipelineBackend, gfx);
		}

		_synchronization->ReleaseFrame(renderFrame->GetIndexInSwapChain());

		_synchronization->LoopFinish();
		_renderSubsystem->NotifyMainThread();
	}
}

void triton::cRenderThread::NotifyThread()
{
	_cv.notify_one();
}

void triton::cRenderThread::MakeContextCurrent(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->MakeWindowGraphicsContextCurrent(renderFrame->GetWindow()->GetBackendWindow());
}

void triton::cRenderThread::ExecuteCommands(const CRenderFrame* renderFrame, iGraphicsDrawcallBackend* drawcallBackend, iGraphicsResourceBackend* resourceBackend, iGraphicsPipelineBackend* pipelineBackend, cGraphics* gfx)
{
	while (auto result = renderFrame->Next())
	{
		const SRenderCommand* cmd = *result;

		switch (cmd->_command)
		{
			case ERenderCommand::RESIZE_RENDER_TARGETS:
			{
				gfx->ResizeRenderTargets(cVector2(
					cmd->_args._argA,
					cmd->_args._argB
				));
				break;
			}
			case ERenderCommand::CLEAR:
			{
				drawcallBackend->ClearColor(cVector4(
					cmd->_args._argA,
					cmd->_args._argB,
					cmd->_args._argC,
					cmd->_args._argD
				));
				drawcallBackend->ClearDepth(
					cmd->_args._argE
				);
				break;
			}
			case ERenderCommand::DRAW:
			{
				drawcallBackend->Draw(
					cmd->_args._argA,
					cmd->_args._argB,
					cmd->_args._argC,
					cmd->_args._argD
				);
				break;
			}
			case ERenderCommand::WRITE_BUFFER:
			{
				resourceBackend->WriteBuffer(
					(cBuffer*)cmd->_args._argA,
					cmd->_args._argB,
					cmd->_args._argC,
					(const u8*)cmd->_args._argD
				);
				break;
			}
			case ERenderCommand::CREATE_BUFFER:
			{
				cBuffer* resultBuffer = resourceBackend->CreateBuffer(
					(cBuffer::eType)cmd->_args._argA,
					(u8*)cmd->_args._argB,
					cmd->_args._argC,
					cmd->_args._argD
				);
				memcpy(&_resultBuffer[0], &resultBuffer, sizeof(cBuffer*));
				break;
			}
			case ERenderCommand::BIND_BUFFER:
			{
				cBuffer* buffer = (cBuffer*)cmd->_args._argA;
				resourceBackend->BindBuffer(buffer);
				break;
			}
			case ERenderCommand::CREATE_VERTEX_ARRAY:
			{
				CGPUVertexArray resultVertexArray = pipelineBackend->CreateVertexArray();
				memcpy(&_resultBuffer[0], &resultVertexArray, sizeof(CGPUVertexArray));
				break;
			}
			case ERenderCommand::BIND_VERTEX_ARRAY:
			{
				CGPUVertexArray* vertexArray = (CGPUVertexArray*)cmd->_args._argA;
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
				CGPUVertexArray* vertexArray = (CGPUVertexArray*)cmd->_args._argA;
				pipelineBackend->DestroyVertexArray(*vertexArray);
				break;
			}
			case ERenderCommand::CREATE_TEXTURE:
			{
				cTexture* resultTexture = resourceBackend->CreateTexture(
					cVector3(cmd->_args._argA, cmd->_args._argB, cmd->_args._argC),
					(cTexture::eDimension)cmd->_args._argD,
					(cTexture::eFormat)cmd->_args._argE,
					(u8*)cmd->_args._argF,
					cmd->_args._argG
				);
				memcpy(&_resultBuffer[0], &resultTexture, sizeof(cTexture*));
				break;
			}
			case ERenderCommand::WRITE_TEXTURE:
			{
				resourceBackend->WriteTexture(
					(cTexture*)cmd->_args._argA,
					cVector3(cmd->_args._argB, cmd->_args._argC, cmd->_args._argD),
					cVector2(cmd->_args._argE, cmd->_args._argF),
					(u8*)cmd->_args._argG
				);
				break;
			}
			case ERenderCommand::GENERATE_TEXTURE_MIPS:
			{
				resourceBackend->GenerateTextureMips(
					(cTexture*)cmd->_args._argA
				);
				break;
			}
			case ERenderCommand::CREATE_RENDER_TARGET:
			{
				usize attachmentCount = cmd->_args._argA;
				cTexture** attachments = (cTexture**)cmd->_args._argB;
				std::vector<cTexture*> attachmentsVec;
				for (usize i = 0; i < attachmentCount; i++)
					attachmentsVec.push_back(attachments[i]);
				XRenderTarget* resultRT = pipelineBackend->CreateRenderTarget(
					attachmentsVec,
					(cTexture*)cmd->_args._argC
				);
				memcpy(&_resultBuffer[0], &resultRT, sizeof(XRenderTarget*));
				XRenderTarget* pr = (XRenderTarget*)&_resultBuffer[0];
				break;
			}
			case ERenderCommand::CREATE_SHADER:
			{
				CGPUShader resultGPUShader = pipelineBackend->CreateShader(
					(const char*)cmd->_args._argA,
					(const char*)cmd->_args._argB,
					(const char*)cmd->_args._argC,
					(const char*)cmd->_args._argD,
					cmd->_args._argE,
					(const SShaderDefine*)cmd->_args._argF,
					cmd->_args._argG,
					(const char**)cmd->_args._argH,
					cmd->_args._argI,
					(const char**)cmd->_args._argJ
				);
				memcpy(&_resultBuffer[0], &resultGPUShader, sizeof(CGPUShader));
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

void triton::cRenderThread::Present(const CRenderFrame* renderFrame, iGraphicsContextBackend* contextBackend)
{
	contextBackend->SwapWindowBuffers(renderFrame->GetWindow()->GetBackendWindow());
}