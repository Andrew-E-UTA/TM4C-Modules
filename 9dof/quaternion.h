/*
 * quaternion.h
 *
 *  Created on: Mar 17, 2026
 *      Author: kyojin
 */

#ifndef QUATERNION_H_
#define QUATERNION_H_

#include <stdint.h>

typedef struct {float w, x, y, z; } Quaternion;

inline Quaternion quaternion_hamilton(Quaternion q1, Quaternion q2);
inline Quaternion quaternion_scalar(Quaternion q1, float scalar);
inline Quaternion quaternion_add(Quaternion q1, Quaternion q2);
inline Quaternion quaternion_sub(Quaternion q1, Quaternion q2);
inline Quaternion quaternion_normalize(Quaternion q1);
inline float      quaternion_q_norm(Quaternion q1);
float             q_sqrt(float f);

#endif /* QUATERNION_H_ */
