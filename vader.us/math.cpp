#include "includes.h"

#define M_PI		(float)3.14159265358979323846f
#define M_RADPI		57.295779513082f
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI) )
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI / 180.f) )

#define Assert( _exp ) ((void)0)

void math::AngleMatrix(const ang_t& ang, const vec3_t& pos, matrix3x4_t& out) {
    g_csgo.AngleMatrix(ang, out);
    out.SetOrigin(pos);
}

float math::random_float(float min, float max) {
    typedef float(*RandomFloat_t)(float, float);
    static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(XOR("vstdlib.dll")), XOR("RandomFloat"));
    return m_RandomFloat(min, max);
}

int math::random_int(int min, int max) {
    typedef int(*RandomInt_t)(int, int);
    static RandomInt_t m_RandomInt = (RandomInt_t)GetProcAddress(GetModuleHandle(XOR("vstdlib.dll")), XOR("RandomInt"));
    return m_RandomInt(min, max);
}


void math::random_seed(int seed) {
    typedef void(*RandomSeed_t)(int);
    static RandomSeed_t m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandle(XOR("vstdlib.dll")), XOR("RandomSeed"));
    return m_RandomSeed(seed);
}

void math::VectorAngles3(const vec3_t& forward, vec3_t& up, ang_t& angles)
{
    vec3_t left = left.CrossProduct(up, forward);
    left.normalize();

    float forwardDist = forward.length_2d();

    if (forwardDist > 0.001f)
    {
        angles.x = atan2f(-forward.z, forwardDist) * 180 / M_PI;
        angles.y = atan2f(forward.y, forward.x) * 180 / M_PI;

        float upZ = (left.y * forward.x) - (left.x * forward.y);
        angles.z = atan2f(left.z, upZ) * 180 / M_PI;
    }
    else
    {
        angles.x = atan2f(-forward.z, forwardDist) * 180 / M_PI;
        angles.y = atan2f(-left.x, left.y) * 180 / M_PI;
        angles.z = 0;
    }
}
void math::AngleVectors69(const ang_t& angles, vec3_t& forward)
{
    float	sp, sy, cp, cy;

    sy = sin(math::deg_to_rad(angles[1]));
    cy = cos(math::deg_to_rad(angles[1]));

    sp = sin(math::deg_to_rad(angles[0]));
    cp = cos(math::deg_to_rad(angles[0]));

    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
}

void math::NormalizeAngle(float& angle) {
    float rot;

    // bad number.
    if (!std::isfinite(angle)) {
        angle = 0.f;
        return;
    }

    // no need to normalize this angle.
    if (angle >= -180.f && angle <= 180.f)
        return;

    // get amount of rotations needed.
    rot = std::round(std::abs(angle / 360.f));

    // normalize.
    angle = (angle < 0.f) ? angle + (360.f * rot) : angle - (360.f * rot);
}

float math::NormalizeYaw(float angle) {
    if (!std::isfinite(angle))
        angle = 0.f;

    return std::remainderf(angle, 360.0f);
}

vec3_t math::CalcAngle(const vec3_t& vecSource, const vec3_t& vecDestination) {
    vec3_t vAngle;
    vec3_t delta((vecSource.x - vecDestination.x), (vecSource.y - vecDestination.y), (vecSource.z - vecDestination.z));
    double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

    vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
    vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
    vAngle.z = 0.0f;

    if (delta.x >= 0.0)
        vAngle.y += 180.0f;

    return vAngle;
}

#define M_RADPI 57.295779513082f
void math::CalcAngle3(const vec3_t src, const vec3_t dst, ang_t& angles) {
    auto delta = src - dst;
    vec3_t vec_zero = vec3_t(0.0f, 0.0f, 0.0f);
    ang_t ang_zero = ang_t(0.0f, 0.0f, 0.0f);


    if (delta == vec_zero)
        angles = ang_zero;

    const auto len = delta.length();

    if (delta.z == 0.0f && len == 0.0f)
        angles = ang_zero;

    if (delta.y == 0.0f && delta.x == 0.0f)
        angles = ang_zero;


#ifdef QUICK_MATH
    angles.x = (fast_asin(delta.z / delta.Length()) * M_RADPI);
    angles.y = (fast_atan(delta.y / delta.x) * M_RADPI);
#else
    angles.x = (asinf(delta.z / delta.length()) * M_RADPI);
    angles.y = (atanf(delta.y / delta.x) * M_RADPI);
#endif

    angles.z = 0.0f;
    if (delta.x >= 0.0f) { angles.y += 180.0f; }

    angles.clamp();
}

