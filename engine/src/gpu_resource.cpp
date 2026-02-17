// gpu_resource.cpp

#include "gpu_resource.hpp"

using namespace types;

triton::cGPUResource::cGPUResource(cContext* context, qword instance, qword viewInstance)
	: iObject(context), _instance(instance), _viewInstance(viewInstance) {}