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

#ifndef _rfc2190_h_
#define _rfc2190_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif // _MSC_VER

// RFC2190 H.263 RTP payload headers
#define H263_HEADER_COMMON \
   /* F: 1 bit \
    * The flag bit indicates the mode of the payload header. F=0, mode A; \
    * F=1, mode B or mode C depending on P bit defined below.  \
    */ \
   unsigned int f: 1; \
 \
   /* P: 1 bit \
    * Optional PB-frames mode as defined by the H.263. "0" implies \
    * normal I or P frame, "1" PB-frames. When F=1, P also indicates modes: \
    * mode B if P=0, mode C if P=1.  \
    */ \
   unsigned int p: 1; \
 \
   /* SBIT: 3 bits \
    * Start bit position specifies number of most significant bits that \
    * shall be ignored in the first data byte. \
    */ \
   unsigned int sbit: 3; \
 \
   /* EBIT: 3 bits \
    * End bit position specifies number of least significant bits that \
    * shall be ignored in the last data byte. \
    */ \
   unsigned int ebit: 3; 

// Mode A header
struct h263_header_a
{
   H263_HEADER_COMMON

   // SRC : 3 bits
   // Source format, bit 6,7 and 8 in PTYPE defined by H.263, specifies
   // the resolution of the current picture.
   unsigned int src: 3;

   // I:  1 bit.
   // Picture coding type, bit 9 in PTYPE defined by H.263, "0" is
   // intra-coded, "1" is inter-coded.
   unsigned int i: 1;

   // U: 1 bit
   // Set to 1 if the Unrestricted Motion Vector option, bit 10 in PTYPE
   // defined by H.263 was set to 1 in the current picture header,
   // otherwise 0.
   unsigned int u: 1;

   // S: 1 bit
   // Set to 1 if the Syntax-based Arithmetic Coding option, bit 11 in
   // PTYPE defined by the H.263 was set to 1 for current picture
   // header, otherwise 0.
   unsigned int s: 1;

   // A: 1 bit
   // Set to 1 if the Advanced Prediction option, bit 12 in PTYPE defined
   // by H.263 was set to 1 for current picutre header, otherwise 0.
   unsigned int a: 1;

   // R: 4 bits
   // Reserved, must be set to zero.
   unsigned int r: 4;

   // DBQ: 2 bits
   // Differential quantization parameter used to calculate quantizer for
   // the B frame based on quantizer for the P frame, when PB-frames option
   // is used. The value should be the same as DBQUANT defined by H.263.
   // Set to zero if PB-frames option is not used.
   unsigned int dbq: 2;

   // TRB: 3 bits
   // Temporal Reference for the B frame as defined by H.263. Set to
   // zero if PB-frames option is not used.
   unsigned int trb: 3;

   // TR: 8 bits
   // Temporal Reference for the P frame as defined by H.263. Set to
   // zero if the PB-frames option is not used.
   unsigned int tr: 8;

   void pack(void* bitstream) const;

   void unpack(const void* bitstream);
};

