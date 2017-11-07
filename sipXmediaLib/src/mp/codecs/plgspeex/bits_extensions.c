//  
// Copyright (C) 2007-2017 SIPez LLC. All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// This file contains some utilities which extend the functionality of the
// libspeex/src/bits.c functions.  This have been integrated into the 
// sipXmediaLIb/contrib/libspeex, but when we use libspeex which is installed
// it may not have these functions included.

#include <speex/speex_bits.h>
#include <speex/speex.h>
#include <bits_extensions.h>

// From os_support.h
static inline void speex_notify(const char *str)
{
#ifdef TEST_PRINT
   fprintf (stderr, "notification: %s\n", str);
#endif
}


// From modes.h
#define SB_SUBMODE_BITS 3
#define NB_SUBMODE_BITS 4

// From nb_celp.c
static const int wb_skip_table[8] = {SB_SUBMODE_BITS+1, 36, 112, 192, 352, -1, -1, -1};

static const int inband_skip_table[16] =
   {1, 1, 4, 4, 4, 4, 4, 4, 8, 8, 16, 16, 32, 32, 64, 64 };


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

int speex_bits_get_num_frames(SpeexBits *bits)
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
