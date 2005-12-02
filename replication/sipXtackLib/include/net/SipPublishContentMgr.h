// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _SipPublishContentMgr_h_
#define _SipPublishContentMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class HttpBody;
class UtlString;

// TYPEDEFS

//! Class for managing body content to be accepted via PUBLISH or provided in NOTIFY requests
/*! This class is basically a database that is used to store and retrieve
 *  content (i.e. SIP Event state bodies).  This class does not actually
 *  touch or process SIP messages.  It is used by other classes and applications
 *  to store and retrieve content related to SIP SUBSCRIBE, NOTIFY and
 *  PUBLISH requests.  The resourceId and eventTypeKey have no symmantics.
 *  It is up to the application or event package to decide what the 
 *  resourceId and eventTypeKey look like.  A suggested default is to
 *  use:  <userId>@<hostname>[:port] as provided in the SUBSCRIBE or
 *  PUBLISH request URI.  It is suggested that host be the domainname
 *  not the specific IP address.  It is also suggested the SIP event
 *  type token be used (without any event header parameters) as the
 *  eventTypeKey.  Only in special cases where the content varies
 *  based upon an event parameter, should the parameter(s) be include
 *  included in the eventTypeKey.
 *
 * \par Put Event State In
 *  Applications put Event state information for a specific resourceId
 *  and eventTypeKey into the SipPublishContentMgr via the publish method.
 *
 * \par Retrieve Event State
 *  Applications retrieve published content type via the getContent
 *  method.
 *
 * \par Remove Event State
 * All event state information for a resource Id and event type key can
 * can be removed via the unpublish method.  The content associated
 * with the keys is passed back so the application can destroy the
 * content.
 *
 * \par Default Event State
 * It is possible to define a default event state for an event type key.
 * This default content is provided in the getContent method if no
 * content was provided for the specific resource Id.  Default content
 * is set via the publishDefault method.
 */
class SipPublishContentMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    //! Callback used to notify interested appliactions when content has changed
    /*! Well behaved applications that register and implement this function
     *  should not block.  They should quickly return as failure to do so
     *  may hinder timely processing and system performance.  The memory
     *  for the content provided in the arguments should not be presumed to
     *  exist beyond the point where the application returns from this function.
     *
     *  /param applicationData - provided with the callback function pointer
     *  when it was registered.
     *  The rest of the arguments in this callback have the same meaning as 
     *  the getContent method.
     */
    typedef void (*SipPublisherContentChangeCallback) (void* applicationData,
                                                       const char* resourceId,
                                                       const char* eventTypeKey,
                                                       const char* eventType,
                                                       UtlBoolean isDefaultContent);

/* ============================ CREATORS ================================== */

    //! Default publish container constructor
    SipPublishContentMgr();


    //! Destructor
    virtual
    ~SipPublishContentMgr();


