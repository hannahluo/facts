#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "angle_calculation.h"
#include <assert.h>

#define MAG_VEC(vec)            (sqrt((vec).x*(vec).x + (vec).y*(vec).y + (vec).z*(vec).z))
#define MAG_QUAT(quat)          (sqrt((quat).w*(quat).w + (quat).x*(quat).x + (quat).y*(quat).y + (quat).z*(quat).z))
#define DOT_PROD(vec1, vec2)    ((vec1).x*(vec2).x + (vec1).y*(vec2).y + (vec1).z*(vec2).z)
#define IS_ZERO_VEC(vec)        ((vec).x == 0 && (vec).y == 0 && (vec).z == 0)

static vector_t calfCross = {.x=0, .y=0, .z=0};
static vector_t thighCross = {.x=0, .y=0, .z=0};

#define PARALELLISM_THRESHOLD   (0.1)
static uint8_t is_parallel(vector_t* vec1, vector_t* vec2)
{
    double factor[3] = {0};
    factor[0] = vec1->x / vec2->x;
    factor[1] = vec1->y / vec2->y;
    factor[2] = vec1->z / vec2->z;

    return fabs(factor[0] - factor[1]) < PARALELLISM_THRESHOLD && fabs(factor[1]-factor[2]) < PARALELLISM_THRESHOLD;
}

static void make_unit_vector(vector_t* vec)
{
    double mag = MAG_VEC(*vec);
    if(mag != 0) {
        vec->x /= mag;
        vec->y /= mag;
        vec->z /= mag;
    }
}

static void make_unit_quat(quat_t* quat)
{
    double mag = MAG_QUAT(*quat);
    if(quat != 0) {
        quat->w /= mag;
        quat->x /= mag;
        quat->y /= mag;
        quat->z /= mag;
    }
}

// vec1 x vec2
static void cross_product(vector_t* vec1, vector_t* vec2, vector_t* cross)
{
    cross->x = vec1->y*vec2->z - vec1->z*vec2->y;
    cross->y = vec1->z*vec2->x - vec1->x*vec2->z;
    cross->z = vec1->x*vec2->y - vec1->y*vec2->x;
}

void update_joint_axes(vector_t* calf, vector_t* thigh)
{
    vector_t random_vector = {.x=1, .y=1, .z=1};
    vector_t calfJointAxis = {.x=0, .y=0, .z=0};
    vector_t thighJointAxis = {.x=0, .y=0, .z=0};

    calfJointAxis.x = calf->x;
    calfJointAxis.y = calf->y;
    calfJointAxis.z = calf->z;

    thighJointAxis.x = thigh->x;
    thighJointAxis.y = thigh->y;
    thighJointAxis.z = thigh->z;

    // Ensure not parallel with random_vector
    if(is_parallel(&calfJointAxis, &random_vector) || is_parallel(&thighJointAxis, &random_vector)) {
        random_vector.z += 0.25;
    }

    // Store cross product
    cross_product(&thighJointAxis, &random_vector, &thighCross);
    cross_product(&calfJointAxis, &random_vector, &calfCross);
     
    // Make the cross product result vectors unit vectors
    make_unit_vector(&thighCross);
    make_unit_vector(&calfCross);

}

static uint8_t quat_mult(quat_t* q1, quat_t* q2, quat_t* result)
{
    result->w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
    result->x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
    result->y = q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x;
    result->z = q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w;
}

static uint8_t apply_quat_rot(quat_t* rot, vector_t* vec, vector_t* result)
{
    if(rot == NULL || vec == NULL || result == NULL) {
        NRF_LOG_ERROR("Invalid input to apply_quat_rot");
        return 1;
    }
    
    // Ensure that rotation quaternion is a unit quaternion
    make_unit_quat(rot);
    
    // convert the vector into a quaternion
    quat_t vecQuat = {.w=0, .x=vec->x, .y=vec->y, .z=vec->z};
    
    // get the inverse of the rotation quaternion
    quat_t invRot = {.w=rot->w, .x=-1*rot->x, .y=-1*rot->y, .z=-1*rot->z};

    // temp structures to hold intermediate val
    quat_t intQuat = {.w=0,.x=0,.y=0,.z=0}; // intermediate result
    quat_t resultQuat = {.w=0,.x=0,.y=0,.z=0};

    // Multiply quaterions
    quat_mult(rot, &vecQuat, &intQuat);
    quat_mult(&intQuat, &invRot, &resultQuat);
    result->x = resultQuat.x;
    result->y = resultQuat.y;
    result->z = resultQuat.z;    

    return 0;
}

