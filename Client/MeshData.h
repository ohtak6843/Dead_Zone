#pragma once
#include "Object.h"
#include "Mesh.h"
#include "Material.h"
#include "Animator.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

class Mesh;
class Material;
class GameObject;

struct MeshRenderInfo
{
	shared_ptr<Mesh>				mesh;
	vector<shared_ptr<Material>>	materials;

	Vec3							center;
	Vec3							extents;

	Vec3							position;
	Vec3							rotation;
	Vec3							scale;
};

class MeshData : public Object
{
public:
	MeshData();
	virtual ~MeshData();

public:
	static shared_ptr<MeshData> LoadFromFBX(const wstring& path);

	virtual void Load(const wstring& path);
	virtual void Save(const wstring& path);

	vector<shared_ptr<GameObject>> Instantiate(ColliderType colliderType = ColliderType::NONE);

	template<typename T>
	vector<shared_ptr<T>> InstantiateAs(ColliderType colliderType = ColliderType::NONE);

private:
	shared_ptr<Mesh>				_mesh;
	vector<shared_ptr<Material>>	_materials;

	vector<MeshRenderInfo> _meshRenders;
};

template<typename T>
inline vector<shared_ptr<T>> MeshData::InstantiateAs(ColliderType colliderType)
{
	vector<shared_ptr<T>> v;

	for (MeshRenderInfo& info : _meshRenders)
	{
		shared_ptr<T> gameObject = make_shared<T>();
		gameObject->SetTransform(make_shared<Transform>());
		gameObject->SetMeshRenderer(make_shared<MeshRenderer>());
		gameObject->GetMeshRenderer()->SetMesh(info.mesh);

		for (uint32 i = 0; i < info.materials.size(); i++)
		{
			//info.materials[i]->SetInt(0, 0);
			//gameObject->GetMeshRenderer()->SetMaterial(info.materials[i], i);
			gameObject->GetMeshRenderer()->SetMaterial(info.materials[i]->Clone(), i);
		}


		if (info.mesh->IsAnimMesh())
		{
			shared_ptr<Animator> animator = make_shared<Animator>();
			gameObject->SetAnimator(animator);
			animator->SetBones(info.mesh->GetBones());
			animator->SetAnimClip(info.mesh->GetAnimClip());
		}

#pragma region Add Collider
		switch (colliderType)
		{
		case ColliderType::NONE:
			break;
		case ColliderType::SPHERE:
		{
			shared_ptr<SphereCollider> sphere = make_shared<SphereCollider>();
			sphere->SetCenter(info.center);
			sphere->SetRadius(max(max(info.extents.x, info.extents.y), info.extents.z));
			gameObject->SetCollider(sphere);
			break;
		}
		case ColliderType::OBB:
		{
			shared_ptr<OrientedBoxCollider> obb = make_shared<OrientedBoxCollider>();
			obb->SetCenter(info.center);
			obb->SetExtents(info.extents);
			gameObject->SetCollider(obb);
			break;
		}
		}
#pragma endregion

#pragma region Set Transform
		//gameObject->GetTransform()->SetLocalPosition(info.position);
		//gameObject->GetTransform()->SetLocalRotation(info.rotation);
		//gameObject->GetTransform()->SetLocalScale(info.scale);
#pragma endregion

		v.push_back(gameObject);
	}


	return v;
}
