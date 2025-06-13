#include "pch.h"
#include "JsonMgr.h"
#include "MeshData.h"
#include "Resources.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

void JsonMgr::to_json(json& j, const shared_ptr<GameObject>& gameObject)
{
	//shared_ptr<BaseCollider> collider = gameObject->GetCollider();
	//if (collider == nullptr)
	//	return;

	//ColliderType type = collider->GetColliderType();
	//switch (type)
	//{
	//case ColliderType::SPHERE:
	//	collider = static_pointer_cast<SphereCollider>(collider);
	//	break;
	//case ColliderType::AABB:
	//	// AABB는 안 만들었음
	//	return;
	//	//break;
	//case ColliderType::OBB:
	//	collider = static_pointer_cast<OrientedBoxCollider>(collider);
	//	break;
	//case ColliderType::NONE:
	//	// NONE은 처리 안함
	//	return;
	//	//break;
	//}

	//j = json{
	//	{"Name", gameObject->GetName()},
	//	{"ColliderType", type},
	//	{"Center", {collider->GetCenter().x, collider->GetCenter().y, collider->GetCenter().z}},
	//	{"Radius", collider->GetRadius()},
	//	{"Extents", {collider->GetExtents().x, collider->GetExtents().y, collider->GetExtents().z}},
	//	{"Orientation", {collider->GetRotation().x, collider->GetRotation().y, collider->GetRotation().z}},
	//}
}

void JsonMgr::SaveMapCollider(vector<shared_ptr<GameObject>> gameObjects, const wstring& fileName)
{
	for (auto& gameObject : gameObjects)
		gameObject->Awake();

	for (auto& gameObject : gameObjects)
		gameObject->Start();

	for (auto& gameObject : gameObjects)
		gameObject->Update();

	for (auto& gameObject : gameObjects)
		gameObject->LateUpdate();

	for (auto& gameObject : gameObjects)
		gameObject->FinalUpdate();


	json j;
	ofstream file(fileName);
	if (file.is_open())
	{

	}

	// 콜라이더 저장
	for (auto& gameObject : gameObjects)
	{
		shared_ptr<BaseCollider> collider = gameObject->GetCollider();
		if (collider == nullptr)
			continue;

	}
}

void JsonMgr::LoadMapCollider(const wstring& path)
{
}
