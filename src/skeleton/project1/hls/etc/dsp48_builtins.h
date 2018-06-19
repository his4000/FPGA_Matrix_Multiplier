/* dsp48_builtins.h*/
/*
 * __VIVADO_HLS_COPYRIGHT-INFO__
 *
 * $Id$
 */

#ifndef _DSP48_BUILTINS_H_
#define _DSP48_BUILTINS_H_
#ifdef __DSP48E1__
#include "etc/dsp48e1_builtins.h"
#elif __DSP48E2__
#include "etc/dsp48e2_builtins.h"
#else
#error Please select a proper device to use dsp builtins.
#endif
#endif
