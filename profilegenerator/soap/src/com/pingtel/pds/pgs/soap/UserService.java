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
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.profile.RefConfigurationSet;
import com.pingtel.pds.pgs.profile.RefConfigurationSetHome;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;

import com.pingtel.pds.pgs.profile.ConfigurationSet;

import org.jdom.output.XMLOutputter;
import org.jdom.output.Format;
import org.jdom.input.SAXBuilder;
import org.jdom.JDOMException;
import org.jdom.Element;
import org.jdom.Content;
import org.jdom.CDATA;
import org.jdom.Document;
import org.jdom.xpath.*;

import javax.ejb.FinderException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.text.MessageFormat;
import java.io.StringReader;



/**
 * UserService is an adapter class whose purpose is to
 * provide high, user friendly methods to do with Users.   It is called
 * from the Axis SOAP servlet.
 */
public class UserService extends SoapService {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    private UserAdvocate mUserAdvocateEJBObject;
    private UserHome mUserHome;
    private RefConfigurationSetHome mRefConfigurationSetHome;
    private DeviceHome mDeviceHome;

    private static String mAllUserRefConfigurationSetID;
    private SAXBuilder mSaxBuilder = new SAXBuilder();
    private ConfigurationSetHome mConfigurationSetHome;
    private XMLOutputter mXmlOutputter = new XMLOutputter();
    private static int s_uniqueLineIdCounter;
    

//////////////////////////////////////////////////////////////////////////
// Construction
////

    public UserService() {
        try {
            UserAdvocateHome userAdvocateHome =
                    (UserAdvocateHome) EJBHomeFactory.getInstance().getHomeInterface(UserAdvocateHome.class, "UserAdvocate");
            mUserAdvocateEJBObject = userAdvocateHome.create();
            mUserHome =
                    (UserHome) EJBHomeFactory.getInstance().getHomeInterface(UserHome.class, "User");
            mRefConfigurationSetHome =
                (RefConfigurationSetHome)
                EJBHomeFactory.getInstance().getHomeInterface(RefConfigurationSetHome.class, "RefConfigurationSet");
            mConfigurationSetHome = 
                (ConfigurationSetHome)EJBHomeFactory.getInstance().getHomeInterface(ConfigurationSetHome.class, "ConfigurationSet");
            mDeviceHome = (DeviceHome) EJBHomeFactory.getInstance().getHomeInterface(DeviceHome.class, "Device");
            Format cleanCompactFmt = Format.getCompactFormat();
            cleanCompactFmt.setOmitDeclaration( true );
            mXmlOutputter.setFormat( cleanCompactFmt );
        }
        catch (Exception e) {
            throw new RuntimeException (e.getMessage());
        }
    }

//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * createUser delegates the creating of a User to the UserAdvocate EJB.
     * Its value is that it takes human-friendly values for User ID and
     * puts in default values for other non-obvious values.
     *
     * @param userId the ID that you want to assign to the new user.
     * This is the value that you will see in the Configuration Server
     * UI and it will be used as the identity part of the Primary Line.
     * @param firstName first name for the new User.   May be null
     * @param lastName last name for the new User.    May be null
     * @param extension the extension you want to give the new User.
     * May be null.
     * @param PIN the PIN to use for the new User.  Must be numeric.
     *    May be null.
     * @param userGroupName the name of the User Group that you want to
     * place this User in.
     * @param alias an optional email address for the user.   This
     * is added as an alias for the new user.
     * @throws PDSException is thrown for all application level errors.
     */
    public void createUser (String userId,
                            String PIN,
                            String firstName,
                            String lastName,
                            String extension,
                            String alias,
                            String userGroupName) throws PDSException {


        try {
            checkMandatoryArgument("userId", userId);
            checkMandatoryArgument("userGroupName", userGroupName);

            User user =
                mUserAdvocateEJBObject.createUser(
                                            "1",
                                            firstName,
                                            lastName,
                                            PIN,
                                            userId,
                                            getAllUserRefConfigurationSetID(),
                                            getUserGroupIDFromName(userGroupName),
                                            extension);

            if(alias != null && alias.trim().length() != 0) {
                if (alias.trim().length() >= User.MAX_ALIAS_LEN) {
                    throw new PDSException (
                            MessageFormat.format(mSoapStrings.getString("E_EMAIL_TOO_LONG"),
                                    new Object []{new Integer(User.MAX_ALIAS_LEN)}) );
                }
                else {
                    user.setAliases(alias);
                }
            }
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }
    }


