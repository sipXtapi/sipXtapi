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

package com.pingtel.pds.pgs.jsptags.ejb;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PropertyGroupLink;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.common.ElementUtilException;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.phone.*;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;
import com.pingtel.pds.pgs.user.UserHome;
import org.jdom.Element;
import org.jdom.CDATA;

import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.*;


/**
 * TagHandlerHelperBean is the EJ Bean implementation class for the
 * TagHandlerHelper bean.   TagHandlerHelper provides back-end UI
 * processing for JSP custom tag handlers, encapsulating the tag handler
 * processing in a session facade improves performance.
 *
 * @author IB
 */
public class TagHandlerHelperBean extends JDBCAwareEJB
    implements SessionBean, TagHandlerHelperBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // standard session bean stuff
    private SessionContext mCTX;

    // Home i/f references
    private DeviceHome mDeviceHome = null;
    private DeviceGroupHome mDeviceGroupHome = null;
    private UserHome mUserHome = null;
    private UserGroupHome mUserGroupHome = null;
    private RefPropertyHome mRefPropertyHome = null;
    private RefConfigurationSetHome mRefConfigSetHome = null;
    private DeviceTypeHome mDeviceTypeHome = null;
    private ConfigurationSetHome mConfigSetHome = null;
    private ManufacturerHome mManufacturerHome = null;
    private OrganizationHome mOrganizationHome = null;

    // Stateless Session Bean refs.
    private ProjectionHelper mProjectionHelperEJBObject = null;

    // Misc.
    private HashMap mUserEJBObjectCache = new HashMap();
    private HashMap mUserGroupEJBObjectCache = new HashMap();
    private HashMap mUserConfigSetEJBObjectCache = new HashMap();
    private HashMap mDeviceConfigSetEJBObjectCache = new HashMap();
    private HashMap mManufacturerEJBObjectCache = new HashMap();

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    // SessionBean callbacks
    public void ejbCreate() { }

    public void ejbRemove() { }

    public void ejbActivate() { }

    public void ejbPassivate() { }

    public void setSessionContext(SessionContext ctx) {
        mCTX = ctx;

        try {
            mDeviceHome = ( DeviceHome )
                EJBHomeFactory.getInstance().getHomeInterface(  DeviceHome.class,
                                                                "Device" );

            mUserHome = ( UserHome )
                EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                "User" );

            mDeviceGroupHome = ( DeviceGroupHome )
                EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupHome.class,
                                                                "DeviceGroup" );

            mRefPropertyHome = ( RefPropertyHome )
                EJBHomeFactory.getInstance().getHomeInterface(  RefPropertyHome.class,
                                                                "RefProperty" );

            mRefConfigSetHome = ( RefConfigurationSetHome )
                EJBHomeFactory.getInstance().getHomeInterface(  RefConfigurationSetHome.class,
                                                                "RefConfigurationSet" );

            mDeviceTypeHome = ( DeviceTypeHome )
                EJBHomeFactory.getInstance().getHomeInterface(  DeviceTypeHome.class,
                                                                "DeviceType" );

            mConfigSetHome = ( ConfigurationSetHome )
                EJBHomeFactory.getInstance().getHomeInterface(  ConfigurationSetHome.class,
                                                                "ConfigurationSet" );

            mManufacturerHome = ( ManufacturerHome )
                EJBHomeFactory.getInstance().getHomeInterface(  ManufacturerHome.class,
                                                                "Manufacturer" );

            mUserGroupHome = ( UserGroupHome )
                EJBHomeFactory.getInstance().getHomeInterface(  UserGroupHome.class,
                                                                "UserGroup" );

            mOrganizationHome = ( OrganizationHome )
                EJBHomeFactory.getInstance().getHomeInterface(  OrganizationHome.class,
                                                                "Organization" );

            ProjectionHelperHome phm = ( ProjectionHelperHome )
                EJBHomeFactory.getInstance().getHomeInterface(  ProjectionHelperHome.class,
                                                                "ProjectionHelper" );

            mProjectionHelperEJBObject = phm.create();
        }
        catch ( Exception ex ) {
            logFatal ( ex.toString(), ex  );
            throw new EJBException ( ex );
        }
    }


    public void unsetSessionContext() {
        mCTX = null;
    }


    // TagHandlerBussines methods

    /**
     * returns XML markup to be run through an xslt stylesheet to give
     * details for a given device.
     *
     * @param m_deviceID String representation of the PK for the device
     * to be viewd
     * @return JDOM Element containing the XML markup
     * @throws PDSException for all checked exceptions.
     */
    public Element getDeviceDetails ( String m_deviceID )
            throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            Device device =
                mDeviceHome.findByPrimaryKey( new Integer ( m_deviceID ) );

            Element attributes = getDeviceAttributes(device);

            DeviceGroup deviceGroup = null;

            if ( device.getDeviceGroupID() != null ) {
                deviceGroup =
                    mDeviceGroupHome.findByPrimaryKey( device.getDeviceGroupID());

                Element dgn = new Element ("devicegroupname");
                dgn.addContent( deviceGroup.getName() );
                attributes.addContent( dgn );
            }

            Element setProperties = new Element ( "setproperties" );
            documentRootElement.addContent( setProperties );

            if ( deviceGroup != null ) {
                Collection parentPIs =
                    mProjectionHelperEJBObject.addParentGroupConfigSets( deviceGroup );

                ProjectionInput projectedPI =
                    mProjectionHelperEJBObject.project( "com.pingtel.pds.pgs.plugins.projection.StandardTopDown",
                                                parentPIs,
                                                null,
                                                PDSDefinitions.PROF_TYPE_PHONE );

                Element content = projectedPI.getDocument().getRootElement();

                for (   Iterator iExisting = content.getChildren().iterator();
                        iExisting.hasNext(); ) {

                    Element property = (Element) iExisting.next();
                    String refPropertyID = property.getAttributeValue( "ref_property_id" );
                    setProperties.addContent( new Element ( "ref_property_id" ).setText( refPropertyID ) );
                }
            }

            documentRootElement.addContent( attributes );

            Element csElement = new Element ( "configurationset" );
            documentRootElement.addContent( csElement );
            ConfigurationSet cs = getDevicesConfigSetEJBObject( device.getID() );

            // configuration sets may be null for Cisco devices, no auto generation of device lines.
            if (cs != null) {
                csElement.addContent( cs.getContent() );
            }

            // find and all all ref properties content to the XML, we use it
            // to build the input fields dynamically
            ArrayList rows =
                    executePreparedQuery (  "SELECT code, id, name, content " +
                                            "FROM REF_PROPERTIES " +
                                            "WHERE profile_type = ? ",
                                            new Object [] { new Integer ( PDSDefinitions.PROF_TYPE_PHONE ) },
                                            4,
                                            10000 );

            Element refPropertiesElement = createRefPropertyElements(rows);
            documentRootElement.addContent( refPropertiesElement );

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );


            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( device.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            rcsElement.addContent( new Element ( "name" ).setText ( rcs.getName() ) );
            rcsElement.addContent( new Element ( "id" ).setText ( rcs.getID().toString() ) );
            rcsElement.addContent( new Element ( "organizationid" ).setText ( rcs.getOrganizationID().toString() ) );

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );
                rcsElement.addContent( rpElement );
            }
        }
        catch ( Exception e) {
            mCTX.setRollbackOnly();

            throw new PDSException ( e.getMessage() );
        }

        return documentRootElement;
    }


    /**
     * returns XML markup to be run through an xslt stylesheet to give
     * details for a given user.   Eventually this produces the User Details
     * tabbed page.
     *
     * @param userID primary key of the User entity for whom the details are
     * required
     * @return JDOM root Element for the "details"
     * @throws PDSException for all application level errors
     */
    public Element getUserDetails ( String userID ) throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            User user = getUserEJBObject( userID );
            Integer usersOrganizationID = user.getOrganizationID();

            Element attributes = createUserAttributes(user);
            Element organization = getUsersOrganizationDetails(usersOrganizationID);
            attributes.addContent( organization );

            Element aliases = getUserAliases(user);
            attributes.addContent( aliases );

            UserGroup ug = null;
            Integer userGroupID = user.getUserGroupID();
            ug = getUserGroupEJBObject(userGroupID);

            documentRootElement.addContent( attributes );

            //////////////////////////////////////////////////////////////////
            //
            // Add content for ref properties which have been set in parent
            // groups.
            //
            /////////////////////////////////////////////////////////////////

            Element setProperties = new Element ( "setproperties" );
            documentRootElement.addContent( setProperties );

            if ( ug != null ) {
                Collection parentPIs =
                    mProjectionHelperEJBObject.addParentGroupConfigSets(
                        ug,
                        PDSDefinitions.PROF_TYPE_USER );

                ProjectionInput projectedPI =
                    mProjectionHelperEJBObject.project( "com.pingtel.pds.pgs.plugins.projection.StandardTopDown",
                                                parentPIs,
                                                null,
                                                PDSDefinitions.PROF_TYPE_USER );

                Element content = projectedPI.getDocument().getRootElement();

                for (   Iterator iExisting = content.getChildren().iterator();
                        iExisting.hasNext(); ) {

                    Element property = (Element) iExisting.next();
                    String refPropertyID = property.getAttributeValue( "ref_property_id" );
                    setProperties.addContent( new Element ( "ref_property_id" ).setText( refPropertyID ) );
                }
            }


            Element csElement = new Element ( "configurationset" );
            documentRootElement.addContent( csElement );

            ConfigurationSet cs = getUsersConfigSetEJBObject( userID );

            if ( cs != null ) {
                csElement.addContent( cs.getContent() );
            }

            Element modelsElement = getModelsElement();
            documentRootElement.addContent( modelsElement );

            ArrayList rows =
                    executePreparedQuery (  "SELECT code, id, name, content " +
                                            "FROM REF_PROPERTIES",
                                            null,
                                            4,
                                            10000 );

            Element refPropertiesElement = createRefPropertyElements(rows);
            documentRootElement.addContent( refPropertiesElement );


            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );

            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( user.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch (  Exception ex ) {
            mCTX.setRollbackOnly();
            ex.printStackTrace();
            throw new PDSException ( ex.toString() );
        }

        return documentRootElement;
    }


    /**
     * getDeviceTabs creates the XML for the tabs for the User
     * details page.
     *
     * @param userID PK of the User whose tabs you are building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getUserTabs (String userID) throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            User user = getUserEJBObject(userID);
            Element attributes = createUserAttributes(user);
            documentRootElement.addContent( attributes );

            ArrayList rows =
                    executePreparedQuery (  "SELECT code " +
                                            "FROM REF_PROPERTIES",
                                            null,
                                            1,
                                            10000 );

            Element refPropertiesElement = createRefPropertyTabElements(rows);
            documentRootElement.addContent( refPropertiesElement );

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );

            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( user.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch (  Exception ex ) {
            mCTX.setRollbackOnly();
            ex.printStackTrace();
            throw new PDSException ( ex.toString() );
        }

        return documentRootElement;
    }


     /**
     * getDeviceTabs creates the XML for the tabs for the Device
     * details page.
     *
     * @param deviceId PK of the Device whose tabs you are
     * building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getDeviceTabs (String deviceId) throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            Device device =  mDeviceHome.findByPrimaryKey( new Integer ( deviceId ) );

            Element attributes = getDeviceAttributes(device);
            documentRootElement.addContent( attributes );

            // find and all all ref properties content to the XML, we use it
            // to build the input fields dynamically
            ArrayList rows =
                    executePreparedQuery (  "SELECT code, id, name, content " +
                                            "FROM REF_PROPERTIES " +
                                            "WHERE profile_type = ? ",
                                            new Object [] { new Integer ( PDSDefinitions.PROF_TYPE_PHONE ) },
                                            4,
                                            10000 );

            Element refPropertiesElement = createRefPropertyElements(rows);
            documentRootElement.addContent( refPropertiesElement );

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );


            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( device.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            rcsElement.addContent( new Element ( "name" ).setText ( rcs.getName() ) );

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch ( Exception e) {
            mCTX.setRollbackOnly();

            throw new PDSException ( e.getMessage() );
        }

        return documentRootElement;
    }


    /**
     * getDeviceTabs creates the XML for the tabs for the User Group
     * details page.
     *
     * @param userGroupId PK of the UserGroup whose tabs you are
     * building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getUserGroupTabs (String userGroupId) throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            UserGroup userGroup = mUserGroupHome.findByPrimaryKey(new Integer(userGroupId));
            Element attributes = createUserGroupAttributes(userGroup);
            attributes.addContent( new Element ( "refconfigsetid").setText( userGroup.getRefConfigSetID().toString()));
            documentRootElement.addContent( attributes );

            ArrayList rows =
                    executePreparedQuery (  "SELECT code " +
                                            "FROM REF_PROPERTIES",
                                            null,
                                            1,
                                            10000 );

            Element refPropertiesElement = createRefPropertyTabElements(rows);
            documentRootElement.addContent(refPropertiesElement);

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );

            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( userGroup.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            rcsElement.addContent( new Element ("name").setText( rcs.getName()));

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch (RemoteException e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }
        catch ( Exception e) {
            throw new PDSException ( e.getMessage() );
        }

        return documentRootElement;
    }


    /**
     * getDeviceGroupTabs creates the XML for the tabs for the Device
     * Group details page.
     *
     * @param deviceGroupId PK of the DeviceGroup whose tabs you are
     * building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getDeviceGroupTabs (String deviceGroupId) throws PDSException {

        Element documentRootElement = new Element("details");

        try {
            DeviceGroup deviceGroup =
                mDeviceGroupHome.findByPrimaryKey( new Integer(deviceGroupId) );

            Element attributes = new Element( "attributes");
            attributes.addContent(new Element("id").setText(deviceGroup.getID().toString()));
            attributes.addContent(new Element("name").setText(deviceGroup.getName()));
            documentRootElement.addContent( attributes );

            ArrayList rows =
                    executePreparedQuery (  "SELECT code " +
                                            "FROM REF_PROPERTIES",
                                            null,
                                            1,
                                            10000 );

            Element refPropertiesElement = createRefPropertyTabElements(rows);
            documentRootElement.addContent(refPropertiesElement);

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );
            documentRootElement.addContent( mappingElement );

            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( deviceGroup.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            rcsElement.addContent( new Element ("name").setText( rcs.getName()));
            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =  mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException (e.getMessage());
        }
        catch (Exception e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage()) ;
        }

        return documentRootElement;
    }


    /**
     * getAvailableDevicesList is used by the AvailDevicesTag tag handler to
     * give a list of all of the devices which don't have an owner.
     *
     * @return an html String (option/select)
     */
    public String getAvailableDevicesList () {

        TreeMap availableDevicesMap = new TreeMap();
        StringBuffer htmlString = new StringBuffer ();

        try {
            ArrayList rows = executePreparedQuery ( "SELECT short_name, id " +
                                                    "FROM logical_phones " +
                                                    "WHERE usrs_id IS NULL",
                                                    null,
                                                    2,
                                                    1000000 );

            for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                ArrayList row = (ArrayList) iRow.next();

                availableDevicesMap.put( row.get( 0), row.get( 1 ) );
            }

            htmlString.append ( "<select name=\"deviceid\"" ) ;
            htmlString.append ( "\">" ) ;
            htmlString.append( "<option>Select Device to Assign" );

            for ( Iterator mapIterator = availableDevicesMap.entrySet().iterator(); mapIterator.hasNext(); ) {
                Map.Entry me = ( Map.Entry )mapIterator.next();
                htmlString.append( "<option value=\"" +
                                    me.getValue() + "\">" +
                                    me.getKey() +
                                    "\n");
            }

            htmlString.append ( "</select>" ) ;

            return htmlString.toString();
        }
        catch (SQLException e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }
    }


    /**
     * returns XML containing all of the user groups and the users within
     * the group whose ID is passed in as a parameter.
     *
     * @param groupID the PK of the group whose members should be listed.
     * May be null
     * @return JDOM root Element
     * @throws PDSException for application type errors.
     */
    public Element listUsers ( Integer groupID ) throws PDSException {

        Element root = new Element ( "groups" );
        try {
            Collection all = null;

            all = mUserGroupHome.findAll();

            for ( Iterator iGroup = all.iterator(); iGroup.hasNext(); ) {
                UserGroup ug = (UserGroup) iGroup.next();
                if ( ug.getParentID() == null ) {
                    buildGroups ( ug, root, groupID );
                }
            }
        }
        catch (Exception e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }

        return root;
    }


    /**
     * listAllUsers returns the xml markup for all of the User Groups and
     * Users in those groups.
     *
     * @return Element containing all the User and UserGroups in the
     * database.
     * @throws PDSException is thrown for all application errors.
     */
    public Element listAllUsers() throws PDSException {
        Element root = new Element ( "groups" );
        try {
            Collection all = mUserGroupHome.findAll();

            for ( Iterator iGroup = all.iterator(); iGroup.hasNext(); ) {
                UserGroup ug = (UserGroup) iGroup.next();
                if ( ug.getParentID() == null ) {
                    buildGroups (ug, root);
                }
            }
        }
        catch (Exception e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }

        return root;
    }

    /**
     * produces XML markup for all groups and the devices within the given
     * device group ID (if present).
     *
     * @param deviceGroupID
     * @return
     * @throws PDSException
     */
    public Element listDevices ( Integer deviceGroupID ) throws PDSException {
        Element root = new Element ( "groups" );

        try {
            Collection all = mDeviceGroupHome.findAll();
            for ( Iterator iGroup = all.iterator(); iGroup.hasNext(); ) {
                DeviceGroup dg = (DeviceGroup) iGroup.next();
                if ( dg.getParentID() == null ) {
                    buildGroups ( dg, root, deviceGroupID );
                }
            }
        }
        catch (Exception e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }

        return root;
    }


    /**
     * this tag handler implementation is called prior to allocating extensions
     * to an extension pool.  It lets you know if the extensions that you are
     * trying to add already exist in a pool.   In the UI the user will then
     * have to confirm that they want to add the extensions 'around' the existing
     * ones.
     * @param minExtension
     * @param maxExtension
     * @param extensionPoolID PK of extension Pool that you want to add the
     * extensions to
     * @return true if and extension in the extension range given between
     * minExtension and maxExtesnion exists in any existing pool.  false is
     * returned in all other cases.
     */
    public boolean extensionInExistingPool (    String minExtension,
                                                String maxExtension,
                                                String extensionPoolID ) {

        boolean returnValue = false;

        int min;
        int max;
        try {
            try {
                min = new Integer ( minExtension ).intValue();
            }
            catch (NumberFormatException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC623",
                                            "E5020",
                                            new Object[]{ minExtension } ),
                    e );
            }

            try {
                max = new Integer ( maxExtension ).intValue();
            }
            catch (NumberFormatException e) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC623",
                                            "E5020",
                                            new Object[]{ maxExtension } ),
                    e );
            }

            for ( int counter = min; counter <= max; ++counter) {

                // This checks to see that the extension isn't owned by an existing
                // extension pool.
                ArrayList rows = executePreparedQuery ( "SELECT EXTENSION_NUMBER " +
                                                        "FROM EXTENSIONS " +
                                                        "WHERE EXTENSION_NUMBER = ? ",
                                                        new Object [] { String.valueOf ( counter ) },
                                                        1,
                                                        1 );

                if ( !rows.isEmpty() ) {
                    returnValue = true;
                }
            }
        }

        catch ( Exception e) {
            logFatal ( e.toString(), e );

            throw new EJBException(
                collateErrorMessages(   "UC623",
                                        "E5021",
                                        new Object[]{ extensionPoolID } )
            );
        }

        return returnValue;
    }


    /**
     * Returns XML markup to be run through an xslt stylesheet to give
     * details for a given user group.   Eventually this produces the User Group
     * details tabbed page.
     * @param userGroup group whose detail you want
     * @return JDOM Element containing XML markup
     * @throws PDSException for application errors
     */
    public Element getUserGroupDetails ( UserGroup userGroup ) throws PDSException {
        Element documentRootElement = new Element("details");

        try {
            Element attributes = createUserGroupAttributes(userGroup);

            Integer parentGroupID = userGroup.getParentID();
            Element parentIDElement = new Element ( "parentid");
            attributes.addContent (parentIDElement);

            if ( parentGroupID != null ) {
                parentIDElement.setText( parentGroupID.toString());
            }

            attributes.addContent( new Element ( "refconfigsetid").setText( userGroup.getRefConfigSetID().toString()));

            if ( parentGroupID != null ) {
                UserGroup parentGroup =
                    mUserGroupHome.findByPrimaryKey( parentGroupID );

                Element parentName = new Element ( "parentname" );
                parentName.setText( parentGroup.getName() );
                attributes.addContent( parentName );
            }
            else {
                Element parentName = new Element ( "parentname" );
                attributes.addContent( parentName );
            }

//            ArrayList workingSet = new ArrayList();
//            Collection inelligable =
//                getInelligableParentGroups ( userGroup, workingSet );
//            Element possibleGroups = new Element ( "possible_groups" );
//            attributes.addContent( possibleGroups );
//
//            // Need to add parent org's groups at a later date.
//            Collection allGroups =
//                mUserGroupHome.findByOrganizationID( userGroup.getOrganizationID() );
//
//            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
//                UserGroup possGroup = (UserGroup) allGroupsI.next();
//
//                if (    !inelligable.contains( possGroup ) &&
//                        !possGroup.getID().equals( userGroup.getID() ) ) {
//
//                    Element possibleGroupElem = new Element ( "group" );
//                    possibleGroups.addContent( possibleGroupElem );
//                    possibleGroupElem.addContent(
//                        new Element ( "id" ).setText( possGroup.getID().toString() ) );
//                    possibleGroupElem.addContent(
//                        new Element ( "name" ).setText( possGroup.getName() ) );
//                }
//            }

            documentRootElement.addContent( attributes );

            Element csElement = new Element ( "configurationset" );
            documentRootElement.addContent( csElement );

            Collection cCS = mConfigSetHome.findByUserGroupID( userGroup.getID() );
            for ( Iterator iCS = cCS.iterator(); iCS.hasNext(); ) {
                ConfigurationSet cs = (ConfigurationSet) iCS.next();
                csElement.addContent( cs.getContent() );
            }


            LinkedList allDeviceTypes = new LinkedList ();
            Collection cAllDeviceTypes = mDeviceTypeHome.findAll();

            for ( Iterator iAll = cAllDeviceTypes.iterator(); iAll.hasNext(); ) {
                DeviceType deviceType = (DeviceType) iAll.next();
                allDeviceTypes.add( deviceType );
            }

            LinkedList lightweightDeviceTypes = new LinkedList();

            for (   Iterator iLightweight = allDeviceTypes.iterator();
                    iLightweight.hasNext(); ) {

                DeviceType deviceType = (DeviceType) iLightweight.next();
                Manufacturer manu =
                    mManufacturerHome.findByPrimaryKey( deviceType.getManufacturerID() );

                lightweightDeviceTypes.add(
                    new LightweightDeviceType ( manu.getName(),
                                                deviceType.getModel() ) );
            }

            Collections.sort(   lightweightDeviceTypes,
                                new LightweightDeviceTypeComparator () );

            Element modelsElement = new Element ( "models" );
            documentRootElement.addContent( modelsElement );

            for (   Iterator iLightweight = lightweightDeviceTypes.iterator();
                    iLightweight.hasNext(); ) {

                LightweightDeviceType ldt =
                    (LightweightDeviceType) iLightweight.next();
                Element modelElement = new Element ( "model" );
                modelElement.addContent( ldt.getModel() );
                modelsElement.addContent( modelElement );
            }

            ArrayList rows =
                    executePreparedQuery (  "SELECT code, id, name, content " +
                                            "FROM REF_PROPERTIES",
                                            null,
                                            4,
                                            10000 );

            Element refPropertiesElement = createRefPropertyElements(rows);
            documentRootElement.addContent( refPropertiesElement );

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            mappingElement.addContent( PropertyGroupLink.getInstance().getMappings() );

            RefConfigurationSet rcs =
                mRefConfigSetHome.findByPrimaryKey( userGroup.getRefConfigSetID() );

            Element rcsElement = new Element ( "refconfigurationset" );
            rcsElement.addContent( new Element ("name").setText( rcs.getName()));
            rcsElement.addContent( new Element ("id").setText ( rcs.getID().toString()));
            rcsElement.addContent( new Element ("organizationid").setText( rcs.getOrganizationID().toString()));
            rcsElement.addContent( new Element ("externalid").setText( rcs.getExternalID()));

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                mRefPropertyHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }
        }
        catch (RemoteException e) {
            logFatal ( e.getMessage(), e );
            throw new EJBException ( e.getMessage() );
        }
        catch ( Exception e) {
            throw new PDSException ( e.getMessage() );
        }

        return documentRootElement;
    }


    /**
     * getUserGroupTree returns XML markup containing all of the User Groups
     * in the Configuration server's database.
     *
     * @return xml markup to be run though an XSLT stylesheet.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getUserGroupTree() throws PDSException {

        Element groups = new Element ( "groups" );
        ArrayList emptyCollection = new ArrayList(); // intentially empty, all groups valid

        try {
            Collection allGroups =  mUserGroupHome.findAll();

            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
                UserGroup possGroup = (UserGroup) allGroupsI.next();
                if (possGroup.getParentID() == null) {
                    Element groupContent = getUserGroupElement(possGroup, emptyCollection);
                    groups.addContent(groupContent);
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


    /**
     * getUserGroupTree returns XML markup containing all of the User Groups
     * who are valid parent groups for a given UserGroup.   Valid groups
     * are any groups who are not the User Group passed in the parameter
     * or any of that groups children (that would cause a circular reference).
     *
     * @param userGroupId PK of the User Group whose elligable parents you
     * want.
     * @return xml markup to be run though an XSLT stylesheet.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getUserGroupTree (Integer userGroupId) throws PDSException {

        Element groups = new Element ( "groups" );

        try {
            UserGroup userGroup = getUserGroupEJBObject(userGroupId);

            ArrayList workingSet = new ArrayList();
            Collection inelligable =
                    getInelligableParentGroups ( userGroup, workingSet, new HashSet() );

            Collection allGroups =
                mUserGroupHome.findByOrganizationID( userGroup.getOrganizationID() );

            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
                UserGroup possGroup = (UserGroup) allGroupsI.next();

                if (possGroup.getParentID() == null && !possGroup.getID().equals(userGroupId)) {

                    Element groupContent = getUserGroupElement(possGroup, inelligable);
                    groups.addContent(groupContent);
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


    /**
     * getDeviceGroupTree returns XML markup containing all of the Device Groups
     * in the Configuration server's database.
     *
     * @return xml markup to be run though an XSLT stylesheet.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getDeviceGroupTree() throws PDSException {

        Element groups = new Element ( "groups" );
        ArrayList emptyCollection = new ArrayList(); // intentially empty, all groups valid

        try {
            Collection allGroups =  mDeviceGroupHome.findAll();

            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
                DeviceGroup possGroup = (DeviceGroup) allGroupsI.next();
                if (possGroup.getParentID() == null) {
                    Element groupContent = getDeviceGroupElement(possGroup, emptyCollection);
                    groups.addContent(groupContent);
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


    /**
     * getDeviceGroupTree returns XML markup containing all of the DeviceGroups
     * who are valid parent groups for a given DeviceGroup.   Valid groups
     * are any groups who are not the User Group passed in the parameter
     * or any of that groups children (that would cause a circular reference).
     *
     * @param deviceGroupId PK of the Device Group whose elligable parents you
     * want.
     * @return xml markup to be run though an XSLT stylesheet.
     * @throws PDSException is thrown for all application errors.
     */
    public Element getDeviceGroupTree (Integer deviceGroupId) throws PDSException {

        Element groups = new Element ( "groups" );

        try {
            DeviceGroup deviceGroup = mDeviceGroupHome.findByPrimaryKey(deviceGroupId);

            ArrayList workingSet = new ArrayList();
            Collection inelligable =
                    getInelligableParentGroups (deviceGroup, workingSet, new HashSet());

            Collection allGroups =
                mDeviceGroupHome.findByOrganizationID(deviceGroup.getOrganizationID());

            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
                DeviceGroup possGroup = (DeviceGroup) allGroupsI.next();

                if (possGroup.getParentID() == null && !possGroup.getID().equals(deviceGroupId)) {
                    Element groupContent = getDeviceGroupElement(possGroup, inelligable);
                    groups.addContent(groupContent);
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private Element getUserGroupElement (UserGroup userGroup, Collection invalidGroups)
            throws PDSException, RemoteException {

        Element groups = createUserGroupTreeElement(userGroup);

        try {
            Collection children = mUserGroupHome.findByParentID(userGroup.getID());

            for ( Iterator allGroupsI = children.iterator(); allGroupsI.hasNext(); ) {
                UserGroup child = (UserGroup) allGroupsI.next();
                if (!invalidGroups.contains(child)) {
                    groups.addContent( getUserGroupElement(child, invalidGroups) );
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


    private Element createUserGroupTreeElement (UserGroup userGroup) throws RemoteException {
        Element group = new Element ( "group" );
        group.addContent(
            new Element ( "id" ).setText( userGroup.getID().toString() ) );
        group.addContent(
            new Element ( "name" ).setText( userGroup.getName() ) );

        return group;
    }



    private Element getDeviceGroupElement (DeviceGroup deviceGroup, Collection invalidGroups)
                throws PDSException, RemoteException {

        Element groups = createDeviceGroupTreeElement(deviceGroup);

        try {
            Collection children = mDeviceGroupHome.findByParentID(deviceGroup.getID());

            for ( Iterator allGroupsI = children.iterator(); allGroupsI.hasNext(); ) {
                DeviceGroup child = (DeviceGroup) allGroupsI.next();
                if (!invalidGroups.contains(child)) {
                    groups.addContent( getDeviceGroupElement(child, invalidGroups) );
                }
            }
        }
        catch (RemoteException e) {
            logFatal(e.getMessage(), e);

            throw new EJBException(e.getMessage());
        }
        catch (FinderException e) {
            mCTX.setRollbackOnly();

            throw new PDSException(e.getMessage());
        }

        return groups;
    }


    private Element createDeviceGroupTreeElement (DeviceGroup deviceGroup) throws RemoteException {
        Element group = new Element ( "group" );
        group.addContent(
            new Element ( "id" ).setText( deviceGroup.getID().toString() ) );
        group.addContent(
            new Element ( "name" ).setText( deviceGroup.getName() ) );

        return group;
    }



    private Element getDeviceAttributes(Device device) throws RemoteException {

        Element attributes = new Element("attributes");
        Element des = new Element("description");
        des.setText(device.getDescription());
        attributes.addContent( des );
        Element id = new Element("id");
        id.setText(device.getID().toString());
        attributes.addContent( id );
        Element org = new Element("organizationid");
        org.setText(device.getOrganizationID().toString());
        attributes.addContent( org );
        Element dev = new Element("devicetypeid");
        dev.setText(device.getDeviceTypeID().toString());
        attributes.addContent( dev );
        Element exId = new Element("externalid");
        exId.setText(device.getExternalID());
        attributes.addContent( exId );
        Element core = new Element("coresoftwaredetailsid");
        core.setText(device.getCoreSoftwareDetailsID().toString());
        attributes.addContent( core );
        Element rci = new Element("refconfigsetid");
        rci.setText(device.getRefConfigSetID().toString());
        attributes.addContent( rci );
        Element sn = new Element("shortname");
        sn.setText(device.getShortName());
        attributes.addContent( sn );
        Element uid = new Element("userid");
        uid.setText(device.getUserID());
        attributes.addContent( uid );
        Element dgid = new Element("devicegroupid");
        dgid.setText(device.getDeviceGroupID().toString());
        attributes.addContent( dgid );
        Element ser = new Element("serialnumber");
        ser.setText(device.getSerialNumber());
        attributes.addContent( ser );
        return attributes;
    }

    private Element createUserGroupAttributes(UserGroup userGroup)
            throws RemoteException, PDSException {

        Element attributes = new Element ( "attributes" );
        attributes.addContent( new Element ( "name").setText( userGroup.getName()));
        attributes.addContent( new Element ( "id").setText( userGroup.getID().toString()));
        attributes.addContent( new Element ( "organizationid").setText( userGroup.getOrganizationID().toString()));
        attributes.addContent( new Element ( "externalid").setText( userGroup.getExternalID()));
        return attributes;
    }


    private Element getModelsElement() throws FinderException, RemoteException {
        Element modelsElement = new Element ( "models" );
        LinkedList lightweightDeviceTypes = new LinkedList();
        Collection cAllDeviceTypes = mDeviceTypeHome.findAll();

        for ( Iterator iAll = cAllDeviceTypes.iterator(); iAll.hasNext(); ) {
            DeviceType deviceType = (DeviceType) iAll.next();

            Manufacturer manu =
                getManufacturerEJBObject ( deviceType.getManufacturerID() );

            lightweightDeviceTypes.add(
                new LightweightDeviceType ( manu.getName(),
                                            deviceType.getModel() ) );
        }

        Collections.sort(   lightweightDeviceTypes,
                            new LightweightDeviceTypeComparator () );


        for (   Iterator iLightweight = lightweightDeviceTypes.iterator();
                iLightweight.hasNext(); ) {

            LightweightDeviceType ldt =
                (LightweightDeviceType) iLightweight.next();
            Element modelElement = new Element ( "model" );
            modelElement.addContent( ldt.getModel() );
            modelsElement.addContent( modelElement );
        }
        return modelsElement;
    }



    private Collection getInelligableParentGroups (UserGroup userGroup, Collection workingSet, Set ids)
            throws RemoteException, FinderException {

        workingSet.add(userGroup);
        Collection children = mUserGroupHome.findByParentID( userGroup.getID() );

        for ( Iterator i = children.iterator(); i.hasNext(); ) {
            UserGroup child = (UserGroup) i.next();
            if (ids.contains(child.getID()))
            {
                logFatal("Bogus DB data. Child is lists one of it's ancestors as a " +
                    "desendant. Incestuous child = " + child.getName(), 
                    new Exception("Recursion"));
            } 
            else
            {
                ids.add(child.getID());
                workingSet.add( child );
                getInelligableParentGroups ( child, workingSet, ids );
            }
        }

        return workingSet;
    }


    private Collection getInelligableParentGroups(DeviceGroup deviceGroup, Collection workingSet, Set ids)
            throws RemoteException, FinderException {

        workingSet.add(deviceGroup);
        Collection children = mDeviceGroupHome.findByParentID( deviceGroup.getID() );

        for (Iterator i = children.iterator(); i.hasNext(); ) {
            DeviceGroup child = (DeviceGroup) i.next();
            if (ids.contains(child.getID()))
            {
                logFatal("Bogus DB data. Child is lists one of it's ancestors as a " +
                    "desendant. Incestuous child = " + child.getName(), 
                    new Exception("Recursion"));
            } 
            else
            {
                ids.add(child.getID());
                workingSet.add(child);
                getInelligableParentGroups(child, workingSet, ids);
            }
        }

        return workingSet;
    }


    private Element createUserAttributes(User user)
            throws RemoteException, PDSException {

        Element attributes = new Element ("attributes");

        String id = user.getID();
        if(id != null) {
            Element idn = new Element ("id");
            idn.addContent(new CDATA(id));
            attributes.addContent( idn );
        } else {
            attributes.addContent(new Element("id"));
        }

        String displayId = user.getDisplayID();
        if(displayId != null) {
            Element did = new Element ("displayid");
            did.addContent(new CDATA(displayId));
            attributes.addContent( did );
        } else {
            attributes.addContent(new Element ("displayid"));
        }

        String firstName = user.getFirstName();
        if(firstName != null) {
            Element fn = new Element ("firstname");
            fn.addContent(new CDATA(firstName));
            attributes.addContent(fn);
        } else {
            attributes.addContent(new Element ("firstname"));
        }

        String lastName = user.getLastName();
        if (lastName != null) {
            Element ln = new Element ("lastname");
            ln.addContent( new CDATA(lastName));
            attributes.addContent( ln );
        } else {
            attributes.addContent(new Element ("lastname"));
        }

        String userGroupId = user.getUserGroupID().toString();
        if (userGroupId != null) {
            Element gid = new Element ("usergroupid");
            gid.addContent(new CDATA(userGroupId));
            attributes.addContent( gid );
        } else {
            attributes.addContent(new Element ("usergroupid"));
        }

        String extension = user.getExtension();
        if (extension != null) {
            Element ex = new Element ("extension");
            ex.addContent(new CDATA(user.getExtension()));
            attributes.addContent(ex);
        } else {
            attributes.addContent(new Element ("extension"));
        }

        return attributes;
    }

    private Element getUsersOrganizationDetails(Integer usersOrganizationID)
            throws FinderException, RemoteException {

        Element organization = new Element ( "organization" );
        Organization usersOrganization = mOrganizationHome.findByPrimaryKey( usersOrganizationID );
        organization.addContent ( new Element ( "id").setText( usersOrganizationID.toString() ) );
        organization.addContent ( new Element ( "dnsdomain").setText( usersOrganization.getDNSDomain() ) );
        return organization;
    }



    private Element getUserAliases(User user) throws RemoteException {
        Element aliases = new Element ( "aliases" );

        String userAliases = user.getAliases();
        StringTokenizer aliasTok = new StringTokenizer ( userAliases, "," );

        while ( aliasTok.hasMoreTokens() ) {
            Element alias = new Element ( "alias" ).setText( aliasTok.nextToken() );
            aliases.addContent( alias );
        }
        return aliases;
    }



    private Element createRefPropertyElements(ArrayList rows) {

        Element refPropertiesElement = new Element ( "refproperties" );

        for ( Iterator rowsI = rows.iterator(); rowsI.hasNext(); ) {
            ArrayList row = (ArrayList) rowsI.next();
            Element rpElement = new Element ( "refproperty" );
            refPropertiesElement.addContent( rpElement );

            Element code = new Element ( "code");
            code.addContent( (String) row.get( 0 ) );
            rpElement.addContent( code );
            Element id = new Element ( "id" );
            id.addContent( (String) row.get( 1 ) );
            rpElement.addContent( id );
            Element name = new Element ( "name" );
            name.addContent( (String) row.get( 2 ) ) ;
            rpElement.addContent( name );
            Element cont = new Element ( "content" );
            cont.addContent( (String) row.get( 3 ) );
            rpElement.addContent( cont );
        }

        return refPropertiesElement;
    }


    private Element createRefPropertyTabElements(ArrayList rows) {

        Element refPropertiesElement = new Element ( "refproperties" );

        for ( Iterator rowsI = rows.iterator(); rowsI.hasNext(); ) {
            ArrayList row = (ArrayList) rowsI.next();
            Element rpElement = new Element ( "refproperty" );
            refPropertiesElement.addContent( rpElement );
            Element code = new Element ( "code");
            code.addContent( (String) row.get( 0 ) );
            rpElement.addContent( code );
        }

        return refPropertiesElement;
    }


    private User getUserEJBObject ( String userID )
        throws FinderException, RemoteException{

        if ( !mUserEJBObjectCache.containsKey( userID ) ) {
            User user = mUserHome.findByPrimaryKey( userID );
            mUserEJBObjectCache.put( userID, user );
        }

        return (User) mUserEJBObjectCache.get( userID );
    }



    private UserGroup getUserGroupEJBObject ( Integer userGroupID )
        throws FinderException, RemoteException{

        if ( !mUserGroupEJBObjectCache.containsKey( userGroupID ) ) {
            UserGroup userGroup =
                mUserGroupHome.findByPrimaryKey( userGroupID );

            mUserGroupEJBObjectCache.put( userGroupID, userGroup );
        }

        return (UserGroup) mUserGroupEJBObjectCache.get( userGroupID );
    }


    private ConfigurationSet getUsersConfigSetEJBObject ( String userID )
        throws FinderException, RemoteException {

        if ( !mUserConfigSetEJBObjectCache.containsKey( userID ) ) {
            Collection cCS = mConfigSetHome.findByUserID( userID );
            for ( Iterator iCS = cCS.iterator(); iCS.hasNext(); ) {
                ConfigurationSet cs = (ConfigurationSet) iCS.next();
                mUserConfigSetEJBObjectCache.put( userID, cs );
            }
        }

        return (ConfigurationSet) mUserConfigSetEJBObjectCache.get( userID );
    }


    private ConfigurationSet getDevicesConfigSetEJBObject ( Integer deviceID )
        throws FinderException, RemoteException {

        if ( !mDeviceConfigSetEJBObjectCache.containsKey( deviceID ) ) {
            Collection cCS = mConfigSetHome.findByLogicalPhoneID( deviceID );
            for ( Iterator iCS = cCS.iterator(); iCS.hasNext(); ) {
                ConfigurationSet cs = (ConfigurationSet) iCS.next();
                mDeviceConfigSetEJBObjectCache.put( deviceID, cs );
            }
        }

        return (ConfigurationSet) mDeviceConfigSetEJBObjectCache.get( deviceID );
    }


    private Manufacturer getManufacturerEJBObject ( Integer manufacturerID )
        throws FinderException, RemoteException{

        if ( !mManufacturerEJBObjectCache.containsKey( manufacturerID ) ) {
            Manufacturer manufacturer = mManufacturerHome.findByPrimaryKey( manufacturerID );
            mManufacturerEJBObjectCache.put( manufacturerID, manufacturer );
        }

        return (Manufacturer) mManufacturerEJBObjectCache.get( manufacturerID );
    }


    private Element buildGroups ( UserGroup userGroup, Element node, Integer groupID )
        throws  RemoteException, FinderException, ElementUtilException,
            PostProcessingException, PDSException, SQLException {

        Element groupElement = buildUserGroupElement(userGroup);
        node.addContent( groupElement );

        if ( groupID != null && groupID.equals( userGroup.getID() ) ) {
            Map deviceMap = retrieveUserGroupsDevices ( userGroup.getID() );
            createMemberElements ( groupElement, userGroup.getID(), deviceMap );
        }

        Collection childGroups =
            mUserGroupHome.findByParentID( userGroup.getID() );

        for ( Iterator iGroup = childGroups.iterator(); iGroup.hasNext(); ) {
            UserGroup ug = (UserGroup) iGroup.next();
            buildGroups (ug, groupElement, groupID);
        }

        return node;
    }


    private Element buildGroups (UserGroup userGroup, Element node)
        throws  RemoteException, FinderException, ElementUtilException,
            PostProcessingException, PDSException, SQLException {

        Element groupElement = buildUserGroupElement(userGroup);

        node.addContent( groupElement );

        Map deviceMap = retrieveUserGroupsDevices ( userGroup.getID() );
        createMemberElements ( groupElement, userGroup.getID(), deviceMap );

        Collection childGroups =
            mUserGroupHome.findByParentID( userGroup.getID() );

        for ( Iterator iGroup = childGroups.iterator(); iGroup.hasNext(); ) {
            UserGroup ug = (UserGroup) iGroup.next();
            buildGroups (ug, groupElement);
        }

        return node;
    }


    private Element buildUserGroupElement(UserGroup userGroup) throws RemoteException {
        Element groupElement = new Element ( "group" );

        groupElement.addContent ( new Element ( "name" ).setText( userGroup.getName() ) );
        groupElement.addContent( new Element ( "id" ).setText( userGroup.getID().toString( ) ) );
        groupElement.addContent( new Element ( "organizationid").setText( userGroup.getOrganizationID().toString() ) );

        Element parentElement = new Element ( "parentid");
        if ( userGroup.getParentID() != null )
            parentElement.setText( userGroup.getParentID().toString() );

        groupElement.addContent( parentElement );
        return groupElement;
    }


    private void createMemberElements ( Element container, Integer userGroupID, Map deviceMap )
            throws PostProcessingException, SQLException {

        ArrayList rows = executePreparedQuery ("SELECT ID, DISPLAY_ID, EXTENSION FROM USERS WHERE UG_ID = ?",
                new Object[]{userGroupID}, 3, 1000000);

        ArrayList row = null;

        for (Iterator rowI = rows.iterator(); rowI.hasNext(); ) {
            row = (ArrayList) rowI.next();

            if ( !row.get(1).equals( "SDS" ) ) {

                Element nextElement = new Element ( "member" );
                String userID = (String)row.get(0);
                if (userID != null){
                    Element id = new Element ("id");
                    id.addContent(new CDATA(userID));
                    nextElement.addContent( id );
                } else {
                    nextElement.addContent(new Element ("id"));
                }

                String displayId = (String)row.get(1);
                if(displayId != null) {
                    Element did = new Element("displayid");
                    did.addContent(new CDATA(displayId));
                    nextElement.addContent( did );
                } else {
                    nextElement.addContent(new Element("displayid"));
                }

                String extension = (String)row.get(2);
                if (extension != null){
                    Element ext = new Element("extension");
                    ext.addContent(new CDATA(extension));
                    nextElement.addContent( ext );
                } else {
                    nextElement.addContent(new Element("extension"));
                }

                Element postElement = addUsersDevices ( nextElement, userID , deviceMap );

                container.addContent( postElement );
            }
        }
    }



    private Map retrieveUserGroupsDevices ( Integer userGroupID ) throws SQLException {

        HashMap map = new HashMap();

        ArrayList rows =
                executePreparedQuery (  "SELECT u.id, lp.id, lp.short_name, pt.model, manu.name " +
                                        "FROM logical_phones lp, users u, phone_types pt, manufacturers manu " +
                                        "WHERE lp.usrs_id = u.id " +
                                        "AND lp.pt_id = pt.id " +
                                        "AND manu.id = pt.manu_id " +
                                        "AND u.ug_id = ?",
                                        new Object [] { userGroupID },
                                        5,
                                        100000 );

        for ( Iterator iRows = rows.iterator(); iRows.hasNext(); ) {
            ArrayList row = (ArrayList) iRows.next();

            String userID = (String) row.get( 0 );
            String deviceID = (String) row.get( 1 );
            String shortName = (String) row.get( 2 );
            String model = (String) row.get( 3 );
            String manufacturer = (String) row.get( 4 );

            ArrayList existingDevices = null;

            if ( map.containsKey( userID ) ) {
                existingDevices = (ArrayList) map.get( userID );
            }
            else {
                existingDevices = new ArrayList();
            }

            existingDevices.add( new String [] {deviceID, shortName, model, manufacturer} );

            if ( !map.containsKey( userID ) ) {
                map.put( userID, existingDevices );
            }
        }

        return map;
    }


    private Element addUsersDevices( Element inputElement, String userID, Map deviceMap )
        throws PostProcessingException {

        try {
            // Now the more challenging part is to list all the devices
            // associated with this user in a container
            if (deviceMap.containsKey(userID)) {
                ArrayList userDevices = (ArrayList) deviceMap.get( userID );

                Element devicesElement = new Element ("devices");
                inputElement.addContent( devicesElement );

                for ( Iterator iDevice = userDevices.iterator(); iDevice.hasNext(); ) {
                    String [] row = (String []) iDevice.next();

                    Element deviceElement = new Element ("device");
                    if (row[0] != null) {
                        Element id = new Element ("id");
                        id.addContent(new CDATA(row[0]));
                        deviceElement.addContent( id );
                    } else {
                        deviceElement.addContent(new Element ("id"));
                    }

                    if (row[1] != null){
                        Element sn = new Element ("shortname");
                        sn.addContent(new CDATA(row[1]));
                        deviceElement.addContent( sn );
                    } else {
                        deviceElement.addContent(new Element ("shortname"));
                    }

                    if (row[2] != null){
                        Element mod = new Element ("model");
                        mod.addContent(new CDATA(row[2]));
                        deviceElement.addContent( mod );
                    } else {
                        deviceElement.addContent(new Element ("model"));
                    }

                    if (row[3] != null){
                        Element man = new Element ("manufacturer");
                        man.addContent(new CDATA(row[3]));
                        deviceElement.addContent( man );
                    } else {
                        deviceElement.addContent(new Element ("manufacturer"));
                    }

                    // and add it to the devices element
                    devicesElement.addContent (deviceElement);
                }
            }

            return inputElement;
        }
        catch ( Exception ex ) {
            throw new PostProcessingException(ex.getMessage());
        }
    }


    private Element buildGroups ( DeviceGroup deviceGroup, Element node, Integer deviceGroupID )
            throws  RemoteException, FinderException, ElementUtilException,
                PostProcessingException, SQLException {


        Element groupElement = new Element ( "group" );
        Element name = new Element ( "name" );
        name.setText( deviceGroup.getName() );
        groupElement.addContent( name );
        Element id = new Element ( "id" );
        id.setText( deviceGroup.getID().toString() );
        groupElement.addContent( id );
        Element oid = new Element ( "organizationid" );
        oid.setText( deviceGroup.getOrganizationID().toString() );
        groupElement.addContent( oid );

        Integer parentID = deviceGroup.getParentID();

        Element parentIDElement = new Element ( "parentid" );
        groupElement.addContent ( parentIDElement );

        if ( parentID != null )
            parentIDElement.setText( parentID.toString() );

        node.addContent( groupElement );

        if ( deviceGroupID != null && deviceGroupID.equals( deviceGroup.getID() ) ) {
            createMemberElements ( groupElement, deviceGroup.getID() );
        }

        Collection childGroups =
            mDeviceGroupHome.findByParentID( deviceGroup.getID() );

        for ( Iterator iGroup = childGroups.iterator(); iGroup.hasNext(); ) {
            DeviceGroup dg = (DeviceGroup) iGroup.next();
            buildGroups (  dg, groupElement, deviceGroupID );
        }

        return node;
    }



    private void createMemberElements ( Element container, Integer deviceGroupID )
        throws SQLException {

        ArrayList rows = executePreparedQuery (
                "SELECT d.id, d.serial_number, d.org_id, d.short_name, d.usrs_id, u.display_id, p.model, m.name " +
                "FROM logical_phones d, users u, phone_types p, manufacturers m " +
                "WHERE d.usrs_id = u.id " +
                "AND d.pg_id = ? " +
                "AND d.pt_id = p.id " +
                "AND p.manu_id = m.id " +
                "UNION " +
                "SELECT d.id, d.serial_number, d.org_id, d.short_name, null, null, p.model, m.name " +
                "FROM logical_phones d, phone_types p, manufacturers m " +
                "WHERE d.usrs_id IS NULL " +
                "AND d.pg_id = ? " +
                "AND d.pt_id = p.id " +
                "AND p.manu_id = m.id  ",
                new Object [] { deviceGroupID, deviceGroupID },
                8,
                1000000 );

        for (Iterator iter = rows.iterator(); iter.hasNext(); ) {
            ArrayList row =  (ArrayList) iter.next();

            Element nextElement = new Element ( "member" );
            nextElement.addContent( new Element ( "id" ).setText( (String) row.get( 0 ) ) );
            nextElement.addContent( new Element ( "serialnumber" ).setText( (String) row.get( 1 ) ) );
            nextElement.addContent( new Element ( "organizationid" ).setText( (String) row.get( 2 ) ) );
            nextElement.addContent( new Element ( "shortname" ).setText( (String) row.get( 3 ) ) );

            if ( row.get( 4 ) != null && row.get( 4 ) != "" ){
                nextElement.addContent( new Element ( "userid" ).setText( (String) row.get( 4 ) ) );
                nextElement.addContent( new Element ( "userid_displayid" ).setText( (String) row.get( 5 ) ) );
            }

            nextElement.addContent( new Element ( "model" ).setText( (String) row.get( 6 ) ) );
            nextElement.addContent( new Element ( "manufacturer" ).setText( (String) row.get( 7 ) ) );

            container.addContent( nextElement );
        }
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
