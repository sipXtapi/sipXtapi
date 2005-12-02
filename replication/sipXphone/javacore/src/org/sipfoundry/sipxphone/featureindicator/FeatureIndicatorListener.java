/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.featureindicator ;

import org.sipfoundry.sipxphone.awt.* ;
import java.awt.* ;


/**
 * The feature indicator listener informs subscribers when feature indicators
 * are added, removed, or when a refreshed is requested.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface FeatureIndicatorListener
{
    /**
     * This method is invoked when a new indicator is installed.
     *
     * @param indicator The indicator that was installed.
     */
    public void indicatorInstall(FeatureIndicator indicator) ;
    
    
    /**
     * This method is invoked when a indicator is uninstalled.
     *
     * @param indicator The indicator that was removed.
     */
    public void indicatorRemoved(FeatureIndicator indicator) ;
    
    
    /**
     * This method is invoked when an end user requests a indicator refresh.
     *
     * @param indicator The indicator that is being refreshed.
     */
    public void indicatorRefreshed(FeatureIndicator indicator) ;
}
