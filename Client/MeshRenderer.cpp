#include "pch.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "InstancingMgr.h"
#include "Resources.h"
#include "Animator.h"

MeshRenderer::MeshRenderer()
{

}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::SetMaterial(shared_ptr<Material> material, uint32 idx)
{
	if (_materials.size() <= static_cast<size_t>(idx))
		_materials.resize(static_cast<size_t>(idx + 1));

	_materials[idx] = material;
}

void MeshRenderer::Render()
{
	for (uint32 i = 0; i < _materials.size(); i++)
	{
		shared_ptr<Material>& material = _materials[i];

		if (material == nullptr || material->GetShader() == nullptr)
			continue;

		GetTransform()->PushData();

		if (GetAnimator())
		{
			GetAnimator()->PushData();
			material->SetInt(1, 1);
		}

		material->PushGraphicsData();
		_mesh->Render(1, i);
	}
}

void MeshRenderer::Render(shared_ptr<InstancingBuffer>& buffer)
{
	for (uint32 i = 0; i < _materials.size(); i++)
	{
		shared_ptr<Material>& material = _materials[i];

		if (material == nullptr || material->GetShader() == nullptr)
			continue;

		buffer->PushData();

		if (GetAnimator())
		{
			GetAnimator()->PushData();
			material->SetInt(1, 1);
		}

		material->PushGraphicsData();
		_mesh->Render(buffer, i);
	}
}

void MeshRenderer::RenderShadow()
{
	//GetTransform()->PushData();

	//shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Shadow")->Clone();

	//if (GetAnimator())
	//{
	//	GetAnimator()->PushData();
	//	material->SetInt(1, 1);
	//}
	//else
	//{
	//	material->SetInt(1, 0);
	//}

	//material->PushGraphicsData();
	//_mesh->Render();
	shared_ptr<Material> shadowMaterial = GET_SINGLE(Resources)->Get<Material>(L"Shadow");


	for (uint32 i = 0; i < _materials.size(); ++i)
	{
		if (_materials[i] == nullptr)
			continue;

		GetTransform()->PushData();

		if (GetAnimator())
		{
			GetAnimator()->PushData();
			shadowMaterial->SetInt(1, 1);
		}
		else
		{
			shadowMaterial->SetInt(1, 0);
		}

		shadowMaterial->PushGraphicsData();
		_mesh->Render(1, i);
	}
}

uint64 MeshRenderer::GetInstanceID()
{
	if (_mesh == nullptr || _materials.empty())
		return 0;

	//uint64 id = (_mesh->GetID() << 32) | _material->GetID();
	InstanceID instanceID{ _mesh->GetID(), _materials[0]->GetID() };
	return instanceID.id;
}