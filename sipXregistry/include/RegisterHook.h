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

#ifndef _REGISTERHOOK_H_
#define _REGISTERHOOK_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/PluginHook.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class SipMessage;
class SipUserAgent;

/**
 * SIP Registrar Hook Action.
 *
 * A RegisterHook is an action invoked by the SipRegistrarServer whenever a
 *   successful REGISTER request has been processed (that is, after it has
 *   effected its change on the registry database).  The hook may then take
 *   any action based on the fact that the registration has occured.
 *
 * This class is the abstract base from which all RegisterHooks must inherit.
 *
 * To configure a RegisterHook into the SipRegistrarServer, the registrar-config
 * file should have a directive configuring the plugin library:
 * @code
 * SIP_REGISTRAR_HOOK_LIBRARY.[instance] : [path to libexamplereghook.so]
 * @endcode
 * Where [instance] is replaced by a unique hook name, and the value
 * points to the libary that provides the hook code.
 *
 * In addition to the class derived from this base, a RegisterHook library must
 * provide a factory routine named getRegisterHook with extern "C" linkage so
 * that the OsSharedLib mechanism can look it up in the dynamically loaded library
 * (looking up C++ symbols is problematic because of name mangling).
 * The factory routine looks like:
 * @code
 * class ExampleRegisterHook : public RegisterHook
 * {
 *    virtual void takeAction( const SipMessage&   registerMessage ///< the successful registration
 *                            ,const unsigned int  registrationDuration ///< the actual allowed
 *                                                                      /// registration time (note
 *                                                                      /// that this may be < the
 *                                                                      /// requested time).
 *                            ,SipUserAgent*       sipUserAgent     ///< to be used if the hook
 *                                                                  /// wants to send any SIP msg
 *                            );
 *
 *    friend RegisterHook* getRegisterHook(const UtlString& name);
 * }
 *
 * extern "C" RegisterHook* getRegisterHook(const UtlString& instance)
 * {
 *   return new ExampleRegisterHook(instance);
 * }
 * @endcode
 *
 * @see PluginHook
 */
class RegisterHook : public PluginHook
{
public:

    /// Take whatever action this register hook exists to perform.
    virtual void takeAction( const SipMessage&   registerMessage  ///< the successful registration
                            ,const unsigned int  registrationDuration /**< the actual allowed
                                                                       * registration time (note
                                                                       * that this may be < the
                                                                       * requested time). */
                            ,SipUserAgent*       sipUserAgent     /**< to be used if the hook
                                                                   *   wants to send any SIP msg */
                            ) = 0;
    /**<
     * A hook may not actually affect the registration itself; it may only take
     * other actions (such as updating presence information) that are triggered
     * by the registration.
     *
     * Hooks are invoked in lexically sorted order based on the name used to identify
     * them in the registrar-config file, but hook authors are strongly discouraged
     * from making any assumptions based on this ordering.
     */


    static const char* Prefix;  ///< the configuration file prefix = "SIP_REGISTRAR"
    static const char* Factory; ///< the factory routine name = "getRegisterHook"

  protected:

    /// Constructor is private so that it is only callable from getHook.
    RegisterHook(UtlString& instanceName) :
       PluginHook(instanceName)
       {
       };
    
    virtual ~RegisterHook()
       {
       };

  private:

    /// There is no copy constructor.
    RegisterHook(const RegisterHook&);

    /// There is no assignment operator.
    RegisterHook& operator=(const RegisterHook&);
    
};

#endif // _REGISTERHOOK_H_