    /**
     * deleteUser delegates the deletion of a User to the UserAdvocate EJB.
     *
     * @param userID the ID of the User you want to delete.
     * @throws PDSException is thrown for all application level errors.
     */
    public void deleteUser (String userID) throws PDSException {
        try {
            checkMandatoryArgument("userID", userID);
            Collection users = mUserHome.findByDisplayID(userID);
            if(users.isEmpty()){
                throw new PDSException(collateErrorMessages("UC625", "E1026", new Object[]{userID}));
            }

            User user = (User) users.iterator().next();

            mUserAdvocateEJBObject.deleteUser(user.getID());
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }
    }

    /**
     * editUser allows you to modify an existing User.   The User you
     * wish to modify is specified by the existingUserId parameter.  The
     * only mandatory parameter is the existingUserId, you can supply null
     * for any of the others.
     *
     * @param existingUserId the existingUserId must correspond to an
     * existing User in the Config Server.
     * @param newUserId the purpose of this parameter is to allow you to
     * change the id for the User.   User ids must be unique accross the
     * namespace of all User Ids and extensions in the Config Server
     * @param PIN the new value for the User's PIN.   You may supply a
     * value of "NULL" or "null" to blank out the PIN if it has been
     * previously set.   PINs must only contain characters 0-9.
     * @param firstName the new value for the User's firstName.   You may
     * supply a value of "NULL" or "null" to blank out the firstName
     * if it has been previously set.
     * @param lastName the new value for the User's lastName.   You may
     * supply a value of "NULL" or "null" to blank out the lastName
     * if it has been previously set.
     * @param extension he new value for the User's extension.   You may
     * supply a value of "NULL" or "null" to blank out the extension
     * if it has been previously set.    Extensions must be numeric.
     * They must also be unique accross the namespace of all User Ids and
     * extensions in the Config Server.
     * @param alias the new value for the Users aliases.  Note that
     * when you set an alias you remove all of the previous alias
     * values.
     * @param userGroupName if you supply a fully-qualified device group name
     * then the User will be moved into this group.    You must specify the both the
     * ancestry and user group name relative to the root of the system.   For example
     * if I wanted to change the User's user group to the sales user group
     * which belongs to the (top) west coast user group I would supply
     * "west coast/sales".
     *
     * @throws PDSException is thrown for all application level errors.
     */
    public void editUser(   String existingUserId,
                            String newUserId,
                            String PIN,
                            String firstName,
                            String lastName,
                            String extension,
                            String alias,
                            String userGroupName) throws PDSException{

        try {
            checkMandatoryArgument("existingUserId", existingUserId);

            Collection users = mUserHome.findByDisplayID(existingUserId);
            if(users.isEmpty()) {
                throw new PDSException(collateErrorMessages("UC630", "E1026", new Object[]{existingUserId}));
            }

            User user = (User) users.iterator().next();

            String newUserIdParam = null;
            if (newUserId == null || isEmptySoapValue(newUserId)){
                newUserIdParam = user.getDisplayID();
            } else if (newUserId.equalsIgnoreCase(NULL_STRING)) {
                throw new PDSException(collateErrorMessages("UC630", "E3039", null)) ;
            } else {
                newUserIdParam = newUserId;
            }

            String firstNameParam = null;
            if (firstName == null || isEmptySoapValue(firstName)){
                firstNameParam = user.getDisplayID();
            } else if (!firstName.equalsIgnoreCase(NULL_STRING)) {
                firstNameParam = firstName;
            }

            String lastNameParam = null;
            if (lastName == null || isEmptySoapValue(lastName)){
                lastNameParam = user.getLastName();
            } else if (!lastName.equalsIgnoreCase(NULL_STRING)) {
                lastNameParam = lastName;
            }

            String pinParam = null;
            if (PIN == null || isEmptySoapValue(PIN)){
                pinParam = user.getPassword();
            } else if (!PIN.equalsIgnoreCase(NULL_STRING)){
                pinParam = PIN;
            }

            String userGroupNameParam = null;
            if (userGroupName == null || isEmptySoapValue(userGroupName)){
                userGroupNameParam = user.getUserGroupID().toString();
            } else if (userGroupName.equalsIgnoreCase(NULL_STRING)){
                throw new PDSException(collateErrorMessages("E3040", null));
            } else {
                userGroupNameParam = getUserGroupIDFromName(userGroupName);
            }

            String aliasParam = null;
            if (alias == null || isEmptySoapValue(alias)){
                aliasParam = user.getAliases();
            } else if (!alias.equalsIgnoreCase(NULL_STRING)){
                aliasParam = alias;
            }

            String extensionParam = null;
            if (extension == null || isEmptySoapValue(extension)){
                extensionParam = user.getExtension();
            } else if (!extension.equalsIgnoreCase(NULL_STRING)){
                extensionParam = extension;
            }

            mUserAdvocateEJBObject.editUser(user.getID(),
                    newUserIdParam,
                    firstNameParam,
                    lastNameParam,
                    pinParam,
                    user.getRefConfigSetID().toString(),
                    userGroupNameParam,
                    aliasParam,
                    extensionParam);
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }

    }
    