int math::minimum(int array[], int size)
{
    int i;
    int min = array[0];
    for (i = 1; i < size; i++)
    {
        if (array[i] < min)
            min = array[i];
    }
    return min;
}

float math::ApproachAngle(float target, float value, float speed) {
    float delta;

    target = AngleMod(target);
    value = AngleMod(value);
    delta = target - value;

    // speed is assumed to be positive.
    speed = std::abs(speed);

    math::NormalizeAngle(delta);

    if (delta > speed)
        value += speed;

    else if (delta < -speed)
        value -= speed;

    else
        value = target;

    return value;
}

void math::VectorAngles(const vec3_t& forward, ang_t& angles, vec3_t* up) {
    vec3_t  left;
    float   len, up_z, pitch, yaw, roll;

    // get 2d length.
    len = forward.length_2d();

    if (up && len > 0.001f) {
        pitch = rad_to_deg(std::atan2(-forward.z, len));
        yaw = rad_to_deg(std::atan2(forward.y, forward.x));

        // get left direction vector using cross product.
        left = (*up).cross(forward).normalized();

        // calculate up_z.
        up_z = (left.y * forward.x) - (left.x * forward.y);

        // calculate roll.
        roll = rad_to_deg(std::atan2(left.z, up_z));
    }

    else {
        if (len > 0.f) {
            // calculate pitch and yaw.
            pitch = rad_to_deg(std::atan2(-forward.z, len));
            yaw = rad_to_deg(std::atan2(forward.y, forward.x));
            roll = 0.f;
        }

        else {
            pitch = (forward.z > 0.f) ? -90.f : 90.f;
            yaw = 0.f;
            roll = 0.f;
        }
    }

    // set out angles.
    angles = { pitch, yaw, roll };
}

void math::NormalizeVector(vec3_t& vec)
{
    for (int i = 0; i < 2; ++i)
    {
        while (vec[i] > 180.f)
            vec[i] -= 360.f;

        while (vec[i] < -180.f)
            vec[i] += 360.f;
    }

    vec[2] = 0.f;
}

void math::AngleVectors(const ang_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up) {
    float cp = std::cos(deg_to_rad(angles.x)), sp = std::sin(deg_to_rad(angles.x));
    float cy = std::cos(deg_to_rad(angles.y)), sy = std::sin(deg_to_rad(angles.y));
    float cr = std::cos(deg_to_rad(angles.z)), sr = std::sin(deg_to_rad(angles.z));

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right) {
        right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
        right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
        right->z = -1.f * sr * cp;
    }

    if (up) {
        up->x = cr * sp * cy + -sr * -sy;
        up->y = cr * sp * sy + -sr * cy;
        up->z = cr * cp;
    }
}

void inline math::SinCos(float radians, float* sine, float* cosine)
{
    _asm
    {
        fld		DWORD PTR[radians]
        fsincos

        mov edx, DWORD PTR[cosine]
        mov eax, DWORD PTR[sine]

        fstp DWORD PTR[edx]
        fstp DWORD PTR[eax]
    }
}

void math::AngleVectorKidua(ang_t& vAngle, vec3_t& vForward)
{
    float sp, sy, cp, cy;
    SinCos(vAngle[1] * math::pi / 180.f, &sy, &cy);
    SinCos(vAngle[0] * math::pi / 180.f, &sp, &cp);

    vForward[0] = cp * cy;
    vForward[1] = cp * sy;
    vForward[2] = -sp;
}

float math::GetFOV(const ang_t& view_angles, const vec3_t& start, const vec3_t& end) {
    vec3_t dir, fw;

    // get direction and normalize.
    dir = (end - start).normalized();

    // get the forward direction vector of the view angles.
    AngleVectors(view_angles, &fw);

    // get the angle between the view angles forward directional vector and the target location.
    return std::max(rad_to_deg(std::acos(fw.dot(dir))), 0.f);
}

void math::VectorTransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out) {
    out = {
        in.dot(vec3_t(matrix[0][0], matrix[0][1], matrix[0][2])) + matrix[0][3],
        in.dot(vec3_t(matrix[1][0], matrix[1][1], matrix[1][2])) + matrix[1][3],
        in.dot(vec3_t(matrix[2][0], matrix[2][1], matrix[2][2])) + matrix[2][3]
    };
}