#define H263_HEADER_B \
   H263_HEADER_COMMON \
   /* SRC : 3 bits \
    * Source format, bit 6,7 and 8 in PTYPE defined by H.263, specifies \
    * the resolution of the current picture. \
    */ \
   unsigned int src: 3; \
 \
   /* QUANT: 5 bits \
    * Quantization value for the first MB coded at the starting of the \
    * packet.  Set to 0 if the packet begins with a GOB header. This is the \
    * equivalent of GQUANT defined by the H.263. \
    */ \
   unsigned int quant: 5; \
 \
   /*GOBN: 5 bits \
    *GOB number in effect at the start of the packet. GOB number is \
    *specified differently for different resolutions. See H.263 [4] for \
    *details. \
    */ \
   unsigned int gobn: 5; \
 \
   /*MBA: 9 bits \
    *The address within the GOB of the first MB in the packet, counting \
    *from zero in scan order. For example, the third MB in any GOB is \
    *given MBA = 2. \
    */ \
   unsigned int mba: 9; \
 \
   /*R: 2 bits \
    *Reserved, must be set to zero. \
    */ \
   unsigned int r: 2; \
 \
   /* I:  1 bit. \
    * Picture coding type, bit 9 in PTYPE defined by H.263, "0" is \
    * intra-coded, "1" is inter-coded. \
    */ \
   unsigned int i: 1; \
 \
   /* U: 1 bit \
    * Set to 1 if the Unrestricted Motion Vector option, bit 10 in PTYPE \
    * defined by H.263 was set to 1 in the current picture header, \
    * otherwise 0. \
    */ \
   unsigned int u: 1; \
 \
   /* S: 1 bit \
    * Set to 1 if the Syntax-based Arithmetic Coding option, bit 11 in \
    * PTYPE defined by the H.263 was set to 1 for current picture \
    * header, otherwise 0. \
    */ \
   unsigned int s: 1; \
 \
   /* A: 1 bit \
    * Set to 1 if the Advanced Prediction option, bit 12 in PTYPE defined \
    * by H.263 was set to 1 for current picutre header, otherwise 0. \
    */ \
   unsigned int a: 1; \
 \
   /* HMV1, VMV1: 7 bits each. \
    * Horizontal and vertical motion vector predictors for the first MB in \
    * this packet [4]. When four motion vectors are used for current MB \
    * with advanced prediction option, these would be the motion vector \
    * predictors for block number 1 in the MB. Each 7 bits field encodes a \
    * motion vector predictor in half pixel resolution as a 2's complement \
    * number. \
    */ \
   unsigned int hmv1: 7; \
   unsigned int vmv1: 7; \
 \
   /* HMV2, VMV2: 7 bits each. \
    * Horizontal and vertical motion vector predictors for block number 3 \
    * in the first MB in this packet when four motion vectors are used with \
    * the advanced prediction option. This is needed because block number 3 \
    * in the MB needs different motion vector predictors from other blocks \
    * in the MB. These two fields are not used when the MB only has one \
    * motion vector. See the H.263 [4] for block organization in a \
    * macroblock.  Each 7 bits field encodes a motion vector predictor in \
    * half pixel resolution as a 2's complement number. \
    */ \
   unsigned int hmv2: 7; \
   unsigned int vmv2: 7;

// Mode B header
struct h263_header_b
{
   H263_HEADER_B

   void pack(void* bitstream) const;

   void unpack(const void* bitstream);
};

// Mode C header
struct h263_header_c
{
   H263_HEADER_B

   // RR field takes 19 bits, and is currently
   // reserved.  It must be set to zero.
   unsigned int rr: 19;

   // DBQ: 2 bits
   // Differential quantization parameter used to calculate quantizer for
   // the B frame based on quantizer for the P frame, when PB-frames option
   // is used. The value should be the same as DBQUANT defined by H.263.
   // Set to zero if PB-frames option is not used.
   unsigned int dbq: 2;

   // TRB: 3 bits
   // Temporal Reference for the B frame as defined by H.263. Set to
   // zero if PB-frames option is not used.
   unsigned int trb: 3;

   // TR: 8 bits
   // Temporal Reference for the P frame as defined by H.263. Set to
   // zero if the PB-frames option is not used.
   unsigned int tr: 8;

   void pack(void* bitstream, size_t size) const;
};

struct h263_payload_header
{
   // Picture Start Code (PSC) (22 bits)
   // PSC is a word of 22 bits. Its value is 0000 0000 0000 0000 1 00000. All picture start codes shall be
   // byte aligned. This shall be achieved by inserting PSTUF bits as necessary before the start code such
   // that the first bit of the start code is the first (most significant) bit of a byte.
   unsigned int psc: 22;

