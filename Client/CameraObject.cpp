#include "pch.h"
#include "CameraObject.h"
#include "Transform.h"
#include "Camera.h"
#include "BaseCollider.h"
#include "Animator.h"

CameraObject::CameraObject()
{
	_type = GAMEOBJECT_TYPE::CAMERA;
}

CameraObject::~CameraObject()
{
}

void CameraObject::Awake()
{
}

void CameraObject::Start()
{
}

void CameraObject::Update()
{
}

void CameraObject::LateUpdate()
{
}

void CameraObject::FinalUpdate()
{
	if (_transform != nullptr)
		_transform->FinalUpdate();

	if (_camera != nullptr)
		_camera->FinalUpdate();

	if (_collider != nullptr)
		_collider->FinalUpdate();

	if (_animator != nullptr)
		_animator->FinalUpdate();
}

void CameraObject::SetCamera(shared_ptr<Camera> camera)
{
	camera->SetGameObject(shared_from_this());
	_camera = camera;
}
