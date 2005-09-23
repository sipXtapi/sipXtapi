// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 

//Author: Daniel Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsTask.h>
#include <os/OsConfigDb.h>
#include <sipXmediaFactoryImpl.h>
#include <net/QoS.h>
#include <net/SdpCodecFactory.h>

#define CODEC_G711_PCMU         "258"   // ID for PCMU
#define CODEC_G711_PCMA         "257"   // ID for PCMA
#define CODEC_DTMF_RFC2833      "128"   // ID for RFC2833 DMTF 

/* ============================ FUNCTIONS ================================= */

int main(int argc, const char* argv[])
{

    OsConfigDb configDb;

    // Create the factory to instantiate flow graphs.
    sipXmediaFactoryImpl mediaInterfaceFactory(&configDb);

    const char* publicAddress = "127.0.0.1";
    const char* localAddress = "127.0.0.1";

    const int numCodecs = 3;
    SdpCodec::SdpCodecTypes sdpCodecEnumArray[numCodecs];
    sdpCodecEnumArray[0] = SdpCodecFactory::getCodecType(CODEC_G711_PCMU);
    sdpCodecEnumArray[1] = SdpCodecFactory::getCodecType(CODEC_G711_PCMA);
    sdpCodecEnumArray[2] = SdpCodecFactory::getCodecType(CODEC_DTMF_RFC2833);
    SdpCodecFactory codecFactory;
    codecFactory.buildSdpCodecFactory(numCodecs, sdpCodecEnumArray);
    SdpCodec** sdpCodecArray = NULL;
    int numCodecsInArray;
    codecFactory.getCodecs(numCodecsInArray,
                           sdpCodecArray);

    const char* locale = "";
    int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS;

    const char* szStunServer = "";
    int stunOptions = 0;
    int iStunKeepAliveSecs = 25;

    // Instatiate a flow graph
    CpMediaInterface* callMediaInterface = 
        mediaInterfaceFactory.createMediaInterface(publicAddress,
                                                 localAddress,
                                                 numCodecsInArray,
                                                 sdpCodecArray,
                                                 locale,
                                                 expeditedIpTos,
                                                 szStunServer,
                                                 stunOptions,
                                                 iStunKeepAliveSecs);

    while(1)
    {
        OsTask::delay(10000);
    }


    if(sdpCodecArray)
    {
        for (int codecIndex = 0; codecIndex < numCodecsInArray; codecIndex++)
        {
            delete sdpCodecArray[codecIndex];
        }
        delete[] sdpCodecArray;
    }

    return(1);
}