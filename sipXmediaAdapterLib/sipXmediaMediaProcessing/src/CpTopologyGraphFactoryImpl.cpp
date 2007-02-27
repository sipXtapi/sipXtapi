//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpInputDeviceManager.h>
#include <mp/MpMisc.h>
#include <include/CpTopologyGraphFactoryImpl.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <include/CpTopologyGraphInterface.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS
extern CpMediaInterfaceFactory* spFactory;
extern int siInstanceCount;

extern "C" CpMediaInterfaceFactory* cpTopologyGraphFactoryFactory(OsConfigDb* pConfigDb)
{
    if(spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new CpTopologyGraphFactoryImpl(pConfigDb));
    }    
    siInstanceCount++;
    return spFactory;
}

#ifdef ENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    return(cpTopologyGraphFactoryFactory(pConfigDb));
}
#endif


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpTopologyGraphFactoryImpl::CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb) :
sipXmediaFactoryImpl(pConfigDb)
{    
    mpInitialResourceTopology = NULL;
    mpResourceFactory = NULL;
    mpConnectionResourceTopology = NULL;

    assert(MpMisc.RawAudioPool);
    mpInputDeviceManager = 
        new MpInputDeviceManager(80, // samples per frame
                                 8000, // samples per second
                                 4, // number of buffered frames saved
                                 *MpMisc.RawAudioPool);
}


// Destructor
CpTopologyGraphFactoryImpl::~CpTopologyGraphFactoryImpl()
{
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* 
CpTopologyGraphFactoryImpl::createMediaInterface(const char* publicAddress,
                                                 const char* localAddress,
                                                 int numCodecs,
                                                 SdpCodec* sdpCodecArray[],
                                                 const char* locale,
                                                 int expeditedIpTos,
                                                 const char* stunServer,
                                                 int stunPort,
                                                 int stunKeepAliveSecs,
                                                 const char* turnServer,
                                                 int turnPort,
                                                 const char* turnUserName,
                                                 const char* turnPassword,
                                                 int turnKeepAliveSecs,
                                                 UtlBoolean enableIce)
{
    return(new CpTopologyGraphInterface(this, 
                                       publicAddress, 
                                       localAddress, 
                                       numCodecs, 
                                       sdpCodecArray, 
                                       locale, 
                                       expeditedIpTos, 
                                       stunServer,
                                       stunPort, 
                                       turnKeepAliveSecs, 
                                       turnServer, 
                                       turnPort, 
                                       turnUserName, 
                                       turnPassword, 
                                       turnKeepAliveSecs, 
                                       enableIce));
}

/* ============================ ACCESSORS ================================= */

void CpTopologyGraphFactoryImpl::setInitialResourceTopology(MpResourceTopology& resourceTopology)
{
    mpInitialResourceTopology = &resourceTopology;
}

MpResourceTopology* CpTopologyGraphFactoryImpl::getInitialResourceTopology() const
{
    return(mpInitialResourceTopology);
}

void CpTopologyGraphFactoryImpl::setResourceFactory(MpResourceFactory& resourceFactory)
{
    mpResourceFactory = &resourceFactory;
}

MpResourceFactory* CpTopologyGraphFactoryImpl::getResourceFactory() const
{
    return(mpResourceFactory);
}

void CpTopologyGraphFactoryImpl::setConnectionResourceTopology(MpResourceTopology& connectionResourceTopology)
{
    mpConnectionResourceTopology = &connectionResourceTopology;
}

MpResourceTopology* CpTopologyGraphFactoryImpl::getConnectionResourceTopology() const
{
    return(mpConnectionResourceTopology);
}

MpInputDeviceManager* CpTopologyGraphFactoryImpl::getInputDeviceManager() const
{
    return(mpInputDeviceManager);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


