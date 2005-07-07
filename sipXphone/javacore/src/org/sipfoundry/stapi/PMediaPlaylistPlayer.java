/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/PMediaPlaylistPlayer.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.stapi;

import java.net.URL;
import org.sipfoundry.telephony.* ;


public class PMediaPlaylistPlayer extends PtWrappedObject
{
    public static final int PLAYER_UNREALIZED  = 0 ;
    public static final int PLAYER_REALIZED    = 1 ;
    public static final int PLAYER_PREFETCHED  = 2 ;
    public static final int PLAYER_PLAYING     = 3 ;
    public static final int PLAYER_PAUSED      = 4 ;
    public static final int PLAYER_STOPPED     = 5 ;
    public static final int PLAYER_FAILED      = 6 ;

    protected boolean bSynchronous = true ;
    protected PtTerminalConnection m_tc ;


    protected PMediaPlaylistPlayer(PtTerminalConnection tc)
    {
        super(JNI_createPlaylistPlayer(tc.getHandle(), 0)) ;
        m_tc = tc ;
    }


    public void add(URL url, int flags)
    {
        System.out.println("in add" );
        JNI_addUrl(getHandle(), url.toString(), flags) ;
    }

/*
    protected void add(byte bytes[], int flags)
    {
        JNI_addBuffer(bytes, flags) ;
    }
*/


    public void realize()
    {
        JNI_realize(getHandle(), bSynchronous) ;
    }


    public void prefetch()
    {
        JNI_prefetch(getHandle(), bSynchronous) ;
    }


    public void play()
    {
        JNI_play(getHandle(), bSynchronous) ;
    }


    public void pause()
    {
        JNI_pause(getHandle(), 0) ;
    }


    public void stop()
    {
        JNI_stop(getHandle(), 0) ;
    }


    public int getState()
    {
        return JNI_getState(getHandle(), 0) ;
    }


    protected void finalize()
        throws Throwable
    {
/*
        if (m_lHandle != 0)
        {
            JNI_destroyPlaylistPlayer(m_lHandle, m_tc.getHandle()) ;
            m_lHandle = 0 ;
        }
*/
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native static long JNI_createPlaylistPlayer(long lTCHandle, int unused) ;
    protected native static void JNI_destroyPlaylistPlayer(long lHandle, long lTCHandle) ;

    protected native static void JNI_addUrl(long lHandle, String url, int flags);
    protected native static void JNI_addBuffer(long lHandle, byte bytes[], int flags);

    protected native static void JNI_realize(long lHandle, boolean bBlock) ;
    protected native static void JNI_prefetch(long lHandle, boolean bBlock) ;
    protected native static void JNI_play(long lHandle, boolean bBlock) ;
    protected native static void JNI_pause(long lHandle, int unused) ;
    protected native static void JNI_stop(long lHandle, int unused) ;
    protected native static int  JNI_getState(long lHandle, int unused) ;
}
