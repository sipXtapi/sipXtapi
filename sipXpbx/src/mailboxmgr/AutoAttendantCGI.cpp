// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/AutoAttendantCGI.h"
#include "mailboxmgr/MailboxManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

AutoAttendantCGI::AutoAttendantCGI( const Url& from, const UtlString& name, const char* digits ) :
   m_from ( from ),
   m_name ( name ),
   m_digits ( digits )
{
   m_name.toLower();
}

AutoAttendantCGI::~AutoAttendantCGI()
{}

OsStatus
AutoAttendantCGI::execute(UtlString* out)
{
   // Get the base URL of mediaserver. This is necessary for playing prompts.
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   UtlString ivrPromptUrl;
   pMailboxManager->getIvrPromptURL( ivrPromptUrl );

   UtlString mediaserverUrl;
   MailboxManager::getInstance()-> getMediaserverURL( mediaserverUrl );

   UtlString secureMediaserverUrl;
   pMailboxManager->getMediaserverSecureURL( secureMediaserverUrl );

   // call helper method to convert string to friendly form suitable for
   // transfer to the OpenVXI engine
   UtlString vxmlFriendlyFrom = m_from.toString();
   MailboxManager::convertUrlStringToXML ( vxmlFriendlyFrom );

   // Default URL of the system-wide greeting.
   // This is played when a caller dials into the auto attendant.
   UtlString systemgreeting = ivrPromptUrl + "/stdprompts/welcome.wav";

   // Default URL of the auto attendant prompt.
   // This is an optional prompt that is played after the system-wide greeting.
   UtlString autoattendantprompt = ivrPromptUrl + "/stdprompts/autoattendant-internal.wav";

   // Parse the organization prefs XML file to get 
   // the active system-wide greeting and the auto attendant prompt.
   UtlHashMap *orgPrefsHashDict = new UtlHashMap();
   pMailboxManager->parseOrganizationPrefsFile( orgPrefsHashDict );

   if( orgPrefsHashDict->entries() > 0 )
   {
      UtlString* rwSystemGreeting = (UtlString*) orgPrefsHashDict->
         findValue( new UtlString("systemgreeting") );
      UtlString* rwAutoattendantPrompt = (UtlString*) orgPrefsHashDict->
         findValue( new UtlString("autoattendant") );

      UtlString systemGreetingType = rwSystemGreeting->data();
      UtlString autoattendantPromptType = rwAutoattendantPrompt->data();

      UtlString greetingUrl;
      // Get the URL of the active system-wide greeting
      if( pMailboxManager->getSystemPromptUrl(systemGreetingType, greetingUrl, FALSE ) == OS_SUCCESS )
         systemgreeting = greetingUrl;

      // If the administrator has disabled playing auto attendant prompt, set to -1.
      // Else, get the URL of the active autoattendant prompt
      if( autoattendantPromptType == DISABLE_AUTOATTENDANT_PROMPT )
         autoattendantprompt = "-1";
      else if( pMailboxManager->getSystemPromptUrl(autoattendantPromptType, greetingUrl, FALSE ) == OS_SUCCESS )
      {
         if (!m_name.isNull() && m_name.compareTo("internal") != 0)
         {
            // Modify the prompt to use the application specific greeting
            int length = greetingUrl.index("internal.wav");
            UtlString baseUrl = greetingUrl(0, length);
            UtlString modUrl = baseUrl + m_name + ".wav";
            autoattendantprompt = modUrl;
         }
         else
         {
            autoattendantprompt = greetingUrl;
         }
      }
   }


   // Construct the dynamic VXML
   UtlString dynamicVxml(VXML_BODY_BEGIN);
   dynamicVxml +=  "<form>\n";
   if (m_digits)
   {
      UtlString urlSeparator ( URL_SEPARATOR );

      dynamicVxml += "<block> <prompt><audio src=\"" + 
         ivrPromptUrl + 
         urlSeparator +
         UtlString (PROMPT_ALIAS) + 
         urlSeparator + "extension.wav\"/></prompt>";
      dynamicVxml += "<prompt><say-as type=\"acronym\">" + UtlString(m_digits) + "</say-as></prompt>";
      dynamicVxml += "<prompt><audio src=\"" + 
         ivrPromptUrl + 
         urlSeparator +
         UtlString (PROMPT_ALIAS) + 
         urlSeparator + "is_invalid.wav\"/></prompt>"\
         "</block>";
   }
   if (m_name.isNull())
   {
      dynamicVxml += "<subdialog name=\"autoattendant\" src=\"" + mediaserverUrl + "/aa_vxml/autoattendant-internal.vxml\">\n";
   }
   else
   {
      dynamicVxml += "<subdialog name=\"autoattendant\" src=\"" + mediaserverUrl + "/aa_vxml/autoattendant" + "-" + m_name + ".vxml\">\n";
   }
   dynamicVxml += "<param name=\"from\" value=\"" + vxmlFriendlyFrom + "\"/>\n" \
      "<param name=\"mediaserverurl\" expr=\"'" + ivrPromptUrl + "'\"/>\n" \
      "<param name=\"securemediaserverurl\" expr=\"'" + secureMediaserverUrl + "'\"/>\n" \
      "<param name=\"systemgreetingurl\" expr=\"'" + systemgreeting + "'\"/>\n" \
      "<param name=\"autoattendantprompturl\" expr=\"'" + autoattendantprompt + "'\"/>\n" \
      "</subdialog>\n" \
      "</form>\n";
   dynamicVxml += VXML_END;

   // Write out the dynamic VXML script to be processed by OpenVXI
   if (out) 
   {
      out->remove(0);
      UtlString responseHeaders;
      MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

      out->append(responseHeaders.data());
      out->append(dynamicVxml.data());
   }
   return OS_SUCCESS;
}


