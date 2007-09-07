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

#ifndef _MpVideoStreamParams_h_
#define _MpVideoStreamParams_h_

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

/**
*  @brief Container for parameters of video stream.
*
*  Stores properties of video stream, like frame size, frame rate (fps),
*  bitrate etc.
*
*  @nosubgrouping
*/
class MpVideoStreamParams
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

 /* =============================== CREATORS =============================== */
///@name Creators
//@{

   MpVideoStreamParams()
   : mFrameWidth(0)
   , mFrameHeight(0)
   , mFrameRate(0.f)
   , mStreamBitrate(0)
   , mQuality(0)
   {
   }

    ~MpVideoStreamParams()
    {}

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   inline void setFrameSize(int width, int height);
   inline void setFrameWidth(int width);
   inline void setFrameHeight(int height);
   inline void setFrameRate(float fps);
   inline void setStreamBitrate(int bitrate);
   inline void setQuality(int quality);
   void setFrameFormat(int frameFormat);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   inline void getFrameSize(int& width, int& height) const;
   inline int getFrameWidth() const;
   inline int getFrameHeight() const;
   inline float getFrameRate() const;
   inline int getStreamBitrate() const;
   inline int getQuality() const;
   inline int getFrameFormat() const;

   static int getFrameFormat(int width, int height);
   static void getFrameSize(int frameFormat, int& width, int& height);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   int mFrameWidth;
   int mFrameHeight;
   float mFrameRate;
   int mStreamBitrate;
   int mQuality;
};

/* ============================ INLINE METHODS ============================ */

void MpVideoStreamParams::setFrameSize(int width, int height)
{
   mFrameWidth = width;
   mFrameHeight = height;
}

void MpVideoStreamParams::setFrameWidth(int width)
{
   mFrameWidth = width;
}

void MpVideoStreamParams::setFrameHeight(int height)
{
   mFrameHeight = height;
}

void MpVideoStreamParams::setFrameRate(float fps)
{
   mFrameRate = fps;
}

void MpVideoStreamParams::setStreamBitrate(int bitrate)
{
   mStreamBitrate = bitrate;
}

void MpVideoStreamParams::setQuality(int quality)
{
   mQuality = quality;
}

void MpVideoStreamParams::getFrameSize(int& width, int& height) const
{
   width = mFrameWidth;
   height = mFrameHeight;
}

int MpVideoStreamParams::getFrameWidth() const
{
   return mFrameWidth;
}

int MpVideoStreamParams::getFrameHeight() const
{
   return mFrameHeight;
}

float MpVideoStreamParams::getFrameRate() const
{
   return mFrameRate;
}

int MpVideoStreamParams::getStreamBitrate() const
{
   return mStreamBitrate;
}

int MpVideoStreamParams::getQuality() const
{
   return mQuality;
}

int MpVideoStreamParams::getFrameFormat() const
{
   return getFrameFormat(mFrameWidth, mFrameHeight);
}

#endif // _MpVideoStreamParams_h_
