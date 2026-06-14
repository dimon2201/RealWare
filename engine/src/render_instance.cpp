// render_instance.cpp

#include "render_instance.hpp"

using namespace types;

triton::SRenderInstance::SRenderInstance(s32 materialIndex, const cTransform& transform)
{
    // TODO: Implement 2D/3D render instances
    //_use2D = transform._use2D;
    _materialIndex = materialIndex;
    _world = transform.GetWorld();
}