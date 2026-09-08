#include "Culling.h"
#include "Vector3.h"

// 三角形(a,b,c)が裏面かどうかを判定する
// 頂点の巻き順(a->b->c)から法線を求め(edge1×edge2)、
// 視線方向(eye-a)との内積が負であれば、カメラと反対方向を向いている＝裏面と判定する
bool IsBackFace(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& eye)
{
    Vector3 edge1 = b.sub(a);
    Vector3 edge2 = c.sub(a);
    Vector3 normal = edge1.crosspro(edge2);

    Vector3 viewDir = eye.sub(a);

    return normal.dotpro(viewDir) < 0.0f;
}