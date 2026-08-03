#include "Transform.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
using namespace std;

Matrix4x4 Transform::world() const {
    Matrix4x4 baseM;
    baseM = baseM.identity();

    Matrix4x4 matScale = baseM.scale(scale.x, scale.y, scale.z);
    Matrix4x4 matRot = rota.convert();
    Matrix4x4 matTrans = baseM.translate(posi.x, posi.y, posi.z);

    return matScale.ply(matRot).ply(matTrans);
}