#include "pch.h"
#include "Camera.h"
#include "Transform.h"
#include "Scene.h"
#include "SceneMgr.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Framework.h"
#include "Material.h"
#include "Shader.h"
#include "Particle.h"
#include "InstancingMgr.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

Matrix Camera::S_MatView;
Matrix Camera::S_MatProjection;

Camera::Camera()
{
	_width = static_cast<float>(gameFramework->GetWindow().width);
	_height = static_cast<float>(gameFramework->GetWindow().height);
}

Camera::~Camera()
{
}

void Camera::FinalUpdate()
{
	_matView = GetTransform()->GetLocalToWorldMatrix().Invert();

	if (_type == PROJECTION_TYPE::PERSPECTIVE)
		_matProjection = ::XMMatrixPerspectiveFovLH(XMConvertToRadians(_fov), _width / _height, _near, _far);
	else
		_matProjection = ::XMMatrixOrthographicLH(_width * _scale, _height * _scale, _near, _far);

	Camera::S_MatView = _matView;
	Camera::S_MatProjection = _matProjection;

	// 절두체 위치 업데이트
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, Camera::S_MatProjection);
	Matrix viewInv = Camera::S_MatView.Invert();
	frustum.Transform(frustum, viewInv);
	_frustum = frustum;
}

ContainmentType Camera::FrustumCulling(shared_ptr<GameObject> gameObject)
{
	if (gameObject->GetCollider() == nullptr)
		return ContainmentType::CONTAINS;

	ColliderType colliderType = gameObject->GetCollider()->GetColliderType();

	if (colliderType == ColliderType::SPHERE)
	{
		shared_ptr<SphereCollider> sphere = static_pointer_cast<SphereCollider>(gameObject->GetCollider());
		return _frustum.Contains(*sphere->GetBoundingSphere());
	}
	else if (colliderType == ColliderType::OBB)
	{
		shared_ptr<OrientedBoxCollider> obb = static_pointer_cast<OrientedBoxCollider>(gameObject->GetCollider());
		return _frustum.Contains(*obb->GetBoundingOrientedBox());
	}
	else
	{
		return ContainmentType::CONTAINS;
	}
}
