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
#include <net/NameValuePair.h>
#include <net/HttpMessage.h>
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
                           const char* contentType,
                           SipPublishContentMgr& eventPublisher) :
   mDomainName(domainName),
   mEventType(eventType),
   mContentType(contentType),
   // Generate the initial report with version 1, so we can generate
   // the default report with version 0 in the default content.
   mVersion(1),
   mEventPublisher(&eventPublisher),
   mSemaphore(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Compose the resource list name.
   mResourceName = "sip:";
   mResourceName.append(userPart);
   mResourceName.append("@");
   mResourceName.append(domainName);
   // Compose the resource list URI as it will appear in the SUBSCRIBE.
   mResourceUri = "sip:";
   mResourceUri.append(userPart);
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

   // Counter for generating CIDs.
   int partCounter = 0;
   
   // Construct the multipart body.
   // We add the <...> here, as they are used in all the contexts where
   // rlmiBodyPartCid appears.
   UtlString rlmiBodyPartCid;
   rlmiBodyPartCid += "<rlmi@";
   rlmiBodyPartCid += mDomainName;
   rlmiBodyPartCid += ">";

   UtlString content_type(CONTENT_TYPE_MULTIPART_RELATED
                          ";type=\"" RLMI_CONTENT_TYPE "\""
                          ";start=\"");
   content_type += rlmiBodyPartCid;
   content_type += "\"";
   HttpBody* body = new HttpBodyMultipart(content_type);

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
         rlmi += "    <instance id=\"";
         XmlEscape(rlmi, instance->mInstanceName);
         rlmi += "\" state=\"";
         // Subscription states don't require escaping.
         rlmi += instance->mSubscriptionState;
         rlmi += "\"";

         // Generate the body part for the instance, if necessary.
         if (instance->mContentPresent)
         {
            UtlString contentBodyPartCid;
            XmlDecimal(contentBodyPartCid, ++partCounter);
            contentBodyPartCid += "@";
            contentBodyPartCid += mDomainName;

            rlmi += " cid=\"";
            rlmi += contentBodyPartCid;
            rlmi += "\"";

            // Now add the <...> and use it in the header.
            contentBodyPartCid.prepend("<");
            contentBodyPartCid.append(">");

            HttpBody content_body(instance->mContent.data(),
                                  instance->mContent.length(),
                                  mContentType);
            UtlDList content_body_parameters;
            content_body_parameters.append(
               new NameValuePair(HTTP_CONTENT_ID_FIELD,
                                 contentBodyPartCid));
            body->appendBodyPart(content_body, content_body_parameters);
            content_body_parameters.destroyAll();
         }
         
         rlmi += "/>\r\n";
      }

      // Generate the postamble for the resource.
      rlmi += "  </resource>\r\n";
   }

   // Generate the postamble for the resource list.
   rlmi += "</list>\r\n";

   // Construct the RLMI body part.
   HttpBody rlmi_body(rlmi.data(), rlmi.length(), RLMI_CONTENT_TYPE);
   UtlDList rlmi_body_parameters;
   rlmi_body_parameters.append(new NameValuePair(HTTP_CONTENT_ID_FIELD,
                                                 rlmiBodyPartCid));

   // Attach the RLMI.
   body->appendBodyPart(rlmi_body, rlmi_body_parameters);

   // Clean up the parameter list.
   rlmi_body_parameters.destroyAll();

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
   // If the name XML is not empty and does not end with LF, add CR-LF.
   if (!mNameXml.isNull() && mNameXml(mNameXml.length() - 1) != '\n')
   {
      mNameXml += "\r\n";
   }
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource:: this = %p, resourceList = %p, mUri = '%s', mNameXml = '%s'",
                 this, mResourceList, mUri.data(), mNameXml.data());
}


// Destructor
ResourceListResource::~ResourceListResource()
{
   mInstances.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Add an instance to the resource list and publish appropriate notices.
void ResourceListResource::addInstancePublish(const char* instanceName,
                                              const char* subscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource::addInstancePublish instanceName = '%s', subscriptionState = '%s'",
                 instanceName, subscriptionState);
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
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource::addInstance instanceName = '%s', subscriptionState = '%s'",
                 instanceName, subscriptionState);
   ResourceListResourceInstance* inst =
      new ResourceListResourceInstance(instanceName, subscriptionState);
   mInstances.append(inst);
}

// Delete an instance from the resource list and publish appropriate notices.
void ResourceListResource::deleteInstancePublish(const char* instanceName,
                                                 const char* subscriptionState,
                                                 const char* resourceSubscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource::deleteInstancePublish instanceName = '%s', subscriptionState = '%s'",
                 instanceName, subscriptionState);
}

// A notice has arrived for an instance.  Publish it.
void ResourceListResource::updateInstancePublish(const char* instanceName,
                                                 const char* bytes,
                                                 int length)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource::updateInstancePublish instanceName = '%s', bytes = '%s'",
                 instanceName, bytes);
   ResourceList* resourceList = getResourceList();
   SipPublishContentMgr* eventPublisher = resourceList->getEventPublisher();
   const char* eventType = resourceList->getEventType();
   const char* resourceUri = resourceList->getResourceUri();
   ResourceListResourceInstance* inst = findInstance(instanceName);
   if (!inst)
   {
      // Add the instance to the resource.
      addInstance(instanceName, "active");
      inst = findInstance(instanceName);
   }

   // Set the content.
   inst->mContent.remove(0);
   inst->mContent.append(bytes, length);
   inst->mContentPresent = TRUE;
   inst->mSubscriptionState = "active";

   // Generate the notice body.
   HttpBody* body = resourceList->generateBody();
   // Publish the notice.
   eventPublisher->publish(resourceUri, eventType, eventType, 1, &body, TRUE);

   // Leave the content, because all active istances are supposed to
   // have content.
}

// Find the contained instance with a particular name.
ResourceListResourceInstance*
ResourceListResource::findInstance(const char* instanceName)
{
   UtlSListIterator itor(mInstances);
   ResourceListResourceInstance* inst;
   while ((inst = dynamic_cast <ResourceListResourceInstance*> (itor())))
   {
      if (inst->mInstanceName.compareTo(instanceName) == 0)
      {
         return inst;
      }
   }
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListResource::findInstance not found: instanceName = '%s'",
                 instanceName);
   return NULL;
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
   mSubscriptionState(subscriptionState),
   mContentPresent(FALSE)
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