bool math::IntersectLineWithBB(vec3_t& vStart, vec3_t& vEndDelta, vec3_t& vMin, vec3_t& vMax) {
    float d1, d2, f;
    bool startsolid = true;
    float t1 = -1.0f, t2 = 1.0f;

    for (int i = 0; i < 6; i++) {
        if (i >= 3) {
            int j = i - 3;

            d1 = vStart[j] - vMax[j];
            d2 = d1 + vEndDelta[j];
        }
        else {
            d1 = -vStart[i] + vMin[i];
            d2 = d1 - vEndDelta[i];
        }

        if (d1 > 0.0f && d2 > 0.0f)
            return false;
        else if (d1 <= 0.0f && d2 <= 0.0f)
            continue;

        if (d1 > 0)
            startsolid = false;

        if (d1 > d2) {
            f = d1;
            if (f < 0.0f)
                f = 0.0f;

            f /= d1 - d2;
            if (f > t1)
                t1 = f;
        }
        else {
            f = d1 / (d1 - d2);
            if (f < t2)
                t2 = f;
        }
    }

    return (startsolid || (t1 < t2&& t1 >= 0.0f));
}

float math::SegmentToSegment(const vec3_t s1, const vec3_t s2, const vec3_t k1, const vec3_t k2)
{
    static auto constexpr epsilon = 0.00000001;

    auto u = s2 - s1;
    auto v = k2 - k1;
    const auto w = s1 - k1;

    const auto a = u.dot(u);
    const auto b = u.dot(v);
    const auto c = v.dot(v);
    const auto d = u.dot(w);
    const auto e = v.dot(w);
    const auto D = a * c - b * b;
    float sn, sd = D;
    float tn, td = D;

    if (D < epsilon) {
        sn = 0.0;
        sd = 1.0;
        tn = e;
        td = c;
    }
    else {
        sn = b * e - c * d;
        tn = a * e - b * d;

        if (sn < 0.0) {
            sn = 0.0;
            tn = e;
            td = c;
        }
        else if (sn > sd) {
            sn = sd;
            tn = e + b;
            td = c;
        }
    }

    if (tn < 0.0) {
        tn = 0.0;

        if (-d < 0.0)
            sn = 0.0;
        else if (-d > a)
            sn = sd;
        else {
            sn = -d;
            sd = a;
        }
    }
    else if (tn > td) {
        tn = td;

        if (-d + b < 0.0)
            sn = 0;
        else if (-d + b > a)
            sn = sd;
        else {
            sn = -d + b;
            sd = a;
        }
    }

    const float sc = abs(sn) < epsilon ? 0.0 : sn / sd;
    const float tc = abs(tn) < epsilon ? 0.0 : tn / td;

    m128 n;
    auto dp = w + u * sc - v * tc;
    n.f[0] = dp.dot(dp);
    const auto calc = sqrt_ps(n.v);
    return reinterpret_cast<const m128*>(&calc)->f[0];

}

void math::matrix_set_column(const vec3_t& in, int column, matrix3x4_t& out)
{
    out[0][column] = in.x;
    out[1][column] = in.y;
    out[2][column] = in.z;
}

void math::angle_matrix(const ang_t& angles, const vec3_t& position, matrix3x4_t& matrix)
{
    angle_matrix(angles, matrix);
    matrix_set_column(position, 3, matrix);
}

void math::angle_matrix(const ang_t& angles, matrix3x4_t& matrix)
{
    float sr, sp, sy, cr, cp, cy;

    SinCos(DEG2RAD(angles.x), &sp, &cp);
    SinCos(DEG2RAD(angles.y), &sy, &cy);
    SinCos(DEG2RAD(angles.z), &sr, &cr);

    // matrix = (YAW * PITCH) * ROLL
    matrix[0][0] = cp * cy;
    matrix[1][0] = cp * sy;
    matrix[2][0] = -sp;

    const auto crcy = cr * cy;
    const auto crsy = cr * sy;
    const auto srcy = sr * cy;
    const auto srsy = sr * sy;
    matrix[0][1] = sp * srcy - crsy;
    matrix[1][1] = sp * srsy + crcy;
    matrix[2][1] = sr * cp;

    matrix[0][2] = (sp * crcy + srsy);
    matrix[1][2] = (sp * crsy - srcy);
    matrix[2][2] = cr * cp;

    matrix[0][3] = 0.0f;
    matrix[1][3] = 0.0f;
    matrix[2][3] = 0.0f;
}

