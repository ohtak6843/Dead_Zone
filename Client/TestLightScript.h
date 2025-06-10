#pragma once
#include "MonoBehaviour.h"

class TestLightScript : public MonoBehaviour
{
public:
	TestLightScript();
	virtual ~TestLightScript();

	virtual void LateUpdate() override;
};

