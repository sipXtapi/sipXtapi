//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

/**
 * @mainpage sipXtapi SDK Overview
 * 
 * @htmlinclude sipXtapi-overview.html
 */ 

/** 
 * @file sipXtapi.h
 *
 * sipXtapi main API declarations
 **/

  
#ifndef _sipXtapi_h_
#define _sipXtapi_h_

#include <memory.h>
#include <string.h>
#include <stddef.h>       // size_t

// SYSTEM INCLUDES
#ifdef _WIN32
#include <windows.h>
#include <Unknwn.h>
#    if !defined __strmif_h__
#        include <strmif.h>
#    endif 
#endif
#if !defined (_WIN32) || !defined (VIDEO)
    struct IBaseFilter;
#endif

// APPLICATION INCLUDES

// DEFINES
#define DEFAULT_UDP_PORT        5060    /**< Default UDP port */
#define DEFAULT_TCP_PORT        5060    /**< Default TCP port */
#define DEFAULT_TLS_PORT        5061    /**< Default TLS port */
#define DEFAULT_RTP_START_PORT  9000    /**< Starting RTP port for RTP port range.
                                             The user agent will use ports ranging 
                                             from the start port to the start port 
                                             + (default connections * 2). */
#define DEFAULT_STUN_PORT       3478    /**< Default stun server port */

#define DEFAULT_CONNECTIONS     32      /**< Default number of max sim. conns. */
#define DEFAULT_IDENTITY        "sipx"  /**< sipx@<IP>:UDP_PORT used as identify if lines
                                             are not defined.  This define only controls
                                             the userid portion of the SIP url. */
#define DEFAULT_BIND_ADDRESS    "0.0.0.0" /**< Bind to the first physical interface discovered */

#define CODEC_G711_PCMU         "258"   /**< ID for PCMU vocodec */
#define CODEC_G711_PCMA         "257"   /**< ID for PCMA vocodec*/
#define CODEC_DTMF_RFC2833      "128"   /**< ID for RFC2833 DMTF (out of band DTMF codec) */

#define GAIN_MIN                1       /**< Min acceptable gain value */
#define GAIN_MAX                100     /**< Max acceptable gain value */
#define GAIN_DEFAULT            70      /**< Nominal gain value */

#define VOLUME_MIN              1       /**< Min acceptable volume value */
#define VOLUME_MAX              100     /**< Max acceptable volume value */
#define VOLUME_DEFAULT          70      /**< Nominal volume value */

#define MAX_AUDIO_DEVICES       16      /**< Max number of input/output audio devices */
#define MAX_VIDEO_DEVICES       8       /**< Max number of video capture devices. */
#define MAX_VIDEO_DEVICE_LENGTH 256     /**< Max length of video capture device string. */

#define CONF_MAX_CONNECTIONS    32      /**< Max number of conference participants */
#define SIPX_MAX_IP_ADDRESSES   32      /**< Maximum number of IP addresses on the host */
#define SIPX_MAX_CALLS          64      /**< Maximum number of simultaneous calls. */



#define SIPX_PORT_DISABLE       -1      /**< Special value that disables the transport 
                                             type (e.g. UDP, TCP, or TLS) when passed 
                                             to sipXinitialize */
#define SIPX_PORT_AUTO          -2      /**< Special value that instructs sipXtapi to
                                             automatically select an open port for 
                                             signaling or audio when passed to 
                                             sipXinitialize */

#define SIPXTAPI_VERSION_STRING "SIPxua SDK %s.%s %s (built %s)" /**< Version string format string */
#define SIPXTAPI_VERSION        "2.9.0"      /**< sipXtapi API version -- automatically filled in 
                                                  during release process */   
#define SIPXTAPI_BUILDNUMBER    "0"          /**< Default build number -- automatically filled in 
                                                  during release process*/
#define SIPXTAPI_BUILD_WORD     2,9,0,0      /**< Default build word -- automatically filled in 
                                                  during release process */
#define SIPXTAPI_FULL_VERSION   "2.9.0.X"    /**< Default full version number -- automatically filled in 
                                                  during release process*/
#define SIPXTAPI_BUILDDATE      "2005-03-23" /**< Default build date -- automatically filled in 
                                                  during release process*/
#define SIPX_MAX_ADAPTER_NAME_LENGTH 256	 /**< Max length of an adapter name */

#if defined(_WIN32)
#  ifdef SIPXTAPI_EXPORTS
#    define SIPXTAPI_API extern "C" __declspec(dllexport)  /**< Used for Win32 imp lib creation */
#  else
#    define SIPXTAPI_API extern "C" __declspec(dllimport)  /**< Used for Win32 imp lib creation */
#  endif
#else
#  define SIPXTAPI_API extern "C"   /**< Assume extern "C" for non-win32 platforms */
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// FORWARD DECLARATIONS
class UtlString ;
class Url ;
class securityHelper;

// STRUCTS
class SipUserAgent ;
class SdpCodecFactory ;
class CallManager ;
class SipLineMgr ;
class SipRefreshMgr ;


// TYPEDEFS
/**
 * Speaker output types are used to differentiate between the logical ringer 
 * (used to alert user of in inbound call) and speaker (in call audio device).
 */
typedef enum SPEAKER_TYPE
{
    SPEAKER,    /**< Speaker / in call device */
    RINGER      /**< Ringer / alerting device */
} SPEAKER_TYPE ;

/**
 * Codec bandwidth ids are used to select a group of codecs with equal or lower
 * bandwidth requirements
 *
 * Bandwidth requirements for supported codecs:
 *
 * <pre>
 * High:     IPCMWB  ~ 80 kbps                        
 * Normal    G711U   64 kpbs                          
 *           G711A   64 kbps, 20 ms frame size
 *           PCMU    64 kbps
 *           PCMA    64 kbps
 * Low:      iLBC    13.33 kbps, 30 ms frame size and
 *                   15.2 kbps, 20 ms frame size
 *           GSM     13 kbps, 20 ms frame size
 *           G729    8 Kbps, 10ms frame size
 * Variable: ISAC    variable bitrate
 * <pre>
 */
typedef enum SIPX_AUDIO_BANDWIDTH_ID
{
    AUDIO_CODEC_BW_VARIABLE=0,   /**< ID for codecs with variable bandwidth requirements */

    AUDIO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    AUDIO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    AUDIO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */

    AUDIO_CODEC_BW_CUSTOM,		 /**< Possible return value for sipxConfigGetAudioCodecPreferences.
                                      This ID indicates the available list of codecs was
                                      overriden by a sipxConfigSetAudioCodecByName call. */
    AUDIO_CODEC_BW_DEFAULT       /**< Value used to signify the default bandwidth level 
                                      when calling sipxCallConnect, sipxCallAccept, or 
                                      sipxConferenceAdd */
} SIPX_AUDIO_BANDWIDTH_ID;


/**
 * Video Codec bandwidth ids are used to select a group of codecs with equal 
 * or lower bandwidth requirements. The codec name is a combination of the
 * actual codec name and the video resolution.
 *
 * Supported codecs are:
 * 
 *    VP71, IYUV, I420, and RGB24
 *
 * Supported resolutions are
 * 
 *    CIF (352x288), QCIF (176x144), SQCIF (128x96), and QVGA (320x240)
 *
 * A VP71 codec in QCIF resolution would be named VP71-QCIF.
 */
typedef enum SIPX_VIDEO_BANDWIDTH_ID
{
    VIDEO_CODEC_BW_VARIABLE=0,   /**< ID for codecs with variable bandwidth requirements */
    VIDEO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    VIDEO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    VIDEO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */
    VIDEO_CODEC_BW_CUSTOM,       /**< Possible return value for sipxConfigGetVideoCodecPreferences.
                                      This ID indicates the available list of codecs was
                                      overriden by a sipxConfigSetVideoCodecByName call. */
    VIDEO_CODEC_BW_DEFAULT       /**< Value used to signify the default bandwidth level 
                                      when calling sipxCallLimitCodecPreferences */
} SIPX_VIDEO_BANDWIDTH_ID;

/**
 * Video Codec quality definitions.  Quality is used as a trade off between between 
 * CPU usage and the amount of bandwidth used.
 */
typedef enum SIPX_VIDEO_QUALITY_ID
{
    VIDEO_QUALITY_LOW=1,         /**< Low quality video */
    VIDEO_QUALITY_NORMAL=2,      /**< Normal quality video */
    VIDEO_QUALITY_HIGH=3         /**< High quality video */
} SIPX_VIDEO_QUALITY_ID;

/**
 *  Enumeration of possible video sizes.
 */
typedef enum SIPX_VIDEO_FORMAT
{
    VIDEO_FORMAT_CIF=0,          /**< 352x288   */ 
    VIDEO_FORMAT_QCIF,           /**< 176x144   */
    VIDEO_FORMAT_SQCIF,          /**< 128x96    */
    VIDEO_FORMAT_QVGA            /**< 320x240   */
} SIPX_VIDEO_FORMAT;


/**
 * Format definitions for memory resident audio data
 */
typedef enum SIPX_AUDIO_DATA_FORMAT
{
    RAW_PCM_16=0                 /**< Signed 16 bit PCM data, mono, 8KHz, no header */
} SIPX_AUDIO_DATA_FORMAT;


/**
 * Signature for a log callback function that gets passed three strings,
 * first string is the priority level, second string is the source id of 
 * the subsystem that generated the message, and the third string is the 
 * message itself.
 */
typedef void (*sipxLogCallback)(const char* szPriority,
                                const char* szSource,
                                const char* szMsg);

/**
 * SIPX_RESULT is an enumeration with all the possible result/return codes.
 */ 
typedef enum SIPX_RESULT 
{
    SIPX_RESULT_SUCCESS = 0,         /**< Success */
    SIPX_RESULT_FAILURE,             /**< Generic Failure*/
    SIPX_RESULT_NOT_IMPLEMENTED,     /**< Method/API not implemented */
    SIPX_RESULT_OUT_OF_MEMORY,       /**< Unable to allocate enough memory to perform operation*/
    SIPX_RESULT_INVALID_ARGS,        /**< Invalid arguments; bad handle, argument out of range, 
                                          etc.*/
    SIPX_RESULT_BAD_ADDRESS,         /**< Invalid SIP address */
    SIPX_RESULT_OUT_OF_RESOURCES,    /**< Out of resources (hit some max limit) */
    SIPX_RESULT_INSUFFICIENT_BUFFER, /**< Buffer too short for this operation */
    SIPX_RESULT_EVAL_TIMEOUT,        /**< The evaluation version of this product has expired */
    SIPX_RESULT_BUSY,                /**< The operation failed because the system was busy */
    SIPX_RESULT_INVALID_STATE,       /**< The operation failed because the object was in
                                          the wrong state.  For example, attempting to split
                                          a call from a conference before that call is 
                                          connected. */
    SIPX_RESULT_MISSING_RUNTIME_FILES,/**< The operation failed because required runtime dependencies are missing. */
    SIPX_RESULT_TLS_DATABASE_FAILURE, /**< The operation failed because the certificate database did not initialize. */
    SIPX_RESULT_TLS_BAD_PASSWORD,     /**< The operation failed because the certificate database did not accept the password.*/
    SIPX_RESULT_TLS_TCP_IMPORT_FAILURE, /**< The operation failed because a TCP socket could not be imported by the SSL/TLS module. */
    SIPX_RESULT_NSS_FAILURE,          /**< The operation failed due to an NSS failure. */
    
} SIPX_RESULT ;

/**
 * DTMF/other tone ids used with sipxCallStartTone/sipxCallStopTone 
 */
typedef enum TONE_ID
{
    ID_DTMF_0              = '0',   /**< DMTF 0 */
    ID_DTMF_1              = '1',   /**< DMTF 1 */
    ID_DTMF_2              = '2',   /**< DMTF 2 */
    ID_DTMF_3              = '3',   /**< DMTF 3 */
    ID_DTMF_4              = '4',   /**< DMTF 4 */
    ID_DTMF_5              = '5',   /**< DMTF 5 */
    ID_DTMF_6              = '6',   /**< DMTF 6 */
    ID_DTMF_7              = '7',   /**< DMTF 7 */
    ID_DTMF_8              = '8',   /**< DMTF 8 */
    ID_DTMF_9              = '9',   /**< DMTF 9 */
    ID_DTMF_STAR           = '*',   /**< DMTF * */
    ID_DTMF_POUND          = '#',   /**< DMTF # */
    ID_DTMF_FLASH          = '!',   /**< DTMF Flash */
    ID_TONE_DIALTONE  = 512,        /**< Dialtone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_BUSY,                   /**< Call-busy tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_RINGBACK,               /**< Remote party is ringing feedback tone 
                                         (Not supported with GIPS VoiceEngine)*/
    ID_TONE_RINGTONE,               /**< Default ring/alert tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLFAILED,             /**< Fasy Busy / call failed tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_SILENCE,                /**< Silence 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_BACKSPACE,              /**< Backspace tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLWAITING,            /**< Call waiting alert tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLHELD,               /**< Call held feedback tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_LOUD_FAST_BUSY          /**< Off hook / fast busy tone 
                                         (Not supported with GIPS VoiceEngine)*/
} TONE_ID ;                 


/**
 * Various log levels available for the sipxConfigEnableLog method.  
 * Developers can choose the amount of detail available in the log.
 * Each level includes messages generated at lower levels.  For 
 * example, LOG_LEVEL_EMERG will limit the log to emergency messsages,
 * while LOG_LEVEL_ERR includes emergency messages, alert messages, 
 * critical messages, and errors.  LOG_LEVEL_ERR is probably best for
 * general runtime situations.  LOG_LEVEL_INFO or LOG_LEVEL_DEBUG is 
 * best for diagnosing problems.
 */
