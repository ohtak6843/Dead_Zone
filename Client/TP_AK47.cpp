#include "pch.h"
#include "TP_AK47.h"
#include "GameObject.h"
#include "Transform.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Animator.h"

TP_AK47::TP_AK47()
{
	_baseRotation.x = DegreeToRadian(_baseRotation.x);
	_baseRotation.y = DegreeToRadian(_baseRotation.y);
	_baseRotation.z = DegreeToRadian(_baseRotation.z);
}

TP_AK47::~TP_AK47()
{
}

void TP_AK47::LateUpdate()
{
	int32 rightHandBoneIndex = _parentObject->GetMeshRenderer()->GetMesh()->GetRightHandBoneIndex();
	Matrix rightHandBoneMatrix = _parentObject->GetAnimator()->GetBoneMatrix(rightHandBoneIndex);

	// Base 행렬 계산
	Matrix matScale = Matrix::CreateScale(_baseScale);

	SimpleMath::Quaternion q;

	float sp = sinf(_baseRotation.x * 0.5f);
	float cp = cosf(_baseRotation.x * 0.5f);

	float sy = sinf(_baseRotation.y * 0.5f);
	float cy = cosf(_baseRotation.y * 0.5f);

	float sr = sinf(_baseRotation.z * 0.5f);
	float cr = cosf(_baseRotation.z * 0.5f);

	q.w = cy * cp * cr + sy * sp * sr;
	q.x = cy * sp * cr + sy * cp * sr;
	q.y = sy * cp * cr - cy * sp * sr;
	q.z = cy * cp * sr - sy * sp * cr;

	Matrix matRotation = Matrix::CreateFromQuaternion(q);
	Matrix matTranslation = Matrix::CreateTranslation(_basePosition);

	Matrix matLocal = matScale * matRotation * matTranslation;

	Matrix matFinal = matLocal * rightHandBoneMatrix;

	Vec3 scale{};
	Vec3 rotation{};
	Vec3 translation{};
	SimpleMath::Quaternion orientation{};

	matFinal.Decompose(scale, orientation, translation);

	rotation = Transform::QuaternionToEuler(orientation);
	rotation.x = RadianToDegree(rotation.x);
	rotation.y = RadianToDegree(rotation.y);
	rotation.z = RadianToDegree(rotation.z);

	GetTransform()->SetLocalScale(scale);
	GetTransform()->SetLocalRotation(rotation);
	GetTransform()->SetLocalPosition(translation);
}