/* ============================ MANIPULATORS ============================== */

    //! Provide the default content for the given event type key
    /*! If content cannot be found for a specific resourceId, then
     *  the default content for a given eventTypeKey is provided.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no symantics enforced.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.
     *  \param eventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content 
     *         types match more than one of the mime types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.  The bodies are
     *         NOT copied.  The application is expected to provide bodies
     *         that will exist as long as they are contained in this class.
     */
    virtual UtlBoolean publishDefault(const char* eventTypeKey,
                                      const char* eventType,
                                      int numContentTypes,
                                      HttpBody* eventContent[],
                                      int maxOldContentTypes,
                                      int& numOldContentTypes,
                                      HttpBody* oldEventContent[]);

    //! Remove the default content for eventTypeKey, returning the old content
    virtual UtlBoolean unpublishDefault(const char* eventTypeKey,
                                        const char* eventType,
                                      int maxOldContentTypes,
                                      int& numOldContentTypes,
                                      HttpBody* oldEventContent[]);

    //! Provide the given content for the resource and event type key
    /*! An application provides content (i.e. SIP event state bodies)
     *  through this interface for the given resourceId and eventTypeKey.
     *  The resourceId and eventTypeKey together compose a unique key which
     * identifies the provided content.  The resourceId is optained from
     * the PUBLISH or SUBSCRIBE request URI.  The eventTypeKey is obtained
     * from the SIP Event header field.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         symantics enforces.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent orderf for all uses of eventTypeKey in this class.
     *         There is no symantics enforces.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.
     *  \param eventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content 
     *         types match more than one of the mime types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.  The bodies are
     *         NOT copied.  The application is expected to provide bodies
     *         that will exist as long as they are contained in this class.
     */
    virtual UtlBoolean publish(const char* resourceId,
                               const char* eventTypeKey,
                               const char* eventType,
                               int numContentTypes,
                               HttpBody* eventContent[],
                               int maxOldContentTypes,
                               int& numOldContentTypes,
                               HttpBody* oldEventContent[]);

    //! Remove the content for the given resourceId and eventTypeKey
    /*! The content bodies are given back so that the application can
     *  release or delete the bodies.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         symantics enforces.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no symantics enforces.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param maxOldContentTypes - the maximum size of the given eventContent array.
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.
     *  \param oldEventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content 
     *         types match more than one of the mime types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.  The bodies are
     *         NOT copied.  The application is expected to provide bodies
     *         that will exist as long as they are contained in this class.
     */
    virtual UtlBoolean unpublish(const char* resourceId,
                                const char* eventTypeKey,
                                const char* eventType,
                               int maxOldContentTypes,
                               int& numOldContentTypes,
                               HttpBody* oldEventContent[]);

    //! Get the content for the given resourceId, eventTypeKey and contentTypes
    /*! Retrieves the content type identified by the resourceID and eventTypeKey.
     *  The given contentTypes indicates what content types are accepted (i.e.
     *  the mime types from the SUBSCRIBE Accept header).
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         symantics enforces.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no symantics enforces.  This is an opaque string used 
     *         as part of the key.
     *  \param numContentTypes - the number of mime types in the contentTypes
     *         array.  numContentTypes is -1 and getContent returns FALSE if
     *         maxContentTypes is not big enough.
     *  \param acceptHeaderValue - the mime types allowed to be returned in 
     *         the content argument.  The first match is the one returned.
     *         This string has the same syntax/format as the SIP accept header.
     *  \param content - the content body if a match was found, otherwise NULL.
     *         The content body is a copy that must be freed.
     *  \param isDefaultContent - if there was no content specific to the resourceId
     *         and default content was provided for the given eventTypeKey,
     *         then isDefaultContent is set to TRUE and numContentTypes and
     *         content contain values from the eventTypeKey content default.
     */
    virtual UtlBoolean getContent(const char* resourceId,
                                  const char* eventTypeKey,
                                  const char* acceptHeaderValue,
                                  HttpBody*& content,
                                  UtlBoolean& isDefaultContent);

    //! Set the callback which gets invoked when ever the content changes
    /*! Currently only one observer is allowed per eventTypeKey.  If
     *  a subsequent observer is set for the same eventTypeKey, it replaces
     *  the existing one.  The arguments of the callback function have
     *  the same meaning as getContent.
     *  Note: the callback is invoked when the default content changes as well.
     *  When the default content for an eventTypeKey changes, the
     *  resourceId is NULL.  The application is responsible for knowing
     *  which resources do not have specific content (i.e. are observing
     *  the default content and may need to be notified).
     *  \param eventType - SIP event type token
     *  \param applicationData - application specific data that is to be
     *         passed back to the application in the callback function.
     *  Returns TRUE if the callback is set for the eventTypeKey.  Will
     *  not set the callback if it is already set for the given eventTypeKey
     */
    virtual UtlBoolean setContentChangeObserver(const char* eventType,
                                                void* applicationData,
                             SipPublisherContentChangeCallback callbackFunction);

    //! Remove the current observer for the eventTypeKey
    /*! If the given callbackFunction does not match the existing one,
     *  this method returns FALSE and the existing observer(s) remain.
     */
    virtual UtlBoolean removeContentChangeObserver(const char* eventType,
                                                   void*& applicationData,
                            SipPublisherContentChangeCallback& callbackFunction);


/* ============================ ACCESSORS ================================= */

    //! Get some debug information
    void getStats(int& numDefaultContent,
                    int& numResourceSpecificContent,
                    int& numCallbacksRegistered);
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! parse the accept header field and create a HashMap with a UtlString for each MIME type
    UtlBoolean buildContentTypesContainer(const char* acceptHeaderValue, 
                                          UtlHashMap& contentTypes);

    //! Copy constructor NOT ALLOWED
    SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr);

    //! Assignment operator NOT ALLOWED
    SipPublishContentMgr& operator=(const SipPublishContentMgr& rhs);



    //! lock for single thread use
    void lock();

    //! unlock for use
    void unlock();

    OsMutex mPublishMgrMutex;
    UtlHashMap mContentEntries; 
    UtlHashMap mDefaultContentEntries;
    UtlHashMap mEventContentCallbacks;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishContentMgr_h_
