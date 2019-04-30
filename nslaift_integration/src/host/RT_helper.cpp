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
    if(x == NULL || y == NULL || z == NULL)
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
