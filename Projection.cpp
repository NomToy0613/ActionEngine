#include "Projection.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
using namespace std;

Matrix4x4 Projection::projection() const {
	Matrix4x4 baseM;
	baseM = baseM.identity();
	return baseM.perspe(this->fov, this->aspect, this->nearZ, this->farZ);
}