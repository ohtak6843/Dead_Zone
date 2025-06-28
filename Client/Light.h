#pragma once
#include "GameObject.h"

class GameObject;
class Mesh;
class Material;

class CameraObject;

enum class LIGHT_TYPE : uint8
{
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT,
};

struct LightColor
{
	Vec4	diffuse;
	Vec4	ambient;
	Vec4	specular;
};

struct LightInfo
{
	LightColor	color;
	Vec4		position;
	Vec4		direction;
	int32		lightType;
	float		range;
	float		angle;
	int32		padding;
};

struct LightParams
{
	uint32		lightCount;
	Vec3		padding;
	LightInfo	lights[50];
};

class Light
{
public:
	Light();
	virtual ~Light();

public:
	void FinalUpdate();

	void Render();
	void RenderShadow();

public:
	LIGHT_TYPE GetLightType() { return static_cast<LIGHT_TYPE>(_lightInfo.lightType); }

	const LightInfo& GetLightInfo() { return _lightInfo; }

	void SetLightDirection(Vec3 direction);

	void SetDiffuse(const Vec3& diffuse) { _lightInfo.color.diffuse = diffuse; }
	void SetAmbient(const Vec3& ambient) { _lightInfo.color.ambient = ambient; }
	void SetSpecular(const Vec3& specular) { _lightInfo.color.specular = specular; }

	void SetLightType(LIGHT_TYPE type);
	void SetLightRange(float range) { _lightInfo.range = range; }
	void SetLightAngle(float angle) { _lightInfo.angle = angle; }

	int8 GetLightIndex() { return _lightIndex; }
	void SetLightIndex(int8 index) { _lightIndex = index; }

	shared_ptr<Mesh> GetVolumeMesh() { return _volumeMesh; }
	shared_ptr<Material> GetLightMaterial() { return _lightMaterial; }

	shared_ptr<CameraObject> GetShadowCamera() { return _shadowCamera; }

public:
	shared_ptr<GameObject> GetGameObject() { return _gameObject.lock(); }
	shared_ptr<Transform> GetTransform() { return _gameObject.lock()->GetTransform(); }

private:
	friend class LightObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
	weak_ptr<GameObject> _gameObject;

private:
	LightInfo _lightInfo = {};

	int8 _lightIndex = -1;
	shared_ptr<Mesh> _volumeMesh;
	shared_ptr<Material> _lightMaterial;

	shared_ptr<CameraObject> _shadowCamera;
};

