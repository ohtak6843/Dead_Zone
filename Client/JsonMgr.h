#pragma once
#include "json.hpp"
#include "BaseCollider.h"
#include "OrientedBoxCollider.h"

class GameObject;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

class JsonMgr
{
	DECLARE_SINGLE(JsonMgr);

public:

	void SaveMapCollider(const wstring& fileName, vector<shared_ptr<GameObject>> gameObjects);
	void LoadMapCollider(const wstring& path);
};

