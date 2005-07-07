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

package com.pingtel.pds.pgs.common.ejb;

import com.pingtel.pds.common.PDSException;

import javax.ejb.EJBHome;
import java.lang.reflect.Method;

/**
 * <description of class including its purpose>
 * @author ibutcher
 *
 */
public class InternalToExternalIDTranslator  {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private static InternalToExternalIDTranslator mInstance;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////

    private InternalToExternalIDTranslator() { }

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * singleton instance accessor method
     * @return singleton instance
     */
    public static InternalToExternalIDTranslator getInstance() {
        if ( mInstance == null )
            mInstance = new InternalToExternalIDTranslator ();

        return mInstance;
    }

    /**
     * translate takes an EJBHome interface for entity beans along with the
     * primary key of an instance for an entity bean and returns a user-
     * friendly name/description.   This is useful for reporting errors or
     * logging.
     *
     * @param homeInterface EJBHome interface for the particular entity bean
     * @param id PK of the entity you wish to translate
     * @return user-friendly name for the entity
     * @throws PDSException is thrown for application errors
     */
    public String translate ( EJBHome homeInterface, Object id ) throws PDSException {

        String returnValue = null;
        Object bean = null;

        try {
            Method [] methods = homeInterface.getClass().getMethods();

            for ( int counter = 0; counter < methods.length; ++counter ) {
                if ( methods [ counter ].getName().equals( "findByPrimaryKey" ) ) {
                    bean = methods [ counter ].invoke( homeInterface, new Object [] {id} );
                    break;
                }
            }

            Method getExternalID = bean.getClass().getMethod( "getExternalID", null );
            returnValue = (String) getExternalID.invoke( bean, null );
        }
        catch ( Exception ex ) {
            throw new PDSException ( "Error in InternalToExternalIDTranslator: " + ex.toString(), ex );
        }

        return returnValue;
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