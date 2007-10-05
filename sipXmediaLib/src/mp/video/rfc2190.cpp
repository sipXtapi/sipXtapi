//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 Wirtualna Polska S.A. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Andrzej Ciarkowski <andrzejc AT wp-sa DOT pl>

#ifdef SIPX_VIDEO 

#include "mp/video/rfc2190.h"

extern "C" {
#  include <bitstream.h>
}

#include <assert.h>
#include <stddef.h>

void h263_payload_header::unpack(const void* bitstream, size_t size)
{
   GetBitContext gbc;
   init_get_bits(&gbc, (const uint8_t*)bitstream, size * 8);
   psc = get_bits_long(&gbc, 22);
   tr = get_bits_long(&gbc, 8);

   ptype.bit1 = get_bits1(&gbc);
   ptype.bit2 = get_bits1(&gbc);
   ptype.ssi  = get_bits1(&gbc);
   ptype.dci  = get_bits1(&gbc);
   ptype.fpfr = get_bits1(&gbc);
   ptype.sf   = get_bits(&gbc, 3);
   ptype.pct  = get_bits1(&gbc);
   ptype.umv  = get_bits1(&gbc);
   ptype.sac  = get_bits1(&gbc);
   ptype.ap   = get_bits1(&gbc);
   ptype.pfm  = get_bits1(&gbc);
}

h263_mode h263_query_mode(const void* header)
{
   assert(NULL != header);
   GetBitContext g;
   init_get_bits(&g, (const uint8_t*)header, 32);

   int f = get_bits1(&g);
   if (!f)
      return h263_mode_a;

   int p = get_bits1(&g);
   if (p)
      return h263_mode_c;
   else
      return h263_mode_b;
}

unsigned int h263_header_length(const void* header)
{
   return h263_header_length(h263_query_mode(header));
}

bool h263_is_psc_payload(const void* payload, size_t size)
{
   assert(NULL != payload);
   h263_payload_header p;
   p.unpack(payload, size);
   return h263_psc == p.psc;
}

void h263_header_a::pack(void *bitstream) const
{
   PutBitContext pb;
   init_put_bits(&pb, (uint8_t*)bitstream, 64);

   put_bits(&pb, 1, f);
   put_bits(&pb, 1, p);
   put_bits(&pb, 3, sbit);
   put_bits(&pb, 3, ebit);
   put_bits(&pb, 3, src);
   put_bits(&pb, 1, i);
   put_bits(&pb, 1, u);
   put_bits(&pb, 1, s);
   put_bits(&pb, 1, a);
   put_bits(&pb, 4, r);
   put_bits(&pb, 2, dbq);
   put_bits(&pb, 3, trb);
   put_bits(&pb, 8, tr);

   assert(32 == put_bits_count(&pb));
}

#define H263_UNPACK_COMMON(gb) \
   f = get_bits1(&gb); \
   p = get_bits1(&gb); \
   sbit = get_bits(&gb, 3); \
   ebit = get_bits(&gb, 3)


void h263_header_a::unpack(const void *bitstream)
{
   GetBitContext gb;
   init_get_bits(&gb, (const uint8_t*)bitstream, 32);

   H263_UNPACK_COMMON(gb);

   src = get_bits(&gb, 3);
   i = get_bits1(&gb);
   u = get_bits1(&gb);
   s = get_bits1(&gb);
   a = get_bits1(&gb);
   r = get_bits(&gb, 4);
   dbq = get_bits(&gb, 2);
   trb = get_bits(&gb, 3);
   tr = get_bits(&gb, 8);
}

void h263_header_b::unpack(const void *bitstream)
{
   GetBitContext gb;
   init_get_bits(&gb, (const uint8_t*)bitstream, 64);

   H263_UNPACK_COMMON(gb);

   src = get_bits(&gb, 3);
   quant = get_bits(&gb, 5);
   gobn = get_bits(&gb, 5);
   mba = get_bits(&gb, 9);
   r = get_bits(&gb, 2);
   i = get_bits1(&gb);
   u = get_bits1(&gb);
   s = get_bits1(&gb);
   a = get_bits1(&gb);
   hmv1 = get_bits(&gb, 7);
   vmv1 = get_bits(&gb, 7);
   hmv2 = get_bits(&gb, 7);
   vmv2 = get_bits(&gb, 7);
}

#endif // SIPX_VIDEO
