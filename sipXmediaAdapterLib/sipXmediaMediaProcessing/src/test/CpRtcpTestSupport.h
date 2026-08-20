//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpRtcpTestSupport_h_
#define _CpRtcpTestSupport_h_

// Scaffolding shared by the media-interface RTCP tests.
//
// There are two such suites because they have different build requirements:
//
//   CpRtcpMuxTest  needs only RTCP, and so runs on the default build. It
//                  covers plain RTCP and rtcp-mux, neither of which involves
//                  encryption.
//   CpSrtcpTest    additionally needs ENABLE_SRTP, because every one of its
//                  cases turns on a negotiated crypto suite.
//
// Keeping the scaffolding here rather than duplicating it is what lets the
// unencrypted cases be tested in the configuration most builds actually ship,
// where the SRTP-gated suite compiles to nothing.

#include <string.h>

#include <sipxunittests.h>
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "CpTopologyGraphInterface.h"
#include "mi/CpMediaInterface.h"

#include <sdp/SdpMediaLine.h>
#include <sdp/SdpCodecList.h>

#include <os/OsTask.h>
#include <os/OsSocket.h>
#include <os/OsDatagramSocket.h>
#include <os/OsSysLog.h>

// Test hook exported by RTCPConnection.cpp under DEBUGGING_RTCP_REPORTS.  The
// production reporting cadence is 5 seconds, which would make these suites
// intolerably slow.  The period is sampled when a connection is constructed,
// so it must be set before createConnection().
extern "C" { extern int adjustRtcpPeriod(int x); }

// Defaults for Media Interface Factory and Media Interface initialization.
#define CP_RTCP_TEST_FRAME_SIZE_MS       0
#define CP_RTCP_TEST_MAX_SAMPLE_RATE     0
#define CP_RTCP_TEST_DEFAULT_SAMPLE_RATE 0

// Ports, chosen to avoid CpCryptoTest (6000) and CpDtlsTest (16000/17000).
//
// The RTP sink and the collector must differ: an endpoint really does emit RTP
// once startRtpSend() is called and an audio source is live, and aiming it at
// the collector makes the capture race between a 172 byte PCMU packet and the
// report actually under test.
#define CP_RTCP_TEST_ENDPOINT_PORT   18000
#define CP_RTCP_TEST_COLLECTOR_PORT  18100
#define CP_RTCP_TEST_RTP_SINK_PORT   18200

// Shortened RTCP reporting period. Correctness does not depend on this being
// generous: startRtpSendImpl() latches the renderer closed before starting it,
// so a report cannot precede a key install however tight the cadence.
#define CP_RTCP_TEST_REPORT_PERIOD_MS  50

// How long to wait for reports to arrive before giving up.
#define CP_RTCP_TEST_COLLECT_TIMEOUT_MS  6000

#ifdef WIN32
#include <string>
static std::string cpRtcpTestExecutableDir()
{
   char buf[MAX_PATH];
   memset(buf, 0, sizeof(buf));
   DWORD len = GetModuleFileNameA(GetModuleHandle(NULL), buf, sizeof(buf) - 1);
   for (char* p = buf + len; p > buf; p--)
   {
      if (*p == '\\')
      {
         *p = 0;
         break;
      }
   }
   return std::string(buf);
}
#endif


