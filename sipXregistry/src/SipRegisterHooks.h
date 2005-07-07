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

#ifndef _SIPREGISTERHOOKS_H_
#define _SIPREGISTERHOOKS_H_


// SYSTEM INCLUDES

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

/**
 * SIP Register Hooks Manager.
 *
 * Maintains the list of RegisterHooks and invokes them as needed.
 *
 * A hook must be built as a dynamic library that provides at least:
 * - A class derived from RegisterHook
 * - An @code extern "C" @endcode factory routine to obtain an instance of that class.
 *
 * To configure a hook into the registrar, add a configuration entry to the
 * registrar-config file like:
 * @code
 * SIP_REGISTRAR_HOOK_LIBRARY.EXAMPLE : /path/to/libexamplereghook.so
 * @endcode
 *
 * The token EXAMPLE above may be any word; it serves to identify the instance
 * of the hook.  Any configuration entries for the hook are made with entries like:
 * @code
 * SIP_REGISTRAR_HOOK.EXAMPLE.FOO : foovalue
 * SIP_REGISTRAR_HOOK.EXAMPLE.BAR : barvalue
 * @endcode
 * The RegisterHook::readConfig method is passed an OsConfigDb subhash containing
 * just its own configuation entries with the SIP_REGISTRAR_HOOK.EXAMPLE. removed.
 */
class SipRegisterHooks
{
public:
   SipRegisterHooks();

   ~SipRegisterHooks();

   /// Read what hooks are configured, and instantiate and configure each hook.
   void readConfig( OsConfigDb& configDb );

   /// Execute the RegisterHook::takeAction method of each configured hook.
   void doHooks( const SipMessage&   registerMessage
                ,const unsigned int  registrationDuration
                ,SipUserAgent*       sipUserAgent
                );

private:

   UtlSList mConfiguredHooks;
   
};

#endif // _SIPREGISTERHOOKS_H_

