// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ResourceList.h>
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <utl/XmlContent.h>
#include <utl/UtlSListIterator.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// URN for the xmlns attribute for Resource List Meta-Information XML.
#define RLMI_XMLNS "urn:ietf:params:xml:ns:rlmi"
// MIME information for RLMI XML.
#define RLMI_CONTENT_TYPE "application/rlmi+xml"

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ResourceList::TYPE = "ResourceList";
const UtlContainableType ResourceListResource::TYPE = "ResourceListResource";
const UtlContainableType ResourceListResourceInstance::TYPE = "ResourceListResourceInstance";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceList::ResourceList(const char* userPart,
                           const UtlString& domainName,
                           const UtlString& hostPart,
                           const char* eventType,
                           SipPublishContentMgr& eventPublisher) :
   mEventType(eventType),
   // Generate the initial report with version 1, so we can generate
   // the default report with version 0 in the default content.
   mVersion(1),
   mEventPublisher(&eventPublisher),
   mSemaphore(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Compose the resource list name.
   mResourceName = userPart;
   mResourceName.append("@");
   mResourceName.append(domainName);
   // Compose the resource list URI as it will appear in the SUBSCRIBE.
   mResourceUri = userPart;
   mResourceUri.append("@");
   mResourceUri.append(hostPart);
}


// Destructor
ResourceList::~ResourceList()
{
   mResources.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

//! Create and add a resource to the resource list.
//  Returns the generated ResourceListResource object.
ResourceListResource* ResourceList::addResource(const char* uri,
                                                const char* nameXml)
{
   // Serialize access to the resource list.
   OsLock lock(mSemaphore);
   
   ResourceListResource* rlr = new ResourceListResource(this, uri, nameXml);
   mResources.append(rlr);

   return rlr;
}
/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

//! Generate the HttpBody for the current state of the resource list.
HttpBody* ResourceList::generateBody() const
{
   // Serialize access to the resource list.
   OsLock lock(mSemaphore);

   // This is the Resource List Meta-Information, XML describing the resources
   // and their instances.  It is the main part of the NOTIFY body.
   UtlString rlmi;

   // Generate the initial part of the RLMI.
   rlmi += "<?xml version=\"1.0\"?>\r\n";
   rlmi += "<list xmlns=\"" RLMI_XMLNS "\" uri=\"";
   XmlEscape(rlmi, mResourceName);
   rlmi += "\" version=\"";
   XmlDecimal(rlmi, mVersion);
   rlmi += "\" fullState=\"true\">\r\n";
   // If we implemented names for resource lists, <name> elements would go here.

   // Iterate through the resources.
   UtlSListIterator resourcesItor(mResources);
   ResourceListResource* resource;
   while ((resource = dynamic_cast <ResourceListResource*> (resourcesItor())))
   {
      // Generate the preamble for the resource.
      rlmi += "  <resource uri=\"";
      XmlEscape(rlmi, resource->mUri);
      rlmi += "\">\r\n";
      rlmi += resource->mNameXml;

      // Iterate through the instances of the resource.
      UtlSListIterator instancesItor(resource->mInstances);
      ResourceListResourceInstance* instance;
      while ((instance =
              dynamic_cast <ResourceListResourceInstance*> (instancesItor())))
      {
         // Generate the XML for the instance.
         // *** currently does not include content!
         rlmi += "    <instance id=\"";
         XmlEscape(rlmi, instance->mInstanceName);
         rlmi += "\" state=\"";
         // Subscription states don't require escaping.
         rlmi += instance->mSubscriptionState;
         rlmi += "\"/>\r\n";
      }

      // Generate the postamble for the resource.
      rlmi += "  </resource>\r\n";
   }

   // Generate the postamble for the resource list.
   rlmi += "</list>\r\n";

   // *** Cheat, and just put the RLMI in the body.  The body really should
   // be multipart.
   HttpBody* body = new HttpBody(rlmi.data(), rlmi.length(),
                                 RLMI_CONTENT_TYPE);

   return body;
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceList::getContainableType() const
{
   return ResourceList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListResource::ResourceListResource(ResourceList* resourceList,
                                           const char* uri,
                                           const char* nameXml) :
   mResourceList(resourceList),
   mUri(uri),
   mNameXml(nameXml)
{
   // If the name XML is not empty and does not end with NL, add CR-NL.
   if (!mNameXml.isNull() && mNameXml(mNameXml.length() - 1) != '\n')
   {
      mNameXml += "\r\n";
   }
}


// Destructor
ResourceListResource::~ResourceListResource()
{
   mInstances.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

/// Add an instance to the resource list and publish appropriate notices.
void ResourceListResource::addInstancePublish(const char* instanceName,
                                              const char* subscriptionState)
{
   ResourceList* resourceList = getResourceList();
   SipPublishContentMgr* eventPublisher = resourceList->getEventPublisher();
   const char* eventType = resourceList->getEventType();
   const char* resourceUri = resourceList->getResourceUri();

   // Add the instance to the resource.
   addInstance(instanceName, subscriptionState);

   // Generate the notice body.
   HttpBody* body = resourceList->generateBody();
   // Publish the notice.
   eventPublisher->publish(resourceUri, eventType, eventType, 1, &body, TRUE);
}

void ResourceListResource::addInstance(const char* instanceName,
                                       const char* subscriptionState)
{
   ResourceListResourceInstance* inst =
      new ResourceListResourceInstance(instanceName, subscriptionState);
   mInstances.append(inst);
}

/// Delete an instance from the resource list and publish appropriate notices.
void ResourceListResource::deleteInstancePublish(const char* instanceName,
                                                 const char* subscriptionState)
{
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceListResource::getContainableType() const
{
   return ResourceListResource::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListResourceInstance::ResourceListResourceInstance(const char* instanceName,
                                                           const char* subscriptionState) :
   mInstanceName(instanceName),
   mSubscriptionState(subscriptionState)
{
}


// Destructor
ResourceListResourceInstance::~ResourceListResourceInstance()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceListResourceInstance::getContainableType() const
{
   return ResourceListResourceInstance::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
