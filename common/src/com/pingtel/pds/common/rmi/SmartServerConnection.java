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


package com.pingtel.pds.common.rmi;

import org.apache.log4j.Category;
import org.apache.log4j.Priority;

import java.lang.reflect.Method;
import java.rmi.Naming;
import java.rmi.Remote;

/**
 * SmartServerConnection is a class that essentially manages any java.rmi.Remote
 * subclass (any RMI server service).   It will perform an application level
 * 'ping' on a given heathcheck method to see that service is reachable by clients.
 *
 * It solves the following problem:
 *
 * 1. start rmiregistry
 * 2. start server
 * 3. start client a - executes successfully
 * 4. rmiregistry terminates/is terminated
 * 5. rmiregistry restarted
 * 6. start client b - ERROR server not bound in rmiregistry
 */
public class SmartServerConnection implements Runnable {

    // How long the thread should sleep when it believes the the RMI
    // service is correctly bound.
    private static final int HEATHCHECK_PING_INTERVAL = 10000;

    // How long the thread should sleep when it knows that the RMI
    // service is unreachable.
    private static final int BROKEN_HEALTHCHECK_PING_INTERVAL = 1000;

    private String m_registryServerURL;
    private String m_interfaceBindName;
    private Remote m_interfaceImpl;
    private String m_healthCheckMethod;
    private Class [] m_healthCheckMethodParams;
    private Object [] m_healthCheckMethodArgs;
    private Category m_logger;


    /**
     *
     * @param registryServerURL the RMI URL of the machine that you want to
     * bind the interace to.
     * @param interfaceBindName the name of the service that you want to bind
     * the interface to.
     * @param interfaceImpl the RMI service/interface that you are managing.
     * @param healthCheckMethod the name of the method that you want to be
     * 'pinged'/called to make sure that the service/interace is bound.
     * @param healthCheckMethodParams Class array of the parameters of the
     * health check method.   Optional, if the healthCheckMethod does not
     * take parameters then you can supply null.
     * @param healthCheckMethodArgs Object array of arguments that you want
     * to use in call to the health check method.   Optional, if the
     * healthCheckMethod does not take parameters then you can supply null.
     * @param logger optionaly Log4J logger, if not supplied then errors
     * are send to stdout.
     */
    public SmartServerConnection (  String registryServerURL,
                                    String interfaceBindName,
                                    Remote interfaceImpl,
                                    String healthCheckMethod,
                                    Class [] healthCheckMethodParams,
                                    Object [] healthCheckMethodArgs,
                                    Category logger ) {

        m_registryServerURL = registryServerURL;
        m_interfaceBindName = interfaceBindName;
        m_interfaceImpl = interfaceImpl;
        m_healthCheckMethod = healthCheckMethod;
        m_healthCheckMethodParams = healthCheckMethodParams;
        m_healthCheckMethodArgs = healthCheckMethodArgs;
        m_logger = logger;

        ensureRebind();
    }

    /**
     * implements Runnable.run() method.    Sits in a busy loop checking periodically
     * to see if the RMI service is avaialable.
     */
    public void run() {

        while ( true ) {
            try {
                Thread.sleep( HEATHCHECK_PING_INTERVAL );
                if ( !isInterfacePingable() )
                    ensureRebind();
            }
            catch (InterruptedException e) { }
        }
    }


    //////////////////////////////////////////////////////////////////////////////
    //
    // isInterfacePingable is the 'brain's of the outfit.   If tries to locate
    // the remote service and uses relflection to create the heathcheck method
    // and then executes it.
    //
    // returns a true if it can sucessfully execute the method.
    //
    //////////////////////////////////////////////////////////////////////////////
    private boolean isInterfacePingable () {

        boolean isHealthy = true;
        try {
            if ( m_healthCheckMethod != null ) {
                Remote r = Naming.lookup( m_registryServerURL + "/" + m_interfaceBindName );
                Method m =
                        r.getClass().getMethod( m_healthCheckMethod, m_healthCheckMethodParams );

                m.invoke( r, m_healthCheckMethodArgs );
            }
        }
        catch ( Exception e) {
            logMessage ( Priority.ERROR, "Can not ping method: " + m_healthCheckMethod +
                    " for interface: " + m_interfaceBindName +
                    " on server: " + m_registryServerURL );
            isHealthy = false;
        }

        return isHealthy;
    }


    //////////////////////////////////////////////////////////////////////////////
    //
    // ensureRebind is used to check bind the service in the RMIRegistry.   If it
    // can do this right off the bat it keep trying to do it.
    //
    //////////////////////////////////////////////////////////////////////////////
    private void ensureRebind () {
        rebind();

        while ( !isInterfacePingable() ) {
            rebind();

            try {
                Thread.sleep( BROKEN_HEALTHCHECK_PING_INTERVAL );
            } catch (InterruptedException e) { }

        }
    }


    private void rebind() {
        try {
            Naming.rebind( m_registryServerURL + "/" + m_interfaceBindName,
                    m_interfaceImpl);
        }
        catch ( Exception e) {
            logMessage ( Priority.ERROR, "Can not bind: " +
                    m_registryServerURL + "/" + m_interfaceBindName );
        }
    }


    private void logMessage ( Priority priority, String message ) {
        if ( m_logger != null ) {
            m_logger.log( priority, message );
        }
        else {
            System.out.println( message );
        }
    }

    private void logMessage ( Priority priority, String message, Throwable t ) {
        if ( m_logger != null ) {
            m_logger.log( priority, message, t );
        } else {
            System.out.println( message );
        }
    }

}
