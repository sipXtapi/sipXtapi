/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/CSProfileDetailBean.java#4 $
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
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

public class CSProfileDetailBean extends JDBCAwareEJB implements EntityBean, CSProfileDetailBusiness {

    public Integer id;
    public Integer profileType;
    public String fileName;
    public String renderClassName;
    public String projectionClassName;
    public Integer coreSoftwareDetailsID;
    public String xsltURL;


    // remote methods

    // Immutable fields
    public Integer getID() {
        return this.id;
    }

    public Integer getCoreSoftwareDetailsID () {
        return this.coreSoftwareDetailsID;
    }

    public Integer getProfileType () {
        return this.profileType;
    }

    // mutable fields

    public String getFileName () {
        return this.fileName;
    }

    public void setFileName ( String fileName ) {
        this.fileName = fileName;
    }

    public String getRenderClassName () {
        return this.renderClassName;
    }

    public void setRenderClassName ( String renderClassName ) {
        this.renderClassName = renderClassName;
    }

    public String getProjectionClassName () {
        return this.projectionClassName;
    }

    public void setProjectionClassName ( String projectionClassName ) {
        this.projectionClassName = projectionClassName;
    }

    public String getXSLTURL() {
        return this.xsltURL;
    }

    public void setXSLTURL ( String xsltURL ) {
        this.xsltURL = xsltURL;
    }

    public String getExternalID () {
        CoreSoftware cs = null;

        try {
           cs = m_csHome.findByPrimaryKey( coreSoftwareDetailsID );
           return "core software " + cs.getExternalID() +
                " profile type " + this.profileType;
        }
        catch ( Exception ex ) {
            throw new EJBException ( ex );
        }
    }


    // Entity Bean methods

    public Integer ejbCreate (  Integer coreSoftwareDetailsID,
                                Integer profileType,
                                String fileName,
                                String renderClassName,
                                String xsltURL,
                                String projectionClassName )
        throws CreateException {

        id = new Integer( this.getNextSequenceValue( "CS_PROF_DET_SEQ" ) );
        this.coreSoftwareDetailsID = coreSoftwareDetailsID;
        this.profileType = profileType;
        this.fileName = fileName;
        this.renderClassName = renderClassName;
        this.xsltURL = xsltURL;
        this.projectionClassName = projectionClassName;

        return null;
    }

    public void ejbPostCreate ( Integer coreSoftwareDetailsID,
                                Integer profileType,
                                String fileName,
                                String renderClassName,
                                String xsltURL,
                                String projectionClassName ) {}


    public void ejbLoad () {}

    public void ejbStore  () {}

    public void ejbRemove () {}

    public void ejbActivate () {}

    public void ejbPassivate () {}

    public void setEntityContext ( EntityContext ctx ) {
        this.m_ctx = ctx;

        try {
            InitialContext initial = new InitialContext();
            m_csHome = (CoreSoftwareHome) initial.lookup( "CoreSoftware" );
        }
        catch ( NamingException ex ) {
            logFatal ( ex.toString(), ex  );
            throw new EJBException ( ex );
        }
    }

    /** Boiler Plage Entity Bean Implementation */
    public void unsetEntityContext ( ) {
        this.m_ctx = null;
    }

    private CoreSoftwareHome m_csHome;

    private EntityContext m_ctx;


}