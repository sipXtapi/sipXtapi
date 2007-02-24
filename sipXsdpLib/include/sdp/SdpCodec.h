//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2007 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SdpCodec_h_
#define _SdpCodec_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlString.h>


// DEFINES

// Mime major types
#define MIME_TYPE_AUDIO "audio"
#define MIME_TYPE_VIDEO "video"

// Mime Sub types
#define MIME_SUBTYPE_PCMU "PCMU"
#define MIME_SUBTYPE_PCMA "PCMA"
#define MIME_SUBTYPE_G729A "G729"
#define MIME_SUBTYPE_G729AB "G729"
#define MIME_SUBTYPE_G729ACISCO7960 "G729a"
#define MIME_SUBTYPE_G723 "G723"
#define MIME_SUBTYPE_DTMF_TONES "telephone-event"
#define MIME_SUBTYPE_IPCMU "EG711U"
#define MIME_SUBTYPE_IPCMA "EG711A"
#define MIME_SUBTYPE_IPCMWB "IPCMWB"
#define MIME_SUBTYPE_ILBC "iLBC"
#define MIME_SUBTYPE_ISAC "ISAC"
#define MIME_SUBTYPE_GSM "GSM"
#define MIME_SUBTYPE_SPEEX "speex"
#define MIME_SUBTYPE_VP71 "VP71"
#define MIME_SUBTYPE_IYUV "IYUV"
#define MIME_SUBTYPE_I420 "I420"
#define MIME_SUBTYPE_RGB24 "RGB24"
#define MIME_SUBTYPE_H263 "H263"

// Bandwidth requirements for SDP Codecs
#define SDP_CODEC_BANDWIDTH_VARIABLE 0
#define SDP_CODEC_BANDWIDTH_LOW      1
#define SDP_CODEC_BANDWIDTH_NORMAL   2
#define SDP_CODEC_BANDWIDTH_HIGH     3