    /**
     * Change just the pintoken for a user, pintoken is used to log into sipXconfig
     * and voicemail access.  this is different than sip password or it's md5 form the 
     * passtoken used purely for SIP registration authorization. 
     * 
     * @param
     */
    public void setPintoken(String userID, String pintoken) throws PDSException {
        try {
            checkMandatoryArgument("userID", userID);
            checkMandatoryArgument("pintoken", pintoken);

            Collection users = mUserHome.findByDisplayID(userID);
            if (users.isEmpty()) {
                throw new PDSException(collateErrorMessages("UC600", "E1026",
                        new Object[] { userID }));
            }

            User user = (User) users.iterator().next();
            mUserAdvocateEJBObject.setUsersPinToken(user, user.getDisplayID(), pintoken);

        } catch (PDSException e) {
            throw e;
        } catch (Exception e) {
            throw new PDSException(e.getMessage());
        }        
    }


    /**
     * assignDevice gives ownership of a Device to a User.   This means that all the User
     * settigns will be sent to the Device after the profiles have been sent.
     *
     * @param userID the ID of the User who is to own the Device.
     * @param deviceSerialNumber the serial number of the Device who is to be
     * assigned.
     * @throws PDSException is thrown for all application level errors.
     */
    public void assignDevice (String userID, String deviceSerialNumber) throws PDSException {
        try {
            checkMandatoryArgument("userID", userID);
            checkMandatoryArgument("deviceSerialNumber", deviceSerialNumber);

            Collection users = mUserHome.findByDisplayID(userID);
            if(users.isEmpty()){
                throw new PDSException(collateErrorMessages("UC600", "E1026", new Object[]{userID}));
            }

            User user = (User) users.iterator().next();

            Collection devices = mDeviceHome.findBySerialNumber(deviceSerialNumber);
            if(devices.isEmpty()){
                throw new PDSException(collateErrorMessages("UC600", "E1001", new Object[]{deviceSerialNumber}));
            }

            Device device = (Device) devices.iterator().next();

            mUserAdvocateEJBObject.assignDevice(device.getID().toString(), user.getID());
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }
    }

    


