//
// Created by melchert on 30.04.19.
//

#include "RT_helper.h"

/**
  @brief    parse a comma separated string 3-vector of the form "1.2,43, -12.455" into  its 3 float values
  @param    str string to decompose
  @param    x   target for first element
  @param    y   target for second element
  @param    z   target for third element
  @return   returns 0 on success, non-zero on errors
  **/
int rthelpers::RT_parse_float3(const QString &str, float *x, float *y, float *z, const QString& delimiter /*=QString(","*/)
{
    if(x == nullptr || y == nullptr || z == nullptr)
        return -1;

    QStringList sList = str.split(delimiter);
    if (sList.count() != 3) {
        *x = 0;
        *y = 0;
        *z = 0;
        return -2;
    }
    bool ok = false;
    *x = sList.at(0).toFloat(&ok);
    if (!ok)
        return -3;
    *y = sList.at(1).toFloat(&ok);
    if (!ok)
        return -4;
    *z = sList.at(2).toFloat(&ok);
    if (!ok)
        return -5;
    return 0;
}

/**
 @brief parse a comma separated string 4x4 Matrix of the form "23.0, 2344.34, ... , 3.46, 65.235"
 @param str string to decompose
 @param mat resulting matrix parsed by string
 @return returns 0 on success, non-zero on errors
 **/
int rthelpers::RT_parse_matrix(const QString &str, optix::Matrix4x4 *mat, const QString& delimiter /*=QString(","*/)
{
    if (mat == nullptr)
        return -1;

    QStringList sList = str.split(delimiter);
    if (sList.count() != 16) {
        *mat = optix::Matrix4x4::identity();
        return -2;
    }

    bool ok = false;
    for (int i=0; i<16; i++)
    {
        (*mat)[i] = sList.at(i).toFloat(&ok);
        if (!ok)
            return -i;
    }

    return 0;
}
