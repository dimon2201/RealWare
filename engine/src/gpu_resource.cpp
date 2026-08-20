// gpu_resource.cpp

#include "gpu_resource.hpp"

using namespace types;

triton::cGPUResource::cGPUResource(qword instance, qword viewInstance)
	: _instance(instance), _viewInstance(viewInstance) {}