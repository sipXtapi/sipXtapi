/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/jsptags/ejb/TagHandlerHelperBusiness.java#4 $
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

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.user.UserGroup;
import org.jdom.Element;

import java.rmi.RemoteException;


public interface TagHandlerHelperBusiness {

    /**
     * returns XML markup to be run through an xslt stylesheet to give
     * details for a given user.   Eventually this produces the User Details
     * tabbed page.
     *
     * @param userID primary key of the User entity for whom the details are
     * required
     * @return JDOM root Element for the "details"
     * @throws PDSException for all application level errors
     * @throws RemoteException for RMI remoting errors
     */
    public Element getUserDetails ( String userID )
        throws PDSException, RemoteException;

    /**
     * getAvailableDevicesList is used by the AvailDevicesTag tag handler to
     * give a list of all of the devices which don't have an owner.
     *
     * @return an html String (option/select)
     * @throws RemoteException for RMI remoting errors
     */
    String getAvailableDevicesList () throws RemoteException;

    /**
     * returns XML containing all of the user groups and the users within
     * the group whose ID is passed in as a parameter.
     *
     * @param groupID the PK of the group whose members should be listed.
     * May be null
     * @return JDOM root Element
     * @throws PDSException for application type errors.
     * @throws RemoteException for RMI remoting errors
     */
    public Element listUsers ( Integer groupID )
            throws PDSException, RemoteException;

    /**
     * produces XML markup for all groups and the devices within the given
     * device group ID (if present).
     *
     * @param deviceGroupID
     * @return
     * @throws PDSException
     * @throws RemoteException for RMI remoting errors
     */
    public Element listDevices ( Integer deviceGroupID )
            throws PDSException, RemoteException;

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
     * @throws RemoteException for RMI remoting errors
     */
    public boolean extensionInExistingPool (    String minExtension,
                                                String maxExtension,
                                                String extensionPoolID )
            throws RemoteException;

    /**
     * returns XML markup to be run through an xslt stylesheet to give
     * details for a given device.
     *
     * @param m_deviceID String representation of the PK for the device
     * to be viewd
     * @return JDOM Element containing the XML markup
     * @throws PDSException for all checked exceptions.
     * @throws RemoteException for RMI remoting errors
     */
    public Element getDeviceDetails ( String m_deviceID )
            throws PDSException, RemoteException;

    /**
     * Returns XML markup to be run through an xslt stylesheet to give
     * details for a given user group.   Eventually this produces the User Group
     * details tabbed page.
     * @param userGroup
     * @return JDOM Element containing XML markup
     * @throws PDSException for application errors
     * @throws RemoteException for RMI remoting errors
     */
    public Element getUserGroupDetails ( UserGroup userGroup )
            throws PDSException, RemoteException;

    /**
     * getDeviceGroupTabs creates the XML for the tabs for the Device
     * Group details page.
     *
     * @param deviceGroupId PK of the DeviceGroup whose tabs you are
     * building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     * @throws RemoteException for RMI remoting errors
     */
    Element getDeviceGroupTabs (String deviceGroupId) throws PDSException, RemoteException;

    /**
    * getDeviceTabs creates the XML for the tabs for the Device
    * details page.
    *
    * @param deviceId PK of the Device whose tabs you are
    * building.
    * @return XML content for the tabs to be run through XSLT.
    * @throws PDSException is thrown for all application errors.
    * @throws RemoteException for RMI remoting errors
    */
    Element getDeviceTabs (String deviceId) throws PDSException, RemoteException;

    /**
    * getDeviceTabs creates the XML for the tabs for the User Group
    * details page.
    *
    * @param userGroupId PK of the UserGroup whose tabs you are
    * building.
    * @return XML content for the tabs to be run through XSLT.
    * @throws PDSException is thrown for all application errors.
    * @throws RemoteException for RMI remoting errors
    */
    Element getUserGroupTabs (String userGroupId) throws PDSException, RemoteException;

    /**
     * getDeviceTabs creates the XML for the tabs for the User
     * details page.
     *
     * @param userID PK of the User whose tabs you are building.
     * @return XML content for the tabs to be run through XSLT.
     * @throws PDSException is thrown for all application errors.
     * @throws RemoteException for RMI remoting errors
     */
    Element getUserTabs (String userID) throws PDSException, RemoteException;

    Element getUserGroupTree (Integer userGroupId) throws PDSException, RemoteException;

    Element getUserGroupTree() throws PDSException, RemoteException;

    /**
     * getDeviceGroupTree returns XML markup containing all of the Device Groups
     * in the Configuration server's database.
     *
     * @return xml markup to be run though an XSLT stylesheet.
     * @throws PDSException is thrown for all application errors.
     */
    Element getDeviceGroupTree() throws PDSException, RemoteException;

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
    Element getDeviceGroupTree (Integer deviceGroupId) throws PDSException, RemoteException;

    /**
     * listAllUsers returns the xml markup for all of the User Groups and
     * Users in those groups.
     *
     * @return Element containing all the User and UserGroups in the
     * database.
     * @throws PDSException is thrown for all application errors.
     */
    Element listAllUsers() throws PDSException, RemoteException;

}