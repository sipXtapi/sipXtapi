//
// Copyright (C) 2006-2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpResourceTopology_h_
#define _MpResourceTopology_h_

// SYSTEM INCLUDES
#include <mp/MpTypes.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <utl/UtlDList.h>
#include <utl/UtlHashMap.h>

// APPLICATION INCLUDES
// DEFINES
#define DEFAULT_FROM_FILE_RESOURCE_NAME "FromFile1"
#define DEFAULT_FROM_INPUT_DEVICE_RESOURCE_NAME "FromMic1"
#define DEFAULT_BRIDGE_RESOURCE_NAME "Bridge1"
#define DEFAULT_VIDEO_MIXER_RESOURCE_NAME "VideoMixer1"
#define DEFAULT_SPEAKER_SELECTOR_RESOURCE_NAME "SpeakerSelector1"
#define DEFAULT_TONE_GEN_RESOURCE_NAME "ToneGen1"
#define DEFAULT_TONE_DETECT_RESOURCE_NAME "ToneDetect"
#define DEFAULT_TO_OUTPUT_DEVICE_RESOURCE_NAME "ToSpeaker1"
#define DEFAULT_SPEEX_PREPROCESS_RESOURCE_NAME "SpeexPreprocess"
#define DEFAULT_NULL_RESOURCE_NAME "Null1"
#define DEFAULT_RTP_INPUT_RESOURCE_NAME "InRtp-%d"
#define DEFAULT_VIDEO_RTP_INPUT_RESOURCE_NAME "InVideoRtp-%d"
#define DEFAULT_DECODE_RESOURCE_NAME "Decode-%d"
#define DEFAULT_RTP_OUTPUT_RESOURCE_NAME "OutRtp-%d"
#define DEFAULT_VIDEO_RTP_OUTPUT_RESOURCE_NAME "OutVideoRtp-%d"
#define DEFAULT_ENCODE_RESOURCE_NAME "Encode-%d"
#define DEFAULT_RECORDER_RESOURCE_NAME "CallRecorder"
#define DEFAULT_TO_OUTPUT_SPLITTER_RESOURCE_NAME "ToOutputSplitter1"
#define DEFAULT_VAD_RESOURCE_NAME "Vad"
#define DEFAULT_VOICE_ACTIVITY_NOTIFIER_RESOURCE_NAME "VoiceActivityNotifier"
#define DEFAULT_DELAY_RESOURCE_NAME "Delay"
#define DEFAULT_NOTCH_FILTER_RESOURCE_NAME "NotchFilter"

#define CONNECTION_NAME_SUFFIX "-%d"
#define STREAM_NAME_SUFFIX "-%d"
#define MIC_NAME_SUFFIX "-Mic"
#define SPEAKER_NAME_SUFFIX "-Spkr"
#define AEC_OUTPUT_BUFFER_RESOURCE_NAME_SUFFIX "-outBuffer"
#define AEC_NAME_SUFFIX "-AEC"

/**
*  VIRTUAL_NAME_LOCAL_STREAM_OUTPUT is mapped to an output of local audio
*  input stream (i.e. stream from Mic).
*/
#define VIRTUAL_NAME_LOCAL_STREAM_OUTPUT "StreamOutput" MIC_NAME_SUFFIX

/**
*  VIRTUAL_NAME_LOCAL_STREAM_INPUT is mapped to an input of local audio
*  output stream (i.e. stream to Speaker).
*/
#define VIRTUAL_NAME_LOCAL_STREAM_INPUT "StreamInput" SPEAKER_NAME_SUFFIX

/**
*  VIRTUAL_NAME_RTP_STREAM_OUTPUT is mapped to an outputs of RTP streams. This
*  way we don't need to know about RTP stream implementation details no more.
*  E.g. RTP stream might end with Voice Activity Detector resource or with Delay
*  resource and VIRTUAL_NAME_RTP_STREAM_OUTPUT will always be mapped to
*  the latest one.
*/
#define VIRTUAL_NAME_RTP_STREAM_OUTPUT "StreamOutput" CONNECTION_NAME_SUFFIX