   // Temporal Reference (TR) (8 bits)
   // The value of TR is formed by incrementing its value in the temporally previous reference picture
   // header by one plus the number of skipped or non-reference pictures at the picture clock frequency
   // since the previously transmitted one. The interpretation of TR depends on the active picture clock
   // frequency. Under the standard CIF picture clock frequency, TR is an 8-bit number which can have
   // 256 possible values. The arithmetic is performed with only the eight LSBs. If a custom picture
   // clock frequency is signalled in use, Extended TR in 5.1.8 and TR form a 10-bit number where TR
   // stores the eight Least Significant Bits (LSBs) and ETR stores the two Most Significant Bits
   // (MSBs). The arithmetic in this case is performed with the ten LSBs. In the optional PB-frames or
   // Improved PB-frames mode, TR only addresses P-pictures; for the temporal reference for the
   // B-picture part of PB or Improved PB frames, refer to 5.1.22.
   unsigned int tr: 8;

   struct
   {
      // Always "1", in order to avoid start code emulation.
      unsigned int bit1: 1; 
      // Always "0", for distinction with ITU-T Rec. H.261.
      unsigned int bit2: 1;
      // Split screen indicator, "0" off, "1" on.
      unsigned int ssi: 1;
      // Document camera indicator, "0" off, "1" on.
      unsigned int dci: 1;
      // Full Picture Freeze Release, "0" off, "1" on.
      unsigned int fpfr: 1;
      // Source Format, "000" forbidden, "001" sub-QCIF, "010" QCIF, "011" CIF,
      // "100" 4CIF, "101" 16CIF, "110" reserved, "111" extended PTYPE.
      unsigned int sf: 3;

      // If bits 6-8 are not equal to "111", which indicates an extended PTYPE (PLUSPTYPE), the
      // following five bits are also present in PTYPE:

      // Picture Coding Type, "0" INTRA (I-picture), "1" INTER (P-picture).
      unsigned int pct: 1;
      // Optional Unrestricted Motion Vector mode (see Annex D), "0" off, "1" on.
      unsigned int umv: 1;
      // Optional Syntax-based Arithmetic Coding mode (see Annex E), "0" off, "1" on.
      unsigned int sac: 1;
      // Optional Advanced Prediction mode (see Annex F), "0" off, "1" on.
      unsigned int ap: 1;
      // Optional PB-frames mode (see Annex G), "0" normal I- or P-picture, "1"
      // PB-frame.
      unsigned int pfm: 1;
   } ptype;

   void unpack(const void* bitstream, size_t size);
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif // _MSC_VER

enum h263_mode
{
   h263_mode_a,
   h263_mode_b,
   h263_mode_c,
};

// @see ITU-T Rec. H.263 (01/2005) p. 23
enum h263_src
{
   h263_src_forbidden,  // 000
   h263_src_SQCIF,      // 001
   h263_src_QCIF,       // 010
   h263_src_CIF,        // 011
   h263_src_4CIF,       // 100
   h263_src_16CIF,      // 101
   h263_src_reserved,   // 110
   h263_src_extended    // 111
};

h263_mode h263_query_mode(const void* header);

inline unsigned int h263_header_length(h263_mode mode)
{
   switch (mode) {
   case h263_mode_a: return 4;
   case h263_mode_b: return 8;
   case h263_mode_c: return 12;
   default:
      return 0;
   }
}

unsigned int  h263_header_length(const void* header);

// Test whether payload starts with picture start code (h263_psc)
// or GOB start code (GBSC). In case of GBSC, the payload is a continuation
// of previous frame.
bool h263_is_psc_payload(const void* payload, size_t size);

// Start Code identifying full picture start (as opposed to 
// GOB start code, which includes nonzero group number in
// lowest 5 bits).
static const unsigned int h263_psc = 0x20;

static const unsigned int h263_timestamp_freq = 90000;

#endif  // _rfc2190_h_
