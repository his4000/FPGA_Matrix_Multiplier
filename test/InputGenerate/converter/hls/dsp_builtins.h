/*
 * __VIVADO_HLS_COPYRIGHT-INFO__
 *
 *
 */

#ifndef X_DSP_BUILTINS_H
#define X_DSP_BUILTINS_H
#include <stdint.h>

#ifdef __cplusplus
#include <ap_int.h>
typedef ap_int<1> __dsp1_t;
typedef ap_int<6> __dsp6_t;
typedef ap_int<18> __dsp18_t;
typedef ap_int<25> __dsp25_t;
typedef ap_int<27> __dsp27_t;
typedef ap_int<30> __dsp30_t;
typedef ap_int<48> __dsp48_t;
#else
#include <ap_cint.h>
typedef int1 __dsp1_t;
typedef int6 __dsp6_t;
typedef int18 __dsp18_t;
typedef int25 __dsp25_t;
typedef int27 __dsp27_t;
typedef int30 __dsp30_t;
typedef int48 __dsp48_t;
#endif

#include "etc/dsp_dp_builtins.h"
#include "etc/dsp48_builtins.h"

#endif

