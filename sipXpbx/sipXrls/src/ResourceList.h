// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceList_h_
#define _ResourceList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <net/HttpBody.h>
#include <net/SipPublishContentMgr.h>
#include <os/OsBSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ResourceListResource;
class ResourceListResourceInstance;


/**
 * This class maintains information about a resource list that is needed to
 * generate resource list events, as described in RFC 4662.
 */

class ResourceList : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource list.
   ResourceList(/// The user-part of the resource list URI.
                const char* userPart,
                /** The domain name of the canonical form of the resource list
                 *  URI, which is the sipX domain. */
                const UtlString& domainName,
                /** The host-part of the resource list URI as it is in
                 *  the SUBSCRIBE we receive. */
                const UtlString& hostPart,
                /// The event-type to handle.
                const char* eventType,
                /// The content-type to handle.
                const char* contentType,
                /// The SipPublishContentMgr for sending event notices.
                SipPublishContentMgr& eventPublisher);

   virtual ~ResourceList();

   //! Generate the HttpBody for the current state of the resource list.
   HttpBody* generateBody() const;

   //! Create and add a resource to the resource list.
   //  Returns the generated ResourceListResource object.
   ResourceListResource* addResource(const char* uri,
                                     const char* nameXml);

   //! Get the event type for the resource list.
   // Return value is valid as long as the ResourceList exists.
   const char* getEventType() const;

   //! Get the content type for the resource list.
   // Return value is valid as long as the ResourceList exists.
   const char* getContentType() const;

   //! Get the SIP domain name for the resource list.
   // Return value is valid as long as the ResourceList exists.
   const char* getDomainName() const;

   //! Get the resource name URI for the resource list.
   // Return value is valid as long as the ResourceList exists.
   const char* getResourceName() const;

   //! Get the resource request URI for the resource list.
   // Return value is valid as long as the ResourceList exists.
   const char* getResourceUri() const;

   //! Get the event publisher for the resource list.
   SipPublishContentMgr* getEventPublisher() const;

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! SIP domain name.
   UtlString mDomainName;
   //! Event type.
   UtlString mEventType;
   //! Content type.
   UtlString mContentType;
   //! Resource name, the URI of the resource list showing the sipX domain
   UtlString mResourceName;
   //! Resource URI, the URI of the SUBSCRIBE as it reaches us.
   UtlString mResourceUri;
   //! version number
   int mVersion;

   //! The SipPublishContentMgr.
   SipPublishContentMgr* mEventPublisher;

   //! List of contained ResourceListResource's.
   UtlSList mResources;

   /** Reader/writer lock for synchronization of the ResourceList and its
    *  contained ResourceListResources.
    */
   mutable OsBSem mSemaphore;

   //! Disabled copy constructor
   ResourceList(const ResourceList& rResourceList);

   //! Disabled assignment operator
   ResourceList& operator=(const ResourceList& rhs);

};

/* ============================ INLINE METHODS ============================ */

//! Get the event type for the resource list.
inline const char* ResourceList::getEventType() const
{
   return mEventType.data();
}

//! Get the content type for the resource list.
inline const char* ResourceList::getContentType() const
{
   return mContentType.data();
}

//! Get the SIP domain name for the resource list.
inline const char* ResourceList::getDomainName() const
{
   return mDomainName.data();
}

//! Get the resource name URI for the resource list.
inline const char* ResourceList::getResourceName() const
{
   return mResourceName.data();
}

//! Get the resource request URI for the resource list.
inline const char* ResourceList::getResourceUri() const
{
   return mResourceUri.data();
}

//! Get the event publisher for the resource list.
inline SipPublishContentMgr* ResourceList::getEventPublisher() const
{
   return mEventPublisher;
}


//! Container for a resource within a ResourceList.
/** The ResourceListResource object knows the ResourceList it is contained within,
 *  and does not have an independent life.
 */
class ResourceListResource : public UtlContainableAtomic
{
   // *** this should be replaced by appropriate accessors
   friend class ResourceList;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an empty body of a dialog event package
   ResourceListResource(ResourceList* resourceList,
                        const char* state,
                        const char* entity);

   //! Destructor
   virtual ~ResourceListResource();

   //! Get the containing resource list.
   ResourceList* getResourceList() const;

   /// Add an instance to the resource and publish appropriate notices.
   void addInstancePublish(const char* instanceName,
                           const char* subscriptionState);

   /// Add an instance to the resource.
   void addInstance(const char* instanceName,
                    const char* subscriptionState);

   /// Delete an instance from the resource and publish appropriate notices.
   void deleteInstancePublish(const char* instanceName,
                              const char* subscriptionState,
                              const char* resourceSubscriptionState);

   /// Delete an instance from the resource.
   void deleteInstance(const char* instanceName,
                       const char* subscriptionState);

   /// A notice has arrived for an instance.  Publish it.
   //  Also sets the subscription state to "active".
   void updateInstancePublish(const char* instanceName,
                              const char* bytes,
                              int length);

   /// Find the contained instance with a particular name.
   //  Returns NULL if not found.
   ResourceListResourceInstance* findInstance(const char* instanceName);

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The containing ResourceList.
   ResourceList* mResourceList;

   //! The URI of the resource.
   UtlString mUri;
   //! The XML fragment containing the <name> elements for the resource.
   UtlString mNameXml;

   //! The instances of this resource.
   UtlSList mInstances;

   //! Disabled copy constructor
   ResourceListResource(const ResourceListResource& rResourceListResource);

   //! Disabled assignment operator
   ResourceListResource& operator=(const ResourceListResource& rhs);

};

//! Get the containing resource list.
inline ResourceList* ResourceListResource::getResourceList() const
{
   return mResourceList;
}


//! Container for an instance of a resource within a ResourceList.
/** The ResourceListResourceInstance object knows the
 *  ResourceListResource it is contained within, and does not have an
 *  independent life.
 */
class ResourceListResourceInstance : public UtlContainableAtomic
{
   friend class ResourceList;
   friend class ResourceListResource;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an instance
   ResourceListResourceInstance(const char* instanceName,
                                const char* subscriptionState);

   //! Destructor
   virtual ~ResourceListResourceInstance();

   //! Get the containing resource list resource.
   ResourceListResource* getResourceListResource() const;

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The containing ResourceListResource.
   ResourceListResource* mResourceListResource;

   //! The instance name.
   UtlString mInstanceName;
   //! The subscription state.
   UtlString mSubscriptionState;

   //! The current content for the resource.
   UtlString mContent;
   //! Whether content is present for the resource.
   UtlBoolean mContentPresent;

   //! Disabled copy constructor
   ResourceListResourceInstance(const ResourceListResourceInstance& rResourceListResourceInstance);

   //! Disabled assignment operator
   ResourceListResourceInstance& operator=(const ResourceListResourceInstance& rhs);

};

//! Get the containing resource list.
inline ResourceListResource* ResourceListResourceInstance::getResourceListResource() const
{
   return mResourceListResource;
}

#endif  // _ResourceList_h_
