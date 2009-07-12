/* Copyright (C) 2002 Jean-Marc Valin 
   File: speex_bits.c

   Handles bit packing/unpacking

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../include/speex/speex_bits.h"
#include "arch.h"
#include "os_support.h"
#include "modes.h"
#include "nb_celp.h"

/* Maximum size of the bit-stream (for fixed-size allocation) */
#ifndef MAX_CHARS_PER_FRAME
#define MAX_CHARS_PER_FRAME (2000/BYTES_PER_CHAR)
#endif

static const int inband_skip_table[16] =
   {1, 1, 4, 4, 4, 4, 4, 4, 8, 8, 16, 16, 32, 32, 64, 64 };

EXPORT void speex_bits_init(SpeexBits *bits)
{
   bits->chars = (char*)speex_alloc(MAX_CHARS_PER_FRAME);
   if (!bits->chars)
      return;

   bits->buf_size = MAX_CHARS_PER_FRAME;

   bits->owner=1;

   speex_bits_reset(bits);
}

EXPORT void speex_bits_init_buffer(SpeexBits *bits, void *buff, int buf_size)
{
   bits->chars = (char*)buff;
   bits->buf_size = buf_size;

   bits->owner=0;

   speex_bits_reset(bits);
}

EXPORT void speex_bits_set_bit_buffer(SpeexBits *bits, void *buff, int buf_size)
{
   bits->chars = (char*)buff;
   bits->buf_size = buf_size;

   bits->owner=0;

   bits->nbBits=buf_size<<LOG2_BITS_PER_CHAR;
   bits->charPtr=0;
   bits->bitPtr=0;
   bits->overflow=0;
   
}

EXPORT void speex_bits_destroy(SpeexBits *bits)
{
   if (bits->owner)
      speex_free(bits->chars);
   /* Will do something once the allocation is dynamic */
}

EXPORT void speex_bits_reset(SpeexBits *bits)
{
   /* We only need to clear the first byte now */
   bits->chars[0]=0;
   bits->nbBits=0;
   bits->charPtr=0;
   bits->bitPtr=0;
   bits->overflow=0;
}

EXPORT void speex_bits_rewind(SpeexBits *bits)
{
   bits->charPtr=0;
   bits->bitPtr=0;
   bits->overflow=0;
}

EXPORT void speex_bits_read_from(SpeexBits *bits, const char *chars, int len)
{
   int i;
   int nchars = len / BYTES_PER_CHAR;
   if (nchars > bits->buf_size)
   {
      speex_notify("Packet is larger than allocated buffer");
      if (bits->owner)
      {
         char *tmp = (char*)speex_realloc(bits->chars, nchars);
         if (tmp)
         {
            bits->buf_size=nchars;
            bits->chars=tmp;
         } else {
            nchars=bits->buf_size;
            speex_warning("Could not resize input buffer: truncating input");
         }
      } else {
         speex_warning("Do not own input buffer: truncating oversize input");
         nchars=bits->buf_size;
      }
   }
#if (BYTES_PER_CHAR==2)
/* Swap bytes to proper endian order (could be done externally) */
#define HTOLS(A) ((((A) >> 8)&0xff)|(((A) & 0xff)<<8))
#else
#define HTOLS(A) (A)
#endif
   for (i=0;i<nchars;i++)
      bits->chars[i]=HTOLS(chars[i]);

   bits->nbBits=nchars<<LOG2_BITS_PER_CHAR;
   bits->charPtr=0;
   bits->bitPtr=0;
   bits->overflow=0;
}

static void speex_bits_flush(SpeexBits *bits)
{
   int nchars = ((bits->nbBits+BITS_PER_CHAR-1)>>LOG2_BITS_PER_CHAR);
   if (bits->charPtr>0)
      SPEEX_MOVE(bits->chars, &bits->chars[bits->charPtr], nchars-bits->charPtr);
   bits->nbBits -= bits->charPtr<<LOG2_BITS_PER_CHAR;
   bits->charPtr=0;
}

