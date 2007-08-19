//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
#ifndef _JB_API_H_
#define _JB_API_H_

#define JB_EXTERN extern

#include "mp/JB/jb_typedefs.h"
#include "mp/MpTypes.h"
#include "mp/MpRtpBuf.h"

#ifdef __cplusplus
extern "C" {
#endif
 

JB_EXTERN JB_ret JB_RecIn(MpJitterBuffer *MpJitterBuffer, 
                          MpRtpBufPtr &rtpPacket);

JB_EXTERN JB_ret JB_RecOut(MpJitterBuffer *MpJitterBuffer,
                           MpAudioSample *voiceSamples,
                           JB_size *length);

#ifdef __cplusplus
}
#endif

#endif /* _JB_API_H_ */
