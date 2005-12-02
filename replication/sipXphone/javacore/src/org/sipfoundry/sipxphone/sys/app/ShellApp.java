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


package org.sipfoundry.sipxphone.sys.app ;

import java.awt.* ;
import java.util.Vector ;
import java.util.Hashtable ;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.* ;

//import org.sipfoundry.sipxphone.app.phonebook.* ;
import org.sipfoundry.sipxphone.app.preferences.CallHandlingManager ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.sys.app.core.* ;
import org.sipfoundry.sipxphone.hook.CoreAppButtonHook;

import org.sipfoundry.sipxphone.sys.calllog.*;
import org.sipfoundry.stapi.* ;

import javax.siplite.CallProvider;
import javax.siplite.Factory;
import javax.siplite.SIPStack;


import org.sipfoundry.sipxphone.sys.startup.PingerApp ;

/**
 * The ShellApp is the central beast that connects applications and services to
 * core system.  Applications and get references to this class and then either
 * post/send requests or call some methods directly.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ShellApp extends Application
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static boolean DEBUG_EVENTS = false ;
    public static boolean DEBUG_SAFE_WAITS = false ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** singleton reference to the one and only shell instance */
    protected static ShellApp m_shellReference = null ;
    /** JTAPI: provider */
    protected static Provider m_provider = null ;
    /** JTAPI: terminal */
    protected static Terminal m_terminal = null ;
    /** Core Phone Application */
    protected static CoreApp m_appCore = null ;

    protected static PMediaManager m_mediaManager = null ;
    protected static PCallManager m_callManager = null ;
    protected static AudioSourceControl m_asc ;

    //tracks if we hit the more for a hint, so we dont make a form change
    protected static boolean m_bWasMoreHint = false ;




    /** the actual physical display */
    protected Panel m_panelDisplay ;
    /** Form Manager: Manages the stack of forms */
    public FormManager m_formManager ;
    /** App Manager: Manages the list of Applications*/
    protected ApplicationManager m_appManager ;

    /** The shell has one displayable component: The task form */
    protected TaskForm m_formTask ;

    /** The shell also displays hints, this label is the actual representation */
    // protected PLabel m_lblHint = null ;
    protected PHintPopup m_lblHint = null ;
    /** The shell also displays fly overs, this label is the actual representation */
    protected PLabel m_lblFlyOver = null ;

    protected ChoiceListDropDown m_dropDownList = null ;

    /** Fly overs die after some fixed time period, icFlyoverKiller enforces this */
    protected icFlyoverKiller m_FlyoverKiller = null ;

    //protected PLabel m_lblStatus = null ;

    /** instance of status frame to show status */
    protected StatusFrame m_statusFrame;

    /** objSource which made the call to display status */
    protected Object m_objSourceForStatus;

    /** Event queue where Applications send/post events */
    private   icEventQueueItem m_queue = null ;
    /** object used to synchronize send events */
    private Object m_objMonitor = new Object() ;
    /** object used to synchronize the event queue */
    private Object m_objQueueMutex = new Object() ;

    private Hashtable m_htTempDispatchers = new Hashtable() ;

    /** singleton instance of JAIN SIPLITE FACTORY */
    private static Factory m_sipliteFactory;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor
     */
    public ShellApp()
    {
        m_shellReference = this ;

        m_panelDisplay = new Panel(null) ;
        m_panelDisplay.setSize(new Dimension(160, 160)) ;
        m_panelDisplay.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;

        AppResourceManager.getInstance().preloadImages(m_panelDisplay, "images.properties") ;
        AppResourceManager.getInstance().addStringResourceFile("hints.properties") ;
        AppResourceManager.getInstance().addStringResourceFile("tips.properties") ;
        AppResourceManager.getInstance().addStringResourceFile("help.properties") ;
        m_formTask = new TaskForm(this) ;


        m_formManager = new FormManager(m_panelDisplay) ;
        m_appManager = ApplicationManager.getInstance() ;

        // Create the core application world
        m_appCore = (CoreApp) m_appManager.loadApplication(Settings.getString("CORE_APPLICATION", "org.sipfoundry.sipxphone.sys.app.CoreApp"),"") ;
        m_FlyoverKiller = new icFlyoverKiller() ;
        m_appCore.initialize(m_formManager) ;

        // Initialize JAIN SIP Lite
        getSipliteFactory() ;
    }

    /**
     * Gets the singleton instance of JAIN SipLite factory.
     * @deprecated DO NOT EXPOSE
     */
    public static Factory getSipliteFactory()
    {
        if( m_sipliteFactory ==  null ){
            m_sipliteFactory = Factory.getInstance();
            //initialize the provider so that it can listen to all the calls.
            try{
                m_sipliteFactory.getSIPStack("org.sipfoundry.siplite").getCallProvider();
            }catch( javax.siplite.PeerUnavailableException e){
                SysLog.log(e);
            }
        }
        return m_sipliteFactory;
    }

    /**
     * Initialize the JTAPI world
     */
    public static void initializeJTAPI()
    {

        try {
            JtapiPeer peer = JtapiPeerFactory.getJtapiPeer("org.sipfoundry.telephony.PtJtapiPeer") ;

            m_provider = peer.getProvider("servicename;login=username;passwd=password;server=10.1.1.17:9000") ;

            Terminal terminals[] = m_provider.getTerminals() ;
            if (terminals.length > 0) {
                m_terminal = terminals[0] ;
            } else {
                System.out.println("ERROR:: No terminals are available, aborting JTAPI initialization!") ;
                return ;
            }

            // NOTE: The order of these listeners is important.  The phone state
            // is added first and passively records state.  This allows other
            // object to inquire about the current state and have it 'most'
            // accurate.
            m_terminal.addTerminalListener(new ButtonFeed()) ;

             //PingtelEventDispatcher.setCoreListener(m_appCore) ;
             //install coreAppButtonHook to make coreapp listen to button events.
             Shell.getHookManager().installHook(HookManager.HOOK_BUTTON, new CoreAppButtonHook()) ;
             m_terminal.addTerminalListener(m_appCore.getTerminalListener()) ;

            m_asc = new AudioSourceControl((PhoneTerminal) m_terminal) ;
            m_asc.initializeAudioLevels() ;
            PhoneHelper.getInstance().setAudioSourceControl(m_asc) ;

        } catch (Exception e) {
            System.out.println(e) ;
            SysLog.log(e) ;
        }

        m_callManager = STAPIFactory.getInstance().getCallManager() ;
        m_mediaManager = STAPIFactory.getInstance().getMediaManager() ;

        ShellApp.getInstance().getCoreApp().initializeCallState() ;
        //adding calllog connection listener to the CallManager
        ShellApp.getInstance().getCallManager().addConnectionListener(new CallLogConnectionListener());

        // Notify anyone that cares in the C++ layer that Java is up and ready
        // for work.
        PingerApp.JNI_notifyJavaInitialized() ;

    }



    /**
     * get a singleton reference to the Shell object
     *
     */
    public static ShellApp getInstance()
    {
        // Create if needed
        if (m_shellReference == null) {
            m_shellReference = (ShellApp) ApplicationManager.getInstance().loadApplication("org.sipfoundry.sipxphone.sys.app.ShellApp","") ;
        }
        return m_shellReference ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Accessor Methods
////

    /**
     * Safely suspend the specified thread.
     * <p>
     * Most of the the system/phone is stimulus based and reacts to some
     * sort of an event.  In most cases, these events come in the form of a
     * button press or an AWT event.  Our code must not block / suspend on
     * these event/ threads or we may lock up the phone.
     * <p>
     * To combat this potential problem, we create temporary event pumps
     * that pumps our event queues while the original event pump/dispatcher
     * is blocked.
     * <p>
     * @TODO We have a third source of events- JTAPI.  We currently do not
     * have control of that event stream and stop users from suspending
     * that queue.  This is something that needs to be addressed.
     */
    public boolean safeWait(Object obj)
    {
        boolean bSuccessful = true ;
        Thread thread = Thread.currentThread() ;

        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeWait: Blocking: " + thread) ;
        }

        PingtelEventDispatcher eventDispatcher = null ;

        /*
         * Start up a new AWT event dispatcher/pump if required
         */

        // We cannot touch the AWT event queue from here (package protected),
        // so we have an invader that lives in the awt package and exposes
        // what we need...

/*DWW:  I Commented these out because and have been using xpressa and instant xpressa
        for a few months without it.  Everything seems to be fine.

        AWTInvader invader = new AWTInvader() ;
        if (invader.isEventDispatcher(thread)) {
            invader.startEventDispatcher() ;
            synchronized (m_htTempDispatchers) {
                m_htTempDispatchers.put(obj, invader) ;
            }
        } else {
            invader = null ;
        }
*/
        /*
         * Start up a new Pingtel event dispatcher/pump if required
         */
        if (thread instanceof PingtelEventDispatcher) {
            if (DEBUG_EVENTS)
                System.out.println("shellapp : *********************************** CREATING EVENT DISPATCHER!");
            eventDispatcher = new PingtelEventDispatcher() ;
            eventDispatcher.start() ;
            synchronized (m_htTempDispatchers) {
                m_htTempDispatchers.put(obj, eventDispatcher) ;
            }
        }


        /*
         * Wait
         */
        try {
            synchronized (obj) {
                obj.wait() ;
            }
        } catch (InterruptedException e) {
            bSuccessful = false ;
        }

        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeWait: Woke: " + thread) ;
        }

        return bSuccessful ;
    }


    /**
     *
     */
    public void safeSuspend(Thread thread)
    {
        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeSuspend: Blocking: " + thread) ;
        }

        PingtelEventDispatcher eventDispatcher = null ;

        /*
         * Start up a new AWT event dispatcher/pump if required
         */

        // We cannot touch the AWT event queue from here (package protected),
        // so we have an invader that lives in the awt package and exposes
        // what we need...

