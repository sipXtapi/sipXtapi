/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/jobs/JobManagerBean.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jobs;

import javax.ejb.EJBException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.ejb.CreateException;

import java.util.Date;
import java.util.ArrayList;
import java.util.Iterator;
import java.sql.SQLException;

import org.jdom.Element;
import org.jdom.CDATA;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.common.PDSException;


public class JobManagerBean extends JDBCAwareEJB implements SessionBean, JobManagerBusiness {

    private static final int ALL_JOBS = 0;
    private static final int CURRENT_JOBS = 1;

    //The Session Context object
    SessionContext m_ctx = null;



    public int createJob ( int jobType, String details, String progress ) throws PDSException {

        Integer nextJobID = new Integer ( getNextSequenceValue( "JOB_SEQ" ) );

        if ( jobType != JobManager.PROJECTION && jobType != JobManager.DEVICE_RESTART ) {
            m_ctx.setRollbackOnly();
            throw new PDSException ( "Invalid job type" );
        }

        try {
            logDebug ( "Insert into jobs ( ID, TYPE, DETAILS, START_TIME, STATUS, PROGRESS ) VALUES " +
                    nextJobID + ", " + new Integer ( jobType ) + ", " + details + ", " + new  Date().toString() +
                    ", " + JobManager.STARTED + ", " + progress );

            executePreparedUpdate ( "INSERT INTO JOBS ( ID, TYPE, DETAILS, START_TIME, STATUS, PROGRESS ) VALUES ( ?,?,?,?,?,? )",
                                    new Object [] { nextJobID,
                                                    new Integer ( jobType ),
                                                    details,
                                                    new  Date().toString(),
                                                    JobManager.STARTED,
                                                    progress } );
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception creating Job record", e );
        }

        return nextJobID.intValue();
    }


    public void updateJobProgress ( int jobID, String progress ) throws PDSException {

        try {
            executePreparedUpdate ( "UPDATE JOBS SET PROGRESS = ? WHERE ID = ?",
                                    new Object [] { progress,
                                                    new Integer ( jobID ) } );
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception updating Job progress", e );
        }
    }


    public void updateJobStatus ( int jobID, String status, String exceptionMessage )
            throws PDSException {

        if ( !status.equals( JobManager.STARTED ) &&
                !status.equals( JobManager.COMPLETE ) &&
                !status.equals( JobManager.FAILED ) ) {

            m_ctx.setRollbackOnly();

            throw new PDSException( "" );
        }

        try {
            executePreparedUpdate ( "UPDATE JOBS SET STATUS = ?, EXCEPTION_MESSAGE = ? WHERE ID = ?",
                                    new Object [] { status,
                                                    exceptionMessage,
                                                    new Integer ( jobID ) } );
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception updating Job status", e );
        }
    }


    public void flushJobs (  ) throws PDSException {

        try {
            executePreparedUpdate ( "DELETE FROM JOBS WHERE STATUS = 'F' OR STATUS = 'C'",
                                    new Object [] {} );
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception clearing jobs", e );
        }
    }


    public Element getCurrentJobStatuses () throws PDSException {
        return getStatuses ( CURRENT_JOBS );
    }

    public Element getAllJobStatuses () throws PDSException {
        return getStatuses ( ALL_JOBS );
    }


    private Element getStatuses( int query ) throws PDSException {
        Element statusRoot = new Element ( "statuses");

        ArrayList rows = new ArrayList();

        try {
            String queryString = null;

            if ( query == ALL_JOBS ) {
                queryString = "SELECT ID, TYPE, DETAILS, STATUS, PROGRESS, START_TIME FROM JOBS";
            }
            else if ( query == CURRENT_JOBS)  {
                queryString =  "SELECT ID, TYPE, DETAILS, STATUS, PROGRESS, START_TIME FROM JOBS WHERE STATUS = 'S' OR STATUS = 'F'";
            }
            else {
                throw new EJBException( "Programmer error!" );
            }

            rows = executePreparedQuery (   queryString,
                                            null,
                                            6,
                                            1000000 );

            for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                ArrayList row = (ArrayList) iRow.next();

                Element jobStatus = new Element ( "jobstatus" );
                statusRoot.addContent ( jobStatus );

                jobStatus.addContent( new Element ( "id").setText( (String) row.get(0) ) );
                jobStatus.addContent( new Element ( "type").setText( (String) row.get(1) ) );
                jobStatus.addContent( new Element ( "details").setText( (String) row.get(2) ) );
                jobStatus.addContent( new Element ( "status").setText( (String) row.get(3) ) );
                Element progressElem = new Element ( "progress");
                jobStatus.addContent ( progressElem );

                String progress = (String) row.get(4);
                if ( progress != null ) {
                    progressElem.setText( progress );
                }

                jobStatus.addContent( new Element ( "starttime").setText( (String) row.get(5) ) );

            }
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception fetching Job statuses", e );
        }

        return statusRoot;
    }


    public Element getExceptionMessage ( int jobID ) throws PDSException {
        Element jobRoot = new Element ( "job");
        jobRoot.addContent( new Element ("id").setText( String.valueOf( jobID ) ) );

        ArrayList rows = new ArrayList();

        try {
            rows = executePreparedQuery (   "SELECT EXCEPTION_MESSAGE FROM JOBS WHERE ID = ?",
                                            new Object [] { new Integer ( jobID ) },
                                            1,
                                            1 );

            for ( Iterator iRow = rows.iterator(); iRow.hasNext(); ) {
                ArrayList row = (ArrayList) iRow.next();

                Element exceptionMessageElem = new Element ( "exception");

                jobRoot.addContent ( exceptionMessageElem );

                String exceptionMessage = (String) row.get(0);
                if ( exceptionMessage != null) {
                    CDATA messageCDATA = new CDATA ( exceptionMessage );
                    exceptionMessageElem.addContent ( messageCDATA );
                }
            }
        }
        catch (SQLException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException( "Exception getting Job exception messages", e );
        }

        return jobRoot;
    }


    public void ejbCreate() { }

    public void ejbActivate() { }

    public void ejbPassivate() { }

    public void ejbRemove() { }

    public void setSessionContext( SessionContext sessionContext ) {
        m_ctx = sessionContext;
    }
}
