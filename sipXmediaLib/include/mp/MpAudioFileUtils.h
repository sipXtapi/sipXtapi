//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef MP_AUDIO_FILE_UTILS_H
#define MP_AUDIO_FILE_UTILS_H

#include <os/iostream>

typedef short AudioSample; 

// Utils functions to handle the audiofile 

/// Read int from Most Significant Bytes
long readIntMsb(istream &in, int bytes);

/// Most Significant Bytes: change bytes to int
long bytesToIntMsb(void *buff, int bytes);

/// Least Siginificant Bytes: read int
long readIntLsb(istream &in, int bytes);

/// Least significant byte: byte to int 
long bytesToIntLsb(void *buff, int bytes);

/// Skip one byte
void skipBytes(istream &in, int bytes);

/// write int in Most Significant bytes
void writeIntMsb(ostream &out, long l, int bytes);

/// Write int in Least Significant bytes
void writeIntLsb(ostream &out, long l, int bytes);


#endif
