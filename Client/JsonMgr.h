#pragma once
#include "json.hpp"

class GameObject;

using json = nlohmann::json;

class JsonMgr
{
	DECLARE_SINGLE(JsonMgr);

public:
	void to_json(json& j, const shared_ptr<GameObject>& gameObject);

	void SaveMapCollider(vector<shared_ptr<GameObject>> gameObjects, const wstring& fileName);
	void LoadMapCollider(const wstring& path);
};

