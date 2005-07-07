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
import java.io.* ;
import java.awt.event.* ;
import java.util.Date;
import java.util.Hashtable;
import java.util.Properties;
import java.util.PropertyResourceBundle;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.sipxphone.* ;

/**
 *
 */
public class SoftPhone extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int LEFT_COL_INDENT = 0 ; // indent for when using LH column - 0 for now

    /** dimensions of all the components on the softphone */
    public static final Dimension DIM_BACKGROUND_IMAGE = new Dimension(433,500) ;

    /** absolute positions of all the components on the softphone */
    protected static final Point POINT_SHELL = new Point(LEFT_COL_INDENT + 66, 59) ;

    protected static final Point POINT_L1 = new Point(LEFT_COL_INDENT + 11,86) ;
    protected static final Point POINT_L2 = new Point(LEFT_COL_INDENT + 11,112) ;
    protected static final Point POINT_L3 = new Point(LEFT_COL_INDENT + 11,140) ;
    protected static final Point POINT_L4 = new Point(LEFT_COL_INDENT + 11,166) ;

    protected static final Point POINT_R1 = new Point(LEFT_COL_INDENT + 243,84) ;
    protected static final Point POINT_R2 = new Point(LEFT_COL_INDENT + 243,113) ;
    protected static final Point POINT_R3 = new Point(LEFT_COL_INDENT + 243,140) ;
    protected static final Point POINT_R4 = new Point(LEFT_COL_INDENT + 243,168) ;

    protected static final Point POINT_B1 = new Point(LEFT_COL_INDENT + 73,229) ;
    protected static final Point POINT_B2 = new Point(LEFT_COL_INDENT + 126,229) ;
    protected static final Point POINT_B3 = new Point(LEFT_COL_INDENT + 180,229) ;

    protected static final Point POINT_1 = new Point(LEFT_COL_INDENT + 16,306) ;
    protected static final Point POINT_2 = new Point(LEFT_COL_INDENT + 59,306) ;
    protected static final Point POINT_3 = new Point(LEFT_COL_INDENT + 107,306) ;
    protected static final Point POINT_4 = new Point(LEFT_COL_INDENT + 17,340) ;
    protected static final Point POINT_5 = new Point(LEFT_COL_INDENT + 59,340) ;
    protected static final Point POINT_6 = new Point(LEFT_COL_INDENT + 107,340) ;
    protected static final Point POINT_7 = new Point(LEFT_COL_INDENT + 16,377) ;
    protected static final Point POINT_8 = new Point(LEFT_COL_INDENT + 59,377) ;
    protected static final Point POINT_9 = new Point(LEFT_COL_INDENT + 107,377) ;
    protected static final Point POINT_0 = new Point(LEFT_COL_INDENT + 59,417) ;
    protected static final Point POINT_STAR = new Point(LEFT_COL_INDENT + 17,417) ;
    protected static final Point POINT_POUND = new Point(LEFT_COL_INDENT + 107,417) ;

    protected static final Point POINT_MORE = new Point(LEFT_COL_INDENT + 306,223) ;
    protected static final Point POINT_HDSET = new Point(LEFT_COL_INDENT + 180,307) ;
    protected static final Point POINT_MUTE = new Point(LEFT_COL_INDENT + 180,343) ;
    protected static final Point POINT_VOL_UP = new Point(LEFT_COL_INDENT + 180,379) ;
    protected static final Point POINT_VOL_DOWN = new Point(LEFT_COL_INDENT + 180,415) ;
    protected static final Point POINT_XFER = new Point(LEFT_COL_INDENT + 236,307) ;
    protected static final Point POINT_CONF = new Point(LEFT_COL_INDENT + 236,343) ;
    protected static final Point POINT_HOLD = new Point(LEFT_COL_INDENT + 236,379) ;
    protected static final Point POINT_SPKR = new Point(LEFT_COL_INDENT + 305,304) ;

    protected static final Point POINT_SCROLL_UP =   new Point(LEFT_COL_INDENT + 377, 96) ;
    protected static final Point POINT_SCROLL_DOWN = new Point(LEFT_COL_INDENT + 377, 139) ;

    /** Create Dimensions for each of the softphone elements */
    protected static final Dimension DIM_SHELL = new Dimension(160,160);

    protected static final Dimension DIM_L1 = new Dimension(42,25);
    protected static final Dimension DIM_L2 = new Dimension(42,25);
    protected static final Dimension DIM_L3 = new Dimension(42,25);
    protected static final Dimension DIM_L4 = new Dimension(42,25);

    protected static final Dimension DIM_R1 = new Dimension(42,25);
    protected static final Dimension DIM_R2 = new Dimension(42,25);
    protected static final Dimension DIM_R3 = new Dimension(42,25);
    protected static final Dimension DIM_R4 = new Dimension(42,25);

    protected static final Dimension DIM_B1 = new Dimension(42,25);
    protected static final Dimension DIM_B2 = new Dimension(42,25);
    protected static final Dimension DIM_B3 = new Dimension(42,25);

    protected static final Dimension DIM_1 = new Dimension(42,34);
    protected static final Dimension DIM_2 = new Dimension(42,34);
    protected static final Dimension DIM_3 = new Dimension(42,34);
    protected static final Dimension DIM_4 = new Dimension(42,34);
    protected static final Dimension DIM_5 = new Dimension(42,34);
    protected static final Dimension DIM_6 = new Dimension(42,34);
    protected static final Dimension DIM_7 = new Dimension(42,34);
    protected static final Dimension DIM_8 = new Dimension(42,34);
    protected static final Dimension DIM_9 = new Dimension(42,34);
    protected static final Dimension DIM_0 = new Dimension(42,34);
    protected static final Dimension DIM_POUND  = new Dimension(42,34);
    protected static final Dimension DIM_STAR   = new Dimension(42,34);

    protected static final Dimension DIM_MORE   = new Dimension(47,36);
    protected static final Dimension DIM_HDSET  = new Dimension(52,34);
    protected static final Dimension DIM_MUTE   = new Dimension(52,37);
    protected static final Dimension DIM_VOL_UP = new Dimension(52,38);
    protected static final Dimension DIM_VOL_DOWN = new Dimension(52,37);
    protected static final Dimension DIM_XFER   = new Dimension(53,34);
    protected static final Dimension DIM_CONF   = new Dimension(53,37);
    protected static final Dimension DIM_HOLD   = new Dimension(53,38);
    protected static final Dimension DIM_SPKR   = new Dimension(47,71);

    protected static final Dimension DIM_SCROLL_UP = new Dimension(39,39);
    protected static final Dimension DIM_SCROLL_DOWN = new Dimension(39,39);

    /** ActionCommands that each of the buttons generate */
    protected static final String COMMAND_L1 = "L1" ;
    protected static final String COMMAND_L2 = "L2" ;
    protected static final String COMMAND_L3 = "L3" ;
    protected static final String COMMAND_L4 = "L4" ;
    protected static final String COMMAND_R1 = "R1" ;
    protected static final String COMMAND_R2 = "R2" ;
    protected static final String COMMAND_R3 = "R3" ;
    protected static final String COMMAND_R4 = "R4" ;
    protected static final String COMMAND_B1 = "B1" ;
    protected static final String COMMAND_B2 = "B2" ;
    protected static final String COMMAND_B3 = "B3" ;

    protected static final String COMMAND_1 = "1" ;
    protected static final String COMMAND_2 = "2" ;
    protected static final String COMMAND_3 = "3" ;
    protected static final String COMMAND_4 = "4" ;
    protected static final String COMMAND_5 = "5" ;
    protected static final String COMMAND_6 = "6" ;
    protected static final String COMMAND_7 = "7" ;
    protected static final String COMMAND_8 = "8" ;
    protected static final String COMMAND_9 = "9" ;
    protected static final String COMMAND_0 = "0" ;
    protected static final String COMMAND_POUND = "POUND" ;
    protected static final String COMMAND_STAR = "STAR" ;

    protected static final String COMMAND_MORE      = "MORE" ;
    protected static final String COMMAND_HDSET     = "HDSET" ;
    protected static final String COMMAND_MUTE      = "MUTE" ;
    protected static final String COMMAND_VOL_UP    = "VOL_UP" ;
    protected static final String COMMAND_VOL_DOWN  = "VOL_DOWN" ;
    protected static final String COMMAND_XFER      = "XFER" ;
    protected static final String COMMAND_CONF      = "CONF" ;
    protected static final String COMMAND_HOLD      = "HOLD" ;
    protected static final String COMMAND_SPKR      = "SPKR" ;

    protected static final String COMMAND_SCROLL_UP = "SCROLL_UP" ;
    protected static final String COMMAND_SCROLL_DOWN = "SCROLL_DOWN" ;
    protected static final String COMMAND_MSGLIGHT = "MSGLIGHT" ;

   // here are the value for the lamps...THESE MUST MATCH THE JNI Code!!!!!!
    protected static final long LAMP_HEADSET   = 0x01;
    protected static final long LAMP_HOLD      = 0x02;
    protected static final long LAMP_MESSAGES  = 0x04;
    protected static final long LAMP_MUTE      = 0x08;
    protected static final long LAMP_SPEAKER   = 0x10;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Reference to the softphone screen
    protected     Panel   m_pnlShell = (Panel) ShellApp.getInstance().getDisplayPanel() ;

    /** reference to our display image */
    protected Image m_backImage = null;

    /** reference to MediaTracker */
    protected MediaTracker m_mediaTracker = null ;
    protected int m_iMediaCount = 0 ;

    PropertyResourceBundle m_propsSkinDefinition ;
    String                 m_strSkinFolder ;

    protected PingtelImageButton m_btnL1, m_btnL2, m_btnL3, m_btnL4 ;
    protected PingtelImageButton m_btnR1, m_btnR2, m_btnR3, m_btnR4 ;
    protected PingtelImageButton m_btnB1, m_btnB2, m_btnB3 ;

    protected PingtelImageButton m_btn1, m_btn2, m_btn3, m_btn4, m_btn5 ;
    protected PingtelImageButton m_btn6, m_btn7, m_btn8, m_btn9, m_btn0 ;
    protected PingtelImageButton m_btnStar, m_btnPound ;

    protected PingtelImageButton m_btnMore, m_btnHdset, m_btnMute, m_btnVolUp ;
    protected PingtelImageButton m_btnVolDown, m_btnXfer, m_btnConf, m_btnHold ;
    protected PingtelImageButton m_btnSpkr ;
    protected PingtelImageButton m_btnScrollUp, m_btnScrollDown ;
    protected PingtelImageButton m_btnMsgLight;

   protected static Hashtable m_htButtonLamps ;
   protected static TestbedFrame m_Parent;

