/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ResetPatchStatusTag.java#4 $
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
import java.util.StringTokenizer;


public class ResetPatchStatusTag extends ExTagSupport {

    private PatchManagerHome m_pmHome;
    private String m_patchStatusList;

    public void setPatchstatuslist ( String patchstatuslist ) {
        this.m_patchStatusList = patchstatuslist;
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


            StringTokenizer st = new StringTokenizer ( m_patchStatusList, "," );

            while ( st.hasMoreTokens() ) {
                String token = st.nextToken();
                StringTokenizer pair = new StringTokenizer ( token, "=" );
                String patchNumber = pair.nextToken();
                String patchStatus = pair.nextToken();
                advocate.setPatchStatus(    patchStatus,
                                            Integer.valueOf( patchNumber ) );
            }
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_patchStatusList = null;

        super.clearProperties();
    }
}