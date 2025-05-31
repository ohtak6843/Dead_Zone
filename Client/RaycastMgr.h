#pragma once
class RaycastMgr
{
	DECLARE_SINGLE(RaycastMgr);

public:
	void SetRayOriginAndDir(int32 screenX, int32 screenY);

	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);
	shared_ptr<class GameObject> PickZombie(int32 screenX, int32 screenY);

private:
	Vec4 _rayOrigin = {};
	Vec4 _rayDir = {};

	float _distance = 0.f;
};

