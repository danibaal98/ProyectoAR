#include <GL/glut.h>    
#include <AR/gsub.h>    
#include <AR/video.h>   
#include <AR/param.h>   
#include <AR/ar.h>
#include <math.h>

#include "../include/utils.h"

double calculateDistance(double patt_trans1[3][4], double patt_trans2[3][4]) {
    double m1[3][4], m2[3][4];
    double distance;

    arUtilMatInv(patt_trans1, m1);
    arUtilMatMul(m1, patt_trans2, m2);
    distance = sqrt(pow(m2[0][3], 2) + pow(m2[1][3], 2) + pow(m2[2][3], 2));

    return distance;
} 