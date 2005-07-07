// 
// Copyright (C) 2005 Pingtel Corp.
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

#include "utl/PluginHook.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

const char* PluginHook::FactoryMethodName = "getHook";

// CONSTANTS
#define HOOK_LIB_PREFIX "_HOOK_LIBRARY."

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
// GLOBAL VARIABLES

/// ConfiguredHook is the container used to hold each PluginHook.
/**
 * ConfiguredHook inherits from UtlString so that it will be a
 * UtlContainable and will be identifiable by its configured name.
 */
class ConfiguredHook : public UtlString
{
public:

   // load the library for a hook and use its factory to get a new instance.
   ConfiguredHook(const UtlString& hookName, const UtlString& libName )
      : UtlString(hookName),
        hook(NULL)
      {
         OsSharedLibMgrBase* sharedLibMgr = OsSharedLibMgr::getOsSharedLibMgr();

         if (sharedLibMgr)
         {
            PluginHook::Factory factory;
            
            if (OS_SUCCESS == sharedLibMgr->getSharedLibSymbol(libName.data(),
                                                               PluginHook::FactoryMethodName,
                                                               (void*&)factory
                                                               )
                )
            {
               // Use the factory to get an instance of the hook
               // and tell the new instance its own name.
               hook = factory(hookName); 

               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "PluginHooks created '%s' from '%s'",
                             data(), libName.data()
                             );
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_ERR,
                             "PluginHooks: factory '%s' not found in library '%s' for '%s'",
                             PluginHook::FactoryMethodName, libName.data(), data()
                             );

            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                          "PluginHooks: failed to getOsSharedLibMgr"
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
   
   /// Get the name of this hook.
   void name(UtlString* hookName) const
      {
         if (hookName)
         {
            hookName->remove(0);
            hookName->append(*this);
         }
      }

   /// Get the actual hook object.
   PluginHook* plugin() const
      {
         return hook;
      }

   /// Construct the subhash for the hook and configure it.
   void readConfig(const UtlString& prefix, OsConfigDb& configDb)
      {
         if (hook)
         {
            OsConfigDb myConfig;
            UtlString myConfigName;

            // build up "<prefix>.<instance>." key for configuration subhash
            myConfigName.append(prefix);
            myConfigName.append('.');
            myConfigName.append(*this);
            myConfigName.append('.');
            
         
            if (OS_SUCCESS == configDb.getSubHash(myConfigName, myConfig))
            {
               OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                             "PluginHooks configuring '%s' from '%s'",
                             data(), myConfigName.data()
                             );
               hook->readConfig(myConfig);
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                             "PluginHooks no configuration found for '%s'",
                             data()
                             );
            }
         }
      };

private:

   PluginHook* hook; ///< the actual hook instance
};


PluginHooks::PluginHooks(const UtlString& hookPrefix)
   : mPrefix(hookPrefix)
{
}

PluginHooks::~PluginHooks()
{
   mConfiguredHooks.destroyAll();
}

void PluginHooks::readConfig(OsConfigDb& configDb)
{
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "PluginHooks::readConfig" );

   // Move any existing hooks from the current configured list to
   // a temporary holding list.
   UtlSList existingHooks;
   UtlContainable* existingHook;
   
   UtlSortedListIterator nextHook(mConfiguredHooks);
   while (existingHook = nextHook())
   {
      existingHooks.append(mConfiguredHooks.removeReference(existingHook));
   }
   // the mConfiguredHooks list is now empty
   
   // Walk the current configuration,
   //   any existing hook is moved back to the mConfiguredHooks list,
   //   newly configured hooks are added,
   //   each configured hook is called to read its own configuration.
   UtlString  hookPrefix(mPrefix);
   hookPrefix.append(HOOK_LIB_PREFIX);
   
   OsConfigDb allHooks;
   
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "PluginHooks::readConfig looking up hooks '%s'",
                 hookPrefix.data()
                 );
   if (OS_SUCCESS == configDb.getSubHash(hookPrefix, allHooks)) // any hooks configured for prefix?
   {
      UtlString lastHook;
      UtlString hookName;
      UtlString hookLibrary;

      // walk each hook and attempt to load and configure it 
      for ( lastHook = "";
            OS_SUCCESS == allHooks.getNext(lastHook, hookName, hookLibrary);
            lastHook = hookName
           )
      {
         ConfiguredHook* thisHook;
         
         if (NULL == (thisHook = dynamic_cast<ConfiguredHook*>(existingHooks.remove(&hookName))))
         {
            // not an existing hook, so create a new one
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                          "PluginHooks: loading '%s'", hookName.data()
                          );
            thisHook = new ConfiguredHook(hookName, hookLibrary);
         }

         // put the hook onto the list of active hooks
         mConfiguredHooks.insert(thisHook);

         // (re)configure the hook
         thisHook->readConfig(mPrefix, configDb);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_INFO,
                    "PluginHooks: no '%s' hooks configured", mPrefix.data()
                    );
   }

   // discard any hooks that are no longer in the configuration
   existingHooks.destroyAll();
}

PluginIterator::PluginIterator( const PluginHooks& pluginHooks ) :
   mConfiguredHooksIterator(pluginHooks.mConfiguredHooks)
{
}

PluginHook* PluginIterator::next(UtlString* name)
{
   PluginHook* nextPlugin = NULL;
   
   // make sure that name is cleared if passed in case this is the last hook
   if (name)
   {
      name->remove(0);
   }
   
   // step the parent iterator on the mConfiguredHooks list
   ConfiguredHook* nextHook = static_cast<ConfiguredHook*>(mConfiguredHooksIterator());
   if (nextHook)
   {
      nextHook->name(name); // return the name, if it's been asked for
      nextPlugin = nextHook->plugin();
   }

   return nextPlugin;
}

PluginIterator::~PluginIterator()
{
}
