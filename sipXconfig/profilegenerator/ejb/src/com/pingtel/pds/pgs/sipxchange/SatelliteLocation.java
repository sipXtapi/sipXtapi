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

import java.net.URL;
import java.util.Collection;
import java.util.Iterator;
import java.util.ArrayList;

/**
  *  SatelliteLocation component encapsulates a host machine.
  *  A SatelliteLocation will have
  *
  * 1) an URL to send replication data,
  * 2) an URL to access process information
  * 3) an unique id identifying itself e.g. primary media server,
  *     seconday media server, etc
  *
  *  A SatelliteLocation will have zero or many SatelliteComponents.
  *
  * @author Pradeep Paudyal
  */
public class SatelliteLocation
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected String m_strLocationID     ;
    protected String m_strReplicationURL ;
    protected String m_strAgentURL ;
    protected Collection m_colComponents;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Protected constructor to guard against rogue instantiation.
     */
    protected SatelliteLocation(String strLocationID,  String strReplicationURL,
                                String strAgentURL, Collection colComponents) {
          this.m_strLocationID      = strLocationID;
          this.m_strReplicationURL  = strReplicationURL;
          this.m_strAgentURL        = strAgentURL;
          this.m_colComponents     = colComponents;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * return a unique id identifying this location .
     * eg Primary Media Server, Secondary Media Server
     */
    public String getLocationID(){
        return m_strLocationID;
    }

    /**
     * gets the replication URL to send data to this location(host).
     */
    public String getReplicationURL(){
        return m_strReplicationURL;
    }

    /**
     * gets the replication URL to get process information from this host.
     */
    public String getAgentURL(){
        return m_strAgentURL;
    }

    /**
     * gets the components that reside in this location(host)
     */
    public Collection getComponents(){
        return m_colComponents;
    }

    /**
     * Does this location have component of this type?
     */
    public boolean hasComponents( int iType ){
        boolean bRet = false;
        if( m_colComponents != null ){
            for ( Iterator iComponents  = m_colComponents.iterator(); iComponents.hasNext(); ) {
                SatelliteComponent component = (SatelliteComponent) iComponents.next();
                if( component.getType() == iType ){
                    bRet = true;
                    break;
                }
            }
        }
        return bRet;
    }

    /**
     * Does this location have component of this type?
     */
    public Collection getComponents( int iType ){
        ArrayList arrayList = new ArrayList();
        if( m_colComponents != null ){
            for ( Iterator iComponents  = m_colComponents.iterator(); iComponents.hasNext(); ) {
                SatelliteComponent component = (SatelliteComponent) iComponents.next();
                if( component.getType() == iType ){
                    arrayList.add( component );
                }
            }
        }
        return arrayList;
    }

    /**
     * gets the hostname of this location(host).
     * For example, if the agent URL of this location is
     * https://10.1..107/, it returns 10.1.1.107
     *
     */
    public String getHostName(){
        String strRet = null;
        try{
             if (m_strAgentURL.startsWith("https"))
            {
                System.setProperty
                    ("java.protocol.handler.pkgs", "com.sun.net.ssl.internal.www.protocol");
                java.security.Security.addProvider
                    (new com.sun.net.ssl.internal.ssl.Provider());
            }
            URL url = new URL(m_strAgentURL);
            strRet = url.getHost();
        }catch( Exception e ){
            e.printStackTrace();
        }
        return strRet;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    public boolean equals( Object obj ){
        boolean bRet = false;
        if( obj instanceof SatelliteLocation ){
            if( ((SatelliteLocation)(obj)).getLocationID().equals( this.m_strLocationID) )
                bRet = true;
        }
        return bRet;
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
