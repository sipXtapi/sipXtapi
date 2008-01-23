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
#include <sipXmediaFactoryImpl.h>

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

      /// Constructor.
    CpTopologyGraphFactoryImpl(OsConfigDb* pConfigDb,
                               uint32_t maxSamplesPerFrame, 
                               uint32_t maxSamplesPerSec);

      /// Destructor.
    virtual
    ~CpTopologyGraphFactoryImpl();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// @brief Factory method to construct new flowgraph and media interface
      /// using CpTopologyFlowgraph.
    virtual
    CpMediaInterface* createMediaInterface(const char* publicAddress,
                                           const char* localAddress,
                                           int numCodecs,
                                           SdpCodec* sdpCodecArray[],
                                           const char* locale,
                                           int expeditedIpTos,
                                           const char* stunServer,
                                           int stunOptions,
                                           int stunKeepAliveSecs,
                                           const char* turnServer,
                                           int turnPort,
                                           const char* turnUserName,
                                           const char* turnPassword,
                                           int turnKeepAlivePeriodSecs,
                                           UtlBoolean enableIce);

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

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpTopologyGraphFactoryImpl_h_
