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

package com.pingtel.pds.pgs.soap;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.ErrorMessageBuilder;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;

import javax.ejb.FinderException;
import javax.naming.NamingException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.ResourceBundle;
import java.text.MessageFormat;

/**
 * SoapService is the parent class of all of PGS Axis service classes.
 */
abstract class SoapService {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String TREE_DELIM = "/";
    protected static final String NULL_STRING = "null";

//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static ErrorMessageBuilder mErrorMessageBuilder = new ErrorMessageBuilder("pgs-strings");
    protected static ResourceBundle mSoapStrings = ResourceBundle.getBundle("soap-strings");

    private UserGroupHome mUserGroupHome;
    private DeviceGroupHome mDeviceGroupHome;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    protected SoapService() {
        try {
            EJBHomeFactory.getInstance().init();
        }
        catch (NamingException e) {
            throw new RuntimeException (e.getMessage());
        }
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * checkMandatoryArgument checks to make sure that the value given is
     * not null or of zero length.
     *
     * @param argumentName the name of the parameter, this is used for error
     * handling only.
     * @param value the value to check.
     * @throws PDSException is thrown for all errors.
     */
    protected void checkMandatoryArgument (String argumentName, String value) throws PDSException {
        if (value == null || value.trim().length() == 0) {
            throw new PDSException(
                    MessageFormat.format(mSoapStrings.getString("E_MAN_ARG_MISSING"), new Object []{argumentName}));
        }
    }

    protected String collateErrorMessages ( String useCase,
                                            String errorMessageNumber,
                                            Object [] detailMsgSubstitutions) {

        return
            mErrorMessageBuilder.collateErrorMessages( useCase,
                                                                    errorMessageNumber,
                                                                    detailMsgSubstitutions );

    }

    protected String collateErrorMessages ( String errorMessageNumber,
                                            Object [] detailMsgSubstitutions) {
        return
            mErrorMessageBuilder.collateErrorMessages( errorMessageNumber,
                                                                    detailMsgSubstitutions );

    }

    protected String collateErrorMessages ( String useCase ) {
        return
            mErrorMessageBuilder.collateErrorMessages( useCase );
    }


    protected synchronized String getUserGroupFromTree(String tree) throws PDSException {
        String returnValue = null;

        try {
            if(mUserGroupHome == null) {
                mUserGroupHome = (UserGroupHome)
                        EJBHomeFactory.getInstance().getHomeInterface(UserGroupHome.class, "UserGroup");
            }

            UserGroup userGroup = null;
            boolean firstTime = true;
            StringTokenizer treeTokenizer = new StringTokenizer(tree, TREE_DELIM);

            while(treeTokenizer.hasMoreTokens()){
                String groupName = treeTokenizer.nextToken();

                if (firstTime) {
                    firstTime = false;
                    Collection c = mUserGroupHome.findByOrganizationIDAndName(new Integer("1"), groupName);
                    if (c.isEmpty()) {
                        throw new PDSException(collateErrorMessages("UC620", "E1019", new Object[]{tree}));
                    }

                    userGroup = (UserGroup) c.iterator().next();
                } else {
                    boolean foundChild = false;
                    Collection c = mUserGroupHome.findByParentID(userGroup.getID());

                    for(Iterator iChild = c.iterator(); iChild.hasNext(); ){
                        UserGroup child = (UserGroup) iChild.next();
                        if (child.getName().equalsIgnoreCase(groupName)){
                            foundChild = true;
                            userGroup = child;
                            break; // to continue while
                        }
                    }

                    if (!foundChild) {
                        throw new PDSException(collateErrorMessages("UC620", "E1019", new Object[]{tree}));
                    }
                }
            }  // while

            returnValue = userGroup.getID().toString();
        }
        catch (FinderException e) {
            throw new PDSException(collateErrorMessages("UC620", "E1019", new Object[]{tree}));
        }
        catch (RemoteException e) {
            throw new PDSException(collateErrorMessages("UC620", null), e);
        }
        catch (NamingException e) {
            throw new PDSException(collateErrorMessages("UC620", null), e);
        }

        return returnValue;
    }


    protected synchronized String getDeviceGroupFromTree(String tree) throws PDSException {
        String returnValue = null;

        try {
            if(mDeviceGroupHome == null) {
                mDeviceGroupHome = (DeviceGroupHome)
                        EJBHomeFactory.getInstance().getHomeInterface(DeviceGroupHome.class, "DeviceGroup");
            }

            DeviceGroup deviceGroup = null;
            boolean firstTime = true;
            StringTokenizer treeTokenizer = new StringTokenizer(tree, TREE_DELIM);

            while(treeTokenizer.hasMoreTokens()){
                String groupName = treeTokenizer.nextToken();

                if (firstTime) {
                    firstTime = false;
                    Collection c = mDeviceGroupHome.findByName(groupName);
                    if (c.isEmpty()) {
                        throw new PDSException(collateErrorMessages("UC430", "E1003", new Object[]{tree}));
                    }

                    deviceGroup = (DeviceGroup) c.iterator().next();
                } else {
                    boolean foundChild = false;
                    Collection c = mDeviceGroupHome.findByParentID(deviceGroup.getID());

                    for(Iterator iChild = c.iterator(); iChild.hasNext(); ){
                        DeviceGroup child = (DeviceGroup) iChild.next();
                        if (child.getName().equalsIgnoreCase(groupName)){
                            foundChild = true;
                            deviceGroup = child;
                            break; // to continue while
                        }
                    }

                    if (!foundChild) {
                        throw new PDSException(collateErrorMessages("UC430", "E1003", new Object[]{tree}));
                    }
                }
            }  // while

            returnValue = deviceGroup.getID().toString();
        }
        catch (FinderException e) {
            throw new PDSException(collateErrorMessages("UC430", "E1003", new Object[]{tree}));
        }
        catch (RemoteException e) {
            throw new PDSException(collateErrorMessages("UC430", "E4084", null), e);
        }
        catch (NamingException e) {
            throw new PDSException(collateErrorMessages("UC430", "E4084", null), e);
        }

        return returnValue;
    }

    protected boolean isEmptySoapValue(String newUserId) {
        return newUserId.length() == 0;
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