vec3_t math::vector_rotate(const vec3_t& in1, const matrix3x4_t& in2)
{
    return vec3_t(in1.Dot(in2[0]), in1.Dot(in2[1]), in1.Dot(in2[2]));
}

vec3_t math::vector_rotate(const vec3_t& in1, const ang_t& in2)
{
    matrix3x4_t mat;
    math::angle_matrix(in2, mat);

    //const auto matrix = angle_matrix(in2);
    return vector_rotate(in1, mat);
}

void math::VectorITransform(const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out) {
    vec3_t diff;

    diff = {
        in.x - matrix[0][3],
        in.y - matrix[1][3],
        in.z - matrix[2][3]
    };

    out = {
        diff.x * matrix[0][0] + diff.y * matrix[1][0] + diff.z * matrix[2][0],
        diff.x * matrix[0][1] + diff.y * matrix[1][1] + diff.z * matrix[2][1],
        diff.x * matrix[0][2] + diff.y * matrix[1][2] + diff.z * matrix[2][2]
    };
}

void math::vector_i_rotate(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out)
{
    Assert(s_bMathlibInitialized);
    Assert(in1 != out);
    out[0] = in1[0] * in2[0][0] + in1[1] * in2[1][0] + in1[2] * in2[2][0];
    out[1] = in1[0] * in2[0][1] + in1[1] * in2[1][1] + in1[2] * in2[2][1];
    out[2] = in1[0] * in2[0][2] + in1[1] * in2[1][2] + in1[2] * in2[2][2];
}

void math::MatrixAngles(const matrix3x4_t& matrix, ang_t& angles) {
    vec3_t forward, left, up;

    // extract the basis vectors from the matrix. since we only need the z
    // component of the up vector, we don't get x and y.
    forward = { matrix[0][0], matrix[1][0], matrix[2][0] };
    left = { matrix[0][1], matrix[1][1], matrix[2][1] };
    up = { 0.f, 0.f, matrix[2][2] };

    float len = forward.length_2d();

    // enough here to get angles?
    if (len > 0.001f) {
        angles.x = rad_to_deg(std::atan2(-forward.z, len));
        angles.y = rad_to_deg(std::atan2(forward.y, forward.x));
        angles.z = rad_to_deg(std::atan2(left.z, up.z));
    }

    else {
        angles.x = rad_to_deg(std::atan2(-forward.z, len));
        angles.y = rad_to_deg(std::atan2(-left.x, left.y));
        angles.z = 0.f;
    }
}

void math::MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out) {
    std::memcpy(out.Base(), in.Base(), sizeof(matrix3x4_t));
}

void math::ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out) {
    if (&in1 == &out) {
        matrix3x4_t in1b;
        MatrixCopy(in1, in1b);
        ConcatTransforms(in1b, in2, out);
        return;
    }

    if (&in2 == &out) {
        matrix3x4_t in2b;
        MatrixCopy(in2, in2b);
        ConcatTransforms(in1, in2b, out);
        return;
    }

    out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
    out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
    out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
    out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];

    out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
    out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
    out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
    out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];

    out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
    out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
    out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
    out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

bool math::IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, BoxTraceInfo_t* out_info) {
    int   i;
    float d1, d2, f;

    for (i = 0; i < 6; ++i) {
        if (i >= 3) {
            d1 = start[i - 3] - maxs[i - 3];
            d2 = d1 + delta[i - 3];
        }

        else {
            d1 = -start[i] + mins[i];
            d2 = d1 - delta[i];
        }

        // if completely in front of face, no intersection.
        if (d1 > 0.f && d2 > 0.f) {
            out_info->m_startsolid = false;

            return false;
        }

        // completely inside, check next face.
        if (d1 <= 0.f && d2 <= 0.f)
            continue;

        if (d1 > 0.f)
            out_info->m_startsolid = false;

        // crosses face.
        if (d1 > d2) {
            f = std::max(0.f, d1 - tolerance);

            f = f / (d1 - d2);
            if (f > out_info->m_t1) {
                out_info->m_t1 = f;
                out_info->m_hitside = i;
            }
        }

        // leave.
        else {
            f = (d1 + tolerance) / (d1 - d2);
            if (f < out_info->m_t2)
                out_info->m_t2 = f;
        }
    }

    return out_info->m_startsolid || (out_info->m_t1 < out_info->m_t2&& out_info->m_t1 >= 0.f);
}

