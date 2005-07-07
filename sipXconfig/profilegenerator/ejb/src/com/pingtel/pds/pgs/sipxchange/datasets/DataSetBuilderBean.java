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

package com.pingtel.pds.pgs.sipxchange.datasets;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.XMLSupport;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.phone.*;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.pgs.sipxchange.replication.ReplicationException;
import com.pingtel.pds.pgs.sipxchange.replication.ReplicationManager;
import com.pingtel.pds.pgs.sipxchange.replication.ReplicationResource;
import com.pingtel.pds.pgs.sipxchange.SatelliteComponent;
import com.pingtel.pds.pgs.user.*;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;
import org.apache.regexp.RE;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.activation.FileDataSource;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringReader;
import java.rmi.RemoteException;
import java.util.*;


/**
 * DataSetBuilderBean is the bean implementation class for the DatasetBuilder
 * bean.  It is used to produce the various SIPxchange datasets.
 *
 * @author IB
 */
public class DataSetBuilderBean extends JDBCAwareEJB
    implements SessionBean, DataSetBuilderBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String PHONESET_LINE_CODE = "xp_1031";
    private static final String USER_LINE_CODE = "xp_2029";
    private static final String PRIMARY_LINE_CODE = "xp_10001";
    private static final String SIP_FORWARD_ON_BUSY_CODE = "xp_2020";
    private static final String SIP_FORWARD_ON_NO_ANSWER_CODE = "xp_2021";
    private static final String SIP_FORWARD_UNCONDITIONAL_CODE = "xp_2022";

    private static final int USER_CONFIG_SETS = 0x1;
    private static final int USER_GROUP_CONFIG_SETS = 0x2;
    private static final int DEVICE_CONFIG_SETS = 0x4;
    private static final int DEVICE_GROUP_CONFIG_SETS = 0x8;

    private static final int MIN_EXTERNAL_FORWARDING_LENGTH = 7;


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Home references
    private RefPropertyHome mRefPropertyHome = null;
    private OrganizationHome mOrganizationHome = null;
    private DeviceHome mDeviceHome = null;
    private ConfigurationSetHome mConfigurationSetHome = null;
    private UserHome mUserHome = null;
    private UserGroupHome mUserGroupHome = null;
    private DeviceGroupHome mDeviceGroupHome = null;


    // Stateless Session references
    private ProjectionHelper mProjectionHelperEJBObject = null;

    // Non-clearing Entity Bean references
    private RefProperty mPhonesetLineRPEJBObject = null;
    private RefProperty mUserLineRPEJBObject = null;
    private RefProperty mPrimaryLineRPEJBObject = null;
    private RefProperty mSIPForwardOnBusyEJBObject = null;
    private RefProperty mSIPForwardOnNoAnswerEJBObject = null;
    private RefProperty mSIPForwardUnconditionalEJBObject = null;

    // Bean context
    private SessionContext mCTX;

    // Misc.
    private XMLOutputter mXMLOutputter = new XMLOutputter();

    private HashMap mOrganizationEJBOBjectsMap = new HashMap();
    private HashMap mUserConfigSetsEJBObjectsMap = new HashMap();
    private HashMap mDeviceConfigSetsEJBObjectsMap = new HashMap();

    private Integer mXpressaDeviceTypeID = null;

    // used to cache whether this is an entprise of (in the future) a
    // service provider installation.
    private boolean mIsEnterprise;


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbActivate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbPassivate() { }

    /**
     *  Standard EJB implementation
     */
    public void setSessionContext(SessionContext ctx) {
        mCTX = ctx;

        if(getInstallStereoType().equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)){
            mIsEnterprise = true;
        }

        try {
            Context initial = new InitialContext();
            mRefPropertyHome = (RefPropertyHome) initial.lookup( "RefProperty" );
            mOrganizationHome = (OrganizationHome) initial.lookup( "Organization" );
            mDeviceHome = (DeviceHome) initial.lookup( "Device" );
            mConfigurationSetHome = (ConfigurationSetHome) initial.lookup( "ConfigurationSet" );
            mUserHome = (UserHome) initial.lookup( "User" );
            mUserGroupHome = (UserGroupHome) initial.lookup( "UserGroup" );
            mDeviceGroupHome = (DeviceGroupHome) initial.lookup( "DeviceGroup" );
            ProjectionHelperHome projectionHelperHome =
                    (ProjectionHelperHome) initial.lookup( "ProjectionHelper" );

            mProjectionHelperEJBObject = projectionHelperHome.create();

            DeviceTypeHome deviceTypeHome =
                    (DeviceTypeHome) initial.lookup("DeviceType");

            Collection xpressaC =
                    deviceTypeHome.findByModel(PDSDefinitions.MODEL_HARDPHONE_XPRESSA);

            DeviceType xpressa = (DeviceType) xpressaC.iterator().next();
            mXpressaDeviceTypeID = xpressa.getID();

            mPhonesetLineRPEJBObject = lookupRefProperty (PHONESET_LINE_CODE);
            mUserLineRPEJBObject = lookupRefProperty (USER_LINE_CODE);
            mPrimaryLineRPEJBObject = lookupRefProperty(PRIMARY_LINE_CODE);
            mSIPForwardOnBusyEJBObject =
                    lookupRefProperty(SIP_FORWARD_ON_BUSY_CODE);

            mSIPForwardOnNoAnswerEJBObject =
                    lookupRefProperty(SIP_FORWARD_ON_NO_ANSWER_CODE);

            mSIPForwardUnconditionalEJBObject =
                    lookupRefProperty(SIP_FORWARD_UNCONDITIONAL_CODE);
        }
        catch (Exception ex) {
            logFatal (ex.toString(), ex );
            throw new EJBException (ex);
        }
    }

    /**
     *  Standard EJB implementation
     */
    public void unsetSessionContext() {
        mCTX = null;
    }


    /**
     * generateAuthExceptions produces a complete version of the
     * authexception.xml dataset.
     *
     * @param changePoints are the location(s) in the User/UserGroup hierarchy.  If the
     * forwarding settings or permission change occurred for just one user then just pass
     * in that User.  If an entire UserGroup (and their subgroups) need changing then
     * pass that in.   Any number or combination of the above can be passed in.
     * @throws com.pingtel.pds.common.PDSException is thrown for applcation errors.
     */
    public void generateAuthExceptions (Collection changePoints) throws PDSException {

        try {
            java.util.Date d1 = new java.util.Date();
            HashMap forwardingRefPropertyIds = new HashMap();
            forwardingRefPropertyIds.put(mSIPForwardOnBusyEJBObject.getID().toString(), null);
            forwardingRefPropertyIds.put(mSIPForwardOnNoAnswerEJBObject.getID().toString(), null);
            forwardingRefPropertyIds.put(mSIPForwardUnconditionalEJBObject.getID().toString(), null);

            RE integerOnlyRegExp =
                    new RE("^[0-9]{" + MIN_EXTERNAL_FORWARDING_LENGTH + ",}$");

            // HACK: why do we even pretend we can support more than one org?
            // this code looks like its unsafe however there should only be one organization
            // in the database in the sipxchange/enterprise mode.   If this requirement
            // changes we can get the individual organizations for each user and rebuild
            // the RE each time.
            Collection allOrganizations = mOrganizationHome.findAll();
            Organization enterpriseOrg = (Organization) allOrganizations.iterator().next();

            RE localSIPDestinationRegExp =
                    new RE("^sip:[0-9]{" + MIN_EXTERNAL_FORWARDING_LENGTH + ",}@" +
                    enterpriseOrg.getDNSDomain() +"(:)?.*$");

            localSIPDestinationRegExp.setMatchFlags(RE.MATCH_CASEINDEPENDENT);

            Element authExceptionsRoot = new Element ("items").setAttribute("type","authexception");

            ArrayList authExceptions = new ArrayList();

            for (Iterator iChangePoints = changePoints.iterator(); iChangePoints.hasNext(); ) {
                Object o = iChangePoints.next();

                if (o instanceof UserGroup) {
                    UserGroup userGroup = (UserGroup) o;

                    authExceptions.addAll(
                            getAuthExceptions(
                                    userGroup,
                                    forwardingRefPropertyIds,
                                    integerOnlyRegExp,
                                    localSIPDestinationRegExp));

                } else {
                    // @todo
                }
            }

            for (Iterator iAuthExceptions = authExceptions.iterator(); iAuthExceptions.hasNext(); ) {
                Element authException = (Element) iAuthExceptions.next();

                authExceptionsRoot.addContent(authException);
            }

            java.util.Date d2 = new java.util.Date();
            logInfo( "IT TOOK " + (d2.getTime() - d1.getTime()) + " ms TO GENERATE AUTH EXCEPTIONS" );

            logDebug ( mXMLOutputter.outputString( (Element)authExceptionsRoot.detach() ) );

            writeFile(  new Document ( (Element)authExceptionsRoot.detach() ),
                        SatelliteComponent.TYPE_COMM_SERVER,
                        ReplicationResource.DATABASE_AUTH_EXCEPTIONS );

        }
        catch (  Exception ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (collateErrorMessages ("UC1110", "E4083", null));
        }
    }


    /**
     * generateCredentials produces a complete version of the
     * credential.xml dataset.
     *
     * @throws com.pingtel.pds.common.PDSException
     */
    public void generateCredentials () throws PDSException {
         try {
            // get ref property names for the CODEs for PHONESET_LINE and USER_LINE
            java.util.Date d1 = new java.util.Date();
            String phonesetLineRPID = null;
            String userLineRPID = null;
            String primaryLineRPID = null;

            phonesetLineRPID = mPhonesetLineRPEJBObject.getID().toString();
            logDebug ( "generateCredentials::phoneset_line rp_id is: " + phonesetLineRPID );

            userLineRPID = mUserLineRPEJBObject.getID().toString();
            logDebug ( "generateCredentials::user_line rp_id is: " + userLineRPID );

            primaryLineRPID = mPrimaryLineRPEJBObject.getID().toString();
            logDebug ( "generateCredentials::primary_line rp_id is: " + primaryLineRPID );

            Collection configSets =
                getConfigSets ( USER_CONFIG_SETS | DEVICE_CONFIG_SETS );

            // add to XML output doc
            /*
            <!-- Load the Credentials DB -->
               <items type="credentials">
                  <item>
                     <uri>sip:jcoffey@pingtel.com:5060;internal=YES</uri>
                     <realm>pingtel</realm>
                     <userid>jcoffey</userid>
                     <passtoken>e456abcefd876347aaa678678</passtoken>
                     <authtype>DIGEST</authtype>
                  </item>
                </items>*/
            final SAXBuilder saxBuilder = new SAXBuilder();
            try {
                String xchangeRealm = null;

                if(mIsEnterprise){
                    Organization xchangeOrganization =
                            mOrganizationHome.findByPrimaryKey(
                                    new Integer ("1") );

                    xchangeRealm = xchangeOrganization.getAuthenticationRealm();
                }

                logDebug ( "generateCredentials::creating the database XML document" );

                Element credentials =
                    new Element ( "items" ).setAttribute( "type", "credential" );

                // fix for bug# 2953, remove when we get SSO sorted out between the CS
                // and MS.
                credentials.addContent(createSuperadminCredential());

                for ( Iterator csI = configSets.iterator(); csI.hasNext(); ) {
                    ConfigurationSet cs = (ConfigurationSet) csI.next();

                    logDebug ( "generateCredentials::Looking at configuration set: " + cs.getID() );
                    Document doc = null;
                    try {
                        doc = saxBuilder.build(new StringReader(cs.getContent()));
                    } catch ( java.io.IOException ex ) {
                        throw new PDSException( ex.toString() );
                    } catch ( JDOMException jde ) {
                        throw new PDSException( jde.toString() );
                    }
                     
                    Element csRoot = doc.getRootElement ();
                    for ( Iterator contentI = csRoot.getChildren().iterator(); contentI.hasNext(); ) {
                        Element e = (Element) contentI.next();

                        String rpID = e.getAttributeValue( "ref_property_id");

                        if (    rpID != null &&
                                ( rpID.equals( phonesetLineRPID ) ||
                                        rpID.equals( primaryLineRPID ) ) ) {

                            Collection lineWrapperC = e.getChildren();
                            Element lineWrapper = null;
                            for ( Iterator iWrap = lineWrapperC.iterator(); iWrap.hasNext(); )
                                lineWrapper = (Element) iWrap.next();

                            for (   Iterator credI = lineWrapper.getChildren( "CREDENTIAL").iterator();
                                    credI.hasNext(); ) {

                                Element credential = (Element) credI.next();

                                String credentialRealm = credential.getChild( "REALM").getText();
                                logDebug ( "generateCredentials::realm for credential: " + credentialRealm );

                                if ((mIsEnterprise &&
                                        credentialRealm.equalsIgnoreCase(xchangeRealm)) ||
                                        !mIsEnterprise){

                                    Element item = new Element ( "item" );
                                    credentials.addContent( item );

                                    Element uri = new Element ( "uri" );
                                    logDebug ( "generateCredentials::uri for credential: " + lineWrapper.getChild( "URL" ).getText() );
                                    uri.addContent( lineWrapper.getChild( "URL" ).getText() );
                                    item.addContent( uri );

                                    Element realm = new Element ( "realm" );
                                    realm.addContent( credential.getChild( "REALM").getText() );
                                    item.addContent( realm );

                                    Element userID = new Element ( "userid" );
                                    logDebug ( "generateCredentials::userid for credential: " + credential.getChild( "USERID").getText() );
                                    userID.addContent( credential.getChild( "USERID").getText() );
                                    item.addContent( userID );

                                    Element password = new Element ( "passtoken" );
                                    logDebug ( "generateCredentials::passtoken for credential: " + credential.getChild( "PASSTOKEN").getText() );
                                    password.addContent( credential.getChild( "PASSTOKEN").getText() );
                                    item.addContent( password );

                                    // This is a hardcoded value for now; it is needed by the
                                    // database but is not part of line definitions.
                                    Element authType = new Element ( "authtype" );
                                    authType.addContent( "DIGEST");
                                    item.addContent ( authType );
                                }
                            } // for
                        } // if

                    } // for all elements in cs

                } // for each config set

                // sent to MS and PRRS

                java.util.Date d2 = new java.util.Date();

                logInfo( "IT TOOK " + (d2.getTime() - d1.getTime()) + " ms TO GENERATE CREDENTIALS" );

                logDebug ( mXMLOutputter.outputString( (Element)credentials.detach() ) );

                writeFile(  new Document ( (Element)credentials.detach() ),
                            SatelliteComponent.TYPE_COMM_SERVER,
                            ReplicationResource.DATABASE_CREDENTIAL );

            }
            catch ( java.io.IOException ex ) {
ex.printStackTrace();
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC1000",
                                            "E7007",
                                            null ),
                    ex );
            }
            catch ( FinderException e ) {
e.printStackTrace();
                mCTX.setRollbackOnly();

                throw new PDSException( collateErrorMessages (  "UC1000",
                                        "E4056",
                                        null ), e );
            }
        }
        catch ( PDSException ex ) {
ex.printStackTrace();
            mCTX.setRollbackOnly();
            throw new PDSException (
                collateErrorMessages ( "UC1000" ), ex );
        }
        catch ( RemoteException ex ) {
ex.printStackTrace();
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "UC1000",
                                        "E4056",
                                        null ) );
        }
    }


    /**
     * generateAliases produces the FULL version of the aliases database.
     *
     * @exception com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     */
    public void generateAliases () throws PDSException {

        try {
            /*
            <items type="alias">
              <item>
                <identity>jsmith@acme.com</identity>
                <contact>sip:111@acme.com</contact>
              </item>
                <identity>bjones@acme.com</identity>
                <contact>sip:112@acme.com</contact>
              </item>
            </items>
            */
            java.util.Date d1 = new java.util.Date();

            Element items = new Element ( "items" );
            items.setAttribute( "type", "alias" );

            // add to XML output doc
            Collection userC = null;
            try {
                userC = mUserHome.findAll();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1048",
                                            null ),
                    ex );
            }

            for ( Iterator iUser = userC.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();

                logDebug ( "generateAliases::Looking at user: " + user.getDisplayID() );

                if (    user.getDisplayID().equals( "superadmin" ) ||
                        user.getDisplayID().equals( "SDS" ))
                    continue;

                String aliases = user.getAliases();
                logDebug ( "generateAliases::aliases: " + aliases );
                String extension = user.getExtension();
                logDebug ( "generateAliases::extension: " + extension );

                if ( aliases == null && extension == null )
                    continue;

                String primaryLineURL = user.calculatePrimaryLineURL();

                Organization organization =
                    getOrganizationEJBObject( user.getOrganizationID() );

                StringTokenizer aliasTok = new StringTokenizer ( aliases, "," );
                while ( aliasTok.hasMoreTokens() ) {
                    String alias = aliasTok.nextToken();

                    Element item = new Element ( "item" );
                    items.addContent( item );
                    Element identity = new Element ( "identity" );

                    String trimmedAlias = alias.trim() + "@" + organization.getDNSDomain();
                    logDebug ( "generateAliases::identity: " + trimmedAlias );

                    identity.setText( trimmedAlias.toString() );

                    item.addContent( identity );
                    Element contact = new Element ( "contact" );
                    logDebug ( "generateAliases::contact: " + primaryLineURL );

                    contact.setText( primaryLineURL );
                    item.addContent( contact );
                } // while

                if ( extension != null ) {
                    Element item = new Element ( "item" );
                    items.addContent( item );
                    Element identity = new Element ( "identity" );
                    item.addContent( identity );

                    StringBuffer fullUserID = new StringBuffer ();
                    fullUserID.append( user.getExtension() );
                    fullUserID.append( "@" );
                    fullUserID.append( organization.getDNSDomain() );

                    logDebug ( "generateAliases::identity: " + fullUserID.toString() );
                    identity.setText( fullUserID.toString() );

                    Element contact = new Element ( "contact" );
                    logDebug ( "generateAliases::contact: " + primaryLineURL );
                    contact.setText( primaryLineURL );
                    item.addContent( contact );
                }
            } // for user

            String addInFiles = getPGSProperty( "dataset.alias.addins" );
            final SAXBuilder saxBuilder = new SAXBuilder();

            if ( addInFiles != null && addInFiles.trim().length() > 0 ) {
                String addInsPath =
                        PathLocatorUtil.getInstance().getPath(  PathLocatorUtil.DATASET_ADDINS_FOLDER,
                                                                PathLocatorUtil.PGS );

                StringTokenizer fileTokens = new StringTokenizer ( addInFiles, "," );

                while ( fileTokens.hasMoreTokens() ) {
                    String fileName = fileTokens.nextToken();

                    logInfo ( "adding in alias file <" + addInsPath + fileName + ">" );

                    FileDataSource fds = new FileDataSource ( addInsPath + fileName );
                    if ( !fds.getFile().exists() ) {
                        logError ( collateErrorMessages ( "UC1080", "E4082", new Object [] { fileName } ) );
                        continue;
                    }
                    Document imported = saxBuilder.build(fds.getInputStream());
                    Element importedRoot = imported.getRootElement();

                    Collection itemC = importedRoot.getChildren( "item");

                    for ( Iterator itemI = XMLSupport.detachableIterator(itemC.iterator()); itemI.hasNext(); ) {
                        Element item = (Element) itemI.next();
                        items.addContent( (Element)item.detach() );
                    }
                }
            }

            java.util.Date d2 = new java.util.Date();

            logInfo( "IT TOOK " + (d2.getTime() - d1.getTime()) + " ms TO GENERATE ALIASES" );

            writeFile(  new Document ( (Element)items.detach() ),
                        SatelliteComponent.TYPE_COMM_SERVER,
                        ReplicationResource.DATABASE_ALIAS );

        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException (
                collateErrorMessages ( "UC1080" ), ex );
        }
        catch (Exception ex) {  // If we can't find a user's organization
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "UC1080",
                                        "E4066",
                                        null ) );
        }
    }


    /**
     * generatePermissions builds the Permissions export dataset and sends it to
     * the other SIPxchange servers.
     *
     * @param changePoints are the location(s) in the User/UserGroup hierarchy.  If the
     * forwarding settings or permission change occurred for just one user then just pass
     * in that User.  If an entire UserGroup (and their subgroups) need changing then
     * pass that in.   Any number or combination of the above can be passed in.
     * @throws com.pingtel.pds.common.PDSException is thrown for applcation errors.
     */
    public void generatePermissions (Collection changePoints) throws PDSException {

        try {
            java.util.Date d1 = new java.util.Date();
            ArrayList permissionList = new ArrayList();

            for (Iterator iChangePoints = changePoints.iterator(); iChangePoints.hasNext(); ) {
                Object o = iChangePoints.next();

                if (o instanceof UserGroup) {
                    UserGroup userGroup = (UserGroup) o;

                    permissionList.addAll(generatePermissionsList(userGroup));
                } else {
                    // @todo
                }
            }

            Element items = new Element ( "items" );
            items.setAttribute( "type", "permission" );

            for (Iterator iPermission = permissionList.iterator(); iPermission.hasNext(); ) {
                Element item = (Element) iPermission.next();
                items.addContent(item);
            }

            java.util.Date d2 = new java.util.Date();

            logInfo ( "IT TOOK " + (d2.getTime() - d1.getTime()) + " ms TO GENERATE PERMISSIONS" );

            logDebug ( mXMLOutputter.outputString( (Element)items.detach()) );

            writeFile(  new Document ( (Element)items.detach() ),
                        SatelliteComponent.TYPE_COMM_SERVER,
                        ReplicationResource.DATABASE_PERMISSION );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "UC1090",
                                        "E4067",
                                        null ) );
        }
        catch (Exception ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages ( "UC1090" ), ex );
        }
    }




    /**
     * generateExtensions produces the FULL version of the extensions database.
     *
     * @exception com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     */
    public void generateExtensions () throws PDSException {

        try {
            java.util.Date d1 = new java.util.Date();

            Element items = new Element ( "items" );
            items.setAttribute( "type", "extension" );

            Collection allUsers = mUserHome.findAll();

            for ( Iterator iUser = allUsers.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();
                logDebug ( "generateExtensions::looking at user: " + user.getDisplayID() );
                String extension = user.getExtension();

                if ( extension != null ) {
                    Organization usersOrganization =
                            getOrganizationEJBObject ( user.getOrganizationID() );

                    String uriText = user.calculatePrimaryLineURL();

                    Element item = new Element ( "item" );

                    Element uri = new Element ( "uri" );
                    uri.setText( uriText  );
                    logDebug ( "generateExtensions::setting uri: " + uriText );
                    item.addContent( uri );

                    Element extensionElem = new Element ("extension");
                    StringBuffer extensionText = new StringBuffer();
                    extensionText.append( extension );
                    extensionText.append ( "@" );
                    extensionText.append ( usersOrganization.getDNSDomain() );

                    logDebug ( "generateExtensions::setting extension: " + extensionText.toString() );

                    extensionElem.setText( extensionText.toString() );

                    item.addContent ( extensionElem );

                    items.addContent( item );
                }
            }

            java.util.Date d2 = new java.util.Date();

            logInfo ( "IT TOOK " + (d2.getTime() - d1.getTime()) +
                " ms TO GENERATE EXTENSIONS" );

            logDebug  ( mXMLOutputter.outputString( (Element)items.detach() ) );

            writeFile(  new Document ( (Element)items.detach() ),
                        SatelliteComponent.TYPE_MEDIA_SERVER,
                        ReplicationResource.DATABASE_EXTENSION );
        }
        catch (PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages ( "UC1100" ), ex );
        }
        catch (Exception ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "UC1100",
                                        "E4078",
                                        null ) );
        }
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


    private Collection getUserGroupsConfigurationSets(
            HashMap userGroupConfigSetsMap,
            UserGroup userGroup) throws RemoteException {

        Collection groupConfigSets;
        if ( !userGroupConfigSetsMap.containsKey( userGroup.getID() ) ) {
            logDebug ( "generatePermissions::looking at user group:" +
                    userGroup.getName() );

            ArrayList configSetList = getUserGroupTreeConfigSets(userGroup);
            userGroupConfigSetsMap.put(userGroup.getID(), configSetList);
            groupConfigSets = configSetList;
        }
        else {
            groupConfigSets =
              (Collection)
                  userGroupConfigSetsMap.get(userGroup.getID());
        }
        return groupConfigSets;
    }


    private ArrayList getUserGroupTreeConfigSets(UserGroup userGroup)
            throws RemoteException {

        Collection groupConfigSets;
        ArrayList configSetList = new ArrayList();

        groupConfigSets =
          mProjectionHelperEJBObject.addParentGroupConfigSets(
              userGroup,
              PDSDefinitions.PROF_TYPE_USER );

        logDebug ( "generatePermissions::got projection inputs for user group" );

        for ( Iterator iUGProjInputs = groupConfigSets.iterator(); iUGProjInputs.hasNext(); ) {
          ProjectionInput pi = (ProjectionInput) iUGProjInputs.next();

          configSetList.add(pi.getDocument());
        }
        return configSetList;
    }


    private HashMap projectPermissions(Collection configSets, Map cachedPreviousResults) {
        HashMap propertySettings = new HashMap();

        if (cachedPreviousResults != null) {
            propertySettings.putAll(cachedPreviousResults);
        }

        for ( Iterator csI = configSets.iterator(); csI.hasNext(); ) {
            Document doc = (Document) csI.next();

            Element csRoot = doc.getRootElement ();

            for ( Iterator contentI = csRoot.getChildren().iterator(); contentI.hasNext(); ) {
                Element e = (Element) contentI.next();

                if ( e.getName().equals( "PERMISSIONS" ) ) {

                    Collection wrapperC = e.getChildren();
                    Element wrapper = null;
                    for ( Iterator iWrap = wrapperC.iterator(); iWrap.hasNext(); )
                        wrapper = (Element) iWrap.next();

                    Collection permissionsC = wrapper.getChildren();

                    for (   Iterator iPermissions = permissionsC.iterator();
                            iPermissions.hasNext(); ) {

                        Element m = (Element) iPermissions.next();
                        logDebug ( "generatePermissions::got permission: " + m.getName() + " value: " + m.getText() );
                        propertySettings.put( m.getName(), m.getText() );
                    }

                }
            }

        }// for all projection inputs
        return propertySettings;
    }


    private Collection generatePermissionsList(UserGroup userGroup)
            throws RemoteException, PDSException, FinderException, JDOMException, java.io.IOException {

        final SAXBuilder saxBuilder = new SAXBuilder();

        ArrayList permissionList = new ArrayList();
        HashMap userGroupConfigSetsMap = new HashMap();
        ArrayList configSets = new ArrayList();
        HashMap cachedGroupPermissionProjectionMap = new HashMap();

        Collection groupConfigSets = getUserGroupsConfigurationSets(userGroupConfigSetsMap, userGroup);

        ArrayList configSetsList = new ArrayList();
        configSetsList.addAll(groupConfigSets);
        Collections.reverse(configSetsList);

        cachedGroupPermissionProjectionMap = projectPermissions(groupConfigSets, null);

        Collection userC = null;
        try {
            userC = mUserHome.findByUserGroupID(userGroup.getID());
        }
        catch ( FinderException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException ( collateErrorMessages ( "E1049" ), ex );
        }

        for ( Iterator iUser = userC.iterator(); iUser.hasNext(); ) {
            User user = (User) iUser.next();

            if (    user.getDisplayID().equals( "superadmin" ) ||
                    user.getDisplayID().equals( "SDS" ) ) {
                continue;
            }

            logDebug ( "generatePermissions::looking at user:" + user.getDisplayID() );

            ConfigurationSet userCS = getUserCSEJBObject( user.getID() );
            Document userCSDoc = null;

            if ( userCS != null ) {
                userCSDoc = saxBuilder.build(new StringReader(userCS.getContent()));
                configSets.add( userCSDoc );
                logDebug ( "generatePermissions::got users configuration set" );
            }

            HashMap propertySettings = projectPermissions(configSets, cachedGroupPermissionProjectionMap);

            Set projectedPermissions = propertySettings.entrySet();

            for (   Iterator iPermissions = projectedPermissions.iterator();
                    iPermissions.hasNext(); ) {

                Map.Entry entry = (Map.Entry) iPermissions.next();
                String permissionName = (String) entry.getKey();
                String permissionValue = (String) entry.getValue();

                Element item = new Element ( "item" );
                Element identityElement = new Element ( "identity" );
                String identity = user.calculatePrimaryLineURL();
                logDebug ( "generatePermissions::identity: " + identity);
                identityElement.setText( identity );

                if ( permissionValue.equalsIgnoreCase( "ENABLE" ) ) {
                    Element permissionElement = new Element ( "permission" );

                    if ( permissionName.startsWith( "_" ) )
                        permissionName = permissionName.substring( 1 );

                    logDebug ( "generatePermissions::adding permission: " + permissionName );

                    permissionElement.setText( permissionName );
                    item.addContent( identityElement );
                    item.addContent( permissionElement );
                    permissionList.add( item );
                }  // if

            } // for all permissions

            if (userCSDoc != null) {
                configSets.remove( userCSDoc );
            }

        } // for all Users

        // process all the child groups of this UserGroup
        Collection childGroups = mUserGroupHome.findByParentID(userGroup.getID());

        for (Iterator iChild = childGroups.iterator(); iChild.hasNext(); ) {
            UserGroup child = (UserGroup) iChild.next();
            permissionList.addAll(generatePermissionsList(child));
        }

        return permissionList;
    }



    private void writeFile (Document content, int componentType, String targetDataName )
        throws PDSException {

        try {
            java.util.Date d1 = new java.util.Date();
            ByteArrayOutputStream bas = new ByteArrayOutputStream ( );

            mXMLOutputter.output( content, bas );

            ReplicationManager.getInstance().
                replicateData(  bas.toByteArray(),
                                componentType,
                                ReplicationResource.TYPE_DATABASE,
                                targetDataName  );

            java.util.Date d2 = new java.util.Date();

            logInfo( "IT TOOK " + (d2.getTime() - d1.getTime()) + " ms TO WRITE FILE" );
        }
        catch ( IOException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E8006",
                                        new Object [] { targetDataName } ),
                ex );
        }
        catch ( ReplicationException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E8006",
                                        new Object [] { targetDataName } ),
                ex );
        }

    }


    /**
     * getConfigSets retrieves ConfigurationSets for all of the Organizations
     * in the PGS.   You can specify whether you and ConfigurationSets fors
     * Users, UserGroups, Devices and DeviceGroups.
     *
     * @param mask bit mask for the types of ConfigurationSets that you want
     * @return Collection of ConfigurationSet objects.
     * @throws com.pingtel.pds.common.PDSException is thrown for application errors.
     * @see #USER_CONFIG_SETS #USER_GROUP_CONFIG_SETS #DEVICE_CONFIG_SETS
     * #DEVICE_GROUP_CONFIG_SETS
     */
    private Collection getConfigSets ( int mask ) throws PDSException {

        try {
            // for each organization - this is xchange mode therefore should
            // only be one.
            Collection organizationsC = null;

            try {
                organizationsC = mOrganizationHome.findAll();
            }
            catch ( FinderException ex ) {
                 mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1041",
                                            null ),
                    ex );
            }

            ArrayList configSets = new ArrayList();

            for ( Iterator orgI = organizationsC.iterator(); orgI.hasNext(); ) {
                Organization organization = (Organization) orgI.next();
                configSets.addAll( getConfigurationSets ( organization, mask ) );
            } // for all organizations

            return configSets;
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "E4056",
                                        null ) );
        }
    }


    /**
     * For a given organization find all the configuration sets for the
     * four entities (user, usergroup, device, devicegroup).   All of these
     * are necessary as line definitions can be defined for any of these
      * entity types.
     * @param organization PK of the Organization whose ConfigurationSets
     * are being fetched
     * @param mask bit mask for the types of ConfigurationSets that you want
     * @return Collection of ConfigurationSets
     * @throws java.rmi.RemoteException is thrown for system errors
     * @throws com.pingtel.pds.common.PDSException is thrown for application level errors.
     */
    private Collection getConfigurationSets ( Organization organization, int mask )
        throws RemoteException, PDSException {

        ArrayList configSets = new ArrayList();

        logDebug ( "fetching config sets for organization " + organization.getID() );

        if ( ( DEVICE_CONFIG_SETS & mask ) == DEVICE_CONFIG_SETS )  {
            Collection devices = null;

            try {
                devices =
                    mDeviceHome.findByOrganizationID( organization.getID() );
            }
            catch ( FinderException ex ) {
ex.printStackTrace();
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC1000",
                                            "E1023",
                                            new Object [] { organization.getExternalID() } ),
                    ex );
            }


            for ( Iterator deviceI = devices.iterator(); deviceI.hasNext(); ) {
                Device device = (Device) deviceI.next();

                try {
                    ConfigurationSet cs = getDeviceCSEJBObject ( device.getID() );
                    if ( cs != null ) {
                        configSets.add( cs );
                    }
                }
                catch ( FinderException ex ) {
ex.printStackTrace();
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC1000",
                                                "E1002",
                                                new Object [] { device.getExternalID() } ),
                        ex );
                }

            } // for
        }

        if ( ( USER_CONFIG_SETS & mask ) == USER_CONFIG_SETS )  {
            Collection users = null;

            try {
                users =
                    mUserHome.findByOrganizationID( organization.getID() );

            }
            catch ( FinderException ex ) {
ex.printStackTrace();
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC1000",
                                            "E1023",
                                            new Object [] { organization.getExternalID() } ),
                    ex );
            }

            for ( Iterator userI = users.iterator(); userI.hasNext(); ) {
                User user = (User) userI.next();

                if (    user.getDisplayID().equals( "superadmin" ) ||
                        user.getDisplayID().equals( "SDS" ) )
                    continue;

                try {
                    ConfigurationSet cs = getUserCSEJBObject( user.getID() );
                    if ( cs != null ) {
                        configSets.add ( cs );
                    }
                }
                catch ( FinderException ex ) {
ex.printStackTrace();
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC1000",
                                                "E1024",
                                                new Object [] { user.getExternalID() } ),
                        ex );
                }
            }
        }

        if ( ( DEVICE_GROUP_CONFIG_SETS & mask ) == DEVICE_GROUP_CONFIG_SETS )  {
            Collection deviceGroups = null;

            try {
                deviceGroups = mDeviceGroupHome.findByOrganizationID( organization.getID() );
            }
            catch ( FinderException ex ) {
ex.printStackTrace();
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC1000",
                                                "E1042",
                                                new Object [] { organization.getExternalID() } ),
                        ex );
            }

            for ( Iterator dgI = deviceGroups.iterator(); dgI.hasNext(); ) {
                DeviceGroup dg = (DeviceGroup) dgI.next();

                try {
                    configSets.addAll( mConfigurationSetHome.findByDeviceGroupID( dg.getID() ) );
                }
                catch ( FinderException ex ) {
ex.printStackTrace();
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC1000",
                                                "E1010",
                                                new Object [] { dg.getExternalID() } ),
                        ex );
                }
            }
        }

        if ( ( USER_GROUP_CONFIG_SETS & mask ) == USER_GROUP_CONFIG_SETS )  {
            Collection userGroups = null;

            try {
                userGroups = mUserGroupHome.findByOrganizationID( organization.getID() );
            }
            catch ( FinderException ex ) {
ex.printStackTrace();
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC1000",
                                            "E1020",
                                            new Object [] { organization.getExternalID() } ),
                    ex );
            }

            for ( Iterator ugI = userGroups.iterator(); ugI.hasNext(); ) {
                UserGroup ug = (UserGroup) ugI.next();

                try {
                    configSets.addAll( mConfigurationSetHome.findByUserGroupID( ug.getID() ) );
                }
                catch ( FinderException ex ) {
ex.printStackTrace();
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC1000",
                                                "E1031",
                                                new Object [] { ug.getExternalID() } ),
                        ex );
                }
            }
        }

        return configSets;
    }


    private Organization getOrganizationEJBObject ( Integer id )
        throws FinderException, RemoteException {

        Organization returnValue = null;

        if ( !mOrganizationEJBOBjectsMap.containsKey( id ) ) {
            returnValue = mOrganizationHome.findByPrimaryKey( id );
            mOrganizationEJBOBjectsMap.put( id, returnValue );
        }
        else {
            returnValue = (Organization) mOrganizationEJBOBjectsMap.get( id );
        }

        return returnValue;
    }


    private ConfigurationSet getUserCSEJBObject ( String id )
        throws FinderException, RemoteException {

        ConfigurationSet returnValue = null;

        if ( !mUserConfigSetsEJBObjectsMap.containsKey( id ) ) {
            Collection c =
                mConfigurationSetHome.findByUserIDAndProfileType( id, PDSDefinitions.PROF_TYPE_USER );

            for ( Iterator i = c.iterator(); i.hasNext(); )
                returnValue = (ConfigurationSet) i.next();

            if ( returnValue != null ) {
                mUserConfigSetsEJBObjectsMap.put( id, returnValue );
            }
        }
        else {
            returnValue = (ConfigurationSet) mUserConfigSetsEJBObjectsMap.get( id );
        }

        return returnValue;
    }


    private ConfigurationSet getDeviceCSEJBObject ( Integer id )
        throws FinderException, RemoteException {

        ConfigurationSet returnValue = null;

        if ( !mDeviceConfigSetsEJBObjectsMap.containsKey( id ) ) {
            Collection c = mConfigurationSetHome.findByLogicalPhoneID( id );

            for ( Iterator i = c.iterator(); i.hasNext(); )
                returnValue = (ConfigurationSet) i.next();

            if ( returnValue != null ) {
                mDeviceConfigSetsEJBObjectsMap.put( id, returnValue );
            }
        }
        else {
            returnValue = (ConfigurationSet) mDeviceConfigSetsEJBObjectsMap.get( id );
        }

        return returnValue;
    }


    private RefProperty lookupRefProperty ( String code )
            throws FinderException, RemoteException {

        RefProperty refProperty = null;

        Collection refPropertyCollection = mRefPropertyHome.findByCode( code );
        for ( Iterator plcI = refPropertyCollection.iterator(); plcI.hasNext(); ) {
            refProperty = (RefProperty) plcI.next();
        }

        return refProperty;
    }

    /**
     * createSuperadminCredential is used to add the superadmin's user ID
     * and passtoken to the credentails export.   This was added to fix
     * bug# 2953 - it should be removed when we sort our SSO between the
     * web servers (Config and Media Serv.).
     *
     * @return Element with the "item" for the superadmin
     * @throws java.rmi.RemoteException is thrown for system/remoting errors
     * @throws javax.ejb.FinderException when we can't find the superadmin user;
     * really bad thing.
     */
    private Element createSuperadminCredential()
            throws RemoteException, FinderException {

        Collection superC = mUserHome.findByDisplayID("superadmin");
        User superadmin = (User) superC.iterator().next();

        Organization organization =
                getOrganizationEJBObject(superadmin.getOrganizationID());

        String domain = organization.getDNSDomain();
        String authenticationRealm = organization.getAuthenticationRealm();
        String displayID = "superadmin";
        String fullUserID = displayID + "@" + domain;

        Element item = new Element ("item"); // root which we return

        Element uri = new Element ("uri");
        item.addContent(uri);
        uri.addContent(fullUserID);

        Element realm = new Element ("realm");
        realm.addContent(authenticationRealm);
        item.addContent(realm);

        Element userID = new Element ("userid");
        userID.addContent(displayID);
        item.addContent(userID);

        Element password = new Element ("passtoken");
        password.addContent(superadmin.getPassword());
        item.addContent( password );

        Element authType = new Element ( "authtype" );
        authType.addContent( "DIGEST");
        item.addContent ( authType );

        return item;
    }



    /**
     * getAuthExceptions returns a Collection of JDOM Elements each of which contains
     * individual Auth Exception elements.
     *
     * @param userGroup the UserGroup whose Users you want to calcuate the AuthExceptions
     * for.
     * @param forwardingRefPropertyIds a Map containing the list of RefProperty Ids which
     * are significant for forwarding.
     * @param integerOnlyRegExp Regular Expression to detect one version of a valid external
     *  forwarding number.
     * @param localSIPDestinationRegExp another Regular Expression to detect one version
     * of a valid external forwarding number.
     * @return Collection of JDOM Elements with individual Auth Exception elements.
     * @throws RemoteException
     * @throws FinderException
     * @throws PDSException
     * @throws JDOMException
     */
    private Collection getAuthExceptions(   UserGroup userGroup,
                                            Map forwardingRefPropertyIds,
                                            RE integerOnlyRegExp,
                                            RE localSIPDestinationRegExp)
            throws RemoteException, FinderException, PDSException, JDOMException {
        SAXBuilder saxbuilder = new SAXBuilder();

        ArrayList exceptionList = new ArrayList();
        ArrayList configSets = new ArrayList();
        HashMap userGroupConfigSetsMap = new HashMap();
        HashMap userGroupProjectionInputsMap = new HashMap();
        ArrayList projectionInputs = new ArrayList();

        boolean userCanForwardExternal = false;

        // project the UserGroup ancestry of config sets for permissions first,
        // we can then use this cached Map with the permissions for each User.
        ArrayList userGroupConfigSets = new ArrayList();
        userGroupConfigSets.addAll(
                getUserGroupsConfigurationSets( userGroupConfigSetsMap,
                                                userGroup));

        HashMap projectedUserGroupsProperties =
                projectPermissions(userGroupConfigSets, null);

        Collection users = mUserHome.findByUserGroupID(userGroup.getID());

        for (Iterator iUser = users.iterator(); iUser.hasNext(); ) {
            User user = (User) iUser.next();

            configSets.clear();
            projectionInputs.clear();

            ConfigurationSet userCS = getUserCSEJBObject( user.getID() );
            if ( userCS != null ) {
                Document userCSDoc = null;
                try {
                     userCSDoc = saxbuilder.build(new StringReader(userCS.getContent()));
                } catch ( java.io.IOException e ){
                    throw new PDSException( e.toString() );
                }

                configSets.add( userCSDoc );
                logDebug ( "generateAuthExceptions::got users configuration set" );
            }

            HashMap propertySettings =
                    projectPermissions(configSets, projectedUserGroupsProperties);

            if (propertySettings.containsKey("ForwardCallsExternal") ){
                String forwardExternalValue =
                        (String) propertySettings.get("ForwardCallsExternal");
                if (forwardExternalValue != null &&
                        forwardExternalValue.equals("ENABLE")){
                    userCanForwardExternal = true;
                }
            }

            if (userCanForwardExternal) {
                if (!userGroupProjectionInputsMap.containsKey(userGroup.getID())){
                    logDebug("getting projectionInputs for user group: " +
                            userGroup.getID());
                    ProjectionInput projectionResult =
                            getProjectedUserGroupPI(userGroup);

                    userGroupProjectionInputsMap.put(   userGroup.getID(),
                                                        projectionResult);
                }

                projectionInputs.add(userGroupProjectionInputsMap.get(userGroup.getID()));

                ProjectionInput usersProjectionInput =
                        mProjectionHelperEJBObject.getProjectionInput(
                                user,
                                PDSDefinitions.PROF_TYPE_USER);

                if ( usersProjectionInput != null ) {
                    projectionInputs.add( usersProjectionInput );
                    logDebug ("generateAuthExceptions::got users configuration set");
                }

                ProjectionInput projectionResult =
                            mProjectionHelperEJBObject.project(
                                    "com.pingtel.pds.pgs.plugins.projection.StandardTopDown",
                                    projectionInputs,
                                    mXpressaDeviceTypeID,
                                    PDSDefinitions.PROF_TYPE_USER);


                Element projectionRootElement = projectionResult.getDocument().getRootElement();

                exceptionList.addAll(getUsersAuthExceptions(
                        projectionRootElement,
                        forwardingRefPropertyIds,
                        integerOnlyRegExp,
                        localSIPDestinationRegExp));
            }
        }  // for all users


        // process all the child groups of this UserGroup
        Collection childGroups = mUserGroupHome.findByParentID(userGroup.getID());

        for (Iterator iChild = childGroups.iterator(); iChild.hasNext(); ) {
            UserGroup child = (UserGroup) iChild.next();
            exceptionList.addAll(
                    getAuthExceptions(  child,
                                        forwardingRefPropertyIds,
                                        integerOnlyRegExp,
                                        localSIPDestinationRegExp));
        }

        return exceptionList;
    }


    /**
     * getUsersAuthExceptions parses an ConfigurationSet XML document to find validing Auth Exception
     * cases.   These are returned as a Collection of JDOM Elements.
     *
     * @param projectedInputRoot ConfigurationSet XML document to be parsed.
     * @param forwardingRefPropertyIds a Map containing the list of RefProperty Ids which
     * are significant for forwarding.
     * @param integerOnlyRegExp Regular Expression to detect one version of a valid external
     *  forwarding number.
     * @param localSIPDestinationRegExp another Regular Expression to detect one version
     * of a valid external forwarding number.
     * @return Collection of JDOM Elements with individual Auth Exception elements.
     */
    private Collection getUsersAuthExceptions (Element projectedInputRoot, Map forwardingRefPropertyIds,
                                               RE integerOnlyRegExp, RE localSIPDestinationRegExp) {
        ArrayList authExceptions = new ArrayList();

        for ( Iterator contentI = projectedInputRoot.getChildren().iterator();
              contentI.hasNext(); ) {

            Element e = (Element) contentI.next();

            String refPropertyID = e.getAttributeValue( "ref_property_id");

            if (refPropertyID != null &&
                    forwardingRefPropertyIds.containsKey(refPropertyID)) {

                Collection wrapperC = e.getChildren();
                Element wrapper = null;
                for (Iterator iWrap = wrapperC.iterator(); iWrap.hasNext();)
                    wrapper = (Element) iWrap.next();
                    String destination = wrapper.getText();

                    if (integerOnlyRegExp.match(destination)) {
                        // matches digits only
                        Element item = new Element ( "item" );
                        authExceptions.add(item);

                        Element userElement = new Element ("user");
                        userElement.setText(destination);
                        item.addContent( userElement );
                    }  // if length >=7 and digits
                    else if (localSIPDestinationRegExp.match(destination)) {
                        // matches SIP URLs
                        Element item = new Element ( "item" );
                        authExceptions.add(item);

                        Element userElement = new Element ("user");
                        userElement.setText(
                                getDigitsFromForwardDestination(destination));

                        item.addContent( userElement );
                    }
                //} // for
            } // if RP
        } // for all elements in projection result

        return authExceptions;
    }


    /**
     * getProjectedUserGroupPI returns the projected ProjectionInput for a given UserGroup.
     * This ProjectionInput can then be projected with leaf-level Projection Inputs
     * (Users).
     *
     * @param userGroup UserGroup whose projected ProjectionInput you want.
     * @return projected ProjectionInput object for the UserGroup.
     * @throws PDSException
     * @throws RemoteException
     */
    private ProjectionInput getProjectedUserGroupPI(UserGroup userGroup)
            throws PDSException, RemoteException {

        Collection parentGroupsProjectionInputs =
            mProjectionHelperEJBObject.addParentGroupConfigSets(
                    userGroup,
                    PDSDefinitions.PROF_TYPE_USER);

        ArrayList parentProjectionInputsList = new ArrayList();
        parentProjectionInputsList.addAll(parentGroupsProjectionInputs);
        Collections.reverse(parentProjectionInputsList);

        ProjectionInput projectionResult =
                mProjectionHelperEJBObject.project(
                        "com.pingtel.pds.pgs.plugins.projection.StandardTopDown",
                        parentProjectionInputsList,
                        mXpressaDeviceTypeID,
                        PDSDefinitions.PROF_TYPE_USER);
        return projectionResult;
    }


    /**
     * getDigitsFromForwardDestination extracts the digists part of a forwarding
     * destinations.  This is used in the generateAuthExceptions method for
     * forwarding destinations of the form sip:1234567@<domain>.
     *
     * @param forwardingDestination forwarding destination string to extract
     * the digits from.
     * @return just the digists
     */
    private String getDigitsFromForwardDestination (String forwardingDestination) {
        String result = null;
        if (forwardingDestination.toLowerCase().startsWith("sip")) {
            result = forwardingDestination.substring(4);
        }

        int atIndex = result.indexOf('@');
        if (atIndex != -1) {
            result = result.substring(0, atIndex);
        }

        return result;
    }



//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
