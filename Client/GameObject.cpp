#include "pch.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Light.h"
#include "MonoBehaviour.h"
#include "Particle.h"
#include "BaseCollider.h"
#include "Animator.h"

GameObject::GameObject() : Object(OBJECT_TYPE::GAMEOBJECT)
{

}

GameObject::~GameObject()
{

}

void GameObject::Awake()
{
	if (_transform != nullptr)
		_transform->Awake();

	if (_meshRenderer != nullptr)
		_meshRenderer->Awake();

	if (_camera != nullptr)
		_camera->Awake();

	if (_light != nullptr)
		_light->Awake();

	if (_particle != nullptr)
		_particle->Awake();

	if (_collider != nullptr)
		_collider->Awake();

	if (_animator != nullptr)
		_animator->Awake();

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Awake();
	}
}

void GameObject::Start()
{
	if (_transform != nullptr)
		_transform->Start();

	if (_meshRenderer != nullptr)
		_meshRenderer->Start();

	if (_camera != nullptr)
		_camera->Start();

	if (_light != nullptr)
		_light->Start();

	if (_particle != nullptr)
		_particle->Start();

	if (_collider != nullptr)
		_collider->Start();

	if (_animator != nullptr)
		_animator->Start();

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Start();
	}
}

void GameObject::Update()
{
	if (_transform != nullptr)
		_transform->Update();

	if (_meshRenderer != nullptr)
		_meshRenderer->Update();

	if (_camera != nullptr)
		_camera->Update();

	if (_light != nullptr)
		_light->Update();

	if (_particle != nullptr)
		_particle->Update();

	if (_collider != nullptr)
		_collider->Update();

	if (_animator != nullptr)
		_animator->Update();

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Update();
	}
}

void GameObject::LateUpdate()
{
	if (_transform != nullptr)
		_transform->LateUpdate();

	if (_meshRenderer != nullptr)
		_meshRenderer->LateUpdate();

	if (_camera != nullptr)
		_camera->LateUpdate();

	if (_light != nullptr)
		_light->LateUpdate();

	if (_particle != nullptr)
		_particle->LateUpdate();

	if (_collider != nullptr)
		_collider->LateUpdate();

	if (_animator != nullptr)
		_animator->LateUpdate();

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->LateUpdate();
	}
}

void GameObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

	if (_meshRenderer != nullptr)
		_meshRenderer->FinalUpdate();

	if (_camera != nullptr)
		_camera->FinalUpdate();

	if (_light != nullptr)
		_light->FinalUpdate();

	if (_particle != nullptr)
		_particle->FinalUpdate();

	if (_collider != nullptr)
		_collider->FinalUpdate();

	if (_animator != nullptr)
		_animator->FinalUpdate();
}

shared_ptr<MonoBehaviour> GameObject::GetMonoBehaviour(const wstring& name)
{
	for (auto script : _scripts)
	{
		if (script->GetName() == name)
			return script;
	}

	return nullptr;
}

void GameObject::SetTransform(shared_ptr<Transform> transform)
{
	transform->SetGameObject(shared_from_this());
	_transform = transform;
}

void GameObject::AddScript(shared_ptr<MonoBehaviour> script)
{
	_scripts.push_back(script);
}