bool math::IntersectRayWithBox(const vec3_t& start, const vec3_t& delta, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr, float* fraction_left_solid) {
    BoxTraceInfo_t box_tr;

    // note - dex; this is Collision_ClearTrace.
    out_tr->m_startpos = start;
    out_tr->m_endpos = start;
    out_tr->m_endpos += delta;
    out_tr->m_startsolid = false;
    out_tr->m_allsolid = false;
    out_tr->m_fraction = 1.f;
    out_tr->m_contents = 0;

    if (IntersectRayWithBox(start, delta, mins, maxs, tolerance, &box_tr)) {
        out_tr->m_startsolid = box_tr.m_startsolid;

        if (box_tr.m_t1 < box_tr.m_t2 && box_tr.m_t1 >= 0.f) {
            out_tr->m_fraction = box_tr.m_t1;

            // VectorMA( pTrace->startpos, trace.t1, vecRayDelta, pTrace->endpos );

            out_tr->m_contents = CONTENTS_SOLID;
            out_tr->m_plane.m_normal = vec3_t{};

            if (box_tr.m_hitside >= 3) {
                box_tr.m_hitside -= 3;

                out_tr->m_plane.m_dist = maxs[box_tr.m_hitside];
                out_tr->m_plane.m_normal[box_tr.m_hitside] = 1.f;
                out_tr->m_plane.m_type = box_tr.m_hitside;
            }

            else {
                out_tr->m_plane.m_dist = -mins[box_tr.m_hitside];
                out_tr->m_plane.m_normal[box_tr.m_hitside] = -1.f;
                out_tr->m_plane.m_type = box_tr.m_hitside;
            }

            return true;
        }

        if (out_tr->m_startsolid) {
            out_tr->m_allsolid = (box_tr.m_t2 <= 0.f) || (box_tr.m_t2 >= 1.f);
            out_tr->m_fraction = 0.f;

            if (fraction_left_solid)
                *fraction_left_solid = box_tr.m_t2;

            out_tr->m_endpos = out_tr->m_startpos;
            out_tr->m_contents = CONTENTS_SOLID;
            out_tr->m_plane.m_dist = out_tr->m_startpos.x;
            out_tr->m_plane.m_normal = { 1.f, 0.f, 0.f };
            out_tr->m_plane.m_type = 0;
            out_tr->m_startpos = start + (box_tr.m_t2 * delta);

            return true;
        }
    }

    return false;
}

bool math::IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const matrix3x4_t& obb_to_world, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr) {
    vec3_t box_extents, box_center, extent{}, uextent, segment_center, cross, new_start, tmp_end;
    float  coord, tmp, cextent, sign;

    // note - dex; this is Collision_ClearTrace.
    out_tr->m_startpos = start;
    out_tr->m_endpos = start;
    out_tr->m_endpos += delta;
    out_tr->m_startsolid = false;
    out_tr->m_allsolid = false;
    out_tr->m_fraction = 1.f;
    out_tr->m_contents = 0;

    // compute center in local space and transform to world space.
    box_extents = (mins + maxs) / 2.f;
    VectorTransform(box_extents, obb_to_world, box_center);

    // calculate extents from local center.
    box_extents = maxs - box_extents;

    // save the extents of the ray.
    segment_center = start + delta - box_center;

    // check box axes for separation.
    for (int i = 0; i < 3; ++i) {
        extent[i] = delta.x * obb_to_world[0][i] + delta.y * obb_to_world[1][i] + delta.z * obb_to_world[2][i];
        uextent[i] = std::abs(extent[i]);

        coord = segment_center.x * obb_to_world[0][i] + segment_center.y * obb_to_world[1][i] + segment_center.z * obb_to_world[2][i];
        coord = std::abs(coord);
        if (coord > (box_extents[i] + uextent[i]))
            return false;
    }

    // now check cross axes for separation.
    cross = delta.cross(segment_center);

    cextent = cross.x * obb_to_world[0][0] + cross.y * obb_to_world[1][0] + cross.z * obb_to_world[2][0];
    cextent = std::abs(cextent);
    tmp = box_extents.y * uextent.z + box_extents.z * uextent.y;
    if (cextent > tmp)
        return false;

    cextent = cross.x * obb_to_world[0][1] + cross.y * obb_to_world[1][1] + cross.z * obb_to_world[2][1];
    cextent = std::abs(cextent);
    tmp = box_extents.x * uextent.z + box_extents.z * uextent.x;
    if (cextent > tmp)
        return false;

    cextent = cross.x * obb_to_world[0][2] + cross.y * obb_to_world[1][2] + cross.z * obb_to_world[2][2];
    cextent = std::abs(cextent);
    tmp = box_extents.x * uextent.y + box_extents.y * uextent.x;
    if (cextent > tmp)
        return false;

    // we hit this box, compute intersection point and return.
    // compute ray start in bone space.
    VectorITransform(start, obb_to_world, new_start);

    // extent is ray.m_Delta in bone space, recompute delta in bone space.
    extent *= 2.f;

    // delta was prescaled by the current t, so no need to see if this intersection is closer.
    if (!IntersectRayWithBox(start, extent, mins, maxs, tolerance, out_tr))
        return false;

    // fix up the start/end pos and fraction
    VectorTransform(out_tr->m_endpos, obb_to_world, tmp_end);

    out_tr->m_endpos = tmp_end;
    out_tr->m_startpos = start;
    out_tr->m_fraction *= 2.f;

    // fix up the plane information
    sign = out_tr->m_plane.m_normal[out_tr->m_plane.m_type];

    out_tr->m_plane.m_normal.x = sign * obb_to_world[0][out_tr->m_plane.m_type];
    out_tr->m_plane.m_normal.y = sign * obb_to_world[1][out_tr->m_plane.m_type];
    out_tr->m_plane.m_normal.z = sign * obb_to_world[2][out_tr->m_plane.m_type];
    out_tr->m_plane.m_dist = out_tr->m_endpos.dot(out_tr->m_plane.m_normal);
    out_tr->m_plane.m_type = 3;

    return true;
}

