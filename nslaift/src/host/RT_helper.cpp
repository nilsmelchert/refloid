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

std::string rthelpers::ptxPath(const std::string &cuda_file)
{
    return std::string(BIN_DIR) + std::string(SAMPLE_NAME) + "_generated_" + cuda_file + ".ptx";
}

std::string rthelpers::printMat4x4(optix::Matrix4x4 &mat)
{
    std::cout << mat[0] << " " << mat[1] << " " << mat[2] << " " << mat[3] << std::endl;
    std::cout << mat[4] << " " << mat[5] << " " << mat[6] << " " << mat[7] << std::endl;
    std::cout << mat[8] << " " << mat[9] << " " << mat[10] << " " << mat[11] << std::endl;
    std::cout << mat[12] << " " << mat[13] << " " << mat[14] << " " << mat[15] << std::endl;
    std::cout << "" << std::endl;
}

std::vector<unsigned char> rthelpers::writeBufferToPipe(optix::Buffer buffer)
{
    return writeBufferToPipe(buffer->get());
}

std::vector<unsigned char> rthelpers::writeBufferToPipe(RTbuffer buffer)
{
    int width, height;
    RTsize buffer_width, buffer_height;

    void *imageData;
    RT_CHECK_ERROR(rtBufferMap(buffer, &imageData));

    RT_CHECK_ERROR(rtBufferGetSize2D(buffer, &buffer_width, &buffer_height));
    width = static_cast<int>(buffer_width);
    height = static_cast<int>(buffer_height);

    std::vector<unsigned char> pix(width * height * 3);

    RTformat buffer_format;
    RT_CHECK_ERROR(rtBufferGetFormat(buffer, &buffer_format));

    switch (buffer_format) {
        case RT_FORMAT_UNSIGNED_BYTE4:
            // Data is BGRA and upside down, so we need to swizzle to RGB
            for (int j = height - 1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3 * width * (height - 1 - j));
                unsigned char *src = ((unsigned char *) imageData) + (4 * width * j);
                for (int i = 0; i < width; i++) {
                    *dst++ = *(src + 0);
                    *dst++ = *(src + 1);
                    *dst++ = *(src + 2);
                    src += 4;
                }
            }
            break;

        case RT_FORMAT_FLOAT:
            // This buffer is upside down
            for (int j = height - 1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + width * (height - 1 - j);
                float *src = ((float *) imageData) + (3 * width * j);
                for (int i = 0; i < width; i++) {
                    int P = static_cast<int>((*src++) * 255.0f);
                    unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;

                    // write the pixel to all 3 channels
                    *dst++ = static_cast<unsigned char>(Clamped);
                    *dst++ = static_cast<unsigned char>(Clamped);
                    *dst++ = static_cast<unsigned char>(Clamped);
                }
            }
            break;

        case RT_FORMAT_FLOAT3:
            // This buffer is upside down
            for (int j = height - 1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3 * width * (height - 1 - j));
                float *src = ((float *) imageData) + (3 * width * j);
                for (int i = 0; i < width; i++) {
                    for (int elem = 0; elem < 3; ++elem) {
                        int P = static_cast<int>((*src++) * 255.0f);
                        unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
                        *dst++ = static_cast<unsigned char>(Clamped);
                    }
                }
            }
            break;

        case RT_FORMAT_FLOAT4:
            // This buffer is upside down
            for (int j = height - 1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3 * width * (height - 1 - j));
                float *src = ((float *) imageData) + (4 * width * j);
                for (int i = 0; i < width; i++) {
                    for (int elem = 0; elem < 3; ++elem) {
                        int P = static_cast<int>((*src++) * 255.0f);
                        unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
                        *dst++ = static_cast<unsigned char>(Clamped);
                    }

                    // skip alpha
                    src++;
                }
            }
            break;

        default:
            fprintf(stderr, "Unrecognized buffer data type or format.\n");
            exit(2);
            break;
    }

    // Now unmap the buffer
    RT_CHECK_ERROR(rtBufferUnmap(buffer));
    return pix;
}
