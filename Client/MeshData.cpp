#include "pch.h"
#include "MeshData.h"
#include "FBXLoader.h"
#include "Mesh.h"
#include "Material.h"
#include "Resources.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Animator.h"

#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

MeshData::MeshData() : Object(OBJECT_TYPE::MESH_DATA)
{
}

MeshData::~MeshData()
{
}

shared_ptr<MeshData> MeshData::LoadFromFBX(const wstring& path)
{
	// BIN 파일 경로 구성 (.fbx → .bin)
	wstring binPath = fs::path(path).replace_extension(L".bin");

	// BIN 파일이 존재하면 바로 로드
	if (fs::exists(binPath))
	{
		shared_ptr<MeshData> meshData = make_shared<MeshData>();
		meshData->Load(binPath);
		return meshData;
	}

	// 그렇지 않으면 FBX에서 로드
	FBXLoader loader;
	loader.LoadFbx(path);

	shared_ptr<MeshData> meshData = make_shared<MeshData>();

	for (int32 i = 0; i < loader.GetMeshCount(); i++)
	{
		shared_ptr<Mesh> mesh = Mesh::CreateFromFBX(&loader.GetMesh(i), loader);
		GET_SINGLE(Resources)->Add<Mesh>(mesh->GetName(), mesh);

		vector<shared_ptr<Material>> materials;
		for (size_t j = 0; j < loader.GetMesh(i).materials.size(); j++)
		{
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(loader.GetMesh(i).materials[j].name);
			materials.push_back(material);
		}

		MeshRenderInfo info = {};
		info.mesh = mesh;
		info.materials = materials;

		// 바운딩 박스 생성
		BoundingBox boundingBox;
		FbxMeshInfo meshInfo = loader.GetMesh(i);
		BoundingBox::CreateFromPoints(
			boundingBox,
			meshInfo.vertices.size(),
			&meshInfo.vertices[0].pos,
			sizeof(Vertex));

		info.center = boundingBox.Center;
		info.extents = boundingBox.Extents;

		info.position = meshInfo.position;
		info.rotation = meshInfo.rotation;
		info.scale = meshInfo.scale;

		meshData->_meshRenders.push_back(info);
	}

	// BIN 파일로 저장
	meshData->Save(binPath);

	return meshData;
}

