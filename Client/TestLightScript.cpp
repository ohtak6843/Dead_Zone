#include "pch.h"
#include "TestLightScript.h"
#include "InputMgr.h"
#include "framework.h"
#include "Transform.h"
#include "Timer.h"

TestLightScript::TestLightScript()
{
}

TestLightScript::~TestLightScript()
{
}

void TestLightScript::LateUpdate()
{
	if (INPUT->GetButton(KEY_TYPE::KEY_5))
	{
		Vec3 pos = GetTransform()->GetLocalPosition();
		pos.y += 300.f * DELTA_TIME;
		GetTransform()->SetLocalPosition(pos);
	}

	if (INPUT->GetButton(KEY_TYPE::KEY_6))
	{
		Vec3 pos = GetTransform()->GetLocalPosition();
		pos.y -= 300.f * DELTA_TIME;
		GetTransform()->SetLocalPosition(pos);
	}
}