typedef enum SIPX_LOG_LEVEL
{
    LOG_LEVEL_DEBUG,     /**< debug-level messages */
    LOG_LEVEL_INFO,      /**< informational messages */
    LOG_LEVEL_NOTICE,    /**< normal, but significant, conditions */
    LOG_LEVEL_WARNING,   /**< warning conditions */
    LOG_LEVEL_ERR,       /**< error conditions */
    LOG_LEVEL_CRIT,      /**< critical conditions */
    LOG_LEVEL_ALERT,     /**< action must be taken immediately */
    LOG_LEVEL_EMERG,     /**< system is unusable */
    LOG_LEVEL_NONE,      /**< disable logging */
} SIPX_LOG_LEVEL ;

#define MAX_SRTP_KEY_LENGTH   31        /**< srtp key length */
#define MAX_SMIME_KEY_LENGTH  2048      /**< s/mime key length */
#define MAX_PKCS12_KEY_LENGTH 4096      /**< pkcs12 key length */
#define MAX_PASSWORD_LENGTH   32        /**< maximum password length PKI operations */

/**
 * Enumeration of the possible levels of SRTP. 
 */
enum SIPX_SRTP_LEVEL
{
    SRTP_LEVEL_NONE=0,
    SRTP_LEVEL_ENCRYPTION,
    SRTP_LEVEL_AUTHENTICATION,
    SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION
};

/**
 * Container class for security attributes.  
 */
class SIPX_SECURITY_ATTRIBUTES
{
  public:
    friend class SecurityHelper;    /**< The security help can set private values of this class. */
    /** Constructor. */
    SIPX_SECURITY_ATTRIBUTES() 
    {
        nSrtpKeyLength = 0 ;
        nSmimeKeyLength = 0 ;
        nSrtpLevel = SRTP_LEVEL_NONE ;
        memset(szSrtpKey, 0, sizeof(szSrtpKey));
        memset(szSmimeKeyDer, 0, sizeof(szSmimeKeyDer));
        memset(dbLocation, 0, sizeof(dbLocation));
        memset(szMyCertNickname, 0, sizeof(szMyCertNickname));
        memset(szCertDbPassword, 0, sizeof(szCertDbPassword));
    }    
    /** Copy constructor. */
    SIPX_SECURITY_ATTRIBUTES(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        copyData(ref);
    }    
    /** Destructor. */
    virtual ~SIPX_SECURITY_ATTRIBUTES() { }    
    /** Assignment operator. */
    SIPX_SECURITY_ATTRIBUTES& operator=(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        if (this == &ref) return *this;
        copyData(ref);
        return *this;
    }    
    /**
     * Sets the symmetric srtp key.  If this is not supplied by the user,
     * sipXtapi will generate a random key.
     */
    void setSrtpKey(const char* szKey, const int length)
    {
        int safeLen = (length < (int) sizeof(szSrtpKey)) ? length : (int) sizeof(szSrtpKey);
        memcpy(szSrtpKey, szKey, safeLen);
        nSrtpKeyLength = safeLen;
    }    
    /**
     * Sets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    void setSmimeKey(const char* szKey, const int length)
    {
        int safeLen = (length < (int) sizeof(szSmimeKeyDer)) ? length : (int) sizeof(szSmimeKeyDer);
        memcpy(szSmimeKeyDer, szKey, safeLen);
        nSmimeKeyLength = safeLen;
    }
    /**
     * Sets the S/MIME & SRTP security level
     */
    void setSecurityLevel(SIPX_SRTP_LEVEL security) { nSrtpLevel = security; }
    /**
     * Gets the symmetric srtp key.
     */
    const char* getSrtpKey() const  { return szSrtpKey; }    
    /**
     * Gets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    const char* getSmimeKey() const { return szSmimeKeyDer; }
    /**
     * Gets the symmetric srtp key length.
     */
    const int getSrtpKeyLength() const  { return nSrtpKeyLength; }
    /**
     * Gets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    const int getSmimeKeyLength() const { return nSmimeKeyLength; }
    /**
     * Sets the S/MIME & SRTP security level
     */
    const int getSecurityLevel() const {return nSrtpLevel;}
    /**
     * Gets the Certificate Database location (set internally to
     * the location specified in the call to 
     * sipxConfigSetSecurityParameters() )
     */
    const char* getCertDbLocation() const { return dbLocation; }
  private:
    SIPX_SRTP_LEVEL nSrtpLevel;
    char szSrtpKey[MAX_SRTP_KEY_LENGTH];
    int  nSrtpKeyLength;    
    char szSmimeKeyDer[MAX_SMIME_KEY_LENGTH];
    int  nSmimeKeyLength; 
    // internally set private member, use sipxConfigSetSecurityParameters
    char dbLocation[256];                         
    // internally set private member, use sipxConfigSetSecurityParameters
    char szMyCertNickname[32];   
    // internally set private member, use sipxConfigSetSecurityParameters
    char szCertDbPassword[MAX_PASSWORD_LENGTH];   
    void copyData(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        nSrtpLevel = ref.nSrtpLevel;
        nSrtpKeyLength = ref.nSrtpKeyLength;
        nSmimeKeyLength = ref.nSmimeKeyLength;
        memcpy(szSrtpKey, ref.szSrtpKey, ref.nSrtpKeyLength);
        memcpy(szSmimeKeyDer, ref.szSmimeKeyDer, ref.nSmimeKeyLength);
        strncpy(dbLocation, ref.dbLocation, sizeof(dbLocation) - 1);
        strncpy(szMyCertNickname, ref.szMyCertNickname, sizeof(szMyCertNickname) - 1);
        strncpy(szCertDbPassword, ref.szCertDbPassword, sizeof(szCertDbPassword) - 1);
    }
};

/**
 * SIPX_CONTACT_TYPE is an enumeration of possible address types for use with
 * SIP contacts and SDP connection information.  Application developers and 
 * choose to setup calls with specific contact types (e.g. use my local IP 
 * address, a stun-derived IP address, turn-derived IP address, etc).  Unless
 * you have complete knowledge and control of your network environment, you
 * should likely use CONTACT_AUTO.
 */
typedef enum
{
    CONTACT_LOCAL,      /**< Local address for a particular interface */
    CONTACT_NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
    CONTACT_RELAY,      /**< Relay address (e.g. TURN)                */
    CONTACT_CONFIG,     /**< Manually configured address              */

    CONTACT_AUTO = -1,  /**< Automatic contact selection; used for API 
                             parameters */
} SIPX_CONTACT_TYPE ;

typedef enum
{
    TRANSPORT_UDP = 1,  /**< Indicator for a UDP socket type. */
    TRANSPORT_TCP = 0,  /**< Indicator for a TCP socket type. */ 
    TRANSPORT_TLS = 3,  /**< Indicator for a TLS socket type. */
} SIPX_TRANSPORT_TYPE;

/**
 * Type for storing a "window object handle" - in Windows,
 * the application should cast their HWND to a SIPX_WINDOW_HANDLE.
 */
typedef void* SIPX_WINDOW_HANDLE;

/**
 * Enum for specifying the type of display object
 * to be used for displaying video
 */
typedef enum SIPX_VIDEO_DISPLAY_TYPE
{
    SIPX_WINDOW_HANDLE_TYPE,     /**< A handle to the window for
                                      the remote video display */
    DIRECT_SHOW_FILTER           /**< A DirectShow render filter object for
                                      handling the remote video display */
} SIPX_VIDEO_DISPLAY_TYPE;

/**
 * Structure used to pass window handle/filter interface for video calls.
 */
struct SIPX_VIDEO_DISPLAY
{
	/** Default constructor */
    SIPX_VIDEO_DISPLAY()
    {
        cbSize = sizeof(SIPX_VIDEO_DISPLAY);
        type = SIPX_WINDOW_HANDLE_TYPE;
        handle = NULL;
    }
    /** Destructor. */
    ~SIPX_VIDEO_DISPLAY()
    {
        if (type == DIRECT_SHOW_FILTER)
        {
#ifdef      _WIN32
            if (handle) ((IUnknown*)handle)->Release();
#endif              
        }
    }
    
	/** Copy constructor */
    SIPX_VIDEO_DISPLAY(const SIPX_VIDEO_DISPLAY& ref)
    {
        copy(ref);
    }
    /** Assignment operator. */
    SIPX_VIDEO_DISPLAY& operator=(const SIPX_VIDEO_DISPLAY& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;
        copy(ref);
        return *this;
    }    
        
    int cbSize;						/**< Size of structure */
    SIPX_VIDEO_DISPLAY_TYPE type;	/**< Type of video display */
    union
    {
		SIPX_WINDOW_HANDLE handle;	/**< Window handle if type SIPX_WINDOW_HANDLE_TYPE */
		IBaseFilter* filter;		/**< Direct Show filter if type is DIRECT_SHOW_FILTER */
    };
private:
    void copy(const SIPX_VIDEO_DISPLAY& ref)
    {
        cbSize = ref.cbSize;
        type = ref.type;
        handle = ref.handle;
        if (type == DIRECT_SHOW_FILTER)
        {
#ifdef      _WIN32
            // we should addRef here.
            if (handle) ((IBaseFilter*)handle)->AddRef();
#endif            
        }
    }
    
};

/** 
 * Type for storing Contact Record identifiers 
 * @see sipxConfigGetLocalContacts
 */
typedef int SIPX_CONTACT_ID; 

/**
 * The CONTACT_ADDRESS structure includes contact information (ip and port),
 * address source type, and interface.
 *
 * @see sipxConfigGetLocalContacts
 */
struct SIPX_CONTACT_ADDRESS
{
    /** Contructor. */
    SIPX_CONTACT_ADDRESS()
    {
        memset((void*)cInterface, 0, sizeof(cInterface));
        memset((void*)cIpAddress, 0, sizeof(cIpAddress));
        eContactType = CONTACT_AUTO;
        eTransportType = TRANSPORT_UDP ;
        id = 0;
        iPort = -1;
    }
    /** Copy constructor. */
    SIPX_CONTACT_ADDRESS(const SIPX_CONTACT_ADDRESS& ref)
    {
        strcpy(cInterface, ref.cInterface);
        strcpy(cIpAddress, ref.cIpAddress);
        eContactType = ref.eContactType;
        eTransportType = ref.eTransportType;
        id = ref.id;
        iPort = ref.iPort;
    }
    /** Assignment operator. */
    SIPX_CONTACT_ADDRESS& operator=(const SIPX_CONTACT_ADDRESS& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;
        strcpy(cInterface, ref.cInterface);
        strcpy(cIpAddress, ref.cIpAddress);
        eContactType = ref.eContactType;
        eTransportType = ref.eTransportType;
        id = ref.id;
        iPort = ref.iPort;
        return *this;
    }    
    SIPX_CONTACT_ID     id;              /**< Contact record Id      */
    SIPX_CONTACT_TYPE   eContactType ;   /**< Address type/source    */
    SIPX_TRANSPORT_TYPE eTransportType ; /**< Contact transport type */
    char                cInterface[32] ; /**< Source interface       */
    char                cIpAddress[32] ; /**< IP Address             */
    int                 iPort ;          /**< Port                   */
};


/**
 * The SIPX_AUDIO_CODEC structure includes codec name and bandwidth info.
 */
typedef struct 
{
#define SIPXTAPI_CODEC_NAMELEN 32       /**< Maximum length for codec name */
    char              cName[SIPXTAPI_CODEC_NAMELEN];  /**< Codec name    */
    SIPX_AUDIO_BANDWIDTH_ID iBandWidth; /**< Bandwidth requirement */
    int               iPayloadType;     /**< Payload type          */
} SIPX_AUDIO_CODEC ;


/**
 * RTCP statistics computed according to RFC 3550
 */
typedef struct 
{
    int cbSize;						/**< Size of structure */

	unsigned short fraction_lost;   /**< Fraction of lost packets. */
	unsigned long cum_lost;         /**< Cumulative lost packets. */
	unsigned long ext_max;          /**< Max size of rtcp extention header. */
	unsigned long jitter;           /**< Jitter measurement. */
	int RTT;                        /**< Round trip time. */
	int bytesSent;                  /**< Number of bytes sent. */
	int packetsSent;                /**< Number of packets sent. */
	int bytesReceived;              /**< Number of bytes received. */
	int packetsReceived;            /**< Number of packets received. */
} SIPX_RTCP_STATS ;

/**
 * The SIPX_VIDEO_CODEC structure includes codec name and bandwidth info.
 */
typedef struct 
{
#define SIPXTAPI_CODEC_NAMELEN 32        /**< Maximum length for codec name */
    char              cName[SIPXTAPI_CODEC_NAMELEN];  /**< Codec name    */
    SIPX_VIDEO_BANDWIDTH_ID iBandWidth;  /**< Bandwidth requirement */
    int               iPayloadType;      /**< Payload type          */
} SIPX_VIDEO_CODEC ;


/**
 * In the MEDIA_LOCAL_START and MEDIA_REMOTE_START events the SIPX_CODEC_INFO 
 * structure is being passed up to the event handler and contains information 
 * about the negotiated audio and video codec.
 */
typedef struct
{
    SIPX_AUDIO_CODEC audioCodec;     /**< Audio codec  */
    SIPX_VIDEO_CODEC videoCodec;     /**< Video codec  */
    bool bIsEncrypted;               /**< SRTP enabled */
} SIPX_CODEC_INFO;


/**
 * This structure gets passed into sipxCallConnect, sipxCallAccept, and
 * sipxConferenceAdd calls and sets options on a per call basis.
 */
