#pragma once
#include "GameObject.h"

enum class UI_TYPE
{
	DEFAULT = 0,
	PANEL,
	IMAGE,
	BUTTON,
	CROSSHAIR,
	BAR,
	ICON,
	END
};

class UIObject : public GameObject
{
public:
	UIObject();
	virtual ~UIObject() {};

	virtual void Awake() override {};
	virtual void Start() override {};
	virtual void Update() override {};
	virtual void LateUpdate() override {};
public:
	void SetSelectable(bool selectable) { _isSelectable = selectable; }
	bool IsSelectable() const { return _isSelectable; }
	void SetUIType(UI_TYPE type) { _uiType = static_cast<int32>(type); }
	int32 GetUIType() const { return _uiType; }

	bool _isSelectable = false;
	int32 _uiType = static_cast<int32>(UI_TYPE::DEFAULT);
};

