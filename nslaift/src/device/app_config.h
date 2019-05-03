/*
   @file    app_config.h
  @author   Pascal Kern   (kern@imr.uni-hannover.de)
  @since    2018 / 28 / 6
  @brief    Application config for common defines
*/

#pragma once

// This header with defines is included in all shaders
// to be able to switch different code paths at a central location.
// Changing any setting here will rebuild the whole solution.

// From the OptiX Header:
// Unless compatibility with SM_10 is needed, new code should use this define and rely on the new templated version of rtCallableProgram.
#define RT_USE_TEMPLATED_RTCALLABLEPROGRAM 1

// DAR Prevent that division by very small floating point values results in huge values, for example dividing by pdf.
#define DENOMINATOR_EPSILON 1.0e-6f

// 0 == Brute force path tracing without next event estimation (direct lighting). // Debug setting to compare lighting results.
// 1 == Next event estimation per path vertex (direct lighting) and using MIS with power heuristic. // Default.
#define USE_NEXT_EVENT_ESTIMATION 1

// 0 == Disable all OptiX exceptions, rtPrintfs and rtAssert functionality. (Benchmark only in this mode!)
// 1 == Enable  all OptiX exceptions, rtPrintfs and rtAssert functionality. (Really only for debugging, big performance hit!)
#define USE_DEBUG_EXCEPTIONS 0

// Defines for getting the cmake environment variables in c++
#ifdef OPTIX_BIN_PATH
    #define BIN_DIR OPTIX_BIN_PATH
#else
    #define BIN_DIR NULL
#endif
#ifdef OPTIX_BASE_PATH
    #define BASE_DIR OPTIX_BASE_PATH
#else
    #define BASE_DIR NULL
#endif

// 0 = Disable the Visualization of Multireflection red = 1 bounce, orange = 2 bounces, green = 3 bounces , blue/purple = 4 bounces
// 1 = Enable the Visualization of Multireflection
#define Multireflection_Visualization 0
// 0 = Enable Transparency of Visualization
// 1 = Disable Transparency of Visualization
#define Visualization_Transparency_OFF 1

