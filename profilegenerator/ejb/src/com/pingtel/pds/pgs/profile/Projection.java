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

import java.util.Collection;

import com.pingtel.pds.common.PDSException;

public interface Projection  {

    /**
     *
     * @param       Takes a collection of ProjectionInput objects.
     * @return      returns the result of the projection operation on the
     *              set of input ProjectionInput objects
     * @exception   PDSException for all errors
    */
    public ProjectionInput project ( Collection projectionInputs, Collection validRefPropertyIDs )
        throws PDSException;
}
