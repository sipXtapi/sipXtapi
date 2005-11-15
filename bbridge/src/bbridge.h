#ifndef _BBRIDGE_H_
#define _BBRIDGE_H_
// Constants used throughout Boston Bridge

#ifndef SIPX_VERSION
#  include "config/bbridge-buildstamp.h"
#  define SIPX_BBRIDGE_VERSION          bbridgeVersion
#  define SIPX_BBRIDGE_VERSION_COMMENT  bbridgeBuildStamp
#else
#  define SIPX_BBRIDGE_VERSION          SIPX_VERSION
#  define SIPX_BBRIDGE_VERSION_COMMENT  ""
#endif

#ifndef SIPX_LOGDIR
#  define SIPX_LOGDIR "."
#endif

#ifndef SIPX_CONFDIR
#  define SIPX_CONFDIR "."
#endif

#define CONFIG_SETTINGS_FILE          "bbridge.conf"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "bbridge.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define LOG_FACILITY                  FAC_CONFERENCE

#define DEFAULT_UDP_PORT              5060       // Default UDP port
#define DEFAULT_TCP_PORT              5060       // Default TCP port
#define DEFAULT_TLS_PORT              5061       // Default TLS port
#define DEFAULT_XMLRPC_PORT           8201
#define DEFAULT_RTP_START             15000
#define DEFAULT_RTP_END               20000

#endif