// Video formats - must be bitmap values
#define SDP_VIDEO_FORMAT_SQCIF       0x0001
#define SDP_VIDEO_FORMAT_QCIF        0x0002
#define SDP_VIDEO_FORMAT_CIF         0x0004
#define SDP_VIDEO_FORMAT_QVGA        0x0008

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Container for SDP/RTP codec specification
/**
*  This class holds the SDP definition of a codec
*  Included in this information is: sample rate,
*  number of channels, the mapping from an internal
*  pingtel codec id to the public SDP format and RTP
*  payload type id.
*
*  This is the base class.  Specific codec types
*  should implement sub classes which define the
*  codec specific parameters.  All specific codec
*  types MUST be registered with the SdpCodecFactory
*  to be useable.  Generally codecs are constructed
*  ONLY by the SdpCodecFactory.
*
*  The method that we have been using on
*  SdpCodec::getCodecType() retrieved
*  the static codec type/id.  I have defined
*  an enum in SdpCodec which contains the
*  current values as well as some additional
*  ones.  The idea is that these are private,
*  internally assigned ids to the codecs we
*  support.
*
*  I have added a new method
*  SdpCodec::getCodecPayloadFormat()
*  which returns the RTP payload id to be
*  used in RTP and the SDP.  For static
*  codec id the returned value for both of these
*  methods would typically be the same.
*  However for the dynamic codecs they
*  will mostly be different.
*
*  The intent is that eventually we will
*  support a factory which will allow
*  registration of new codec types.
*/
class SdpCodec : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// Unique identifier used for each supported codec
    /**
    *  Note it is possible that the format id/type used
    *  in the SDP "m" field and RTP header is different
    *  than these internally used ids.
    */
    enum SdpCodecTypes
    {
        SDP_CODEC_UNKNOWN = -1,
        SDP_CODEC_PCMU = 0,        ///< G.711 mu-law
        SDP_CODEC_GSM = 3,         ///< GSM codec
        SDP_CODEC_G723 = 4,
        SDP_CODEC_PCMA = 8,        ///< G.711 a-law
        SDP_CODEC_L16_STEREO = 10, ///< PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_L16_MONO = 11,   ///< PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_G729 = 18,       ///< G.729, with or without Annexes A or B
        SDP_CODEC_H263 = 34,       ///< H.263 video codec
        SDP_CODEC_MAXIMUM_STATIC_CODEC = 95,
        SDP_CODEC_SPEEX = 110,     ///< Speex, mono 8bits 8000samples/sec
        SDP_CODEC_SPEEX_5 = 111,   ///< Speex Profile 1
        SDP_CODEC_SPEEX_15 = 112,  ///< Speex Profile 2
        SDP_CODEC_SPEEX_24 = 113,  ///< Speex Profile 3
        SDP_CODEC_TONES = 128,     ///< AVT/DTMF Tones, RFC 2833
        SDP_CODEC_G729A = 129,
        SDP_CODEC_G7221 = 130,     ///< Siren
        SDP_CODEC_G7231 = 131,
        SDP_CODEC_L16_8K = 132,    ///< Mono PCM 16 bit/sample 8000 samples/sec.
        SDP_CODEC_G729AB = 133,
        SDP_CODEC_G729ACISCO7960 = 134,

         // Range for 3rd party add in codec types
        SDP_CODEC_3RD_PARTY_START = 256,
        SDP_CODEC_GIPS_PCMA  = 257,
        SDP_CODEC_GIPS_PCMU  = 258,
        SDP_CODEC_GIPS_IPCMA = 259,
        SDP_CODEC_GIPS_IPCMU = 260,
        SDP_CODEC_GIPS_IPCMWB = 261,
        SDP_CODEC_ILBC = 262,
        SDP_CODEC_GIPS_ISAC = 263,
        SDP_CODEC_VP71_CIF = 264,
        SDP_CODEC_VP71_QCIF = 265,
        SDP_CODEC_VP71_SQCIF = 266,
        SDP_CODEC_VP71_QVGA = 267,
        SDP_CODEC_IYUV_CIF = 268,
        SDP_CODEC_IYUV_QCIF = 269,
        SDP_CODEC_IYUV_SQCIF = 270,
        SDP_CODEC_IYUV_QVGA = 271,
        SDP_CODEC_I420_CIF = 272,
        SDP_CODEC_I420_QCIF = 273,
        SDP_CODEC_I420_SQCIF = 274,
        SDP_CODEC_I420_QVGA = 275,
        SDP_CODEC_RGB24_CIF = 276,
        SDP_CODEC_RGB24_QCIF = 277,
        SDP_CODEC_RGB24_SQCIF = 278,
        SDP_CODEC_RGB24_QVGA = 279,
        SDP_CODEC_H263_CIF = 280,
        SDP_CODEC_H263_QCIF = 281,
        SDP_CODEC_H263_SQCIF = 282,
        SDP_CODEC_H263_QVGA = 283,
        SDP_CODEC_3RD_PARTY_END = 511
    };


    /// Identifies the relative CPU cost for a SDP Codec.
    enum SdpCodecCPUCost
    {
       SDP_CODEC_CPU_LOW = 0,
       SDP_CODEC_CPU_HIGH = 1
    };