//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    public SoftPhone(TestbedFrame parent)
    {
        //save off who the parent is
        m_Parent = parent;

        initializeSkinResources() ;

        createComponents() ;
        layoutComponents() ;

        // wait for all images
        try {
            m_mediaTracker.waitForAll() ;
        }
        catch (Exception e)
        {
            System.out.println("Media Tracker failed") ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public int getStartX()
    {
        String strStartX = m_propsSkinDefinition.getString("AppStartX");
        return Integer.parseInt(strStartX) ;
    }

    public int getStartY()
    {
        String strStartY = m_propsSkinDefinition.getString("AppStartY");
        return Integer.parseInt(strStartY) ;
    }

    public int getAppWidth()
    {
        String strWidth  = m_propsSkinDefinition.getString("AppWidth");
        return Integer.parseInt(strWidth) ;
    }

    public int getAppHeight()
    {
        String strHeight = m_propsSkinDefinition.getString("AppHeight");
        return Integer.parseInt(strHeight) ;
    }

    /**
     * Method to return the numeric ID of the button, when its string is provided
     *
     * @param strButton  String of button for which want ID
     */
    public int getButtonId (String strButton)
    {
        if (strButton.equalsIgnoreCase(COMMAND_L1)) return PButtonEvent.BID_L1 ;
        else if (strButton.equalsIgnoreCase(COMMAND_L2)) return PButtonEvent.BID_L2 ;
        else if (strButton.equalsIgnoreCase(COMMAND_L3)) return PButtonEvent.BID_L3 ;
        else if (strButton.equalsIgnoreCase(COMMAND_L4)) return PButtonEvent.BID_L4 ;
        else if (strButton.equalsIgnoreCase(COMMAND_R1)) return PButtonEvent.BID_R1 ;
        else if (strButton.equalsIgnoreCase(COMMAND_R2)) return PButtonEvent.BID_R2 ;
        else if (strButton.equalsIgnoreCase(COMMAND_R3)) return PButtonEvent.BID_R3 ;
        else if (strButton.equalsIgnoreCase(COMMAND_R4)) return PButtonEvent.BID_R4 ;
        else if (strButton.equalsIgnoreCase(COMMAND_B1)) return PButtonEvent.BID_B1 ;
        else if (strButton.equalsIgnoreCase(COMMAND_B2)) return PButtonEvent.BID_B2 ;
        else if (strButton.equalsIgnoreCase(COMMAND_B3)) return PButtonEvent.BID_B3 ;

        else if (strButton.equalsIgnoreCase(COMMAND_1)) return PButtonEvent.BID_1 ;
        else if (strButton.equalsIgnoreCase(COMMAND_2)) return PButtonEvent.BID_2 ;
        else if (strButton.equalsIgnoreCase(COMMAND_3)) return PButtonEvent.BID_3 ;
        else if (strButton.equalsIgnoreCase(COMMAND_4)) return PButtonEvent.BID_4 ;
        else if (strButton.equalsIgnoreCase(COMMAND_5)) return PButtonEvent.BID_5 ;
        else if (strButton.equalsIgnoreCase(COMMAND_6)) return PButtonEvent.BID_6 ;
        else if (strButton.equalsIgnoreCase(COMMAND_7)) return PButtonEvent.BID_7 ;
        else if (strButton.equalsIgnoreCase(COMMAND_8)) return PButtonEvent.BID_8 ;
        else if (strButton.equalsIgnoreCase(COMMAND_9)) return PButtonEvent.BID_9 ;
        else if (strButton.equalsIgnoreCase(COMMAND_0)) return PButtonEvent.BID_0 ;
        else if (strButton.equalsIgnoreCase(COMMAND_POUND)) return PButtonEvent.BID_POUND ;
        else if (strButton.equalsIgnoreCase(COMMAND_STAR)) return PButtonEvent.BID_STAR ;

        else if (strButton.equalsIgnoreCase(COMMAND_MORE)) return PButtonEvent.BID_PINGTEL ;
        else if (strButton.equalsIgnoreCase(COMMAND_HDSET)) return PButtonEvent.BID_HEADSET;
        else if (strButton.equalsIgnoreCase(COMMAND_MUTE)) return PButtonEvent.BID_MUTE ;
        else if (strButton.equalsIgnoreCase(COMMAND_VOL_UP)) return PButtonEvent.BID_VOLUME_UP ;
        else if (strButton.equalsIgnoreCase(COMMAND_VOL_DOWN)) return PButtonEvent.BID_VOLUME_DN ;
        else if (strButton.equalsIgnoreCase(COMMAND_XFER)) return PButtonEvent.BID_TRANSFER ;
        else if (strButton.equalsIgnoreCase(COMMAND_CONF)) return PButtonEvent.BID_CONFERENCE ;
        else if (strButton.equalsIgnoreCase(COMMAND_HOLD)) return PButtonEvent.BID_HOLD ;
        else if (strButton.equalsIgnoreCase(COMMAND_SPKR)) return PButtonEvent.BID_SPEAKER ;

        else if (strButton.equalsIgnoreCase(COMMAND_SCROLL_UP)) return PButtonEvent.BID_SCROLL_UP ;
        else if (strButton.equalsIgnoreCase(COMMAND_SCROLL_DOWN)) return PButtonEvent.BID_SCROLL_DOWN ;

        return -1;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * Create the components for the softphone
     */
    protected void createComponents()
    {
        String imgName;

        imgName = m_strSkinFolder + m_propsSkinDefinition.getString("imgBackground");

        m_backImage = Toolkit.getDefaultToolkit().getImage(imgName);

        m_mediaTracker = new MediaTracker(this) ;
        m_mediaTracker.addImage(m_backImage,0);

        // wait for all images
        try
        {
            m_mediaTracker.waitForAll() ;
        }
        catch (Exception e)
        {
            System.out.println("Media Tracker failed") ;
        }

        setBackgroundImage(m_backImage);

        createButtons() ;
    }

    /**
     * Layout the components for the softphone
     */
    protected void layoutComponents()
    {
        String strStartX = m_propsSkinDefinition.getString("PhoneGuiStartX");
        String strStartY = m_propsSkinDefinition.getString("PhoneGuiStartY");
        int iStartX = Integer.parseInt(strStartX);
        int iStartY = Integer.parseInt(strStartY);
        int iWidth = DIM_SHELL.width;
        int iHeight = DIM_SHELL.height;

        m_pnlShell.setBounds(iStartX, iStartY, iWidth, iHeight);

        // set screen background color
        float fColor = 206/256 ;
        Color colGray = new Color(255,255,255) ;
        m_pnlShell.setBackground(colGray) ;

        // Add components to softphone
        addComponents() ;
    }

    /**
     * Initialize skin resources
     */
    protected void initializeSkinResources()
    {
        Properties prop = System.getProperties() ;
        String strSkinDefFile ;

        m_strSkinFolder = prop.getProperty("softphone.skin.path") ;

        // Append trailing slash if a path is specified and doesn't have one
        if ((m_strSkinFolder != null) && (m_strSkinFolder.length() > 0))
        {
            if (!m_strSkinFolder.endsWith(File.separator))
                m_strSkinFolder += File.separator ;
        }
        else
        {
            m_strSkinFolder = "" ;
        }

        strSkinDefFile = m_strSkinFolder + "_skinlayout.properties" ;

        try
        {
            FileInputStream fis = new FileInputStream(strSkinDefFile) ;
            m_propsSkinDefinition = new PropertyResourceBundle(fis) ;
        }
        catch (Exception e)
        {
            System.out.println("Unable to load skin definition file: " + strSkinDefFile) ;
            TestbedFrame.getInstance().shutdown(-1) ;
        }
    }

    /**
     * Method that creates creates button down and button up events when user clicks softphone GUI
     *
     * @param int Button Id, as defined in PButtonEvent
     */

    protected void createButtonEvent(int iButtonId)
    {
        TestbedButtonEvent btnEventDown = new TestbedButtonEvent(iButtonId, PButtonEvent.BUTTON_DOWN);

        try {
            Thread.currentThread().sleep(100);
        } catch (Exception e) {
            System.out.println("Softphone: error sleeping") ;
        }

        TestbedButtonEvent btnEventUp   = new TestbedButtonEvent(iButtonId, PButtonEvent.BUTTON_UP);
    }


    /**
     * Method that creates the buttons for the softphone
     */
    protected void createButtons()
    {
        m_htButtonLamps = new Hashtable() ;

        m_btnMsgLight = createButton2("imgMsg",COMMAND_MSGLIGHT);
        m_htButtonLamps.put(new Long(LAMP_MESSAGES), m_btnMsgLight) ;

        m_btnL1 = createButton2("imgL1",COMMAND_L1);
//        m_btnL1 = createButton("imgL1","imgL1Press",COMMAND_L1,DIM_L1);
        m_btnL2 = createButton2("imgL2",COMMAND_L2);
        m_btnL3 = createButton2("imgL3",COMMAND_L3);
        m_btnL4 = createButton2("imgL4",COMMAND_L4);

        m_btnR1 = createButton2("imgR1",COMMAND_R1);
        m_btnR2 = createButton2("imgR2",COMMAND_R2);
        m_btnR3 = createButton2("imgR3",COMMAND_R3);
        m_btnR4 = createButton2("imgR4",COMMAND_R4);

        m_btnB1 = createButton2("imgB1",COMMAND_B1);
        m_btnB2 = createButton2("imgB2",COMMAND_B2);
        m_btnB3 = createButton2("imgB3",COMMAND_B3);

        m_btn0  = createButton2("imgZero",COMMAND_0);
        m_btn1  = createButton2("imgOne",COMMAND_1);
        m_btn2  = createButton2("imgTwo",COMMAND_2);
        m_btn3  = createButton2("imgThree",COMMAND_3);
        m_btn4  = createButton2("imgFour",COMMAND_4);
        m_btn5  = createButton2("imgFive",COMMAND_5);
        m_btn6  = createButton2("imgSix",COMMAND_6);
        m_btn7  = createButton2("imgSeven",COMMAND_7);
        m_btn8  = createButton2("imgEight",COMMAND_8);
        m_btn9  = createButton2("imgNine",COMMAND_9);

        m_btnStar = createButton2("imgStar",COMMAND_STAR);
        m_btnPound = createButton2("imgPound",COMMAND_POUND);
        m_btnMore = createButton2("imgMore",COMMAND_MORE);
        m_btnHdset = createButton2("imgHdset",COMMAND_HDSET);
        m_htButtonLamps.put(new Long(LAMP_HEADSET), m_btnHdset) ;
        m_btnMute = createButton2("imgMute",COMMAND_MUTE);
        m_htButtonLamps.put(new Long(LAMP_MUTE), m_btnMute) ;

        m_btnVolUp = createButton2("imgVolUp",COMMAND_VOL_UP);
        m_btnVolDown = createButton2("imgVolDown",COMMAND_VOL_DOWN);

        m_btnXfer = createButton2("imgXfer",COMMAND_XFER);
        m_btnConf = createButton2("imgConf",COMMAND_CONF);
        m_btnHold = createButton2("imgHold",COMMAND_HOLD);
        m_htButtonLamps.put(new Long(LAMP_HOLD), m_btnHold) ;

        m_btnSpkr = createButton2("imgSpkr",COMMAND_SPKR);
        m_htButtonLamps.put(new Long(LAMP_SPEAKER), m_btnSpkr) ;

        m_btnScrollUp = createButton2("imgScrollup",COMMAND_SCROLL_UP  );
        m_btnScrollDown = createButton2("imgScrolldown",COMMAND_SCROLL_DOWN  );
    }


    protected PingtelImageButton  createButton2(String strImageName, String btnName)
    {

        PingtelImageButton btn = new PingtelImageButton(m_propsSkinDefinition, m_strSkinFolder,strImageName,btnName);
        Image img;

        img = btn.getDefaultImage();
        if (img != null)
            m_mediaTracker.addImage(img, m_iMediaCount++) ;

        img = btn.getPressedImage();
        if (img != null)
            m_mediaTracker.addImage(img, m_iMediaCount++) ;

        img = btn.getOverImage();
        if (img != null)
            m_mediaTracker.addImage(img, m_iMediaCount++) ;

        img = btn.getLitImage();
        if (img != null)
            m_mediaTracker.addImage(img, m_iMediaCount++) ;

        img = btn.getLitPressedImage();
        if (img != null)
            m_mediaTracker.addImage(img, m_iMediaCount++) ;
        //to draw the label, we ask the button for it's label, and x and y
        //then we add a PLabel to the component
        img = btn.getLabelImage();
        if (img != null)
        {
            m_mediaTracker.addImage(img, m_iMediaCount++) ;
            PLabel label = new PLabel(img);
            add(label);

            label.setBounds(btn.getLabelStartX(),btn.getLabelStartY(),
                    img.getWidth(null),img.getHeight(null));
        }

        return btn;
    }


     /**
     * Add all the buttons and screen to the softphone
     *
     */
    protected void addComponents()
    {
        add(m_btnMsgLight) ;
        add(m_pnlShell);
        add(m_btnL1) ;
        add(m_btnL2) ;
        add(m_btnL3) ;
        add(m_btnL4) ;
        add(m_btnR1) ;
        add(m_btnR2) ;
        add(m_btnR3) ;
        add(m_btnR4) ;
        add(m_btnB1) ;
        add(m_btnB2) ;
        add(m_btnB3) ;

        add(m_btn1) ;
        add(m_btn2) ;
        add(m_btn3) ;
        add(m_btn4) ;
        add(m_btn5) ;
        add(m_btn6) ;
        add(m_btn7) ;
        add(m_btn8) ;
        add(m_btn9) ;
        add(m_btn0) ;

        add(m_btnStar) ;
        add(m_btnPound) ;

        add(m_btnMore) ;
        add(m_btnHdset) ;
        add(m_btnMute) ;
        add(m_btnVolUp) ;
        add(m_btnVolDown) ;
        add(m_btnXfer) ;
        add(m_btnConf) ;
        add(m_btnHold) ;
        add(m_btnSpkr) ;
        add(m_btnScrollUp) ;
        add(m_btnScrollDown) ;
    }


    //LAMP_HEADSET   = 0x01;
    //LAMP_HOLD      = 0x02;
    //LAMP_MESSAGES  = 0x04;
    //LAMP_MUTE      = 0x08;
    //LAMP_SPEAKER   = 0x10;
    static void LightButton(long buttonIDs)
    {
        long mask = 1;
        boolean bState;

        // Lookup the buttons which have changed and call them
        PingtelImageButton button;
        do
        {
            if ((buttonIDs & mask) == mask)
                bState = true;
            else
                bState = false;

            button = (PingtelImageButton) m_htButtonLamps.get(new Long(mask)) ;
            if (button != null)
            {
                // Poke it based on state.
                button.lightButton(bState);
            }

            mask = (mask << 1);
        } while (mask < 0x01000000); // last button is bit 6
    }

    static void SoftPhoneReboot(long ms)
    {
        try
        {
            if (ms > 0)
            {
                System.out.println("sleeping: " + ms);
                Thread.sleep(1000); //should sleep for ms but JNI wasn't working.
                //i'll need to talk to bob about this
            }
            System.exit(2) ;
        }
        catch(Exception e)
        {
            System.out.println(e);
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     * listen for button presses
     */
    protected class icSoftButtonActionListener implements ActionListener
    {

        public void actionPerformed(ActionEvent e)
        {
             // Determine action command
             int iButtonId = getButtonId(e.getActionCommand());

             // Action depending on command
             switch (iButtonId){
                case PButtonEvent.BID_L1: createButtonEvent(PButtonEvent.BID_L1) ; break ;
                case PButtonEvent.BID_L2: createButtonEvent(PButtonEvent.BID_L2) ; break ;
                case PButtonEvent.BID_L3: createButtonEvent(PButtonEvent.BID_L3) ; break ;
                case PButtonEvent.BID_L4: createButtonEvent(PButtonEvent.BID_L4) ; break ;
                case PButtonEvent.BID_R1: createButtonEvent(PButtonEvent.BID_R1) ; break ;
                case PButtonEvent.BID_R2: createButtonEvent(PButtonEvent.BID_R2) ; break ;
                case PButtonEvent.BID_R3: createButtonEvent(PButtonEvent.BID_R3) ; break ;
                case PButtonEvent.BID_R4: createButtonEvent(PButtonEvent.BID_R4) ; break ;
                case PButtonEvent.BID_B1: createButtonEvent(PButtonEvent.BID_B1) ; break ;
                case PButtonEvent.BID_B2: createButtonEvent(PButtonEvent.BID_B2) ; break ;
                case PButtonEvent.BID_B3: createButtonEvent(PButtonEvent.BID_B3) ; break ;

                default: System.out.println("Softphone: Can't find button event type") ; break ;
            }
        }
    }

    protected class icKeyPadActionListener implements ActionListener
    {

        public void actionPerformed(ActionEvent e)
        {
             // Determine action command
             int iButtonId = getButtonId(e.getActionCommand());

             // Action depending on command
             switch (iButtonId){
                case PButtonEvent.BID_1: createButtonEvent(PButtonEvent.BID_1) ; break ;
                case PButtonEvent.BID_2: createButtonEvent(PButtonEvent.BID_2) ; break ;
                case PButtonEvent.BID_3: createButtonEvent(PButtonEvent.BID_3) ; break ;
                case PButtonEvent.BID_4: createButtonEvent(PButtonEvent.BID_4) ; break ;
                case PButtonEvent.BID_5: createButtonEvent(PButtonEvent.BID_5) ; break ;
                case PButtonEvent.BID_6: createButtonEvent(PButtonEvent.BID_6) ; break ;
                case PButtonEvent.BID_7: createButtonEvent(PButtonEvent.BID_7) ; break ;
                case PButtonEvent.BID_8: createButtonEvent(PButtonEvent.BID_8) ; break ;
                case PButtonEvent.BID_9: createButtonEvent(PButtonEvent.BID_9) ; break ;
                case PButtonEvent.BID_0: createButtonEvent(PButtonEvent.BID_0) ; break ;
                case PButtonEvent.BID_POUND: createButtonEvent(PButtonEvent.BID_POUND) ; break ;
                case PButtonEvent.BID_STAR: createButtonEvent(PButtonEvent.BID_STAR) ; break ;

                default: System.out.println("Softphone: Can't find button event type") ; break ;
            }
        }
    }

    protected class icHardButtonActionListener implements ActionListener
    {

        public void actionPerformed(ActionEvent e)
        {
             // Determine action command
             int iButtonId = getButtonId(e.getActionCommand());

             // Action depending on command
             switch (iButtonId){
                case PButtonEvent.BID_PINGTEL: createButtonEvent(PButtonEvent.BID_PINGTEL) ; break ;
                case PButtonEvent.BID_REDIAL: createButtonEvent(PButtonEvent.BID_REDIAL) ; break ;
                case PButtonEvent.BID_CONFERENCE: createButtonEvent(PButtonEvent.BID_CONFERENCE) ; break ;
                case PButtonEvent.BID_TRANSFER: createButtonEvent(PButtonEvent.BID_TRANSFER) ; break ;
                case PButtonEvent.BID_HOLD: createButtonEvent(PButtonEvent.BID_HOLD) ; break ;
                case PButtonEvent.BID_SPEAKER: createButtonEvent(PButtonEvent.BID_SPEAKER) ; break ;
                case PButtonEvent.BID_MUTE: createButtonEvent(PButtonEvent.BID_MUTE) ; break ;
                case PButtonEvent.BID_VOLUME_UP: createButtonEvent(PButtonEvent.BID_VOLUME_UP) ; break ;
                case PButtonEvent.BID_VOLUME_DN: createButtonEvent(PButtonEvent.BID_VOLUME_DN) ; break ;
                case PButtonEvent.BID_SCROLL_UP: createButtonEvent(PButtonEvent.BID_SCROLL_UP) ; break ;
                case PButtonEvent.BID_SCROLL_DOWN: createButtonEvent(PButtonEvent.BID_SCROLL_DOWN) ; break ;

                default: System.out.println("Softphone: Can't find button event type") ; break ;
             }
        }
    }
}


