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
 * A feature indication is an interface used to display visual feedback to the
 * end user.  The feature manager decides what feature icons are shown, in
 * what order, and in what state/style.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface FeatureIndicator
{
    /**
     * Get the icon view of this feature indicator.
     * <p>
     * Developers should implement this method even if they install their
     * FeatureIndicator as a line view.
     *
     * @return Icon image that will be displayed as this feature.
     */
    public Image getIcon() ;


    /**
     * Get the short description for this feature indicator.  The short
     * description is used when expanding a list of icon view feature
     * indicators.
     * <p>
     * Developers should implement this method even if they install
     * their FeatureIndicator as a line view.
     */
    public String getShortDescription() ;


    /**
     * Get the component for a line view or multiline view.
     *
     * @return The component that will displayed as the feature indicator
     */
    public Component getComponent() ;


    /**
     * Get the hint text displayed when an associated button is pressed and
     * held for an extended period of time.
     *
     * @return Hint text displayed for this feature indicator.
     */
    public String getHint() ;


    /*
     * This method is invoked when a feature icon has been associated with a
     * button and that button is pressed.
     */
    public void buttonPressed() ;
}
