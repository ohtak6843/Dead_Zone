#pragma once

class Scene;

enum SCENE_TYPE
{
	TITLE,
	LOADING,
	STAGE01,
	STAGE02,
	END,
};

enum
{
	MAX_LAYER = 32
};

class SceneMgr
{
	DECLARE_SINGLE(SceneMgr);

public:
	void Update();
	void Render();
	
	void LoadScene(SCENE_TYPE type);
	void SwitchScene(SCENE_TYPE type);

	void SetLayerName(uint8 index, const wstring& name);
	const wstring& IndexToLayerName(uint8 index) { return _layerNames[index]; }
	uint8 LayerNameToIndex(const wstring& name);

	SCENE_TYPE GetSceneType() { return _sceneType; }
	void SetSceneType(SCENE_TYPE type) { _sceneType = type; }

	bool IsDebugMode() { return _debugMode; }
	void SetDebugMode(bool debugMode) { _debugMode = debugMode; }

	bool IsFullScreen() { return _fullScreen; }
	void SetFullScreen(bool flag) { _fullScreen = flag; }

	bool GetChangeScene() { return _changeScene; }
	void SetChangeScene(bool changeScene) { _changeScene = changeScene; }

	SCENE_TYPE GetNextSceneType() { return _nextSceneType; }
	void SetNextSceneType(SCENE_TYPE type) { _nextSceneType = type; }

public:
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }

private:
	shared_ptr<Scene> LoadLoadingScene();
	shared_ptr<Scene> LoadStage01();

private:
	SCENE_TYPE _sceneType = SCENE_TYPE::LOADING;

	shared_ptr<Scene> _activeScene;

	array<wstring, MAX_LAYER> _layerNames;
	map<wstring, uint8> _layerIndex;

	bool _debugMode = false;
	bool _fullScreen = false;

	bool _changeScene = false;
	SCENE_TYPE _nextSceneType;
};

