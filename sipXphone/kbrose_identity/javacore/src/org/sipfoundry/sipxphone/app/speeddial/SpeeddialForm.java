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

package org.sipfoundry.sipxphone.app.speeddial ;

import java.awt.Insets ;
import java.awt.Dimension;
import java.util.Enumeration ;
import java.util.Vector;

import javax.naming.directory.*;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.sipxphone.sys.directoryservice.provider.*;
import org.sipfoundry.sipxphone.app.CoreAboutbox ;

public class SpeeddialForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int OK = 0 ;
    public static final int CANCEL = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our text field */
    protected PTextField        m_textField ;

    /** our speed dial list */
    protected PList             m_listAddressBook ;

    /** our speed dial list model */
    protected PDefaultListModel m_listModel ;

    /** our command bar */
    protected PCommandBar        m_commandBar ;

    /** our filter string */
    protected String            m_strFilterKey = "" ;

    /** action command dispatcher */
    protected icCommandDispatcher m_commandDispatcher
        = new icCommandDispatcher() ;

    //listener to listen to data add/remove in list
    protected icListDataListener  m_listDataListener
        = new icListDataListener();

    public static final String STRING_RESOURCES = "SpeeddialForm.properties" ;

    protected String m_strAddress ;

    private SpeedDialDSP m_speedDialDSP =
        (SpeedDialDSP) DirectoryServiceProviderRegistry.getDirectoryServiceProvider("speed_dial");

    boolean m_bContextSpecificEnabled ;

    protected Application m_application = null;

    /** ID that was edited */
    private BasicAttributes m_editedData = null;

    /** tip to display if there are <= 2 items in the list */
    private PLabel m_labelTip ;

    /** boolean to denote if tip was already added or not*/
    private boolean m_bLabelTipAdded  = false;

    private String m_strResourceFile;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     */
    public SpeeddialForm(Application application, String[] argv)
    {
        super(application, "Speed Dial") ;

        if( argv != null && argv.length >=1 )
        {
            m_strResourceFile = argv[0];
        }
        if( m_strResourceFile != null )
        {
            setStringResourcesFile(m_strResourceFile) ;
            m_speedDialDSP.setDataResourcesFile( m_strResourceFile );
        }else
        {
            setStringResourcesFile(STRING_RESOURCES) ;
            m_speedDialDSP.setDataResourcesFile( null );

        }
        m_application = application;

        setTitle(getString("speeddial_title"));
        setIcon(getImage("imgSpeedDialIcon")) ;
        setHelpText(getString("speed_dial"), getString("speed_dial_title")) ;


        // Create everything else
        initSpeeddial() ;
        initMenubar() ;
        initCommandBar() ;
        initializeMenus() ;

        // physically lay them out
        layoutComponents() ;

        addButtonListener(new icButtonListener()) ;

        m_bContextSpecificEnabled = false ;

        enableMenusByAction
            (m_bContextSpecificEnabled, m_commandDispatcher.ACTION_DIAL) ;
        enableMenusByAction
            (m_bContextSpecificEnabled, m_commandDispatcher.ACTION_DELETE) ;
        enableMenusByAction
            (m_bContextSpecificEnabled, m_commandDispatcher.ACTION_EDIT) ;
        m_listDataListener.handleDrawingInfoMessage();
    }


    /**
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_strFilterKey = "" ;
        applyDialingStrategy() ;
    }


    public void onFormOpening()
    {
        updateFilter() ;
    }



    /**
     *
     */
    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_commandBar.setBounds(0, 0, 28, 108) ;
        container.add(m_commandBar) ;

        m_listAddressBook.setBounds(28, 2, 129, 104) ;
        container.add(m_listAddressBook) ;

        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }


    /**
     *
     */
    protected void initCommandBar()
    {
        m_commandBar = new PCommandBar() ;

        PActionItem actionBackspace =
            new PActionItem(new PLabel(getImage("imgBackspaceIcon")),
               getString("hint/speeddial/main/backspace"),
              m_commandDispatcher,
              m_commandDispatcher.ACTION_BACKSPACE) ;
        m_commandBar.addButton(actionBackspace) ;

        //if NOT readonly,  show the add,edit and delete buttons.
        if( ! SpeedDialDSP.getInstance().isDataSourceReadOnly() ){
            // Initialize Right menu
            PActionItem actionAdd =
                new PActionItem(new PLabel(getImage("imgAddIcon")),
                    getString("hint/speeddial/main/add"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_ADD);
            m_commandBar.addButton(actionAdd) ;

            PActionItem actionEdit =
                new PActionItem(new PLabel(getImage("imgEditIcon")),
                    getString("hint/speeddial/main/edit"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_EDIT);
            m_commandBar.addButton(actionEdit) ;

            PActionItem actionDelete =
                new PActionItem(new PLabel(getImage("imgDeleteIcon")),
                    getString("hint/speeddial/main/delete"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_DELETE);
            m_commandBar.addButton(actionDelete) ;
        }
    }


    /**
     *
     */
    protected void initMenubar()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionExit = new PActionItem(new PLabel(getString("Exit")),
            getString("hint/core/system/exitform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionExit) ;

        PActionItem actionDial = new PActionItem
            (new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
             getString(Shell.getInstance()
                       .getDialingStrategy().getActionHint()),
             m_commandDispatcher,
             m_commandDispatcher.ACTION_DIAL) ;
        menuControl.setItem(PBottomButtonBar.B3, actionDial) ;
    }


    /**
     * Initialize the menus that are displayed when a user hits the pingtel
     * key and selects the menu tab.
     */
    protected void initializeMenus()
    {
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;

        // Initialize Left Menu
        PActionItem actionDial = new PActionItem
            (new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
             getString(Shell.getInstance()
                       .getDialingStrategy().getActionHint()),
             m_commandDispatcher,
             m_commandDispatcher.ACTION_DIAL) ;
        leftMenu.addItem(actionDial) ;

        PActionItem actionExit = new PActionItem(new PLabel("Exit"),
                getString("hint/core/system/exitform"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_CANCEL) ;
        leftMenu.addItem(actionExit) ;

        PActionItem actionAbout =  new PActionItem(new PLabel("About"),
                getString("hint/core/system/aboutform"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_ABOUT) ;
        rightMenu.addItem(actionAbout) ;


        // Initialize Right menu
         //if NOT readonly , show the add,edit and delete buttons.
        if( ! SpeedDialDSP.getInstance().isDataSourceReadOnly() ){
            PActionItem actionAdd = new PActionItem(new PLabel("Add"),
                    getString("hint/speeddial/main/add"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_ADD);
            rightMenu.addItem(actionAdd) ;

            PActionItem actionEdit = new PActionItem(new PLabel("Edit"),
                    getString("hint/speeddial/main/edit"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_EDIT);
            rightMenu.addItem(actionEdit) ;

            PActionItem actionDelete = new PActionItem(new PLabel("Delete"),
                    getString("hint/speeddial/main/delete"),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_DELETE);
            rightMenu.addItem(actionDelete) ;
        }
    }


    /**
     *
     */
    protected void initSpeeddial()
    {
        // create speeddial display objects
        m_listAddressBook = new PList() ;
        m_listModel = new PDefaultListModel() ;
        m_textField = new PTextField();

        m_listModel.addListDataListener(m_listDataListener) ;

        // add speeddial entries to our list
        Vector vEntries = m_speedDialDSP.getEntries();
        if( vEntries != null )
        {
            Enumeration enum = vEntries.elements() ;
            while (enum.hasMoreElements())
                m_listModel.addElement(enum.nextElement()) ;
        }

        // Set up the list to display correctly
        m_listAddressBook.setListModel(m_listModel) ;
        m_listAddressBook.addActionListener(m_commandDispatcher) ;
        m_listAddressBook.setItemRenderer(new SpeeddialEntryRenderer()) ;

        updateFilter();
    }


    /**
     *
     */
    public String getFrameID()
    {
        return "Speed Dial" ;
    }


    /**
     *
     */
    protected void onDial()
    {
        int size = m_listAddressBook.getListModel().getSize();

        if (size != 0)
        {
            BasicAttributes data =
                (BasicAttributes) m_listAddressBook.getSelectedElement() ;
            if (data != null) {
                m_strAddress = m_speedDialDSP.getSpeedDialPhoneNumber(data);
                if (m_strAddress.length() == 0)
                {
                    // It must be a SIP URL
                    m_strAddress = m_speedDialDSP.getSpeedDialURL(data);
                    if (m_strAddress == null)
                        m_strAddress = new String();
                }
                else
                {
                    // If this looks like a phone number, then remove all the dashes, dots,
                    // whitespace, extensions, etc.
                    if (PhoneNumberScrubber.looksLikePhoneNumber(m_strAddress))
                        m_strAddress = PhoneNumberScrubber.scrub(m_strAddress) ;
                }
            } else {
                m_strAddress = new String();
            }

            if (m_strAddress != null && m_strAddress.length() > 0)
            {
                closeForm(OK) ;
            } else {
                MessageBox alert =
                    new MessageBox(getApplication(), MessageBox.TYPE_ERROR);

                alert.setMessage("The Speed Dial entry is blank. Please edit the entry and enter a number or SIP URL before dialing.");
                alert.showModal();
            }

        } else {
            MessageBox alert = new MessageBox(getApplication(), MessageBox.TYPE_ERROR);

            alert.setMessage("There is nothing to dial. Please press the More button, select the 'Menu' tab, then select the 'Add' entry to add a speed dial entry.");
            alert.showModal();
        }
    }


    public String getAddress()
    {
        return m_strAddress ;
    }




    /**
     * on cancel
     */
    protected void onCancel()
    {
        closeForm(CANCEL) ;
    }



    /**
     * on pressing backspace
     */
    public void onBackspace()
    {
        if (m_strFilterKey.length() > 0) {
            m_strFilterKey =
                m_strFilterKey.substring(0, m_strFilterKey.length()-1) ;
            updateFilter() ;
        }
    }


    /**
     * invoked when "Delete" is pressed in SpeedDial form
     */
    public void onDelete()
    {
        int size = m_listAddressBook.getListModel().getSize();

        if (size != 0)
        {

            BasicAttributes data =
                (BasicAttributes) m_listAddressBook.getSelectedElement() ;

            MessageBox alert =
                new MessageBox(getApplication(), MessageBox.TYPE_WARNING);

            String strId = m_speedDialDSP.getSpeedDialId(data);
            String strLabel = m_speedDialDSP.getLabel(data);
            String strURL = m_speedDialDSP.getSpeedDialURL(data);
            String strPhoneNumber = m_speedDialDSP.getSpeedDialPhoneNumber(data);

            alert.setMessage("Are you sure you want to delete entry " + strId + ", for " + strLabel + "?");
            // Bail out if they don't want to overwrite the entry with this number
            if (alert.showModal() != MessageBox.OK)
                return;

            try{
                m_speedDialDSP.deleteEntry(data);
            }catch( IllegalArgumentException e ){
                SysLog.log(e);
            }
            // Refresh the display
            m_strFilterKey = "" ;
            updateFilter();

            //getApplication().showMessageBox("Delete", "I'm sorry, but this feature is still under construction.") ;
        } else {
            MessageBox alert =
                new MessageBox(getApplication(), MessageBox.TYPE_ERROR);

            alert.setMessage("There is nothing to delete.");
            alert.showModal();
        }
    }

    /**
     * Prints out the value of the members encapsulated in attrs.
     */
    private void dumpEntry(BasicAttributes attrs)
    {
        String strId = m_speedDialDSP.getSpeedDialId(attrs);
        String strLabel = m_speedDialDSP.getLabel(attrs);
        String strURL = m_speedDialDSP.getSpeedDialURL(attrs);
        String strPhoneNumber = m_speedDialDSP.getSpeedDialPhoneNumber(attrs);

        System.out.println(">>>> dumpEntry: Id '" + strId + "' Label '" + strLabel + "' URL '" + strURL + "' PhoneNumber '" + strPhoneNumber + "'");
    }


    /**
     * This is invoked when user presses "Edit" on Speed Dial form
     */
    public void onEdit()
    {
        int size = m_listAddressBook.getListModel().getSize();

        if (size != 0)
        {
            BasicAttributes data =
                (BasicAttributes) m_listAddressBook.getSelectedElement();
            if( data != null ){
                m_editedData = data;
                onEdit( data,  "Edit Speed Dial",  false );
            }
        }
    }



    /**
     * This is invoked when user presses "Add" on SpeedDial form
     */
    public void onAdd()
    {
        onEdit( null,  "Add Speed Dial",  true);
    }



    /**
     * This method is called by both onAdd() and onEdit() methods.
     *
     * @param data The data to be used to fill the edit/add screen.
     *             null if nothing
     * @param strTitle The title of the form
     * @param bAdd boolean to denote if it is a ADD operation or
     *            EDIT operation. It is TRUE if it is ADD and FALSE
     *            if it is EDIT.
     */
    public void onEdit(BasicAttributes data, String strTitle ,  boolean bAdd)
    {

        SpeeddialAddForm editForm =
            new SpeeddialAddForm(m_application, strTitle, bAdd);
        editForm.setEntryIsURL(false);
        editForm.setEntryPhoneNumber(new String(""));
        editForm.setEntryURL(new String("sip:"));
        if( data != null ){
            editForm.setEntryId(m_speedDialDSP.getSpeedDialId(data));
            editForm.setEntryLabel(m_speedDialDSP.getLabel(data));

            String strURL = m_speedDialDSP.getSpeedDialURL(data);

            if (strURL != null && strURL.length() > 0){
                editForm.setEntryIsURL(true);
                editForm.setEntryPhoneNumber(new String(""));
                editForm.setEntryURL(strURL);
            } else {
                editForm.setEntryIsURL(false);
                editForm.setEntryPhoneNumber
                    (m_speedDialDSP.getSpeedDialPhoneNumber(data));
                editForm.setEntryURL(new String("sip:"));
            }
        }else{
            editForm.setEntryId(m_speedDialDSP.getNextAvailableID()+"");
        }
        editForm.setInitialFocus() ;


        BasicAttributes enteredAttributes = new BasicAttributes();
        if (editForm.showModal() == SpeeddialAddForm.OK){
            String strEntryID = editForm.getEntryId();
            String strEntryLabel = editForm.getEntryLabel();
            String strEntryPhoneNumber = editForm.getEntryPhoneNumber();
            String strURL = editForm.getEntryURL();
            enteredAttributes.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID, strEntryID);
            enteredAttributes.put
                (SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL, strEntryLabel);
            enteredAttributes.put
               (SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER, strEntryPhoneNumber);
            enteredAttributes.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL, strURL);
            boolean bAddEntry = true;
            BasicAttributes searchEntry = null;
            String strError =  m_speedDialDSP.validateEntry
                ( strEntryID, strEntryLabel,
                  strEntryPhoneNumber, strURL,
                  editForm.isPhoneNumber(), "\n"  );

            if( strError.length() > 0 ){
                MessageBox alert =
                  new MessageBox
                      (getApplication(), MessageBox.TYPE_INFORMATIONAL);

                alert.setMessage(strError);
                alert.showModal();
                //back to edit screen
                bAddEntry = false;
                onEdit( enteredAttributes, strTitle, bAdd );

            }else {
                boolean bCheckForDuplicate = true;
                boolean bDeleteOriginalEntry = false;

                String strEditedID = null;
                if( m_editedData != null ){
                    strEditedID = m_speedDialDSP.getSpeedDialId(m_editedData) ;
                }
                if( (!bAdd) && (strEditedID != null) ){
                    bDeleteOriginalEntry = true;
                    if(strEditedID.equals(strEntryID))
                        bCheckForDuplicate = false;
                }

                if( bCheckForDuplicate ){
                    try {
                       searchEntry = m_speedDialDSP.lookupSpeedDialId(strEntryID);
                    } catch (Exception e) {
                    }

                    if (searchEntry != null) {
                        MessageBox alert =
                            new MessageBox
                                (getApplication(), MessageBox.TYPE_ERROR);

                        alert.setMessage("There is already a speed dial entry with an ID of " + strEntryID + ", Please enter a new ID.");
                        alert.showModal();

                        bAddEntry = false;
                        bDeleteOriginalEntry = false;
                        //back to edit screen
                        onEdit( enteredAttributes, strTitle, bAdd );
                    }
                }
                if( bDeleteOriginalEntry ){
                   try{
                        m_speedDialDSP.deleteEntry(m_editedData);
                   }catch( IllegalArgumentException e ){
                      SysLog.log(e);
                   }
                }
                if( bAddEntry)
                    m_speedDialDSP.addEntry(enteredAttributes);
            }

            // Refresh the display
            m_strFilterKey = "" ;
            updateFilter();

        }
    }


    private void debug(String str ){
        System.out.println("--------------------------------");
        System.out.println(str);
        System.out.println("--------------------------------");
    }




    /**
     * This form can morph slighly based on the current dialing strategy.
     * This method applies those morphing changes
     */
    private void applyDialingStrategy()
    {
        DialingStrategy strategy = Shell.getInstance().getDialingStrategy() ;

        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionDial = new PActionItem(new PLabel(strategy.getAction()),
            strategy.getActionHint(),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_DIAL) ;
        menuControl.setItem(PBottomButtonBar.B3, actionDial) ;
        enableMenusByAction(m_bContextSpecificEnabled, m_commandDispatcher.ACTION_DIAL) ;
    }


    /**
     *
     */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_DIAL         = "action_dial" ;
        public final String ACTION_CANCEL       = "action_cancel" ;
        public final String ACTION_BACKSPACE    = "action_backspace" ;
        public final String ACTION_BACKCHAR     = "action_backchar" ;
        public final String ACTION_FORWARDCHAR  = "action_forwardchar" ;
        public final String ACTION_ABOUT        = "action_about" ;
        public final String ACTION_DELETE       = "action_delete" ;
        public final String ACTION_EDIT         = "action_edit" ;
        public final String ACTION_ADD          = "action_add" ;



        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_DIAL)) {
                onDial() ;
            } else  if (event.getActionCommand().equals(ACTION_BACKSPACE)) {
                onBackspace() ;
            } else  if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_ABOUT)) {
                CoreAboutbox.display(getApplication(),"SpeedDial") ;
            } else  if (event.getActionCommand().equals(ACTION_DELETE)) {
                onDelete() ;
            } else  if (event.getActionCommand().equals(ACTION_EDIT)) {
                onEdit() ;
            } else  if (event.getActionCommand().equals(ACTION_ADD)) {
                onAdd() ;
            } else if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                // Default Double-click action
                onDial() ;
            }
        }
    }


    protected void updateFilter()
    {
        setTitle(getString("speeddial_title")+": " + m_strFilterKey) ;

        Vector vEntries = null;
        if( ! TextUtils.isNullOrSpace(m_strFilterKey) ){
            vEntries = m_speedDialDSP.filterSpeedEntries(m_strFilterKey) ;
            vEntries = m_speedDialDSP.sortEntries
                        (vEntries, m_speedDialDSP.SPEED_DIAL_SCHEMA_ID, true);
        }else{
            //we get the sorted by ID vector when using getEntries.
            vEntries = m_speedDialDSP.getEntries();
        }
        m_listModel.removeAllElements() ;
        Enumeration enum = vEntries.elements() ;
        while (enum.hasMoreElements()) {
            m_listModel.addElement(enum.nextElement()) ;
        }

        if (m_listModel.getSize() > 0) {
            // Reset the highlight bar to first row.
            // This is done for case in which
            // the filter removes all entries:
            // removing the last character from the filter
            // will make the previous list appear again.
            m_listAddressBook.setSelectedIndex(0) ;
            if (!m_bContextSpecificEnabled) {
                m_bContextSpecificEnabled = true ;
            }
        }else{
            if (m_bContextSpecificEnabled) {
                m_bContextSpecificEnabled = false ;
            }
        }
        enableMenusByAction
            (m_bContextSpecificEnabled,
             m_commandDispatcher.ACTION_DIAL) ;

        if( ! SpeedDialDSP.getInstance().isDataSourceReadOnly() ){
            enableMenusByAction
                (m_bContextSpecificEnabled,
                 m_commandDispatcher.ACTION_DELETE) ;

            enableMenusByAction
                (m_bContextSpecificEnabled,
                 m_commandDispatcher.ACTION_EDIT) ;
        }

        m_listDataListener.handleDrawingInfoMessage();
    }



    public void enableMenusByAction(boolean bEnable, String strAction)
    {
        super.enableMenusByAction(bEnable, strAction) ;
        if (m_commandBar != null) {
            m_commandBar.enableByAction(strAction, bEnable) ;
        }
    }



    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_0:
                    m_strFilterKey += '0' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_1:
                    m_strFilterKey += '1' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_2:
                    m_strFilterKey += '2' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_3:
                    m_strFilterKey += '3' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_4:
                    m_strFilterKey += '4' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_5:
                    m_strFilterKey += '5' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_6:
                    m_strFilterKey += '6' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_7:
                    m_strFilterKey += '7' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_8:
                    m_strFilterKey += '8' ;
                    updateFilter() ;
                    break ;
                case PButtonEvent.BID_9:
                    m_strFilterKey += '9' ;
                    updateFilter() ;
                    break ;
            }
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {

        }


        /**
         * The specified button is being held down
         */
        public void buttonRepeat(PButtonEvent event)
        {

        }
    }

    /**
     * Listener to listen to data added/removed actions from the list
     */
    private class icListDataListener implements PListDataListener
    {

        /**
         * An interval was added/inserted into the data model
         */
        public void intervalAdded(PListDataEvent e)
        {
            handleDrawingInfoMessage();
        }

        /**
         * An interval was deleted from the data model
         */
        public void intervalRemoved(PListDataEvent e)
        {
            handleDrawingInfoMessage();
        }

        public void contentsChanged(PListDataEvent e) {
        }

        private void drawInfoMessage()
        {
            String strTip = "";
            if( m_speedDialDSP.isDataSourceLocal() )
            {
                strTip = getString("lblDataSourceLocalSpeedDialTip");
            }else
            {
                strTip = getString("lblDataSourceConfigSpeedDialTip");
            }
            if( m_labelTip == null )
                m_labelTip = new PLabel( strTip, PLabel.ALIGN_SOUTH);
            Dimension dim = m_listAddressBook.getSize() ;
            int iHeight = dim.height ;
            int iWidth = dim.width ;
            m_labelTip.setBounds(0, iHeight/2, iWidth, iHeight/2 );
            if( iWidth != 0 && iHeight != 0 ){
                m_listAddressBook.add( m_labelTip);
                m_bLabelTipAdded = true;
            }
        }

        private void removeInfoMessage(){
            if( m_labelTip != null )
                m_listAddressBook.remove(m_labelTip);
            m_bLabelTipAdded = false;
        }

        public void handleDrawingInfoMessage(){
            if( m_listModel.getSize() > 2 ){
                if( m_bLabelTipAdded == true ){
                    removeInfoMessage();
                }
            }else{
                if( m_bLabelTipAdded == false){
                    drawInfoMessage();
                }

            }
        }

     }
}