double calculate_angle(quat_t* calfQuat, quat_t* thighQuat)
{
    double angle = 0;
    if(calfQuat == NULL || thighQuat == NULL) {
        NRF_LOG_ERROR("Invalid input to calculate_angle");
        return 0;
    }

    vector_t calfVec = {.x=0, .y=0, .z=0};
    vector_t thighVec = {.x=0, .y=0, .z=0};

    // apply the quat rotations
    uint8_t ret = 0;
    ret = apply_quat_rot(calfQuat, &calfCross, &calfVec);
    ret += apply_quat_rot(thighQuat, &thighCross, &thighVec);
    if(ret != 0) {
        NRF_LOG_ERROR("Failed to apply rotations\n");
        return 0;
    }
    

    // Find angle between calfVec and thighVec
    double dotProd = DOT_PROD(calfVec, thighVec);
    double magCalfVec = MAG_VEC(calfVec);
    double magThighVec = MAG_VEC(thighVec);

    return acos(DOT_PROD(calfVec, thighVec) / (MAG_VEC(calfVec)*MAG_VEC(thighVec)));
}

#ifndef NDEBUG
#define EQUALITY_THRESHOLD    (0.00001)
static void test_dot_prod()
{
    vector_t vec1 = {.x=1.5, .y=-4.5, .z=0};
    vector_t vec2 = {.x=-0.2, .y=-10, .z=1000};
    double result = DOT_PROD(vec1, vec2);
    assert(result == 44.7);

    vec1.x = -0.5;
    vec1.y = 0.5;
    vec1.z = 2;
    vec2.x = 2;
    vec2.y = 0;
    vec2.z = 0.25;
    result = DOT_PROD(vec1, vec2);
    assert(result == -0.5);
}

static void test_mag_vec()
{
    vector_t vec = {.x = 0, .y = 0, .z = 0};
    double mag = MAG_VEC(vec);
    assert(mag == 0);

    vec.x = 1;
    vec.y = 1;
    vec.z = -1;
    mag = MAG_VEC(vec);
    assert(mag == sqrt(3));

    vec.x = 2;
    vec.y = 0;
    vec.z = -0.5;
    mag = MAG_VEC(vec);
    assert(mag == sqrt(4.25));
}

static void test_unit_vector()
{
    vector_t vec = {.x = 0, .y = 0, .z = 0};
    make_unit_vector(&vec);
    assert(vec.x == 0);
    assert(vec.y == 0);
    assert(vec.z == 0);

    vec.x = 1;
    vec.y = -2;
    vec.z = 3;
    make_unit_vector(&vec);
    assert(fabs(vec.x - 0.267261) <= EQUALITY_THRESHOLD);
    assert(fabs(vec.y - (-0.534522)) <= EQUALITY_THRESHOLD);
    assert(fabs(vec.z - (0.801784)) <= EQUALITY_THRESHOLD);

    vec.x = 1;
    vec.y = -2;
    vec.z = 0;
    make_unit_vector(&vec);
    assert(fabs(vec.x - 0.447214) <= EQUALITY_THRESHOLD);
    assert(fabs(vec.y - (-0.894427)) <= EQUALITY_THRESHOLD);
    assert(fabs(vec.z - 0) <= EQUALITY_THRESHOLD);
}

static void test_mag_quat()
{
    quat_t quat = {.w = 1, .x = 2, .y = 3.23, .z = -4};
    double mag = MAG_QUAT(quat);
    assert(fabs(mag - 5.60651) <= EQUALITY_THRESHOLD);

    quat.w = 0;
    quat.x = -5;
    quat.y = 0.25;
    quat.z = 4;
    mag = MAG_QUAT(quat);
    assert(fabs(mag - 6.408000) <= EQUALITY_THRESHOLD);
}

static void test_unit_quat()
{
    quat_t quat = {.w = 1, .x = 2, .y = 3.23, .z = -4};
    make_unit_quat(&quat);
    assert(fabs(quat.w - 0.17836) <= EQUALITY_THRESHOLD);
    assert(fabs(quat.x - 0.35673) <= EQUALITY_THRESHOLD);
    assert(fabs(quat.y - 0.57612) <= EQUALITY_THRESHOLD);
    assert(fabs(quat.z - (-0.71346)) <= EQUALITY_THRESHOLD);
}

static void test_parallel_check()
{
    vector_t vec1 = {.x = 1, .y = -3, .z = 2};
    vector_t vec2 = {.x = 3, .y = -9, .z = 6};
    assert(is_parallel(&vec1, &vec2) == 1);

    vec1.x = 1;
    vec1.y = 1;
    vec1.z = 1;
    vec2.x = 1;
    vec2.y = -1;
    vec2.z = 1;
    assert(is_parallel(&vec1, &vec2) == 0);

    vec1.x = -0.99;
    vec1.y = 0.99;
    vec1.z = -1.01;
    vec2.x = 1;
    vec2.y = -1;
    vec2.z = 1;
    assert(is_parallel(&vec1, &vec2) == 1);
}

