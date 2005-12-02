/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/speeddial/PhoneNumberOptions.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.app.speeddial;

import java.awt.*;
import org.sipfoundry.sipxphone.*;
import org.sipfoundry.util.PhoneNumberParser;
import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.*;

/**
 * Special codes that can be inserted into a phone number string
 * typcially modal form. Used in SpeedDial app right now
 *
 * @author Douglas Hubler
 */
public class PhoneNumberOptions extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** list of special characters */
    private PList m_specialChars;

    /** special code to insert that was selected (e.g. "p", "i") */
    private String m_selected;

    /** action command dispatcher */
    private icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * UI to select special digits like pause and wait characters that can be
     * inserted into a phone number
     */
    public PhoneNumberOptions(Application application)
    {
        super(application, "Phone Number Options");
        setStringResourcesFile("SpeeddialAddForm.properties");

        setIcon(getImage("imgPauseWait"));
        setTitle(getString("lblPhoneOptionsTitle"));

        // build menu
        PActionItem items[] = new PActionItem[2];
        items[0] = new PActionItem(new PLabel(getString("lblGenericOk")),
            getString("lblPhoneOptionsOk"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_OK);

        items[1] = new PActionItem(new PLabel(getString("lblGenericCancel")),
            getString("lblPhoneOptionsCancel"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL);
        setLeftMenu(items);

        PBottomButtonBar buttons = getBottomButtonBar() ;
        buttons.setItem(PBottomButtonBar.B2, items[1]);
        buttons.setItem(PBottomButtonBar.B3, items[0]);

        // build list
        PDefaultListModel model = new PDefaultListModel();
        model.addElement(new ListItem(getString("lblPauseOption"),
            String.valueOf(PhoneNumberParser.CHAR_PAUSE)));
        model.addElement(new ListItem(getString("lblWaitOption"),
            String.valueOf(PhoneNumberParser.CHAR_WAIT)));
        m_specialChars = new PList(model);
        addToDisplayPanel(m_specialChars, new Insets(0, 0, 0, 0));

        setHelpText(getString("edit_speed_dial_value"), getString("edit_speed_dial_value_title"));
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Special code to insert into phone number that was selected by user
     */
    public String getSelectedItem()
    {
        return m_selected;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    protected void onOk()
    {
        m_selected = ((ListItem)m_specialChars.getSelectedElement()).data;
        closeForm(1);
    }

    protected void onCancel()
    {
        m_selected = null;
        closeForm(0);
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////
    /**
     * toString()-able  for PListModel items
     */
    class ListItem
    {
        String label;
        String data;
        public ListItem(String label, String data)
        {
            this.label = label;
            this.data = data;
        }

        public String toString() {
            return label;
        }
    }

    /**
     * UI Controller
     */
    private class icCommandDispatcher implements PActionListener
    {
        /** action command: exit from the release calls form */
        public final String ACTION_CANCEL = "action_cancel" ;
        /** action command: release a call from hold */
        public final String ACTION_OK = "action_ok" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_OK)) {
                onOk() ;
            } else  if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                onOk() ;
            }
        }
    }
}