/// Helpers with no dependence on a fixture. Free functions rather than a base
/// fixture class so the suites stay single-inheritance from
/// SIPX_UNIT_BASE_CLASS, which is what the unit test macros expect.
namespace CpRtcpTestSupport
{

/// Does this look like a plain (unprotected) RTCP packet? Version 2 and a
/// packet type in the RTCP range. SRTCP leaves both of those fields in the
/// clear, so this alone cannot distinguish the two -- use it only to confirm a
/// capture is RTCP-shaped, never as proof of protection.
inline bool looksLikeRtcp(const unsigned char* buf, int len)
{
   if (len < 8) return false;
   if ((buf[0] & 0xC0) != 0x80) return false;
   return (buf[1] >= 200 && buf[1] <= 204);
}

/// Sum of the length fields across a compound RTCP packet, in bytes, stopping
/// at the first sub-packet that does not parse.
///
/// For plain RTCP this accounts for the whole datagram exactly. It cannot do
/// so for SRTCP: RFC 3711 encrypts everything past the first 8 bytes,
/// including the headers of the second and later sub-packets, so the walk runs
/// into ciphertext and stops early at a meaningless offset. That asymmetry is
/// a "this is not plaintext RTCP" signal only; the real evidence of protection
/// is that the bytes decrypt.
inline int rtcpDeclaredLength(const unsigned char* buf, int len)
{
   int offset = 0;
   while (offset + 4 <= len)
   {
      if ((buf[offset] & 0xC0) != 0x80) break;
      int words = (buf[offset + 2] << 8) | buf[offset + 3];
      int bytes = (words + 1) * 4;
      if (bytes <= 0 || offset + bytes > len) break;
      offset += bytes;
   }
   return offset;
}

/// Build an RTCP Sender Report. 28 bytes.
inline int buildSenderReport(unsigned char* buf, uint32_t ssrc)
{
   memset(buf, 0, 28);
   buf[0] = 0x80;                    // V=2, P=0, RC=0
   buf[1] = 200;                     // PT = SR
   buf[2] = 0x00; buf[3] = 0x06;     // length = 6 (28 bytes)
   buf[4] = (unsigned char)(ssrc >> 24);
   buf[5] = (unsigned char)(ssrc >> 16);
   buf[6] = (unsigned char)(ssrc >> 8);
   buf[7] = (unsigned char)(ssrc);
   buf[8]  = 0xc7;                   // NTP timestamp msw
   buf[12] = 0x40;                   // NTP timestamp lsw
   buf[19] = 0xe8;                   // RTP timestamp
   buf[23] = 0x64;                   // sender packet count
   buf[26] = 0x27; buf[27] = 0x10;   // sender octet count
   return 28;
}

/// Build an RTP packet: 12 byte header plus a short payload.
inline int buildRtpPacket(unsigned char* buf, uint32_t ssrc, uint16_t seq)
{
   memset(buf, 0, 16);
   buf[0] = 0x80;                    // V=2
   buf[1] = 0;                       // M=0, PT=0 (PCMU)
   buf[2] = (unsigned char)(seq >> 8);
   buf[3] = (unsigned char)(seq);
   buf[7] = 0x64;                    // timestamp
   buf[8]  = (unsigned char)(ssrc >> 24);
   buf[9]  = (unsigned char)(ssrc >> 16);
   buf[10] = (unsigned char)(ssrc >> 8);
   buf[11] = (unsigned char)(ssrc);
   buf[12] = 0xde; buf[13] = 0xad; buf[14] = 0xbe; buf[15] = 0xef;
   return 16;
}

/// Block until an RTCP datagram lands on the collector socket, or the timeout
/// expires. Returns the byte count, or 0 on timeout.
///
/// The endpoint's sockets are OsNatDatagramSockets, so the port also carries
/// STUN, and a misrouted media packet would land here too. Datagrams are
/// demultiplexed the way RFC 7983 and MprFromNet::pushPacket do it -- 0-3 is
/// STUN, 128-191 is RTP/RTCP -- and then narrowed to RTCP by packet type, per
/// the RFC 5761 section 4 rule that RTP payload types never collide with the
/// RTCP packet types 192-223.
///
/// Filtering on the type rather than just the range matters: RTP shares the
/// 128-191 first-byte range, so the looser test would hand back a media packet
/// and fail downstream with a confusing "not RTCP shaped". SRTCP leaves both
/// header bytes in the clear, so this holds for protected traffic too.
inline int collectOneReport(OsDatagramSocket& collector,
                            unsigned char* buf,
                            int bufSize,
                            int timeoutMs)
{
   int elapsed = 0;
   const int slice = 50;
   while (elapsed < timeoutMs)
   {
      if (collector.isReadyToRead(slice))
      {
         int n = collector.read((char*)buf, bufSize);
         if (n > 0)
         {
            if (n >= 2 && buf[0] >= 128 && buf[0] <= 191 &&
                buf[1] >= 192 && buf[1] <= 223)
            {
               return n;
            }
            continue;   // STUN, or RTP that found its way here
         }
      }
      elapsed += slice;
   }
   return 0;
}

/// Poll a connection's inbound RTCP count until it reaches atLeast, or give
/// up. Returns the last value seen.
inline int waitForRtcpCount(CpTopologyGraphInterface* ti, int connId,
                            int atLeast, int timeoutMs)
{
   int elapsed = 0;
   int rtp = 0, rtcp = 0;
   while (elapsed < timeoutMs)
   {
      if (ti->getRtpPacketCounts(connId, CpMediaInterface::AUDIO_STREAM,
                                 rtp, rtcp) == OS_SUCCESS && rtcp >= atLeast)
      {
         return rtcp;
      }
      OsTask::delay(25);
      elapsed += 25;
   }
   return rtcp;
}

} // namespace CpRtcpTestSupport

#endif  // _CpRtcpTestSupport_h_
