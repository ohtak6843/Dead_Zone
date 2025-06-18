#include "pch.h"
#include "JsonMgr.h"
#include "MeshData.h"
#include "Resources.h"

#include "MeshRenderer.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

void to_json(ordered_json& j, const shared_ptr<GameObject>& gameObject)
{
	shared_ptr<BaseCollider> collider = gameObject->GetCollider();
	if (collider == nullptr)
		return;

	ColliderType type = collider->GetColliderType();
	if (type != ColliderType::OBB)
		return;

	shared_ptr<BoundingOrientedBox> obbCollider = static_pointer_cast<OrientedBoxCollider>(collider)->GetBoundingOrientedBox();

	wstring name = gameObject->GetMeshRenderer()->GetMesh()->GetName();
	string nameStr = ws2s(name);
	nameStr.erase(std::remove(nameStr.begin(), nameStr.end(), '\0'), nameStr.end());

	j = ordered_json{
		{"name", nameStr},
		{"position", {obbCollider->Center.x, obbCollider->Center.y, obbCollider->Center.z}},
		{"orientation", {obbCollider->Orientation.x, obbCollider->Orientation.y, obbCollider->Orientation.z, obbCollider->Orientation.w}},
		{"extents", {obbCollider->Extents.x, obbCollider->Extents.y, obbCollider->Extents.z}},
	};
}

void JsonMgr::SaveMapCollider(const wstring& fileName, vector<shared_ptr<GameObject>> gameObjects)
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


	ordered_json j = gameObjects;
	ofstream file(fileName);
	if (file.is_open())
	{
		file << j.dump(4);
		file.close();
	}
}

void JsonMgr::LoadMapCollider(const wstring& path)
{
}