void MeshData::Load(const wstring& path)
{
	fs::path binPath = path;
	fs::path fbxStem = binPath.stem();

	// 상대 경로로 FBM 폴더 구성
	fs::path baseFolder = fs::path(L"..\\Resources\\FBX");
	fs::path fbmFolder = baseFolder / (fbxStem.wstring() + L".fbm");

	ifstream fin(path, ios::binary);
	if (!fin.is_open())
	{
		std::wcerr << L"[error] BIN Load Fail: " << path << std::endl;
		return;
	}

	int32 meshCount = 0;
	fin.read(reinterpret_cast<char*>(&meshCount), sizeof(int32));

	for (int i = 0; i < meshCount; ++i)
	{
		MeshRenderInfo info;

		// 1. Mesh 이름
		wstring meshName = ReadWString(fin);

		// 2. Vertex
		uint32 vertexCount;
		fin.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32));
		vector<Vertex> vertices(vertexCount);
		fin.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));

		// 3. Index Group
		uint32 groupCount;
		fin.read(reinterpret_cast<char*>(&groupCount), sizeof(uint32));
		vector<vector<uint32>> indices(groupCount);
		for (uint32 g = 0; g < groupCount; ++g)
		{
			uint32 indexCount;
			fin.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32));
			indices[g].resize(indexCount);
			fin.read(reinterpret_cast<char*>(indices[g].data()), indexCount * sizeof(uint32));
		}

		// 4. Anim 여부
		bool hasAnim = false;
		fin.read(reinterpret_cast<char*>(&hasAnim), sizeof(bool));

		vector<BoneInfo> bones;
		vector<AnimClipInfo> clips;

		if (hasAnim)
		{
			// 5. Bone 정보
			uint32 boneCount;
			fin.read(reinterpret_cast<char*>(&boneCount), sizeof(uint32));
			bones.resize(boneCount);

			for (uint32 b = 0; b < boneCount; ++b)
			{
				bones[b].boneName = ReadWString(fin);
				fin.read(reinterpret_cast<char*>(&bones[b].parentIdx), sizeof(int32));
				fin.read(reinterpret_cast<char*>(&bones[b].matOffset), sizeof(Matrix));
			}

			// 6. Animation Clip
			uint32 clipCount;
			fin.read(reinterpret_cast<char*>(&clipCount), sizeof(uint32));
			clips.resize(clipCount);

			for (uint32 c = 0; c < clipCount; ++c)
			{
				clips[c].animName = ReadWString(fin);
				fin.read(reinterpret_cast<char*>(&clips[c].duration), sizeof(double));
				fin.read(reinterpret_cast<char*>(&clips[c].frameCount), sizeof(int32));

				uint32 boneClipCount;
				fin.read(reinterpret_cast<char*>(&boneClipCount), sizeof(uint32));
				clips[c].keyFrames.resize(boneClipCount);

				for (uint32 b = 0; b < boneClipCount; ++b)
				{
					uint32 frameCount;
					fin.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32));
					clips[c].keyFrames[b].resize(frameCount);
					fin.read(reinterpret_cast<char*>(clips[c].keyFrames[b].data()), sizeof(KeyFrameInfo) * frameCount);
				}
			}
		}

		// 7. Mesh 생성
		shared_ptr<Mesh> mesh = make_shared<Mesh>();
		mesh->_rawVertices = vertices;
		mesh->_rawIndices = indices;

		for (auto& buffer : indices)
			mesh->CreateIndexBuffer(buffer);
		mesh->CreateVertexBuffer(vertices);

		mesh->SetName(meshName);

		if (hasAnim)
		{
			mesh->_bones = bones;
			mesh->_animClips = clips;
			mesh->UploadAnimation(); // UploadBuffer 등 생성
		}

		GET_SINGLE(Resources)->Add<Mesh>(meshName, mesh);
		info.mesh = mesh;

		// 8. Material 로드
		uint32 matCount;
		fin.read(reinterpret_cast<char*>(&matCount), sizeof(uint32));

		for (uint32 m = 0; m < matCount; ++m)
		{
			wstring matName = ReadWString(fin);
			shared_ptr<Material> material = make_shared<Material>();
			material->SetName(matName);
			material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"Deferred"));

			for (int t = 0; t < 3; ++t)
			{
				wstring texFileName = ReadWString(fin);
				if (!texFileName.empty())
				{
					fs::path texFullPath = fbmFolder / texFileName;
					auto texture = GET_SINGLE(Resources)->Load<Texture>(texFileName, texFullPath.wstring());
					material->SetTexture(t, texture);
				}
			}

			GET_SINGLE(Resources)->Add<Material>(matName, material);
			info.materials.push_back(material);
		}

		// 9. 기타 정보
		fin.read(reinterpret_cast<char*>(&info.center), sizeof(Vec3));
		fin.read(reinterpret_cast<char*>(&info.extents), sizeof(Vec3));
		fin.read(reinterpret_cast<char*>(&info.position), sizeof(Vec3));
		fin.read(reinterpret_cast<char*>(&info.rotation), sizeof(Vec3));
		fin.read(reinterpret_cast<char*>(&info.scale), sizeof(Vec3));

		_meshRenders.push_back(info);
	}

	fin.close();
}

