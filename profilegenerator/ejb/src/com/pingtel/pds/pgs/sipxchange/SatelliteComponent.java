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

package  com.pingtel.pds.pgs.sipxchange;

/**
 * SatelliteComponent component encapsulates the component inside a host machine.
 *  A SatelliteComponent will have
 * 1) an unique id identifying this component e.g.CommServer1,CommServer2, MediaServer1
 * 2) a type, eg commServer, mediaServer, etc.
 *
 * @author Pradeep Paudyal
 */
public class SatelliteComponent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** UNDEFINED COMPONENT TYPE */
    public static final int TYPE_UNDEFINED     = -1;

    /** MEDIA SERVER COMPONENT TYPE */
    public static final int TYPE_MEDIA_SERVER  = 1;

    /** COMM SERVER COMPONENT TYPE */
    public static final int TYPE_COMM_SERVER   = 2;

    protected static final String STR_MEDIA_SERVER_COMPONENT    = "media-server";
    protected static final String STR_COMM_SERVER_COMPONENT    = "comm-server";

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected String m_strComponentID   ;
    protected int m_iComponentType ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Protected constructor to guard against rogue instantiation.
     */
    protected SatelliteComponent( String strComponentID, int iComponentType) {

          this.m_strComponentID   = strComponentID;
          this.m_iComponentType   = iComponentType;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


    /**
     * return the type of this component.
     * eg TYPE_MEDIA_SERVER, TYPE_COMM_SERVER
     */
    public  int getType(){
        return m_iComponentType;
    }

    public  String getTypeAsString(){
        String retStr = "";
        if( m_iComponentType == TYPE_MEDIA_SERVER ){
            retStr = STR_MEDIA_SERVER_COMPONENT;
        }else if( m_iComponentType == TYPE_COMM_SERVER ){
            retStr = STR_COMM_SERVER_COMPONENT;
        }
        return retStr;
    }


    /**
     * return the unique ID of this component.
     * eg media_sever1, pingtel_comm_server1, pingtel_comm_server2
     */
    public String  getComponentID(){
        return m_strComponentID;
    }


    /**
     * return a unique id identifying the location this component lives in.
     * eg Primary Media Server, Secondary Media Server
     */
    /*
    public SatelliteLocation getLocation(){
        return m_location;
    }
    */





//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
