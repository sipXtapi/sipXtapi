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

package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.patch.PatchManager;
import com.pingtel.pds.pgs.patch.PatchManagerHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class UpgradeDatabaseVersionTag extends ExTagSupport {

    private Integer m_version;

    private PatchManagerHome m_pmHome;

    public void setVersion( String version ) {
        if ( version.equalsIgnoreCase( "latest" ) ) {
            m_version = new Integer ( Integer.MAX_VALUE );
            System.out.println( "latest mapped to " + m_version);
        }
        else {
            m_version = Integer.valueOf( version );
        }
    }



    public int doStartTag() throws JspException {
        try {
            PatchManager advocate = null;

            if ( m_pmHome == null ) {
                m_pmHome = ( PatchManagerHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  PatchManagerHome.class,
                                                                    "PatchManager" );

                advocate = m_pmHome.create();
            }

            advocate.processPatches( m_version );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_version = null;

        super.clearProperties();
    }
}