/*DWW:  I Commented these out because and have been using xpressa and instant xpressa
        for a few months without it.  Everything seems to be fine.

        AWTInvader invader = new AWTInvader() ;
        if (invader.isEventDispatcher(thread)) {
            invader.startEventDispatcher() ;
            synchronized (m_htTempDispatchers) {
                m_htTempDispatchers.put(thread, invader) ;
            }
        } else {
            invader = null ;
        }
        */

        /*
         * Start up a new Pingtel event dispatcher/pump if required
         */
        if (thread instanceof PingtelEventDispatcher) {
            eventDispatcher = new PingtelEventDispatcher() ;
            eventDispatcher.start() ;
            synchronized (m_htTempDispatchers) {
                m_htTempDispatchers.put(thread, eventDispatcher) ;
            }
        }



        /*
         * Suspend our thread as requested
         */
        thread.suspend() ;

        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeSuspend: Woke: " + thread) ;
        }
    }


    /**
     *
     */
    public void safeResume(Thread thread)
    {
        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeResume: Waking: " + thread) ;
        }

        try {
            // First stop any other dispatcher that we may have started
            synchronized (m_htTempDispatchers) {
                Object objDispatcher = m_htTempDispatchers.get(thread) ;
                if (objDispatcher != null) {
                    if (objDispatcher instanceof PingtelEventDispatcher) {
                        ((PingtelEventDispatcher) objDispatcher).stopDispatching() ;
                    } else if (objDispatcher instanceof AWTInvader) {
                        ((AWTInvader) objDispatcher).stopEventDispatcher() ;
                    } else {
                        System.out.println("ShellApp::safeResume:: Unknown object stored as event dispatcher") ;
                    }
                    m_htTempDispatchers.remove(thread) ;
                }
            }
        } catch (Exception e) {
            System.out.println("Error trying to stop event dispatcher:") ;
            SysLog.log(e) ;
        }

        thread.resume() ;
    }


    /**
     *
     */
    public void safeNotify(Object obj)
    {
        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeNotify: Waking: " + obj) ;
        }

        try {
            synchronized (m_htTempDispatchers) {
                // First stop any other dispatcher that we may have started
                Object objDispatcher = m_htTempDispatchers.get(obj) ;
                if (objDispatcher != null) {
                    if (objDispatcher instanceof PingtelEventDispatcher) {
                        ((PingtelEventDispatcher) objDispatcher).stopDispatching() ;
                    } else if (objDispatcher instanceof AWTInvader) {
                        ((AWTInvader) objDispatcher).stopEventDispatcher() ;
                    } else {
                        System.out.println("ShellApp::safeNotify:: Unknown object stored as event dispatcher") ;
                    }
                    m_htTempDispatchers.remove(obj) ;
                }
            }
        } catch (Exception e) {
            System.out.println("Error trying to stop event dispatcher:") ;
            SysLog.log(e) ;
        }

        synchronized (obj) {
            obj.notify() ;
        }
    }



    /**
     * Safely join 2 threads.
     * <p>
     * Most of the the system/phone is stimulus based and reacts to some
     * sort of an event.  In most cases, these events come in the form of a
     * button press or an AWT event.  Our code must not block / suspend on
     * these event/ threads or we may lock up the phone.
     * <p>
     * To combat this potential problem, we create temporary event pumps
     * that pumps our event queues while the original event pump/dispatcher
     * is blocked.
     * <p>
     * @TODO We have a third source of events- JTAPI.  We currently do not
     * have control of that event stream and stop users from suspending
     * that queue.  This is something that needs to be addressed.
     */
    public void safeJoin(Thread threadJoinTo)
    {
        if (DEBUG_SAFE_WAITS) {
            System.out.println("@@@@ SafeJoin: Joining:") ;
            System.out.println("@@@@     this: " + Thread.currentThread()) ;
            System.out.println("@@@@  join to: " + threadJoinTo) ;
        }

        Thread threadSource  = Thread.currentThread() ;

        PingtelEventDispatcher eventDispatcher = null ;

        /*
         * Start up a new AWT event dispatcher/pump if required
         */

        // We cannot touch the AWT event queue from here (package protected),
        // so we have an invader that lives in the awt package and exposes
        // what we need...
        /*
        AWTInvader invader = new AWTInvader() ;
        if (invader.isEventDispatcher(threadSource)) {
            invader.startEventDispatcher() ;
        } else {
            invader = null ;
        }
*/
        /*
         * Start up a new Pingtel event dispatcher/pump if required
         */
        if (threadSource instanceof PingtelEventDispatcher) {
            eventDispatcher = new PingtelEventDispatcher() ;
            eventDispatcher.start() ;
        }


        /*
         * Join our threads as requested
         */
        try {
            threadJoinTo.join() ;
        } catch (InterruptedException ie) {
            /* burp */
        } finally {
            /*
             * Turn off/stop the pumps
             */
/*
            if (invader != null) {
                invader.stopEventDispatcher() ;
            }
*/
            if (eventDispatcher != null) {
                eventDispatcher.stopDispatching() ;
            }
        }
    }


    /**
     * Is the specified application in focus?
     *
     * @return boolean true if the application is in focus (has a form with
     *         control of the display) or false if inactive/error
     */
    public boolean isApplicationInFocus(String strAppName)
    {
        boolean bRC = false ;

        Application app = m_appManager.getApplicationByKey(strAppName) ;
        if (app != null) {
            PForm formActive = m_formManager.getActiveForm() ;
            if (formActive == m_appManager.getActiveForm(app)) {
                bRC = true ;
            }
        }

        return bRC ;
    }


    /**
     *
     */
    public Container getDisplayPanel()
    {
        return m_panelDisplay ;
    }


    /**
     * The system has a hook where, it will
     *
     *
     */
    public void handlePingtelEvent(PButtonEvent event)
    {

        // If status is being displayed, eat the button presses
        //except if its a B2
        if( m_statusFrame != null ){
            if(( event.getButtonID() ==  PButtonEvent.BID_B2 )
                &&  (event.getEventType() == PButtonEvent.BUTTON_UP)){
                //do something
                m_statusFrame.fireStatusAbortedEvent(this, null);
            }
            event.consume() ;
            return ;
        }

        // A button up/key up should clear a fly over
        if (m_lblFlyOver != null) {
            if ((event.getEventType() == PButtonEvent.BUTTON_UP) ||
                    (event.getEventType() == PButtonEvent.KEY_UP)) {
                Shell.getInstance().clearFlyOver() ;
            }
            event.consume() ;
            return ;
        }


        // If we are displaying a dropdown list, let that list process
        // the event
        if (m_dropDownList != null) {
            m_dropDownList.processPingtelEvent(event) ;
            event.consume() ;
            return ;
        }

        // If we are displaying a hint, consume extra ups and close on
        // downs.
        if (m_lblHint != null) {
            if ((event.getEventType() == PButtonEvent.BUTTON_UP) ||
                    (event.getEventType() == PButtonEvent.KEY_UP)) {
                Shell.getInstance().clearHint() ;
            }
            event.consume() ;
        }


        // Finally if this is the more key, make sure we handle it outself.
        if (event.getButtonID() == PButtonEvent.BID_PINGTEL) {

            if (m_lblHint == null  && event.getEventType() == event.BUTTON_REPEAT) //held more?
            {
                Shell.getInstance().displayHint(AppResourceManager.getInstance().getString("hint/core/system/more"));
                event.consume() ;
                m_bWasMoreHint = true;
            }
            else
            if ((event.getEventType() == PButtonEvent.BUTTON_UP) ||
                    (event.getEventType() == PButtonEvent.KEY_UP)) {

                //if more hint wasn't showing, then it's ok to call taskmanager.
                if (!m_bWasMoreHint)
                    onTaskManager() ;
                else
                    m_bWasMoreHint = false;
            }
            else
                event.consume() ;
        }
    }


    /**
     * Determine if the task manager is in focus
     */
    public boolean isTaskManagerInFocus()
    {
        return (m_formManager.getActiveForm() == m_formTask) ;

    }



    /**
     *
     */
    public Call createCall()
    {
        Call call = null ;
        try {
            call = m_provider.createCall() ;
        } catch (Exception e) {
            System.out.println("Unable to create call: " + e.toString()) ;
            SysLog.log(e) ;
        }

        return call ;
    }


    /**
     *
     */
    public AudioSourceControl getAudioSourceControl()
    {
        return m_asc ;
    }


    /**
     *
     */
    public static Terminal getTerminal()
    {
        return m_terminal ;

    }

    /**
     *
     */
    public Provider getProvider()
    {
        return m_provider ;
    }


    /**
     *
     */
    public void ignoreCall(PCall call)
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_IGNORECALL, this, call.getCallID(), null) ;
        postEvent(event) ;
    }


    /**
     *
     */
    public void ignoreCallByID(String strCallID)
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_IGNORECALL, this, strCallID, null) ;
        postEvent(event) ;
    }


    /**
     *
     */
    public void monitorCall(PCall call)
    {
//        ShellEvent event = new ShellEvent(ShellEvent.SE_MONITORCALL, this, call.getCallID(), null) ;
//        postEvent(event) ;
        m_appCore.monitorCall(call) ;
    }


    /**
     *
     */
    public CoreApp getCoreApp()
    {
        return m_appCore ;
    }


    /**
     * send a shell event.  Execution is returned after the event has been
     * processed.
     */
    public void sendEvent(ShellEvent event)
    {

        if (Thread.currentThread() instanceof PingtelEventDispatcher)
        {
            processShellEvent(event) ;
        }
        else
        {
            PingtelEventQueue.sendEvent(event) ;
        }
    }


    /**
     * post a shell event.  Execution is returned immediately after the
     * event has been queued.
     */
    public void postEvent(PingtelEvent event)
    {
        PingtelEventQueue.postEvent(event) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void onTaskManager()
    {
        PForm form = m_formManager.getActiveForm() ;

        if (form == m_formTask) {
            m_formTask.closeForm();
        } else {
            if (!(form instanceof MessageBox)) {
                m_formTask.setForm(form) ;
                m_formTask.showModeless() ;
            }
        }
    }


    /**
     *
     */
    private void doDisplayForm(PForm form, int iType, Thread thread)
    {
        // Tell the app manager about the new form
        if (m_appManager.pushForm(form, iType))
        {
            m_formManager.pushForm(form) ;
        }
    }


    /**
     *
     */
    private void doCloseForm(PForm form)
    {
        PForm formActive = m_formManager.getActiveForm() ;
        boolean bCloseTaskManager = false ;

        // Close the task manager if the task manager form is opened.
        if ((formActive == m_formTask) && (form != m_formTask))
        {
            if (m_formTask.getForm() == form)
            {
                bCloseTaskManager = true ;
            }
        }

        // Close all children first, before closing parent
        PForm children[] = m_formManager.getFormChildren(form) ;
        for (int i=0; i<children.length; i++)
        {
            // Check for loops and a null child
            if ((children[i] != form) && (children[i] != null))
                doCloseForm(children[i]) ;
        }
        m_formManager.popForm(form) ;

        if (bCloseTaskManager)
            onTaskManager() ;
    }


    protected void doIgnoreCall(String strCallID)
    {
        m_appCore.ignoreCallByID(strCallID) ;
    }


    protected void doMonitorCall(String strCallID)
    {
        m_appCore.monitorCallByID(strCallID) ;
    }


    /**
     *
     */
    public void doDisplayFlyOver(String strText, int iTimeOut, int iFont)
    {
        Timer.getInstance().resetTimer(iTimeOut, m_FlyoverKiller, null) ;
        if (m_lblFlyOver == null) {
            m_lblFlyOver = new PLabel(strText) ;
            m_lblFlyOver.setFont(SystemDefaults.getFont(iFont)) ;
            m_lblFlyOver.setBackgroundImage(m_lblFlyOver.getImage("imgTooltipFrame")) ;
            m_lblFlyOver.setInsets(new Insets(6, 10, 6, 10)) ;
            m_lblFlyOver.setBounds(0, 96, 160, 64) ;
            m_panelDisplay.add(m_lblFlyOver, 0) ;
            m_lblFlyOver.repaint() ;
        } else {
            m_lblFlyOver.setFont(SystemDefaults.getFont(iFont)) ;
            m_lblFlyOver.setText(strText) ;
        }
        Thread.currentThread().yield() ;
    }


    /**
     *
     */
    public void doClearFlyOver()
    {
        if (m_lblFlyOver != null) {
            m_panelDisplay.remove(m_lblFlyOver) ;
            m_lblFlyOver = null ;
            m_panelDisplay.repaint(10, 0, 96, 160, 64) ;
            Thread.currentThread().yield() ;
        }
    }


    /**
     *
     */
    public void doDisplayHint(String strText)
    {
        doDisplayHint(strText, null) ;
    }


    /**
     *
     */
    public void doDisplayHint(String strText, String strTitle)
    {
        if (m_lblHint != null) {
            m_panelDisplay.remove(m_lblHint) ;
        }

        m_lblHint = new PHintPopup(strText, strTitle) ;

        m_lblHint.setBounds(0, 82, 160, 78) ;
        m_panelDisplay.add(m_lblHint, 0) ;
        m_lblHint.repaint() ;
        // Thread.currentThread().yield() ;
    }



    /**
     *
     */
    public void doDisplayChoiceDropDown(PChoice choice)
    {
        m_dropDownList = new ChoiceListDropDown(choice) ;
        m_dropDownList.setBounds(13, 11, 147, 127) ;
        m_panelDisplay.add(m_dropDownList, 0) ;
        m_panelDisplay.validate() ;
        m_dropDownList.repaint() ;
        Thread.currentThread().yield() ;
    }


    /**
     *
     */
    public void doClearHint()
    {
        if (m_lblHint != null) {
            m_panelDisplay.remove(m_lblHint) ;
            m_lblHint = null ;
            m_panelDisplay.repaint(0, 82, 160, 78) ;
            Thread.currentThread().yield() ;
        }
    }

    ////////STATUS


    /**
     * Displays a staus window with the strText displayed.
     * Calls fireStatusOpenedEvent in m_statusFrame.
     * @param objSource source that asked to display status
     * @param strText text to be displayed in status frame
     * @param objParam2 with the current implementation, it is
      generally a PStatusListener that listens to status events.
    */
    public void doDisplayStatus(Object objSource,
                                String strText, Object objParam2)
    {
        if ((strText == null) || (strText.length() == 0))
            return ;

        if(m_statusFrame == null)
        {
            if ((objParam2 != null) && (objParam2 instanceof PStatusListener))
            {
                m_statusFrame= new StatusFrame(strText, true);
                m_statusFrame.addStatusListener((PStatusListener) objParam2);
            }
            else
            {
                m_statusFrame = new StatusFrame(strText, false);
            }
            m_objSourceForStatus = objSource;
            //m_statusFrame.setInsets(6, 10, 6, 10) ;
            m_statusFrame.setBounds(0, 96, 160, 64) ;
            m_panelDisplay.add(m_statusFrame, 0) ;
            m_statusFrame.repaint();
        }
        else
        {
            if (objSource == m_objSourceForStatus)
            {
                if (m_FlyoverKiller != null)
                    Timer.getInstance().removeTimers(m_FlyoverKiller) ;
                m_statusFrame.setText( strText );
            }
            else
            {
                // TODO: Handle stacking...
                m_objSourceForStatus = objSource ;
                if (m_FlyoverKiller != null)
                    Timer.getInstance().removeTimers(m_FlyoverKiller) ;
                m_statusFrame.setText( strText );
            }
        }
        m_statusFrame.fireStatusOpenedEvent(this, strText);
        Thread.currentThread().yield();
    }


     /**
      * clears the status frame.
      */
    public void doClearStatus(int iTimeOut){
        if (iTimeOut == 0)
        {
            if (m_statusFrame != null)
            {
                m_statusFrame.fireStatusClosedEvent(this, null);
                m_panelDisplay.remove(m_statusFrame) ;
                m_statusFrame = null;
                m_panelDisplay.repaint(0, 96, 160, 64) ;
                Thread.currentThread().yield() ;

            }
        }
        else
        {
            Timer.getInstance().addTimer(iTimeOut, m_FlyoverKiller, m_statusFrame) ;
        }

    }


    /**
     * adds a status listener to status frame.
     */
    public  void addStatusListener( PStatusListener listener ){
        if( m_statusFrame != null ){
            m_statusFrame.addStatusListener( listener );
        }
    }

    /**
     * removes a status listener from the status frame.
     */
    public void removeStatusListener( PStatusListener listener ){
        if( m_statusFrame != null ){
            m_statusFrame.removeStatusListener( listener );
        }
    }



    /**
     *
     */
    public void doNop()
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_NOP, this, null, null) ;
        CountingSemaphore semaphore = new CountingSemaphore(0, false) ;
        event.setSemaphore(semaphore) ;

        postEvent(event) ;

        // We we should wait for a response...
        try {
            semaphore.down() ;
        } catch (Exception e) {
            SysLog.log(e) ;
        }
    }




    /**
     *
     */
    public PMediaManager getMediaManager()
    {
        return m_mediaManager ;
    }


    /**
     *
     */
    public PCallManager getCallManager()
    {
        return m_callManager ;
    }

    /**
     * gets the FormManager.
     */
     public FormManager getFormManager()
    {
        return m_formManager ;
    }

    private void doDial()
    {
        new icDialThread().start() ;
    }

    protected class icDialThread extends Thread
    {
        public void run()
        {
            m_appCore.clearDialer() ;
            m_appCore.doDial() ;
        }
    }





    /**
     * process / dispatch and events sent directly to the shell
     */
    public void processShellEvent(ShellEvent event)
    {
        if (DEBUG_EVENTS)
        {
            System.out.println("<BEGIN> processEvent") ;
            System.out.println(event) ;
        }

        switch (event.getEventType()) {

            // Don't do anything...
            case ShellEvent.SE_NOP:
            {
                break ;
            }

            // Is the form in focus?
            case ShellEvent.SE_FOCUSCHECK:
            {
                boolean bInFocus = false ;

                if (DEBUG_EVENTS)
                    System.out.println("   <BEGIN> ShellEvent.SE_FOCUSCHECK") ;

                PForm form = (PForm) event.getObjParam1() ;
                if (form != null) {
                    if (m_formManager.isFormInFocus(form)) {
                        bInFocus = true ;
                    }

                }
                event.setReturnValue(new Boolean(bInFocus)) ;
                if (DEBUG_EVENTS)
                    System.out.println("   <END> ShellEvent.SE_FOCUSCHECK") ;
                break ;

            }


            // Is the form stacked?
            case ShellEvent.SE_STACKCHECK:
            {
                boolean bStacked = false ;

                PForm form = (PForm) event.getObjParam1() ;
                if (form != null) {
                    if (m_formManager.isFormStacked(form)) {
                        bStacked = true ;
                    }
                }
                event.setReturnValue(new Boolean(bStacked)) ;
                break ;
            }


            // Display modal status information
            case ShellEvent.SE_SHOWSTATUS:
            {
                String strText = (String) event.getObjParam1() ;
                if (strText != null) {
                    doDisplayStatus(event.getSource(),
                                    strText, event.getObjParam2()) ;
                } else {
                    doClearStatus(0) ;
                }
                break ;
            }


            // Clear status Information
            case ShellEvent.SE_CLEARSTATUS:
            {
                Integer integer = (Integer) event.getObjParam1() ;
                doClearStatus(integer.intValue()) ;
                break ;
            }


            // Show a Form
            case ShellEvent.SE_SHOWFORM:
            {
                if (DEBUG_EVENTS)
                    System.out.println("   <BEGIN> ShellEvent.SE_SHOWFORM") ;

                PForm form = (PForm) event.getObjParam1() ;
                if (form != null) {
                    Integer integer = (Integer) event.getObjParam2() ;
                    if (integer != null) {
                        doDisplayForm(form, integer.intValue(), event.getThread()) ;
                    } else {
                        doDisplayForm(form, PForm.MODAL, event.getThread()) ;
                    }
                } else {
                    throw new IllegalArgumentException("SE_SHOWFORM requires a form as objParam1") ;
                }
                if (DEBUG_EVENTS)
                    System.out.println("   <END> ShellEvent.SE_SHOWFORM") ;
                break ;
            }
            // Close a Form
            case ShellEvent.SE_CLOSEFORM:
            {
                if (DEBUG_EVENTS)
                    System.out.println("   <BEGIN> ShellEvent.SE_CLOSEFORM") ;
                PForm form = (PForm) event.getObjParam1() ;
                if (form != null) {
                    doCloseForm(form) ;
                }else {
                    throw new IllegalArgumentException("SE_CLOSEFORM requires a form as objParam1") ;
                }

                if (DEBUG_EVENTS)
                    System.out.println("   <END> ShellEvent.SE_CLOSEFORM") ;
                break ;
            }
            // Show a hint
            case ShellEvent.SE_SHOWHINT:
            {
                String strText = (String) event.getObjParam1() ;
                String strTitle = (String) event.getObjParam2() ;
                if (strText != null) {
                    doDisplayHint(strText, strTitle) ;
                } else {
                    doClearHint() ;
                }
                break ;
            }
            // Clear / Hide a hint
            case ShellEvent.SE_CLEARHINT:
            {
                doClearHint() ;
                break ;
            }
            // Show a Fly over
            case ShellEvent.SE_SHOWFLYOVER:
            {
                String strText = (String) event.getObjParam1() ;
                if (strText != null) {
                    Vector vDurationAndFont = (Vector) event.getObjParam2() ;
                    int iTimeout = Settings.getInt("FLYOVER_DEFAULT_TIMEOUT", 6000) ;
                    int iFont    = SystemDefaults.FONTID_DEFAULT_SMALL;

                    if((vDurationAndFont != null) && (vDurationAndFont.size() == 2)){
                        Integer timeout = (Integer)(vDurationAndFont.elementAt(0));
                        Integer font    = (Integer)(vDurationAndFont.elementAt(1));
                        // Figure out what our timeout and font should be...
                        if( timeout != null )
                            iTimeout = timeout.intValue() ;
                        if( font != null )
                            iFont    = font.intValue() ;
                    }
                    // Finally display the hint.
                    doDisplayFlyOver(strText, iTimeout, iFont) ;
                } else {
                    doClearFlyOver() ;
                }
                break ;

            }
            // Clear a Flyover
            case ShellEvent.SE_CLEARFLYOVER:
            {
                doClearFlyOver() ;
                break ;
            }

            // Invoke the dialer
            case ShellEvent.SE_DIAL:
            {
                doDial() ;
                break ;
            }

            case ShellEvent.SE_IGNORECALL:
            {
                String strCallID = (String) event.getObjParam1() ;
                doIgnoreCall(strCallID) ;
                break ;
            }

            case ShellEvent.SE_MONITORCALL:
            {
                String strCallID = (String) event.getObjParam1() ;
                doMonitorCall(strCallID) ;
                break ;
            }
        }

        if (DEBUG_EVENTS)
            System.out.println("<END> processEvent") ;
    }


    /**
     * get the next enqueued event in the shell event queue
     */
    private PingtelEvent getNextEvent()
        throws InterruptedException
    {
        PingtelEvent oldEvent ;
        PingtelEvent event = null ;

        // Wait for an event to get posted...
        while (m_queue == null) {
            synchronized (m_objMonitor) {
                m_objMonitor.wait() ;
            }
        }

        synchronized (m_objQueueMutex) {
            oldEvent = event ;
            event = m_queue.event ;
            m_queue = m_queue.next ;
            oldEvent = null ;
        }


        return event  ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner and nested Classes
////
   /**
    * A linked list style event queue item.
    */
    private class icEventQueueItem
    {
        PingtelEvent event ;
        icEventQueueItem next ;
        Thread thread ;

        public icEventQueueItem(PingtelEvent event)
        {
            this.event = event ;
            this.next  =  null ;
        }
    }


    /**
     * Action Listener responsible for clearing a fly over
     */
    private class icFlyoverKiller implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if ((event.getObjectParam() == m_statusFrame) && (m_statusFrame != null)) {
                doClearStatus(0) ;
            } else {
                doClearFlyOver() ;
            }
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Application entrypoint
////
    /**
     *
     */
    private synchronized void waitForDeath()
    {
        try {
            wait() ;
        } catch (InterruptedException e) { }
    }


    /**
     * The Shell thread sits around and pumps the shell event queue.  This
     * implementation must be public, however, should not be modified.
     */
    public void main(String argv[])
    {
        waitForDeath()  ;
    }


    public class ChoiceListDropDown extends PContainer
    {
        Image             m_imgBackground = null  ;
        PList             m_listItems ;
        PDefaultListModel m_listModel ;
        PChoice           m_choice ;


        public ChoiceListDropDown(PChoice choice)
        {
            setBackgroundImage(getImage("imgChoiceDropDownFrame")) ;
            setOpaque(true) ;

            m_choice = choice ;
            m_listModel = new PDefaultListModel() ;
            for (int i=0; i<m_choice.getItemCount(); i++) {
                m_listModel.addElement(m_choice.getItem(i)) ;
            }
            m_listItems = new PList(m_listModel) ;
            m_listItems.setOpaque(false) ;

            add(m_listItems) ;
        }

        public void doLayout()
        {
            m_listItems.setBounds(6, 13, 138, 108) ;
        }
    }
}










