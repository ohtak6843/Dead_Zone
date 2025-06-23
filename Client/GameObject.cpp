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

GameObject& GameObject::operator=(const GameObject& other)
{
	_transform = other._transform;
	_meshRenderer = other._meshRenderer;
	_camera = other._camera;
	_light = other._light;
	_particle = other._particle;
	_collider = other._collider;
	_animator = other._animator;
	_scripts = other._scripts;

	_isActive = other._isActive;
	_checkFrustum = other._checkFrustum;
	_layerIndex = other._layerIndex;
	_static = other._static;

	return *this;
}

void GameObject::Awake()
{
	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Awake();
	}
}

void GameObject::Start()
{
	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Start();
	}
}

void GameObject::Update()
{
	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Update();
	}
}

void GameObject::LateUpdate()
{
	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->LateUpdate();
	}
}

void GameObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

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

void GameObject::SetMeshRenderer(shared_ptr<MeshRenderer> meshRenderer)
{
	meshRenderer->SetGameObject(shared_from_this());
	_meshRenderer = meshRenderer;
}

void GameObject::SetCamera(shared_ptr<Camera> camera)
{
	camera->SetGameObject(shared_from_this());
	_camera = camera;
}

void GameObject::SetLight(shared_ptr<Light> light)
{
	light->SetGameObject(shared_from_this());
	_light = light;
}

void GameObject::SetParticle(shared_ptr<Particle> particle)
{
	particle->SetGameObject(shared_from_this());
	_particle = particle;
}

void GameObject::SetCollider(shared_ptr<BaseCollider> collider)
{
	collider->SetGameObject(shared_from_this());
	_collider = collider;
}

void GameObject::SetAnimator(shared_ptr<Animator> animator)
{
	animator->SetGameObject(shared_from_this());
	_animator = animator;
}

void GameObject::AddScript(shared_ptr<MonoBehaviour> script)
{
	script->SetGameObject(shared_from_this());
	_scripts.push_back(script);
}