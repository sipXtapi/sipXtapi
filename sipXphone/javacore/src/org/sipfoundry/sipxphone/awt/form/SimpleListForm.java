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


package org.sipfoundry.sipxphone.awt.form ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;

import java.text.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;


/**
 * SimpleListForm contains a basic list control that presents the user with
 * three possible choices or actions. A scroll bar appears as needed when
 * the form contains more list items than can display at once on screen.
 * Optionally, a command bar may be enabled to offer additional commands.
 * <p>
 * Developers should use the <i>addElement</i> and <i>insertElementAt</i>
 * methods to populate the list control. Additional methods are provided
 * for removing, accessing, and controlling selection of list items.
 * <p>
 * The bottom button bar presents three actions or choices to the user.
 * Use the <i>getBottomButtonBar</i> method to gain a reference to
 * the button bar; afterwards, use the <i>setItem</i> method to set these
 * actions.
 * <p>
 * By default, the Bottom Button Bar is initialized with a "Cancel" action
 * assigned to the middle (B2) button and an "Ok" action assigned to the right
 * (B3) button. Pressing either of these buttons will call the <i>onCancel</i>
 * handler, which then calls <i>closeForm</i>.
 * <p>
 * Developers can also populate help and additional menu items by invoking the
 * <i>setHelpText</i> and <i>setLeftMenu</i> or <i>setRightMenu</i> methods,
 * respectively. Users access help and the menu of additional actions by
 * pressing "More".
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SimpleListForm extends PApplicationForm
{
    /** Exit code return from showModal() when the form is closed by pressing
        the OK Button */
    public static final int OK = 1 ;
    /** Exit code return from showModal() when the form is closed by pressing
        the Cancel Button */
    public static final int CANCEL = 0 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** List control used in the simple form. */
    private PList m_listControl = null ;
    /** List control data model. */
    private PDefaultListModel m_listModel = null ;
    /** List selection listeners. */
    private Vector m_vListListeners = new Vector() ;
    /** Cmmand bar (null if not enabled). */
    private PCommandBar m_commandBar = null ;
    /** Display Container. */
    private icDisplayContainer m_contDisplay ;
    /** Button listener */
    private icActionDispatcher m_actionDispatcher ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor, takes an application object and form title.
     *
     * @param application Reference to the invoking application.
     * @param strTitle The title of the form (to display in the title bar).
     */
    public SimpleListForm(Application application,
                          String      strTitle)
    {
        super(application, strTitle) ;

        setTitle(strTitle) ;

        // Create all of our components.
        initControls() ;
        initMenus() ;

        // Physically lay out everything.
        layoutComponents() ;
    }

    /**
     * Constructor, takes a parent form and form title.
     *
     * @param formParent Parent form responsible for displaying this sub-form.
     * @param strTitle The title of the form (to display in the title bar).
     */
    public SimpleListForm(PForm  formParent,
                          String strTitle)
    {
        super(formParent, strTitle) ;

        setTitle(strTitle) ;

        // Create all of our components.
        initControls() ;
        initMenus() ;

        // Physically lay out everything.
        layoutComponents() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * <i>onCancel</i> is called when the user presses the middle (B2) Cancel
     * button at the bottom of the screen. It simply closes the form by
     * invoking <i>closeForm</i> with an error code of CANCEL
     *
     * @see #closeForm
     */
    public void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     * <i>onOk</i> is called when the user presses the right (B3) Ok
     * button at the bottom of the screen. It simply closes the form by
     * invoking <i>closeForm</i> with and error code of OK
     *
     * @see #closeForm
     */
    public void onOk()
    {
        closeForm(OK) ;
    }



    /**
     * Insert an object into the list control at the specified index. The
     * string obtained by invoking the object's <i>toString</i> method is
     * presented to the user.
     *
     * @param obj The object to be added.
     * @param i The index point at which to insert the component.
     *
     * @exception ArrayIndexOutOfBoundsException Thrown if the index is invalid.
     *
     * @see #addElement
     * @see #getElementAt
     */
    public void insertElementAt(Object obj, int i)
    {
        m_listModel.insertElementAt(obj, i) ;
    }


    /**
     * Add an object to the end of the list control. The string obtained by
     * invoking the object's <i>toString</i> method is presented to the user.
     *
     * @param obj The object to be added.
     *
     * @see #removeElement
     * @see #removeAllElements
     */
    public void addElement(Object obj)
    {
        m_listModel.addElement(obj) ;
    }


    /**
     * Get an object at the specified index position.
     *
     * @param iIndex An index into the list.
     * @return The object at the specified index.
     *
     * @exception ArrayIndexOutOfBoundsException Thrown if the index is
     *        negative or not less than the current size of this list.
     *
     * @see #insertElementAt
     * @see #getCount
     */
    public Object getElementAt(int iIndex)
    {
        return m_listModel.getElementAt(iIndex) ;
    }


    /**
     * Return the number of objects in this list.
     *
     * @return The number of objects in this list.
     *
     * @see #getElementAt
     */
    public int getCount()
    {
        return m_listModel.getSize() ;
    }


    /**
     * Remove the first (lowest-indexed) occurrence of the specified object
     * from this list.
     *
     * @param obj The component to be removed.
     * @return True if the argument was a component of this list; false otherwise.
     * @return The number of components in this list.
     *
     * @see #addElement
     * @see #insertElementAt
     * @see #removeAllElements
     */
    public boolean removeElement(Object obj)
    {
        return m_listModel.removeElement(obj) ;
    }


    /**
     * Delete the object at the specified index position.
     *
     * @param index The index of the object to remove.
     *
     * @exception ArrayIndexOutOfBoundsException Thrown if the index is invalid.
     *
     * @see #addElement
     * @see #insertElementAt
     */
    public void removeElementAt(int iIndex)
    {
        m_listModel.removeElementAt(iIndex) ;
    }


    /**
     * Remove all components from this list and sets its size to zero.
     *
     * @see #addElement
     * @see #insertElementAt
     */
    public void removeAllElements()
    {
        m_listModel.removeAllElements() ;
    }


    /**
     * Select the specified index position. The list control will
     * automatically scroll the object into view if not presently
     * visible.
     *
     * @exception ArrayIndexOutOfBoundsException Thrown if the index is invalid or
     *            out of range.
     *
     * @see #getSelectedIndex
     * @see #getSelectedElement
     */
    public void setSelectedIndex(int iIndex)
    {
        m_listControl.setSelectedIndex(iIndex) ;
    }


    /**
     * Get the index position of the presently selected object.
     *
     * @return Selected index position, or -1 if no selection exists.
     *
     * @see #getSelectedElement
     */
    public int getSelectedIndex()
    {
        return m_listControl.getSelectedIndex() ;
    }


    /**
     * Get the selected object.
     *
     * @return The selected object, or null if no selection exists.
     *
     * @see #getSelectedIndex
     */
    public Object getSelectedElement()
    {
        return m_listControl.getSelectedElement() ;
    }



    /**
     * Add a list selection listener. Listeners are informed whenever
     * the list selection changes.
     *
     * @param list Selection listener.
     */
    public void addListListener(PListListener list)
    {
        m_listControl.addListListener(list) ;
    }


    /**
     * Remove a list selection listener.
     *
     * @param list Selection listener.
     */
    public void removeListListener(PListListener list)
    {
        m_listControl.removeListListener(list) ;
    }


    /**
     * Add an action listener to the SimpleListForm. The SimpleListForm will
     * fire an action event when a currently selected item is selected again.
     * Forms should listen for this "double-click" action and take a sensible
     * default action. The action command will be set to the static constant
     * PList.ACTION_DOUBLE_CLICK.
     *
     * @param listener Action listener to receive double-click notifications.
     *
     * @see #removeActionListener
     */
    public void addActionListener(PActionListener listener)
    {
        m_listControl.addActionListener(listener) ;
    }


    /**
     * Remove an action listener from the SimpleListForm. The action listener
     * should have been added using the <i>addActionListener</i> method.
     *
     * @see #addActionListener
     */
    public void removeActionListener(PActionListener listener)
    {
        m_listControl.removeActionListener(listener) ;
    }


    /**
     * Is the command bar enabled and currently visible in this form?
     * The default is false and developers need to explicitly turn this
     * on by invoking <i>enableCommandBar</i>. When the command bar is enabled,
     * it presents additional actions to users.
     *
     * @return True if the command bar is enabled, otherwise false.
     *
     * @see #enableCommandBar
     * @see #getCommandBar
     */
    public boolean isCommandBarEnabled()
    {
        return (m_commandBar != null) ;
    }


    /**
     * Enable or disable the command bar.
     *
     * @param bEnable True to enable and show the command bar, or false to disable
     *        and hide it.
     *
     * @see #isCommandBarEnabled
     * @see #getCommandBar
     */
    public void enableCommandBar(boolean bEnable)
    {
        if (bEnable == true) {
            if (m_commandBar == null) {
                m_commandBar = new PCommandBar() ;
                m_contDisplay.add(m_commandBar) ;
                doLayout() ;
            }
        } else {
            if (m_commandBar != null) {
                m_contDisplay.remove(m_commandBar) ;
                m_commandBar.removeAllButtons() ;
                m_commandBar = null ;
                doLayout() ;
            }
        }
    }


    /**
     * Get a reference to the command bar. By default, the command bar is
     * disabled and developers must call <i>enableCommandBar(true)</i> prior to
     * invoking this accessor method.
     *
     * @return Reference to the command bar, or null if not enabled.
     *
     * @see #isCommandBarEnabled
     * @see #enableCommandBar
     */
    public PCommandBar getCommandBar()
    {
        return m_commandBar ;
    }


    /**
     * Close this form with the specified exit code. This exit code is
     * returned to the form invoker as the return value of <i>showModal</i>.
     *
     * @param iExitCode The exit code of the form.
     *
     * @see #showModal
     */
    public void closeForm(int iExitCode)
    {
        super.closeForm(iExitCode) ;
    }


    /**
     * Set the help text that will display on this form's help tab.
     * The help text will wrap automatically; however, you can insert
     * explicit new lines by placing a "\n" into the text string.
     *
     * @param strHelp The help string that will display on this form's
     *        help tab.
     */
    public void setHelpText(String strHelp)
    {
        super.setHelpText(strHelp) ;
    }


    /**
     * Get a reference to the bottom button bar. The bottom button bar is a
     * menu-like control that generally is filled with navigational commands.
     *
     * @return A reference to the bottom button bar control.
     */
    public PBottomButtonBar getBottomButtonBar()
    {
        return super.getBottomButtonBar() ;
    }


    /**
     * Set the left column of menu items for this form's menu tab. Each
     * form can display two columns of menu tabs. See the xDK
     * <u>Guidelines for User Interface Design</u> for recommendations
     * on menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     */
    public void setLeftMenu(PActionItem[] items)
    {
        super.setLeftMenu(items) ;
    }


    /**
     * Set the right column of menu items for this form's menu tab. Each
     * form can display two columns of menu tabs. See the xDK
     * <u>Guidelines for User Interface Design</u> for recommendations
     * on menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     */
    public void setRightMenu(PActionItem[] items)
    {
        super.setRightMenu(items) ;
    }


    /**
     * Show the form in a modal mode. The thread invoking this method will
     * be blocked until the form is closed.
     *
     * @return The return value of the form. This is the value passed into
     *        <i>closeForm</i>.
     *
     * @see #closeForm
     * @see #showModeless
     */
    public int showModal()
    {
        return super.showModal() ;

    }


    /**
     * Show the form in a modeless mode. This method is non-blocking and does
     * not wait form the form to close.
     *
     * @return boolean True on success or false on error.
     *
     * @see #showModal
     */
    public boolean showModeless()
    {
        return super.showModeless() ;
    }


    /**
     * Set the item renderer that is used to display each individual
     * list item.
     *
     * @param renderer The item renderer that will be used to display each
     *        list item.
     *
     * @see org.sipfoundry.sipxphone.awt.PItemRenderer
     */
    public void setItemRenderer(PItemRenderer renderer)
    {
        m_listControl.setItemRenderer(renderer) ;
    }


    /**
     * Get the item renderer that is currently being used to display each
     * individual list item.
     *
     * @return The current item renderer.
     *
     * @see org.sipfoundry.sipxphone.awt.PItemRenderer
     */
    public PItemRenderer getItemRenderer()
    {
        return m_listControl.getItemRenderer() ;
    }


    /**
     * @deprecated do not expose
     */
    public void doLayout()
    {
        m_contDisplay.doLayout() ;
        super.doLayout() ;
    }


    /**
     * @deprecated do not expose
     */
    public void setDefaultListModel(PDefaultListModel model)
    {
        m_listModel = model ;

        m_listControl.setListModel(m_listModel) ;
    }


    /**
     * @deprecated do not expose
     */
    public PDefaultListModel getDefaultListModel()
    {
        return m_listModel ;

    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void initControls()
    {
        m_listControl = new PList() ;
        m_listModel = new PDefaultListModel() ;
        m_listControl.setListModel(m_listModel) ;

        //add global listener
        m_actionDispatcher = new icActionDispatcher() ;
        getBottomButtonBar().addActionListener(m_actionDispatcher);
    }


    /**
     *
     */
    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;


        PActionItem actionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simplelistform/ok"),
                null, //let the global listener for this form have it
                "ACTION_OK") ;
        PActionItem menuActionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simplelistform/ok"),
                m_actionDispatcher,
                "ACTION_OK") ;
        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
        getLeftMenuComponent().addItem(menuActionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simplelistform/cancel"),
                null, //let the global listener for this form have it
                "ACTION_CANCEL") ;
        PActionItem menuActionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simplelistform/cancel"),
                m_actionDispatcher,
                "ACTION_CANCEL") ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(menuActionCancel) ;
    }

    /**
     *
     */
    private void layoutComponents()
    {
        // perform first time initialization of display container if needed
        if (m_contDisplay == null) {
            m_contDisplay = new icDisplayContainer() ;
            m_contDisplay.setLayout(null) ;
            m_contDisplay.add(m_listControl) ;

            if (m_commandBar != null)
                m_contDisplay.add(m_commandBar) ;
        }

        addToDisplayPanel(m_contDisplay, new Insets(0,0,0,0)) ;
    }


    /**
     *
     */
    private class icActionDispatcher implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals("ACTION_CANCEL"))
                onCancel() ;
            if (event.getActionCommand().equals("ACTION_OK"))
                onOk() ;
        }
    }


    private class icDisplayContainer extends PContainer
    {
        public icDisplayContainer()
        {
            this.setOpaque(false) ;
        }

        public void doLayout()
        {
            Dimension dim = this.getSize() ;

            if (m_commandBar != null) {
                m_commandBar.setBounds(0, 0, 28, dim.height-2) ;
                m_listControl.setBounds(28, 0, dim.width-30, dim.height-2) ;
            } else {
                m_listControl.setBounds(0, 0, dim.width-2, dim.height-2) ;
            }

            super.doLayout() ;
        }
    }
}