typedef struct {
    int cbSize;                          /**< Size of structure          */
    SIPX_AUDIO_BANDWIDTH_ID bandwidthId; /**< Bandwidth range            */
    bool sendLocation;                   /**< True sends location header */
} SIPX_CALL_OPTIONS;


/** 
 * The SIPX_INST handle represents an instance of a user agent.  A user agent 
 * includes a SIP stack and media processing framework.  sipXtapi does support 
 * multiple instances of user agents in the same process space, however, 
 * certain media processing features become limited or ambiguous.  For 
 * example, only one user agent should control the local system's input and 
 * output audio devices. */
typedef void* SIPX_INST ;         
const SIPX_INST SIPX_INST_NULL = 0; /**< Represents a null instance handle */

/** 
 * The SIPX_LINE handle represents an inbound or outbound identity.  When 
 * placing outbound the application programmer must define the outbound 
 * line.  When receiving inbound calls, the application can query the 
 * line.
 */
typedef unsigned int SIPX_LINE ;
const SIPX_LINE SIPX_LINE_NULL = 0; /**< Represents a null line handle */

/** 
 * The SIPX_CALL handle represents a call or connection between the user 
 * agent and another party.  All call operations require the call handle
 * as a parameter.
 */
typedef unsigned int SIPX_CALL ;
const SIPX_CALL SIPX_CALL_NULL = 0; /**< Represents a null call handle */

/** 
 * The SIPX_CONF handle represents a collection of CALLs that have bridge
 * (mixed) audio.  Application developers can manipulate each leg of the 
 * conference through various conference functions.
 */
typedef unsigned int SIPX_CONF ;
const SIPX_CONF SIPX_CONF_NULL = 0; /**< Represents a null conference handle */

/**
 * The SIPX_INFO handle represents a handle to an INFO message sent by
 * a sipXtapi instance.  INFO messages are useful for communicating 
 * information between user agents within a logical call.  The SIPX_INFO 
 * handle is returned when sending an INFO message via 
 * sipxCallSendInfo(...).  The handle is references as part of the 
 * EVENT_CATEGORY_INFO_STATUS event callback/observer.  sipXtapi will 
 * automatically deallocate this handle immediately after the status
 * call back.
 */
typedef unsigned int SIPX_INFO;

/**
 * The SIPX_PUB handle represent a publisher context.  Publisher are used
 * to publish application-data to interested parties (Subscribers).  This
 * maps directly to the SIP SUBSCRIBE, and NOTIFY methods.  The handle is
 * used to mange the life cycle of the publisher.
 *
 * SIPX_PUB handles are created by using sipxCreatePublisher.
 * SIPX_PUB handles should be torn down using sipxDestroyPublisher.
 */
typedef unsigned int SIPX_PUB;

const SIPX_PUB SIPX_PUB_NULL = 0; /**< Represents a null publisher handle */

/**
 * A SIPX_SUB handle represent a subscription to a remote publisher.  This
 * maps directly to the SIP SUBSCRIBE, and NOTIFY methods.  The handle is 
 * used to mange the life cycle of the subscription.
 *
 * SIPX_SUB handles are created by using the sipxCallSubscribe function.
 * SIPX_SUB handles should be destroyed using the sipxCallUnsubscribe function.
 */
typedef unsigned int SIPX_SUB ;


/** 
 * Typedef for audio source (microphone) hook procedure.  This typedef 
 * coupled with the sipxConfigSetMicAudioHook API allows developers to 
 * view, modify or substitute microphone data.
 * 
 * @param nSamples number of 16 bit unsigned PCM samples
 * @param pSamples pointer to array of samples.
 */
typedef void (*fnMicAudioHook)(const int nSamples, short* pSamples) ;

/** 
 * Typedef for audio target(speaker) hook procedure.  This typedef 
 * coupled with the sipxConfigSetSpkrAudioHook API allows developers to 
 * intercept and modify audio headed for the speaker.
 *
 * @param nSamples number of 16 bit unsigned samples
 * @param pSamples pointer to array of samples
 */
typedef void (*fnSpkrAudioHook)(const int nSamples, short* pSamples) ;


/**
 * SIPX_KEEPALIVE_TYPEs define different methods of keeping NAT/firewall
 * port open.   These approaches are used for the signaling path of a call
 * and are generally only needed under specific network configurations.
 * 
 * Examples: - When not using a proxy
 *           - When the registration period is longer then NAT bindings 
 *             timeout
 */
typedef enum
{
    SIPX_KEEPALIVE_CRLF,        /**<Send a Carriage Return/Line Feed to other side */
    SIPX_KEEPALIVE_STUN,        /**<Send a Stun request to the other side */
    SIPX_KEEPALIVE_SIP_PING     /**<Send a SIP PING method request to the other side 
                                    (not implemented) */
} SIPX_KEEPALIVE_TYPE ;



/** 
 * SIPX_AEC_MODE defines different AEC modes.  Options included DISABLED,
 * SUPPRESS, CANCEL, and CANCEL_AUTO.
 *
 * NOTE: This functionally is only supported when sipXtapi is bundled with
 * VoiceEngine from Global IP Sound.
 */
typedef enum SIPX_AEC_MODE
{
    SIPX_AEC_DISABLED,   /**<Disabled AEC; do not attempt to cancel or 
                             suppress echo */
    SIPX_AEC_SUPPRESS,   /**<Echo suppression; attempt to suppress echo by
                             effectively forcing a half-duplex audio channel.
                             If you are speaking, the speaker will be silenced
                             to avoid echo.  Echo cancellation is consider a 
                             better approach/experience, however, requires more
                             CPU consumption.  */
    SIPX_AEC_CANCEL,     /**<Full echo cancellation; attempt to cancel echo 
                             between the the speaker and microphone.  Depending
                             on the quality of your speaker/microphone, this 
                             may result in some suppression.  For example, if 
                             either the speaker or microphone distorts the 
                             signal (making it non-linear), it is becomes 
                             increasingly difficult to cancel.  This is 
                             consider a full-duplex solution. */
    SIPX_AEC_CANCEL_AUTO,/**<Full echo cancellation; attempt to cancel echo 
                             between the the speaker and microphone; however,
                             automatically disable echo cancellation if it
                             appears not needed.  */

} SIPX_AEC_MODE ;


/**
 * SIPX_NOISE_REDUCTION_MODE defines the various noise reduction options.  
 * Options include, DISABLED, LOW, MEDIUM, and HIGH.  When selecting a 
 * noise reduction level, you are trading off reducing back ground noise
 * with the possibility of suppressing speech.  We recommend selecting the
 * LOW level.
 *
 * NOTE: This functionally is only supported when sipXtapi is bundled with
 * VoiceEngine from Global IP Sound.
 */
typedef enum SIPX_NOISE_REDUCTION_MODE
{
    SIPX_NOISE_REDUCTION_DISABLED,  /**< Disable NR; Do not attempt to reduce 
                                         background noise */
    SIPX_NOISE_REDUCTION_LOW,       /**< Enable NR with least amount of 
                                         aggressiveness. */
    SIPX_NOISE_REDUCTION_MEDIUM,    /**< Enable NR with modest amount of 
                                         aggressiveness. */
    SIPX_NOISE_REDUCTION_HIGH,      /**< Enable NR with highest amount of 
                                         aggressiveness. */
} SIPX_NOISE_REDUCTION_MODE ;

/* ============================ FUNCTIONS ================================= */

/** @name Initialization */
//@{


/** 
 * Initialize the sipX tapi-like API layer.  This method initialized the
 * basic SIP stack and media process resources and must be called before 
 * any other sipxXXX methods.  Additionally, this method fills in a 
 * SIPX_INST parameter which must be passed to a number of sipX methods.
 *
 * @param phInst A pointer to a hInst that must be various other
 *        sipx routines. 
 * @param udpPort The default UDP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable UDP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open UDP port.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.    Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable TCP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open TCP port.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open port.
 * @param maxConnections The maximum number of simultaneous connections
 *        that the sipX layer will support.
 * @param szIdentity The default outbound identity used by the SIP stack
 *        if no lines are defined. Generally, the szIdentity is only used
 *        for inbound calls since all of sipXtapi APIs required a line ID
 *        for outbound calls.  The identity will be used to form the 
 *        "From" field (caller-id) and the username/URL parameters are 
 *        may be used as part of the "Contact" header.  In other words,
 *        this field does not impact any routing aspects of the call
 *        session.
 * @param szBindToAddr Defines which IP/address the user agent / rtp 
 *        stack will listen on.  The default "0.0.0.0" listens on all
 *        interfaces.  The address must be in dotted decimal form -- 
 *        hostnames will not work.
 * @param bUseSequentialPorts If unable to bind to the udpPort, tcpPort, 
 *        or tlsPort, try sequential ports until a successful port is 
 *        found.  If enabled, sipXtapi will try 10 sequential port 
 *        numbers after the initial port.
 * @param szTLSCertificateNickname Nickname of the certificate to use as an SSL server.
 * @param szTLSCertificatePassword Password for the SSL server certificate.
 * @param szDbLocation Path to the certificate database.
 */
SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST* phInst,
                                        const int udpPort = DEFAULT_UDP_PORT,
                                        const int tcpPort = DEFAULT_TCP_PORT,
                                        const int tlsPort = DEFAULT_TLS_PORT,
                                        const int rtpPortStart = DEFAULT_RTP_START_PORT,
                                        const int maxConnections = DEFAULT_CONNECTIONS,
                                        const char* szIdentity = DEFAULT_IDENTITY,
                                        const char* szBindToAddr = DEFAULT_BIND_ADDRESS,
                                        bool      bUseSequentialPorts = false,
                                        const char* szTLSCertificateNickname = NULL,
                                        const char* szTLSCertificatePassword = NULL,
                                        const char* szDbLocation = NULL) ;


/** 
 * Re-initialize the sipX tapi-like API layer.  This method will remove all lines,
 * conferences, calls, publishers, and subscribers, while events are still enabled.
 * Before calling this function, the application should un-register all registered
 * lines.  
 *
 * Your listeners are maintained.
 *
 * @param phInst A pointer to a SIPX_INST variable.  Your old SIPX_INST
 *        pointer will be invalid after this call.  Calling any routines
 *        with the old SIPX_INST variable is undefined and may result in
 *        an exception.
 * @param udpPort The default UDP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable UDP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open UDP port.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.    Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable TCP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open TCP port.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open port.
 * @param maxConnections The maximum number of simultaneous connections
 *        that the sipX layer will support.
 * @param szIdentity The default outbound identity used by the SIP stack
 *        if no lines are defined. Generally, the szIdentity is only used
 *        for inbound calls since all of sipXtapi APIs required a line ID
 *        for outbound calls.  The identity will be used to form the 
 *        "From" field (caller-id) and the username/URL parameters are 
 *        may be used as part of the "Contact" header.  In other words,
 *        this field does not impact any routing aspects of the call
 *        session.
 * @param szBindToAddr Defines which IP/address the user agent / rtp 
 *        stack will listen on.  The default "0.0.0.0" listens on all
 *        interfaces.  The address must be in dotted decimal form -- 
 *        hostnames will not work.
 * @param bUseSequentialPorts If unable to bind to the udpPort, tcpPort, 
 *        or tlsPort, try sequential ports until a successful port is 
 *        found.  If enabled, sipXtapi will try 10 sequential port 
 *        numbers after the initial port.
 * @param szTLSCertificateNickname Nickname of the certificate to use as an SSL server.
 * @param szTLSCertificatePassword Password for the SSL server certificate.
 * @param szDbLocation Path to the certificate database.
 */
SIPXTAPI_API SIPX_RESULT sipxReInitialize(SIPX_INST* phInst,
                                        const int udpPort = DEFAULT_UDP_PORT,
                                        const int tcpPort = DEFAULT_TCP_PORT,
                                        const int tlsPort = DEFAULT_TLS_PORT,
                                        const int rtpPortStart = DEFAULT_RTP_START_PORT,
                                        const int maxConnections = DEFAULT_CONNECTIONS,
                                        const char* szIdentity = DEFAULT_IDENTITY,
                                        const char* szBindToAddr = DEFAULT_BIND_ADDRESS,
                                        bool      bUseSequentialPorts = false,
                                        const char* szTLSCertificateNickname = NULL,
                                        const char* szTLSCertificatePassword = NULL,
                                        const char* szDbLocation = NULL) ;

/** 
 * Un-initialize the sipX tapi-like API layer.  This method tears down the
 * basic SIP stack and media process resources and should be called before 
 * exiting the process.  Users are responsible for ending all calls and 
 * unregistering line appearances before calling sipxUnInitialize.  Failing
 * to end calls/conferences or remove lines will result in a 
 * SIPX_RESULT_BUSY return code.
 *
 * @param hInst An instance handle obtained from sipxInitialize. 
 * @param bForceShutdown forces sipXtapi to shutdown regardless of live 
 *        calls/unregistered lines.  Enabling this in NOT RECOMMENDED,
 *        please tear down all calls and lines prior to calling 
 *        sipxUnitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst, bool bForceShutdown = false);

//@}
/** @name Call Methods */
//@{