/**
*  VIRTUAL_NAME_CONNECTION_PORTS is mapped to inputs and outputs to which
*  output and input RTP connections should be linked. Usually this virtual
*  inputs and output are mapped to Bridge inputs and outputs. But in case
*  we want to insert Speaker Selection resource before the Bridge, virtual
*  VIRTUAL_NAME_CONNECTION_PORTS inputs will be mapped to Speaker Selection
*  inputs (because the Bridge inputs will be connected to the Speaker Selection
*  outputs).
*/
#define VIRTUAL_NAME_CONNECTION_PORTS  "ConnectionPorts"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceFactory;
class UtlHashMapIterator;

/**
*  @brief MpResourceTopology is used to define a set of resources and how they
*         are to be connected in a flowgraph.
*
*  MpResourceTopology defines a list of resources to be created and a list of
*  connections defining how the resources are to be connected in a flowgraph.
*  There are two types of MpResourceTopologies that may be defined:
*  
*  1) Full topologies which define all the resources and connections for a
*  new, empty flowgraph.  In a full topology the connections must reference
*  only resources defined in the MpResourceTopology.
*
*  2) Incremental topologies which define a set of resources to be added to
*  a flowgraph and a set of connections defining how the new resources are
*  connected amongst themselves and the existing resources in the flowgraph.
*  An incremental topology the connections may reference resources which
*  already exist in the flowgraph or resources defined in the MpResourceTopology.
*
*  The MpResourceTopology only defines the resources and connections to
*  be created.  The actual creation and linking of resources is performed
*  by the flowgraph itself.  The flowgraph uses a MpResourceFactory to
*  create the resources listed in the MpResourceTopology.  The list of
*  resources in the MpResourceTopology consists of pairs of a resource
*  type and name.  The type is used by the flowgraph to instruct the
*  MpResourceFactory to create a specific resource type.  The name is
*  the name the resource is to assume in the flowgraph.
*
*/
class MpResourceTopology
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum MpLogicalPortTypes
    {
        MP_TOPOLOGY_NEXT_AVAILABLE_PORT = -1
    };

    typedef UtlHashMapIterator* VirtualPortIterator;

      /// A resource type for use with addResources().
    struct ResourceDef
    {
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
       // Visual Studio 6.0 is broken when it comes to statically initializing
       // an array of structs containing object references, so in this case
       // just store const char* -- Note: this means reduced functionality
       // under VS6 -- only static names and types can be used here.
       const char* resourceType;
       const char* resourceName;
#else
       const UtlString resourceType;
       const UtlString resourceName;
#endif

       MpConnectionID connectionId;
       int streamId;
    };
      /// A connection type for use with addConnections().
    struct ConnectionDef
    {
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
       // See above notice about VS6 being broken
       const char* outputResourceName;
#else
       const UtlString outputResourceName;
#endif

       int outputPortIndex;

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
       // See above notice about VS6 being broken
       const char* inputResourceName;
#else
       const UtlString inputResourceName;
#endif

       int inputPortIndex;
    };

/* ============================ CREATORS ================================== */

      /// Constructor
    MpResourceTopology();

      /// Destructor
    virtual ~MpResourceTopology();

