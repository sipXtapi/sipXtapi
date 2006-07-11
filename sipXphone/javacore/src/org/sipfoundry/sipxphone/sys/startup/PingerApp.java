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

import java.io.* ;
import java.awt.* ;
import java.util.* ;
import java.text.MessageFormat ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.NetUtilities;
import org.sipfoundry.sipxphone.app.preferences.DoNotDisturbFeatureIndicator ;
import org.sipfoundry.sipxphone.app.preferences.CallForwardingFeatureIndicator ;

import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.service.Timer;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.awt.*;

import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager;

/**
 * Entrypoint for the Pinger Application.  This code instantiates a PingerFrame
 * and coordinates initialization.  The initalization code is fairly complex
 * and ugly -- make sure you understand the code before making major changes.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PingerApp
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Default size of our frame window */
    public static final Dimension DEFAULT_DIMENSION_SIZE = new Dimension(160,160) ;

    public static final String TESTBED_WORLD = "TestbedWorld" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static boolean bWarnedLowMemoryCondition = false ;

    /**
     * The error form to display when a phone is not able to
     * obtain DHCP.
     */
    private  SimpleTextForm m_dhcpErrorForm;

    /**
     * The action Listener to listen to events in the DHCP error form.
     */
    private  icDHCPErrorFormEventDispatcher m_dispatcher =
                                new icDHCPErrorFormEventDispatcher();

    /**
     * Singleton instance of PingerApp.
     */
    private static PingerApp pingerApp ;

    private static PingerFrame s_frameWindow ;  // Reference to the frame window


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Gets a singleton instance of PingerApp. Not very useful as most of the
     * methods are static anyway.
     */
    public static PingerApp getInstance()
    {
        if (pingerApp == null)
            pingerApp = new PingerApp();

        return pingerApp;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * What component is currently being displayed?  This method only works
     * under the hard phone.
     */
    public static Component getDisplayComponent()
    {
        Component compRC = null ;

        PingerFrame frame = s_frameWindow ;
        if (frame != null)
        {
            compRC = frame.getDisplayComponent() ;
        }

        return compRC ;
    }

    /**
     * Apply the current time zone settings
     */
    static public void applyTimezoneSettings()
    {
        JNI_ApplyTimezoneSettings() ;

        initializeTimeZone() ;
    }


    /**
     * Stop the cursor from being displayed under vxWorks
     */
    static public void disableCursor()
    {
        Properties props = System.getProperties() ;

        // Resetting the Default Cursor/Cursor Class to blank
        // disables the cursor on vxWorks
        props.put("sun.awt.aw.DefaultCursor", "blank") ;
        props.put("sun.awt.aw.NonExistentCursorClass", "blank") ;

        System.setProperties(props) ;
    }



    /**
     * Should the softphone bring the GUI to the front of the screen when a
     * call comes in
     */
    public boolean isBringToFrontOnCallEnabled()
    {
        return !"DISABLE".equalsIgnoreCase(PingerConfig.getInstance().getValue(
            PingerConfig.PHONESET_BRING_TO_FRONT_ON_CALL));
    }

    /**
     * Is this the testbed environment?  (a.k.a. the softphone)
     */
    public static boolean isTestbedWorld()
    {
        Properties props = System.getProperties() ;
        if (props.get(TESTBED_WORLD)=="true")
            return true ;
        else
            return false ;
    }


    /**
     * Helper routine that returns the list of URLs/Application in the jrunonce file
     */
    public static Vector getRunOneList()
    {
        String strRunOnceFile = PingerInfo.getInstance().getFlashFileSystemLocation() + File.separator + "jrunonce" ;
        Vector vRC = new Vector() ;
        File fileRunOnce = new File(strRunOnceFile) ;

        // Only bother processing if the file exists
        if (fileRunOnce.exists()) {

            // Parse each line of text file and create vector of those lines
            try {
                BufferedReader reader = new BufferedReader(new FileReader(fileRunOnce)) ;
                String strAppURL = reader.readLine() ;
                while (strAppURL != null) {
                    vRC.addElement(strAppURL) ;
                    strAppURL = reader.readLine() ;
                }
                reader.close() ;

            } catch (IOException e) {
                SysLog.log(e) ;
            }

            // Delete the file
            fileRunOnce.delete() ;
        }

        return vRC ;
    }


    /**
     * This looks for applications in the jrunonce file in the file system.
     * Anything found within there is run/executed once.  This routine is
     * called before the initializeApplications so that it can manipulate
     * the app load list.
     */
    public static void processRunOnceApplications()
    {
        ApplicationManager  appManager = ApplicationManager.getInstance() ;
        ApplicationRegistry appRegistry = ApplicationRegistry.getInstance() ;
        Vector              vRunList = getRunOneList() ;

        int iNumItems = vRunList.size() ;
        for (int i=0; i<iNumItems; i++) {
            try {
                String strAppURL = (String) vRunList.elementAt(i) ;
                appRegistry.addApplication(strAppURL) ;
                appManager.preloadApplication(strAppURL) ;
                appManager.startApplication(strAppURL) ;
            } catch (Exception e) {
                SysLog.log(e) ;
            }
        }
    }

    /**
     * Populate the application list by loading application descriptors and
     * then start whatever applications that should be loaded/started.
     * If the "maxmimum user applications limit" has been exceeded,
     * it pops up a warning message box with appropriate message.
     */
    public static void initializeApplications()
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        ApplicationRegistry appRegistry = ApplicationRegistry.getInstance() ;
        if (appManager != null)
        {
            try {
                appRegistry.populateApplicationDescriptors() ;
                if( appRegistry.isMaxUserAppLimitExceeded() ){
                    MessageBox msgBox =
                        new MessageBox(ShellApp.getInstance().getCoreApp(),
                                       MessageBox.TYPE_WARNING) ;
                    msgBox.setMessage
                        (AppResourceManager.getInstance().getString
                         ("lblWarningMaxUserAppsLimitExceeded")) ;
                    msgBox.showModal() ;
                }
                appManager.preloadRegisteredApplications() ;
                appManager.startRegisteredApplications() ;
            } catch (Exception e) {
                SysLog.log(e) ;
            }
        }
    }


    public static void remapOutputStreams()
    {
        // Redirect all the System.err.print messages to both SysLog and console
        System.setErr( new PrintStreamAdapter(new SysLogOutputStreamAdapter()) ) ;
        System.setErr( new PrintStreamAdapter(new ConsoleOutputStreamAdapter()) ) ;

        String strRedirect =
            PingerConfig.getInstance().getValue(PingerConfig.JAVA_OUTPUTSTREAM_REDIRECT) ;

        if ((strRedirect != null) && strRedirect.trim().equalsIgnoreCase("CONSOLE"))
        {
            if (PingerApp.isTestbedWorld())
            {
                // Do nothing if we are in the testbed world
            }
            else
            {
                System.setOut( new PrintStreamAdapter(new ConsoleOutputStreamAdapter()) ) ;
            }
        }
        else
        {
            // Reset the output streams to null
            System.setOut(new PrintStream(new OutputStreamGobbler())) ;
        }
    }


    /**
     * Displays a warning message if the default ring file is missing
     */
    public static void reportIfDefaultRingFileIsMissing(){
        if( ! (PingerInfo.getInstance().doesDefaultRingFileExists()) ){
            MessageBox msgBox = new MessageBox
                (ShellApp.getInstance().getCoreApp(),
                 MessageBox.TYPE_ERROR) ;
            msgBox.setMessage
                (AppResourceManager.getInstance().getString
                 ("lblWarningDefaultRingNotAvailable")) ;
            msgBox.showModal() ;
        }
    }


    /**
     * Check the free memory availability and force a garbage collection or or
     * warning if warranted.
     *
     * @return true if we are in a low memory condition otherwise false
     */
    public static boolean checkLowMemoryCondition(boolean bWarn)
    {
        boolean        bDangerousCondition = false ;
        Runtime        runtime = Runtime.getRuntime() ;

        long lTotalMemory = runtime.totalMemory() ;
        long lFreeMemory = runtime.freeMemory() ;
        long lNewFreeMemory = lFreeMemory ;

        // Do we need to force a garbage collection?
        float fPercentFree = (((float) lFreeMemory / (float) lTotalMemory) * (float) 100.0) ;
        System.out.println("Free memory percentage: " + fPercentFree) ;
        if (fPercentFree < getForceGCThreshold())
        {
            System.out.println("WARNING: Low Memory Condition") ;
            System.out.println("  Total:" + lTotalMemory) ;
            System.out.println("   Free:" + lFreeMemory) ;
            System.out.println("   Forcing GC...") ;
            runtime.gc() ;
            System.out.println("   Forcing Finalization...") ;
            runtime.runFinalization() ;
            lNewFreeMemory = runtime.freeMemory() ;
            System.out.println("   Free:" + lNewFreeMemory) ;
        }

        // Should we warn the user?
        if (bWarn)
        {
            float fAfterGCPercentFree = (((float) lNewFreeMemory / (float) lTotalMemory) * (float) 100.0) ;
            System.out.println("Free memory percentage: " + fAfterGCPercentFree) ;
            if (fAfterGCPercentFree < getLowMemoryThreshold())
            {
                Application app = ShellApp.getInstance().getCoreApp() ;
                MessageBox mbWarning = new MessageBox(app, MessageBox.TYPE_WARNING) ;
                mbWarning.setMessage(mbWarning.getString("lblWarningLowMemory")) ;
                if (mbWarning.showModal() == MessageBox.CANCEL)
                {
                    bDangerousCondition = true ;
                }
            }
        }
        return bDangerousCondition ;
    }

    /**
     * ** MAIN ENTRY POINT **
     *
     * Called from JNI to startup the Java application layer
     */
    public static void startPingerApp()
    {
        disableCursor() ;

        if (PingerInfo.getInstance().isEVM())
        {
            //
            // Under Insignia's EVM, we need to load the JNI layer explicitly
            // -- even though the binaries are linked into the object.  I
            // believe this forces a load of that lib's symbol table.  In
            // previous revisions, the EVM simply used the global vxWorks
            // symbol table.
            //

            try
            {
                System.loadLibrary("xpressa_jni_layer");
            }
            catch (Throwable t)
            {
                t.printStackTrace() ;
            }
        }

        initializeTimeZone() ;
        remapOutputStreams() ;

        // The orignal specs had a Init application that would kick off the
        // entire world.  We have moved towards that, but not completely.
        Init.main(null) ;

        // The PingtelFrame is used to display this application on vxWorks
        s_frameWindow = new PingerFrame() ;
        s_frameWindow.setVisible(true) ;


        Thread threadStartupShell = s_frameWindow.startShell() ;
        try
        {
            // While waiting, exercise fonts
            if (!isTestbedWorld())
                exerciseFonts(s_frameWindow) ;

            threadStartupShell.join() ;
        }
        catch (InterruptedException e) { }

        // Give the GUI nudge to catch up and paint.
        Thread.currentThread().yield() ;

        // Initialize JTAPI
        ShellApp.initializeJTAPI() ;

        // Common initialization
        completeStartup() ;
    }


    /**
     * Initialization common across xpressa and instant xpressa
     */
    public static void completeStartup()
    {
        // Process any runonce applications
        processRunOnceApplications() ;

        // Start accepting calls
        ShellApp.getInstance().getCoreApp().setAcceptCalls(true) ;

        // Load/Startup the application list
        initializeApplications() ;

        // Report a warning if the default ring file is missing
        reportIfDefaultRingFileIsMissing();

        // Display DND indicator if we come up with DND set.
        DoNotDisturbFeatureIndicator fiDND = DoNotDisturbFeatureIndicator.getInstance() ;
        if (fiDND.shouldInstall()) {
            fiDND.install() ;
        }

        // Display the call forwarding feature indicator (if necessary)
        CallForwardingFeatureIndicator fiCW = CallForwardingFeatureIndicator.getInstance() ;
        if (fiCW.shouldInstall())
        {
            fiCW.install() ;
        }
    }



    /**
     * org.sipfoundry.PingerApp entrypoint.
     *
     * @param argv[] array of string arguments
     */
    public static void main(String argv[])
    {
        disableCursor() ;
        startPingerApp();
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * initialize the time zone from the pinger config settings
     */
    static protected void initializeTimeZone()
    {
        int iTimeOffset = JNI_getTimeOffset() ;
        int iDSTRule = JNI_getDSTRRule() ;

        SimpleTimeZone stz ;

        switch (iDSTRule) {
            case 0:
                stz = new SimpleTimeZone(iTimeOffset*60*1000, "PT0",
                    Calendar.MARCH, -1, Calendar.SUNDAY, 2*60*60*1000,
                    Calendar.SEPTEMBER, -1, Calendar.SUNDAY, 2*60*60*1000) ;
                break ;
            case 1:
                stz = new SimpleTimeZone(iTimeOffset*60*1000, "PT1",
                    Calendar.APRIL, 1, Calendar.SUNDAY, 2*60*60*1000,
                    Calendar.OCTOBER, -1, Calendar.SUNDAY, 2*60*60*1000) ;
                break ;
            default:
                stz = new SimpleTimeZone(iTimeOffset*60*1000, "PT2") ;
                break ;
        }
        TimeZone.setDefault(stz) ;
    }





    /**
     * Get the configured "low memory threshold" percent.  This value is used
     * to signal a warning that the user should close some applications.
     */
    protected static float getLowMemoryThreshold()
    {
        PingerConfig config = PingerConfig.getInstance() ;
        float fPercent = 15.0f ;

        String strValue = config.getValue(PingerConfig.JAVA_LOW_MEMORY_THRESHOLD_PERCENT) ;
        if ((strValue != null) && (strValue.length() > 0)) {
            try {
                fPercent = (Float.valueOf(strValue)).floatValue() ;
            } catch (NumberFormatException nfe) {
                SysLog.log(nfe);
            }
        }
        return fPercent ;
    }


    /**
     * Get the configured "force garbage collection threshold" percent.  This
     * value is used to determine if/when we should force a GC.
     */
    protected static float getForceGCThreshold()
    {
        PingerConfig config = PingerConfig.getInstance() ;
        float fPercent = 30.0f ;

        String strValue = config.getValue(PingerConfig.JAVA_FORCE_GC_THRESHOLD_PERCENT) ;
        if ((strValue != null) && (strValue.length() > 0)) {
            try {
                fPercent = (Float.valueOf(strValue)).floatValue() ;
            } catch (NumberFormatException nfe) {
                SysLog.log(nfe);
            }
        }
        return fPercent ;
    }




    /**
     * Exercise a single font, by drawing all normal printable characters
     * to an offscreen bitmap
     */
    protected static void exerciseFont(Graphics g, Font font)
    {
        g.setFont(font);

        char data[] = new char[1] ;
        for (char c=0x20; c<0x7F; c++)
        {
            data[0] = c ;
            g.drawChars(data, 0, 1, 0, 39) ;
        }
    }


    /**
     *
     */
    protected static void exerciseFonts(Frame frame)
    {
        Image imgOffscreen = frame.createImage(40, 40) ;
        Graphics og = imgOffscreen.getGraphics() ;

        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_MENU_ENTRY)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_TAB_ENTRY)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_TITLEBAR)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_HINT)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_HINT_TITLE)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_CALLERID_SMALL)) ;
        exerciseFont(og, SystemDefaults.getFont(SystemDefaults.FONTID_CALLERID_LARGE)) ;

        // Clean Up
        og.dispose() ;
        imgOffscreen.flush() ;
    }



    /**
     * Shows a error form which displays that a DHCP error has occured and the
     * phone will automatically reboot in configurable time( PHONESET_DHCP_FAILURE_RESTART_PERIOD ).
     * If "Cancel" is not pressed, the phone will reboot in that configured time +- 5 minutes.
     * If "Restart now" is pressed, the phone will reboot immediately.
     */
    private  void  showDHCPErrorForm(){
        m_dhcpErrorForm = new SimpleTextForm(ShellApp.getInstance().getCoreApp(),
            "DHCP Error") ;
        PActionItem cancelActionItem = new PActionItem
            (new PLabel("Cancel"),
             AppResourceManager.getInstance().getString("hint/preference/network/cancel_restart" ),
             m_dispatcher,
             (m_dispatcher.ACTION_CANCEL));
        PActionItem restartActionItem = new PActionItem
            (new PLabel("Restart Now"),
             AppResourceManager.getInstance().getString("hint/preference/network/restart_because_dhcp_failed" ),
             m_dispatcher,
             (m_dispatcher.ACTION_RESTART_NOW));

        PActionItem[] actionItems = new PActionItem[2];
        actionItems[0] = cancelActionItem;
        actionItems[1] = restartActionItem;
        m_dhcpErrorForm.getBottomButtonBar().setItem(PBottomButtonBar.B2, cancelActionItem);
        m_dhcpErrorForm.getBottomButtonBar().setItem(PBottomButtonBar.B3, restartActionItem);
        m_dhcpErrorForm.setLeftMenu(actionItems);
        int iTimeoutInMins = Settings.getInt
                ("PHONESET_DHCP_FAILURE_RESTART_PERIOD", 15);
        String strErrorMessage = AppResourceManager.getInstance().getString("lblErrorDHCPUnavailable1")+
                                " "+iTimeoutInMins +" "+
                                AppResourceManager.getInstance().getString("lblErrorDHCPUnavailable2");
        m_dhcpErrorForm.setText(strErrorMessage);

        //to avoid heavy load on supporting servers, the restart time should
        //be randomized slightly( +- 5 mins )
        int iRandom = (new Random().nextInt())%6;
        int iActualTimeoutInMins = iTimeoutInMins + iRandom;
        Timer.getInstance().addTimer( (iActualTimeoutInMins*60*1000), m_dispatcher, null )  ;
        m_dhcpErrorForm.showModal();
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native int JNI_getTimeOffset() ;
    protected static native int JNI_getDSTRRule() ;
    protected static native void JNI_ApplyTimezoneSettings() ;
    protected static native void JNI_consolePrint(String strOutput, int iType) ;

    public static native void JNI_notifyJavaInitialized() ;


//////////////////////////////////////////////////////////////////////////////
// Nested/Inner classes
////
    /**
     * Action Listener to listen to Cancel, Restart Now and Timer in
     * the DHCP error form.
     */
    private class icDHCPErrorFormEventDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL = "action_cancel" ;
        public final String ACTION_RESTART_NOW = "action_restart_now" ;
        boolean bRestart = true;
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                m_dhcpErrorForm.closeForm();
                bRestart = false;
            }
            else if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                if( bRestart )
                     PingerInfo.getInstance().reboot() ;
            }
            else if (event.getActionCommand().equals(ACTION_RESTART_NOW))
            {
                 PingerInfo.getInstance().reboot() ;
            }

        }
    }

}