EXPORT void speex_bits_read_whole_bytes(SpeexBits *bits, const char *chars, int nbytes)
{
   int i,pos;
   int nchars = nbytes/BYTES_PER_CHAR;

   if (((bits->nbBits+BITS_PER_CHAR-1)>>LOG2_BITS_PER_CHAR)+nchars > bits->buf_size)
   {
      /* Packet is larger than allocated buffer */
      if (bits->owner)
      {
         char *tmp = (char*)speex_realloc(bits->chars, (bits->nbBits>>LOG2_BITS_PER_CHAR)+nchars+1);
         if (tmp)
         {
            bits->buf_size=(bits->nbBits>>LOG2_BITS_PER_CHAR)+nchars+1;
            bits->chars=tmp;
         } else {
            nchars=bits->buf_size-(bits->nbBits>>LOG2_BITS_PER_CHAR)-1;
            speex_warning("Could not resize input buffer: truncating oversize input");
         }
      } else {
         speex_warning("Do not own input buffer: truncating oversize input");
         nchars=bits->buf_size;
      }
   }

   speex_bits_flush(bits);
   pos=bits->nbBits>>LOG2_BITS_PER_CHAR;
   for (i=0;i<nchars;i++)
      bits->chars[pos+i]=HTOLS(chars[i]);
   bits->nbBits+=nchars<<LOG2_BITS_PER_CHAR;
}

EXPORT int speex_bits_write(SpeexBits *bits, char *chars, int max_nbytes)
{
   int i;
   int max_nchars = max_nbytes/BYTES_PER_CHAR;
   int charPtr, bitPtr, nbBits;

   /* Insert terminator, but save the data so we can put it back after */
   bitPtr=bits->bitPtr;
   charPtr=bits->charPtr;
   nbBits=bits->nbBits;
   speex_bits_insert_terminator(bits);
   bits->bitPtr=bitPtr;
   bits->charPtr=charPtr;
   bits->nbBits=nbBits;

   if (max_nchars > ((bits->nbBits+BITS_PER_CHAR-1)>>LOG2_BITS_PER_CHAR))
      max_nchars = ((bits->nbBits+BITS_PER_CHAR-1)>>LOG2_BITS_PER_CHAR);

   for (i=0;i<max_nchars;i++)
      chars[i]=HTOLS(bits->chars[i]);
   return max_nchars*BYTES_PER_CHAR;
}

EXPORT int speex_bits_write_whole_bytes(SpeexBits *bits, char *chars, int max_nbytes)
{
   int max_nchars = max_nbytes/BYTES_PER_CHAR;
   int i;
   if (max_nchars > ((bits->nbBits)>>LOG2_BITS_PER_CHAR))
      max_nchars = ((bits->nbBits)>>LOG2_BITS_PER_CHAR);
   for (i=0;i<max_nchars;i++)
      chars[i]=HTOLS(bits->chars[i]);

   if (bits->bitPtr>0)
      bits->chars[0]=bits->chars[max_nchars];
   else
      bits->chars[0]=0;
   bits->charPtr=0;
   bits->nbBits &= (BITS_PER_CHAR-1);
   return max_nchars*BYTES_PER_CHAR;
}

EXPORT void speex_bits_pack(SpeexBits *bits, int data, int nbBits)
{
   unsigned int d=data;

   if (bits->charPtr+((nbBits+bits->bitPtr)>>LOG2_BITS_PER_CHAR) >= bits->buf_size)
   {
      speex_notify("Buffer too small to pack bits");
      if (bits->owner)
      {
         int new_nchars = ((bits->buf_size+5)*3)>>1;
         char *tmp = (char*)speex_realloc(bits->chars, new_nchars);
         if (tmp)
         {
            bits->buf_size=new_nchars;
            bits->chars=tmp;
         } else {
            speex_warning("Could not resize input buffer: not packing");
            return;
         }
      } else {
         speex_warning("Do not own input buffer: not packing");
         return;
      }
   }

   while(nbBits)
   {
      int bit;
      bit = (d>>(nbBits-1))&1;
      bits->chars[bits->charPtr] |= bit<<(BITS_PER_CHAR-1-bits->bitPtr);
      bits->bitPtr++;

      if (bits->bitPtr==BITS_PER_CHAR)
      {
         bits->bitPtr=0;
         bits->charPtr++;
         bits->chars[bits->charPtr] = 0;
      }
      bits->nbBits++;
      nbBits--;
   }
}

