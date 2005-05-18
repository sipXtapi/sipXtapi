/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.common;

import java.util.Hashtable;

import javax.ejb.EJBHome;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.rmi.PortableRemoteObject;

/**
 * EJBHomeFactory is used to isolate clients of EJ Beans from the
 * annoyance and complexity of looking up EJBHome interfaces.   As an
 * added bonus is improves performance.   It is a type of
 * 'ServiceLocator' pattern object.
 *
 * Ensure that the appropriate init() method is called before you
 * try to use it.   This is necessary so as to make the class usable
 * by clients in the same and different JVMs.
 *
 * @author ibutcher
 */
public class EJBHomeFactory {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static EJBHomeFactory mSingletonInstance = new EJBHomeFactory ();

    private Hashtable mInterfaceCache;
    private Context mContext;

//////////////////////////////////////////////////////////////////////////
// Construction
////
    private EJBHomeFactory() {
        mInterfaceCache = new Hashtable();
    }

//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * getInstance is the accessor to the Singleton instance.
     *
     * @return instance of the EJBHomeFactory.
     */
    public static EJBHomeFactory getInstance() {
        return mSingletonInstance;
    }

    /**
     * init must be called before the EJBHomeFactory can be used.   This version
     * of init sets up the InitialContext specifying no properties.   This is fine
     * when it is used in the same JVM as the EJBContainer.   If you need to use
     * EjBHomeFactory in a separate JVM then use the init method which takes
     * and environnment.
     *
     * @throws NamingException is thrown when the EJBHomeFactory is unable to
     * establish an InitialContext with the EJBContainer.
     * @see #init(Hashtable environment)
     *
     */
    public void init () throws NamingException {
        mContext = new InitialContext();
    }

    /**
     * init must be called before the EJBHomeFactory can be used.   This version
     * of init sets up the InitialContext with JNDI properties.   This should be
     * used when the EJBHomeFactory is in a different JVM to the  EJBContainer.
     *
     * @param environment
     * @throws NamingException is thrown when the EJBHomeFactory is unable to
     * establish an InitialContext with the EJBContainer.
     * @see #init()
     *
     */
    public void init (Hashtable environment) throws NamingException {
        mContext = new InitialContext (environment);
    }


    /**
     * getHomeInterface returns EJBHome interface objects.
     *
     * @param clazz pass in the Class object which represents the
     * home interface you want from the factory.  Example UserHome
     *
     * @param JNDIName the name which is used to bind the home i/f
     * in the JNDI server.  Example "User".
     *
     * @return the EJBHome object.   Note you have to cast this to the required
     * subclass.
     *
     */
    public EJBHome getHomeInterface (Class clazz, String JNDIName) throws NamingException {

        if (mContext == null) {
            throw new RuntimeException ("EJBHomeFactory not initialized; Call init()");
        }

        EJBHome home = null;
        home = (EJBHome) mInterfaceCache.get(clazz);

        if ( home == null ) {
            Object o = mContext.lookup(JNDIName);
            home = (EJBHome) PortableRemoteObject.narrow(o, clazz);
            mInterfaceCache.put(clazz, home);
        }

        return home;
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