/**
 * Accepts an inbound call and proceed immediately to alerting.  This method
 * is invoked in response to a NEWCALL event.  Whenever a new call is received,
 * the application developer should ACCEPT (proceed to ringing), REJECT (send
 * back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param pDisplay Pointer to an object describing the display object for 
 *        rendering remote video.
 * @param pSecurity Pointer to an object describing the security attributes for 
 *        the call.
 * @param options Pointer to a SIPX_CALL_OPTIONS structure. 
 *
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL hCall, 
                                        SIPX_VIDEO_DISPLAY* const pDisplay = NULL,
                                        SIPX_SECURITY_ATTRIBUTES* const pSecurity = NULL,
                                        SIPX_CALL_OPTIONS* options = NULL);


/**
 * Reject an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipxCallAccept).
 * Whenever a new call is received, the application developer should ACCEPT 
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param errorCode RFC specified error code.
 * @param szErrorText null terminated text string to explain the error code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall,
                                        const int errorCode = 400,
                                        const char* szErrorText = "Bad Request") ;


/**
 * Redirect an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipxCallAccept).
 * Whenever a new call is received, the application developer should ACCEPT 
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szForwardURL SIP url to forward/redirect the call to.
 */
SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall,
                                          const char* szForwardURL) ;

/**
 * Answer an alerting call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bTakeFocus Should SIPxua place the newly answered call in focus
 *        (engage local microphone and speaker).  In some cases, application
 *        developer may want to answer the call in the background and play
 *        audio while the user finishes up with their active (in focus) call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall, 
                                        bool  bTakeFocus = true) ;


/**
 * Create a new call for the purpose of creating an outbound connection/call.
 * As a side effect, a DIALTONE event is fired to simulate the PSTN world.
 * Generally an application would simulate dialtone in reaction to that
 * event.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param hLine Line Identity for the outbound call.  The line identity 
 *        helps defines the "From" caller-id.
 * @param phCall Pointer to a call handle.  Upon success, this value is
 *        replaced with a valid call handle.  Success is determined by
 *        the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst, 
                                        const SIPX_LINE hLine,
                                        SIPX_CALL*  phCall) ;

/**
 * Connects an idle call to the designated target address
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url of the target party
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param pDisplay Pointer to an object describing the display object for 
 *        rendering remote video.
 * @param pSecurity Pointer to an object describing the security attributes for 
 *        the call.
 * @param bTakeFocus Should SIPxua place the this call in focus (engage 
 *        local microphone and speaker).  In some cases, application developer
 *        may want to place the call in the background and play audio while 
 *        the user finishes up with their active (in focus) call. 
 * @param options Pointer to a SIPX_CALL_OPTIONS structure.
 * @param szCallId A call-id for the session, if NULL, one is generated.
 *
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxCallConnect(const SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_CONTACT_ID contactId = 0,
                                         SIPX_VIDEO_DISPLAY* const pDisplay = NULL,
                                         SIPX_SECURITY_ATTRIBUTES* const pSecurity = NULL,
                                         bool bTakeFocus = true,
                                         SIPX_CALL_OPTIONS* options = NULL,
                                         const char* szCallId = NULL);

/**
 * Place the specified call on hold.  When placing calls on hold or
 * having a remote party place you on hold the event sequeneces will
 * differ.  In this documentation, we refer to "local" hold and/or 
 * "focus" and "remote" and/or "full" hold.  A call is on local hold 
 * when that call is taken out of focus and is no longer connected to
 * the local microphone and speaker.  Remote hold is used to indicate 
 * that RTP is no longer flowing between parties.  The "bRemoteStopAudio" 
 * flags to this method controls whether the party is placed on local 
 * hold or full hold.  See the table below for expected events:
 *
 *<pre>
 *                   RTP Flowing  RTP Stopped
 *                   ---------    -----------
 *       In Focus    CONNECTED    REMOTE_HELD
 *   Out of Focus     BRIDGED        HELD
 *</pre>
 *
 * CONNECTED indicates that both RTP is flowing and the call is attached
 *    to the local.  This is the normal state for a connected call.
 *
 * BRIDGED indicates that RTP is flowing, but the call is out of focus.
 *    This event is generally caused by holding a conference (conference 
 *    will bridge by default) or if you accept/place a new call without
 *    explicitly holding the active call.
 *
 * REMOTE_HELD indicates that RTP has stopped flowing.  This is generally
 *    caused when the remote side places you on hold.  The call is still
 *    locally in focus and audio will automatically resume once your are 
 *    take off remote hold.
 *
 * HELD indicates that both RTP has stopped flowing and the call is out
 *    of focus.
 *
 * Developers can also expect media events (e.g. MEDIA_LOCAL_STOP) 
 * whenever RTP is stopped (REMOTE_HELD and HELD).  Since media is still 
 * flowing for CONNECTED and BRIDGED, no media stop events are sent.
 *
 * NOTE: If this call is part of a conference, sipxCallHold will only
 *       change the remote held state (RTP).  You must use 
 *       sipxConferenceHold to change call focus.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bStopRemoteAudio Flag which controls whether sipXtapi takes
 *        the call out of focus (stops engaging local audio microphone
 *        and speaker) or stops sending/receiving audio.  Specify true
 *        to stop audio (default) or false to take the call out of 
 *        focus.  To play audio or generate tones to a remote connection
 *        while on hold, please specify false.  This parameter is 
 *        ignored (and assumed true) if the call is part of a conference.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall, 
                                      bool bStopRemoteAudio = true) ;


/**
 * Take the specified call off hold.  This API will take the call off
 * both local hold and remote/full hold.
 * 
 * @see sipxCallHold for a description of expected events 
 *      associated with hold events.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall) ;


/**
 * Drop/Destroy the specified call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL& hCall) ;


/**
 * Get the SIP call ID of the call represented by the specified call handle.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetID(const SIPX_CALL hCall,
                                       char* szId, 
                                       const size_t iMaxLength) ;

/**
 * Get the SIP identity of the local connection.  The identity represents
 * either 1) who was called in the case of a inbound call, or 2) the
 * line identity used in an outbound call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalID(const SIPX_CALL hCall, 
                                            char* szId, 
                                            const size_t iMaxLength) ;


/**
 * Get the SIP identity of the remote connection.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szId Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteID(const SIPX_CALL hCall, 
                                             char* szId, 
                                             const size_t iMaxLength) ;


/**
 * Gets the media interface connectionid.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param connectionId Reference to the returned connection identifier.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetConnectionId(const SIPX_CALL hCall,
                                                 int& connectionId);
                                                 

/**
 * Get the SIP request uri.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szUri Buffer to store the request uri.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the request uri buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRequestURI(const SIPX_CALL hCall, 
                                               char* szUri, 
                                               const size_t iMaxLength) ;


                                                           
/**
 * Get the SIP remote contact.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContact Buffer to store the remote contact.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the remote contact buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteContact(const SIPX_CALL hCall, 
                                                  char* szContact, 
                                                  const size_t iMaxLength) ;

                                                           
/**
 * Get the remote user agent of the call represented by the specified call handle.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAgent Buffer to store the user agent name.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the buffer
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteUserAgent(const SIPX_CALL hCall,
                                                    char* szAgent, 
                                                    const size_t iMaxLength) ;


/**
 * Play a tone (DTMF, dialtone, ring back, etc) to the local and/or
 * remote party.  See the DTMF_ constants for built-in tones.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param toneId ID of the tone to play
 * @param bLocal Should the tone be played locally? 
 * @param bRemote Should the tone be played to the remote party?
 */
SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall, 
                                           const TONE_ID toneId,
                                           const bool bLocal,
                                           const bool bRemote) ;

/**
 * Stop playing a tone (DTMF, dialtone, ring back, etc). to local
 * and remote parties.  Under a GIPS VoiceEngine build, this method 
 * is a NOP -- VoiceEngine only plays tones for a specific time 
 * interval.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall) ;


/**
 * Play the designated file.  The file may be a raw 16 bit signed PCM at
 * 8000 samples/sec, mono, little endian or a .WAV file.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played in the
 *        context of a call.
 * @param szFile Filename for the audio file to be played.
 * @param bRepeat True if the file is supposed to be played repeatedly
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 * @param bMixWithMicrophone True to mix the audio with the microphone
 *        data or false to replace it.  This option is only supported 
 *        when sipXtapi is bundled with GIPS VoiceEngine.
 * @param fVolumeScaling Volume down scaling for the audio file.  Valid 
 *        values are between 0 and 1.0, where 1.0 is the no down-scaling.
 *        This option is only supported when sipXtapi is bundled with GIPS
 *        VoiceEngine.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStart(const SIPX_CALL hCall, 
                                                    const char* szFile,
                                                    const bool bRepeat,
                                                    const bool bLocal,
                                                    const bool bRemote,
                                                    const bool bMixWithMicrophone = false,
                                                    const float fVolumeScaling = 1.0) ;

/**
 * Stop playing a file started with sipxCallPlayFileStart
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStop(const SIPX_CALL hCall) ; 


/**
 * Record a call session (including other parties if this is a multi-party 
 * call / conference) to a file.  The resulting file will be a .WAV file
 * with L16 encoding (16000 samples/per second).
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS. 
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 * @param szFile Filename for the resulting audio file.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStart(const SIPX_CALL hCall,
                                                      const char* szFile) ;

/**
 * Stop recording a call to file.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStop(const SIPX_CALL hCall) ;


/**
 * Play the specified audio data.  Currently the only data format that
 * is supported is raw 16 bit signed PCM at 8000 samples/sec, mono,
 * little endian.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played in the context
 *        of a call.
 * @param szBuffer Pointer to the audio data to be played.
 * @param bufSize Length, in bytes, of the audio data.
 * @param bufType The audio encoding format for the data as specified
 *                by the SIPX_AUDIO_DATA_FORMAT enumerations.  Currently
 *                only RAW_PCM_16 is supported.
 * @param bRepeat True if the audio is supposed to be played repeatedly
 * @param bLocal True if the audio is to be rendered locally.
 * @param bRemote True if the audio is to be rendered by the remote endpoint.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStart(const SIPX_CALL hCall,
                                                 const char* szBuffer,
                                                 const int  bufSize,
                                                 const int  bufType,
                                                 const bool bRepeat,
                                                 const bool bLocal,
                                                 const bool bRemote) ;


/**
 * Stop playing the audio started with sipxCallPlayBufferStart
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStop(const SIPX_CALL hCall) ; 


/**
 * Subscribe for NOTIFY events which may be published by the other end-point 
 * of the call.  This API differs from sipxConfigSubscribe in that it allows
 * you to use the contact address from the remote party as the subscription
 * target (see the bRemoteContactIsGruu parameter).
 *
 * sipXtapi will automatically refresh subscriptions until sipxCallUnsubscribe
 * is called.  Please make sure you call sipxCallUnsubscribe before tearing 
 * down your call.
 *
 * @param hCall The call handle of the call associated with the subscription.
 * @param szEventType A string representing the type of event that can be 
 *        published.  This string is used to populate the "Event" header in
 *        the SIP SUBSCRIBE request.  For example, if checking voicemail 
 *        status, your would use "message-summary".
 * @param szAcceptType A string representing the types of NOTIFY events that 
 *        this client will accept.  This string is used to populate the 
 *        "Accept" header in the SIP SUBSCRIBE request.  For example, if
 *        checking voicemail status, you would use 
 *        "application/simple-message-summary"
 * @param phSub Pointer to a subscription handle whose value is set by this 
 *        funtion.  This handle allows you to cancel the subscription and
 *        differeniate between NOTIFY events.
 * @param bRemoteContactIsGruu indicates whether the Contact for the remote 
 *        side of the call can be assumed to be a Globally Routable Unique URI
 *        (GRUU).  Normally one cannot assume that a contact is a GRUU and the
 *        To or From address for the remote side is assumed to be an Address Of
 *        Record (AOR) that is globally routable.
 */                                          
SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub,
                                           bool bRemoteContactIsGruu = false);

/**
 * Unsubscribe from previously subscribed NOTIFY events.  This method will
 * send another subscription request with an expires time of 0 (zero) to end
 * your subscription.  
 *
 * @param hSub The subscription handle obtained from the call to 
 *             sipxCallSubscribe.
 */                                          
SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub) ;


/**
 * Sends an INFO event to the specified call.  
 *
 * This method will fail with an SIPX_RESULT_INVALID_STATE return code 
 * if an existing INFO message transaction is still outstanding (sipXtapi 
 * has not received a final response to the initial request).
 * 
 * @param phInfo Pointer to an INFO message handle, whose value is set by 
 *        this method.
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContentType String representation of the INFO content type
 * @param szContent Pointer to the INFO messasge's content
 * @param nContentLength Size of the INFO content
 */
SIPXTAPI_API SIPX_RESULT sipxCallSendInfo(SIPX_INFO* phInfo,
                                          const SIPX_CALL hCall,
                                          const char* szContentType,
                                          const char* szContent,
                                          const size_t nContentLength);

