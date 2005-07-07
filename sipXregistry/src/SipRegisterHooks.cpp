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

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsSharedLibMgr.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "net/SipMessage.h"

#include "RegisterHook.h"
#include "SipRegisterHooks.h"

class SipUserAgent;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define REGISTER_HOOK_PREFIX     "SIP_REGISTRAR_HOOK."
#define REGISTER_HOOK_LIB_PREFIX "SIP_REGISTRAR_HOOK_LIBRARY."

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
// GLOBAL VARIABLES

// ConfiguredHook inherits from UtlString so that it will be a
// UtlContainable and will be identifiable by its configured name.
class ConfiguredHook : public UtlString
{
public:

   ConfiguredHook(const UtlString& hookName, const UtlString& libName )
      : UtlString(hookName),
        hook(NULL)
      {
         OsSharedLibMgrBase* sharedLibMgr = OsSharedLibMgr::getOsSharedLibMgr();

         if (sharedLibMgr)
         {
            RegisterHookFactoryMethod factory;
            UtlString factoryMethodName(REGISTER_HOOK_FACTORY_METHOD_NAME);
            
            if (OS_SUCCESS == sharedLibMgr->getSharedLibSymbol(libName.data(),
                                                               factoryMethodName.data(),
                                                               (void*&)factory
                                                               )
                )
            {
               hook = factory();

               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "SipRegisterHooks '" REGISTER_HOOK_FACTORY_METHOD_NAME
                             "' loaded '%s' %p from '%s'",
                             data(), hook, libName.data()
                             );
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_ERR,
                             "SipRegisterHooks: method '" REGISTER_HOOK_FACTORY_METHOD_NAME
                             "' not found in library '%s' for '%s'",
                             libName.data(), data()
                             );

            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                          "SipRegisterHooks: failed to getOsSharedLibMgr"
                          );
         }
      }

   ~ConfiguredHook()
      {
         // don't try to unload any libraries
         // it's not worth the complexity of reference counting it
         // the same library could be configured more than once with
         // different hook names and parameters.
      }
   
   void readConfig(OsConfigDb& configDb)
      {
         if (hook)
         {
            OsConfigDb myConfig;
            UtlString myConfigName(REGISTER_HOOK_PREFIX);
            myConfigName.append(*this);
            myConfigName.append('.');
         
            if (OS_SUCCESS == configDb.getSubHash(myConfigName, myConfig))
            {
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "SipRegisterHooks configuring '%s' from '%s'",
                             data(), myConfigName.data()
                             );
               hook->readConfig(myConfig);
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                             "SipRegisterHooks no configuration found for '%s'",
                             data()
                             );
            }
         }
      };

   void takeAction(const SipMessage&   registerMessage,
                   const unsigned int  registrationDuration,
                   SipUserAgent*       sipUserAgent
                   )
      {
         if (hook)
         {
            hook->takeAction(registerMessage, registrationDuration, sipUserAgent);
         }
      };

private:

   RegisterHook* hook;
};


SipRegisterHooks::SipRegisterHooks()
{
}

SipRegisterHooks::~SipRegisterHooks()
{
   mConfiguredHooks.destroyAll();
}

void SipRegisterHooks::readConfig(OsConfigDb& configDb)
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "SipRegisterHooks::readConfig" );

   // Move any existing hooks from the current configured list to
   // a temporary holding list.
   UtlSList existingHooks;
   UtlContainable* existingHook;
   
   UtlSListIterator nextHook(mConfiguredHooks);
   while (existingHook = nextHook())
   {
      existingHooks.append(mConfiguredHooks.removeReference(existingHook));
   }

   // Walk the current configuration,
   //   any existing hook is moved back to the mConfiguredHooks list,
   //   newly configured hooks are added,
   //   each configured hook is called to read its own configuration.
   UtlString  hookPrefix(REGISTER_HOOK_LIB_PREFIX);
   OsConfigDb allHooks;
   
   if (OS_SUCCESS == configDb.getSubHash(hookPrefix, allHooks))
   {
      UtlString lastHook;
      UtlString hookName;
      UtlString hookLibrary;
      
      for ( lastHook = "";
            OS_SUCCESS == allHooks.getNext(lastHook, hookName, hookLibrary);
            lastHook = hookName
           )
      {
         ConfiguredHook* thisHook;
         
         if (NULL == (thisHook = dynamic_cast<ConfiguredHook*>(existingHooks.remove(&hookName))))
         {
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                          "SipRegisterHooks: loading '%s'", hookName.data()
                          );
            thisHook = new ConfiguredHook(hookName, hookLibrary);
         }

         mConfiguredHooks.append(thisHook);
         
         thisHook->readConfig(configDb);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_INFO,
                    "SipRegisterHooks: no hooks configured"
                    );
   }

   // discard any hooks that are no longer in the configuration
   existingHooks.destroyAll();
}


void SipRegisterHooks::doHooks(const SipMessage&   registerMessage,
                               const unsigned int  registrationDuration,
                               SipUserAgent*       sipUserAgent
                               )
{
   UtlSListIterator nextHook(mConfiguredHooks);
   ConfiguredHook* hook;

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "SipRegisterHooks:doHooks" ); 

   while (hook = dynamic_cast<ConfiguredHook*>(nextHook()))
   {
      hook->takeAction(registerMessage, registrationDuration, sipUserAgent);
   }
}

