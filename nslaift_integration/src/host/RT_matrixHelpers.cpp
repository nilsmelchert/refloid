//
// Created by melchert on 30.04.19.
//

#include "RT_matrixHelpers.h"

/**
  @brief    create 4x4 homogeneous rotation matrix with yaw pitch and roll set to parameters (in rad)
  @param    yaw     matrix yaw angle (in rad)
  @param    pitch   matrix pitch angle (in rad)
  @param    roll    matrix roll angle (in rad)
  @return   rotation matrix

  Matrix is 4x4 because for use with homogeneous 3D coordinates

  **/
optix::Matrix4x4 mhelpers::rotation(float yaw, float pitch, float roll) {

    float sx = std::sin(yaw);
    float sy = std::sin(pitch);
    float sz = std::sin(roll);
    float cx = std::cos(yaw);
    float cy = std::cos(pitch);
    float cz = std::cos(roll);

    optix::Matrix4x4 rot = optix::Matrix4x4::identity();
    rot[0] = cy * cz;
    rot[1] = -cx * sz + sx * sy * cz;
    rot[2] = sx * sz + cx * sy * cz;
    rot[3] = 0;

    rot[4] = cy * sz;
    rot[5] = cx * cz - sx * sy * sz;
    rot[6] = -sx * cz + cx * sy * sz;
    rot[7] = 0;

    rot[8] = -sy;
    rot[9] = sx * cy;
    rot[10] = cx * cy;
    rot[11] = 0;

    rot[12] = rot[13] = rot[14] = 0;
    rot[15] = 1;

    return rot;
}
