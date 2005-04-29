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


package  com.pingtel.pds.pgs.sipxchange.replication;

import com.pingtel.pds.pgs.sipxchange.*;
/**
 * This class encapsulates errors that may be encountered
 * when sending data to different components.
 *
 * @author Pradeep Paudyal
 */
public class ReplicationError
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

     /** UNKNOWN ERROR */
    public static final int UNKNOWN_ERROR      = -1;

    /**
     * error when trying to connect to the target.
     * possible network errors.
     */
    public static final int COMMUNICATION_ERROR  = 1;

    /**
     * some internal error in the component that
     * prevented the data from being replicated
     */
    public static final int COMPONENT_ERROR       = 2;

    /** error while authenticating to send data to target */
    public static final int AUTHENTICATION_ERROR = 3;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    private int m_iErrorType = UNKNOWN_ERROR;
    private String m_strError ;
    private SatelliteLocation  m_location;
    private SatelliteComponent m_component;
//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * constrcutor that takes errorType, errorString and
     * the name of the database associated with the error.
     */
    public ReplicationError(int iErrorType, String strError,
                                SatelliteLocation location,
                                SatelliteComponent component){
      m_iErrorType = iErrorType;
      m_strError = strError;
      m_location = location;
      m_component = component;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *  gets the error type.
     *  eg  COMMUNICATION_ERROR, DATABASE_ERROR
     */
    public int getErrorType(){
        return  m_iErrorType;
    }


    /**
     * gets the error string if any.
     */
    public String getErrorString(){
        return m_strError;
    }


    /**
     * gets the database where the error occured.
     * If it was a communication failure and no database
     * informationc could be found, returns null.
     */
    public SatelliteLocation getErrorLocation(){
        return m_location;
    }

    public SatelliteComponent getErrorComponent(){
        return m_component;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    public String toString(){
        StringBuffer buffer = new StringBuffer();
        buffer.append( "\nError Type:");
        if( m_iErrorType == AUTHENTICATION_ERROR )
            buffer.append("AUTHENTICATION_ERROR ");
        else if(  m_iErrorType == COMMUNICATION_ERROR )
            buffer.append("COMMUNICATION_ERROR ");
        else if(  m_iErrorType == COMPONENT_ERROR )
            buffer.append("COMPONENT_ERROR ");
        else if( m_iErrorType == UNKNOWN_ERROR )
            buffer.append("UNKNOWN_ERROR ");

        buffer.append( "\nURL: " + m_location.getReplicationURL());
        buffer.append( "\nError Message: " );
        buffer.append( m_strError );
        buffer.append( "\n");
        return buffer.toString();
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}
