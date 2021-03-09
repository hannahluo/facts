#ifndef ANGLE_CALC_H
#define ANGLE_CALC_H

#define NUM_ROT_MATRIX_ROWS   (3)
#define NUM_ROT_MATRIX_COLS   (3)

typedef struct vector_s {
    double x;
    double y;
    double z;
} vector_t;

/*typedef struct euler_s {
    double roll;
    double pitch;
    double yaw;
} euler_t;*/

typedef struct quat_s {
    double w;
    double x;
    double y;
    double z;
} quat_t;

void update_joint_axes(vector_t* calf, vector_t* thigh);
double calculate_angle(quat_t* calfQuat, quat_t* thighQuat);

#ifndef NDEBUG
// Testing
void test_angle_calc();
#endif

#endif