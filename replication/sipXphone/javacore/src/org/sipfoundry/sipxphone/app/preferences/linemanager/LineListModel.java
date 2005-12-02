/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/linemanager/LineListModel.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.preferences.linemanager ;

import org.sipfoundry.sip.* ;
import org.sipfoundry.sip.event.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;

import org.sipfoundry.util.* ;

/**
 * Data Model housing all of the line information.  This model can be added
 * as a line listener for automatic refreshing.  The model also has sorting
 * options.
 * <p>
 * The user must add the model as a sip line listener explicitly.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LineListModel extends PDefaultListModel implements SipLineListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int SORT_BY_STATE = 0 ;
    public static final int SORT_BY_IDENTITY  = 1 ;

    private static final int[] DESIRED_STATE_ORDER =
    {
        SipLine.LINE_STATE_EXPIRED,
        SipLine.LINE_STATE_FAILED,
        SipLine.LINE_STATE_TRYING,
        SipLine.LINE_STATE_REGISTERED,
        SipLine.LINE_STATE_PROVISIONED,
        SipLine.LINE_STATE_DISABLED,
        SipLine.LINE_STATE_UNKNOWN,
    } ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int m_iSortMethod = SORT_BY_STATE ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Refresh the list model.  This dumps all of the known data and refetches
     * line information from the line manager.
     */
    public synchronized void refresh()
    {
        SipLineManager manager = SipLineManager.getInstance() ;

        // Get all of the lines and sort them.
        SipLine lines[] = manager.getLines() ;
        sortLines(lines, m_iSortMethod) ;

        // Next add them to the list model.
        for (int i=0; i<lines.length; i++)
        {
            addElement(lines[i]) ;
        }
    }


    /**
     * Sets the sorting method for this data model.
     */
    public void setSortingMethod(int iSortMethod)
            throws IllegalArgumentException
    {
        // valid arguments
        if ((iSortMethod != SORT_BY_STATE) && (iSortMethod != SORT_BY_IDENTITY))
            throw new IllegalArgumentException("invalid sorting method") ;

        // Set and resort.
        if (m_iSortMethod != iSortMethod)
        {
            m_iSortMethod = iSortMethod ;
            resortModel() ;
        }
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a line is enabled.
     */
    public synchronized void lineEnabled(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            fireDataChanged(PListDataEvent.CONTENTS_CHANGED, iIndex, iIndex) ;
        }
        else
        {
            addElement(line) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a line fails.
     */
    public synchronized void lineFailed(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            fireDataChanged(PListDataEvent.CONTENTS_CHANGED, iIndex, iIndex) ;
        }
        else
        {
            addElement(line) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a line times out.
     */
    public synchronized void lineTimeout(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            fireDataChanged(PListDataEvent.CONTENTS_CHANGED, iIndex, iIndex) ;
        }
        else
        {
            addElement(line) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a new line is added.
     */
    public synchronized void lineAdded(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            fireDataChanged(PListDataEvent.CONTENTS_CHANGED, iIndex, iIndex) ;
        }
        else
        {
            addElement(line) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a line is removed.
     */
    public synchronized void lineDeleted(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            removeElementAt(iIndex) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever a line is changed.
     */
    public synchronized void lineChanged(SipLineEvent event)
    {
        SipLine line = event.getLine() ;

        int iIndex = m_vElements.indexOf(line) ;
        if (iIndex >= 0)
        {
            fireDataChanged(PListDataEvent.CONTENTS_CHANGED, iIndex, iIndex) ;
        }
        else
        {
            addElement(line) ;
        }

        resortModel() ;
    }


    /**
     * Exposed as an implementation side effect: Called as part of the
     * SipLineListener interface whenever the default line changes
     */
    public void lineDefault(SipLineEvent event)
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * Sort the array of lines given the specifed sort method.
     */
    protected void sortLines(Object objLines[], int iSortMethod)
    {
        if (iSortMethod == SORT_BY_STATE)
        {
            QuickSort qsorter = new QuickSort() ;
            try
            {
                qsorter.sort(objLines, new icStateComparer()) ;
            }
            catch (Exception e) { }
        }
        else if (iSortMethod == SORT_BY_IDENTITY)
        {
            QuickSort qsorter = new QuickSort() ;
            try
            {
                qsorter.sort(objLines, new icIdentityComparer()) ;
            }
            catch (Exception e) { }
        }
    }



    /**
     * Resort the model and fire off data change events for any items that
     * switch postions.
     */
    protected void resortModel()
    {
        int iSize = m_vElements.size() ;
        Object objArraySrc[] = new Object[iSize] ;
        Object objArraySorted[] = new Object[iSize] ;

        m_vElements.copyInto(objArraySrc) ;
        m_vElements.copyInto(objArraySorted) ;

        sortLines(objArraySorted, m_iSortMethod) ;

        for (int i=0; i<iSize; i++)
        {
            if (!objArraySrc[i].equals(objArraySorted[i]))
            {
                m_vElements.setElementAt(objArraySorted[i], i) ;
                fireDataChanged(PListDataEvent.CONTENTS_CHANGED, i, i) ;
            }
        }
    }


    /**
     * Compares by Identity.  For each identity, we will compare against
     * the line's identity, starting with the Display name, and then using the
     * user, and address any and pieces are missing.
     */
    protected int compareIdentity(SipLine line1, SipLine line2)
    {
        String strIdentity1 = line1.getIdentity() ;
        String strIdentity2 = line2.getIdentity() ;
        String strDisplayName1 ;
        String strDisplayName2 ;
        SipParser parser ;

        // Get the first display name
        parser = new SipParser(strIdentity1) ;
        strDisplayName1 = parser.getDisplayName() ;
        if ((strDisplayName1 == null) || (strDisplayName1.length() == 0))
            strDisplayName1 = parser.getUser() ;
        if ((strDisplayName1 == null) || (strDisplayName1.length() == 0))
            strDisplayName1 = parser.getHost() ;

        // Get the second display name
        parser = new SipParser(strIdentity2) ;
        strDisplayName2 = parser.getDisplayName() ;
        if ((strDisplayName2 == null) || (strDisplayName2.length() == 0))
            strDisplayName2 = parser.getUser() ;
        if ((strDisplayName2 == null) || (strDisplayName2.length() == 0))
            strDisplayName2 = parser.getHost() ;

        return strDisplayName1.compareTo(strDisplayName2) ;
    }


    /**
     * Compare the states of the specified lines.
     */
    protected int compareState(SipLine line1, SipLine line2)
    {
        int iState1 = line1.getState() ;
        int iState2 = line2.getState() ;

        // Determine the position of state 1 in the desired order list
        int iStatePosition1 = DESIRED_STATE_ORDER.length - 1;
        for (int i=0; i<DESIRED_STATE_ORDER.length; i++)
        {
            if (DESIRED_STATE_ORDER[i] == iState1)
            {
                iStatePosition1 = i ;
                break ;
            }
        }

        // Determine the position of state 2 in the desired order list
        int iStatePosition2 = DESIRED_STATE_ORDER.length - 1;
        for (int i=0; i<DESIRED_STATE_ORDER.length; i++)
        {
            if (DESIRED_STATE_ORDER[i] == iState2)
            {
                iStatePosition2 = i ;
                break ;
            }
        }

        return iStatePosition1 - iStatePosition2 ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////

    /**
     * Compares by identity and then by state
     */
    public class icIdentityComparer implements Comparer
    {
        public int compare(Object o1, Object o2)
        {
            int iRC = compareIdentity((SipLine) o1, (SipLine) o2) ;
            if (iRC == 0)
                iRC = compareState((SipLine) o1, (SipLine) o2) ;

            return iRC ;
        }
    }

    /**
     * Compares by state and then by user
     */
    public class icStateComparer implements Comparer
    {
        public int compare(Object o1, Object o2)
        {
            int iRC = compareState((SipLine) o1, (SipLine) o2) ;
            if (iRC == 0)
                iRC = compareIdentity((SipLine) o1, (SipLine) o2) ;

            return iRC ;
        }
    }
}
