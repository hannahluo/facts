#include <math.h>
#include <stdint.h>

#include "angle_calculation.h"

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
    double mag = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
    vec->x /= mag;
    vec->y /= mag;
    vec->z /= mag;
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
    while(is_parallel(&calfJointAxis, &random_vector) || is_parallel(&thighJointAxis, &random_vector)) {
        random_vector.x -= 0.25;
        random_vector.z += 0.25;    
    }
    make_unit_vector(&random_vector);

    // Store cross product
    cross_product(&thighJointAxis, &random_vector, &thighCross);
    cross_product(&calfJointAxis, &random_vector, &calfCross);    
}

// multiply matrix by vector
static void matrix_mult_vector(double matrix[NUM_ROT_MATRIX_ROWS][NUM_ROT_MATRIX_COLS], vector_t* vec, vector_t* result)
{
    result->x = matrix[0][0]*vec->x + matrix[0][1]*vec->y + matrix[0][2]*vec->z;
    result->y = matrix[1][0]*vec->x + matrix[1][1]*vec->y + matrix[1][2]*vec->z;
    result->z = matrix[2][0]*vec->x + matrix[2][1]*vec->y + matrix[2][2]*vec->z;
}

double calculate_angle(double calfRotMatrix[NUM_ROT_MATRIX_ROWS][NUM_ROT_MATRIX_COLS], double thighRotMatrix[NUM_ROT_MATRIX_ROWS][NUM_ROT_MATRIX_COLS])
{
    vector_t calfVec = {.x=0, .y=0, .z=0};
    vector_t thighVec = {.x=0, .y=0, .z=0};
    matrix_mult_vector(calfRotMatrix, &calfCross, &calfVec);
    matrix_mult_vector(thighRotMatrix, &thighCross, &thighVec);

    // Find angle between calfVec and thighVec
    double dotProd = calfVec.x*thighVec.x + calfVec.y*thighVec.y + calfVec.z*calfVec.z;
    double magCalfVec = sqrt(calfVec.x*calfVec.x + calfVec.y*calfVec.y + calfVec.z*calfVec.z);
    double magThighVec = sqrt(thighVec.x*thighVec.x + thighVec.y*thighVec.y + thighVec.z*thighVec.z);

    return acos(dotProd / (magCalfVec*magThighVec));
}