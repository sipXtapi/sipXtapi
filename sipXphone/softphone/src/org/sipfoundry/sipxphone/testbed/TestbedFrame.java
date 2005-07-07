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

package org.sipfoundry.sipxphone.testbed ;

import java.awt.* ;
import java.awt.event.* ;
import java.lang.*;
import java.io.* ;
import java.util.* ;

import org.sipfoundry.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.sys.util.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.telephony.phone.* ;

import org.sipfoundry.stapi.*;

import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;
import org.sipfoundry.sipxphone.app.preferences.DoNotDisturbFeatureIndicator ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager;

/**
 * The "TestbedFrame" is the application that launches the softphone.   This
 * code originally written as an emulation environment/development tool for
 * the xpressa hardphone.  It was originally used for application layer
 * development while the TCAS3 (old version of the xpressa phone) was being
 * designed/debugged.  It was then repackaged and distributed with the
 * xpressa development kit as a emulation tool for application developers.
 * It was also packaged as a product once it became popular.
 */
public class TestbedFrame extends Frame
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final Dimension DEFAULT_DIMENSION_SIZE = new Dimension(433,490) ;
    public static final String TESTBED_WORLD             = "TestbedWorld" ;
    public static final String SIPPHONE_PROPERTIES       = "testbed.properties" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    public static boolean m_bTestBedWorld = false ;
    protected static int m_siLicenceValid  = 0 ;
    protected static boolean m_sbBringToFront = true;
    protected static TestbedFrame  m_testbedReference = null;
    
    protected SoftPhone m_softPhone;
    protected TestbedButtonEvent m_TestbedButtonEvent = new TestbedButtonEvent() ;
    protected icKeyBoardActionListener mKeyListener = new icKeyBoardActionListener();
    protected Dimension m_ixPreferredSize = new Dimension();
    protected int miStartX = 0;
    protected int miStartY = 0;
    protected int miWidth = 0;
    protected int miHeight = 0;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock no argument constructor
     */
    public TestbedFrame(String strJNILayerPath)
    {
        super("sipXphone") ;
        m_testbedReference = this;

        loadJNILayer(strJNILayerPath) ;

        try
		{
            JNI_pingerStart() ;
        }
        catch (UnsatisfiedLinkError e) 
		{
            SysLog.log(e);
            return ;
        }

        // preload config files
        AppResourceManager.getInstance().addStringResourceFile(SIPPHONE_PROPERTIES) ;

        Image img = AppResourceManager.getInstance().getImage("imgPhoneIcon");
        this.setIconImage(img);

        m_bTestBedWorld = true ;

        // set frame properties
        setResizable(false) ;
        addWindowListener(new icWindowListener()) ;
        
        // Check that key-config is correct (for softphone)
        m_siLicenceValid = isLicenseValid() ;
        if (m_siLicenceValid==1)
        {
            // Start init - moved from PingerStart
            Init.main(null) ;

            // Wait for Init services to start
            try {
                Thread.sleep(1000) ;
            } catch (InterruptedException ie) { } ;

            try
            {
                ShellApp.getInstance() ;
            }
            catch (Exception e)
            {
                e.printStackTrace() ;
            }

            try {
                Thread.sleep(1000) ;
            } catch (InterruptedException ie) { } ;

            // Start JTAPI
            ShellApp.getInstance().initializeJTAPI() ;

            initializeDisplay() ;
            startDisplay() ;

            // Tell launcher to hide
            JNI_signalStartupFinished();

            PingerApp.completeStartup() ;
            m_sbBringToFront = PingerApp.getInstance().isBringToFrontOnCallEnabled();
        }
    }

    public static TestbedFrame getInstance()
    {
        return m_testbedReference;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

   /**
    * Main Method
    */
    public static void main(String argv[])
    {
        PingerApp.disableCursor() ;

        // Set environment variable saying we're using softphone
        Properties prop = System.getProperties() ;
        prop.put(TESTBED_WORLD, "true") ;
        System.setProperties(prop) ;

        String strJNILayerPath = prop.getProperty("softphone.library.path") ;
        TestbedFrame frame = new TestbedFrame(strJNILayerPath) ;
/*
        // If the license is not valid, then request a license key.
        if (m_siLicenceValid != 1)
        {
            // Tell launcher to hide
            JNI_signalStartupFinished();

            NoKeyAdFrame keyframe = new NoKeyAdFrame();
            keyframe.setSize(480,380);
            keyframe.setTitle("Get sip softphone key");

            Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            Dimension frameSize = keyframe.getSize();
            keyframe.setLocation((screenSize.width/2) - (frameSize.width/2),
                    (screenSize.height/2) - (frameSize.height/2));
            keyframe.setVisible(true);
        }
        else
*/	
        {
	        frame.installKeyboardFocusManager();
            frame.startDisplay();
        }
    }

    public static void bringToTop()
    {
	    Frame frame = getInstance() ;
        
		if (frame != null && m_sbBringToFront)
        {
           // Need to do this because it's broken in 1.1.8
           JNI_restoreMinimizedWindow();

           frame.setVisible(true);
           frame.toFront();
           frame.setEnabled(true);
           frame.requestFocus();
        }
    }

    /**
     * What is the preferred size of our application?
     *
     * @return Dimension representing the preferred size of our frame window
     */
    public Dimension getPreferredSize()
    {
        return m_ixPreferredSize;
    }


    public void shutdown(int iErrorCode)
    {
        JNI_shutdownHook() ;
	    System.exit(iErrorCode) ;
    }

    /**
     * Displays the softphone
     */
    public void startDisplay()
    {
        pack() ;
        setIXSize();
        setVisible(true) ;
        toFront();
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    protected boolean loadJNILayer(String strLibraryPath)
    {
        boolean bSuccess = false ;

        try
        {
            if ((strLibraryPath != null) && (strLibraryPath.length() > 0))
            {
                File fileCheck = new File(strLibraryPath) ;
                if (fileCheck.exists())
                {
                    System.load(strLibraryPath) ; 
                }
                else
                {
                    String fullPath = System.getProperty( "user.dir" ) ;
                    fullPath += File.separatorChar + strLibraryPath ;
                    System.load(fullPath) ;
                }
                bSuccess = true ;
            }
            else
            {
                System.loadLibrary("sipXphone") ;  // try to load using LD_LIBRARY_PATH
                bSuccess = true ;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace() ;
        }

        return bSuccess ;
    }



    protected void installKeyboardFocusManager()
    {
        KeyboardFocusManager mgr =
                KeyboardFocusManager.getCurrentKeyboardFocusManager() ;
        mgr.addKeyEventDispatcher(new icKeyDispatcher()) ;
    }

    protected void setIXSize()
    {
        int iStartX = m_softPhone.getStartX() ;
        int iStartY = m_softPhone.getStartY() ;
        int iWidth = m_softPhone.getAppWidth() ;
        int iHeight = m_softPhone.getAppHeight() ;
        miStartX = iStartX;
        miStartY = iStartY;
        miWidth = iWidth;
        miHeight = iHeight;
        Dimension d = new Dimension(iWidth,iHeight);

        setSize(iWidth,iHeight);

        if (iStartX != -1 && iStartY != -1)
            setLocation(iStartX,iStartY);
        else
        {
            //center on screen
            Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            Dimension frameSize = getSize();
            setLocation((screenSize.width/2) - (frameSize.width/2),
              (screenSize.height/2) - (frameSize.height/2));
        }

        m_ixPreferredSize = d;
    }


    protected void initializeDisplay()
    {
        // Start softphone
        m_softPhone = new SoftPhone(this) ;        
        add(m_softPhone);
    }


    /**
     *  Test whether we have a valid license
     */
    protected static int isLicenseValid()
    {
		return (PingerInfo.getInstance().isLicenseValid()) ;
    }

    /**
     *  Returns error code from license manager
     */
    protected static int getLicenseFailureCode()
    {
        return (PingerInfo.getInstance().getLicenseFailureCode()) ;
    }

    protected boolean isKeyInteresting(int iKeyCode)
    {
        boolean bInteresting ; 

        if (iKeyCode == KeyEvent.VK_SHIFT
                || iKeyCode == KeyEvent.VK_CONTROL 
                || iKeyCode == KeyEvent.VK_ALT 
                || iKeyCode == KeyEvent.VK_NUM_LOCK
                || iKeyCode == KeyEvent.VK_INSERT    
                || iKeyCode == KeyEvent.VK_PAGE_UP 
                || iKeyCode == KeyEvent.VK_PAGE_DOWN 
                || iKeyCode == KeyEvent.VK_DELETE)
        {
            bInteresting = false ;
        }
        else
        {
            bInteresting = true ;
        }

        return bInteresting;
    }


    protected boolean isKeySpecial(int iKeyCode)
    {
        boolean bSpecial ;

        if (    iKeyCode == KeyEvent.VK_HOME 
                || iKeyCode == KeyEvent.VK_END
                || iKeyCode == KeyEvent.VK_ENTER
                || iKeyCode == KeyEvent.VK_TAB
                || iKeyCode == KeyEvent.VK_BACK_SPACE
                || iKeyCode == KeyEvent.VK_LEFT
                || iKeyCode == KeyEvent.VK_RIGHT
                || iKeyCode == KeyEvent.VK_UP
                || iKeyCode == KeyEvent.VK_DOWN
                || (iKeyCode >= KeyEvent.VK_F1  && iKeyCode <= KeyEvent.VK_F2))
        {
            bSpecial = true ;
        }
        else
        {
            bSpecial = false ;
        }

        return bSpecial ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////
    /**
     *
     */
    protected class icWindowListener extends WindowAdapter
    {
        public void windowActivated(WindowEvent e)
        {
            requestFocus();
        }

        public void windowClosed(WindowEvent e)
		{
            setVisible(false);
        }

        public void windowOpened(WindowEvent e)
        {
             setEnabled(true);
             requestFocus();
        }

        public void windowClosing(WindowEvent e)
        {
            //tell lowerlayer to unregister
            PingerInfo.getInstance().unregisterAllLines();
            //and to unsubscribe
            PingerInfo.getInstance().unSubscribeAll();

            // loop through all the calls before exit.
            PCallManager callManager = Shell.getCallManager() ;
            PCall [] calls = callManager.getCalls() ;

            for (int loop  = 0; loop < calls.length; loop++)
            {
                if (calls[loop] != null)
                {
                    try
                    {
                        calls[loop].disconnect();
                    }
                    catch (Exception ex)
                    {
                    }
                }
            }
            // Flush the Repository...
            Repository.getInstance().flush() ;

            // Wait a tad for the calls to get the message to go away
            // Until we have a blocking disconnect...this will do.
            try
            {
                Thread.sleep(1000);
            }
            catch (Exception ee)
            {
            }

            shutdown(1) ;
        }
    }

	protected class icKeyDispatcher implements KeyEventDispatcher
    {
        boolean mbDisableKeyboard = false ;

        public icKeyDispatcher()
        {
            String strDisableKeyboard = AppResourceManager.getInstance().getString("DisableKeyboard");
            if (strDisableKeyboard.equals("1"))
                mbDisableKeyboard = true;
        }

        public boolean dispatchKeyEvent(KeyEvent e)
        {
            if (isKeyInteresting(e.getKeyCode()))
            {
                //if the skin said to disable keyboard events
                if (!mbDisableKeyboard)
                {
                    switch (e.getID())
                    {
                        case KeyEvent.KEY_PRESSED:
                            mKeyListener.keyPressed(e) ;
                            break ;
                        case KeyEvent.KEY_RELEASED:
                            mKeyListener.keyReleased(e) ;
                            break ;
                        case KeyEvent.KEY_TYPED:
                            mKeyListener.keyTyped(e) ;
                            break ;
                    }
                }
                return true ;
            }
            return false ;
        }
    }

	protected class icKeyBoardActionListener implements KeyListener
    {
        public void keyTyped(KeyEvent e)
        {
        }

        public void keyPressed(KeyEvent e)
        {
            if (isKeySpecial(e.getKeyCode()))
            {
                // Add 512 to key code -- this is used later to recognize the key as
                // a special code.
                m_TestbedButtonEvent.JNI_postButtonEvent(e.getKeyCode() + 512, PButtonEvent.KEY_DOWN);
            }
            else
            {
                m_TestbedButtonEvent.JNI_postButtonEvent(e.getKeyChar(), PButtonEvent.KEY_DOWN);
            }
        }

        public void keyReleased(KeyEvent e)
        {
            if (isKeySpecial(e.getKeyCode()))
            {
                // Add 512 to key code -- this is used later to recognize the key as
                // a special code.
                m_TestbedButtonEvent.JNI_postButtonEvent(e.getKeyCode() + 512, PButtonEvent.KEY_UP);
            }
            else
            {
                m_TestbedButtonEvent.JNI_postButtonEvent(e.getKeyChar(), PButtonEvent.KEY_UP);
            }
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    /** native call to start Pinger */
    public native static void JNI_pingerStart();

    public native static void JNI_restoreMinimizedWindow();

    /** native call to signal startup finished */
    public native static void JNI_signalStartupFinished();

    /** invoked when the system is closing down */
    public native static void JNI_shutdownHook() ;
}
