/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/sipxchange/SatelliteManager.java#5 $
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

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import java.io.FileNotFoundException;
import java.io.File;

import java.util.Collection;
import java.util.ArrayList;
import java.util.Iterator;


import com.pingtel.pds.common.PathLocatorUtil;
/**
 * This singleton class manages the data about Satellite Locations(hosts) and
 * the components inside them.
 * @author Pradeep Paudyal
 */
public class SatelliteManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static SatelliteManager m_manager = new SatelliteManager();

    private ArrayList m_alSatelliteComponents = new ArrayList();
    private ArrayList m_alSatelliteLocations  = new ArrayList();

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private SatelliteManager(){
        String xmlBasePath = null;
        try{
            xmlBasePath =
                PathLocatorUtil.getInstance().
                            getPath( PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS );
        }catch( FileNotFoundException e){
            e.printStackTrace();
        }
        if( xmlBasePath != null ){
           String fullPath =  xmlBasePath + "topology.xml";
           parseTopologyFile( fullPath );
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * gets a singleton instance of SatelliteManager.
     */
    public static SatelliteManager getInstance(){
        return  m_manager;
    }

     /**
     * gets an array of all satellite components residing in
     * all hosts defined in topology.xml file.
     */
    public Collection getAllComponents(){
        return m_alSatelliteComponents;
    }


    /**
     * gets a Collection of target components by type.
     * eg We may want to get all media_server components in all
     * the satellite machines that are part of SIpXchange.
     */
    public Collection getComponents(int iType){
        ArrayList arrayListComponents = new ArrayList();
        for ( Iterator iterator = getAllComponents().iterator(); iterator.hasNext(); ) {
            SatelliteComponent component =
                                (SatelliteComponent)iterator.next();
            if( component.getType() == iType ){
                arrayListComponents.add(component);
            }
        }
        return arrayListComponents;
    }

     /**
     * get all the locations(hosts).
     */
    public Collection getAllLocations(){
        return m_alSatelliteLocations;
    }

    /**
     * gets the location(host) with the given id of "strLocationID".
     */
    public SatelliteLocation getLocation( String strLocationID ){
        SatelliteLocation retLocation = null;
        Collection locations = getAllLocations();
        for ( Iterator iLocations = locations.iterator(); iLocations.hasNext(); ) {
            SatelliteLocation location = (SatelliteLocation) iLocations.next();
            if( location.getLocationID().equals(strLocationID) ){
                retLocation = location;
                break;
            }
        }
        return retLocation;
    }





    /**
     * gets an array of satellite target components of a particular type residing
     * in a particular host. eg We may want to get all
     * the components in a particular  machine
     * that are part of SIpXchange.
     */
     /*   public satelliteTargetComponent[]
            getTargetComponentsByTypeAndHostID(int iType, String strHostID);
     */




//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
     /**
     * parses a xml file that has data about the servers and the
     * different components.
     */
    private void parseTopologyFile( String strXMLTopologyFile ){
       try {
            SAXBuilder saxBuilder = new SAXBuilder(  );
            Document doc =
                saxBuilder.build( new File( strXMLTopologyFile ) );

            Element root = doc.getRootElement();
            Collection locations = root.getChildren( "location" );

            for ( Iterator iLocations = locations.iterator(); iLocations.hasNext(); ) {

                Element location = (Element) iLocations.next();
                String  strLocationID    = location.getAttribute( "id" ).getValue();
                String strReplicationURL = location.getChild( "replication_url" ).getTextNormalize();
                String strAgentURL       = location.getChild( "agent_url" ).getTextNormalize();

                Collection components = location.getChildren( "component" );
                ArrayList componentListForThisComponent = new ArrayList();
                for ( Iterator iComponents  = components.iterator(); iComponents.hasNext(); ) {
                    Element elComponent     = (Element) iComponents.next();
                    String strComponentID   = elComponent.getAttribute( "id" ).getValue();
                    String strComponentType = elComponent.getAttribute( "type" ).getValue();
                    int iComponentType      = getIntComponentType(strComponentType);
                    SatelliteComponent satelliteComponent =
                        new SatelliteComponent(strComponentID, iComponentType);
                    componentListForThisComponent.add(satelliteComponent);
                    m_alSatelliteComponents.add( satelliteComponent );
                }
                SatelliteLocation satelliteLocation =
                    new SatelliteLocation( strLocationID, strReplicationURL,
                                            strAgentURL, componentListForThisComponent);
                m_alSatelliteLocations.add( satelliteLocation );

            }
        }
        catch ( JDOMException ex ) {
            ex.printStackTrace();
        } 
        catch ( java.io.IOException ex ) {
            ex.printStackTrace();
        } 
    }


    private int getIntComponentType( String strComponentType){
        int iComponentType = SatelliteComponent.TYPE_UNDEFINED;
        if( strComponentType.equals(SatelliteComponent.STR_MEDIA_SERVER_COMPONENT) ){
            iComponentType = SatelliteComponent.TYPE_MEDIA_SERVER;
        }else if( strComponentType.equals(SatelliteComponent.STR_COMM_SERVER_COMPONENT) ){
            iComponentType = SatelliteComponent.TYPE_COMM_SERVER;
        }
        return iComponentType;
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}