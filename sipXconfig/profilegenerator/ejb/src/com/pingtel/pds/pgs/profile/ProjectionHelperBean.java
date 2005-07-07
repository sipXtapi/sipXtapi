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

package com.pingtel.pds.pgs.profile;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.TemplatesCache;
import com.pingtel.pds.pgs.common.MasterDetailsMap;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ProfileEncryptionCache;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.CSProfileDetail;
import com.pingtel.pds.pgs.phone.CSProfileDetailHome;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.user.*;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.transform.JDOMSource;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.xml.transform.Result;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamResult;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.StringReader;
import java.rmi.RemoteException;
import java.security.GeneralSecurityException;
import java.util.*;

/**
 * ProjectionHelperBean is the bean implementation class for the ProjectionHelper
 * EJ Bean.   ProjectionHelper, as the name suggests, acts as a service provider
 * to other session beans, projection configuration sets and such.
 *
 * @author ibutcher
 *
 */
public class ProjectionHelperBean extends JDBCAwareEJB implements SessionBean, ProjectionHelperBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Home interface objects
    private DeviceGroupHome mDeviceGroupHome;
    private UserGroupHome mUserGroupHome;
    private ConfigurationSetHome mConfigurationSetHome;
    private CSProfileDetailHome mCSProfileDetailHome;
    private RefPropertyHome mRefPropertyHome;
    private DeviceTypeHome mDeviceTypeHome;
    private UserHome mUserHome;
    private DeviceHome mDeviceHome;

    // Stateless Session Bean refs
    private RenderProfile mRenderProfileBean;
    private RefDataAdvocate mRefDataAdvocateEJBObject;

    //The Session Context object
    private SessionContext mCtx;

    // Misc.
    private String mXsltBasePath;
    private MasterDetailsMap mXsltRenderSheetsMap = new MasterDetailsMap();
    private SAXBuilder mSaxBuilder = new SAXBuilder();


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

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
        mCtx = ctx;

        try {
            Context initial = new InitialContext();

            mDeviceGroupHome = (DeviceGroupHome) initial.lookup("DeviceGroup");
            mUserGroupHome = (UserGroupHome) initial.lookup("UserGroup");
            mConfigurationSetHome = (ConfigurationSetHome)
                    initial.lookup("ConfigurationSet");

            RenderProfileHome renderProfileHome =
                    (RenderProfileHome) initial.lookup("RenderProfile");
            RefDataAdvocateHome refDataAdvocateHome =
                (RefDataAdvocateHome) initial.lookup("RefDataAdvocate");

            mCSProfileDetailHome = (CSProfileDetailHome)
                    initial.lookup("CSProfileDetail");
            mRefPropertyHome = (RefPropertyHome) initial.lookup( "RefProperty" );
            mDeviceTypeHome = (DeviceTypeHome) initial.lookup( "DeviceType" );

            mUserHome = (UserHome) initial.lookup ("User");
            mDeviceHome = (DeviceHome) initial.lookup ( "Device" );

            mRenderProfileBean = renderProfileHome.create();
            mRefDataAdvocateEJBObject = refDataAdvocateHome.create();
        }
        catch ( CreateException e ) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        } catch (NamingException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        } catch (RemoteException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }
    }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void unsetSessionContext() {
        mCtx = null;
    }


    /**
     * projectAndPersist performs the 'projection' operation on the given set
     * of ProjectionInputs, transforms the output of this process using
     * XSLT and then sends the results to the ProfileWriter.
     *
     * @param projectionClassName fully-qualified class name to use for the
     * projection process.
     * @param device EJBObject for the Device whose profile(s) you want to
     * project.
     * @param profileType comma-separated list of profile types that you
     * want to project.
     * @param propertySets Collection of ProjectionInput Objects.
     * @throws PDSException is thrown for all application level errors.
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    public void projectAndPersist(  String projectionClassName,
                                    Device device,
                                    int profileType,
                                    Collection propertySets)
             throws RemoteException, PDSException {

        try {
            ProjectionInput input = project(projectionClassName,
                    profileType,
                    device,
                    propertySets);

            StringBuffer extraContent = new StringBuffer();
            String userId = device.getUserID();
            User owner = null;

            if(userId != null){
                owner = mUserHome.findByPrimaryKey(userId);
            }

            if(device.getManufaturerName().equalsIgnoreCase(
                            PDSDefinitions.MANU_PINGTEL)){

                if(profileType == PDSDefinitions.PROF_TYPE_PHONE){
                    extraContent.append("PHONESET_DEFAULT_USER : ");

                    if(owner != null){
                        extraContent.append(owner.getDisplayID());
                    }

                } else if (profileType == PDSDefinitions.PROF_TYPE_USER){
                    extraContent.append("USER_PROFILE : " +
                            owner.getDisplayID());
                }
            }

            byte[] transformedProfile =
                    transformProfile(   input.getDocument(),
                                        device,
                                        profileType,
                                        extraContent.toString());

            if(device.getModel().equals(PDSDefinitions.MODEL_SOFTPHONE_WIN) &&
                    ProfileEncryptionCache.getInstance().shouldEncryptProfiles(device) &&
                    owner != null){

                IxProfileEncrypter ixEncrypter =
                        new IxProfileEncrypter(
                                owner.getProfileEncryptionKey());

                byte [] cipheredProfile =
                        new byte [ixEncrypter.getCipherTextLength(
                                transformedProfile.length)];

                cipheredProfile =
                        ixEncrypter.encryptProfile(transformedProfile);

                mRenderProfileBean.writeProfile(    cipheredProfile,
                                                    device,
                                                    profileType);

            } else {
                mRenderProfileBean.writeProfile(    transformedProfile,
                                                    device,
                                                    profileType);
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }
        catch (GeneralSecurityException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }
    }


    /**
     * project performs the projection operation on a Collection of
     * ProjectionInput Objects.
     *
     * @param projectionClassName fully-qualified class name to use
     * for the projection process.
     * @param propertySets Collection of ProjectionInput Objects.
     * @param deviceTypeID PK of the DeviceType of the Device
     * where the resulting ProjectionInput object will be ultimately
     * delivered.
     * @param profileType type of profile the projection is being
     * projected.
     * @return a ProjectionInput which not contains the 'Projected'
     * values from the Collection of input ProjectionInputs.
     * @throws PDSException for application errors
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    public ProjectionInput project( String projectionClassName,
                                    Collection propertySets,
                                    Integer deviceTypeID,
                                    int profileType ) throws RemoteException, PDSException {

        Collection validRPIDs = null;
        Projection projAlg = null;
        try {
            logDebug ( "projecting using " + projectionClassName );

            validRPIDs = null;

            if ( deviceTypeID != null ) {
                validRPIDs = getRPIDsForDeviceType ( deviceTypeID, profileType );
            }
            else {
                validRPIDs = new HashSet();

                Collection allDeviceTypes = mDeviceTypeHome.findAll();
                for ( Iterator dtI = allDeviceTypes.iterator(); dtI.hasNext(); ) {
                    DeviceType dt = (DeviceType) dtI.next();
                    validRPIDs.addAll( getRPIDsForDeviceType ( dt.getID(), profileType ) );
                }
            }

            if (projectionClassName == null) {
                mCtx.setRollbackOnly();

                throw new PDSException(collateErrorMessages("UC190",
                        "E3006",
                        null));
            }

            projAlg = instantiateProjection(projectionClassName);

            logDebug ( "instantiated projection object" );
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }

        return projAlg.project(propertySets, validRPIDs );
    }


    /**
     * calculateTotalProfiles works out how many Users belong to the given
     * UserGroup and all of its child UserGroups.
     * @param userGroup UserGroup whose total number of Users to be calculated
     * @return the total number of Users contained in the given group and all
     * of its child groups.
     */
    public int calculateTotalProfiles (UserGroup userGroup) throws RemoteException {

        int total = 0;

        try {
            Collection thisGroupsUsers = mUserHome.findByUserGroupID(userGroup.getID());
            total += thisGroupsUsers.size();

            Collection thisGroupsGroups = mUserGroupHome.findByParentID(userGroup.getID());
            for ( Iterator i = thisGroupsGroups.iterator(); i.hasNext(); ) {
                UserGroup child = (UserGroup) i.next();
                total += calculateTotalProfiles (child);
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }

        return total;
    }

    /**
     * calculateTotalProfiles works out how many Devices belong to the given
     * DeviceGroup and all of its child DeviceGroups.
     * @param deviceGroup the DeviceGroup for whom you wish to calculate
     * the number of contained Devices.
     * @return the total number of Devices contained in the given group and all
     * of its child groups.
     */
    public int calculateTotalProfiles (DeviceGroup deviceGroup) throws RemoteException {

        int total = 0;

        try {
            Collection thisGroupsDevices = null;
            thisGroupsDevices = mDeviceHome.findByDeviceGroupID(deviceGroup.getID());

            total += thisGroupsDevices.size();

            Collection thisGroupsGroups = null;
            thisGroupsGroups = mDeviceGroupHome.findByParentID(deviceGroup.getID());

            for ( Iterator i = thisGroupsGroups.iterator(); i.hasNext(); ) {
                DeviceGroup child = (DeviceGroup) i.next();
                total += calculateTotalProfiles ( child );
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }


        return total;
    }


    /**
     * returns the full-qualified class name which is the default
     * projection algorithm for the given Device's Device Type.
     * @param device EJBObject of the Device that you want the
     * projection class name for.
     * @param profileType projection algorithms are specifed at the
     * DeviceType * profile type level.
     * @return fully-qualified class name.
     */
    public String getProjectionClassName(Device device, int profileType) throws RemoteException {

        String projectionClassName = null;
        Integer wrappedProfType = new Integer(profileType);
        Integer cswID = null;

        try {
            logDebug ( "getting projection class name for device: " + device.getExternalID() +
                    " profile type: " + profileType );
            CSProfileDetail cspd = null;
            Collection cspdC = null;
            cswID = device.getCoreSoftwareDetailsID();
            logDebug ( "get core software id: " + cswID );

            cspdC = mCSProfileDetailHome.findByCoreSoftwareAndProfileType(cswID, wrappedProfType);

            logDebug ( "found cs detail record" );

            for (Iterator cspdI = cspdC.iterator(); cspdI.hasNext(); ) {
                cspd = (CSProfileDetail) cspdI.next();
            }

            projectionClassName = cspd.getProjectionClassName();
            logDebug ( "projection class name is: " +  projectionClassName );
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }


        return projectionClassName;
    }


    /**
     * addParentGroupConfigSets returns a Collection the ProjectionInput
     * associated with the given UserGroup and those of the groups parent
     * UserGroups.
     * @param userGroup EJBObject of the UserGroup whose ProjectionInputs
     * you want.
     * @param profileType Not Used really!!  Refactor.
     * @return Collection of ProjectionInputs.
     */
    public Collection addParentGroupConfigSets( UserGroup userGroup,
            	int profileType) throws RemoteException, PDSException {

        ArrayList configSets = new ArrayList();
        Integer parentGroupID = null;
        UserGroup parent = null;

        try {
            ProjectionInput pi = getProjectionInput(userGroup, profileType);

            if (pi != null) {
                configSets.add(pi);
            }

            parentGroupID = userGroup.getParentID();

            if (parentGroupID != null) {
                parent = mUserGroupHome.findByPrimaryKey(parentGroupID);
                configSets.addAll(addParentGroupConfigSets(parent, profileType));
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }


        return configSets;
    }


    /**
     * addParentGroupConfigSets returns a Collection the ProjectionInput
     * associated with the given DeviceGroup and those of the groups parent
     * DeviceGroups.
     * @param deviceGroup EJBObject of the DeviceGroup whose ProjectionInputs
     * you want.
     * @return Collection of ProjectionInputs.
     */
    public Collection addParentGroupConfigSets(DeviceGroup deviceGroup) throws RemoteException, PDSException{

        ArrayList configSets = new ArrayList();
        Integer parentGroupID = null;
        DeviceGroup parent = null;

        try {
            ProjectionInput pi = getProjectionInput(deviceGroup);

            if (pi != null) {
                configSets.add(pi);
            }

            parentGroupID = deviceGroup.getParentID();

            if (parentGroupID != null) {
                parent = mDeviceGroupHome.findByPrimaryKey(parentGroupID);
                configSets.addAll(addParentGroupConfigSets(parent));
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }

        return configSets;
    }

    /**
     * getProjectionInput returns the ProjectionInput object for the
     * given User.
     * @param user EJBObject for the User whose ProjectionInput you want.
     * @param profileType Users effectively have two type of ProjectionInputs.
     * The User profile and the Application.   The User profile settings are
     * stored in the User's ConfigurationSet, the Application assignments are
     * calculated at run-time.
     * @return ProjectionInput for the User.
     */
    public ProjectionInput getProjectionInput(User user, int profileType) 
    	throws RemoteException, PDSException {

        ConfigurationSet cs = null;
        Document doc = null;
        Collection projectionRules = null;
        String userID = null;
        ProjectionInput returnValue = null;

        try {
            logDebug ( "getting projectionInput for user: " + user.getExternalID() );
            userID = user.getID();

            if ( profileType == PDSDefinitions.PROF_TYPE_USER ) {

                Collection c = mConfigurationSetHome.findByUserIDAndProfileType(userID, profileType);
                for (Iterator iC = c.iterator(); iC.hasNext(); ) {
                    cs = (ConfigurationSet) iC.next();
                }

                if (cs != null) {
                    projectionRules = getProjectionRules(cs);
                    doc = getDocFromConfigSetContent(cs);

                    returnValue = new ProjectionInput(doc, projectionRules);
                }
            }
            else {
                Element profile = new Element ( "PROFILE" );

                projectionRules = getProjectionRules( user.getRefConfigSetID() );
                doc = new Document ( profile );

                returnValue = new ProjectionInput(doc, projectionRules);

            } // else
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }

        return returnValue;
    }

    /**
     * getProjectionInput returns the ProjectionInput object for the
     * given Device.
     * @param device EJBObject of the Device whose ProjectionInput
     * you want.
     * @return ProjectionInput for the Device.
     */
    public ProjectionInput getProjectionInput(Device device) throws RemoteException, PDSException {

        ConfigurationSet cs = null;
        Document doc = null;
        Collection projectionRules = null;
        ProjectionInput returnValue = null;

        try {
            logDebug ( "getting projectionInput for device: " + device.getExternalID() );

            Collection c = mConfigurationSetHome.findByLogicalPhoneID(device.getID());
            for (Iterator iC = c.iterator(); iC.hasNext(); ) {
                cs = (ConfigurationSet) iC.next();
            }

            if (cs != null) {
                logDebug ( "configuration set not null" );
                projectionRules = getProjectionRules(cs);
                logDebug ( "added projection rules" );
                doc = getDocFromConfigSetContent(cs);
                logDebug ( "got content" );
                returnValue = new ProjectionInput(doc, projectionRules);
            }
        }
        catch (FinderException e) {
            logFatal ( e.toString(), e  );
            throw new EJBException ( e );
        }

        return returnValue;
    }





//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * project performs the projection operation on a Collection of
     * ProjectionInput Objects for the given Device.
     *
     * @param projectionClassName fully-qualified class name to use
     * for the projection process.
     * @param profileType type of profile the projection is being
     * projected.
     * @param device Device for whom you wish to project.
     * @param propertySets Collection of ProjectionInput Objects.
     * @return a ProjectionInput which not contains the 'Projected'
     * values from the Collection of input ProjectionInputs.
     * @throws PDSException
     * @throws RemoteException
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    private ProjectionInput project(    String projectionClassName,
                                        int profileType,
                                        Device device,
                                        Collection propertySets)
        throws PDSException, RemoteException {

        logDebug ( "projecting using " + projectionClassName );

        Collection validRefPropIDs =
            getRPIDsForDeviceType ( device.getDeviceTypeID(), profileType );

        Projection projAlg = null;
        if (projectionClassName == null) {
            projectionClassName = getProjectionClassName(device, profileType);

            projAlg = instantiateProjection(projectionClassName);
            logDebug ( "instantiated projection class" );
        } else {
            projAlg = instantiateProjection(projectionClassName);
        }

        return projAlg.project(propertySets, validRefPropIDs );
    }


    private byte[] transformProfile( Document profile,
                                    Device device,
                                    int profileType,
                                    String includedContent )
        throws PDSException, RemoteException {

        Integer deviceID = null;
        String styleSheetFileName = null;

        try {
            deviceID = device.getID();
            logDebug ( "tranforming profile for: " + device.getExternalID() );

            String xsltRendererURL = null;

            try {
                xsltRendererURL = getRendererStylesheetName ( device, profileType );
            }
            catch (FinderException ex) {
                mCtx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages (  "UC120",
                                            "E1014",
                                            new Object [] { device.getCoreSoftwareDetailsID(),
                                                            new Integer (profileType) } ),
                    ex );
            }


            // get various sources and Xform.
            JDOMSource docSource = new JDOMSource ( profile );

            if ( mXsltBasePath == null ) {
                mXsltBasePath =
                    PathLocatorUtil.getInstance().
                                getPath( PathLocatorUtil.PHONEDEFS_FOLDER, PathLocatorUtil.PGS );
            }

            Transformer transformer =
                TemplatesCache.getInstance().
                    newTransformer( mXsltBasePath + xsltRendererURL );

            transformer.setParameter( "includedContent", includedContent );

            ByteArrayOutputStream resultStream = new ByteArrayOutputStream();
            Result result = new StreamResult ( resultStream );

            transformer.transform( docSource, result );

            return resultStream.toByteArray();
        }
        catch ( TransformerException ex) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages (  "UC120",
                                            "E7005",
                                            new Object [] { deviceID,
                                                            new Integer (profileType) } ),
                    ex );
        }
        catch ( FileNotFoundException ex) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages (  "UC120",
                                            "E7006",
                                            new Object [] { styleSheetFileName } ),
                    ex );
        }
    }




    ///////////////////////////////////////////////////////////////////////
    //
    // The projection plug-ins take a collection of ref property ids which
    // are valid for a particular device type.   They are used to filter-out
    // settings in configuration sets which pertain to other device types.
    //
    ///////////////////////////////////////////////////////////////////////
    private Collection getRPIDsForDeviceType (  Integer deviceTypeID,
                                                int profileType )
        throws PDSException, RemoteException {

        ArrayList validRefPropIDs = new ArrayList();
        Collection validRefProperties = null;

        validRefProperties =
                mRefDataAdvocateEJBObject.getRefPropertiesForDeviceTypeAndProfile(
                        deviceTypeID, new Integer ( profileType ) );

        for ( Iterator i = validRefProperties.iterator(); i.hasNext(); ) {
            RefProperty rp = (RefProperty) i.next();
            validRefPropIDs.add( rp.getID() );
        }

        return validRefPropIDs;
    }



    private String getRendererStylesheetName ( Device device, int profileType )
        throws FinderException, RemoteException {

        String sheetName = null;
        Integer coreSW = device.getCoreSoftwareDetailsID();
        Integer profileInteger = new Integer( profileType );

        if ( !mXsltRenderSheetsMap.contains(   coreSW,
                                                profileInteger ) ) {

            CSProfileDetail cspd = null;

            Collection cspdCollection =
                    mCSProfileDetailHome.findByCoreSoftwareAndProfileType(
                        coreSW,
                        profileInteger );

            for (Iterator cspdI = cspdCollection.iterator(); cspdI.hasNext(); ) {
                cspd = (CSProfileDetail) cspdI.next();
            }

            mXsltRenderSheetsMap.storeDetail(  coreSW,
                                                profileInteger,
                                                cspd.getXSLTURL() );
        }

        sheetName = (String) mXsltRenderSheetsMap.getDetail( coreSW, profileInteger );

        return sheetName;
    }






    /**
     *  Description of the Method
     *
     *@param  projectionName        Description of the Parameter
     *@return                       Description of the Return Value
     */
    private Projection instantiateProjection(String projectionName) {

      logDebug ( "instantiating projection class for: " + projectionName );
      try {
        return (Projection) Class.forName(projectionName).newInstance();
      }
      catch (ClassNotFoundException ce) {
        logFatal( ce.toString(), ce);

        throw new EJBException(
            collateErrorMessages("UC190",
            "E4033",
            null), ce);
      }
      catch (IllegalAccessException ce) {
          logFatal( ce.toString(), ce);

          throw new EJBException(
              collateErrorMessages("UC190",
              "E4033",
              null), ce);
        }
      catch (InstantiationException ce) {
          logFatal( ce.toString(), ce);

          throw new EJBException(
              collateErrorMessages("UC190",
              "E4033",
              null), ce);
        }
    }



    ///////////////////////////////////////////////////////////////////////
    //
    // a ProjectionInput object is just a wrapper around a JDOM Document
    // and a Collection of ProjectionRule objects for a particular
    // configuration set.   This method gets the ProjectionInput for a
    // given User Group and profile type.   Note: users and user groups may
    // have a profile for User type data and one for Application type data.
    //
    ///////////////////////////////////////////////////////////////////////

    private ProjectionInput getProjectionInput(UserGroup ug, int profileType)
             throws RemoteException, PDSException {

        ConfigurationSet cs = null;
        Document doc = null;
        Collection projectionRules = null;
        Integer userGroupID = null;
        ProjectionInput returnValue = null;

        try {
            userGroupID = ug.getID();

            if ( profileType == PDSDefinitions.PROF_TYPE_USER ) {
                Collection c = mConfigurationSetHome.findByUserGroupIDAndProfileType(userGroupID, profileType);
                for (Iterator iC = c.iterator(); iC.hasNext(); ) {
                    cs = (ConfigurationSet) iC.next();
                }

                if (cs != null) {
                    projectionRules = getProjectionRules(cs);
                    doc = getDocFromConfigSetContent(cs);
                    returnValue = new ProjectionInput(doc, projectionRules);
                }
            }
            else { // PROF_TYPE_APPLICATION_REF
                Element profile = new Element ( "PROFILE" );
                projectionRules = getProjectionRules( ug.getRefConfigSetID() );
                doc = new Document ( profile );

                returnValue = new ProjectionInput(doc, projectionRules);
            } // else


            return returnValue;
        }
        catch (FinderException ex) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC190",
                    "E1031",
                    new Object[]{userGroupID}),
                    ex);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // a ProjectionInput object is just a wrapper around a JDOM Document
    // and a Collection of ProjectionRule objects for a particular
    // configuration set.   This method gets the ProjectionInput for a
    // given Device Group.
    //
    ///////////////////////////////////////////////////////////////////////
    /**
     *  Gets the projectionInput attribute of the ProjectionHelperBean object
     *
     *@param  pg                    Description of the Parameter
     *@return                       The projectionInput value
     *@exception  PDSException  Description of the Exception
     */
    private ProjectionInput getProjectionInput(DeviceGroup pg)
             throws PDSException, RemoteException {

        ConfigurationSet cs = null;
        Document doc = null;
        Collection projectionRules = null;
        ProjectionInput returnValue = null;
        Integer pgID = null;

        try {
            pgID = pg.getID();
            Collection c = mConfigurationSetHome.findByDeviceGroupID(pg.getID());
            for (Iterator iC = c.iterator(); iC.hasNext(); ) {
                cs = (ConfigurationSet) iC.next();
            }

            if (cs != null) {
                projectionRules = getProjectionRules(cs);
                doc = getDocFromConfigSetContent(cs);
                returnValue = new ProjectionInput(doc, projectionRules);
            }
        } catch (FinderException ex) {
            mCtx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC190",
                    "E1010",
                    new Object[]{pgID}),
                    ex);
        }

        return returnValue;
    }


    private Document getDocFromConfigSetContent(ConfigurationSet cs)
             throws PDSException, RemoteException {

        try {
            return mSaxBuilder.build(new StringReader(cs.getContent()));
        }
        catch ( JDOMException ex) {
            mCtx.setRollbackOnly();

            Integer csID = null;
            csID = cs.getID();

            throw new PDSException(
                    collateErrorMessages("UC190",
                    "E3004",
                    new Object[]{csID}),
                    ex);
        }
        catch ( IOException ex) {
            mCtx.setRollbackOnly();

            Integer csID = null;
            csID = cs.getID();

            throw new PDSException(
                    collateErrorMessages("UC190",
                    "E3004",
                    new Object[]{csID}),
                    ex);
        }
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // ProjectionRules define the metadata associated for a particular
    // ref property in a particular RCS, is it final or read-only.
    // This method finds all of the projection rules for a particular
    // configuration set.
    //
    ///////////////////////////////////////////////////////////////////////
    private Collection getProjectionRules( ConfigurationSet cs)
             throws RemoteException, PDSException {

        Collection projectionRules = null;

        if (cs != null) {
            Integer refCsId = cs.getRefConfigSetID();
            projectionRules = mRefDataAdvocateEJBObject.getRefConfigSetsProperties(
                refCsId.toString() );
        }

        return projectionRules;
    }


    private Collection getProjectionRules( Integer rcsID )
             throws RemoteException, PDSException {

        Collection projectionRules = null;

        if (rcsID != null) {
            projectionRules =  mRefDataAdvocateEJBObject.getRefConfigSetsProperties(rcsID.toString());
        }
        return projectionRules;
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