    public void addUserLine(String userID, String allowForwarding, String registration, String url) throws PDSException, RemoteException {
        
        try {
            ConfigurationSet cs = getConfigSetForUser( userID );
            Document doc = getXMLDocumentFromConfigset( cs );
        
            Element root = doc.getRootElement();
            Element outerUserLineEl = new Element("USER_LINE");
            outerUserLineEl.setAttribute( "ref_property_id", "51" );
            outerUserLineEl.setAttribute( "id", "7369703a" + 
               System.currentTimeMillis() + String.valueOf(s_uniqueLineIdCounter++));
            Element innerUserLineEl = new Element("USER_LINE");
            Element registrationEl = new Element("REGISTRATION");
            CDATA regcd = new CDATA( registration );
            registrationEl.addContent( (Content)regcd );

            Element allowForwardingEl = new Element("ALLOW_FORWARDING");
            CDATA allowCd = new CDATA( allowForwarding );
            allowForwardingEl.addContent( (Content)allowCd );
            Element urlEl = new Element("URL");
            CDATA urlCd = new CDATA( url );
            urlEl.addContent( (Content)urlCd );

            outerUserLineEl.addContent( innerUserLineEl );
            innerUserLineEl.addContent( registrationEl );
            innerUserLineEl.addContent( allowForwardingEl );
            innerUserLineEl.addContent( urlEl );
            root.addContent( outerUserLineEl );
            
            //
            cs.setContent( mXmlOutputter.outputString ( doc ) );
            //
        }
        catch ( Exception e ) {
            throw new PDSException( e.toString() );
        }
    }
    
    public void addUserCredential( String userID, String lineUrl, String realm, String credUserId, String passtoken ) throws PDSException, RemoteException {
        try {
            ConfigurationSet cs = getConfigSetForUser( userID );
            Document doc = getXMLDocumentFromConfigset( cs );

            // first, find the line.  This xpath statement reads:
            //  locate the URL matching (url) and return the parent
            //  which is a USER_LINE node.  This SHOULD be a single element list
            String xPathMatchPattern = "/PROFILE/USER_LINE/USER_LINE/URL[contains(text(), '"+ lineUrl +"')]";
            XPath matchingUrlPath = null;
            matchingUrlPath = XPath.newInstance( xPathMatchPattern );
            // this should be a single element list
            List matchingUrls = matchingUrlPath.selectNodes( doc );

            Element matchingUrl = (Element)matchingUrls.get(0);
            Element line = (Element)matchingUrl.getParent();

            // now build a credential
            Element credentialEl = new Element("CREDENTIAL");
            Element realmEl = new Element("REALM");
            realmEl.addContent( realm );
            Element userIdEl = new Element("USERID");
            userIdEl.addContent( credUserId );
            Element passtokenEl = new Element("PASSTOKEN");
            passtokenEl.addContent( passtoken );

            credentialEl.addContent( realmEl );
            credentialEl.addContent( userIdEl );
            credentialEl.addContent( passtokenEl );

            line.addContent( credentialEl );
            //
            cs.setContent( mXmlOutputter.outputString ( doc ) );
            //
        }
        catch ( Exception e ) {
            e.printStackTrace();
            throw new PDSException( e.toString() );
        }
        
    }
    
