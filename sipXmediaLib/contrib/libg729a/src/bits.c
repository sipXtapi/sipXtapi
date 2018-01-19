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

/**
 *  Portions of this file are derived from the following GPL notice:
 *
 *  g729a codec for iPhone and iPod Touch
 *  Copyright (C) 2009 Samuel <samuelv0304@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include "g729a_typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

#include "get_bits.h"
#include "put_bits.h"

/* prototypes for local functions */
static void  int2bin(G729_Word16 value, G729_Word16 no_of_bits, G729_Word16 *bitstream);
static G729_Word16   bin2int(G729_Word16 no_of_bits, G729_Word16 *bitstream);

/*----------------------------------------------------------------------------
 * g729_prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * g729_bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters are:
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *----------------------------------------------------------------------------
 */
void g729_prm2bits_ld8k(
        G729_Word16   prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
        G729_Word16 bits[]            /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = 80 */
        )
{
    G729_Word16 i;
    *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */
    *bits++ = SIZE_WORD;     /* bit[1], to be compatible with hardware      */

    for (i = 0; i < PRM_SIZE; i++)
    {
        int2bin(prm[i], g729_bitsno[i], bits);
        bits += g729_bitsno[i];
    }

    return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
        G729_Word16 value,             /* input : decimal value         */
        G729_Word16 no_of_bits,        /* input : number of bits to use */
        G729_Word16 *bitstream         /* output: bitstream             */
        )
{
    G729_Word16 *pt_bitstream;
    G729_Word16   i, bit;

    pt_bitstream = bitstream + no_of_bits;

    for (i = 0; i < no_of_bits; i++)
    {
        bit = value & (G729_Word16)0x0001;      /* get lsb */
        if (bit == 0)
            *--pt_bitstream = BIT_0;
        else
            *--pt_bitstream = BIT_1;
        value >>= 1;
    }
}

/*----------------------------------------------------------------------------
 *  g729_bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void g729_bits2prm_ld8k(
        G729_Word16 bits[],            /* input : serial bits (80)                       */
        G729_Word16   prm[]            /* output: decoded parameters (11 parameters)     */
        )
{
    G729_Word16 i;
    for (i = 0; i < PRM_SIZE; i++)
    {
        prm[i] = bin2int(g729_bitsno[i], bits);
        bits  += g729_bitsno[i];
    }

}

/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static G729_Word16 bin2int(       /* output: decimal value of bit pattern */
        G729_Word16 no_of_bits,          /* input : number of bits to read       */
        G729_Word16 *bitstream           /* input : array containing bits        */
        )
{
    G729_Word16   value, i;
    G729_Word16 bit;

    value = 0;
    for (i = 0; i < no_of_bits; i++)
    {
        value <<= 1;
        bit = *bitstream++;
        if (bit == BIT_1)  value += 1;
    }
    return(value);
}

void g729_prm2bits_ld8k_compressed(
    G729_Word16 prm[],            /* input : encoded parameters  (PRM_SIZE parameters)  */
    G729_UWord8 bits[]            /* output: serial bits (SERIAL_SIZE )*/
)
{
    PutBitContext pb;
    int i;

    init_put_bits(&pb, bits, 10);

    for (i = 0; i < PRM_SIZE; ++i)
    {
        put_bits(&pb, g729_bitsno[i], prm[i]);
    }
    flush_put_bits(&pb);
}

void g729_bits2prm_ld8k_compressed(
    G729_UWord8  bits[],            /* input : serial bits (80)                       */
    G729_Word16  prm[]              /* output: decoded parameters (11 parameters)     */
)
{
    GetBitContext gb;
    int i;

    init_get_bits(&gb, bits, 10 /*buf_size*/);

    for (i = 0; i < PRM_SIZE; ++i)
    {
        prm[i] = get_bits(&gb, g729_bitsno[i]);
    }
}
