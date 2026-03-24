/*
 * quaternion.c
 *
 *  Created on: Mar 24, 2026
 *      Author: kyojin
 */
#include "quaternion.h"
#include <math.h>

inline Quaternion quaternion_hamilton(Quaternion q1, Quaternion q2) {
    return (Quaternion){
        .w= q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
        .x= q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
        .y= q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
        .z= q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w};
}

inline Quaternion quaternion_scalar(Quaternion q1, float scalar) {
    return (Quaternion){
        .w= q1.w * scalar,
        .x= q1.x * scalar,
        .y= q1.y * scalar,
        .z= q1.z * scalar};
}

inline Quaternion quaternion_add(Quaternion q1, Quaternion q2) {
    return (Quaternion){
        .w= q1.w + q2.w,
        .x= q1.x + q2.x,
        .y= q1.y + q2.y,
        .z= q1.z + q2.z};
}

inline Quaternion quaternion_sub(Quaternion q1, Quaternion q2) {
    return (Quaternion){
        .w= q1.w - q2.w,
        .x= q1.x - q2.x,
        .y= q1.y - q2.y,
        .z= q1.z - q2.z};
}

inline Quaternion quaternion_normalize(Quaternion q1) {
    float norm = quaternion_norm(q1);
    if(norm == 0.0) return q1;
    return (Quaternion) {
        .w= q1.w/norm,
        .x= q1.x/norm,
        .y= q1.y/norm,
        .z= q1.z/norm,
    };
}

inline float quaternion_norm(Quaternion q1) {
    return sqrtf(q1.w*q1.w + q1.x*q1.x + q1.y*q1.y + q1.z*q1.z);
}
