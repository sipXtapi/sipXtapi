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
public class ProjectionRule extends RefConfigSetAssignment {

    /**
     *  Constructor for the ProjectionRule object
     *
     *@param  refPropertyID  Description of the Parameter
     *@param  isFinal        Description of the Parameter
     *@param  isReadOnly     Description of the Parameter
     *@param  cardinality    Description of the Parameter
     */
    public ProjectionRule(Integer refPropertyID,
            boolean isFinal,
            boolean isReadOnly,
            String cardinality) {

        super (refPropertyID,isFinal,isReadOnly);
        m_cardinality = cardinality;
    }


    /**
     *  Gets the cardinality attribute of the ProjectionRule object
     *
     *@return    The cardinality value
     */
    public String getCardinality() {
        return this.m_cardinality;
    }

    private String m_cardinality;
}
