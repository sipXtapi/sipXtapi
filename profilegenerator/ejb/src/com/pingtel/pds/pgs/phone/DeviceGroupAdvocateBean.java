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

package com.pingtel.pds.pgs.phone;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.RemoveException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.profile.ProfileTypeStringParser;
import com.pingtel.pds.pgs.profile.ProjectionHelper;
import com.pingtel.pds.pgs.profile.ProjectionHelperHome;
import com.pingtel.pds.pgs.profile.ProjectionInput;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;
import com.pingtel.pds.pgs.jobs.JobManager;
import com.pingtel.pds.pgs.jobs.JobManagerHome;

/**
 *  EJB Implementation class for DeviceGroupAdvocate.  It is
 * essentially the session facade for all Device Group (entity)
 * use cases.
 *
 *@author     ibutcher
 */
public class DeviceGroupAdvocateBean extends JDBCAwareEJB
         implements SessionBean, DeviceGroupAdvocateBusiness {

    //home interface references
    private DeviceGroupHome m_deviceGroupHome;
    private ConfigurationSetHome m_configSetHome;
    private DeviceHome m_deviceHome;
    private UserHome m_userHome;

    // Stateless Session Bean references
    private DeviceAdvocate m_deviceAdvocateEJBObject;
    private UserAdvocate m_userAdvocateEJBObject;
    private ProjectionHelper m_projectionHelperEJBObject;
    private JobManager m_jobManagerEJBObject;

    //The Session Context object
    private SessionContext m_ctx;


    /**
     * createDeviceGroup implements the create Device Group use
     * case.
     *
     *@param organizationID Organization PK (entity) that this
     * Device Group will belong to.
     *@param refConfigSetID Reference Configuration Set PK (entity)
     * which will be associated with the Device Group.
     *@param parentID PK of an existing Device Group (entity) which
     * you wish to become the parent (be the node immediately above) the
     * new Device Group.
     *@param childGroupIDs a comma-separeted list of Device Group PK
     * IDs. Each of these Device Groups will have the new Device Group become
     * their parent.
     *@param groupName name that the group will be known by.
     *@return The new Device Group's EJBObject.
     *@exception  PDSException  is thrown if there is an application level
     * errors.
     */
    public DeviceGroup createDeviceGroup(   String organizationID,
                                            String groupName,
                                            String refConfigSetID,
                                            String parentID,
                                            String childGroupIDs)
        throws PDSException {

        return createDeviceGroup(   new Integer ( organizationID ),
                                    groupName,
                                    new Integer ( refConfigSetID ),
                                    parentID != null ? new Integer ( parentID ) : null,
                                    childGroupIDs );
    }


    /**
     * NOTE: This is the 'local' implementation.  The web UI should
     * use the other version of this method.
     *
     * createDeviceGroup implements the create Device Group use
     * case.
     *
     *@param organizationID Organization PK (entity) that this
     * Device Group will belong to.
     *@param refConfigSetID Reference Configuration Set PK (entity)
     * which will be associated with the Device Group.
     *@param parentID PK of an existing Device Group (entity) which
     * you wish to become the parent (be the node immediately above) the
     * new Device Group.
     *@param childGroupIDs a comma-separeted list of Device Group PK
     * IDs. Each of these Device Groups will have the new Device Group become
     * their parent.
     *@param groupName name that the group will be known by.
     *@return The new Device Group's EJBObject.
     *@exception  PDSException  is thrown if there is an application level
     * errors.
     */
    public DeviceGroup createDeviceGroup(   Integer organizationID,
                                            String groupName,
                                            Integer refConfigSetID,
                                            Integer parentID,
                                            String childGroupIDs)
        throws PDSException {
        DeviceGroup pg = null;
        String newChildID = null;

        try {
            checkSiblingUnique ( parentID, groupName );

            pg = m_deviceGroupHome.create( groupName, organizationID, refConfigSetID );
            pg.setParentID( parentID );

            if (childGroupIDs != null) {
                StringTokenizer st = new StringTokenizer(childGroupIDs, ",");
                DeviceGroup newChild = null;

                try {
                    while (st.hasMoreElements()) {
                        newChildID = st.nextToken();
                        newChild = m_deviceGroupHome.findByPrimaryKey(new Integer(newChildID));
                        newChild.setParentID(pg.getID());
                    }

                } catch (FinderException ex) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC435",
                                                "E1003",
                                                new Object []{ newChildID}),
                        ex);
                }
            }

            logTransaction ( m_ctx, "Created device group " + pg.getExternalID() );
        }
        catch ( PDSException ex) {
            // We are catching PDS exceptions thrown from checkSiblingUnique
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC435"), ex );
        }
        catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC435",
                                        "E2005",
                                        new Object []{ groupName } ),
                ce);
        }
        catch ( FinderException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC435",
                                        "E2005",
                                        new Object []{ groupName } ),
                ce);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                collateErrorMessages(   "UC435",
                                        "E4008",
                                        new Object[]{   groupName,
                                                        organizationID,
                                                        refConfigSetID,
                                                        parentID,
                                                        childGroupIDs}));
        }

        return pg;
    }



    /**
     * deleteDeviceGroup implements the delete Device Group use case.
     *
     * @param deviceGroupID the PK of the DeviceGroup that you want
     * to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newDeviceGroupID (unused, only use null)
     * @throws PDSException for application level errors.
     */
    public void deleteDeviceGroup(  String deviceGroupID,
                                    String option,
                                    String newDeviceGroupID)
        throws PDSException {

        DeviceGroup dg = null;

        try {
            dg = m_deviceGroupHome.findByPrimaryKey( new Integer ( deviceGroupID ));
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC440",
                                        "E1003",
                                        new Object[]{deviceGroupID}),
                ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex  );

            throw new EJBException(
                collateErrorMessages(   "UC440",
                                        "E4009",
                                        new Object[]{ deviceGroupID } ) );
        }


        deleteDeviceGroup(  dg,
                            new Integer ( option ).intValue(),
                            newDeviceGroupID != null ?
                                new Integer ( newDeviceGroupID ) :
                                null );
    }

    /**
     * NOTE: This is a 'local' method.   The web UI should use the
     * other version of this method.
     *
     * deleteDeviceGroup implements the delete Device Group use case.
     *
     * @param deviceGroup the EJBObject of the DeviceGroup that you want
     * to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newDeviceGroupID (unused, only use null)
     * @throws PDSException for application level errors.
     */
    public void deleteDeviceGroup(  DeviceGroup deviceGroup,
                                    int option,
                                    Integer newDeviceGroupID)
             throws PDSException {

        try {
            String externalID = deviceGroup.getExternalID();

            if ( option == PGSDefinitions.DEL_DEEP_DELETE ) {

                Collection children = null;

                try {
                    children = m_deviceGroupHome.findByParentID( deviceGroup.getID() );
                }
                catch (FinderException ex) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC440",
                                                "E1015",
                                                new Object[]{ deviceGroup.getExternalID() }),
                        ex);
                }

                for (Iterator iChild = children.iterator(); iChild.hasNext(); ) {
                    DeviceGroup pgChild = (DeviceGroup) iChild.next();

                    deleteDeviceGroup(  pgChild,
                                        option,
                                        null);
                }

            }
            // if

            deleteDeviceGroupConfigSets( deviceGroup );

            switch ( option) {
                case PGSDefinitions.DEL_DEEP_DELETE:
                {
                    deepDelete( deviceGroup );
                    break;
                }
                case PGSDefinitions.DEL_NO_GROUP:
                {
                    reassignDelete( deviceGroup, null);
                    break;
                }
                case PGSDefinitions.DEL_REASSIGN:
                {
                    reassignDelete( deviceGroup, newDeviceGroupID);
                    break;
                }
            }
            // switch

            try {
                // delete this phone group.
                deviceGroup.remove();
            }
            catch (RemoveException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC440",
                        "E6004",
                        new Object[]{ deviceGroup.getExternalID()}),
                        ex);
            }

            logTransaction ( m_ctx, "Deleted device group " + externalID );

        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String dgExternal = null;
            try { dgExternal = deviceGroup.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC440",
                                        "E4009",
                                        new Object[]{ dgExternal } ) );
        }

    }



    private void deleteDeviceGroupConfigSets(DeviceGroup deviceGroup )
        throws PDSException {

        try {
            try {
                Collection c =
                    m_configSetHome.findByDeviceGroupID( deviceGroup.getID() );

                for (Iterator i = c.iterator(); i.hasNext(); ) {
                    ((ConfigurationSet) i.next()).remove();
                }
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC440",
                                            "E1010",
                                            new Object[]{ deviceGroup.getExternalID() }),
                    ex);
            }
            catch (RemoveException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC440",
                                            "E6005",
                                            new Object[]{deviceGroup.getExternalID()}),
                    ex);
            }
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String dgExternal = null;
            try { dgExternal = deviceGroup.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC440",
                                        "E4009",
                                        new Object[]{ dgExternal } ) );
        }
    }



    private void deepDelete(DeviceGroup deviceGroup)
        throws PDSException, RemoteException {

        try {
            Collection existing =
                m_deviceHome.findByDeviceGroupID( deviceGroup.getID() );

            for (Iterator iPhone = existing.iterator(); iPhone.hasNext(); ) {
                m_deviceAdvocateEJBObject.deleteDevice( (Device) iPhone.next() );
            }
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC440",
                                        "E1011",
                                        new Object[]{deviceGroup.getExternalID() }),
                ex);
        }
    }


    private void reassignDelete(    DeviceGroup deviceGroup,
                                    Integer newDeviceGroupID)
        throws PDSException, RemoteException {


        Collection existing = null;
        try {
            existing = m_deviceHome.findByDeviceGroupID( deviceGroup.getID() );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC440",
                                        "E1011",
                                        new Object[]{deviceGroup.getExternalID()}),
                ex);
        }

        for (Iterator iPhone = existing.iterator(); iPhone.hasNext(); ) {
            Device lp = (Device) iPhone.next();
            lp.setDeviceGroupID(newDeviceGroupID);
        }

        Collection children = null;

        try {
            children =
                m_deviceGroupHome.findByParentID(deviceGroup.getID());
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC440",
                                        "E1015",
                                        new Object[]{deviceGroup.getExternalID()}),
                ex);
        }

        for (Iterator iChild = children.iterator(); iChild.hasNext(); ) {
            ((DeviceGroup) iChild.next()).setParentID(newDeviceGroupID);
        }
    }


    /**
     * editDeviceGroup implements the edit Device Group use case.
     * All of the parameters are optional except id.   If a null
     * value is provided for any of the optional values then one
     * of two things happens.  If the existing value is null then
     * nothing happens.  If the value is non-null then it is set
     * to null.
     *
     * @param id PK of the Device Group that you want to edit.
     * @param parentDeviceGroupID the PK of the Device Group that
     * you want to assign as the Device Groups parent (optional).
     * @param name the name by which the Device Group should be
     * known (optional).
     * @param refConfigSetID the PK of the RefConfigurationSet (entity)
     * that should be assigned to the DeviceGroup (optional).
     * @throws PDSException
     */
    public void editDeviceGroup(    String id,
                                    String parentDeviceGroupID,
                                    String name,
                                    String refConfigSetID)
        throws PDSException {

        DeviceGroup pg = null;

        try {
            pg = m_deviceGroupHome.findByPrimaryKey( new Integer(id) );

            Integer parentGroupIDInteger =
                parentDeviceGroupID == null ? null : Integer.valueOf( parentDeviceGroupID );

            Integer existingParentID = pg.getParentID();

            if ( ( parentDeviceGroupID != null &&
                    ( existingParentID == null || !( parentGroupIDInteger.equals( existingParentID ) ) ) ) ||
                  parentDeviceGroupID == null && existingParentID != null ) {

                ///////////////////////////////////////////////////////////////////////////////
                //
                // We are moving the device group to another parent, we need to make sure that
                // the new parent group doesn't have an existing sibling with the same name.
                //
                ///////////////////////////////////////////////////////////////////////////////
                checkSiblingUnique( parentGroupIDInteger, name );

                pg.setParentID( parentGroupIDInteger );
            }

            String existingName = pg.getName();

            if ( name != null && ( existingName == null || !( name.equals( existingName ) ) ) ) {
                checkSiblingUnique( parentGroupIDInteger, name );
                pg.setName(name);
            }

            Integer existingRCSID = pg.getRefConfigSetID();

            if (refConfigSetID != null &&
                    ( existingRCSID == null && !(refConfigSetID.equals( existingRCSID.toString() ) ) ) ) {

                pg.setRefConfigSetID( Integer.valueOf( refConfigSetID ) );
            }
        }
        catch ( PDSException ex) {
            // We are catching PDS exceptions thrown from checkSiblingUnique
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC448"), ex );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC448",
                                        "E1003",
                                        new Object[]{ name }),
                ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC448",
                                        "E4010",
                                        new Object[]{   id,
                                                        parentDeviceGroupID,
                                                        name,
                                                        refConfigSetID}));
        }
    }


    private void checkSiblingUnique( Integer parentID, String name )
            throws FinderException, RemoteException, PDSException {

        Collection peers = new ArrayList();

        if ( parentID != null ) {
            peers = m_deviceGroupHome.findByParentID( parentID );
        }
        else { // adding/moving group at top level
            Collection allGroups = m_deviceGroupHome.findAll( );
            for ( Iterator iAll = allGroups.iterator(); iAll.hasNext(); ) {
                DeviceGroup group = (DeviceGroup) iAll.next();

                if ( group.getParentID() == null ) {
                    peers.add( group );
                }
            }
        }

        for ( Iterator iPeer = peers.iterator(); iPeer.hasNext(); ) {
            DeviceGroup peer = (DeviceGroup) iPeer.next();

            if ( peer.getName().equalsIgnoreCase( name ) ) {
                m_ctx.setRollbackOnly();
                throw new PDSException (
                    collateErrorMessages (  "E2030",
                                            new Object [] { name } ) );
            }
        }
    }



    /**
     * @deprecated copyDeviceGroup implements the copy Device
     * Group use case.   The new Device Group is called:
     * 'copy of <source device group's name>'
     *
     * @param sourceDeviceGroupID the PK of the Device Group to be copied.
     * @return the EJBObject of the new copy of the Device Group
     * @throws PDSException is thrown for application level errors.
     */
    public DeviceGroup copyDeviceGroup ( String sourceDeviceGroupID )
        throws PDSException {

        DeviceGroup source = null;
        DeviceGroup copy = null;

        try {
            try {
                source =
                    m_deviceGroupHome.findByPrimaryKey(
                                        new Integer ( sourceDeviceGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC449",
                                            "E1003",
                                            new Object[]{ sourceDeviceGroupID } ),
                    ex);
            }

            try {
                copy = createDeviceGroup(   source.getOrganizationID(),
                                            "Copy of " + source.getName(),
                                            source.getRefConfigSetID(),
                                            source.getParentID(),
                                            null );
            }
            catch ( PDSException ex ) {
                 m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC449" ), ex );
            }

            if ( source.getParentID() != null )
                copy.setParentID( source.getParentID() );

            /*ConfigurationSet sourceCS = null;

            try {
                Collection c = m_configSetHome.findByDeviceGroupID( sourceID );

                for ( Iterator i = c.iterator(); i.hasNext(); )
                    sourceCS = (ConfigurationSet) i.next();

            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC449",
                                            "E1010",
                                            new Object[]{ source.getExternalID() } ),
                    ex);
            }

            if ( sourceCS != null ) {
                try {
                    m_configSetHome.create( source.getRefConfigSetID(),
                                            PROF_TYPE_PHONE,
                                            copy,
                                            sourceCS.getContent() );
                }
                catch ( CreateException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC449",
                                                "E2017",
                                                new Object[]{   copy.getExternalID(),
                                                                copy.getRefConfigSetID(),
                                                                sourceCS.getContent() } ),
                        ex);
                }
            } // if*/

            String externalID =
                InternalToExternalIDTranslator.getInstance().translate( m_deviceGroupHome,
                                                                        Integer.valueOf( sourceDeviceGroupID ) );

            logTransaction ( m_ctx, "Copied device " + externalID );

        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC449",
                                            "E4039",
                                            new Object[]{ sourceDeviceGroupID } ) );
        }

        return copy;
    }



    /**
     * restartDevices restarts all of the Devices which belong to the given
     * Device Group.
     *
     * @param deviceGroupID the PK of the Device Group whose Devices should
     * be restarted.
     * @throws PDSException is thrown for application level errors.
     * @see DeviceAdvocate
     */
    public void restartDevices ( String deviceGroupID ) throws PDSException {
        String dgExternal = null;

        try {
            DeviceGroup dg = null;

            try {
                dg = m_deviceGroupHome.findByPrimaryKey( Integer.valueOf( deviceGroupID ) );
                dgExternal = dg.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC466",
                                            "E1003",
                                            new Object[]{ deviceGroupID } ),
                    ex);
            }

            Collection devices = null;

            try {
                devices =
                    m_deviceHome.findByDeviceGroupID( Integer.valueOf( deviceGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC466",
                                            "E1004",
                                            new Object[]{ dgExternal } ),
                    ex);
            }

            for ( Iterator iDev = devices.iterator(); iDev.hasNext(); ) {
                Device device = (Device) iDev.next();
                m_deviceAdvocateEJBObject.restartDevice( device.getID().toString() );
            }

            String externalID =
                InternalToExternalIDTranslator.getInstance().translate( m_deviceGroupHome,
                                                                        Integer.valueOf( deviceGroupID ) );

            logTransaction ( m_ctx, "Restarted devices in device group " + externalID );

            Collection children =
                m_deviceGroupHome.findByParentID( Integer.valueOf ( deviceGroupID ) );

            for ( Iterator iChild = children.iterator(); iChild.hasNext(); ) {
                DeviceGroup child = (DeviceGroup) iChild.next();
                restartDevices( child.getID().toString() );
            }
        }
        catch ( PDSException ex ) {
            throw new PDSException ( collateErrorMessages ( "UC466" ), ex );
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4071",
                                            new Object[]{ dgExternal } ) );
        }
        catch ( FinderException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4071",
                                            new Object[]{ dgExternal } ) );
        }

    }


    /**
     *@param deviceGroupID the PK of the Device Group whose Devices should
     * have the profiles projected.
     *@param profileTypes comma-separated string of the profile types
     * to be generated for this Phone Tag. For the permissible values are
     * in PGSDefinitions - Macros for these values are ( PROF_TYPE_PHONE,
     * PROF_TYPE_USER, PROF_TYPE_APPLICATION_REF).
     *@param projAlg is the name of the class to be used for the
     * projection part of the operation (optional if a null is passed then
     * the default projection class for the phone type and profile is
     * used instead).
     *@exception PDSException  is thrown when an application level error
     * occurs.
     */
    public void generateProfiles(   String deviceGroupID,
                                    String profileTypes,
                                    String projAlg) throws PDSException {

        int jobID = -1;

        try {
            DeviceGroup pg = null;
            try {
                pg = m_deviceGroupHome.findByPrimaryKey( Integer.valueOf ( deviceGroupID ) );
            }
            catch (FinderException fe) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages("E1003", new Object[]{deviceGroupID}),
                    fe);
            }

            boolean[] profTypesToCreate = null;

            profTypesToCreate = ProfileTypeStringParser.parse(profileTypes);

            StringBuffer jobDetails = new StringBuffer ();
            jobDetails.append( "Projection for device group: ");
            jobDetails.append( pg.getExternalID());
            jobDetails.append ( " profile types: ");

            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_USER ] ) {
                jobDetails.append ( "user ");
            }
            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_PHONE ] ) {
                jobDetails.append ( "device ");
            }
            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_APPLICATION_REF ] ) {
                jobDetails.append ( "application");
            }

            int numberOfDevicesToProject = m_projectionHelperEJBObject.calculateTotalProfiles(pg);

            String initialProgress = "projected 0 of " + numberOfDevicesToProject + " devices.";

            jobID = m_jobManagerEJBObject.createJob(JobManager.PROJECTION, jobDetails.toString(), initialProgress);

            generateGroupProfiles (pg,
                                    projAlg,
                                    profTypesToCreate,
                                    jobID,
                                    numberOfDevicesToProject ,
                                    0);

            m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.COMPLETE, null );
        }
        catch (PDSException e) {
            m_ctx.setRollbackOnly();

            if ( jobID != -1 ) {
                try {
                    m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.FAILED, e.toString() );
                }
                catch ( Exception e1) {}
            }

            throw new PDSException("UC470", e);
        }
        catch (RemoteException e) {
            logFatal ( e.toString() );

            if ( jobID != -1 ) {
                try {
                    m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.FAILED, e.toString() );
                }
                catch ( Exception e1) {}
            }

            throw new EJBException( e.toString() );
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    //
    // @see com.pingtel.pds.pgs.user.UserGroupAdvocate (generateGroupProfiles)
    //
    /////////////////////////////////////////////////////////////////////////////////////////
    private int generateGroupProfiles(  DeviceGroup deviceGroup,
                                        String projectionAlgorithm,
                                        boolean [] profTypesToCreate,
                                        int jobID,
                                        int totalUsersToProject,
                                        int currentTotal )
             throws PDSException {

        ArrayList projectionInputs = new ArrayList();
        Device device = null;
        HashMap partProjectedProfiles = new HashMap();
        HashMap projectionClassNames = new HashMap();
        DeviceGroup pg = null;
        StringBuffer profileTypes = new StringBuffer();

        if ( profTypesToCreate[PDSDefinitions.PROF_TYPE_USER] ) {
            profileTypes.append(PDSDefinitions.PROF_TYPE_USER);
        }

        if ( profTypesToCreate[PDSDefinitions.PROF_TYPE_APPLICATION_REF] ) {
            profileTypes.append(',');
            profileTypes.append(PDSDefinitions.PROF_TYPE_APPLICATION_REF);
        }

        try {
            ////////////////////////////////////////////////////////////////
            // find child phone groups and generate phone profiles for them.
            ////////////////////////////////////////////////////////////////
            Collection children = null;

            try {
                children = m_deviceGroupHome.findByParentID(deviceGroup.getID());
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E1015",
                                            new Object[]{ pg.getExternalID() }),
                    ex);
            }

            for (Iterator i = children.iterator(); i.hasNext(); ) {
                DeviceGroup child = (DeviceGroup) i.next();

                currentTotal =
                    generateGroupProfiles(  child,
                                            projectionAlgorithm,
                                            profTypesToCreate,
                                            jobID,
                                            totalUsersToProject,
                                            currentTotal );
            }


            ///////////////////////////////////////////////////////////////
            // Fetch the ProjectionInputs for the parent Phone Groups
            ///////////////////////////////////////////////////////////////
            projectionInputs.addAll(m_projectionHelperEJBObject.addParentGroupConfigSets( deviceGroup));

            // Reverse the order
            Collections.reverse(projectionInputs);

            Collection phonesInGroup = null;

            try {
                phonesInGroup = m_deviceHome.findByDeviceGroupID( deviceGroup.getID() );
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC470",
                                            "E1004",
                                            new Object[]{ pg.getExternalID() }),
                    ex);
            }

            String projectionClassName = null;
            Integer csID = null;

            for (Iterator iPhones = phonesInGroup.iterator();
                    iPhones.hasNext(); ) {

                device = (Device) iPhones.next();

                if (projectionAlgorithm == null) {
                    csID = device.getCoreSoftwareDetailsID();

                    if (projectionClassNames.containsKey(csID)) {

                        projectionClassName = (String)
                                projectionClassNames.get(csID);
                    } else {
                        projectionClassName =
                                m_projectionHelperEJBObject.getProjectionClassName(device,
                                PDSDefinitions.PROF_TYPE_PHONE);

                        projectionClassNames.put(csID,
                                projectionClassName);

                    }
                } else {
                    // the user has supplied a non-default projection alg.
                    projectionClassName = projectionAlgorithm;
                }


                Integer pdTypeID = device.getDeviceTypeID();

                ProjectionInput partProjectedInput = null;

                if (partProjectedProfiles.containsKey( pdTypeID )) {
                    logDebug ( "found part projected profile for device type: " + pdTypeID );
                    partProjectedInput = (ProjectionInput)
                            partProjectedProfiles.get( pdTypeID );
                    if ( partProjectedInput == null )
                        logDebug ( "partprojected input is NULL" );

                    logDebug ( "partProjectedInput is: " + partProjectedInput );
                } else {
                    logDebug ( "no part projected profile for device type: " + pdTypeID );
                    partProjectedInput =
                        m_projectionHelperEJBObject.project(
                                        projectionClassName,
                                        projectionInputs,
                                        pdTypeID,
                                        PDSDefinitions.PROF_TYPE_PHONE );

                    logDebug ( "produced part projected profile for device type: " + pdTypeID );

                    partProjectedProfiles.put( pdTypeID , partProjectedInput);
                    logDebug ( "stored part projected profile for device type: " + pdTypeID );
                }

                ProjectionInput lpInput = m_projectionHelperEJBObject.getProjectionInput(device);

                ArrayList optimizedInputs = new ArrayList();
                optimizedInputs.add( partProjectedInput );
                if ( lpInput != null )
                    optimizedInputs.add( lpInput );

                String ownerID = null;
                ownerID = device.getUserID();

                if ( ownerID != null ) {

                    User owner = null;

                    try {
                        owner = m_userHome.findByPrimaryKey( ownerID );
                    }
                    catch (FinderException ex) {
                        m_ctx.setRollbackOnly();

                        throw new PDSException(
                            collateErrorMessages(   "UC470",
                                                    "E1005",
                                                    new Object[]{ device.getExternalID() }),
                            ex);
                    }

                    m_userAdvocateEJBObject.generateProfiles(
                            owner,
                            profileTypes.toString(),
                            projectionAlgorithm,
                            false ); // part of group projection.
                }

                logDebug ( "calling projectAndPersist for device: " + device.getExternalID() );
                m_projectionHelperEJBObject.projectAndPersist(
                        projectionClassName,
                        device,
                        PDSDefinitions.PROF_TYPE_PHONE,
                        optimizedInputs );

                logDebug ( "done projectAndPersist for device: " + device.getExternalID() );

                currentTotal++;
                m_jobManagerEJBObject.updateJobProgress(    jobID,
                                                            "projected " + currentTotal + " of " +
                                                                totalUsersToProject + " devices." );

            }
            // end LogicalPhone for loop
        }
        // try
        catch (PDSException pe) {
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC470" ), pe );
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                    collateErrorMessages("UC470",
                    "E4011",
                    new Object[]{deviceGroup}));
        }

        return currentTotal;
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
            m_deviceGroupHome = (DeviceGroupHome) initial.lookup("DeviceGroup");
            m_configSetHome = (ConfigurationSetHome)
                    initial.lookup("ConfigurationSet");

            m_deviceHome = (DeviceHome) initial.lookup("Device");

            ProjectionHelperHome projHelperHome;
            projHelperHome = (ProjectionHelperHome)
                                initial.lookup("ProjectionHelper");

            DeviceAdvocateHome deviceAdvocateHome;
            deviceAdvocateHome = (DeviceAdvocateHome)
                                initial.lookup("DeviceAdvocate");

            m_userHome = (UserHome) initial.lookup("User");
            UserAdvocateHome userAdvocateHome;
            userAdvocateHome = (UserAdvocateHome) initial.lookup("UserAdvocate");

            JobManagerHome jobManagerHome = (JobManagerHome) initial.lookup( "JobManager" );

            m_deviceAdvocateEJBObject = deviceAdvocateHome.create();
            m_userAdvocateEJBObject = userAdvocateHome.create();
            m_projectionHelperEJBObject = projHelperHome.create();
            m_jobManagerEJBObject = jobManagerHome.create();
        }
        catch (NamingException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
        catch (CreateException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
    }


    /**
     *  Description of the Method
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }

}
