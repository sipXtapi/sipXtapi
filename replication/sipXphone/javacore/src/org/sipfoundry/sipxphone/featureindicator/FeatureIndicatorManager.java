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

import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.awt.* ;
import java.awt.* ;
import java.util.* ;


/**
 * The feature manager is reponsible for displaying the various call feature
 * indicators in the home screen of the xpressa phone.  Feature indicators
 * are vehicles for provide visual feedback for specific phone features.
 * <br><br>
 * Feature Indicators can be displayed in different view styles:
 * <ul>
 *   <li>Icon: An icon view provide a simple "on/off" or "true/false" state of a
 *             feature.</li>
 *   <li>Line: A line view allows the developer to display a line or two of
 *             text and images.  The line is associated with a button and the
 *             feature indicator is notified when that button is pressed.
 *             At any point, the line can be collapsed into a icon if there is
 *             a demand for screen real estate.  The feature manager will
 *             expand the icon back into a line view once the demand lowers.
 *             </li>
 *   <li>MultiLine: A multiline view allows developer to display a larger
 *             view of text/images at the lose of the associated button. The
 *             multiline view can be resized often and does not receive
 *             notification when any buttons are pressed.  If the screen
 *             becomes filled with information, a multiline view may be
 *             hidden.</li>
 * </ul>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class FeatureIndicatorManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Icon View Style: A feature indicator is displayed as a single
        icon in the xpressa home screen. */
    public static final int VIEWSTYLE_ICON       = 1 ;
    /** Line View Style: A feature indicator is displayed as a line
        in the xpressa home screen. */
    public static final int VIEWSTYLE_LINE       = 2 ;
    /** Multline View Style: A feature indicator is displayed as a variable
        number of lines in the xpressa home screen.*/
    public static final int VIEWSTYLE_MULTILINE  = 3 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector m_vFeatureIndicator ;    // List of feature indicators
    private Hashtable m_htFeatureStates ;   // State of feature indicators
    private Vector m_vListeners ;           // Listener List

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public FeatureIndicatorManager()
    {
        m_vFeatureIndicator = new Vector() ;
        m_vListeners = new Vector() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Install a feature indicator as the specified view style.
     *
     * @param indicator The feature indicator that will be install.
     * @param iViewStyle The View of this feature indicator: VIEWSTYLE_ICON,
     *        VIEWSTYLE_LINE, or VIEWSTYLE_MULTILINE.
     */
    public synchronized void installIndicator(FeatureIndicator indicator, int iViewStyle)
    {
        icFeatureIndicatorDescriptor desc = findDescriptor(indicator) ;

        if (desc == null) {
            desc = new icFeatureIndicatorDescriptor(indicator, iViewStyle) ;
            m_vFeatureIndicator.addElement(desc) ;
            fireInstallNotification(indicator) ;
        } else {
            refreshIndicator(indicator) ;
        }
    }


    /**
     * Remove the specified feature indicator.
     *
     * @param indicator The feature indicator that will be uninstalled.
     */
    public synchronized void removeIndicator(FeatureIndicator indicator)
    {
        icFeatureIndicatorDescriptor desc = findDescriptor(indicator) ;
        if (desc != null) {
            fireRemovalNotification(indicator) ;
            m_vFeatureIndicator.removeElement(desc) ;
        }
    }


    /**
     * Refresh the specified feature indicator.  At times a developer may wish
     * to refresh a feature indicator.  For example, one may want to update a
     * feature indicator icon without reinstalling the indicator.
     *
     * @param indicator The feature indicator that will be refreshed.
     *
     * @exception IllegalArgumentException Thrown if the indicator is not
     *            installed.
     */
    public synchronized void refreshIndicator(FeatureIndicator indicator)
        throws IllegalArgumentException
    {
        icFeatureIndicatorDescriptor desc = findDescriptor(indicator) ;
        if (desc != null) {
            fireRefreshNotification(indicator) ;
        } else {
            throw new IllegalArgumentException("indicator not installed: " + indicator) ;
        }
    }


    /**
     * What is the requested state of the indicator.  The actual state may or
     * may not match the requested state.
     *
     * @param indicator The feature indicator whose state will be returned.
     *
     * @exception IllegalArgumentException Thrown if the indicator is not
     *            installed.
     */
    public synchronized int getRequestedIndicatorState(FeatureIndicator indicator)
        throws IllegalArgumentException
    {
        int iState = -1 ;
        icFeatureIndicatorDescriptor desc = findDescriptor(indicator) ;
        if (desc != null) {
            iState = desc.m_iRequestedState ;
        } else {
            throw new IllegalArgumentException("indicator not installed: " + indicator) ;
        }
        return iState ;
    }


    /**
     * Add a Listener to the feature indicator listener list.  Subscribers are
     * notified whenever a new indicator is installed, remove, or refreshed.
     * If the specified listener is already subscribed, then nothing is
     * completed (silent error).
     *
     * @param listener The listener that will be subscribed to indicator
     *        notifications.
     */
    public void addIndicatorListener(FeatureIndicatorListener listener)
    {
        if (!m_vListeners.contains(listener)) {
            m_vListeners.addElement(listener) ;
        }
    }


    /**
     * Remove a listener from the feature indicator listener list.  The
     * listener will no longer be notified of installation, removal, or
     * refresh requests.  If the specified listener is not a subscriber,
     * then nothing is completed (silent error).
     *
     * @param listener The listener that will be subscribed to indicator
     *        notifications.
     */
    public void removeIndicatorListener(FeatureIndicatorListener listener)
    {
        m_vListeners.removeElement(listener) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     */
    protected synchronized void fireInstallNotification(FeatureIndicator indicator)
    {
        Enumeration enum = m_vListeners.elements() ;
        while (enum.hasMoreElements()) {
            FeatureIndicatorListener listener = (FeatureIndicatorListener) enum.nextElement() ;
            if (listener != null) {
                try {
                    listener.indicatorInstall(indicator) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     */
    protected synchronized void fireRemovalNotification(FeatureIndicator indicator)
    {
        Enumeration enum = m_vListeners.elements() ;
        while (enum.hasMoreElements()) {
            FeatureIndicatorListener listener = (FeatureIndicatorListener) enum.nextElement() ;
            if (listener != null) {
                try {
                    listener.indicatorRemoved(indicator) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     */
    protected synchronized void fireRefreshNotification(FeatureIndicator indicator)
    {
        Enumeration enum = m_vListeners.elements() ;
        while (enum.hasMoreElements()) {
            FeatureIndicatorListener listener = (FeatureIndicatorListener) enum.nextElement() ;
            if (listener != null) {
                try {
                    listener.indicatorRefreshed(indicator) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////

    /**
     * Find the internal descriptor that contains the specified indicator
     */
    protected icFeatureIndicatorDescriptor findDescriptor(FeatureIndicator indicator)
    {
        icFeatureIndicatorDescriptor rc = null ;

        Enumeration enum = m_vFeatureIndicator.elements() ;
        while (enum.hasMoreElements()) {
            icFeatureIndicatorDescriptor desc = (icFeatureIndicatorDescriptor) enum.nextElement() ;
            if ((desc != null) && (desc.m_featureIndicator == indicator)) {
                rc = desc ;
            }
        }

        return rc ;
    }




//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     * This feature indicator description is used to maintain the state of
     * each feature indicator.
     *
     */
    protected class icFeatureIndicatorDescriptor
    {
    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
        public FeatureIndicator m_featureIndicator ;
        public int              m_iRequestedState ;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
        public icFeatureIndicatorDescriptor(FeatureIndicator indicator,
                                            int              iRequestedState)
        {
            m_featureIndicator = indicator ;
            m_iRequestedState = iRequestedState ;
        }
    }
}
