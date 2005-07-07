/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/LineRegistrationFeatureIndicator.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.app.core ;

import java.awt.* ;

import java.util.Vector ;

import org.sipfoundry.sip.* ;
import org.sipfoundry.sip.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.sipxphone.sys.* ;


/**
 * This feature indicator sits around and waits for line state changes.  If a
 * line fails to register or times out, an error message is displayed in the
 * feature indicator area.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LineRegistrationFeatureIndicator
        implements FeatureIndicator, SipLineListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector m_vFailedLines ;  // List of failed lines
    private Vector m_vPendingLines ; // List of Pending Lines
    private boolean m_bInstalled ;   // Is the indicator visually installed?

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Default constructor
     */
    public LineRegistrationFeatureIndicator()
    {
        m_bInstalled = false ;
    }


    /**
     * Enables and disables the Line Registration feature indicator as a whole.
     *
     * @param bEnable <i>true</i> to enable or <i>false</i> to disable.
     */
    public void enable(boolean bEnable)
    {
        if (bEnable)
        {
            SipLineManager.getInstance().addLineListener(this) ;
            m_vFailedLines = new Vector() ;
            m_vPendingLines = new Vector() ;
            populateState() ;
        }
        else
        {
            SipLineManager.getInstance().removeLineListener(this) ;
            uninstall() ;
        }
    }


    /**
     * Refresh the feature indicator by querying state from the line manager.
     */
    public synchronized void populateState()
    {
        SipLineManager manager = SipLineManager.getInstance() ;
        m_vFailedLines.removeAllElements() ;
        m_vPendingLines.removeAllElements() ;

        SipLine lines[] = manager.getLines() ;
        if (lines != null)
        {
            for (int i=0; i<lines.length; i++)
            {
                int iLineState = lines[i].getState() ;
                switch (iLineState)
                {
                    case SipLine.LINE_STATE_PROVISIONED:
                    case SipLine.LINE_STATE_DISABLED:
                    case SipLine.LINE_STATE_REGISTERED:
                        markSuccess(lines[i]) ;
                        break ;
                    case SipLine.LINE_STATE_FAILED:
                        markPending(lines[i]) ;
                        break ;
                    case SipLine.LINE_STATE_EXPIRED:
                        markFailed(lines[i]) ;
                        break ;
                }
            }
        }
    }


    /**
     * Required as part of the feature indicator API: Tray icon.
     */
    public Image getIcon()
    {
        return null ;
    }


    /**
     * Required as part of the feature indicator API: Tray short desc.
     */
    public String getShortDescription()
    {
        return null ;
    }



    /**
     * Hint Text
     */
    public String getHint()
    {
        return "Line Status|Press to display registration status." ;
    }


    /**
     * @return The component that will displayed as the feature indicator
     */
    public synchronized Component getComponent()
    {
        Component comp = null ;

        int iFailed = m_vFailedLines.size() ;
        int iPending = m_vPendingLines.size() ;

        if (iFailed > 0)
        {
            StringBuffer status = new StringBuffer() ;

            status.append(iFailed) ;
            status.append(" line") ;
            if (iFailed > 1)
                status.append("s") ;
            status.append(" failed to register!\nPress for more info.") ;


            comp = new icIconTextContainer(AppResourceManager.getInstance().getImage("imgErrorIcon"), status.toString()) ;
        }
        else if (iPending > 0)
        {
            StringBuffer status = new StringBuffer() ;

            status.append(iPending) ;
            status.append(" line") ;
            if (iPending > 1)
                status.append("s") ;
            status.append(" pending registration!\nPress for more info.") ;


            comp = new icIconTextContainer(null, status.toString()) ;

        }

        return comp ;
    }


    /*
     * This method is invoked when a feature icon has been associated with a
     * button and that button is pressed.
     */
    public void buttonPressed()
    {
        String strArgs[] = new String[1] ;

        strArgs[0] = org.sipfoundry.sipxphone.app.DevicePreferencesApp.PREF_LINE_MAINTENANCE ;

        Shell.getInstance().activateCoreApplication(ApplicationRegistry.PREFERENCES_APP, strArgs) ;

        populateState() ;
    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has been enabled.
     */
    public void lineEnabled(SipLineEvent event)
    {
        markSuccess(event.getLine()) ;
    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has failed.
     */
    public void lineFailed(SipLineEvent event)
    {
        markPending(event.getLine()) ;
    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has timed out.
     */
    public void lineTimeout(SipLineEvent event)
    {
        markFailed(event.getLine()) ;
    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has been added.
     */
    public void lineAdded(SipLineEvent event)
    {

    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has been deleted.
     */
    public void lineDeleted(SipLineEvent event)
    {
        markSuccess(event.getLine()) ;
    }


    /**
     * Required as part of the SipLineListener APIs- notification that a line
     * has been deleted.
     */
    public void lineChanged(SipLineEvent event)
    {
        populateState() ;
    }

    /**
     * Required as part of the SipLineListener APIs- notification that the
     * default line changed.
     */
    public void lineDefault(SipLineEvent event)
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    protected synchronized void markFailed(SipLine line)
    {
        // Add to the failed list
        if (!m_vFailedLines.contains(line))
        {
            m_vFailedLines.addElement(line) ;
        }

        // Remove from the pending list
        if (m_vPendingLines.contains(line))
        {
            m_vPendingLines.removeElement(line) ;
        }

        // Install/Uninstall the FI.
        if ((m_vFailedLines.size() > 0) || (m_vPendingLines.size() > 0))
        {
            // Mulitple installs are permitted, are refreshes
            install() ;
        }
        else
        {
            // There is no need to allow mulitple uninstalls.
            if (m_bInstalled)
                uninstall() ;
        }
    }


    protected synchronized void markPending(SipLine line)
    {
        // Add to the pending list
        if (!m_vPendingLines.contains(line))
        {
            m_vPendingLines.addElement(line) ;
        }

        // Remove from the failed list
        if (m_vFailedLines.contains(line))
        {
            m_vFailedLines.removeElement(line) ;
        }

        // Install/Uninstall the FI.
        if ((m_vFailedLines.size() > 0) || (m_vPendingLines.size() > 0))
        {
            // Mulitple installs are permitted, are refreshes
            install() ;
        }
        else
        {
            // There is no need to allow mulitple uninstalls.
            if (m_bInstalled)
                uninstall() ;
        }
    }


    /**
     * Mark the internal status for the specified line as either successful
     * or not successful.
     *
     * @param line The Line in whose status is being set.
     * @param bSuccess <i>true</i> if the line has registered or <i>false</i>
     *        if the line has failed registration.
     */
    protected synchronized void markSuccess(SipLine line)
    {
        if (m_vFailedLines.contains(line))
        {
            m_vFailedLines.removeElement(line) ;
        }

        if (m_vPendingLines.contains(line))
        {
            m_vPendingLines.removeElement(line) ;
        }

        // Install/Uninstall the FI.
        if ((m_vFailedLines.size() > 0) || (m_vPendingLines.size() > 0))
        {
            // Mulitple installs are permitted, are refreshes
            install() ;
        }
        else
        {
            // There is no need to allow mulitple uninstalls.
            if (m_bInstalled)
                uninstall() ;
        }
    }


    /**
     * Install the visual component of the feature indicator
     */
    protected synchronized void install()
    {
        Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
        m_bInstalled = true ;
    }


    /**
     * Uninstall the visual component of the feature indicator
     */
    protected synchronized void uninstall()
    {
        Shell.getFeatureIndicatorManager().removeIndicator(this) ;

        m_bInstalled = false ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    /**
     * Internal container that displays the status text on the right of an error
     * icon.
     */
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
            m_lblText.setInsets(new Insets(0, 0, 0, 6)) ;
            m_lblText.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
            add(m_lblText, gbc) ;

            if (icon != null)
            {
                // Add Image
                gbc.weightx = 0.0 ;
                gbc.weighty = 1.0 ;
                gbc.insets = new Insets(0, 0, 0, 0) ;
                gbc.fill = GridBagConstraints.VERTICAL ;
                gbc.gridwidth = GridBagConstraints.REMAINDER ;
                m_lblImage = new PLabel(icon, PLabel.ALIGN_EAST) ;
                add(m_lblImage, gbc) ;
            }
        }

        public void repaint()
        {
            super.repaint() ;
            if (m_lblImage != null)
                m_lblImage.repaint() ;
            if (m_lblText != null)
                m_lblText.repaint() ;

        }
    }
}
