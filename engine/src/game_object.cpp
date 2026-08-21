// game_object.cpp

#include "game_object.hpp"
#include "skin_pool.hpp"
#include "render_instance_pack_pool.hpp"
#include "render_instance_static_pool.hpp"
#include "render_instance_dynamic_pool.hpp"
#include "animation_pool.hpp"
#include "render_instance.hpp"

using namespace types;

triton::XGameObject::XGameObject(cContext* context, s32 poolIndex, const std::string& name)
	: iObject(context, poolIndex), _name(name) {}

triton::XGameObject::XGameObject(
    cContext* context,
	s32 poolIndex,
    const std::string& name,
    const SModel3DData& model,
    const XRenderInstancePack::THandle& renderInstancePack
) : iObject(context, poolIndex)
{
	_skeleton = model.skeleton;
	_animations = model.animations;
	_skin = *_context->GetPool<CSkinPool>()->Create(_skeleton);
	_animations = model.animations;
	_renderInstancePack = renderInstancePack;

	XRenderInstancePack& rip = *_context->GetPool<CRenderInstancePackPool>()->Get(_renderInstancePack);
	_motionType = rip.GetMotionType();

	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance::THandle renderInstanceHandle = *SetRenderable(True, renderInstancePack);
	XRenderInstance& ri = *renderInstancePool->Get(renderInstanceHandle);
	ri.SetSkin(_skin);
}

std::optional<triton::XRenderInstance::THandle> triton::XGameObject::SetRenderable(
	types::boolean bIsRenderable,
	const XRenderInstancePack::THandle& renderInstancePack
)
{
	if (bIsRenderable == True)
	{
		XRenderInstancePack& rip = *_context->GetPool<CRenderInstancePackPool>()->Get(renderInstancePack);
		_motionType = rip.GetMotionType();
		_renderInstancePack = renderInstancePack;

		CRenderInstancePool* renderInstancePool = nullptr;
		if (_motionType == ERenderInstanceMotionType::Static)
			renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
		else if (_motionType == ERenderInstanceMotionType::Dynamic)
			renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

		if (renderInstancePool->Get(_renderInstance).has_value())
			return std::nullopt;

		auto result = rip.AddInstance();
		if (!result.has_value())
			return std::nullopt;

		_renderInstance = *result;

		return _renderInstance;
	}
	else if (bIsRenderable == False)
	{
		CRenderInstancePool* renderInstancePool = nullptr;
		if (_motionType == ERenderInstanceMotionType::Static)
			renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
		else if (_motionType == ERenderInstanceMotionType::Dynamic)
			renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

		if (!renderInstancePool->Get(_renderInstance).has_value())
			return std::nullopt;

		XRenderInstancePack& rip = *_context->GetPool<CRenderInstancePackPool>()->Get(_renderInstancePack);
		rip.RemoveInstance(_renderInstance);

		_motionType = ERenderInstanceMotionType::Unknown;
		_renderInstancePack = XRenderInstancePack::THandle();
		_renderInstance = XRenderInstance::THandle();
	}
}

void triton::XGameObject::SetMaterial(const XMaterial::THandle& material)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance& ri = *renderInstancePool->Get(_renderInstance);
	ri.SetMaterial(material);

	renderInstancePool->WriteToStaging(
		ri.GetAccessIndex(),
		ri
	);
}

void triton::XGameObject::SetWorldPosition(const cVector3& worldPosition)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance& ri = *renderInstancePool->Get(_renderInstance);
	ri.SetWorldPosition(worldPosition);
	ri.Transform();

	renderInstancePool->WriteToStaging(
		ri.GetAccessIndex(),
		ri
	);
}

void triton::XGameObject::SetRotation(const cVector3& rotation)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance& ri = *renderInstancePool->Get(_renderInstance);
	ri.SetRotation(rotation);
	ri.Transform();

	renderInstancePool->WriteToStaging(
		ri.GetAccessIndex(),
		ri
	);
}

void triton::XGameObject::SetScale(const cVector3& scale)
{
	CRenderInstancePool* renderInstancePool = nullptr;
	if (_motionType == ERenderInstanceMotionType::Static)
		renderInstancePool = _context->GetPool<CRenderInstanceStaticPool>();
	else if (_motionType == ERenderInstanceMotionType::Dynamic)
		renderInstancePool = _context->GetPool<CRenderInstanceDynamicPool>();

	XRenderInstance& ri = *renderInstancePool->Get(_renderInstance);
	ri.SetScale(scale);
	ri.Transform();

	renderInstancePool->WriteToStaging(
		ri.GetAccessIndex(),
		ri
	);
}

void triton::XGameObject::PlayAnimation(usize index)
{
	XAnimation& animation = *_context->GetPool<CAnimationPool>()->Get(_animations[index]);
	XSkin& skin = *_context->GetPool<CSkinPool>()->Get(_skin);

	static f32 time = 0.0f;

	SAnimationFrame frame = *animation.EvaluateFrame(_skeleton, time);
	skin.Skin(frame);

	time += 0.1f;
}