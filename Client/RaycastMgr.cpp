#include "pch.h"
#include "RaycastMgr.h"
#include "SceneMgr.h"
#include "Camera.h"
#include "Framework.h"
#include "Scene.h"
#include "GameObject.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

void RaycastMgr::SetRayOriginAndDir(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GET_SINGLE(SceneMgr)->GetActiveScene()->GetMainCamera();

	float width = static_cast<float>(gameFramework->GetWindow().width);
	float height = static_cast<float>(gameFramework->GetWindow().height);

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

	rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
	rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
	rayDir.Normalize();

	_rayOrigin = rayOrigin;
	_rayDir = rayDir;
}

shared_ptr<class GameObject> RaycastMgr::Pick(int32 screenX, int32 screenY)
{
	SetRayOriginAndDir(screenX, screenY);

	vector<shared_ptr<GameObject>> gameObjects = GET_SINGLE(SceneMgr)->GetActiveScene()->GetGameObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> pickedObject = nullptr;
	float distance = 0.f;

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetCollider() == nullptr)
			continue;

		ColliderType colliderType = gameObject->GetCollider()->GetColliderType();
		if (colliderType == ColliderType::SPHERE)
		{
			shared_ptr<SphereCollider> sphere = static_pointer_cast<SphereCollider>(gameObject->GetCollider());
			if (sphere->Intersects(_rayOrigin, _rayDir, OUT distance) == false)
				continue;
		}
		else if (colliderType == ColliderType::OBB)
		{
			shared_ptr<OrientedBoxCollider> obb = static_pointer_cast<OrientedBoxCollider>(gameObject->GetCollider());
			if (obb->Intersects(_rayOrigin, _rayDir, OUT distance) == false)
				continue;
		}
		else
		{
			continue;
		}

		if (distance < minDistance)
		{
			minDistance = distance;
			pickedObject = gameObject;
		}
	}

	_distance = distance;

	return pickedObject;
}

shared_ptr<class GameObject> RaycastMgr::PickZombie(int32 screenX, int32 screenY)
{
	SetRayOriginAndDir(screenX, screenY);

	auto& zombies = GET_SINGLE(SceneMgr)->GetActiveScene()->GetZombies();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> pickedObject = nullptr;
	float distance = 0.f;

	for (auto& zombieGroup : zombies)
	{
		for (auto& zombiePart : zombieGroup)
		{
			if (zombiePart->GetCollider() == nullptr)
				continue;

			ColliderType colliderType = zombiePart->GetCollider()->GetColliderType();
			if (colliderType == ColliderType::SPHERE)
			{
				shared_ptr<SphereCollider> sphere = static_pointer_cast<SphereCollider>(zombiePart->GetCollider());
				if (sphere->Intersects(_rayOrigin, _rayDir, OUT distance) == false)
					continue;
			}
			else if (colliderType == ColliderType::OBB)
			{
				shared_ptr<OrientedBoxCollider> obb = static_pointer_cast<OrientedBoxCollider>(zombiePart->GetCollider());
				if (obb->Intersects(_rayOrigin, _rayDir, OUT distance) == false)
					continue;
			}
			else
			{
				continue;
			}

			if (distance < minDistance)
			{
				// 이번 좀비가 picked된걸 확인했으면 다음 좀비로 넘어가기
				minDistance = distance;
				pickedObject = zombieGroup[0];
				break;
			}
		}
	}

	return pickedObject;
}
