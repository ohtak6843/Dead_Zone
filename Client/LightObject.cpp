#include "pch.h"
#include "LightObject.h"
#include "Transform.h"
#include "Light.h"
#include "BaseCollider.h"
#include "Animator.h"

#include "InputMgr.h"

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
	// TODO: 그림자 테스트용, 나중에 삭제
	if (INPUT->GetButton(KEY_TYPE::KEY_5))
	{
		Vec3 position = GetTransform()->GetLocalPosition();
		position.y += 50.0f;
		GetTransform()->SetLocalPosition(position);
	}

	if (INPUT->GetButton(KEY_TYPE::KEY_6))
	{
		Vec3 position = GetTransform()->GetLocalPosition();
		position.y -= 50.0f;
		GetTransform()->SetLocalPosition(position);
	}

	if (INPUT->GetButton(KEY_TYPE::KEY_7))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.x += 15.0f;
		GetTransform()->SetLocalRotation(rotation);
	}

	if (INPUT->GetButton(KEY_TYPE::KEY_8))
	{
		Vec3 rotation = GetTransform()->GetLocalRotation();
		rotation.x -= 15.0f;
		GetTransform()->SetLocalRotation(rotation);
	}
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