/**
 * Blind transfer the specified call to another party.  Monitor the
 * TRANSFER state events for details on the transfer attempt.  If the
 * call is not already on hold, the party will be placed on hold.
 *
 * Assuming that all parties are using sipXtapi, all of the calls
 * are active (not held), and the transfer works, you should expect the 
 * following event sequence:
 *
 * <h3>Transferee (party being transfered):</h3>
 *
 * The transferee's original call will be placed on remote hold from the 
 * transfer controller and will then put itself on full hold before starting
 * the transfer.  The transfer is implemented as a new call -- this allows
 * the developer to reclaim the original call if the transfer fails.  The 
 * NEWCALL event will include a cause for of CALLSTATE_CAUSE_TRANSFER and
 * the hAssociatedCall member of the SIPX_CALLSTATE_INFO structure will
 * include the handle of the original call.
 *
 * <pre>
 * Original Call: MEDIA_LOCAL_STOP
 * Original Call: MEDIA_REMOTE_STOP
 * Original Call: CALLSTATE_REMOTE_HELD
 * Original Call: CALLSTATE_HELD
 *
 * New Call: CALLSTATE_NEWCALL
 * New Call: CALLSTATE_REMOTE_OFFERING
 * New Call: CALLSTATE_REMOTE_ALERTING
 * New Call: CALLSTATE_CONNECTED
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 * </pre>
 *
 * After the transfer completes, the application developer must destroy 
 * the original call using sipxCallDestroy.  If the new call fails for
 * any reason, the application layer should resume the original call by
 * taking it off hold.
 *
 * <h3>Transfer Controller (this user agent):</h3>
 *
 * The transfer controller will automatically take the call out of 
 * focus and place it on hold.  Afterwards, the transfer controller
 * will receive CALLSTATE_TRANSFER_EVENTs indicating the status of
 * the transfer.
 *
 * <pre>
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_INITIATED
 * Source Call: CALLSTATE_BRIDGED
 * Source Call: CALLSTATE_HELD
 * Source Call: MEDIA_CAUSE_HOLD
 * Source Call: MEDIA_CAUSE_HOLD
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_ACCEPTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_RINGING
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_SUCCESS
 * Source Call: CALLSTATE_DISCONNECTED
 * </pre>
 *
 * Upon success, the call will automatically be disconnected, however,
 * the application layer needs to call sipXcallDestroy to free the handle
 * and call processing resources.
 *
 * <h3>Transfer Target (identified by szAddress):</h3>
 *
 * The transferee will go through the normal event progression for an incoming
 * call:
 *
 * <pre>
 * New Call: CALLSTATE_NEWCALL
 * New Call: CALLSTATE_OFFERING
 * New Call: CALLSTATE_ALERTING
 * New Call: CALLSTATE_CONNECTED
 * </pre>
 *
 * If the transfer target rejects the call or fails to answer, the transfer 
 * will fail.
 *
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url identifing the transfer target (who the call
 *        identified by hCall will be transfered to).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall, 
                                               const char* szAddress) ;

/**
 * Transfer the source call to the target call.  This method can be used
 * to implement consultative transfer (transfer initiator can speak with 
 * the transfer target prior to transferring).  If you wish to consult 
 * privately, create a new call to the transfer target.  If you wish 
 * consult and allow the source (transferee) to participant in the 
 * converstation, create a conference and then transfer one leg to 
 * another.
 *
 * If not already on hold, the transferee (hSourceCall) is placed on 
 * hold as part of the transfer operation.
 *
 * The event sequence may differ slightly depending on whether the calls
 * are part of a conference (attended transfer) or individual calls (semi-
 * attended transfer).
 *
 * Assuming the calls are part of a conference and not on hold, the event
 * sequences are as follows:
 *
 * <h3>Transfer Controller (this user agent):</h3>
 *
 * <pre>
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_INITIATED
 * Source Call: CALLSTATE_REMOTE_HELD
 * Source Call: MEDIA_LOCAL_STOP
 * Source Call: MEDIA_REMOTE_STOP
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_ACCEPTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_SUCCESS
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_DISCONNECTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_DESTROYED
 * </pre>
 *
 * The source call will automatically be disconnected if the transfer is 
 * successful.  Also, if the source call is part of a conference, the call 
 * will automatically be destroyed.  If not part of a conference, the 
 * application must destroy the call using sipxCallDestroy.
 *
 * <pre>
 * Target Call: CALLSTATE_REMOTE_HELD
 * Target Call: MEDIA_LOCAL_STOP
 * Target Call: MEDIA_REMOTE_STOP
 * Target Call: CALLSTATE_DISCONNECTED
 * Target Call: CALLSTATE_DESTROYED
 * </pre>
 * 
 * The target call is remote held as part of the transfer operation.  If the
 * target call is part of a conference, it will automatically be destroyed.
 * Otherwise, the application layer is responsible for calling 
 * sipxCallDestroy.
 *
 * <h3>Transferee (user agent on other side of hSourceCall):</h3>
 *
 * The transferee will create a new call to the transfer target and 
 * automatically disconnect the original call upon success.  The new call
 * will be created with a cause of CALLSTATE_CAUSE_TRANSFER in the
 * SIPX_CALLSTATE_INFO event data.  The application layer can look at
 * the hAssociatedCall member to connect the new call to the original
 * call. 
 *
 * <pre>
 * Original Call: MEDIA_LOCAL_STOP
 * Original Call: MEDIA_REMOTE_STOP
 * Original Call: CALLSTATE_REMOTE_HELD
 * Original Call: CALLSTATE_HELD
 * Original Call: CALLSTATE_DISCONNECTED
 *
 * New Call: CALLSTATE_NEWCALL::CALLSTATE_CAUSE_TRANSFER
 * New Call: CALLSTATE_REMOTE_OFFERING
 * New Call: CALLSTATE_CONNECTED
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 * </pre>
 *
 * The application is responsible for calling sipxCallDestroy on the original
 * call after the CALLSTATE_DISCONNECT event.
 *
 * <h3>Transfer Target (user agent on other side of hTargetCall):</h3>
 *
 * The transfer target will automatically receive and answer the inbound call 
 * from the transferee.  After this completes, the original call is 
 * disconnected.
 *
 * <pre>
 * CALLSTATE_NEWCALL::CALLSTATE_CAUSE_TRANSFERRED
 * CALLSTATE_CONNECTED
 * MEDIA_LOCAL_START
 * MEDIA_REMOTE_START
 * </pre>
 *
 * Please note that no offering event was fired.  The target does not have
 * the option to accept or reject the call.  If this call was part of a 
 * conference, the new call is automatically added to the same conference.
 *
 * @param hSourceCall Handle to the source call (transferee).
 * @param hTargetCall Handle to the target call (transfer target).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallTransfer(const SIPX_CALL hSourceCall,
                                          const SIPX_CALL hTargetCall) ;

#ifdef VIDEO

/**
 * Updates the Video window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUpdateVideoWindow(const SIPX_CALL hCall, const SIPX_WINDOW_HANDLE hWnd);


/**
 * Resizes the video window.  Should be called when the window receives a SIZE message.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param hWnd Window handle of the video window.
 */
SIPXTAPI_API SIPX_RESULT sipxCallResizeWindow(const SIPX_CALL hCall, const SIPX_WINDOW_HANDLE hWnd);

#endif


/**
 * Gets energy levels for a call.  The call must be in the connected state
 * for this request to succeed.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param iInputEnergyLevel Input/Microphone energy level ranging from 0 to 9.
 * @param iOutputEnergyLevel Output/Speaker energy level ranging from 0 to 9. 
 *        The output energy level is pre-mixed (before mixing any files/tones or
 *        other parties).  sipxConferenceGetEnergyLevels provides an API to
 *        obtain post-mixed energy levels.
 * @param nMaxContributors Max number of contributors/energy levels.  
 *        Contributors are derived by looking at the contributing RTP source
 *        IDs from the RTP stream.
 * @param CCSRCs Array of contributing source ids.  This array will be filled 
 *        in up to a max of nMaxContributors.  See nActualContributors for
 *        the actual number of elements returned.
 * @param iEnergyLevels Energy level for each contributing source id ranging 
 *        from 0 to 9.  This array in up to a max of nMaxContributors.  See 
 *        nActualContributors for the actual number of elements returned.
 * @param nActualContributors The actual number of contributing source ids and
 *        energy levels returned.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetEnergyLevels(const SIPX_CALL hCall,
                                                 int&            iInputEnergyLevel,
                                                 int&            iOutputEnergyLevel,
                                                 const size_t    nMaxContributors,
                                                 unsigned int    CCSRCs[],
                                                 int             iEnergyLevels[],
                                                 size_t&         nActualContributors) ;

/** 
 * Gets the sending and receiving Audio RTP SSRC IDs.  The SSRC ID is used to 
 * identify the RTP/audio stream.  The call must be in the connected state
 * for this request to succeed.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param iSendSSRC The RTP SSRC used when sending audio
 * @param iReceiveSSRC The RTP SSRC used by the remote party to sending audio
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtpSourceIds(const SIPX_CALL hCall,
                                                      unsigned int& iSendSSRC,
                                                      unsigned int& iReceiveSSRC)  ;

/**
 * Obtain RTCP stats for the specified call.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 *
 * @param pStats Structure to place call stats, the structure's cbSize 
 *        member must be filled out prior to calling this API.  For example:
 *        myStats.cbSize = sizeof(SIPX_RTCP_STATS);
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtcpStats(const SIPX_CALL hCall,
                                                   SIPX_RTCP_STATS* pStats) ;

/**
 * Limits the codec preferences on a per-call basis.  This API will force a 
 * codec renegotiation with the specified call regardless if the codecs 
 * changed.  A renegotiation includes sending a new INVITE with an updated SDP
 * list.  Local audio will be stopped and restarted during this process, 
 * however, hold events are not sent to the application.
 *
 * NOTE: If a call is on remote hold, it will be taken off remote hold.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param audioBandwidth A bandwidth id to limit audio codecs. Pass in
 *        AUDIO_CODEC_BW_DEFAULT to leave audio codecs unchanged.
 * @param videoBandwidth A bandwidth id to limit video bitrate and framerate.
 *        (see sipxConfigSetVideoBandwidth for an explanation on how 
 *        bandwidth ids affect bitrate and framerate). Pass in AUDIO_CODEC_BW_DEFAULT
 *        to leave these parameters unchanged.
 * @param szVideoCodecName Codec name that limits the supported video codecs
 *        to this one video codec.
 *        
 * @see sipxConfigSetVideoBandwidth
 */
SIPXTAPI_API SIPX_RESULT sipxCallLimitCodecPreferences(const SIPX_CALL hCall,
                                                       const SIPX_AUDIO_BANDWIDTH_ID audioBandwidth,
                                                       const SIPX_VIDEO_BANDWIDTH_ID videoBandwidth,
                                                       const char* szVideoCodecName);

//@}

/** @name Publishing Methods */
//@{


/**
 * Creates a publishing context, which perfoms the processing necessary
 * to accept SUBSCRIBE requests, and to publish NOTIFY messages to subscribers. 
 * The resource may be specific to a single call, conference or global
 * to this user agent.  The naming of the resource ID determines the scope.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phPub Pointer to a publisher handle - this method modifies the value
 *              to refer to the newly created publishing context.
 * @param szResourceId The resourceId to the state information being 
 *        published.  This must match the request URI of the incoming 
 *        SUBSCRIBE request (only the user ID, host and port are significant
 *        in matching the request URI).  Examples: fred\@10.0.0.1:5555, 
 *               sip:conference1\@192.160.0.1, sip:kate\@example.com  
 * @param szEventType A string representing the type of event that can be 
 *               published.
 * @param szContentType String representation of the content type being 
 *        published.
 * @param pContent Pointer to the NOTIFY message's body content.
 * @param nContentLength Size of the content to be published.
 *
 * @return If the resource already has a a publisher created for the given
 *               event type, SIPX_RESULT_INVALID_ARGS is returned.
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherCreate(const SIPX_INST hInst, 
                                             SIPX_PUB* phPub,
                                             const char* szResourceId,
                                             const char* szEventType,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength);

/**
 * Tears down the publishing context.  Any existing subscriptions
 * are sent a final NOTIFY request.  If pFinalContent is not NULL and 
 * nContentLength > 0 the given publish state is given otherwise
 * the final NOTIFY requests are sent with no body or state.
 * 
 * @param hPub Handle of the publishing context to destroy 
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being 
 *        published
 * @param pFinalContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherDestroy(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const size_t nContentLength);

/**
 * Publishes an updated state to specific event via NOTIFY to its subscribers.
 * 
 * @param hPub Handle of the publishing context 
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being 
 *        published
 * @param pContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherUpdate(const SIPX_PUB hPub,
                                             const char* szContentType,
                                             const char* pContent,
                                             const size_t nContentLength);

//@}

/** @name Conference Methods */
//@{


