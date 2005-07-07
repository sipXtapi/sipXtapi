/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/MissedCallFeatureIndicator.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.app.core;

import java.awt.Component ;
import java.awt.Image ;
import java.util.Vector ;

import org.sipfoundry.stapi.PCall ;
import org.sipfoundry.sipxphone.awt.PLabel ;
import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager;

/**
 * MissedCallsFeatureIndicator.java
 *
 *
 * Created: Wed Jan 02 14:37:09 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class MissedCallFeatureIndicator implements FeatureIndicator{

    FeatureIndicatorManager m_manager ;
    Vector m_vMissedCalls ;

    public MissedCallFeatureIndicator()
    {
        m_manager = Shell.getFeatureIndicatorManager() ;
        m_vMissedCalls = new Vector() ;
    }

    /**
     * Required as part of the feature indicator API: Tray icon.
     */
    public Image getIcon()
    {
        return null ;
    }


    /**
     * Required as part of the feature indicator API: short description.
     */
    public String getShortDescription()
    {
        return null ;
    }


    /**
     * Required as part of the feature indicator API: Hint Text
     */
    public String getHint()
    {
        return "Missed Calls|Press to display the call log to see the missed calls." ;
    }


    /**
     * Required as part of the feature indicator API
     * @return The component that will displayed as the feature indicator
     */
    public synchronized Component getComponent()
    {
        PLabel label = new PLabel("Missed Calls: " +
                Integer.toString(m_vMissedCalls.size()), PLabel.ALIGN_EAST) ;
        label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;

        return label ;
    }


    /*
     * Required as part of the feature indicator API:
     * This method is invoked when a feature icon has been associated with a
     * button and that button is pressed.
     */
    public void buttonPressed()
    {
       ApplicationManager.getInstance()
            .activateCoreApplication(ApplicationRegistry.CALL_LOG_APP) ;
       clearAll();
    }

   /**
    * added a call to the container storing missed calls
    * and installs the indicator.
    */
    public void callMissed(PCall call)
    {
            m_vMissedCalls.addElement(call.getCallID()) ;
            if ( m_vMissedCalls.size() > 0)
                m_manager.installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
            else
                m_manager.refreshIndicator(this) ;
     }

     /**
      * clears the container storing missed calls and removes this feature
      * indicator
      */
     public void clearAll(){
        m_vMissedCalls.removeAllElements();
        m_manager.removeIndicator(this) ;
     }



}// MissedCallsFeatureIndicator
