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


/**
 *  Description of the Class
 *
 *@author     ibutcher
 *@created    December 19, 2001
 */
public class RefConfigSetAssignment  {

    public RefConfigSetAssignment ( Integer refPropertyID,
                                    boolean isFinal,
                                    boolean isReadOnly ) {

        m_refPropertyID = refPropertyID;
        m_isFinal = isFinal;
        m_isReadOnly = isReadOnly;
    }


    /**
     *  Gets the refPropertyID attribute of the ProjectionRule object
     *
     *@return    The refPropertyID value
     */
    public Integer getRefPropertyID() {
        return this.m_refPropertyID;
    }


    /**
     *  Gets the isFinal attribute of the ProjectionRule object
     *
     *@return    The isFinal value
     */
    public boolean getIsFinal() {
        return this.m_isFinal;
    }


    /**
     *  Gets the isReadOnly attribute of the ProjectionRule object
     *
     *@return    The isReadOnly value
     */
    public boolean getIsReadOnly() {
        return this.m_isReadOnly;
    }


    private Integer m_refPropertyID;
    private boolean m_isFinal;
    private boolean m_isReadOnly;

}
