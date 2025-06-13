#pragma once
#include "Mesh.h"

class GameObject;
class Material;
class UploadBuffer;
class Mesh;

class Animator
{
public:
	Animator();
	virtual ~Animator();

public:
	void SetBones(const vector<BoneInfo>* bones) { _bones = bones; }
	void SetAnimClip(const vector<AnimClipInfo>* animClips);
	void PushData();

	int32 GetAnimCount() { return static_cast<uint32>(_animClips->size()); }
	int32 GetCurrentClipIndex() { return _clipIndex; }

	const Matrix& GetBoneMatrix(int32 idx);

	void Play(uint32 idx);

	float GetUpdateTime() const { return _updateTime; }
	double GetAnimDuration(uint32 idx)
	{
		assert(idx < _animClips->size());
		return _animClips->at(idx).duration;
	}

public:
	void FinalUpdate();

public:
	shared_ptr<GameObject> GetGameObject() { return _gameObject.lock(); }

private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
	weak_ptr<GameObject> _gameObject;

private:
	const vector<BoneInfo>* _bones;
	const vector<AnimClipInfo>* _animClips;

	float							_updateTime = 0.f;
	int32							_clipIndex = 0;
	int32							_frame = 0;
	int32							_nextFrame = 0;
	float							_frameRatio = 0;

	bool							_blendAnimation = false;
	float							_blendUpdateTime = 0.f;
	int32							_blendClipIndex = 0;
	int32							_blendFrame = 0;

	shared_ptr<Material>			_computeMaterial;
	shared_ptr<UploadBuffer>		_boneFinalMatrix;  // 특정 프레임의 최종 행렬
	bool							_boneFinalUpdated = false;
};
