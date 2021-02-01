#ifndef ANGLE_CALC_H
#define ANGLE_CALC_H

#define NUM_ROT_MATRIX_ROWS   (3)
#define NUM_ROT_MATRIX_COLS   (3)

typedef struct vector_s {
    double x;
    double y;
    double z;
} vector_t;

void update_joint_axes(vector_t* calf, vector_t* thigh);

double calculate_angle(double calfRotMatrix[NUM_ROT_MATRIX_ROWS][NUM_ROT_MATRIX_COLS], double thighRotMatrix[NUM_ROT_MATRIX_ROWS][NUM_ROT_MATRIX_COLS]);


#endif