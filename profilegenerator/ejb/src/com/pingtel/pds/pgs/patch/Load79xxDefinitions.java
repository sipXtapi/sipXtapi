/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/patch/Load79xxDefinitions.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.patch;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.phone.DeviceTypeAdvocate;
import com.pingtel.pds.pgs.phone.DeviceTypeAdvocateHome;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.profile.*;

import javax.naming.InitialContext;
import javax.rmi.PortableRemoteObject;
import javax.ejb.FinderException;
import java.util.HashMap;
import java.util.Collection;
import java.util.Iterator;
import java.io.FileNotFoundException;
import java.rmi.RemoteException;

import org.apache.log4j.Category;


public class Load79xxDefinitions implements Patch {

    private Category m_logger = Category.getInstance( "pgs" );

    private RefConfigurationSetHome refConfigurationSetHome;
    private RefPropertyHome refPropertyHome;
    private RefDataAdvocate refDataAdvocate;


    public void initialize(HashMap props) {}

    public String update() throws PDSException {

        try {
            InitialContext initial = new InitialContext();

            Object objref = initial.lookup( "DeviceTypeAdvocate" );
                DeviceTypeAdvocateHome deviceTypeAdvocateHome =
                    ( DeviceTypeAdvocateHome ) PortableRemoteObject.
                        narrow( objref, DeviceTypeAdvocateHome.class );

            DeviceTypeAdvocate deviceTypeAdvocateEJBObject = deviceTypeAdvocateHome.create();

            objref = initial.lookup( "Organization" );
            OrganizationHome organizationHome =
                    ( OrganizationHome ) PortableRemoteObject.
                        narrow( objref, OrganizationHome.class );

            objref = initial.lookup( "RefConfigurationSet" );
            refConfigurationSetHome =
                    ( RefConfigurationSetHome ) PortableRemoteObject.
                        narrow( objref, RefConfigurationSetHome.class );

            objref = initial.lookup( "RefProperty" );
            refPropertyHome =
                    ( RefPropertyHome ) PortableRemoteObject.
                        narrow( objref, RefPropertyHome.class );

            objref = initial.lookup( "RefDataAdvocate" );
            RefDataAdvocateHome refDataAdvocateHome =
                    ( RefDataAdvocateHome ) PortableRemoteObject.
                        narrow( objref, RefDataAdvocateHome.class );

            refDataAdvocate = refDataAdvocateHome.create();

            m_logger.info( "Found home interfaces");

            StringBuffer basePath = new StringBuffer();

            try {
                String deviceTypePath =
                    PathLocatorUtil.getInstance().
                            getPath( PathLocatorUtil.PHONEDEFS_FOLDER, PathLocatorUtil.PGS );

                basePath.append( deviceTypePath );

            }
            catch ( FileNotFoundException ex ) {
                m_logger.fatal ( ex.toString(), ex );

                throw ex;
            }

            Collection allOrganizations = organizationHome.findAll();
            Organization organization = null;

            for ( Iterator orgI = allOrganizations.iterator(); orgI.hasNext(); ) {
                organization = (Organization) orgI.next();
            }

            DeviceType deviceType = null;
            m_logger.info ( "loading up: " + basePath.toString() + "7960-definition.xml" );
            deviceType =
                    deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() +
                                                                    "7960-definition.xml" );

            m_logger.info ( "creating device specific RCS" );
            deviceTypeAdvocateEJBObject.createDeviceSpecificRefConfigurationSet( organization,
                                                                                    deviceType );

            m_logger.info ( "loading up: " + basePath.toString() + "7940-definition.xml" );

            deviceType =
                    deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() +
                                                                    "7940-definition.xml" );

            m_logger.info ( "creating device specific RCS" );
            deviceTypeAdvocateEJBObject.createDeviceSpecificRefConfigurationSet( organization,
                                                                                    deviceType );

            Collection refConfigurationSets =
                    refConfigurationSetHome.findByName( "Complete User");

            m_logger.info ( "found Complete User ref config set");

            Collection newUserRefProperties = refPropertyHome.findByProfileType( PDSDefinitions.PROF_TYPE_USER );
            newUserRefProperties.addAll ( refPropertyHome.findByProfileType( PDSDefinitions.PROF_TYPE_APPLICATION_REF ));
            m_logger.info ( "got new user ref properties collection");

            repopulateRefConfigurationSet( refConfigurationSets, newUserRefProperties );
            m_logger.info ( "repopulated Complete User");

            refConfigurationSets.clear();

            refConfigurationSets =
                    refConfigurationSetHome.findByName( "Complete Device");

            m_logger.info ( "found Complete Device ref config set");

            newUserRefProperties.clear();
            newUserRefProperties = refPropertyHome.findByProfileType( PDSDefinitions.PROF_TYPE_PHONE );
            m_logger.info ( "got new device ref properties collection");

            repopulateRefConfigurationSet( refConfigurationSets, newUserRefProperties );
            m_logger.info ( "repopulated Complete Device");

        }
        catch ( Exception ex ) {
            m_logger.error ( "Exception in PasswordDigestor: " + ex.toString() );

            throw new PDSException ( ex.toString() );
        }

        return "S";
    }


    private void repopulateRefConfigurationSet( Collection refConfigurationSets,
                                                Collection newRefProperties )
            throws FinderException, RemoteException, PDSException {

        RefConfigurationSet refConfigurationSet = null;

        for ( Iterator rcsI = refConfigurationSets.iterator(); rcsI.hasNext(); ) {
            refConfigurationSet = (RefConfigurationSet) rcsI.next();
        }

        m_logger.info ( "found refConfigurationSet: " + refConfigurationSet.getName() );

        Collection existingProperties =
                refPropertyHome.findByRefConfigurationSetID( refConfigurationSet.getID() );

        m_logger.info ( "found existing properties");

        for ( Iterator propertyI = existingProperties.iterator(); propertyI.hasNext(); ) {
            RefProperty refProperty = (RefProperty) propertyI.next();
            m_logger.info ( "unassigning property: " + refProperty.getName() );
            refDataAdvocate.unassignRefPropertyFromRCS(
                    refConfigurationSet.getID().toString(),
                    refProperty.getID().toString() );
        }

        RefConfigSetAssignment rule = null;

        for (Iterator iRP = newRefProperties.iterator(); iRP.hasNext(); ) {
            RefProperty rp = (RefProperty) iRP.next();
            m_logger.info ( "assigning property: " + rp.getName() );
            rule = new RefConfigSetAssignment( rp.getID(), false, false);
            refDataAdvocate.assignRefPropertytoRCS( refConfigurationSet, rule );
        }
    }
}
