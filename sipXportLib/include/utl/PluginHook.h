// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _PLUGINHOOK_H_
#define _PLUGINHOOK_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
class OsConfigDb;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PluginHook;


/**
 * A PluginHook is a dynamically loaded object that is invoked by some component at some
 *   well defined time (it's an abstract class - how specific can the
 *   description be?).
 *
 * This class is the abstract base from which all hooks must inherit; it
 * decouples the configuration of what hooks should be invoked and the
 * configuration parameters specific to each hook from the program that
 * uses them.
 *
 * All PluginHook classes must implement two methods to configure the hook
 * into the component:
 * - The extern "C" getHook factory routine
 * - The readConfig method used to pass configuration data to the hook (this decouples hook
 *   configuration from the component configuration).
 *
 * Each class derived from PluginHook should also define the method(s) that
 * the program should invoke on the hook, and all those methods must be virtual.
 *
 * @see PluginHooks for details of how a hook is configured into a program,
 * and PluginIterator for how hooks are invoked.
 *
 */
class PluginHook
{
  public:

   /// Name of the factory method to be called when the libary is loaded (must be "getHook")
   static const char* FactoryMethodName;   

   typedef PluginHook* (*Factory)(const UtlString& hookName);
   /**<
    * The getHook uses external C linkage to support dynamic loading of PluginHook objects.
    *
    * In addition to the class derived from this base, a hook must implement a
    * factory routine with extern "C" linkage so that the OsSharedLib mechanism
    * can look it up in the dynamically loaded library (looking up C++ symbols
    * is problematic because of name mangling).  The factory routine looks like:
    * @code
    * class ExampleHook;
    *
    * extern "C" ExampleHook* getHook(const UtlString& name)
    * {
    *   return new ExampleHook;
    * }
    *
    * class ExampleHook : public PluginHook
    * {
    *    friend ExampleHook* getHook(const UtlString& name);
    *   ...
    * private:
    *    ExampleHook(const UtlString& name);
    * }
    * @endcode
    */

   /// The hook destructor must be virtual.
   virtual ~PluginHook()
      {
      };

   /// Read (or re-read) whatever configuration the hook requires.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this hook. */
                           ) = 0;
   /**<
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The hook
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the hook (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */

  protected:

   /// Derived constructors should be private so that only the getHook can call them.
   PluginHook(const UtlString& instanceName) :
      mInstanceName(instanceName)
      {
      };

   /// The instance name from the configuration directive - for logging and other identification.
   UtlString   mInstanceName;
    
  private:

   /// There is no copy constructor.
   PluginHook(const PluginHook&);

   /// There is no assignment operator.
   PluginHook& operator=(const PluginHook&);
    
};

#endif // _PLUGINHOOK_H_
