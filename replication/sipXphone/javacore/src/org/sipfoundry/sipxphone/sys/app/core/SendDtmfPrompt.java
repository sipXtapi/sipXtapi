/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/SendDtmfPrompt.java#2 $
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

import org.sipfoundry.sipxphone.awt.*;
import java.awt.Container;
import org.sipfoundry.sipxphone.sys.*;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.awt.form.*;
import org.sipfoundry.sipxphone.*;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.app.shell.*;

/**
 * Mix between more GUI options of MessageBox and event listener operation of StatusFrame
 * most usable when in modeless form
 *
 * @author Douglas Hubler
 * @version 1.0
 */
public class SendDtmfPrompt extends PContainer implements PFormButtonHook
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private boolean m_isBlocked;

    private boolean m_isOpen;

    private Object m_block = new Object();

    private PLabel m_msg;

    private PAbstractForm m_parent;

    private boolean m_wasOkPressed;

    private PingtelEvent m_showFrame;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SendDtmfPrompt(PAbstractForm parent)
    {
        super();
        m_parent = parent;

        setBackgroundImage(getImage("imgTooltipFrame"));

        m_msg = new PLabel() ;
        //m_msg.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL));
        m_msg.setBounds(8, 8, 145, 36) ;
        add(m_msg) ;

        // TODO: Experiment w/BottonButtonBar again, first attempt failed to draw
        // but getting this to work will give us desired UI hints. - DLH
        PLabel buttomLeftLabel = new PLabel("") ;
        buttomLeftLabel.setBounds(0, 35, 50, 24) ;
        add(buttomLeftLabel) ;

        PLabel buttomCenterLabel = new PLabel("Cancel") ;
        buttomCenterLabel.setBounds(50, 35, 60, 24) ;
        add(buttomCenterLabel) ;

        PLabel buttomRightLabel = new PLabel("Send") ;
        buttomRightLabel.setBounds(110, 35, 50, 24) ;
        add(buttomRightLabel) ;

        addButtonListener(new icUiController());
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * After user OKs it, these dtmf digits will be sent 
     */
    public void setUpcomingDtmf(String digits)
    {
        String msg;
        if (digits == null || digits.length() == 0)
            msg = "Press Send to continue";
        else
            msg = "Send " + digits + "?";
        m_msg.setText(msg);
    }

    /**
     * Redirected button event from callinprogress form that's
     * behind this half frame.  Only capture
     */
    public boolean buttonEvent(PButtonEvent event, PForm form)
    {
        boolean consumed = false;

        switch (event.getButtonID())
        {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
            case PButtonEvent.BID_POUND:
            case PButtonEvent.BID_STAR:
                // emulate a true modeless dialog by not consuming events from parent
                // form. Safest to only pass keypad event. e.g. user should be able to start
                // transfer now
                consumed = false;
                break;

            default:
                // keep all other buttons to this UI
                processPingtelEvent(event);
                consumed = true;
                break;
        }

        return consumed;
    }

    /**
     * Somewhere between modal and modeless, this blocks the caller but not the GUI "behind"
     * this form
     */
    public boolean showModal()
    {
        // open ui
        m_parent.addFormButtonHook(this, false);
        PingtelEventQueue.postEvent(new icEventQueueSync(true));

        m_isBlocked = true;
        synchronized (m_block)
        {
            try
            {
                if (m_isBlocked)
                {
                    m_block.wait();
                }
            }
            catch (InterruptedException ignore)
            {
                ignore.printStackTrace();
            }
        }

        // remove ui and this from parent
        closeUi();

        return m_wasOkPressed;
    }

    /**
     * close ui
     */
    public void close()
    {
        if (m_isBlocked)
            unblock();
        if (m_isOpen)
            closeUi();
    }

    /**
     *  wake up thread in showModel call 
     */
    public void unblock()
    {
        // unblock
        m_isBlocked = false;
        synchronized (m_block)
        {
            m_block.notifyAll();
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * "close" ui by removing panel on the event queue thread
     */
    private void closeUi()
    {
        m_parent.removeFormButtonHook(this);
        PingtelEventQueue.postEvent(new icEventQueueSync(false));
    }
        
//////////////////////////////////////////////////////////////////////////////
// Inner/Nested Classes
////
    /**
     * Sync with event queue becase frame is modal and hence syncrnous with thread that
     * called showModal
     */
    private class icEventQueueSync extends PingtelEvent implements Runnable
    {
        private boolean m_show;

        icEventQueueSync(boolean show)
        {
            super(SendDtmfPrompt.this, 0);
            m_show = show;
        }

        public void run()
        {
            Container display = ShellApp.getInstance().getDisplayPanel();
            setBounds(0, 96, 160, 64); // also triggers invalidate()

            if (m_show)
            {
                display.add(SendDtmfPrompt.this, 0);
                m_isOpen = true;
                repaint();
            }
            else
            {
                display.remove(SendDtmfPrompt.this);
                m_isOpen = false;
                m_parent.repaint();
            }

        }
    }

    private class icUiController implements PButtonListener
    {
        public void buttonDown(PButtonEvent e)
        {
        }

        public void buttonUp(PButtonEvent event)
        {
            if (event.getButtonID() == PButtonEvent.BID_B2)
            {
                unblock();
                event.consume();
            }
            else if (event.getButtonID() == PButtonEvent.BID_B3)
            {
                m_wasOkPressed = true;
                unblock();
                event.consume();
            }
        }

        public void buttonRepeat(PButtonEvent e)
        {
        }
    }
}

