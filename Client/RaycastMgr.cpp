#include "pch.h"
#include "RaycastMgr.h"
#include "SceneMgr.h"
#include "Camera.h"
#include "Framework.h"
#include "Scene.h"
#include "GameObject.h"
#include "Zombie.h"

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

bool RaycastMgr::PickZombie(int32 screenX, int32 screenY, Vec3& hitPos, shared_ptr<Zombie>& pickedZombie)
{
	SetRayOriginAndDir(screenX, screenY);

	auto& zombies = GET_SINGLE(SceneMgr)->GetActiveScene()->GetZombies();

	float minDistance = FLT_MAX;
	bool hit = false;
	Vec3 closestHitPos;
	shared_ptr<Zombie> closestZombie = nullptr;

	for (auto& zombieGroup : zombies)
	{
		for (auto& zombiePart : zombieGroup)
		{
			if (zombiePart->GetCollider() == nullptr)
				continue;

			float distance = 0.f;
			bool intersected = false;

			auto colliderType = zombiePart->GetCollider()->GetColliderType();
			if (colliderType == ColliderType::SPHERE)
			{
				auto sphere = static_pointer_cast<SphereCollider>(zombiePart->GetCollider());
				intersected = sphere->Intersects(_rayOrigin, _rayDir, OUT distance);
			}
			else if (colliderType == ColliderType::OBB)
			{
				auto obb = static_pointer_cast<OrientedBoxCollider>(zombiePart->GetCollider());
				intersected = obb->Intersects(_rayOrigin, _rayDir, OUT distance);
			}

			if (!intersected)
				continue;

			if (distance < minDistance)
			{
				minDistance = distance;
				Vec3 origin3 = Vec3(_rayOrigin.x, _rayOrigin.y, _rayOrigin.z);
				Vec3 dir3 = Vec3(_rayDir.x, _rayDir.y, _rayDir.z);
				closestHitPos = origin3 + dir3 * distance;
				closestZombie = zombieGroup[0]; // 대표 좀비 저장
				hit = true;
				break; // 하나의 파트라도 맞으면 그 좀비 그룹은 더 안 검사
			}
		}
	}

	if (hit)
	{
		hitPos = closestHitPos;
		pickedZombie = closestZombie;
		return true;
	}

	return false;
}


//bool RaycastMgr::PickZombie(int32 screenX, int32 screenY, Vec3& hitPos, shared_ptr<GameObject>& pickedZombie)
//{
//	shared_ptr<Camera> camera = GetActiveScene()->GetMainCamera();
//
//	float width = static_cast<float>(GEngine->GetWindow().width);
//	float height = static_cast<float>(GEngine->GetWindow().height);
//
//	Matrix projectionMatrix = camera->GetProjectionMatrix();
//
//	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
//	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);
//
//	Matrix viewMatrix = camera->GetViewMatrix();
//	Matrix viewMatrixInv = viewMatrix.Invert();
//
//	auto& zombies = GET_SINGLE(SceneManager)->GetActiveScene()->GetZombies();
//
//	// Ray 정의 (world space)
//	Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
//	Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);
//
//	rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
//	rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
//	rayDir.Normalize();
//
//	float minDistance = FLT_MAX;
//	bool hit = false;
//	Vec3 closestHitPos;
//	shared_ptr<GameObject> closestZombie = nullptr;
//
//	for (auto& zombieGroup : zombies)
//	{
//		for (auto& zombiePart : zombieGroup)
//		{
//			if (zombiePart->GetCollider() == nullptr)
//				continue;
//
//			float distance = 0.f;
//			bool intersected = false;
//
//			auto colliderType = zombiePart->GetCollider()->GetColliderType();
//			if (colliderType == ColliderType::SPHERE)
//			{
//				auto sphere = static_pointer_cast<SphereCollider>(zombiePart->GetCollider());
//				intersected = sphere->Intersects(rayOrigin, rayDir, OUT distance);
//			}
//			else if (colliderType == ColliderType::OBB)
//			{
//				auto obb = static_pointer_cast<OrientedBoxCollider>(zombiePart->GetCollider());
//				intersected = obb->Intersects(rayOrigin, rayDir, OUT distance);
//			}
//
//			if (!intersected)
//				continue;
//
//			if (distance < minDistance)
//			{
//				minDistance = distance;
//				Vec3 origin3 = Vec3(rayOrigin.x, rayOrigin.y, rayOrigin.z);
//				Vec3 dir3 = Vec3(rayDir.x, rayDir.y, rayDir.z);
//				closestHitPos = origin3 + dir3 * distance;
//				closestZombie = zombieGroup[0]; // 대표 좀비 저장
//				hit = true;
//				break; // 하나의 파트라도 맞으면 그 좀비 그룹은 더 안 검사
//			}
//		}
//	}
//
//	if (hit)
//	{
//		hitPos = closestHitPos;
//		pickedZombie = closestZombie;
//		return true;
//	}
//
//	return false;
//}