EXPORT int speex_bits_unpack_signed(SpeexBits *bits, int nbBits)
{
   unsigned int d=speex_bits_unpack_unsigned(bits,nbBits);
   /* If number is negative */
   if (d>>(nbBits-1))
   {
      d |= (-1)<<nbBits;
   }
   return d;
}

EXPORT unsigned int speex_bits_unpack_unsigned(SpeexBits *bits, int nbBits)
{
   unsigned int d=0;
   if ((bits->charPtr<<LOG2_BITS_PER_CHAR)+bits->bitPtr+nbBits>bits->nbBits)
      bits->overflow=1;
   if (bits->overflow)
      return 0;
   while(nbBits)
   {
      d<<=1;
      d |= (bits->chars[bits->charPtr]>>(BITS_PER_CHAR-1 - bits->bitPtr))&1;
      bits->bitPtr++;
      if (bits->bitPtr==BITS_PER_CHAR)
      {
         bits->bitPtr=0;
         bits->charPtr++;
      }
      nbBits--;
   }
   return d;
}

EXPORT unsigned int speex_bits_peek_unsigned(SpeexBits *bits, int nbBits)
{
   unsigned int d=0;
   int bitPtr, charPtr;
   char *chars;

   if ((bits->charPtr<<LOG2_BITS_PER_CHAR)+bits->bitPtr+nbBits>bits->nbBits)
     bits->overflow=1;
   if (bits->overflow)
      return 0;

   bitPtr=bits->bitPtr;
   charPtr=bits->charPtr;
   chars = bits->chars;
   while(nbBits)
   {
      d<<=1;
      d |= (chars[charPtr]>>(BITS_PER_CHAR-1 - bitPtr))&1;
      bitPtr++;
      if (bitPtr==BITS_PER_CHAR)
      {
         bitPtr=0;
         charPtr++;
      }
      nbBits--;
   }
   return d;
}

EXPORT int speex_bits_peek(SpeexBits *bits)
{
   if ((bits->charPtr<<LOG2_BITS_PER_CHAR)+bits->bitPtr+1>bits->nbBits)
      bits->overflow=1;
   if (bits->overflow)
      return 0;
   return (bits->chars[bits->charPtr]>>(BITS_PER_CHAR-1 - bits->bitPtr))&1;
}

EXPORT void speex_bits_advance(SpeexBits *bits, int n)
{
    if (((bits->charPtr<<LOG2_BITS_PER_CHAR)+bits->bitPtr+n>bits->nbBits) || bits->overflow){
      bits->overflow=1;
      return;
    }
   bits->charPtr += (bits->bitPtr+n) >> LOG2_BITS_PER_CHAR; /* divide by BITS_PER_CHAR */
   bits->bitPtr = (bits->bitPtr+n) & (BITS_PER_CHAR-1);       /* modulo by BITS_PER_CHAR */
}

EXPORT int speex_bits_remaining(SpeexBits *bits)
{
   if (bits->overflow)
      return -1;
   else
      return bits->nbBits-((bits->charPtr<<LOG2_BITS_PER_CHAR)+bits->bitPtr);
}

EXPORT int speex_bits_nbytes(SpeexBits *bits)
{
   return ((bits->nbBits+BITS_PER_CHAR-1)>>LOG2_BITS_PER_CHAR);
}

