//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#define MIME_TYPE_APPLICATION "application"

// Mime Sub types
#define MIME_SUBTYPE_PCMU "PCMU"
#define MIME_SUBTYPE_PCMA "PCMA"
#define MIME_SUBTYPE_G729A "G729"
#define MIME_SUBTYPE_G729AB "G729"
#define MIME_SUBTYPE_G729ACISCO7960 "G729a"
#define MIME_SUBTYPE_G723 "G723"
#define MIME_SUBTYPE_G726_16 "G726-16"
#define MIME_SUBTYPE_G726_24 "G726-24"
#define MIME_SUBTYPE_G726_32 "G726-32"
#define MIME_SUBTYPE_G726_40 "G726-40"
#define MIME_SUBTYPE_G722    "G722"
#define MIME_SUBTYPE_G7221   "G7221"
#define MIME_SUBTYPE_DTMF_TONES "telephone-event"
#define MIME_SUBTYPE_IPCMU "EG711U"
#define MIME_SUBTYPE_IPCMA "EG711A"
#define MIME_SUBTYPE_IPCMWB "IPCMWB"
#define MIME_SUBTYPE_ILBC "iLBC"
#define MIME_SUBTYPE_ISAC "ISAC"
#define MIME_SUBTYPE_GSM "GSM"
#define MIME_SUBTYPE_GSM_WAVE "GSM_WAVE"
#define MIME_SUBTYPE_OPUS "opus"
#define MIME_SUBTYPE_SPEEX "speex"
#define MIME_SUBTYPE_AMR "amr"
#define MIME_SUBTYPE_AMRWB "amr-wb"
#define MIME_SUBTYPE_AAC_LC "aac_lc"
#define MIME_SUBTYPE_MPEG4_GENERIC "mpeg4-generic"
#define MIME_SUBTYPE_VP71 "VP71"
#define MIME_SUBTYPE_IYUV "IYUV"
#define MIME_SUBTYPE_I420 "I420"
#define MIME_SUBTYPE_RGB24 "RGB24"
#define MIME_SUBTYPE_H263 "H263"
#define MIME_SUBTYPE_H263_1998 "H263-1998"
#define MIME_SUBTYPE_H264 "H264"

// Canonical internal case represation
#define SDP_MIME_TO_CASE toLower
#define SDP_MIME_SUBTYPE_TO_CASE toUpper

// Bandwidth requirements for SDP Codecs
#define SDP_CODEC_BANDWIDTH_VARIABLE 0
#define SDP_CODEC_BANDWIDTH_LOW      1
#define SDP_CODEC_BANDWIDTH_NORMAL   2
#define SDP_CODEC_BANDWIDTH_HIGH     3
#define SDP_CODEC_BANDWIDTH_VERY_HIGH 4