/* ============================ CREATORS ================================== */
///@name Creators
//@{

     ///Default constructor
   SdpCodec(enum SdpCodecTypes sdpCodecType = SDP_CODEC_UNKNOWN,
            int payloadFormat = -1,
            const char* mimeType = MIME_TYPE_AUDIO,
            const char* mimeSubtype = "",
            int sampleRate = 8000,             ///< samples per second
            int preferredPacketLength = 20000, ///< micro seconds
            int numChannels = 1,
            const char* formatSpecificData = "",
            const int CPUCost = SDP_CODEC_CPU_LOW,
            const int BWCost = SDP_CODEC_BANDWIDTH_NORMAL,
            const int videoFormat = SDP_VIDEO_FORMAT_QCIF,
            const int videoFmtp = 0);

   SdpCodec(int payloadFormat,
            const char* mimeType,
            const char* mimeSubType,
            int sampleRate,
            int preferredPacketLength,
            int numChannels,
            const char* formatSpecificData);

     ///Copy constructor
   SdpCodec(const SdpCodec& rSdpCodec);

     ///Destructor
   virtual
   ~SdpCodec();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     ///Assignment operator
   SdpCodec& operator=(const SdpCodec& rhs);

   UtlCopyableContainable* clone() const { return new SdpCodec(*this); }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   /// Get the internal/Pingtel codec type id
   enum SdpCodecTypes getCodecType() const;
   /**<
   *  Note it is possible that the format id/type used
   *  in the SDP "m" field and RTP header is different
   *  than these internally used ids.
   */

   /// Get the SDP/RTP payload id to be used for this codec
   int getCodecPayloadFormat() const;
   /**<
   *  This is the id used in the SDP "m" format sub-field
   *  and RTP header.
   */

   /// Set the SDP/RTP payload id to be used for this codec
   void setCodecPayloadFormat(int formatId);

   /// Get the format specific parameters for the SDP
   virtual void getSdpFmtpField(UtlString& formatSpecificData) const;
   /**<
   *  This is what goes in the SDP "a" field in the
   *  format: "a=fmtp <payloadFormat> <formatSpecificData>
   */

   /// Get the media type for the codec
   void getMediaType(UtlString& mimeMajorType) const;
   /**<
   *  This is the mime major type (i.e. video, audio, etc)
   */

   /// MimeSubtype used as encoding name
   void getEncodingName(UtlString& mimeSubtype) const;
   /**<
   *  This is the encoding name used in the SDP
   *  "a=rtpmap: <payloadFormat> <mimeSubtype/sampleRate[/numChannels]"
   *  field.
   */

   /// Get the number of samples per second
   int getSampleRate() const;

   /// Return the video format
   int getVideoFormat() const;

   /// Get the number of channels
   int getNumChannels() const;

   /// Get the preferred packet size in micro seconds
   int getPacketLength() const;
   /**<
   *  Get the preferred (not mandated) packet
   *  size.  This measure is in microseconds and
   *  is independent of whether this is frame or
   *  sample based codec
   */

   /// Get a string dump of this codecs definition
   void toString(UtlString& sdpCodecContents) const;

   ///Get the CPU cost for this codec.
   int getCPUCost() const;
   /**<
   *  @returns SDP_CODEC_CPU_HIGH or SDP_CODEC_CPU_LOW
   */

   ///Get the bandwidth cost for this codec.
   int getBWCost() const;
   /**<
   *  @returns SDP_CODEC_BANDWIDTH_HIGH, SDP_CODEC_BANDWIDTH_NORMAL or
   *           SDP_CODEC_BANDWIDTH_LOW
   */

   ///Get the video format bitmap
   int getVideoFmtp() const;

   ///Set the video format bitmap
   void setVideoFmtp(const int videoFmtp);

   /// Get the video format string
   void getVideoFmtpString(UtlString& fmtpString) const;

   /// Set the video format string
   void setVideoFmtpString(int videoFmtp);

   /// Clears the format string
   void clearVideoFmtpString();

   ///Set the packet size
   void setPacketSize(const int packetSize);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   /// Returns TRUE if this codec is the same definition as the given codec
   UtlBoolean isSameDefinition(SdpCodec& codec) const;
   /**<
   *  That is the encoding type and its characteristics, not the payload type.
   */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

//    enum SdpCodecTypes mCodecType; ///< Internal id
    int mCodecPayloadFormat;       ///< The id which appears in SDP & RTP
    UtlString mMimeType;           ///< audio, video, etc.
    UtlString mMimeSubtype;        ///< a=rtpmap mime subtype value
    int mSampleRate;               ///< samples per second
    int mPacketLength;             ///< micro seconds
    int mNumChannels;
    UtlString mFormatSpecificData; ///< a=fmtp parameter
    int mCPUCost;                  ///< relative cost of a SDP codec
    int mBWCost;
    int mVideoFormat;
    int mVideoFmtp;
    UtlString mVideoFmtpString;    ///< video format string

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodec_h_
