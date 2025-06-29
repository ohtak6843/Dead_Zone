#include "pch.h"
#include "LightObject.h"
#include "Transform.h"
#include "Light.h"
#include "BaseCollider.h"
#include "Animator.h"

LightObject::LightObject()
{
	_type = GAMEOBJECT_TYPE::LIGHT;
}

LightObject::~LightObject()
{
}

void LightObject::Awake()
{
}

void LightObject::Start()
{
}

void LightObject::Update()
{
}

void LightObject::LateUpdate()
{
}

void LightObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

	if (_light != nullptr)
		_light->FinalUpdate();

	if (_collider != nullptr)
		_collider->FinalUpdate();

	if (_animator != nullptr)
		_animator->FinalUpdate();
}

void LightObject::SetLight(shared_ptr<Light> light)
{
	light->SetGameObject(shared_from_this());
	_light = light;
}