/**
 * Create a conference handle.  Conferences are an association of calls 
 * where the audio media is mixed.  sipXtapi supports conferences up to
 * 4 (CONF_MAX_CONNECTIONS) parties in its default configuration.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phConference Pointer to a conference handle.  Upon success, 
 *        this value is replaced with a valid conference handle.  
 *        Success is determined by the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF* phConference) ;

/**
 * Join (add) an existing held call into a conference.
 * 
 * An existing call can be added to a virgin conference without restriction.
 * Additional calls, must be connected and on remote hold for this operation 
 * to succeed.   A remote hold can be accomplished by calling sipxCallHold on
 * the joining party.  The application layer must wait for the 
 * CALLSTATE_CONNECTION_INACTIVE event prior to calling join. No events 
 * are fired as part of the operation and the newly joined call is left on 
 * hold.  The application layer should call sipxCallUnhold on the new 
 * participant to finalize the join.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param hCall Call handle of the call to join into the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall) ;

/**
 * Split (remove) a held call from a conference.  This method will remove
 * the specified call from the conference.  
 *
 * The call must be connected and on remote hold for this operation to 
 * succeed.   A remote hold can be accomplished by calling sipxCallHold on 
 * the conference participant or by placing the entire conference on hold 
 * with bridging disabled.  The application layer must wait for the 
 * CALLSTATE_CONNECTION_INACTIVE event prior to calling split. No events 
 * are fired as part of the operation and the split call is left on hold.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle of the call that should be removed from the
 *        the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConf,
                                             const SIPX_CALL hCall) ;

/**
 * Add a new party to an existing conference.  A connection is automatically
 * initiated for the specified address.
 * 
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hLine Line Identity for the outbound call.  The line identity 
 *        helps defines the "From" caller-id.
 * @param szAddress SIP url of the conference partipant to add
 * @param phNewCall Pointer to a call handle to store new call.
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param pDisplay Pointer to an object describing the display object for 
 *        rendering remote video.
 * @param pSecurity Pointer to an object describing the security attributes 
 *        for the call.
 * @param bTakeFocus Should SIPxua place the newly answered call in focus
 *        (engage local microphone and speaker).  In some cases, application
 *        developer may want to answer the call in the background and play
 *        audio while the user finishes up with their active (in focus) call.
 * @param options Pointer to a SIPX_CALL_OPTIONS structure.
 *
 * @see sipxConferenceCreate
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigGetLocalContacts
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_CONTACT_ID contactId = 0,
                                           SIPX_VIDEO_DISPLAY* const pDisplay = NULL,
                                           SIPX_SECURITY_ATTRIBUTES* const pSecurity = NULL,
                                           bool bTakeFocus = true,
                                           SIPX_CALL_OPTIONS* options = NULL);

/**
 * Removes a participant from conference by hanging up on them.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle identifying which call to remove from the
 *        conference by hanging up.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceRemove(const SIPX_CONF hConf,
                                              const SIPX_CALL hCall) ;


/**
 * Gets all of the calls participating in a conference.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param calls An array of call handles filled in by the API.
 * @param iMax The maximum number of call handles to return.
 * @param nActual The actual number of call handles returned.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL calls[], 
                                                const size_t iMax, 
                                                size_t& nActual) ;

/**
 * Places a conference on hold.  This API can be used to place a 
 * conference on local hold (continue to bridge participants) or full hold
 * (remaining participants cannot talk to each other).   The default is
 * local hold/bridged.  The bBridged flag can be used to change this
 * behavior (false for full hold).
 *
 * Developers may also hold/unhold individual conference participants by
 * calling sipxCallHold and sipxCallUnhold on individual call handles.  The
 * sipxConferenceGetCalls API can be used to enumerate conference 
 * participants.
 *
 * @see sipxCallHold for a description of the expected
 *      events.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param bBridging true for a bridging conference hold,
 *        false for a non-bridging conference hold.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceHold(const SIPX_CONF hConf,
                                            bool bBridging = true);
                                            
/**
 * Removes conference members from a held state.  This method will take a call
 * off either local or remote/full hold.
 *
 * @see sipxConferenceHold for details on holding 
 *      conferences.
 * @see sipxCallHold for a description of the expected
 *      events.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceUnhold(const SIPX_CONF hConf);


/**
 * Play the designated audio file to all conference partipants and/or the 
 * local speaker.  The file may be a raw 16 bit signed PCM at 8000 
 * samples/sec, mono, little endian or a .WAV file.
 * 
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param szFile Filename for the audio file to be played.
 * @param bRepeat True if the file is supposed to be played repeatedly
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 */

SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStart(const SIPX_CONF hConf, 
                                                          const char* szFile,
                                                          const bool bRepeat,
                                                          const bool bLocal,
                                                          const bool bRemote) ;


/*
 * Stop playing a file started with sipxConferencePlayAudioFileStart
 * 
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStop(const SIPX_CONF hConf) ;


/**
 * Destroys a conference.  All participants within a conference are
 * dropped. 
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 */ 
SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf) ;


/**
 * Gets energy levels for a conference.  The conference must be in the 
 * connected state (not held or bridged) for this request to succeed.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param iInputEnergyLevel Input/Microphone energy level ranging from 0 to 9.
 * @param iOutputEnergyLevel Output/Speaker energy level ranging from 0 to 9. 
 *        The output energy level is post-mixed (after mixing all files/tones
 *        and other parties.  sipxCallGetEnergyLevel provides an API to obtain
 *        pre-mixed energy levels.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceGetEnergyLevels(const SIPX_CONF hConf,
                                                       int&            iInputEnergyLevel,
                                                       int&            iOutputEnergyLevel) ;



/**
 * Limits the codec preferences on a conference.  This API will force a 
 * codec renegotiation with the specified calls regardless if the codecs 
 * changed.  A renegotiation includes sending a new INVITE with an updated SDP
 * list.  Local audio will be stopped and restarted during this process, 
 * however, hold events are not sent to the application.
 *
 * NOTE: If any calls are on remote hold, they will be taken off hold.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param audioBandwidth A bandwidth id to limit audio codecs. Pass in
 *        AUDIO_CODEC_BW_DEFAULT to leave audio codecs unchanged.
 * @param videoBandwidth A bandwidth id to limit video bitrate and framerate.
 *        (see sipxConfigSetVideoBandwidth for an explanation on how 
 *        bandwidth ids affect bitrate and framerate). Pass in AUDIO_CODEC_BW_DEFAULT
 *        to leave these parameters unchanged.
 * @param szVideoCodecName Codec name that limits the supported video codecs
 *        to this one video codec.
 *        
 * @see sipxConfigSetVideoBandwidth
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceLimitCodecPreferences(const SIPX_CONF hConf,
                                                             const SIPX_AUDIO_BANDWIDTH_ID audioBandwidth,
                                                             const SIPX_VIDEO_BANDWIDTH_ID videoBandwidth,
                                                             const char* szVideoCodecName) ;
//@}

/** @name Audio Methods */
//@{

/**
 * Set the local microphone gain.  If the microphone is muted, 
 * resetting the gain will not enable audio -- you must unmute
 * the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the local microphone gain
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetGain(const SIPX_INST hInst,
                                          const int iLevel) ;


/**
 * Get the current microphone gain.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetGain(const SIPX_INST hInst,
                                          int& iLevel) ;


/**
 * Mute or unmute the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMute True if the microphone is to be muted and false if it 
 *        is not to be muted
 */
SIPXTAPI_API SIPX_RESULT sipxAudioMute(const SIPX_INST hInst,
                                       const bool bMute) ;


/**
 * Gets the mute state of the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMuted True if the microphone has been muted and false if it 
 *        is not mute
 */
SIPXTAPI_API SIPX_RESULT sipxAudioIsMuted(const SIPX_INST hInst,
                                          bool &bMuted) ;


/**
 * Enables one of the speaker outputs.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer 
 *		  (used to alert user of in inbound call) or speaker 
 *        (in call audio device).
 */
SIPXTAPI_API SIPX_RESULT sipxAudioEnableSpeaker(const SIPX_INST hInst,
                                                const SPEAKER_TYPE type) ;


/**
 * Gets the enabled speaker selection.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer 
 *	      (used to alert user of in inbound call) or speaker 
 *        (in call audio device).
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetEnabledSpeaker(const SIPX_INST hInst,
                                                    SPEAKER_TYPE& type) ;


/**
 * Sets the audio level for the designated speaker type.  If the speaker type
 * is enabled, the change it audio will be heard instantly.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer 
 *	      (used to alert user of in inbound call) or speaker 
 *        (in call audio device).
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type, 
                                            const int iLevel) ;


/**
 * Gets the audio level for the designated speaker type
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type The type of the speaker either the logical ringer 
 *		  (used to alert user of in inbound call) or speaker 
 *        (in call audio device).
 * @param iLevel The level of the gain of the microphone
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetVolume(const SIPX_INST hInst,
                                            const SPEAKER_TYPE type, 
                                            int& iLevel) ;


/**
 * Enables or disables Acoustic Echo Cancellation (AEC).  By default, sipXtapi
 * assumes SIPX_AEC_CANCEL_AUTO.  Change this parameter will modify the policy 
 * for both existing and new calls.
 *
 * Note: This API is only supported when bundled with VoiceEngine from 
 * Global IP Sound.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode AEC mode.
 *
 * @see SIPX_AEC_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetAECMode(const SIPX_INST hInst,
                                             const SIPX_AEC_MODE mode) ;


/**
 * Get the mode of Acoustic Echo Cancellation (AEC).
 *
 * Note: This API is only supported when bundled with GIPS VoiceEngine.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode AEC mode.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetAECMode(const SIPX_INST hInst,
                                             SIPX_AEC_MODE&  mode) ;


/**
 * Enable/Disable Automatic Gain Control (AGC).  By default, AGC is disabled.
 *
 * Note: This API is only supported when bundled with GIPS VoiceEngine.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnable true to enable AGC or false to disable
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetAGCMode(const SIPX_INST hInst,
                                             const bool bEnable) ;

/**
 * Get the enable/disable state of Automatic Gain Control (AGC).
 * Note: This API is only supported when bundled with GIPS VoiceEngine.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnabled true if AGC is enabled; otherwise false.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetAGCMode(const SIPX_INST hInst,
                                             bool& bEnabled) ;


/**
 * Set the noise reduction mode/policy for suppressing background noise.  By
 * default sipXtapi assumes SIPX_NOISE_REDUCTION_LOW.  Change this parameter
 * will modify the policy for both existing and new calls.
 *
 * Note: This API is only supported when bundled with VoiceEngine from 
 * Global IP Sound.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode noise reduction mode.
 *
 * @see SIPX_NOISE_REDUCTION_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetNoiseReductionMode(const SIPX_INST hInst,
                                                        const SIPX_NOISE_REDUCTION_MODE mode) ;


/**
 * Get the mode/policy for Noise Reduction (NR).
 *
 * Note: This API is only supported when bundled with GIPS VoiceEngine.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode noise reduction mode.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNoiseReductionMode(const SIPX_INST hInst,
                                                        SIPX_NOISE_REDUCTION_MODE& mode) ;
 
/**
 * Get the number of input devices available on this system.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of input devices available
 *        on this system. 
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     size_t& numDevices) ;

/**
 * Get the name/identifier for input device at position index
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the input device to be queried.
 * @param szDevice Reference an character string pointer to receive 
 *                 the device name.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDevice(const SIPX_INST hInst,
                                                 const int index,
                                                 const char*& szDevice) ;

/**
 * Get the number of output devices available on this system
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of output devices available
 *        on this system. 
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumOutputDevices(const SIPX_INST hInst,
                                                      size_t& numDevices) ;

/**
 * Get the name/identifier for output device at position index
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the output device to be queried.
 * @param szDevice Reference an character string pointer to receive 
 *                 the device name.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDevice(const SIPX_INST hInst,
                                                  const int index,
                                                  const char*& szDevice) ;

/**
 * Set the call input device (in-call microphone).  
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice Character string pointer to be set to
 *                 a string name of the output device. 
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetCallInputDevice(const SIPX_INST hInst,
                                                     const char* szDevice) ;

/**
 * Set the call ringer/alerting device.
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice The call ringer/alerting device.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetRingerOutputDevice(const SIPX_INST hInst,
                                                        const char* szDevice) ;


/**
 * Set the call output device (in-call speaker).
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice The call output device.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetCallOutputDevice(const SIPX_INST hInst,
                                                      const char* szDevice) ;


//@}
/** @name Line / Identity Methods*/
//@{


/**
 * Adds a line appearance.  A line appearance defines your address of record
 * and is used both as your "From" caller-id and as the public identity to 
 * which you will receive calls for.  Directing calls to a particular user 
 * agent is achieved using registrations.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szLineURL The address of record for the line identity.  Can be 
 *        prepended with a Display Name.
 *        e.g. -    "Zaphod Beeblebrox" <sip:zaphb@fourty-two.net>
 * @param phLine Pointer to a line handle.  Upon success, a handle to the 
 *        newly added line is returned.
 * @param contactId Id of the desired contact record to use for this line.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineURL,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_ID contactId = 0) ;

/**
 * Adds an alias for a line definition.  Line aliases are used to map an 
 * inbound call request to an existing line definition.  You should only 
 * need to an a aliase if your network infrastructure directs calls to this
 * user agent using multiple identities.  For example, if user agent 
 * registers as "sip:bandreasen@example.com"; however, calls can also be
 * directed to you via an exention (e.g. sip:122@example.com).
 *
 * If sipXtapi receives a call with an unknown line, you can still answer
 * and interact wtih the call; however, the line handle will be SIPX_LINE_NULL
 * in all event callbacks.  Adding an aliases allows you to correlate another 
 * line url with your line definition and receive real line handles with event
 * callbacks.
 *
 * Line aliases are not used for outbound calls.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxLineAddAlias(const SIPX_LINE hLine, const char* szLineURL) ;

/**
 * Registers a line with the proxy server.  Registrations will be re-registered
 * automatically, before they expire.
 *
 * Unless your user agent is designated a static IP address or DNS name and 
 * that routing information is provisioned into a SIP server, you should 
 * register the line by calling this function.
 *
 * Please unregister your line before calling sipxLineRemove.
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param bRegister true if Registration is desired, otherwise, an Unregister is performed.
 */
SIPXTAPI_API SIPX_RESULT sipxLineRegister(const SIPX_LINE hLine, const bool bRegister);

/**
 * Remove the designated line appearence.  If the line was previous registered 
 * using the sipxLineRegister API, please unregister the line and wait for the
 * unregistered event before calling sipxLineRemove.  Otherwise, the line will
 * be removed without unregistering.  
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 */ 
SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine) ;

/**
 * Adds authentication credentials to the designated line appearance.  
 * Credentials are often required by registration services to verify that the
 * line is being used by the line appearance/address of record owner. 
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szUserID user id used for the line appearance.
 * @param szPasswd passwd used for the line appearance.
 * @param szRealm realm for which the user and passwd are valid.
 */ 
SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(const SIPX_LINE hLine,                                                 
                                               const char* szUserID,
                                               const char* szPasswd,
                                               const char* szRealm) ;

/**
 * Gets the active list of line identities.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param lines Pre-allocated array of line handles.
 * @param max Maximum number of lines to return.
 * @param actual Actual number of valid lines returned.
 */ 

SIPXTAPI_API SIPX_RESULT sipxLineGet(const SIPX_INST hInst,
                                     SIPX_LINE lines[],
                                     const size_t max,
                                     size_t& actual) ;