/* ============================ MANIPULATORS ============================== */

     /// Add a new resource definition to the topology
   OsStatus addResource(const UtlString& resourceType,
                        const UtlString& resourceName,
                        MpConnectionID connectionId = MP_INVALID_CONNECTION_ID,
                        int streamId = -1);
     /**<
     *  @param[in] resourceType - type for MpResourceFactory construction
     *  @param[in] resourceName - unique name for resource in this topology and
     *             ultimately in flowgraph.
     *  @param[in] connectionId - the ID of a connection this resource will
     *             belong to. If set to MP_INVALID_CONNECTION_ID then it will be
     *             replaced with the concrete ID if requested.
     *  @param[in] streamId = the ID of a stream in a connection this resource
     *             will belong to.
     *  @returns OS_NAME_IN_USE if resourceName already exists in this topology
     */

     /// Add resources from an array.
   OsStatus addResources(const ResourceDef *defines, int numDefines);
     /**<
     *  This method is barely a convenient wrapper around addResource().
     *
     *  @see addResource() for more details.
     */

     /// Add a new connection definition to the topology
   OsStatus addConnection(const UtlString& outputResourceName,
                          int outputPortIndex,
                          const UtlString& inputResourceName,
                          int inputPortIndex);

     /// Add resources from an array.
   OsStatus addConnections(const ConnectionDef *defines, int numDefines);
     /**<
     *  This method is barely a convenient wrapper around addConnection().
     *  This method also supports easy creation of chained resources. If
     *  outputResourceName is NULL for some connections, name of a source
     *  resource is taken from previous inputResourceName. This implies
     *  that first connection should always have outputResourceName set.
     *
     *  @see addConnection() for more details.
     *
     *  @return The same values as addConnection() and OS_INVALID_ARGUMENT
     *          if first connection have NULL outputResourceName.
     */

     /// Add a new virtual input definition to the topology
   OsStatus addVirtualInput(const UtlString& realResourceName,
                            int realPortIndex,
                            const UtlString& virtualResourceName,
                            int virtualPortIndex);
     /**<
     *  @retval OS_INVALID_ARGUMENT if given virtual port is already used.
     *  @retval OS_SUCCESS on success.
     */

     /// Add a new virtual output definition to the topology
   OsStatus addVirtualOutput(const UtlString& realResourceName,
                             int realPortIndex,
                             const UtlString& virtualResourceName,
                             int virtualPortIndex);
     /**<
     *  @retval OS_INVALID_ARGUMENT if given virtual port is already used.
     *  @retval OS_NOT_FOUND if real resource name is not found in topology.
     *  @retval OS_SUCCESS on success.
     */

     /// Validate that the connections are connect the resources
   OsStatus validateConnections(UtlString& firstUnconnectedResourceName,
                                UtlString& firstDanglingResourceName,
                                UtlBoolean allowExternalResources = FALSE) const;
     /**<
     *  A full topology MUST NOT have connections that reference resources
     *  outside this topology AND all resources MUST be connected to such that
     *  there are no dangling resources.  An incremental topology MAY have
     *  connections that reference resources outside of this topology AND
     *  all resources in this topology MUST have at least one connection.
     *
     *  @param[out] firstUnconnectedResourceName - first resource found to not
     *              have any connections.
     *  @param[out] firstDanglingResourceName - first resource found to not be
     *              connected topologically to the rest of the resources.  This
     *              can only be tested in full topologies.  We do not know the
     *              pre-existing topology in the flowgraph for incremental
     *              topologies.
     *  @param[in] allowExternalResources - FALSE = full topology, TRUE =
     *              incremental topology (see definitions at top of this class).
     */

     /// Validate that all the resource types in this topology have a constructor in the factory
   OsStatus validateResourceTypes(MpResourceFactory& resourceFactory,
                                  int& firstInvalidResourceIndex) const;
      /**<
      *  @param[in]  resourceFactory - resource factory to test against.
      *  @param[out] firstInvalidResourceIndex - index of the first resource
      *              found to lack constructor in the provided factory.
      */

     /// Utility to replace "%d" in resource name with an integer
   static void replaceNumInName(UtlString& resourceName,
                                int resourceNum);

