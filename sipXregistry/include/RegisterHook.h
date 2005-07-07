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
class OsConfigDb;
class SipMessage;
class SipUserAgent;

// DEFINES
// These MUST be the same!!
#define REGISTER_HOOK_FACTORY_METHOD getHook
#define REGISTER_HOOK_FACTORY_METHOD_NAME "getHook"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

/**
 * SIP Registrar Hook Action.
 *
 * A RegisterHook is an action invoked by the SipRegistrarServer whenever a
 *   successful REGISTER request has been processed (that is, after it has
 *   effected its change on the registry database).  The hook may then take
 *   any action based on the fact that the registration has occured.
 *
 * This class is the abstract base from which all hooks must inherit.
 *
 * @see SipRegisterHooks for details of how to configure a hook into the registrar.
 *
 * In addition to the class derived from this base, a hook must implement a
 * factory routine with extern "C" linkage so that the OsSharedLib mechanism
 * can look it up in the dynamically loaded library (looking up C++ symbols
 * is problematic because of name mangling).  The factory routine looks like:
 * @code
 * class ExampleHook : public RegisterHook
 * {
 *   ...
 * }
 *
 * extern "C" RegisterHook* REGISTER_HOOK_FACTORY_METHOD()
 * {
 *   return new ExampleHook;
 * }
 * @endcode
 */
class RegisterHook
{
public:

   /// Factory method that returns the hook object.
   static RegisterHook* REGISTER_HOOK_FACTORY_METHOD();
   /**<
    * The getHook is static to support dynamic loading of RegisterHook objects.
    */
    
   virtual ~RegisterHook()
      {
      };

    /// Read (or re-read) whatever configuration the hook requires.
    virtual void readConfig( OsConfigDb& configDb ) = 0;
    /**<
     * @note
     * The SipRegistrarServer may call the readConfig method at any time to
     * indicate that the registrar configuration may have changed.  The hook
     * should reinitialize itself based on the configuration that exists when
     * this is called.
     * @endnote
     *
     * @param configDb contains a subhash of the individual configuration
     * parameters for this instance of this hook.
     */
    
    /// Take whatever action this hook exists to perform.
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
     * them in the registrar-config file.
     */

  protected:

    /// Constructor is private so that it is only callable from getHook.
    RegisterHook()
       {
       };
    
  private:

    /// There is no copy constructor.
    RegisterHook(const RegisterHook&);

    /// There is no assignment operator.
    RegisterHook& operator=(const RegisterHook&);
    
};

/// The protoype for a pointer to the factory routine for a hook.
typedef RegisterHook* (*RegisterHookFactoryMethod)();

#endif // _REGISTERHOOK_H_