    public void deleteLineCredential(String userID, String lineUrl, String credentialUserId) throws PDSException, RemoteException {
        try {
            ConfigurationSet cs = getConfigSetForUser( userID );
            Document doc = getXMLDocumentFromConfigset( cs );
            
            XPath linesPath = XPath.newInstance("/PROFILE/USER_LINE/USER_LINE" );
            List lines = linesPath.selectNodes( doc );

            Element ltd = null; 
            Element ctd = null;
            for ( Iterator it = lines.iterator(); it.hasNext(); ) {
                Element line = (Element)it.next();
                Element url = line.getChild("URL");

                if (  url.getText().equals( lineUrl ) ) {
                    List creds = line.getChildren("CREDENTIAL");
                    for ( Iterator it2 = creds.iterator(); it2.hasNext(); ) {
                        Element cred = (Element)it2.next();
                        if ( cred.getChild("USERID").getText().equals(credentialUserId) ) {
                            ltd = line;
                            ctd = cred;
                        }
                    }
                }
            }
            ltd.removeContent( ctd );            
            //
            cs.setContent( mXmlOutputter.outputString ( doc ) );
            //
        }
        catch ( Exception e ) {
            throw new PDSException( e.toString() );
        }
    }
    
    public void deleteUserLine(String userID, String lineUrl) throws PDSException, RemoteException {
 
        try {
            ConfigurationSet cs = getConfigSetForUser( userID );
            Document doc = getXMLDocumentFromConfigset( cs );
            
            // first, find the line.  This xpath statement reads:
            //  locate the URL matching (url) and return the parent
            //  which is a USER_LINE node.  This SHOULD be a single element list

            String selectUrlXPath = "/PROFILE/USER_LINE/USER_LINE/URL[contains(text(), '"+ lineUrl +"')]";

            XPath matchingUrlPath = XPath.newInstance( selectUrlXPath );
            // this should be a single element list
            List matchingUrls = matchingUrlPath.selectNodes( doc );

            Element matchingUrl = (Element)matchingUrls.get(0);
            Element innerLine = (Element)matchingUrl.getParent();
            Element outerLine = (Element)innerLine.getParent();
            Element root = (Element)outerLine.getParent();
            root.removeContent( outerLine );
            //
            cs.setContent( mXmlOutputter.outputString ( doc.getRootElement() ) );
            //
        }
        catch ( Exception e ) {
            throw new PDSException( e.toString() );
        }
    }
    
    
    
    

    /**
     * unassignDevice removes ownership of a Device from its current
     * owner.   If the Device is not owned by anyone then nothing
     * is done, no exception is thrown.
     *
     * @param serialNumber the serial number of the Device who
     * you wish to remove ownership from.
     * @throws PDSException is thrown for all application level errors.
     */
    public void unassignDevice(String serialNumber) throws PDSException {
        try {
            checkMandatoryArgument("serialNumber", serialNumber);

            Collection devices = mDeviceHome.findBySerialNumber(serialNumber);
            if(devices.isEmpty()){
                throw new PDSException(collateErrorMessages("UC602", "E1001", new Object[]{serialNumber}));
            }

            Device device = (Device) devices.iterator().next();

            mUserAdvocateEJBObject.unassignDevice(device.getID().toString());
        }
        catch (PDSException e) {
            throw e;
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }
    }

    private ConfigurationSet getConfigSetForUser( String userID ) throws FinderException, RemoteException {
        Collection users = mUserHome.findByDisplayID( userID );
        User user = (User)users.iterator().next();
        Collection configs = mConfigurationSetHome.findByUserID( user.getID () );
        return (ConfigurationSet)configs.iterator().next();

    }

    private Document getXMLDocumentFromConfigset( ConfigurationSet cs ) throws JDOMException, java.io.IOException {
        return mSaxBuilder.build( new StringReader( cs.getContent() ) );
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private synchronized String getAllUserRefConfigurationSetID()
            throws RemoteException, FinderException {

        if (mAllUserRefConfigurationSetID == null) {
            Collection set = mRefConfigurationSetHome.findByName("Complete User");
            RefConfigurationSet refConfigSet = (RefConfigurationSet) set.iterator().next();
            mAllUserRefConfigurationSetID = refConfigSet.getID().toString();
        }

        return mAllUserRefConfigurationSetID;
    }


    private String getUserGroupIDFromName (String userGroupName) throws PDSException {
       return getUserGroupFromTree(userGroupName);
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
