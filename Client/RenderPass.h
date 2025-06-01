#pragma once

class GameObject;
class Camera;
class Light;


class RenderPass
{
public:
	RenderPass();
	virtual ~RenderPass();

	virtual void Render();

public:
	const vector<shared_ptr<Camera>>& GetCameras() const { return _cameras; }
	const vector<shared_ptr<Light>>& GetLights() const { return _lights; }

	void AddCamera(shared_ptr<Camera> camera) { _cameras.push_back(camera); }
	void AddLight(shared_ptr<Light> light) { _lights.push_back(light); }
	
	void RemoveCamera(shared_ptr<Camera> camera);
	void RemoveLight(shared_ptr<Light> light);

	shared_ptr<Camera> GetMainCamera();
	shared_ptr<Camera> GetGunCamera();

public:
	void ClearBuffers();

	void SortGameObjects(shared_ptr<Camera> camera);
	void SortShadowGameObjects(shared_ptr<Camera> camera);

	void UploadLightData();

	void ClearRenderTargetView();
	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();
	void RenderForwardandParticle();

private:
	vector<shared_ptr<Camera>>		_cameras;
	vector<shared_ptr<Light>>		_lights;

	vector<shared_ptr<GameObject>> _vecDeferred;
	vector<shared_ptr<GameObject>> _vecForward;
	vector<shared_ptr<GameObject>> _vecParticle;
	vector<shared_ptr<GameObject>> _vecShadow;
};

