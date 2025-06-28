#include "pch.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Light.h"
#include "Particle.h"
#include "BaseCollider.h"
#include "Animator.h"

GameObject::GameObject() : Object(OBJECT_TYPE::GAMEOBJECT)
{
	_type = GAMEOBJECT_TYPE::DEFAULT;
}

GameObject::~GameObject()
{

}

GameObject& GameObject::operator=(const GameObject& other)
{
	_transform = other._transform;
	_meshRenderer = other._meshRenderer;
	_collider = other._collider;
	_animator = other._animator;

	_isActive = other._isActive;
	_checkFrustum = other._checkFrustum;
	_layerIndex = other._layerIndex;
	_static = other._static;

	return *this;
}

void GameObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

	if (_collider != nullptr)
		_collider->FinalUpdate();

	if (_animator != nullptr)
		_animator->FinalUpdate();
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
