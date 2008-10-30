//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpDspUtilsIntSqrt_h_
#define _MpDspUtilsIntSqrt_h_

/**
*  @file
*
*  DO NOT INCLUDE THIS FILE DIRECTLY! This files is designed to be included
*  to <mp/MpDspUtils.h> and should not be used outside of it.
*
*  This code is taken from http://www.azillionmonkeys.com/qed/sqroot.html
*  (c) Copyright 1996-2006 by Paul Hsieh
*
*  Everything you ever wanted to know about square roots, but were afraid to ask. 
*/

/* ============================ INLINE METHODS ============================ */


#if !defined(_WIN32) || !defined(_MSC_VER)
// Use plain C implementation.
uint32_t MpDspUtils::sqrt(uint32_t val)
{
   uint32_t temp;
   uint32_t g = 0;

   if (val >= 0x40000000)
   {
      g = 0x8000; 
      val -= 0x40000000;
   }

#define INNER_ISQRT(s)                         \
   temp = (g << (s)) + (1 << ((s) * 2 - 2));   \
   if (val >= temp)                            \
   {                                           \
      g += 1 << ((s)-1);                       \
      val -= temp;                             \
   }

   INNER_ISQRT (15);
   INNER_ISQRT (14);
   INNER_ISQRT (13);
   INNER_ISQRT (12);
   INNER_ISQRT (11);
   INNER_ISQRT (10);
   INNER_ISQRT ( 9);
   INNER_ISQRT ( 8);
   INNER_ISQRT ( 7);
   INNER_ISQRT ( 6);
   INNER_ISQRT ( 5);
   INNER_ISQRT ( 4);
   INNER_ISQRT ( 3);
   INNER_ISQRT ( 2);

#undef INNER_ISQRT

   temp = g+g+1;
   if (val >= temp)
   {
      g++;
   }
   return g;
}
#else
// Use asm-optimized version

/* by Norbert Juffa */
static inline
__declspec (naked) uint32_t __stdcall MpDspUtils_sqrt(uint32_t x) {
   /* based on posting by Wilco Dijkstra in comp.sys.arm in 1996 */

   /*
   * For processors that provide single cycle latency
   * LEA, the first four instructions of the following 
   * macro could be replaced as follows
   * __asm   lea    edx, [eax+(1<<(N))]
   * __asm   lea    ebx, [eax+(2<<(N))]
   */
#if CPU == ATHLON || CPU == ATHLON64
#define iterasm(N) \
   __asm   mov    edx, (1 << (N))  \
   __asm   mov    ebx, (2 << (N))  \
   __asm   or     edx, eax         \
   __asm   or     ebx, eax         \
   __asm   shl    edx, (N)         \
   __asm   mov    esi, ecx         \
   __asm   sub    ecx, edx         \
   __asm   cmovnc eax, ebx         \
   __asm   cmovc  ecx, esi         
#else /* generic 386+ */
#define iterasm(N) \
   __asm   mov    edx, (1 << (N))  \
   __asm   mov    ebx, (2 << (N))  \
   __asm   or     edx, eax         \
   __asm   or     ebx, eax         \
   __asm   shl    edx, (N)         \
   __asm   sub    ecx, edx         \
   __asm   sbb    esi, esi         \
   __asm   sub    eax, ebx         \
   __asm   and    eax, esi         \
   __asm   add    eax, ebx         \
   __asm   and    edx, esi         \
   __asm   add    ecx, edx         
#endif

   __asm {
      mov    ecx, [esp+4] ; x
      push   ebx          ; save as per calling convention 
      push   esi          ; save as per calling convention 
      xor    eax, eax     ; 2*root
      /* iteration 15 */
      mov    ebx, (2 << (15))
      mov    esi, ecx        
      sub    ecx, (1 << (30))
      cmovnc eax, ebx                 
      cmovc  ecx, esi
      iterasm (14);
      iterasm (13);
      iterasm (12);
      iterasm (11);
      iterasm (10);
      iterasm (9);
      iterasm (8);
      iterasm (7);
      iterasm (6);
      iterasm (5);
      iterasm (4);
      iterasm (3);
      iterasm (2);
      iterasm (1);
      /* iteration 0 */
      mov    edx, 1
      mov    ebx, 2
      add    edx, eax
      add    ebx, eax
      sub    ecx, edx
      cmovnc eax, ebx
      shr    eax, 1
      mov    esi, [esp]   ; restore as per calling convention
      mov    ebx, [esp+4] ; restore as per calling convention
      add    esp, 8       ; remove temp variables
      ret    4            ; pop one DWORD arg and return
   }
#undef iterasm
}

uint32_t MpDspUtils::sqrt(uint32_t x)
{
   return MpDspUtils_sqrt(x);
}

#endif

#endif  // _MpDspUtilsIntSqrt_h_