/* ============================ ACCESSORS ================================= */

     /// Get the resource definition indicated by the resourceIndex
   OsStatus getResource(int resourceIndex,
                        UtlString& resourceType,
                        UtlString& resourceName,
                        MpConnectionID& connectionId,
                        int& streamId) const;

     /// Get the resource name by the resourceIndex
   OsStatus getResource(int resourceIndex,
                        UtlString& resourceName) const;

     /// Get the resource name and port index by the virtual input name and port index.
   OsStatus getInputVirtualResource(const UtlString &virtualName,
                                    int virtualPort,
                                    UtlString& resourceName,
                                    int &resourcePort) const;

     /// Get the resource name and port index by the virtual output name and port index.
   OsStatus getOutputVirtualResource(const UtlString &virtualName,
                                     int virtualPort,
                                     UtlString& resourceName,
                                     int &resourcePort) const;

     /// Get the connection definition indicated by the connectionIndex
   OsStatus getConnection(int connectionIndex,
                          UtlString& outputResourceName,
                          int& outputPortIndex,
                          UtlString& inputResourceName,
                          int& inputPortIndex);

     /// Get the next logical port, used when the same port number ???
   int getNextLogicalPortNumber();
     /**<
     *  Used when more than one connection should refer to the same
     *  port number, but the port number is not yet known (e.g. use
     *  the next available port).
     */

     /// Initialize virtual input ports iterator.
   void initVirtualInputIterator(VirtualPortIterator &portIter);

     /// Destroy virtual input ports iterator.
   void freeVirtualInputIterator(VirtualPortIterator &portIter);

     /// Get virtual input data from an iterator.
   OsStatus getNextVirtualInput(VirtualPortIterator &portIter,
                                UtlString& realResourceName,
                                int &realPortIndex,
                                UtlString& virtualResourceName,
                                int &virtualPortIndex);
     /**<
     *  This method can be called after initVirtualInputIterator() to get first
     *  virtual input data and then again to get next virtual input data and
     *  so on. On reaching the end of the virtual inputs list method returns
     *  OS_NO_MORE_DATA.
     *
     *  @retval OS_NO_MORE_DATA - No more virtual ports available. Output
     *          parameters are not touched.
     *  @retval OS_SUCCESS - Next virtual port fetched and all output parameters
     *          are filled with correct values.
     */

     /// Initialize virtual output ports iterator.
   void initVirtualOutputIterator(VirtualPortIterator &portIter);

     /// Destroy virtual output ports iterator.
   void freeVirtualOutputIterator(VirtualPortIterator &portIter);

     /// Get virtual output data from an iterator.
   OsStatus getNextVirtualOutput(VirtualPortIterator &portIter,
                                 UtlString& realResourceName,
                                 int &realPortIndex,
                                 UtlString& virtualResourceName,
                                 int &virtualPortIndex);
     /**<
     *  This method can be called after initVirtualOutputIterator() to get first
     *  virtual output data and then again to get next virtual output data and
     *  so on. On reaching the end of the virtual outputs list method returns
     *  OS_NO_MORE_DATA.
     *
     *  @retval OS_NO_MORE_DATA - No more virtual ports available. Output
     *          parameters are not touched.
     *  @retval OS_SUCCESS - Next virtual port fetched and all output parameters
     *          are filled with correct values.
     */

    /// Dump resources to debug string
    int dumpResources(UtlString& dumpString);

    /// Dump connections to debug string
    int dumpConnections(UtlString& dumpString);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   int mPriorLogicalPort;
   UtlDList mResources;
   UtlDList mConnections;
   UtlHashMap mVirtualInputs;  ///< Mapping between virtual and real inputs.
   UtlHashMap mVirtualOutputs; ///< Mapping between virtual and real outputs.

   int findResourceConnections(const UtlString& resourceName,
                               UtlContainer& connectionsToTraverse) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Disabled copy constructor
   MpResourceTopology(const MpResourceTopology& rMpResourceTopology);

     /// Disabled assignment operator
   MpResourceTopology& operator=(const MpResourceTopology& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceTopology_h_
