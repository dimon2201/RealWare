// render_thread.cpp

#include <cstring>
#include <tracy/Tracy.hpp>
#include "render_thread.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "window.hpp"
#include "graphics.hpp"
#include "graphics_backend2.hpp"
#include "thread_guard.hpp"
#include "input_backend.hpp"

using namespace types;

void triton::cRenderThread::ThreadFunction()
{
	std::mutex mtx;
	CThreadGuard::CaptureRenderThreadId();

	tracy::SetThreadName("Render Thread");

	IGraphicsBackend2* gfxBackend = _context->GetBackend<IGraphicsBackend2>();

	// Create graphics contexts for windows
	IApplication* app = _context->GetSubsystem<CEngine>()->GetApplication();
	CWindow* window = app->GetWindow();
	//gfxBackend->CreateGraphicsContext(window->GetBackendWindow());
	//gfxBackend->MakeWindowGraphicsContextCurrent(window->GetBackendWindow());
	EGraphicsDeviceType deviceType =
		_context->GetSubsystem<CEngine>()->
		GetApplication()->
		GetCapabilities().preferredGraphicsDevice;
	gfxBackend->Initialize(
		window->GetBackendWindow(),
		True,
		_context->GetBackend<IInputBackend>()->GetBackendWindowVulkanExtensions(),
		deviceType,
		window->GetSize(),
		app->GetCapabilities().framesInFlight
	);

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
				gfxBackend->BeginFrame();
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend
				);
				gfxBackend->EndFrame();
			}
			else if (frame.operation == EProducedFrameOp::ExecuteCommandsOnly)
			{
				// Execute render commands only
				ExecuteCommands(
					frame.renderCommandPack,
					gfxBackend
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
	IGraphicsBackend2* gfxBackend
)
{
	for (usize i = 0; i < renderCommandPack.count; i++)
	{
		const SRenderCommand& cmd = renderCommandPack.cmds[i];

		switch (cmd._command)
		{
			case ERenderCommand::RESIZE_RENDER_TARGETS:
			{
				/*gfx->ResizeRenderTargets(cVector2(
					cmd._args._argA,
					cmd._args._argB
				));*/
				break;
			}
			case ERenderCommand::CLEAR:
			{
				/*gfxBackend->ClearColor(cVector4(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				));
				gfxBackend->ClearDepth(
					cmd._args._argE
				);*/
				break;
			}
			case ERenderCommand::DRAW:
			{
				/*gfxBackend->Draw(
					cmd._args._argA,
					cmd._args._argB,
					cmd._args._argC,
					cmd._args._argD
				);*/
				break;
			}
			case ERenderCommand::CREATE_BUFFER:
			{
				CGPUBufferResource resultBuffer = gfxBackend->CreateBuffer(
					(EGPUBufferType)cmd._args._argA,
					cmd._args._argB
				);
				new (_sync->GetResultBuffer().data) CGPUBufferResource(resultBuffer);
				break;
			}
			case ERenderCommand::BIND_BUFFER:
			{
				//CGPUBufferResource* buffer = (CGPUBufferResource*)cmd._args._argA;
				//gfxBackend->BindBuffer(*buffer);
				break;
			}
			case ERenderCommand::WRITE_BUFFER:
			{
				gfxBackend->WriteBuffer(
					*((CGPUBufferResource*)cmd._args._argA),
					(usize)cmd._args._argB,
					(const u8*)cmd._args._argC,
					(usize)cmd._args._argD
				);
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
				//CGPUInputLayoutResource resultIA = gfxBackend->CreateInputLayout();
				//memcpy(&_sync->GetResultBuffer().data[0], &resultIA, sizeof(CGPUInputLayoutResource));
				break;
			}
			case ERenderCommand::BIND_INPUT_LAYOUT:
			{
				//CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				//gfxBackend->BindInputLayout(*ia);
				break;
			}
			case ERenderCommand::UNBIND_INPUT_LAYOUT:
			{
				//gfxBackend->UnbindInputLayout();
				break;
			}
			case ERenderCommand::DESTROY_INPUT_LAYOUT:
			{
				//CGPUInputLayoutResource* ia = (CGPUInputLayoutResource*)cmd._args._argA;
				//gfxBackend->DestroyInputLayout(*ia);
				break;
			}
			case ERenderCommand::CREATE_TEXTURE:
			{
				CGPUTextureResource resultTexture = gfxBackend->CreateTexture(
					cmd._args._argA,
					(ETextureFormat)cmd._args._argB,
					(dword)cmd._args._argC,
					(ETextureDimension)cmd._args._argD,
					cVector3(cmd._args._argE, cmd._args._argF, cmd._args._argG)
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultTexture, sizeof(CGPUTextureResource));
				break;
			}
			case ERenderCommand::WRITE_TEXTURE:
			{
				/*gfxBackend->WriteTexture(
					*((CGPUTextureResource*)cmd._args._argA),
					cVector3(cmd._args._argB, cmd._args._argC, cmd._args._argD),
					cVector2(cmd._args._argE, cmd._args._argF),
					(u8*)cmd._args._argG
				);*/
				break;
			}
			case ERenderCommand::GENERATE_TEXTURE_MIPS:
			{
				/*gfxBackend->GenerateTextureMips(
					*((CGPUTextureResource*)cmd._args._argA)
				);*/
				break;
			}
			case ERenderCommand::DESTROY_TEXTURE:
			{
				gfxBackend->DestroyTexture(
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
				CGPURenderTargetResource resultRenderTarget = gfxBackend->CreateRenderTarget(
					attachmentsVec,
					*((CGPUTextureResource*)cmd._args._argC)
				);
				new (_sync->GetResultBuffer().data) CGPURenderTargetResource(resultRenderTarget);
				break;
			}
			case ERenderCommand::CREATE_SHADER:
			{
				CGPUShaderResource resultGPUShader = gfxBackend->CreateShader(
					(dword)cmd._args._argA,
					*((SShaderBytecodeFiles*)cmd._args._argB)
				);
				new (_sync->GetResultBuffer().data) CGPUShaderResource(resultGPUShader);
				break;
			}
			case ERenderCommand::DESTROY_SHADER:
			{
				gfxBackend->DestroyShader(
					*((CGPUShaderResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::BIND_RIGID_INPUT_LAYOUT:
			{
				//gfxBackend->BindRigidInputLayout();
				break;
			}
			case ERenderCommand::BIND_SKINNED_INPUT_LAYOUT:
			{
				//gfxBackend->BindSkinnedInputLayout();
				break;
			}
			case ERenderCommand::CREATE_RENDER_PASS:
			{
				CGPURenderPassResource resultRenderPass = gfxBackend->CreateRenderPass(
					*((CGPURenderTargetResource*)cmd._args._argA),
					cmd._args._argB,
					*(cVector4*)cmd._args._argC,
					*(f32*)cmd._args._argD,
					*(std::vector<EResourceUsage>*)cmd._args._argE,
					*(std::vector<EResourceUsage>*)cmd._args._argF
				);
				memcpy(&_sync->GetResultBuffer().data[0], &resultRenderPass, sizeof(CGPURenderPassResource));
				break;
			}
			case ERenderCommand::BeginRenderPass:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BeginRenderPass,
					(const void*)cmd._args._argA,
					(const void*)cmd._args._argB
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindPipeline,
					(const void*)cmd._args._argC,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindVertexBuffer,
					(const void*)cmd._args._argD,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindIndexBuffer,
					(const void*)cmd._args._argE,
					nullptr
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::PushConstants,
					(const void*)cmd._args._argF,
					(const void*)cmd._args._argC
				);
				break;
			}
			case ERenderCommand::ExecuteRenderPass:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindDescriptorSets,
					(const void*)cmd._args._argA,
					(const void*)cmd._args._argB
				);
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::Draw,
					(const void*)cmd._args._argC,
					nullptr
				);
				break;
			}
			case ERenderCommand::EndRenderPass:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::EndRenderPass,
					nullptr,
					nullptr
				);
				break;
			}
			case ERenderCommand::DESTROY_RENDER_PASS:
			{
				gfxBackend->DestroyRenderPass(
					*((CGPURenderPassResource*)cmd._args._argA)
				);
				break;
			}
			case ERenderCommand::CREATE_PIPELINE:
			{
				CGPUPipelineResource resultPipeline = gfxBackend->CreatePipeline(
					*(CGPUShaderResource*)cmd._args._argA,
					*(SViewport*)cmd._args._argB,
					*(CGPURenderTargetResource*)cmd._args._argC,
					*(CGPURenderPassResource*)cmd._args._argD,
					*(std::vector<CGPUBindingGroupLayoutResource>*)cmd._args._argE,
					(EPrimitiveTopology)cmd._args._argF,
					(EVertexBufferFormat)cmd._args._argG,
					(boolean)cmd._args._argH
				);
				
				new (_sync->GetResultBuffer().data) CGPUPipelineResource(resultPipeline);

				break;
			}
			case ERenderCommand::DESTROY_PIPELINE:
			{
				gfxBackend->DestroyPipeline(
					*(CGPUPipelineResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::CreateBindingGroupLayout:
			{
				CGPUBindingGroupLayoutResource resultBindingGroupLayout = gfxBackend->CreateBindingGroupLayout(
					*(const std::vector<SBindingGroupBinding>*)cmd._args._argA
				);

				new (_sync->GetResultBuffer().data) CGPUBindingGroupLayoutResource(resultBindingGroupLayout);

				break;
			}
			case ERenderCommand::DestroyBindingGroupLayout:
			{
				gfxBackend->DestroyBindingGroupLayout(
					*(CGPUBindingGroupLayoutResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::CreateBindingGroup:
			{
				CGPUBindingGroupResource resultBindingGroup = gfxBackend->CreateBindingGroup(
					*(const CGPUBindingGroupLayoutResource*)cmd._args._argA,
					*(const std::vector<SBindingGroupBinding>*)cmd._args._argB,
					*(const std::vector<SBindingGroupBinding>*)cmd._args._argC
				);

				new (_sync->GetResultBuffer().data) CGPUBindingGroupResource(resultBindingGroup);

				break;
			}
			case ERenderCommand::DestroyBindingGroup:
			{
				gfxBackend->DestroyBindingGroup(
					*(CGPUBindingGroupResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::FinalizeSwapchain:
			{
				gfxBackend->FinalizeSwapchain(
					*(CGPUTextureResource*)cmd._args._argA
				);
				break;
			}
			case ERenderCommand::ReleaseSwapchainResources:
			{
				gfxBackend->ReleaseSwapchainResources();
				break;
			}
			case ERenderCommand::BindVertexBuffer:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindVertexBuffer,
					(const void*)cmd._args._argA,
					nullptr
				);
				break;
			}
			case ERenderCommand::BindIndexBuffer:
			{
				gfxBackend->AddCommandToBuffer(
					ENativeRenderCommand::BindIndexBuffer,
					(const void*)cmd._args._argA,
					nullptr
				);
				break;
			}
		}
	}
}

void triton::cRenderThread::Present(IGraphicsBackend2* gfxBackend)
{
	//gfxBackend->SwapWindowBuffers(_window->GetBackendWindow());
	gfxBackend->BeginFrame();
	gfxBackend->EndFrame();
}