#pragma once
#include "Component.h"

class MonoBehaviour : public Component
{
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();

public:
	virtual void FinalUpdate() sealed { }

public:
	const wstring& GetName() { return _name; }

protected:
	wstring _name;
};