static void test_is_zero_vec()
{
    vector_t vec1 = {.x = 1, .y = -3, .z = 2};
    assert(!IS_ZERO_VEC(vec1));

    vector_t* vec1_ptr = &vec1;
    assert(!IS_ZERO_VEC(*vec1_ptr));

    vec1.x = 0;
    vec1.y = 0;
    vec1.z = 0;
    assert(IS_ZERO_VEC(vec1));
}

static void test_cross_product()
{
    vector_t vec1 = {.x = 1.8, .y = -3.23, .z = 2.2};
    vector_t vec2 = {.x = -3.2, .y = -0.8, .z = 6.78};
    vector_t result = {.x = 0, .y = 0, .z = 0};
    cross_product(&vec1, &vec2, &result);
    assert(result.x == -20.1394);
    assert(result.y == -19.2440);
    assert(result.z == -11.776);
}

static void test_quat_mult()
{
    quat_t quat1 = {.w = 1.2, .x = -0.65, .y = 2, .z = 1};
    quat_t quat2 = {.w = 1, .x = 2, .y = 3.23, .z = -4};
    quat_t result = {.w = 0, .x = 0, .y = 0, .z = 0};
    quat_mult(&quat1, &quat2, &result);
    assert(fabs(result.w - 0.04) <= EQUALITY_THRESHOLD);
    assert(fabs(result.x - (-9.48)) <= EQUALITY_THRESHOLD);
    assert(fabs(result.y - 5.276) <= EQUALITY_THRESHOLD);
    assert(fabs(result.z - (-9.8995)) <= EQUALITY_THRESHOLD);

    quat1.w = 1.2;
    quat1.x = -0.65;
    quat1.y = 2;
    quat1.z = 1;
    quat2.w = 0;
    quat2.x = 2;
    quat2.y = 3.23;
    quat2.z = -4;
    quat_mult(&quat1, &quat2, &result);
    assert(fabs(result.w - (-1.16)) <= EQUALITY_THRESHOLD);
    assert(fabs(result.x - (-8.83)) <= EQUALITY_THRESHOLD);
    assert(fabs(result.y - 3.276) <= EQUALITY_THRESHOLD);
    assert(fabs(result.z - (-10.8995)) <= EQUALITY_THRESHOLD);    
}

static void test_apply_quat_rot()
{
    quat_t rot = {.w = 0.70711, .x = 0.4082, .y = -0.8165, .z = 0.28864};
    vector_t vec1 = {.x = 1.8, .y = -3.23, .z = 2.2};
    vector_t result = {.x = 0, .y = 0, .z = 0};
    apply_quat_rot(&rot, &vec1, &result);
    assert(fabs(result.x - 0.917394) <= EQUALITY_THRESHOLD);
    assert(fabs(result.y - (-4.04707)) <= EQUALITY_THRESHOLD);
    assert(fabs(result.z - 1.13687) <= EQUALITY_THRESHOLD);
}

static void test_calculation()
{
    vector_t calf = {.x = -0.5, .y = -0.5, .z = -0.5};
    vector_t thigh = {.x = 1, .y = -0.4, .z = 0.3};
    update_joint_axes(&calf, &thigh);
    assert(fabs(calfCross.x - (-0.7071)) <= EQUALITY_THRESHOLD);
    assert(fabs(calfCross.y - 0.7071) <= EQUALITY_THRESHOLD);
    assert(fabs(calfCross.z - (0)) <= EQUALITY_THRESHOLD);
    assert(fabs(thighCross.x - (-0.427465)) <= EQUALITY_THRESHOLD);
    assert(fabs(thighCross.y - (-0.507615)) <= EQUALITY_THRESHOLD);
    assert(fabs(thighCross.z - 0.748064) <= EQUALITY_THRESHOLD);
    

    quat_t calfQuat = {.w = 0.70711, .x=0.1, .y=-0.8165, .z=-0.28864};
    quat_t thighQuat = {.w = 0.70711, .x=0.4082, .y = -0.8165, .z=0.28864};
    double angle = calculate_angle(&calfQuat, &thighQuat);
    assert(fabs(angle - (2.21761)) <= EQUALITY_THRESHOLD);
}

void test_angle_calc()
{
    NRF_LOG_INFO("Testing angle calculation");
    test_dot_prod();
    test_mag_vec();
    test_unit_vector();
    test_parallel_check();
    test_is_zero_vec();
    test_cross_product();
    test_mag_quat();
    test_unit_quat();
    test_quat_mult();
    test_apply_quat_rot();
    test_calculation();
}
#endif

/* Quaternion conversion
  const double scale = (1.0 / (1 << 14));
  imu::Quaternion quat(scale * w, scale * x, scale * y, scale * z);
  return quat;
*/