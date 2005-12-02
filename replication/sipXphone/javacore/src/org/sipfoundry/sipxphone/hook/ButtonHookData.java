/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/hook/ButtonHookData.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.hook ;

import org.sipfoundry.sipxphone.awt.event.PButtonEvent ;
/**
 * ButtonHookData.java
 *
 * Data object for a Button hook.  A Button hook allows developers to
 * substitute different actions for button events.
 * <p>
 *
 * @see org.sipfoundry.sipxphone.hook.Hook
 * @see org.sipfoundry.sipxphone.sys.HookManager
 * @see org.sipfoundry.sipxphone.hook.HookData
 *
 * Created: Tue Apr 02 16:05:01 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */
public class ButtonHookData extends HookData{


    private PButtonEvent m_event;

    /**
     * Constructs a new ButtonHookData object.
     *
     * @param event PButtonEvent received from the button.
     */
    public ButtonHookData(PButtonEvent event){
            m_event = event;
    }

    /**
     * gets the PButtonEvent associated with this data.
     * @return PButtonEvent associated with this data.
     */
    public PButtonEvent getEvent(){
        return m_event;
    }


}// ButtonHookData