/**
 * Get the Line URI for the designated line handle
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szBuffer Buffer to place line URL.  A NULL value will return
 *        the amount of storage needed in nActual.
 * @param nBuffer Size of szBuffer in bytes (not to exceed)
 * @param nActual Actual number of bytes written
 */
SIPXTAPI_API SIPX_RESULT sipxLineGetURI(const SIPX_LINE hLine,
                                        char*  szBuffer,
                                        const size_t nBuffer,
                                        size_t& nActual) ;

//@}
/** @name Configuration Methods*/
//@{


/**
 * The sipxConfigEnableLog method enables logging for the sipXtapi API,
 * media processing, call processing, SIP stack, and OS abstraction layer.
 * Logging is disabled by default.  The underlying framework makes no attempts 
 * to bound the log file to a fixed size.
 *
 * Log Format:
 *    time:event id:facility:priority:host name:task name:task id:process id:log message
 *
 * @param logLevel Designates the amount of detail includes in the log.  See
 *        SIPX_LOG_LEVEL for more details.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogLevel(SIPX_LOG_LEVEL logLevel) ;


/** 
 * The sipxConfigSetlogFile method sets the filename of the log file and 
 * directs output to that file
 *
 * NOTE: At this time no validation is performed on the specified filename.  
 * Please make sure the directories exist and the appropriate permissions
 * are available.
 *
 * @param szFilename The filename for the log file.  Designated a NULL 
 *        filename will disable logging, however, threads/resources will not
 *        be deallocated.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogFile(const char *szFilename) ;


/**
 * Set a callback function to collect logging information. This function
 * directs logging output to the specfied function.
 *
 * @param pCallback is a pointer to a callback function. This callback function
 *        gets passed three strings, first string is the priority level,
 *        second string is the source id of the subsystem that generated
 *        the message, and the third string is the message itself.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogCallback(sipxLogCallback pCallback);


/**
 * Enables GIPS tracing in sipXtapi (if bundled with sipXtapi).  Log entries
 * are under the facility "VIDEOENGINE" and "VOICEENGINE" and are Base64 
 * encoded (to avoid issues with binary data.
 *
 * NOTE: Tracing must be enabled prior to setting up a call.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnable true to enable GIPS tracing or false to disable.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableGIPSTracing(SIPX_INST hInst, 
                                                     bool bEnable) ;

/**
 * Designate a callback routine as a microphone replacement or supplement.  
 * The callback is invoked with microphone data and the data can be reviewed,
 * modified, replaced, or discarded.
 * 
 * This callback proc must *NOT* block and must return data quickly.  
 * Additionally, the method should not call any blocking function (i.e. IO 
 * operations, malloc, new, etc).
 *
 * Data must be formatted as mono 16-bit signed PCM, little endian, 8000 
 * samples per second. The callback is handed 80 samples (10ms) of data at 
 * a time.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetMicAudioHook(fnMicAudioHook hookProc) ;


/**
 * Designate a callback routine for post-mixing audio data (e.g. to speaker 
 * data).  The hook may review, modify, replace, or discard data.
 *
 * This callback proc must *NOT* block and must return data quickly.  
 * Additionally, the method should not call any blocking function (i.e. IO 
 * operations, malloc, new, etc).
 *
 * Data must be formatted as mono 16-bit signed PCM, little endian, 8000 
 * samples per second. The callback is handed 80 samples (10ms) of data at 
 * a time.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSpkrAudioHook(fnSpkrAudioHook hookProc) ;

/**
 * Sets the User-Agent name to be used with outgoing SIP messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szName The user-agent name.
 * @param bIncludePlatformName Indicates whether or not to append the
 *        platform description onto the user agent name.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetUserAgentName(const SIPX_INST hInst, 
                                                    const char* szName, 
                                                    const bool bIncludePlatformName = true);
                                                    
/**
 * Defines the SIP proxy used for outbound requests.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szProxy the new outbound proxy
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundProxy(const SIPX_INST hInst,
                                                    const char* szProxy) ;

/**
 * Modifies the timeout values used for DNS SRV lookups.  In generally,
 * you shouldn't need to modified these, however, if you find yourself
 * in a situation where a router/network fails to send responses to
 * DNS SRV requests these values can be tweaked.  Note, failing to send
 * responses is different then a receiving an no-such-animal response.
 * <p>
 * The default values are initialTimeout = 5 seconds, and 4 retries.  The
 * time waited is doubled after each timeout, so with the default settings,
 * a single DNS SRV can block for 75 seconds (5 + 10 + 20 + 40).  In general,
 * 4 DNS SRV requests are made for each hostname (e.g. domain.com):
 * <ul>
 *   <li> _sip._udp.domain.com</li>
 *   <li> _sip._tcp.domain.com</li>
 *   <li> _sip._udp.domain.com.domain.com</li>
 *   <li> _sip._tcp.domain.com.domain.com</li>
 * </ul>
 *
 * If DNS response are dropped in the network (or your DNS server is down), 
 * the API will block for 3 minutes.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvTimeouts(const int initialTimeoutInSecs, 
                                                     const int retries) ;


/**
 * Specifies the time to wait for a REGISTER response before sending a 
 * LINESTATE_REGISTER_FAILED (or LINESTATE_UNREGISTER_FAILED) message.
 * If not set, the user-agent will use a 4 second timeout.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param seconds The number of seconds to wait for a REGISTER response,
                  before it is considered a failure.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterResponseWaitSeconds(const SIPX_INST hInst,
                                                                  const int seconds);   
/**
 * Specifies the time to wait before trying the next DNS SRV record.  The user
 * agent will attempt to obtain DNS SRV resolutions for the child DNS SRV 
 * records.  This setting is the time allowed for attempting a lookup - if the 
 * time expires without a lookup, then next child is attempted.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param failoverTimeoutInSecs Number of seconds until the next DNS SRV 
 *        record is tried.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvFailoverTimeout(const SIPX_INST hInst,
                                                            const int failoverTimeoutInSecs); 


/**
 * Enable or disable the use of "rport".  If rport is included on a via,
 * responses should be sent back to the originating port -- not what is
 * advertised as part of via.  Additionally, the sip stack will not 
 * receive messages sent to the originating port unless this is enabled.  
 *
 * @param hInst Instance pointer obtained by sipxInitialize. 
 * @param bEnable Enable or disable the use of rport.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRport(const SIPX_INST hInst,
                                               const bool bEnable) ;
                                               
/**
 * Specifies the expiration period for registration.  After setting this 
 * configuration, all subsequent REGISTER messages will be sent with the new 
 * registration period. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize. 
 * @param nRegisterExpirationSecs Number of seconds until the expiration of a 
 *        REGISTER message
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterExpiration(const SIPX_INST hInst,
                                                         const int nRegisterExpirationSecs);

/**
 * Specifies the expiration period for subscription.  After setting this 
 * configuration, all subsequent SUBSCRIBE messages will be sent with the new
 * subscribe period. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize. 
 * @param nSubscribeExpirationSecs Number of seconds until the expiration of a
 *        SUBSCRIBE message
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSubscribeExpiration(const SIPX_INST hInst,
                                                          const int nSubscribeExpirationSecs);
   
/**
 * Enables STUN (Simple Traversal of UDP through NAT) support for both 
 * UDP SIP signaling and UDP audio/video (RTP).  STUN helps user agents
 * determine thier external IP address from the inside of NAT/Firewall. 
 * This method should be invoked immediately after calling sipxInitialize 
 * and before creating any lines or calls.  Enabling STUN while calls are 
 * setup should not effect the media path of existing calls.  The "contact"
 * address uses for UDP signaling may change on the next request.
 *
 * TODO :: STUN conforms to IETF RFC/DRAFT XXXX with the following exceptions:
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szServer The stun server that should be used for discovery.
 * @param iServerPort The port of the stun server that should be used for 
 *        discovery.
 * @param iKeepAliveInSecs This setting controls how often to refresh the stun
 *        binding.  The most aggressive NAT/Firewall solutions free port 
 *        mappings after 30 seconds of non-use.  We recommend a value of 28 
 *        seconds to be safe.
 *         
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer, 
                                              int iServerPort,
                                              int iKeepAliveInSecs) ;

/**
 * Disable the use of STUN.  See sipxConfigEnableStun for details on STUN.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableStun(const SIPX_INST hInst) ;


/**
 * Enable TURN for support for UDP audio/video (RTP).  TURN allows VoIP
 * communications while operating behind a symmetric NAT or firewall (cannot 
 * only receive data from the same IP/port that you have sent to).
 *
 * This implementation is based on draft-rosenberg-midcom-turn-04.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szServer The TURN server that should be used for relaying.
 * @param iServerPort The TURN sever port that should be used for relaying
 * @param szUsername TURN username for authentication
 * @param szPassword TURN password for authentication
 * @param iKeepAliveInSecs This setting controls how often to refresh the TURN
 *        binding.  The recommended value is 28 seconds.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableTurn(const SIPX_INST hInst,
                                              const char*     szServer,
                                              const int       iServerPort,
                                              const char*     szUsername,
                                              const char*     szPassword,
                                              const int       iKeepAliveInSecs) ;

/**
 * Disable the use of TURN.  See sipxConfigEnableTurn for details TURN.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableTurn(const SIPX_INST hInst) ;


/**
 * Enables an ICE-like mechanism for determining connecticity of remote 
 * parties dynamically.  By default, ICE is disabled.
 * 
 * The current sipXtapi implementation is a bastardization of 
 * draft-ietf-mmusic-ice-04.  In subsequent release, this will
 * conform to draft-ietf-mmusic-ice-05 or the latest draft.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableIce(const SIPX_INST hInst) ;


/**
 * Disable the use of ICE.  See sipxConfigEnableICE for details.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableIce(const SIPX_INST hInst) ;


/**
 * Add a signaling keep alive to a remote ip address.  
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param contactId Contact ID used for the keep alive.  sipXtapi will
 *        send keep alives from the interface identified by the 
 *        contactId.  Specify a contactId of -1 to send keep alives from
 *        all interfaces.
 * @param type Designates the method of keep alives.
 * @param remoteIp Remote IP address used to send keep alives.  The caller is
 *        responsible for coverting hostnames to IP address.
 * @param remotePort Remote port used to send keep alives.
 * @param keepAliveSecs The number of seconds to wait before resending.  If 
 *        the value is <= 0, only one keep alive will be sent (calling 
 *        sipxConfigKeepAliveRemove will fail).
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveAdd(const SIPX_INST     hInst,
                                                SIPX_CONTACT_ID     contactId,
                                                SIPX_KEEPALIVE_TYPE type,
                                                const char*         remoteIp,
                                                int                 remotePort,
                                                int                 keepAliveSecs) ;


/**
 * Remove a signaling keepalive.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param contactId Contact ID used for the keep alive.  sipXtapi will
 *        remove keep alives from the interface identified by the 
 *        contactId.  Specify a contactId of -1 to remove keep alives from
 *        all interfaces.
 * @param type Designates the method of keep alives.
 * @param remoteIp Remote IP address used to send keep alives.  The caller is
 *        responsible for coverting hostnames to IP address.  This value must
 *        match what was specified in sipxConfigKeepAliveAdd.
 * @param remotePort Remote port used to send keep alives.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveRemove(const SIPX_INST     hInst,
                                                   SIPX_CONTACT_ID     contactId,
                                                   SIPX_KEEPALIVE_TYPE type,
                                                   const char*         remoteIp,
                                                   int                 remotePort) ;


/**
 * Enable/disable sending of out-of-band DTMF tones. If disabled the tones
 * will be sent inband (if in-band dtmf is enabled).  Out-of-band DTMF
 * is enabled by default.
 *
 * Generally, out-of-band DTMF should always be enabled.  In-band DTMF
 * can be distorted and unrecognized by gateways/IVRs/ACDs when using
 * compressed codecs such as G729.  By nature, many compressed codecs
 * are lossy and cannot regenerate DTMF signals.  If you find that you 
 * need to disable out-of-band DTMF (due to duplicate DTMF signals) on 
 * another device, please consider reconfiguring that other device.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Enable or disable out-of-band DTMF tones.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableOutOfBandDTMF(const SIPX_INST hInst,
                                                       const bool bEnable) ;

/**
 * Enable/disable sending of in-band DTMF tones.  If disabled, the tones
 * will be sent out of band (if out-of-band is enabled).  In-band DTMF
 * is enabled by default.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Enable or disable in-band DTMF tones.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableInBandDTMF(const SIPX_INST hInst,
                                                    const bool bEnable) ;
  	 
  	 
/**
 * Enable or disable sending RTCP reports.  By default, RTCP is enabled and
 * sends reports every ~5 seconds.  RTCP is enabled by default.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Enable or disable sending of RTCP reports.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRTCP(const SIPX_INST hInst,
                                              const bool bEnable) ;
  	 

/**
 * Enables/disables sending of DNS SRV request for all sipXtapi instances. 
 * DNS SRV is enabled by default.
 *
 * @param bEnable Enable or disable DNS SRV resolution.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableDnsSrv(const bool bEnable);

/**
 * Determines if sending of out-of-band DTMF tones is enabled or disabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Out-of-band DTMF tones enabled or disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigIsOutOfBandDTMFEnabled(const SIPX_INST hInst,
                                                          bool& bEnable) ;

/**
 * Determines if sending of in-band DTMF tones is enabled or disabled.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable in-band DTMF tones enabled or disabled.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigIsInBandDTMFEnabled(const SIPX_INST hInst,
                                                       bool& bEnable) ;


/**
 * Get the sipXtapi API version string.
 *
 * @param szVersion Buffer to store the version string. A zero-terminated 
 *        string will be copied into this buffer on success.
 * @param nBuffer Size of szBuffer in bytes (not to exceed). A size of 48 bytes
 *        should be sufficient in most cases.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVersion(char* szVersion,
											  const size_t nBuffer) ;

/**
 * Get the local UDP port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipUdpPort(SIPX_INST hInst, int* pPort) ;


/**
 * Get the local TCP port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTcpPort(SIPX_INST hInst, int* pPort) ;


/**
 * Get the local TLS port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTlsPort(SIPX_INST hInst, int* pPort) ;


/**
 * Set the preferred bandwidth requirement for codec selection. Whenever 
 * possible a codec matching that requirement will be selected for a call.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the preference is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bandWidth Valid bandwidth requirements  are AUDIO_CODEC_BW_LOW, 
 *        AUDIO_CODEC_BW_NORMAL, and AUDIO_CODEC_BW_HIGH.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecPreferences(const SIPX_INST hInst, 
                                                            SIPX_AUDIO_BANDWIDTH_ID bandWidth) ;

/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this functon will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetAudioCodecByName(const SIPX_INST hInst, 
                                                       const char* szCodecName) ;

/**
 * Get the current codec preference.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pBandWidth pointer to an integer that will contain AUDIO_CODEC_BW_LOW, 
 *        AUDIO_CODEC_BW_NORMAL, or AUDIO_CODEC_BW_HIGH. AUDIO_CODEC_BW_CUSTOM
 *        will be returned if a specific codec was et using the 
 *        sipxConfigSetAudioCodecByName function.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodecPreferences(const SIPX_INST hInst, 
                                                            SIPX_AUDIO_BANDWIDTH_ID *pBandWidth);

/**
 * Get the number of audio codecs. 
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
 * no be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumAudioCodecs(const SIPX_INST hInst, 
                                                     int* pNumCodecs) ;


/**
 * Get the audio codec at a certain index in the list of codecs. Use this 
 * function in conjunction with sipxConfigGetNumAudioCodecs to enumerate
 * the list of audio codecs.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the audio codec can not
 * be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param index Index in the list of codecs
 * @param pCodec SIPX_AUDIO_CODEC structure that holds information
 *        (name, bandwidth requirement) about the codec.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAudioCodec(const SIPX_INST hInst, 
                                                 const int index, 
                                                 SIPX_AUDIO_CODEC* pCodec) ;

#ifdef VIDEO
/**
 * Set the bandwidth parameters for video codecs.Depending on the bandwidth
 * parameter that is passed in the settings will be set to:
 *
 * VIDEO_CODEC_BW_LOW     bitrate 5 kbps, framerate 10 fps
 * VIDEO_CODEC_BW_NORMAL  bitrate 70 kbps, framerate is what it was set to
 *                        with sipxConfigSetVideoParameters or 30 if not set.
 * VIDEO_CODEC_BW_HIGH    bitrate 400 kbps, framerate is what it was set to
 *                        with sipxConfigSetVideoParameters or 30 if not set.
 *
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the preference is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bandWidth Valid bandwidth requirements  are VIDEO_CODEC_BW_LOW, 
 *        VIDEO_CODEC_BW_NORMAL, and VIDEO_CODEC_BW_HIGH.
 *
 * @see sipxConfigSetVideoParameters
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoBandwidth(const SIPX_INST hInst, 
                                                     SIPX_VIDEO_BANDWIDTH_ID bandWidth);

/**
 * Gets the list of video capture devices.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param arrSzCaptureDevices Array of character arrays to be populated
 *        by this function call.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevices(const SIPX_INST hInst,
                                                          char **arrSzCaptureDevices,
                                                          int nDeviceStringLength,
                                                          int nArrayLength);
                                                          

/**
 * Gets the current video capture device.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCaptureDevice Character array to be populated
 *        by this function call.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevice(const SIPX_INST hInst,
                                                          char* szCaptureDevices,
                                                          int nLength);
                                                          
/**
 * Sets the video capture device.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCaptureDevice Pointer to a character array containing the
 *        name of the desired video capture device.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCaptureDevice(const SIPX_INST hInst,
                                                         const char* szCaptureDevice);
                                                          
                            
/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this functon will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the video codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCodecByName(const SIPX_INST hInst, 
                                                       const char* szCodecName) ;

/**
 * Reset the codec list if it was modified by sipxConfigSetVideoCodecByName. This
 * resets the selection to a full codec list.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigResetVideoCodecs(const SIPX_INST hInst);

/**
 * Get the current codec preference.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pBandWidth pointer to an integer that will contain AUDIO_CODEC_BW_LOW, 
 *        AUDIO_CODEC_BW_NORMAL, or AUDIO_CODEC_BW_HIGH. AUDIO_CODEC_BW_CUSTOM
 *        will be returned if a specific codec was set using the 
 *        sipxConfigSetVideoCodecByName function.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodecPreferences(const SIPX_INST hInst, 
                                                            SIPX_VIDEO_BANDWIDTH_ID *pBandWidth);

/**
 * Get the number of video codecs. 
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
 * no be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumVideoCodecs(const SIPX_INST hInst, 
                                                     int* pNumCodecs) ;


/**
 * Set the supported video format
 * This method will limit the supported video format to either VIDEO_FORMAT_CIF (352x288),
 * VIDEO_FORMAT_QCIF (176x144), VIDEO_FORMAT_SQCIF (128x92), or VIDEO_FORMAT_QVGA (320x240).
 * The method will return SIPX_RESULT_SUCCESS if it is able to set the video format,
 * SIPX_RESULT_FAILURE is returned if the video format can not be set.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFormat(const SIPX_INST hInst,
                                                  SIPX_VIDEO_FORMAT videoFormat);


/**
 * Get the video codec at a certain index in the list of codecs. Use this 
 * function in conjunction with sipxConfigGetNumVideoCodecs to enumerate
 * the list of video codecs.
 * This method will return SIPX_RESULT_SUCCESS if able to set the video codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the video codec can not
 * be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param index Index in the list of codecs
 * @param pCodec SIPX_VIDEO_CODEC structure that holds information
 *        (name, bandwidth requirement) about the codec.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCodec(const SIPX_INST hInst, 
                                                 const int index, 
                                                 SIPX_VIDEO_CODEC* pCodec) ;

#endif // VIDEO

/**
 * Get the local contact address available for outbound/inbound signaling and
 * audio.  The local contact addresses will always include the local IP 
 * addresses.  The local contact addresses may also include external NAT-
 * derived addresses (e.g. STUN).  See the definition of SIPX_CONTACT_ADDRESS
 * for more details on the structure.
 *
 * Determining which contact address to use depends on your network topology.  
 * If you have a proxy/edge proxy within the same firewall/NAT space, you can 
 * use the LOCAL UDP, TCP, or TLS contact type for your calls.  If your 
 * proxy resides outside of the firewall/NAT space, you should use the 
 * NAT_MAPPED or RELAY contact type (UDP only).  Both NAT_MAPPED and RELAY 
 * use your STUN-derived IP address, however RELAY requests TURN for media 
 * paths. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param addresses A pre-allocated list of SIPX_CONTACT_ADDRESS 
 *        structures.  This data will be filled in by the API call.
 * @param nMaxAddresses The maximum number of addresses supplied by the 
 *        addresses parameter.
 * @param nActualAddresses The actual number of addresses populated in
 *        the addresses parameter.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalContacts(const SIPX_INST hInst,
                                                    SIPX_CONTACT_ADDRESS addresses[],
                                                    size_t nMaxAddresses,
                                                    size_t& nActualAddresses) ;


/**
 * Populates an array of IP Addresses in UtlString* form.  The array must be preallocated to 
 * contain MAX_IP_ADDRESSES elements.
 *
 * @param arrAddresses Pre-allocated array to be popluated with ip addresses.
 * @param arrAddressAdapter For each record in arrAddresses, there is a corresponding record,
 *        with the same index, in arrAddressAdpater which represents the 
 *        "sipx adapter name" for that address
 * @param numAddresses Reference to the number of IPs found by the system.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAllLocalNetworkIps(const char* arrAddresses[], 
                                                         const char* arrAddressAdapter[],
                                                         int &numAddresses);
                                                         
/**
 * Set security parameters for an instance of sipXtapi.
 * @deprecated These should be set using sipxInitialize.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szDbLocation The directory in which the certificate database resides.
 * @param szMyCertNickname The local user's certificate nickname, for database lookup.
 * @param szDbPassword The password for the certificated database.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSecurityParameters(const SIPX_INST hInst,
                                                         const char* szDbLocation,
                                                         const char* szMyCertNickname,
                                                         const char* szDbPassword);                                        

/**
 * Enables/Disables use of short field names in sip messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if short names, false if long names
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipShortNames(const SIPX_INST hInst, 
                                                       const bool bEnabled);

/**
 * Enables/Disables use of date header in sip messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if date header, false if no date header
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipDateHeader(const SIPX_INST hInst, 
                                                       const bool bEnabled);
/**
 * Enables/Disables use of allow header in sip messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if allow header, false if no allow header
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipAllowHeader(const SIPX_INST hInst, 
                                                       const bool bEnabled);
/**
 * Sets the Accept Language used in sip messages. e.g. - "EN"
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szLanguage - Accept Language string
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSipAcceptLanguage(const SIPX_INST hInst, 
                                                        const char* szLanguage);
/**
 * Sets the location header for SIP messages.  The location header will be
 * included in SIP requests and responses.  You can disable the location 
 * header on a call-by-call basis in the by changing the bEnableLocationHeader
 * flag on sipxCallAccept, sipxCallConnect, and sipxConferenceAdd methods.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szHeader - Location header
 *
 * @see sipxCallAccept
 * @see sipxCallConnect
 * @see sipxConferenceAdd
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLocationHeader(const SIPX_INST hInst, 
                                                     const char* szHeader);

/**
 * Set the connection idle timeout.  If a media connection is idle for this
 * threshold, a SILENCE event will be fired to the application layer.  
 * 
 * Applications may decide to tear down the call after receiving this event 
 * under the assumption that the remote party is gone away.  Be careful when 
 * using codecs that support silence suppression -- Some implementations 
 * continue to send RTP heartbeats, however, others will not send any data 
 * and may appear to be dead.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param idleTimeout The time in seconds that a socket is idle before a
 *        MEDIA_REMOTE_SILENT event is fired.        
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetConnectionIdleTimeout(const SIPX_INST hInst, 
                                                            const int idleTimeout);

#ifdef VIDEO

/**
 * Sets the display object for the "video preview".
 *
 * @param pDisplay Pointer to a video preview display object.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoPreviewDisplay(const SIPX_INST hInst, 
                                                          SIPX_VIDEO_DISPLAY* const pDisplay);


/**
 * Updates the Preview window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUpdatePreviewWindow(const SIPX_INST hInst, const SIPX_WINDOW_HANDLE hWnd);


/**
 * Sets the video quality.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param quality Id setting the video quality.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoQuality(const SIPX_INST hInst, const SIPX_VIDEO_QUALITY_ID quality);

/**
 * Sets the bit rate and frame rate parameters for video.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter in kbps
 * @param frameRate Frame rate parameter frames per second
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoParameters(const SIPX_INST hInst, 
                                                      const int bitRate,
                                                      const int frameRate);

/**
 * Sets the video bitrate
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter 
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoBitrate(const SIPX_INST hInst, 
                                                   const int bitRate);

/**
 * Sets the video framerate
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter 
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFramerate(const SIPX_INST hInst, 
                                                     const int frameRate);

/**
 * Set the cpu usage
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param cpuUsage CPU usage in percent
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCpuUsage(const SIPX_INST hInst, 
                                                    const int cpuUsage);

#endif


/** 
 * Subscribe for NOTIFY events which may be published by another end-point or
 * server.
 *
 * sipXtapi will automatically refresh subscriptions until 
 * sipxConfigUnsubscribe is called.  Please make sure you call 
 * sipxCallUnsubscribe before tearing down your instance of sipXtapi.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hLine Line Identity for the outbound call.  The line identity 
 *        helps defines the "From" caller-id.
 * @param szTargetUrl The Url of the publishing end-point. 
 * @param szEventType A string representing the type of event that can be 
 *        published.  This string is used to populate the "Event" header in
 *        the SIP SUBSCRIBE request.  For example, if checking voicemail 
 *        status, your would use "message-summary".
 * @param szAcceptType A string representing the types of NOTIFY events that 
 *        this client will accept.  This string is used to populate the 
 *        "Accept" header in the SIP SUBSCRIBE request.  For example, if
 *        checking voicemail status, you would use 
 *        "application/simple-message-summary"
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param phSub Pointer to a subscription handle whose value is set by this 
 *        funtion.  This handle allows you to cancel the subscription and
 *        differeniate between NOTIFY events.
 */ 
SIPXTAPI_API SIPX_RESULT sipxConfigSubscribe(const SIPX_INST hInst, 
                                             const SIPX_LINE hLine, 
                                             const char* szTargetUrl, 
                                             const char* szEventType, 
                                             const char* szAcceptType, 
                                             const SIPX_CONTACT_ID contactId, 
                                             SIPX_SUB* phSub); 
/*
 * Unsubscribe from previously subscribed NOTIFY events.  This method will
 * send another subscription request with an expires time of 0 (zero) to end
 * your subscription.
 *
 * @param hSub The subscription handle obtained from the call to 
 *             sipxConfigSubscribe.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUnsubscribe(const SIPX_SUB hSub); 


//@}

#endif // _sipXtapi_h_