bool math::IntersectRayWithOBB(const vec3_t& start, const vec3_t& delta, const vec3_t& box_origin, const ang_t& box_rotation, const vec3_t& mins, const vec3_t& maxs, float tolerance, CBaseTrace* out_tr) {
    // todo - dex; https://github.com/pmrowla/hl2sdk-csgo/blob/master/public/collisionutils.cpp#L1400
    return false;
}

bool math::IntersectInfiniteRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2) {
    vec3_t sphere_to_ray;
    float  a, b, c, discrim, oo2a;

    sphere_to_ray = start - sphere_center;
    a = delta.dot(delta);

    // this would occur in the case of a zero-length ray.
    if (!a) {
        *out_t1 = 0.f;
        *out_t2 = 0.f;

        return sphere_to_ray.length_sqr() <= (radius * radius);
    }

    b = 2.f * sphere_to_ray.dot(delta);
    c = sphere_to_ray.dot(sphere_to_ray) - (radius * radius);

    discrim = b * b - 4.f * a * c;
    if (discrim < 0.f)
        return false;

    discrim = std::sqrt(discrim);
    oo2a = 0.5f / a;

    *out_t1 = (-b - discrim) * oo2a;
    *out_t2 = (-b + discrim) * oo2a;

    return true;
}

bool math::IntersectRayWithSphere(const vec3_t& start, const vec3_t& delta, const vec3_t& sphere_center, float radius, float* out_t1, float* out_t2) {
    if (!IntersectInfiniteRayWithSphere(start, delta, sphere_center, radius, out_t1, out_t2))
        return false;

    if (*out_t1 > 1.0f || *out_t2 < 0.0f)
        return false;

    // clamp intersection points.
    *out_t1 = std::max(0.f, *out_t1);
    *out_t2 = std::min(1.f, *out_t2);

    return true;
}

vec3_t math::Interpolate(const vec3_t from, const vec3_t to, const float percent) {
    return to * percent + from * (1.f - percent);
}

vec3_t math::vector_angles(const vec3_t& v) {
    float magnitude = sqrtf(v.x * v.x + v.y * v.y);
    float pitch = atan2f(-v.z, magnitude) * M_RADPI;
    float yaw = atan2f(v.y, v.x) * M_RADPI;

    vec3_t angle(pitch, yaw, 0.0f);
    return angle;
}

vec3_t math::angle_vectors(const vec3_t& angles) {
    float sp, sy, cp, cy;
    sp = sinf(angles.x * M_PIRAD);
    cp = cosf(angles.x * M_PIRAD);
    sy = sinf(angles.y * M_PIRAD);
    cy = cosf(angles.y * M_PIRAD);

    return vec3_t{ cp * cy, cp * sy, -sp };
}