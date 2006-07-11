/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *


 *
 * $$
 *
 * ** THIS SOURCE FILE IS OBSOLETE, DO NOT USE / REFERENCE IT **
 */

package org.sipfoundry.stapi ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;

import java.lang.IllegalStateException ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.service.* ;

/**
 * PhoneHelper is a utility class that handles all of the basic calling
 * constructs in a simple non-JTAPI way.  It still exposes JTAPI objects,
 * however, is much easier to use.
 * <br><br>
 * All of our state monitoring is currently plugged in here.  This should
 * be moved to our JTAPI implemention.
 *
 * ** THIS SOURCE FILE IS OBSOLETE, DO NOT USE / REFERENCE IT ** *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PhoneHelper
{
    private static PhoneHelper m_instance = null;

    protected PhoneState         m_phoneState ;
    protected AudioSourceControl m_audioSourceControl ;

    /**
     *
     */
    private PhoneHelper()
    {
        m_phoneState = PhoneState.getInstance() ;
    }


    /**
     *
     */
    public static PhoneHelper getInstance()
    {
        if (m_instance == null) {
            m_instance = new PhoneHelper() ;
        }

        return m_instance ;
    }


    /**
     * get a reference to the audio source control.
     */
    public AudioSourceControl getAudioSourceControl()
    {
        return m_audioSourceControl ;
    }


    /**
     * set the audio source control.
     */
    public void setAudioSourceControl(AudioSourceControl asc)
    {
        m_audioSourceControl = asc ;
    }
}
