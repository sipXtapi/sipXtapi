/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/DoNotDisturbFeatureIndicator.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.app.preferences ;

import java.awt.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

import javax.telephony.callcontrol.* ;

/**
 * The DoNotDisturbFeatureIndicator is displayed when the user enables
 * the DND feature of the phone.  This indicator servers as a visual
 * reminder to the user.  They can also press the button next to the button
 * as a shortcut to disable DND.
 */
public class DoNotDisturbFeatureIndicator implements FeatureIndicator
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** singleton reference to self */
    private static DoNotDisturbFeatureIndicator m_instance = null ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Private constructor; use getInstance() to gain singleton reference.
     */
    private DoNotDisturbFeatureIndicator()
    {
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Obtains a singleton instantance to this feature indicator.
     */
    public static DoNotDisturbFeatureIndicator getInstance()
    {
        if (m_instance == null) {
            m_instance = new DoNotDisturbFeatureIndicator() ;
        }

        return m_instance ;
    }


    /**
     * Installs the feature indicator
     */
    public void install()
    {
        CallHandlingManager manager = Shell.getCallHandlingManager() ;
        Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_ICON) ;
    }


    /**
     * Uninstalls or removes the feature indiciator
     */
    public void uninstall()
    {
        Shell.getFeatureIndicatorManager().removeIndicator(this) ;
    }


    /**
     * Determines whether the feature indicator should be installed or not.
     */
    public boolean shouldInstall()
    {
        CallHandlingManager manager = Shell.getCallHandlingManager() ;
        return manager.isDNDEnabled() ;
    }


    /**
     * Returns the icon for this feature indicator
     */
    public Image getIcon()
    {
        return AppResourceManager.getInstance().getImage("imgDoNotDisturbIcon") ;
    }


    public String getShortDescription()
    {
        return AppResourceManager.getInstance().getString("lblDNDFIDesc") ;
    }


    /**
     * Returns the hint for this feature indicator
     */
    public String getHint()
    {
        return AppResourceManager.getInstance().getString("lblDNDFIHint") ;
    }


    /**
     * Get the component for a line view or multiline view.
     *
     * @return The component that will displayed as the feature indicator
     */
    public Component getComponent()
    {
        return new icIconTextContainer(getIcon(), AppResourceManager.getInstance().getString("lblDNDFIDesc")) ;
    }


    /*
     * This method is invoked when a feature icon has been associated with a
     * button and that button is pressed.
     */
    public void buttonPressed()
    {
        MessageBox msgBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_WARNING) ;
        msgBox.setMessage(AppResourceManager.getInstance().getString("lblDNDFIPrompt")) ;
        if (msgBox.showModal() == MessageBox.OK)
        {
            CallHandlingManager manager = Shell.getCallHandlingManager() ;
            manager.enableDND(false) ;
            uninstall() ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    private class icIconTextContainer extends PContainer
    {
        protected PLabel m_lblImage ;
        protected PLabel m_lblText ;

        public icIconTextContainer(Image icon, String strText)
        {
            GridBagLayout      gbl = new GridBagLayout() ;
            GridBagConstraints gbc = new GridBagConstraints() ;

            setLayout(gbl) ;

            // Add Text
            gbc.weightx = 1.0 ;
            gbc.weighty = 1.0 ;
            gbc.fill = GridBagConstraints.BOTH ;
            gbc.insets = new Insets(0, 0, 0, 0) ;
            gbc.gridwidth = 1 ;
            m_lblText = new PLabel(strText, PLabel.ALIGN_EAST) ;
            m_lblText.setInsets(new Insets(0, 0, 0, 10)) ;
            add(m_lblText, gbc) ;

            // Add Image
            gbc.weightx = 0.0 ;
            gbc.weighty = 1.0 ;
            gbc.insets = new Insets(0, 0, 0, 0) ;
            gbc.fill = GridBagConstraints.VERTICAL ;
            gbc.gridwidth = GridBagConstraints.REMAINDER ;
            m_lblImage = new PLabel(icon) ;
            add(m_lblImage, gbc) ;
        }

        public void repaint()
        {
            super.repaint() ;
            if (m_lblImage != null)
                m_lblImage.repaint() ;
            if (m_lblText != null)
                m_lblText.repaint() ;

        }


        public void setForeground(Color color)
        {
            if (m_lblImage != null)
                m_lblImage.setForeground(color) ;
            if (m_lblText != null)
                m_lblText.setForeground(color) ;
        }


        public void setBackground(Color color)
        {
            if (m_lblImage != null)
                m_lblImage.setBackground(color) ;
            if (m_lblText != null)
                m_lblText.setBackground(color) ;
        }

        public void setFont(Font font)
        {
            if (m_lblImage != null)
                m_lblImage.setFont(font) ;
            if (m_lblText != null)
                m_lblText.setFont(font) ;

        }

        public void setOpaque(boolean bOpaque)
        {
            if (m_lblImage != null)
                m_lblImage.setOpaque(bOpaque) ;
            if (m_lblText != null)
                m_lblText.setOpaque(bOpaque) ;
        }
    }

}
