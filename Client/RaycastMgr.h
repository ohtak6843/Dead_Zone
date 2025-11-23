#pragma once
class RaycastMgr
{
	DECLARE_SINGLE(RaycastMgr);

public:
	float GetDistance() const { return _distance; }
	Vec4 GetCollisionPos() const { return _rayOrigin + _rayDir * _distance; }

public:
	void SetRayOriginAndDir(int32 screenX, int32 screenY);

	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);
	bool PickZombie(int32 screenX, int32 screenY, Vec3& hitPos, shared_ptr<class Zombie>& pickedZombie);

private:
	Vec4 _rayOrigin = {};
	Vec4 _rayDir = {};

	float _distance = 0.f;
};

