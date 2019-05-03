/*
   @file    rt_function.h
  @author   Pascal Kern   (kern@imr.uni-hannover.de)
  @since    2018 / 28 / 6
  @brief    Used for forcing Device code to run on Device
*/

#pragma once

#ifndef RT_FUNCTION
#define RT_FUNCTION __forceinline__ __device__
#endif
