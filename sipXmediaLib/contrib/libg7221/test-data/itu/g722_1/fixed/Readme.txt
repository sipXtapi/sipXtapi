            ==============================================
                ITU-T Recommendation G.722.1 (05/2005)
                       Test Vector Read-me File
                             Release 2.1
            ==============================================

COPYRIGHT AND INTELLECTUAL PROPERTY
===================================
**   (c) 1999, 2005 PictureTel Corporation, Polycom
**          Andover, MA, USA  
**
**	    All rights reserved.

ITU CONTACTS
============
Online availablity:
http://www.itu.int/rec/T-REC-G.722.1

For distribution of updated software, please contact:
Sales Department
ITU
Place des Nations
CH-1211 Geneve 20
SWITZERLAND
email: sales@itu.int

For reporting problems, please contact the SG 16 secretariat at:
ITU-T SG 16 Secretariat
ITU
Place des Nations
CH-1211 Geneve 20
SWITZERLAND
fax: +41 22 730 5853
email: tsbsg16@itu.int

TECHNICAL CONTACT
=================
Minjie Xie, Ph.D.
Senior Audio Engineer
Video Division
Polycom, Inc.
Burlington, MA 01803
USA
Tel  : +1 781 270 0159
email: Minjie.Xie@polycom.com


SOFTWARE NOTES FOR ITU-T RECOMMENDATION G.722.1 FIXED POINT TEST VECTORS
========================================================================

The test vectors in this directory consist of input and output signals
(vectors) to the encoder and decoder.  Each input vector provided has
a corresponding output vector.  Any implementation of G.722.1 using an
input vector should be able to produce a bit exact output when
compared to the corresponding output vector.

These test vectors have been taken from the 1999 version of the fixed
point C code and have been shown to work properly with the 05/2005
version of the fixed point C code.


Warning: Successfully passing the test vectors does not guarantee full
         compliance with this recommendation.  This is because the
         test vectors do not exercise every state and line of the
         code.


The following table describes the purpose of each test vector file
provided.


Vector name                        Comments
-----------                        --------


g722_1_enc_in.pcm                Input vector to encoder.

g722_1_enc_out_24000.pak         Bit stream output from the encoder
                                 in packed format at 24kbit/s,
                                 requires use of input
                                 g722_1_enc_in.pcm. 

g722_1_enc_out_32000.pak         Bit stream output from the encoder
                                 in packed format at 32kbit/s,
                                 requires use of  input
                                 g722_1_enc_in.pcm.           

g722_1_enc_out_24000.itu         Bit stream output from the encoder
                                 in ITU-T G.192 format at 24kbit/s,
                                 requires use of  input
                                 g722_1_enc_in.pcm.           

g722_1_enc_out_32000.itu         Bit stream output from the encoder
                                 in ITU-T G.192 format at 32kbit/s,
                                 requires use of  input
                                 g722_1_enc_in.pcm.           

g722_1_dec_out_24000.pcm         Decoder output vector at 24kbit/s,
                                 requires use of input vector
                                 g722_1_enc_out_24000.pak (or
                                 g722_1_enc_out_24000.itu)
          
g722_1_dec_out_32000.pcm         Decoder output vector at 32kbit/s,
                                 requires use of input vector
                                 g722_1_enc_out_32000.pak (or
                                 g722_1_enc_out_32000.itu) 

g722_1_dec_in_24000_fe.itu       Input vector to decoder at 24kbit/s
                                 uses ITU-T G.192 bit stream format,
                                 it includes erased frames to test
                                 frame erasure code.

g722_1_dec_in_32000_fe.itu       Input vector to decoder at 32kbit/s
                                 uses ITU-T G.192 bit stream format,
                                 it includes erased frames to test
                                 frame erasure code.

g722_1_dec_out_24000_fe.pcm      Decoder output vector at 24kbit/s,
                                 requires use of frame erasure input
                                 vector g722_1_dec_in_24000_fe.itu.

g722_1_dec_out_32000_fe.pcm      Decoder output vector at 32kbit/s,
                                 requires use of frame erasure input 
                                 vector g722_1_dec_in_32000_fe.itu.



Note.  All test vector byte ordering is in a PC DOS format found on IBM
       compatible PCs.



As an example, to test the encoder at 24000 bit/s you may type the
following command line:
  
    encode 0 G722_1_enc_in.pcm  output_file 24000

and compare the "output_file" against the vector
"g722_1_enc_out_24000.pak".  It should be indentical.


Similarly, to test the decoder at 24000 bit/s you may type the
following command line:
  
    decode 0 g722_1_enc_out_24000.pak  output_file 24000

