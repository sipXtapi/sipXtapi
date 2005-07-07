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
 
package org.sipfoundry.telephony ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import javax.telephony.capabilities.* ;

import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.telephony.capabilities.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;

public class PtTerminalConnection extends PtWrappedObject implements TerminalConnection
{

//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Play Tone DTMF constants: 0 key. */
    public static final int DTMF_0  = '0' ;
    /** Play Tone DTMF constants: 1 key. */
    public static final int DTMF_1  = '1' ;
    /** Play Tone DTMF constants: 2 key. */
    public static final int DTMF_2  = '2' ;
    /** Play Tone DTMF constants: 3 key. */
    public static final int DTMF_3  = '3' ;
    /** Play Tone DTMF constants: 4 key. */
    public static final int DTMF_4  = '4' ;
    /** Play Tone DTMF constants: 5 key. */
    public static final int DTMF_5  = '5' ;
    /** Play Tone DTMF constants: 6 key. */
    public static final int DTMF_6  = '6' ;
    /** Play Tone DTMF constants: 7 key. */
    public static final int DTMF_7  = '7' ;
    /** Play Tone DTMF constants: 8 key. */
    public static final int DTMF_8  = '8' ;
    /** Play Tone DTMF constants: 9 key. */
    public static final int DTMF_9  = '9' ;
    /** Play Tone DTMF constants: star (*) key. */
    public static final int DTMF_STAR  = '*' ;
    /** Play Tone DTMF constants: pound (#) key. */
    public static final int DTMF_POUND  = '#' ;

