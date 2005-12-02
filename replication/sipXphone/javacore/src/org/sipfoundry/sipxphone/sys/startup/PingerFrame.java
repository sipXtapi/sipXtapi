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


package org.sipfoundry.sipxphone.sys.startup ;

import java.awt.* ;
import java.awt.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

/**
 * Basic frame for the hard phone.  The size is hardcoded to 160x160 for the
 * time being.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PingerFrame extends Frame
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected     Image       m_imgSplash ;
    protected     Component   m_compDisplaying ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * stock no argument constructor
     */
    public PingerFrame()
    {
        super("Pinger") ;

        m_compDisplaying = null ;
        setResizable(false) ;

        addWindowListener(new icWindowCloser()) ;
        setLayout(new GridLayout(1,1)) ;

        // Add the splash panel at first....
        AppResourceManager appResourceManager = AppResourceManager.getInstance() ;
        PComponent splashComponent = new PComponent() ;
        splashComponent.setBounds(0,0,160,160) ;
        m_imgSplash = appResourceManager.getImage("imgSplashScreen") ;
        splashComponent.setBackgroundImage(m_imgSplash) ;
        setDisplayComponent(splashComponent) ;

        try
        {
            MediaTracker tracker = new MediaTracker(splashComponent) ;
            tracker.addImage(m_imgSplash, 1) ;
            tracker.waitForAll() ;
        }
        catch (Exception e)
        {
            System.out.println("Exception while waiting for media tracker to load splash image:") ;
            SysLog.log(e) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * What is the preferred size of our application?
     *
     * @return Dimension representing the preferred size of our frame window
     */
    public Dimension getPreferredSize()
    {
        return PingerApp.DEFAULT_DIMENSION_SIZE ;
    }


    /**
     * Get the component currently being displayed.
     */
    public synchronized Component getDisplayComponent()
    {
        return m_compDisplaying ;
    }


    /**
     * Set the component that should be displayed
     */
    public synchronized Component setDisplayComponent(Component comp)
    {
        Component oldComponent = m_compDisplaying ;

        if (m_compDisplaying != null)
        {
            remove(m_compDisplaying) ;
        }

        m_compDisplaying = comp ;
        add(comp) ;
        pack() ;
		comp.repaint() ;
		
        return m_compDisplaying ;
    }


    /**
     * Called externally to startup the shell application
     */
    public Thread startShell()
    {
        icStartShellThread startupThread = new icStartShellThread() ;

        startupThread.start() ;

        return startupThread ;
    }


    public void handleStartShell()
    {
        setDisplayComponent(ShellApp.getInstance().getDisplayPanel()) ;
        m_imgSplash.flush() ;
        m_imgSplash = null ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

//////////////////////////////////////////////////////////////////////////////
// Nested classes
////

    /**
     * Handle window closing
     */
    protected class icWindowCloser extends WindowAdapter
    {
        public void windowClosing(WindowEvent e)
        {
            System.exit(1) ;
        }
    }

    /**
     * Thread to startup the shell, but allow us to continue initialization
     */
    protected class icStartShellThread extends Thread
    {
        public void run()
        {
            handleStartShell() ;
        }
    }
}
