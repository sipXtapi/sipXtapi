// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _CpTopologyGraphFactoryImpl_h_
#define _CpTopologyGraphFactoryImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mi/sipXmediaFactoryImpl.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceTopology;
class MpResourceFactory;
class MpInputDeviceManager;
class MpOutputDeviceManager;

/** 
*  @brief Subsystem manager and creator of CpTopologyGraphInterfaces
*         specialization of CpMediaInterface.
*
*  When createMediaInterface is invoked a new CpTopologyGraph (flowgraph)
*  is constructed and used inside the CpTopologyGraphInterface (CpMediaInterface).
*  The topology and types of the resources in the new flowgraph are determined
*  by the MpResourceTopology last set via setInitialResourceTopology.  The
*  specific resources are instantiated using the MpResourceFactory last set
*  via setResourceFactory.
*/
class CpTopologyGraphFactoryImpl : public sipXmediaFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

      /// @copydoc sipXmediaFactoryImpl::sipXmediaFactoryImpl()
    CpTopologyGraphFactoryImpl();

      /// Destructor.
    virtual
    ~CpTopologyGraphFactoryImpl();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    virtual void initialize(   OsConfigDb* pConfigDb,
                               uint32_t frameSizeMs, 
                               uint32_t maxSamplesPerSec,
                               uint32_t defaultDeviceSamplesPerSec);
      /// @brief Factory method to construct new flowgraph and media interface
      /// using CpTopologyFlowgraph.
    virtual void destroy();
    OsMutex* getLock();
    virtual
    IMediaInterface* createMediaInterface(const char* publicAddress,
                                                             const char* localAddress,
                                                             int numCodecs,
                                                             SdpCodec* sdpCodecArray[],
                                                             const char* locale,
                                                             int expeditedIpTos,
                                                             const ProxyDescriptor& stunServer,
                                                             const ProxyDescriptor& turnProxy,
                                                             const ProxyDescriptor& arsProxy,
                                                             const ProxyDescriptor& arsHttpProxy,
                                                             SIPX_MEDIA_PACKET_CALLBACK pMediaPacketCallback,
                                                             UtlBoolean bEnableICE,
                                                             uint32_t samplesPerSec);

    virtual OsStatus getVideoCpuValue(int& cpuValue) const
    {
        return OS_NOT_SUPPORTED;
    }

    // For now inherit all of the modification methods from parent
    // Eventually they will all need to be replaced with specific 
    // implementations

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Set the resource topology to be used when creating new flowgraph.
    virtual
    void setInitialResourceTopology(MpResourceTopology& resourceTopology);
      /**<
      *  The given CpTopolgyGraph defines the topology and types of resources
      *  that are instantiated in the new flowgraph.
      */

      /// @brief Get the resource topology defining the resources connections
      /// for a new flowgraph.
    virtual
    MpResourceTopology* getInitialResourceTopology() const;

      /// Set the factory used to create resources for the new flowgraph contained .
    virtual
    void setResourceFactory(MpResourceFactory& resourceFactory);

      /// Get the factory for constructing new resources.
    virtual
    MpResourceFactory* getResourceFactory() const;

      /// @brief Set the resource topology to be added to the flow graph when
      /// adding an RTP connection.
    virtual
    void setConnectionResourceTopology(MpResourceTopology& connectionTopology);

      /// @brief Get the resource topology defining what resources get added
      /// when adding an RTP connection.
    virtual
    MpResourceTopology* getConnectionResourceTopology() const;

    MpInputDeviceManager* getInputDeviceManager() const;

      /// Build a resource factory with the default set of resource constructors.
    MpResourceFactory* buildDefaultResourceFactory();

    // For now inherit all of the accessors methods from parent
    // Eventually they will all need to be replaced with specific 
    // implementations

//@}

      /// Build a default initial resource topology.
    static
    MpResourceTopology* buildDefaultInitialResourceTopology();

      /// Build a default incremental resource topology for RTP connections.
    static
    MpResourceTopology* buildDefaultIncrementalResourceTopology();

    OsStatus setMicDataHook(MICDATAHOOK pFunc);
    OsStatus setSpeakerDataHook(SPEAKERDATAHOOK pFunc);
    OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const;


/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    MpResourceTopology    *mpInitialResourceTopology;
    MpResourceFactory     *mpResourceFactory;
    MpResourceTopology    *mpConnectionResourceTopology;
    MpInputDeviceManager  *mpInputDeviceManager;
    MpOutputDeviceManager *mpOutputDeviceManager;
    MpInputDeviceHandle  mSourceDeviceId;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphFactoryImpl_h_
