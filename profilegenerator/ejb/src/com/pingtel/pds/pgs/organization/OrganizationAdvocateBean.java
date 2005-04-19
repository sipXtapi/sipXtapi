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

package com.pingtel.pds.pgs.organization;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.patch.PatchManager;
import com.pingtel.pds.pgs.patch.PatchManagerHome;
import com.pingtel.pds.pgs.phone.*;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.pgs.user.*;

import org.jdom.Element;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.io.FileNotFoundException;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.Collection;
import java.util.Iterator;



public class OrganizationAdvocateBean extends JDBCAwareEJB
         implements SessionBean, PDSDefinitions, PGSDefinitions, OrganizationAdvocateBusiness {

    // session bean references
    private UserAdvocate m_userAdvocateEJBObject;
    private UserGroupAdvocate m_userGroupAdvocateEJBObject;
    private RefDataAdvocate m_refDataAdvocateEJBObject;
    private DeviceGroupAdvocate m_deviceGroupAdvocateEJBObject;
    private DeviceTypeAdvocate m_deviceTypeAdvocateEJBObject;
    private ExtensionPoolAdvocate mExtensionPoolAdvocate;

    /*
     *  Home interface references
     */
    private OrganizationHome m_organizationHome;
    private UserGroupHome m_userGroupHome;
    private DeviceGroupHome m_deviceGroupHome;
    private RefConfigurationSetHome m_rcsHome;
    private RefPropertyHome m_refPropertyHome;
    private UserHome m_userHome;
    private DeviceTypeHome m_deviceTypeHome;
    private PatchManager m_patchManager;


    /**
     *  The Session Context object
     */
    private SessionContext m_ctx;



    public Organization install (   String organizationName,
                                    String stereotype,
                                    String dnsDomain,
                                    String superAdminPassword ) throws PDSException {

        Organization organization = null;

        try {

            logDebug ( "about to import device definitions" );
            importDeviceTypeDefinitions();

            organization = createOrganization ( organizationName,
                                                null,
                                                stereotype,
                                                dnsDomain );
            logDebug ( "created organization" );

            mExtensionPoolAdvocate.createExtensionPool( organization.getID().toString(),
                                                        "reserved" );

            createSpecialUsers( organization, superAdminPassword);

            createDatabaseVersionRecord();

            ///////////////////////////////////////////////////////////////////////
            //
            // The installonly organization always has ID "1000000";
            //
            ///////////////////////////////////////////////////////////////////////

            deleteOrganization( "1000000" );
            logDebug ( "deleted installation organization" );
        }
        catch (PDSException ex) {
            logError(ex.getMessage());
            m_ctx.setRollbackOnly();

            throw new PDSException (collateErrorMessages ( "UC930" ), ex);
        }
        catch (Exception ex) {
            logFatal( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "UC930",
                                        "E4045",
                                        new Object [] { organizationName } ) );
        }

        return organization;
    }

    private void createDatabaseVersionRecord()
            throws PDSException, RemoteException, SQLException {

        Element patchRoot = m_patchManager.getUpgradeInfo();
        String currentVersion = patchRoot.getAttributeValue("currentVersion");

        executePreparedUpdate(  "INSERT INTO VERSIONS ( APPLIED, VERSION ) " +
                                "   VALUES ( ?, ? ) ",
                                new Object [] { new java.sql.Date ( new java.util.Date().getTime() ),
                                                Integer.valueOf( currentVersion ) } );
    }


    private void createSpecialUsers(Organization organization, String superAdminPassword)
            throws PDSException, RemoteException {

        User newAdmin =
                m_userAdvocateEJBObject.createUser(  organization.getID(),
                                            null,
                                            null,
                                            superAdminPassword,
                                            "superadmin",
                                            null,
                                            null,
                                            null );

        logDebug ( "created superadmin" );

        m_userAdvocateEJBObject.assignSecurityRole(    newAdmin ,
                                        "SUPER",
                                        superAdminPassword );

        User SDS = m_userAdvocateEJBObject.createUser( organization.getID(),
                                        null,
                                        null,
                                        "SDS",
                                        "SDS",
                                        null,
                                        null,
                                        null );

        m_userAdvocateEJBObject.assignSecurityRole( SDS, "SDS", null );
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // This method basically just drives the importPhoneType method in
    // DeviceTypeAdvocate.
    //
    ///////////////////////////////////////////////////////////////////////
    private void importDeviceTypeDefinitions ()
        throws RemoteException, PDSException {

        StringBuffer basePath = new StringBuffer();
        String deviceTypeName = "Pingtel xpressa";

        try {
            String deviceTypePath =
                PathLocatorUtil.getInstance().
                        getPath( PathLocatorUtil.PHONEDEFS_FOLDER, PathLocatorUtil.PGS );

            basePath.append( deviceTypePath );

        }
        catch ( FileNotFoundException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC930",
                                        "E4055",
                                        new Object[]{ deviceTypeName }));
        }

        m_deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() + "newxpressa.xml" );
        m_deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() + "ixNT.xml" );
        m_deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() + "7960-definition.xml" );
        m_deviceTypeAdvocateEJBObject.importDeviceType( basePath.toString() + "7940-definition.xml" );
    }

    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  parentID  Description of the Parameter
     *@param  dnsDomain             Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException      Description of the Exception
     */
    public Organization createOrganization( String name,
                                            String parentID,
                                            String stereotype,
                                            String dnsDomain )
             throws PDSException {

        Organization newOrg = null;

        int intStereo = new Integer ( stereotype ).intValue();

        try {
            if (    intStereo != ORG_CUSTOMER &&
                    intStereo != ORG_ENTERPRISE &&
                    intStereo != ORG_SERVICE_PROVIDER ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages ( "UC900",
                                            "E3008",
                                            new Object [] { stereotype } ) );
            }

            newOrg = m_organizationHome.create(  name,
                                        parentID == null ? null :
                                        new Integer(parentID),
                                        intStereo,
                                        dnsDomain);


            ////////////////////////////////////////////////////////////////////
            //
            // We create the following 'canned' ref config sets.
            //
            //  All user settings
            //  All device settings
            //  [FOR each device type]
            //      All device settings
            //
            ////////////////////////////////////////////////////////////////////

            RefConfigurationSet rcsPG = createFullRCSList(  "device", newOrg);
            RefConfigurationSet rcsUG = createFullRCSList(  "user", newOrg );

            Collection deviceTypes = m_deviceTypeHome.findAll();
            for ( Iterator devTypeI = deviceTypes.iterator(); devTypeI.hasNext(); ) {
                DeviceType deviceType = (DeviceType) devTypeI.next();
                m_deviceTypeAdvocateEJBObject.createDeviceSpecificRefConfigurationSet ( newOrg,
                                                                                        deviceType );
            }

            m_userGroupHome.create("default", newOrg.getID(), rcsUG.getID(), null);
            m_deviceGroupHome.create("default", newOrg.getID(), rcsPG.getID());

        }
        catch (CreateException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC900",
                    "E2008",
                    new Object[]{name}),
                    ex);
        }
        catch ( FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC900",
                    "E2008",
                    new Object[]{name}),
                    ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages("UC900",
                    "E4014",
                    new Object[]{name}));
        }

        return newOrg;
    }



    private RefConfigurationSet createFullRCSList(  String userOrDevice,
                                                    Organization organization )
             throws PDSException, RemoteException {

        RefConfigurationSet rcs = null;

        try {

            logDebug ( "creating ref config set: " + userOrDevice );
            try {
                if (userOrDevice.equals("device")) {

                    rcs = m_rcsHome.create( "Complete Device",
                                            organization.getID() );
                } else if (userOrDevice.equals("user")) {

                    rcs = m_rcsHome.create( "Complete User",
                                            organization.getID() );
                }
            } catch (CreateException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC900",
                                            "E2009",
                                            new Object[]{   userOrDevice,
                                                            organization.getExternalID() }),
                    ex);
            }

            Collection cRP = null;

            try {
                if (userOrDevice.equals("device")) {
                    cRP = m_refPropertyHome.findByProfileType( PROF_TYPE_PHONE );
                }
                else if (userOrDevice.equals("user")) {
                    cRP = m_refPropertyHome.findByProfileType( PROF_TYPE_USER);
                }
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC900",
                                            "E1017",
                                            new Object[]{   userOrDevice,
                                                            organization.getExternalID()}),
                    ex);
            }

            logDebug ( "about to assigning ref properties to new RCS" );

            assignRefPropertiesToRCS ( cRP, rcs );

            logDebug ( "done assignin ref properties to new RCS" );
        }
        catch (PDSException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException("UC900", ex);
        }

        return rcs;
    }




    private void assignRefPropertiesToRCS ( Collection rpCollection,
                                            RefConfigurationSet rcs )
        throws RemoteException, PDSException {

        RefConfigSetAssignment rule = null;

        logDebug ( "in assignRefPropertiesToRCS" );

        for (Iterator iRP = rpCollection.iterator(); iRP.hasNext(); ) {
            RefProperty rp = (RefProperty) iRP.next();
            rule = new RefConfigSetAssignment( rp.getID(), false, false);
            m_refDataAdvocateEJBObject.assignRefPropertytoRCS( rcs, rule );
        }

        logDebug ( "completed OrganziationAdv:assignRefPropertiesToRCS" );
    }


    /**
     *  Description of the Method
     *
     *@param  organizationID    Description of the Parameter
     *@exception  PDSException  Description of the Exception
     */
    public void deleteOrganization( String organizationID )
             throws PDSException {

        Integer orgID = new Integer(organizationID);
        Organization o = null;
        String externalID = null;

        try {
            try {
                o = m_organizationHome.findByPrimaryKey(orgID);
                externalID = o.getExternalID();

                deleteUserGroups( o );
                deleteUsers ( o );
                deleteDeviceGroups( o );
                deleteRefConfigSets( o );

                o.remove();
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC910",
                                            "E1018",
                                            new Object[]{ organizationID } ),
                    ex);
            }
            catch (RemoveException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC910",
                                            "E6006",
                                            new Object[]{   externalID == null ?
                                                                organizationID :
                                                                 externalID } ),
                    ex);
            }
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC910",
                                        "E4015",
                                        new Object[]{   externalID == null ?
                                                            organizationID :
                                                            externalID } ) );
        }
    }



    private void deleteUserGroups( Organization organization )
             throws PDSException, RemoteException {


        try {
            Collection c = m_userGroupHome.findByOrganizationID( organization.getID() );

            for (Iterator i = c.iterator(); i.hasNext(); ) {
                m_userGroupAdvocateEJBObject.deleteUserGroup(   (UserGroup) i.next(),
                                                                DEL_DEEP_DELETE,
                                                                null);
            }

        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC910",
                                        "E1020",
                                        new Object[]{ organization.getExternalID() }),
                ex);
        }
    }


    private void deleteUsers ( Organization organization )
        throws PDSException, RemoteException {

        try {
            Collection users =
                    m_userHome.findByOrganizationID( organization.getID() );

            for ( Iterator i = users.iterator(); i.hasNext(); ) {
                User u = (User) i.next();
                m_userAdvocateEJBObject.deleteUser( u );
            }

        }
        catch ( FinderException ex ) {
            m_ctx.setRollbackOnly();

            throw new EJBException(
                collateErrorMessages(   "UC910",
                                        "E1035",
                                        new Object[]{ organization.getExternalID() }));
        }

    }


    private void deleteDeviceGroups(Organization organization)
             throws PDSException, RemoteException {

        DeviceGroup deviceGroup = null;

        try {
            Collection c = m_deviceGroupHome.findByOrganizationID(organization.getID() );

            for (Iterator i = c.iterator(); i.hasNext(); ) {
                deviceGroup = (DeviceGroup) i.next();
                m_deviceGroupAdvocateEJBObject.deleteDeviceGroup(   deviceGroup,
                                                                    DEL_DEEP_DELETE,
                                                                    null);
            }

        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC910",
                                        "E1020",
                                        new Object[]{ organization.getExternalID() }),
                ex);
        }
        catch (PDSException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC910" ), ex );
        }
    }



    private void deleteRefConfigSets( Organization organization )
             throws PDSException, RemoteException {

        try {
            Collection c = m_rcsHome.findByOrganizationID( organization.getID() );

            for (Iterator i = c.iterator(); i.hasNext(); ) {
                RefConfigurationSet rcs = (RefConfigurationSet) i.next();
                m_refDataAdvocateEJBObject.deleteRefConfigSet( rcs );
            }

        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC910",
                                        "E1022",
                                        new Object[]{ organization.getExternalID() }),
                                        ex);
        }
        catch (PDSException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException("UC910", ex);
        }
    }





    /**
     *  Description of the Method
     *
     *@param  organizationID        Description of the Parameter
     *@param  name                  Description of the Parameter
     *@param  parentID  Description of the Parameter
     *@param  dnsDomain             Description of the Parameter
     *@exception  PDSException      Description of the Exception
     */
    public void editOrganization(   String organizationID,
                                    String name,
                                    String parentID,
                                    String dnsDomain)
             throws PDSException {

        Organization organization = null;

        try {
            organization =
                    m_organizationHome.findByPrimaryKey(new Integer(organizationID));


            if (parentID != null &&
                    !(parentID.equals(
                    organization.getParentID().toString()))) {

                organization.setParentID(
                        new Integer(parentID));
            }

            if (name != null && !(organization.getName().equals(name))) {
                organization.setName(name);
            }

            String oldDomainName = organization.getDNSDomain(); 
            if (dnsDomain != null && 
                    !(oldDomainName.equals(dnsDomain))) {
                organization.setDNSDomain(dnsDomain);
                fixDomainName(organization,oldDomainName);
            }
        } catch (FinderException ex) {
            this.m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC920",
                                        "E1018",
                                        new Object[]{organizationID}),
                ex);
        } catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String externalID = null;

            try {
                externalID = organization.getExternalID();
            }
            catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages("UC920",
                                    "E4017",
                                    new Object[]{   organization == null ?
                                                        organizationID :
                                                        externalID } ) );
        }

    }


    private void fixDomainName(Organization orgChanged, String oldDomainName) throws RemoteException, PDSException {
        String name = orgChanged.getExternalID();
        try {
            Context context = new InitialContext();
            UserAdvocateHome uaHome = (UserAdvocateHome) context.lookup("UserAdvocate");
            DeviceAdvocateHome daHome = (DeviceAdvocateHome) context.lookup("DeviceAdvocate");
            
            UserAdvocate ua = uaHome.create();
            ua.fixDnsDomain(orgChanged, oldDomainName);

            DeviceAdvocate da = daHome.create();
            da.fixDnsName(orgChanged);
        } catch (NamingException e) {
            logFatal( e.toString(), e );
            throw new EJBException(e);
        }
        catch (CreateException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC920",
                    "E4017",
                    new Object[]{name}),
                    ex);
        }
    }

    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbCreate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbRemove() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbActivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbPassivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     *
     *@param  ctx  The new sessionContext value
     */
    public void setSessionContext(SessionContext ctx) {

        m_ctx = ctx;

        try {
            Context initial = new InitialContext();
            m_organizationHome = (OrganizationHome) initial.lookup("Organization");
            m_userGroupHome = (UserGroupHome) initial.lookup("UserGroup");
            m_deviceGroupHome = (DeviceGroupHome) initial.lookup("DeviceGroup");
            m_rcsHome = (RefConfigurationSetHome)
                    initial.lookup("RefConfigurationSet");

            m_refPropertyHome = (RefPropertyHome) initial.lookup("RefProperty");

            RefDataAdvocateHome rdaHome = (RefDataAdvocateHome)
                    initial.lookup("RefDataAdvocate");

            m_refDataAdvocateEJBObject = rdaHome.create();

            UserGroupAdvocateHome ugaHome = (UserGroupAdvocateHome)
                    initial.lookup("UserGroupAdvocate");

            m_userGroupAdvocateEJBObject = ugaHome.create();

            DeviceGroupAdvocateHome devGroupHome = (DeviceGroupAdvocateHome)
                    initial.lookup("DeviceGroupAdvocate");

            m_deviceGroupAdvocateEJBObject = devGroupHome.create();

            m_userHome = (UserHome) initial.lookup("User");

            UserAdvocateHome userAdvocateHome =
                    (UserAdvocateHome) initial.lookup ( "UserAdvocate" );

            DeviceTypeAdvocateHome dtaHome = (DeviceTypeAdvocateHome)
                    initial.lookup ( "DeviceTypeAdvocate" );

            m_deviceTypeAdvocateEJBObject = dtaHome.create();

            m_deviceTypeHome = (DeviceTypeHome) initial.lookup ( "DeviceType" );

            PatchManagerHome patchManagerHome = (PatchManagerHome)
                    initial.lookup( "PatchManager" );

            m_patchManager = patchManagerHome.create();

            m_userAdvocateEJBObject = userAdvocateHome.create();

            ExtensionPoolAdvocateHome extensionPoolAdvocateHome =
                    (ExtensionPoolAdvocateHome) initial.lookup( "ExtensionPoolAdvocate" );

            mExtensionPoolAdvocate = extensionPoolAdvocateHome.create();
        }
        catch (NamingException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
        catch ( CreateException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
        catch ( RemoteException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }

    }


    /**
     *  Standard Boiler Plate Implementation
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }

}