    /** @deprecated do not expose */
    public static final int DTMF_TONES_BASE = 512 ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_DIALTONE    = (DTMF_TONES_BASE + 0) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_BUSY        = (DTMF_TONES_BASE + 1) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_RINGBACK    = (DTMF_TONES_BASE + 2) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_RINGTONE    = (DTMF_TONES_BASE + 3) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_CALLFAILED  = (DTMF_TONES_BASE + 4) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_SILENCE     = (DTMF_TONES_BASE + 5) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_BACKSPACE   = (DTMF_TONES_BASE + 6) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_CALLWAITING = (DTMF_TONES_BASE + 7) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_CALLHELD    = (DTMF_TONES_BASE + 8) ;
    /** @deprecated do not expose */
    public static final int DTMF_TONE_LOUDBUSY    = (DTMF_TONES_BASE + 9) ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private boolean m_bIsLocal = false ;
    private boolean m_bIsLocalCached = false ;



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /** @deprecated do not expose */
    public PtTerminalConnection(long lHandle)
    {
        super(lHandle) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public methods
////

    /**
     * Start playing a tone on this terminal connection.  This tone must be
     * explicitly stopped by calling the <i>stopTone</i> method.  Calling this
     * method multiple times will implicitly stop playing tones and sound files.
     *
     * @param iTone Tone id constant.  See the DTMF_* constants defined above.
     * @param bLocal Should the tone be played locally?
     * @param bRemote Should the tone be played remotely?
     *
     * @see #stopTone
     */
    public void startTone(int iTone, boolean bLocal, boolean bRemote)
    {
        JNI_startTone(m_lHandle, iTone, bLocal, bRemote) ;
    }


    /**
     * Stop playing a tone on this terminal connection.  This method
     * does nothing if no tone is playing.
     *
     * @see #startTone
     */
    public void stopTone()
    {
        JNI_stopTone(m_lHandle) ;
    }


    /**
     * Play a sound from a file.  The audio file must be a raw audio file
     * format with 8000 samples/second (Hz), 16 bit sample size (signed,
     * little endian), one channel only.
     * <p>
     * Only one sound file can be played at a time.  Attempting to play
     * another sound file without calling <i>stopPlay</i> will abort the playing
     * and may cause a resource leak.
     *
     * @param strRAWFile The fully qualified file name of the desired RAW sound
     *        file.  See method description for sound file specifications.
     * @param bRepeat The repeat flag. If this flag is true, the audio clip will
     *        be played in a loop until stopped. If this flag is false, the audio
     *        clip is played once.
     * @param bLocal Should the tone be played locally?
     * @param bRemote Should the tone be played remotely?
     *
     * @see #stopPlay
     */
    public void playFile(String strFile, boolean bRepeat, boolean bLocal, boolean bRemote)
    {
        JNI_playFile(m_lHandle, strFile, bRepeat, bLocal, bRemote) ;
    }


    /**
     * Stop playing a sound file on this terminal connection.  This method
     * does nothing if no sound files are playing.
     *
     * @param bCloseFile Should the file be closed?
     *
     * @see #playFile
     */
    public void stopPlay(boolean bCloseFile)
    {
        JNI_stopPlay(m_lHandle, bCloseFile) ;
    }


    /**
     * Convenience method that determines whether this terminal connection is local
     * to this machine.
     *
     * @deprecated do not expose, yet.
     */
    public boolean isLocal()
    {
        if (m_bIsLocalCached) {
            return m_bIsLocal ;
        } else {
            m_bIsLocal = JNI_isLocal(m_lHandle) ;
            m_bIsLocalCached = true ;
        }

        return m_bIsLocal ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public int getState()
    {
        return JNI_getState(m_lHandle) ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public Terminal getTerminal()
    {
        Terminal terminal = null ;

        long lHandle = JNI_getTerminal(m_lHandle) ;
        if (lHandle != 0) {
            terminal = new PtTerminal(lHandle) ;
        }

        return terminal ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public Connection getConnection()
    {
        Connection connection = null ;
        long lHandle = JNI_getConnection(m_lHandle) ;
        if (lHandle != 0) {
            connection = PtConnection.fromNativeObject(lHandle) ;
        }
        return connection ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public void answer()
        throws PrivilegeViolationException, ResourceUnavailableException, MethodNotSupportedException, InvalidStateException
    {
        JNI_answer(m_lHandle) ;
    }


    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public TerminalConnectionCapabilities getCapabilities()
    {
        return new PtCallControlTerminalConnectionCapabilities() ;
    }

    /**
     * Please see the Java Telephony API version 1.3 documentation.
     */
    public TerminalConnectionCapabilities getTerminalConnectionCapabilities(Terminal terminal, Address address)
        throws InvalidArgumentException, PlatformException
    {
        return new PtCallControlTerminalConnectionCapabilities() ;
    }


    /**
     * Create an appropriate connection object.  This determines whether the
     * native handle is a CallControlTerminal Connection or a
     * TerminalConnection, and creates the proper java object.
     *
     * @deprecated do not expose
     */
    public static PtTerminalConnection fromNativeObject(long lHandle)
    {
        PtTerminalConnection connection = null ;

        if (lHandle != 0) {
            connection = new PtCallControlTerminalConnection(lHandle) ;
        }

        return connection ;
    }


    /**
     * This is called when the VM decides that the object is no longer
     * useful (no more references are found to it) and it is time to
     * delete it.  At this point, we call down to the native world and
     * free the native object.
     *
     * @deprecated do not expose
     */
    protected void finalize()
        throws Throwable
    {
        JNI_finalize(m_lHandle) ;
        m_lHandle = 0 ;
    }


    /**
     * What is the string representation of this terminal connection
     *
     * @deprecated do not expose, yet.
     */
    public String toString()
    {
        int iState = getState() ;
        return "terminal connection: state=" + iState + "/" + toJTAPIEventString(iState) ;
    }


    /**
     * Convert the specified JTAPI event id into a string representation
     *
     * @deprecated do not expose, yet.
     */
    public String toJTAPIEventString(int iJTAPIEvent)
    {
        String strRC = "Unknown State" ;

        switch (iJTAPIEvent)
        {
            case CallControlTerminalConnection.IDLE:
                strRC = "IDLE (CC)" ;
                break ;
            case CallControlTerminalConnection.RINGING:
                strRC = "RINGING (CC)" ;
                break ;
            case CallControlTerminalConnection.TALKING:
                strRC = "TALKING (CC)" ;
                break ;
            case CallControlTerminalConnection.HELD:
                strRC = "HELD (CC)" ;
                break ;
            case CallControlTerminalConnection.BRIDGED:
                strRC = "BRIDGE (CC)" ;
                break ;
            case CallControlTerminalConnection.INUSE:
                strRC = "INUSE (CC)" ;
                break ;
            case CallControlTerminalConnection.DROPPED:
                strRC = "DROPPED (CC)" ;
                break ;
            case CallControlTerminalConnection.UNKNOWN:
                strRC = "UNKNOWN (CC)" ;
                break ;

            case TerminalConnection.IDLE:
                strRC = "IDLE" ;
                break ;
            case TerminalConnection.RINGING:
                strRC = "RINGING" ;
                break ;
            case TerminalConnection.PASSIVE:
                strRC = "PASSIVE" ;
                break ;
            case TerminalConnection.ACTIVE:
                strRC = "ACTIVE" ;
                break ;
            case TerminalConnection.DROPPED:
                strRC = "DROPPED" ;
                break ;
            case TerminalConnection.UNKNOWN:
                strRC = "UNKNOWN" ;
                break ;
        }

        return strRC ;
    }

    /**
     * @deprecated do not expose
     *
     * Dumps the state of this connection.
     */
    public void dump()
    {
        System.out.println("") ;
        System.out.println("DUMP PtTerminalConnection: ") ;
        System.out.println("State: " + toJTAPIEventString(getState())) ;
//        System.out.println("Terminal: " + getTerminal().getName()) ;
        System.out.println("Connection: " + getConnection().getAddress().getName()) ;
        System.out.println("Local: " + isLocal()) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native void JNI_answer(long lHandle) ;
    protected static native void JNI_startTone(long m_lHandle, int iTone, boolean bLocal, boolean bRemote) ;
    protected static native void JNI_stopTone(long m_lHandle) ;
    protected static native void JNI_playFile(long m_lHandle, String strFile, boolean bRepeat, boolean bLocal, boolean bRemote) ;
    protected static native void JNI_stopPlay(long m_lHandle, boolean bCloseFile) ;

    protected static native long JNI_getConnection(long m_lHandle) ;
    protected static native long JNI_getTerminal(long m_lHandle) ;
    protected static native boolean JNI_isLocal(long m_lHandle) ;
    protected static native void JNI_finalize(long lHandle) ;
    protected static native int  JNI_getState(long lHandle) ;
}
