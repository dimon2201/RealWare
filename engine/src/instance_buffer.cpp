// instance_buffer.cpp

#include "instance_buffer.hpp"
#include "capabilities.hpp"
#include "engine.hpp"
#include "context.hpp"
#include "data_buffer.hpp"
#include "components.hpp"
#include "handle_allocator.hpp"
#include "buffer_view.hpp"
#include "render_subsystem.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XInstanceBuffer::XInstanceBuffer(cContext* context, ERenderInstanceMotionType usage) : cBuffer(context, 0, cBuffer::eType::NONE, 0, 0), _usage(usage) {}

triton::XInstanceBuffer::XInstanceBuffer(cContext* context, ERenderInstanceMotionType usage, cBuffer* buffer) : cBuffer(context, buffer->GetInstance(), buffer->GetBufferType(), buffer->GetByteSize(), buffer->GetSlot()), _usage(usage) {}