EXPORT void speex_bits_insert_terminator(SpeexBits *bits)
{
   if (bits->bitPtr)
      speex_bits_pack(bits, 0, 1);
   while (bits->bitPtr)
      speex_bits_pack(bits, 1, 1);
}

static int speex_bits_skip_wb_layers(SpeexBits *bits)
{
   unsigned int submode;
   unsigned int advance;

   /* skip up to two wideband frames */
   if (  (speex_bits_remaining(bits) >= 4)
      && speex_bits_unpack_unsigned(bits, 1))
   {
      submode = speex_bits_unpack_unsigned(bits, 3);
      advance = wb_skip_table[submode];
      if (advance < 0)
      {
         speex_notify("Invalid mode encountered. The stream is corrupted.");
         return -2;
      } 
/*      printf("\tWB layer skipped: %d bits\n", advance); */
      advance -= (SB_SUBMODE_BITS+1);
      speex_bits_advance(bits, advance);

      if ( (speex_bits_remaining(bits) >= 4)
         && speex_bits_unpack_unsigned(bits, 1))
      {
         submode = speex_bits_unpack_unsigned(bits, 3);
         advance = wb_skip_table[submode];
         if (advance < 0)
         {
            speex_notify("Invalid mode encountered. The stream is corrupted.");
            return -2;
         } 
/*         printf("\tWB layer skipped: %d bits\n", advance); */
         advance -= (SB_SUBMODE_BITS+1);
         speex_bits_advance(bits, advance);

         if ( (speex_bits_remaining(bits) >= 4)
            && speex_bits_unpack_unsigned(bits, 1))
         {
            /* too many in a row */
            speex_notify("More than two wideband layers found. The stream is corrupted.");
            return -2;
         }
      }

   }
   return 0;
}

EXPORT int speex_bits_get_num_frames(SpeexBits *bits)
{
   int frame_count = 0;
   unsigned int submode;

/*   printf("speex_get_samples(%d)\n", speex_bits_nbytes(bits)); */
   while (speex_bits_remaining(bits) >= 5) {
      /* skip wideband frames */
      if (speex_bits_skip_wb_layers(bits) < 0)
      {
/*         printf("\tERROR reading wideband frames\n"); */
         break;
      }

      if (speex_bits_remaining(bits) < 4)
      {
/*         printf("\tEND of stream\n"); */
         break;
      }

      /* get control bits */
      submode = speex_bits_unpack_unsigned(bits, 4);
/*      printf("\tCONTROL: %d at %d\n", submode, bits->charPtr*8+bits->bitPtr); */

      if (submode == 15){
/*         printf("\tTERMINATOR\n"); */
         break;
      } else if (submode == 14) {
         /* in-band signal; next 4 bits contain signal id */
         submode = speex_bits_unpack_unsigned(bits, 4);
/*         printf("\tIN-BAND %d bits\n", SpeexInBandSz[submode]); */
         speex_bits_advance(bits, inband_skip_table[submode]);
      } else if (submode == 13) {
         /* user in-band; next 5 bits contain msg len */
         submode = speex_bits_unpack_unsigned(bits, 5);
/*         printf("\tUSER-BAND %d bytes\n", submode); */
         speex_bits_advance(bits, submode * 8);
      } else if (submode > 8) {
/*         printf("\tUNKNOWN sub-mode %d\n", submode); */
         break;
      } else {
         /* NB frame; skip number bits for submode (less the 5 control bits) */
         unsigned int advance = submode;
         speex_mode_query(&speex_nb_mode, SPEEX_SUBMODE_BITS_PER_FRAME, &advance);
         if (advance < 0)
         {
/*            printf("Invalid mode encountered. The stream is corrupted."); */
            return -2;
         } 
/*         printf("\tSUBMODE %d: %d bits\n", submode, advance); */
         advance -= (NB_SUBMODE_BITS+1);
         speex_bits_advance(bits, advance);

         frame_count += 1;
      }
   }

/*   printf("\tFRAMES: %d\n", frame_count); */
   return frame_count;
}