// Video formats - must be bitmap values
#define SDP_VIDEO_FORMAT_SQCIF       0x0001
#define SDP_VIDEO_FORMAT_QCIF        0x0002
#define SDP_VIDEO_FORMAT_CIF         0x0004
#define SDP_VIDEO_FORMAT_QVGA        0x0008
#define SDP_VIDEO_FORMAT_VGA         0x0010
#define SDP_VIDEO_FORMAT_NTSC        0x0020
#define SDP_VIDEO_FORMAT_4CIF        0x0040
#define SDP_VIDEO_FORMAT_EDTV        0x0080
#define SDP_VIDEO_FORMAT_QHD         0x0100
#define SDP_VIDEO_FORMAT_HD720       0x0200
#define SDP_VIDEO_FORMAT_16CIF       0x0400
#define SDP_VIDEO_FORMAT_HD1088      0x0800

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Container for SDP/RTP codec specification
/**
*  This class holds the SDP definition of a codec. Included information is:
*  sample rate,  number of channels, the mapping from an internal codec id
*  to the public SDP format and RTP payload type id.
*
*  This is the base class. Specific codec types may implement sub classes
*  which define the codec specific parameters.  All specific codec types
*  MUST be registered with the SdpCodecFactory to be usable.  Generally
*  codecs are constructed ONLY by the SdpCodecFactory.
*
*  The method that is used on SdpCodec::getCodecType() retrieves the static
*  codec type/id. Now there is defined an enum in SdpCodec which contains
*  the current values as well as some additional ones. The idea is that these
*  are private, internally assigned ids to the codecs we support.
*
*  A new method SdpCodec::getCodecPayloadFormat() has been added which returns
*  the RTP payload id to be used in RTP and the SDP. For static codec ID, the
*  returned value for both of these methods would typically be the same,
*  however for the dynamic codecs they will mostly be different.
*
*  The intent is that eventually we will support a factory which will allow
*  registration of new codec types.
*/
class SdpCodec : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// Unique identifier used for each supported codec
    /**
    *  Note it is possible that the format id/type used in the SDP "m" field
    *  and RTP header is different than these internally used ids.
    */
    enum SdpCodecTypes
    {
        SDP_CODEC_UNKNOWN = -1,
        SDP_CODEC_PCMU = 0,        ///< G.711 mu-law
        SDP_CODEC_GSM = 3,         ///< GSM codec
        SDP_CODEC_G723 = 4,        ///< G.723
        SDP_CODEC_PCMA = 8,        ///< G.711 a-law
        SDP_CODEC_G722 = 9,        ///< G.722
        SDP_CODEC_L16_44100_STEREO = 10, ///< PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_L16_44100_MONO = 11,   ///< PCM 16 bit/sample 44100 samples/sec.
        SDP_CODEC_G729 = 18,       ///< G.729, with or without Annexes A or B
        SDP_CODEC_H263 = 34,       ///< H.263 video codec
        SDP_CODEC_MAXIMUM_STATIC_CODEC = 95, ///< Payload types for codecs below
                                   ///< will be assigned dynamically.

        SDP_CODEC_SPEEX,           ///< Speex NB,  8,000bps
        SDP_CODEC_SPEEX_5,         ///< Speex NB,  5,950bps
        SDP_CODEC_SPEEX_15,        ///< Speex NB, 15,000bps
        SDP_CODEC_SPEEX_24,        ///< Speex NB, 24,600bps
        SDP_CODEC_SPEEX_WB,        ///< Speex WB   9,800bps (enum 100)
        SDP_CODEC_SPEEX_WB_5,      ///< Speex WB   5,750bps
        SDP_CODEC_SPEEX_WB_21,     ///< Speex WB  20,600bps
        SDP_CODEC_SPEEX_WB_42,     ///< Speex WB  42,200bps
        SDP_CODEC_SPEEX_UWB,       ///< Speex UWB 11,600bps
        SDP_CODEC_SPEEX_UWB_8,     ///< Speex UWB  7,550bps
        SDP_CODEC_SPEEX_UWB_22,    ///< Speex UWB 22,400bps
        SDP_CODEC_SPEEX_UWB_44,    ///< Speex UWB 44,000bps

        SDP_CODEC_ILBC,            ///< Internet Low Bit Rate Codec, 30ms (RFC3951)
        SDP_CODEC_ILBC_20MS,       ///< Internet Low Bit Rate Codec, 20ms (RFC3951)
        SDP_CODEC_TONES,           ///< AVT/DTMF Tones, RFC 2833
        SDP_CODEC_G729A,           ///< G.729A
        SDP_CODEC_G7221,           ///< Siren
        SDP_CODEC_G7221_24,        ///< Siren/G.722.1 (16k sps) 24Kb
        SDP_CODEC_G7221_32,        ///< Siren/G.722.1 (16k sps) 32Kb
        SDP_CODEC_G7221_C_24,      ///< Siren/G.722.1 Annex C (32k sps) 24Kb
        SDP_CODEC_G7221_C_32,      ///< Siren/G.722.1 Annex C (32k sps) 32Kb
        SDP_CODEC_G7221_C_48,      ///< Siren/G.722.1 Annex C (32k sps) 48Kb
        SDP_CODEC_G7231,           ///< G.723.1
        SDP_CODEC_AMR,             ///< AMR Bandwidth Efficient mode
        SDP_CODEC_AMR_ALIGNED,     ///< AMR Octet Aligned mode
        SDP_CODEC_AMRWB,           ///< AMR-WB Bandwidth Efficient mode
        SDP_CODEC_AMRWB_ALIGNED,   ///< AMR-WB Octet Aligned mode
        SDP_CODEC_L16_8000_STEREO, ///< PCM 16 bit/sample 8000 samples/sec stereo.
        SDP_CODEC_L16_8000_MONO,   ///< PCM 16 bit/sample 8000 samples/sec mono.
        SDP_CODEC_L16_11025_STEREO, ///< PCM 16 bit/sample 11025 samples/sec stereo.
        SDP_CODEC_L16_11025_MONO,   ///< PCM 16 bit/sample 11025 samples/sec mono.
        SDP_CODEC_L16_16000_STEREO, ///< PCM 16 bit/sample 16000 samples/sec stereo.
        SDP_CODEC_L16_16000_MONO,   ///< PCM 16 bit/sample 16000 samples/sec mono.
        SDP_CODEC_L16_22050_STEREO, ///< PCM 16 bit/sample 22050 samples/sec stereo.
        SDP_CODEC_L16_22050_MONO,   ///< PCM 16 bit/sample 22050 samples/sec mono. (enum 125)
        SDP_CODEC_L16_24000_STEREO, ///< PCM 16 bit/sample 24000 samples/sec stereo.
        SDP_CODEC_L16_24000_MONO,   ///< PCM 16 bit/sample 24000 samples/sec mono.
        SDP_CODEC_L16_32000_STEREO, ///< PCM 16 bit/sample 32000 samples/sec stereo.
        SDP_CODEC_L16_32000_MONO,   ///< PCM 16 bit/sample 32000 samples/sec mono.
        SDP_CODEC_L16_48000_STEREO, ///< PCM 16 bit/sample 48000 samples/sec stereo.
        SDP_CODEC_L16_48000_MONO,   ///< PCM 16 bit/sample 48000 samples/sec mono.
        SDP_CODEC_G729AB,          ///< G.729A
        SDP_CODEC_G729ACISCO7960,
        SDP_CODEC_G726_16,
        SDP_CODEC_G726_24,
        SDP_CODEC_G726_32,
        SDP_CODEC_G726_40,
        SDP_CODEC_AAC_LC_32000,
        SDP_CODEC_MPEG4_GENERIC_AAC_LC_16000,
        SDP_CODEC_MPEG4_GENERIC_AAC_LC_32000,
        SDP_CODEC_MPEG4_GENERIC_AAC_LC_48000,
        SDP_CODEC_OPUS, ///< Default OPUS VoIP codec upto 48000 samples/sec

        /// GIPS specific codecs
        //@{
        SDP_CODEC_GIPS_IPCMA,
        SDP_CODEC_GIPS_IPCMU,
        SDP_CODEC_GIPS_IPCMWB,
        SDP_CODEC_GIPS_ISAC,
        //@}

        /// Video codecs
        //@{
        SDP_CODEC_VP71_CIF,
        SDP_CODEC_VP71_QCIF,
        SDP_CODEC_VP71_SQCIF,
        SDP_CODEC_VP71_QVGA, // (ENUM 150)
        SDP_CODEC_IYUV_CIF,
        SDP_CODEC_IYUV_QCIF,
        SDP_CODEC_IYUV_SQCIF,
        SDP_CODEC_IYUV_QVGA,
        SDP_CODEC_I420_CIF,
        SDP_CODEC_I420_QCIF,
        SDP_CODEC_I420_SQCIF,
        SDP_CODEC_I420_QVGA,
        SDP_CODEC_RGB24_CIF,
        SDP_CODEC_RGB24_QCIF,
        SDP_CODEC_RGB24_SQCIF,
        SDP_CODEC_RGB24_QVGA,
        SDP_CODEC_H263_CIF,
        SDP_CODEC_H263_QCIF,
        SDP_CODEC_H263_SQCIF,
        SDP_CODEC_H263_QVGA,
        SDP_CODEC_H264, // (enum 166)
        SDP_CODEC_H264_CIF_256,
        SDP_CODEC_H264_NTSC_256,
        SDP_CODEC_H264_4CIF_512,
        SDP_CODEC_H264_EDTV_512,
        SDP_CODEC_H264_QHD_1024,
        SDP_CODEC_H264_HD720_1024,
        SDP_CODEC_H264_HD720_2048,
        SDP_CODEC_H264_HD1088_2048,
        SDP_CODEC_H264_HD1088_4096, // (enum 175)
        SDP_CODEC_H264_PM1_CIF_256,
        SDP_CODEC_H264_PM1_NTSC_256,
        SDP_CODEC_H264_PM1_4CIF_512,
        SDP_CODEC_H264_PM1_EDTV_512,
        SDP_CODEC_H264_PM1_QHD_1024,
        SDP_CODEC_H264_PM1_HD720_1024,
        SDP_CODEC_H264_PM1_HD720_2048,
        SDP_CODEC_H264_PM1_HD1088_2048,
        SDP_CODEC_H264_PM1_HD1088_4096
        //@}
    };


    /// Identifies the relative CPU cost for a SDP Codec.
    enum SdpCodecCPUCost
    {
       SDP_CODEC_CPU_LOW = 0,
       SDP_CODEC_CPU_HIGH = 1,
       SDP_CODEC_CPU_VERY_HIGH = 2
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
            const SdpCodecCPUCost CPUCost = SDP_CODEC_CPU_LOW,
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

   /// Set the SDP/RTP payload id to be used for this codec
   void setCodecPayloadFormat(int formatId);

   /// Set the video format bitmap
   void setVideoFmtp(const int videoFmtp);

   /// Set the video format string
   void setVideoFmtpString(int videoFmtp);

   /// Clears the format string
   void clearVideoFmtpString();

   ///Set the packet size
   void setPacketSize(const int packetSize);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   /// Get the internal/sipX codec type id
   enum SdpCodecTypes getCodecType() const;
   /**<
   *  @note It is possible that the format id/type used in the SDP "m" field
   *        and RTP header is different than these internally used ids.
   */

   /// Get the SDP/RTP payload id to be used for this codec
   int getCodecPayloadFormat() const;
   /**<
   *  This is the id used in the SDP "m" format sub-field and RTP header.
   */

   /// Get the format specific parameters for the SDP
   virtual void getSdpFmtpField(UtlString& formatSpecificData) const;
   /**<
   *  This is what goes in the SDP "a" field in the format:
   *  "a=fmtp <payloadFormat> <formatSpecificData>"
   */

   /// Set the format specific parameters for the SDP
   virtual void setSdpFmtpField(const UtlString& formatSpecificData);
   /**<
   *  This is what goes in the SDP "a" field in the format:
   *  "a=fmtp <payloadFormat> <formatSpecificData>"
   */

   /// Get the named parameter in the format field (fmtp parameter)
   virtual UtlBoolean getFmtpParameter(const UtlString& parameterName, UtlString& parameterValue, char nameValueSeperator = '=') const;

   /// Get the named parameter from the given format field (fmtp parameter)
   static UtlBoolean getFmtpParameter(const UtlString& fmtpField, const UtlString& parameterName, UtlString& parameterValue, char nameValueSeperator = '=');
   /**
    *  Assumes the "a=fmtp:<payloadType>" of the fmtp field has be stripped off
    * "a=fmtp:<payloadType> <fmtpdata>"
    */

   /// Get the named parameter from the given format field (fmtp parameter)
   static UtlBoolean getFmtpParameter(const UtlString& fmtpField, const UtlString& parameterName, int& parameterValue,
           char nameValueSeperator = '=');
   /**
    *  Assumes the "a=fmtp:<payloadType>" of the fmtp field has be stripped off
    * "a=fmtp:<payloadType> <fmtpdata>"
    */

   /// Get the video video sizes from the FMTP parameter
   static OsStatus getVideoSizes(const UtlString& fmtpField, int maxSizes, int& numSizes, int videoSizes[]);
   /**
    *  Assumes the "a=fmtp:<payloadType>" of the fmtp field has be stripped off
    * "a=fmtp:<payloadType> <fmtpdata>"
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
   *  Get the preferred (not mandated) packet size. This measure is
   *  in microseconds and is independent of whether this is frame or
   *  sample based codec
   */

   /// Get a string dump of this codecs definition
   void toString(UtlString& sdpCodecContents) const;

   ///Get the CPU cost for this codec.
   SdpCodecCPUCost getCPUCost() const;
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

   /// Get the video format string
   void getVideoFmtpString(UtlString& fmtpString) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// Returns TRUE if this codec is the same definition as the given codec
    UtlBoolean isSameDefinition(const SdpCodec& codec) const;
    /**<
    *  That is the encoding type and its characteristics, not the payload type.
    */

    /// Returns TRUE if the named fmtp parameters are the same in this and the given codec
    UtlBoolean isFmtpParameterSame(const SdpCodec& codec, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue, const char nameValueSeperater = '=') const;
    /*
     * @param codec - codec to compare fmpt parameter with this codec's
     * @param fmtpParameterName - name of the parameter in the fmtp to compare
     * @param fmtpParameterDefaultValue - default value of the parameter to assume if it is not set
     */

    /// Returns TRUE if the named fmtp parameters are the same in this and the given fmtp field string
    UtlBoolean isFmtpParameterSame(const UtlString& fmtp, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue, const char nameValueSeperater = '=') const;
    /*
     * @param fmpt = SDP fmpt field value string containing zero or more parameters and values
     * @param fmtpParameterName - name of the parameter in the fmtp to compare
     * @param fmtpParameterDefaultValue - default value of the parameter to assume if it is not set
     */

    static UtlBoolean isFmtpParameterSame(const UtlString& fmtp1, const UtlString& fmtp2, const UtlString& fmtpParameterName, const UtlString& fmtpParameterDefaultValue, const char nameValueSeperater = '=');

    // Check fmtp parameters for specific mime type to see if they are equivalent
    UtlBoolean compareFmtp(const SdpCodec& codec, int& compares) const;
    /**
     *  @param compares greater than, equal or less than zero based upon how the fmtp
     *       parameters compare
     *  @returns TRUE/FALSE if the codecs are compatible
     */

    // Check fmtp parameters for specific mime type to see if they are equivalent
    UtlBoolean compareFmtp(const UtlString& fmtp, int& compares) const;
    /**
     *  Assumes mime type of provided fmtp string is same as this.
     *
     *  @param compares greater than, equal or less than zero based upon how the fmtp
     *       parameters compare
     *  @returns TRUE/FALSE if the codecs are compatible
     */

    // Check fmtp parameters for specific mime type to see if they are equivalent
    static UtlBoolean compareFmtp(const UtlString& mimeType, const UtlString& mimeSubtype, const UtlString& fmpt1, const UtlString& fmtp2, int& compares);
    /**
     *  @param mimeType - Mime major type
     *  @param mimeSubtype - Mime sub type
     *  @param fmpt1 - fmtp string from 1st codec (Must have "a:fmtp <payloadId>" removed)
     *  @param fmpt2 - fmtp string from 2nd codec (Must have "a:fmtp <payloadId>" removed)
     *  @param compares greater than, equal or less than zero based upon how the fmtp
     *       parameters compare
     *  @returns TRUE/FALSE if the codecs are compatible
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
    SdpCodecCPUCost mCPUCost;      ///< relative cost of a SDP codec
    int mBWCost;
    int mVideoFormat;
    int mVideoFmtp;
    UtlString mVideoFmtpString;    ///< video format string

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodec_h_