void MeshData::Save(const wstring& path)
{
	ofstream fout(path, ios::binary);
	if (!fout.is_open())
		return;

	int32 meshCount = static_cast<int32>(_meshRenders.size());
	fout.write(reinterpret_cast<const char*>(&meshCount), sizeof(int32));

	for (MeshRenderInfo& info : _meshRenders)
	{
		// 1. Mesh 이름
		WriteWString(fout, info.mesh->GetName());

		// 2. Vertex & Index 저장
		const vector<Vertex>& vertices = info.mesh->_rawVertices;
		const vector<vector<uint32>>& indices = info.mesh->_rawIndices;
		uint32 vertexCount = static_cast<uint32>(vertices.size());
		fout.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32));
		fout.write(reinterpret_cast<const char*>(vertices.data()), vertexCount * sizeof(Vertex));

		uint32 groupCount = static_cast<uint32>(indices.size());
		fout.write(reinterpret_cast<const char*>(&groupCount), sizeof(uint32));
		for (auto& group : indices)
		{
			uint32 indexCount = static_cast<uint32>(group.size());
			fout.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32));
			fout.write(reinterpret_cast<const char*>(group.data()), indexCount * sizeof(uint32));
		}

		// 3. 애니메이션 여부
		bool hasAnim = info.mesh->IsAnimMesh();
		fout.write(reinterpret_cast<const char*>(&hasAnim), sizeof(bool));

		// 4. Bone 정보
		if (hasAnim)
		{
			const auto& bones = *info.mesh->GetBones();
			uint32 boneCount = static_cast<uint32>(bones.size());
			fout.write(reinterpret_cast<const char*>(&boneCount), sizeof(uint32));
			for (auto& bone : bones)
			{
				WriteWString(fout, bone.boneName);
				fout.write(reinterpret_cast<const char*>(&bone.parentIdx), sizeof(int32));
				fout.write(reinterpret_cast<const char*>(&bone.matOffset), sizeof(Matrix));
			}

			// 5. Animation Clip 정보
			const auto& clips = *info.mesh->GetAnimClip();
			uint32 clipCount = static_cast<uint32>(clips.size());
			fout.write(reinterpret_cast<const char*>(&clipCount), sizeof(uint32));

			for (auto& clip : clips)
			{
				WriteWString(fout, clip.animName);
				fout.write(reinterpret_cast<const char*>(&clip.duration), sizeof(double));
				fout.write(reinterpret_cast<const char*>(&clip.frameCount), sizeof(int32));

				uint32 boneClipCount = static_cast<uint32>(clip.keyFrames.size());
				fout.write(reinterpret_cast<const char*>(&boneClipCount), sizeof(uint32));
				for (auto& keyFrames : clip.keyFrames)
				{
					uint32 frameCount = static_cast<uint32>(keyFrames.size());
					fout.write(reinterpret_cast<const char*>(&frameCount), sizeof(uint32));
					fout.write(reinterpret_cast<const char*>(keyFrames.data()), sizeof(KeyFrameInfo) * frameCount);
				}
			}
		}

		// 6. Material 정보
		uint32 matCount = static_cast<uint32>(info.materials.size());
		fout.write(reinterpret_cast<const char*>(&matCount), sizeof(uint32));
		for (auto& mat : info.materials)
		{
			WriteWString(fout, mat->GetName());

			for (int t = 0; t < 3; ++t)
			{
				shared_ptr<Texture> tex = mat->GetTexture(t);
				wstring texPath = tex ? tex->GetName() : L"";
				WriteWString(fout, texPath);
			}
		}

		// 7. 바운딩/트랜스폼 정보
		fout.write(reinterpret_cast<const char*>(&info.center), sizeof(Vec3));
		fout.write(reinterpret_cast<const char*>(&info.extents), sizeof(Vec3));
		fout.write(reinterpret_cast<const char*>(&info.position), sizeof(Vec3));
		fout.write(reinterpret_cast<const char*>(&info.rotation), sizeof(Vec3));
		fout.write(reinterpret_cast<const char*>(&info.scale), sizeof(Vec3));
	}

	fout.close();
}

vector<shared_ptr<GameObject>> MeshData::Instantiate(ColliderType colliderType)
{
	vector<shared_ptr<GameObject>> v;

	for (MeshRenderInfo& info : _meshRenders)
	{
		shared_ptr<GameObject> gameObject = make_shared<GameObject>();
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

