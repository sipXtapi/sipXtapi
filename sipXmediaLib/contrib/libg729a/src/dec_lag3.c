/**
 *  Copyright (c) 2015, Russell
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  Portions of this file are derived from the following ITU notice:
 *
 *  ITU-T G.729 Software Package Release 2 (November 2006)
 *
 *  ITU-T G.729A Speech Coder    ANSI-C Source Code
 *  Version 1.1    Last modified: September 1996
 *
 *  Copyright (c) 1996,
 *  AT&T, France Telecom, NTT, Universite de Sherbrooke
 *  All rights reserved.
 */

/*------------------------------------------------------------------------*
 *    Function g729_Dec_lag3                                                   *
 *             ~~~~~~~~                                                   *
 *   Decoding of fractional pitch lag with 1/3 resolution.                *
 * See "g729_Enc_lag3.c" for more details about the encoding procedure.        *
 *------------------------------------------------------------------------*/

#include "g729a_typedef.h"
#include "basic_op.h"
#include "ld8a.h"

void g729_Dec_lag3(
  G729_Word16 index,       /* input : received pitch index           */
  G729_Word16 pit_min,     /* input : minimum pitch lag              */
  G729_Word16 pit_max,     /* input : maximum pitch lag              */
  G729_Word16 i_subfr,     /* input : subframe flag                  */
  G729_Word16 *T0,         /* output: integer part of pitch lag      */
  G729_Word16 *T0_frac     /* output: fractional part of pitch lag   */
)
{
  G729_Word16 i;
  G729_Word16 T0_min, T0_max;

  if (i_subfr == 0)                  /* if 1st subframe */
  {
    if (g729_sub(index, 197) < 0)
    {
      /* *T0 = (index+2)/3 + 19 */

      *T0 = g729_add(g729_mult(g729_add(index, 2), 10923), 19);

      /* *T0_frac = index - *T0*3 + 58 */

      i = g729_add(g729_add(*T0, *T0), *T0);
      *T0_frac = g729_add(g729_sub(index, i), 58);
    }
    else
    {
      *T0 = g729_sub(index, 112);
      *T0_frac = 0;
    }

  }

  else  /* second subframe */
  {
    /* find T0_min and T0_max for 2nd subframe */

    T0_min = g729_sub(*T0, 5);
    if (g729_sub(T0_min, pit_min) < 0)
    {
      T0_min = pit_min;
    }

    T0_max = g729_add(T0_min, 9);
    if (g729_sub(T0_max, pit_max) > 0)
    {
      T0_max = pit_max;
      T0_min = g729_sub(T0_max, 9);
    }

    /* i = (index+2)/3 - 1 */
    /* *T0 = i + t0_min;    */

    i = g729_sub(g729_mult(g729_add(index, 2), 10923), 1);
    *T0 = g729_add(i, T0_min);

    /* t0_frac = index - 2 - i*3; */

    i = g729_add(g729_add(i, i), i);
    *T0_frac = g729_sub(g729_sub(index, 2), i);
  }

  return;
}