and compare the "output_file" against the vector
"g722_1_dec_out_24000.pcm".  It should be indentical.



Background To Test Vector Design
================================

The test vectors are designed to exercise as much of the G.722.1
algorithm functionality as possible.  The input test vector to the
encoder (g722_1_test_vector.pcm) was optimized for 24kbit/s, and
designed to access as many of the encoder/decoder table entries as
possible (this algorithm essentially consists of table look up
processes).  The same vector may be used for 32kbit/s also - the only
difference being in the bit rate entry, the table entries and code are
otherwise identical at both 24 and 32kbit/s.

The test vector g722_1_test_vector.pcm at 24 kbit/s provides the code
coverage results shown in the following table.  The coverage is shown
as the table (variable) size and the number (and %) of elements in
that table which are indexed by the input vector.

Code Coverage of Variables at 24kbit/s
--------------------------------------
 
Table Name                              Size    Accessed 
----------                              ----    -------- 

Encoder:
differential_region_power_bits          336     312 (92%) 
differential_region_power_codes         336     312 (92%) 
enc_vqm_while1                          2       2 (100%) 
enc_vqm_while2                          2       2 (100%)         
mlt_sqvh_bitcount_category_0            196     181 (92%) 
mlt_sqvh_code_category_0                196     181 (92%) 
mlt_sqvh_bitcount_category_1            100     94 (94%) 
mlt_sqvh_code_category_1                100     94 (94%) 
mlt_sqvh_bitcount_category_2            49      48 (97%) 
mlt_sqvh_code_category_2                49      48 (97%) 
mlt_sqvh_bitcount_category_3            625     520 (83%) 
mlt_sqvh_code_category_3                625     520 (83%) 
mlt_sqvh_bitcount_category_4            256     203 (79%) 
mlt_sqvh_code_category_4                256     203 (79%) 
mlt_sqvh_bitcount_category_5            243     192 (79%) 
mlt_sqvh_code_category_5                243     192 (79%) 
mlt_sqvh_bitcount_category_6            32      32 (100%) 
mlt_sqvh_code_category_6                32      32 (100%) 

Decoder: 
differential_region_power_decoder_tree  644     598 (92%) 
mlt_decoder_tree_category_0             360     360 (100%) 
mlt_decoder_tree_category_1             186     186 (100%) 
mlt_decoder_tree_category_2             94      94 (100%) 
mlt_decoder_tree_category_3             1038    1038 (100%) 
mlt_decoder_tree_category_4             416     410 (98%) 
mlt_decoder_tree_category_5             382     382 (100%) 
mlt_decoder_tree_category_6             62      62 (100%) 
 

The test vector g722_1_test_vector.pcm at 32 kbit/s provides the code
coverage results shown in the following table.  The coverage is shown
as the table (variable) size and the number (and %) of elements in
that table which are indexed by the input vector.


Code Coverage of Variables at 32kbit/s
--------------------------------------

Table Name                              Size    Accessed 
----------                              ----    -------- 

Encoder:
differential_region_power_bits          336     312 (92%) 
differential_region_power_codes         336     312 (92%) 
enc_vqm_while1                          2       2 (100%) 
enc_vqm_while2                          2       2 (100%) 
mlt_sqvh_bitcount_category_0            196     181 (92%) 
mlt_sqvh_code_category_0                196     181 (92%) 
mlt_sqvh_bitcount_category_1            100     73 (73%) 
mlt_sqvh_code_category_1                100     73 (73%) 
mlt_sqvh_bitcount_category_2            49      47 (95%) 
mlt_sqvh_code_category_2                49      47 (95%) 
mlt_sqvh_bitcount_category_3            625     396 (63%) 
mlt_sqvh_code_category_3                625     396 (63%) 
mlt_sqvh_bitcount_category_4            256     58 (22%) 
mlt_sqvh_code_category_4                256     58 (22%) 
mlt_sqvh_bitcount_category_5            243     32 (13%) 
mlt_sqvh_code_category_5                243     32 (13%) 
mlt_sqvh_bitcount_category_6            32      29 (90%) 
mlt_sqvh_code_category_6                32      29 (90%) 
 
Decoder:
differential_region_power_decoder_tree  644     598 (92%) 
mlt_decoder_tree_category_0             360     360 (100%) 
mlt_decoder_tree_category_1             186     134 (72%) 
mlt_decoder_tree_category_2             94      91 (96%) 
mlt_decoder_tree_category_3             1038    569 (54%) 
mlt_decoder_tree_category_4             416     67 (16%) 
mlt_decoder_tree_category_5             382     69 (18%) 
mlt_decoder_tree_category_6             62      57 (91%) 

Last update: 2008-06-26 
--end     
