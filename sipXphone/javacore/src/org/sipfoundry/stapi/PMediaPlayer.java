/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/PMediaPlayer.java#2 $
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
import org.sipfoundry.util.TextUtils;

/**
 * PMediaPlayer provides a developer with methods to manage the life cycle of a player.
 * An instance of PMediaPlayer can be created by using PCall's <i>createMediaPlayer</i>
 * method. Example:<br>
 * <pre>
*    //call is an instance of PCall.
*    PMediaPlayer player = call.createMediaPlayer(
*                                 musicURL, //URL object of the media file
*                                 PMediaPlayer.TARGET_REMOTE,
*                                 PMediaPlayer.SOURCE_FORMAT_WAV,
*                                 PMediaPlayer.FLAG_NONE);
 * </pre>
 * <br>
 * After a player is created,
 * PMediaPlayer's <i>realize</i> and <i>prefetch</i> methods can be called in
 * initialization code to prepare the player to play media. When the <i>start</i>
 * method is called, the player starts playing and the method returns immediately.
 * <p>
 * The player continues to play until it is stopped by a call to PMediaPlayer's
 * <i>stop</i> method, or when it reaches the end of media after playing in a loop
 * for a specified number of times. The number of times a player should loop can be
 * set with the <i>setLoopCount</i> method.
 * <p>
 * A player's state can be one of the following:<br>
 * <ul>
 * <li>STATE_UNREALIZED
 * <li>STATE_REALIZED
 * <li>STATE_PREFETCHED
 * <li>STATE_STARTED
 * <li>STATE_END_OF_MEDIA
 * <li>STATE_STOPPED
 * <li>STATE_FAILED
 * <li>STATE_CLOSED
 * </ul>
 * <p>
 * In a typical lifecycle, a player's state changes to STATE_UNREALIZED,
 * STATE_REALIZED, STATE_PREFETCHED, STATE_STARTED, STATE_STOPPED, and
 * STATE_CLOSED respectively.
 *
 * <p>Be sure that the player is closed when it is no longer needed.
 * Closing the player releases the resources the player has acquired.
 *
 * @see org.sipfoundry.stapi.PCall
 *
 * @author Pradeep Paudyal
 */
public class PMediaPlayer extends PtWrappedObject
{


//////////////////////////////////////////////////////////////////////////////
// Constants
////
    //IMPORTANT:
    //The following constants are based on the constants
    //defined in mp/MpPlayer.h and mp/StreamDefs.h . So make
    //sure you change these values if the corresponding values in
    //the lower layer file are changed.

    /** The player is in the unrealized state.*/
    public static final int STATE_UNREALIZED  = 0 ;

    /** The player is in the realized state.*/
    public static final int STATE_REALIZED    = 1 ;

    /** The player is in the prefetched state. It has now acquired the resources
     *  and processed necessary data to prepare itself to play.
     */
    public static final int STATE_PREFETCHED  = 2 ;

    /** The player is in the started state. Until the player reaches the end of media
     * (if set to loop just once),or is stopped by calling the <i>stop()</i> method,
     * or is paused by calling the <i>pause</i> method, it will be in this state.
     */
    public static final int STATE_STARTED     = 3 ; //corresponds to PLAYER_PLAYING
                                              //in the lower layer

    /** The player is in the paused state.*/
    public static final int STATE_PAUSED      = 4 ;

    /** The player has reached the end of media.*/
    public static final int STATE_END_OF_MEDIA  = 5 ;
                            //corresponds to "PLAYER_STOPPED" in the lower layer.

    /** The player is in the stopped state after <i>stop</i> method is called. */
    public static final int STATE_STOPPED     = 6 ;
                      //corresponds to PLAYER_ABORTED in the lower layer.


    /** The player is in the failed state.*/
    public static final int STATE_FAILED      = 7 ;

    /** The player is in the closed state.*/
    public static final int STATE_CLOSED      = 8 ; //corresponds to PLAYER_DESTROYED
                                                     //in the lower layer.

    /** Flag; indicates that no flags need to be passed when
     *  creating media player.
     */
    public static final int FLAG_NONE          =0x00000000;

    /** Flag; indicates that caching is enabled for the player.*/
    public static final int FLAG_ENABLE_CACHE        =0x10000000;

    /** Target; indicates that the player should play media locally.*/
    public static final int TARGET_LOCAL          =0x00000002;

    /** Target; indicates that the player should play media remotely.*/
    public static final int TARGET_REMOTE         =0x00000004;

    /** Source; indicates that the player should recognize the file format automatically.*/
    public static final int SOURCE_FORMAT_AUTO  =0x00010000;

    /** Source; indicates that the data source is in .RAW format.*/
    public static final int SOURCE_FORMAT_RAW   =0x00020000;

    /** Source; indicates that the data source is in .WAV format.*/
    public static final int SOURCE_FORMAT_WAV   =0x00040000;



    //these are also defined in the lower layer enum
    //and we should make sure they match.
    protected static final int PTSTATUS_SUCCESS = 0;
    protected static final int OSSTATUS_SUCCESS = 1;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** terminal connection associated with this player */
    protected PtTerminalConnection m_tc ;

    /** format of the data source the player plays */
    protected int m_iSourceFormat;

    /** target of the player which denotes whether it plays
     * the media data locally, remotely, or both
     */
    protected int m_iTarget;

    /**flags set for the player when it was created */
    protected int m_iFlags;



//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * protected constructor to create a media player.
     */
    protected PMediaPlayer(PtTerminalConnection tc, URL url,
                            int iTarget, int iSourceFormat, int iFlags)
    {
        super(JNI_createPlayer(tc.getHandle(),
                               url.toString(),
                               iTarget|iSourceFormat|iFlags) ) ;
        m_tc = tc ;
        m_iSourceFormat = iSourceFormat;
        m_iTarget = iTarget;
        m_iFlags = iFlags;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     *  Get the format of the data source the player plays.
     *  @return Source format of the data source. Possible values are:<br>
     *   <ul>
     *     <li>PMediaPlayer.SOURCE_FORMAT_AUTO if the player is set to automatically
     *        detect the format. Only ".wav" and ".raw" format sources are supported
     *        in this release.
     *     <li>PMediaPlayer.SOURCE_FORMAT_RAW to indicate ".raw" format.
     *     <li>PMediaPlayer.SOURCE_FORMAT_WAV to indicate ".wav" format.
     *   </ul>
     */
    public int getSourceFormat()
    {
        return m_iSourceFormat;
    }

    /**
     * Get the target of the player. The target denotes whether the player plays
     * the media data locally, remotely, or both.
     * @return Target of the player. Possible values are:
     *  <ul>
     *   <li>PMediaPlayer.TARGET_LOCAL if it is set to play locally.
     *   <li>PMediaPlayer.TARGET_REMOTE if it is set to play remotely.
     *   <li>(PMediaPlayer.TARGET_LOCAL | PMediaPlayer.TARGET_REMOTE) if it
     *       is set to play both locally and remotely.
     *  </ul>
     */
    public int getTarget()
    {
        return m_iTarget;
    }

    /**
     * Get the flags set for the player when it was created.
     * @return Flags set for the player. Possible values are:
     * <ul>
     *   <li>PMediaPlayer.FLAG_ENABLE_CACHE if caching is turned on.
     *   <li>PMediaPlayer.FLAG_NONE if no flags are set.
     * </ul>
     */
    public int getFlags()
    {
        return m_iFlags;
    }


    /**
     * Examine media data and acquire non-exclusive resources.
     * After <i>realize</i> is complete, the state of the player changes to STATE_REALIZED.
     *
     *<p>A <i>realize</i> request is ignored if it is called when the player is
     * in the STATE_REALIZED, STATE_PREFETCHTED, or STATE_STARTED state.
     *
     * @exception IllegalStateException Thrown if the player is in the STATE_CLOSED state.
     * @exception PMediaException Thrown if the <i>realize</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */
    public synchronized void realize() throws PMediaException
    {
        boolean bSynchronous = true;

        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        /* Calls the lower layer API using JNI. */
        int iStatus = JNI_realize(getHandle(), bSynchronous) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            close();
            throw new PMediaException
                ("Player couldn't be realized, internal status="+iStatus);
        }
    }


    /**
     * Acquire resources and process data to reduce latency before beginning
     * to play.
     *
     * <p>After <i>prefetch</i> completes successfully, the state of the player
     * changes to STATE_PREFETCHED.

     * <p>If <i>prefetch</i> is called when the player is in the STATE_UNREALIZED state, it will
     * <i>realize</i> first and then start prefetching.<p>
     *
     * <p>A <i>prefetch</i> request is ignored if it is called when the player is
     * in the STATE_PREFETCHED or STATE_STARTED state.
     *
     * @exception IllegalStateException Thrown if the player  is in the CLOSED state.
     * @exception PMediaException Thrown if the <i>prefetch</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */

    public synchronized void prefetch() throws PMediaException
    {
        boolean bSynchronous = true;

        if( getState() == STATE_CLOSED )

            throw new IllegalStateException("The state of the player is STATE_CLOSED");
        if( getState() != STATE_REALIZED  && getState() != STATE_FAILED)
        {
            realize();
        }
        /* Calls the lower layer API using JNI. */
        int iStatus = JNI_prefetch(getHandle(), bSynchronous) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            close();
            throw new PMediaException
                ("Player couldn't be prefetched, internal status="+iStatus);
        }
    }

    /**
     * Start playing the media from the player and return immediately. This method changes
     * the state of the player to STATE_STARTED. When the player reaches the end of
     * the specified source audio file,
     * the state changes to STATE_END_OF_MEDIA.
     * </p>
     * <ul>
     * <li>
     * If <i>start</i> is called when the player is in the STATE_UNREALIZED or STATE_REALIZED state,
     * it will <i>prefetch</i> first and then start playing.
     * <li>
     * If <i>start</i> is called when player is in the STATE_PAUSED state, the player
     * will start playing from where it last paused.
     * <li> <i>start</i> is called when the player is in the STATE_STARTED state, the request
     * is ignored.
     * </ul>
     * @exception IllegalStateException Thrown if the player is in the STATE_CLOSED state.
     * @exception PMediaException Thrown if the <i>start</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */

    public synchronized void start() throws PMediaException
    {
        //play will be non-blocking be default.
        //realize, prefetch and then play.
        boolean bSynchronous = false;

        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        if( getState() != STATE_PREFETCHED  && getState() != STATE_FAILED)
        {
            prefetch();
        }

        System.out.println("State before start = "+getState());
        int iStatus = JNI_play(getHandle(), bSynchronous) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            close();
            throw new PMediaException
                ("Player couldn't be started, internal status="+iStatus);
        }
    }


    /**
     * Rewind the player and change the state to STATE_PREFETCHED.
     *
     * <p>The player is ready to be started again after <i>rewind</i> is called.
     *
     * <p>If <i>rewind</i> is called when the player is in the STATE_PREFETCHED state,
     * it is ignored.
     * @exception IllegalStateException Thrown if the player is in the STATE_CLOSED state.
     * @exception PMediaException Thrown if the <i>rewind</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */
    public synchronized void rewind()  throws PMediaException
    {
        boolean bSynchronous = true;

        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        int iStatus = JNI_rewind(getHandle(), bSynchronous) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            throw new PMediaException
                ("Player couldn't be rewinded, internal status="+iStatus);
        }
    }


    /**
     * Pause the player and change the state to STATE_PAUSED.
     *
     * <p>If <i>pause</i> is called when the player is in the STATE_PAUSED state, the request
     * will be ignored.
     *
     * <p>If <i>start</i> is called when player is in the STATE_PAUSED state, the player
     * will start playing from where it last stopped.
     *
     * @exception IllegalStateException Thrown if the player is in the STATE_CLOSED state.
     * @exception PMediaException Thrown if the <i>pause</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */
    public synchronized void pause() throws PMediaException
    {
        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        int iStatus = JNI_pause(getHandle(), 0) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            throw new PMediaException
                ("Player couldn't be paused, internal status="+iStatus);
        }
    }

    /**
     * Stop playing media in the player and change the state to STATE_STOPPED.
     *
     * <p>The player will only be ready to be started again if <i>rewind</i> is called
     * after the <i>stop</i>.
     *
     * <p>If <i>stop</i> is called when the player is in the STATE_STOPPED state,
     * it will be ignored.
     *
     * @exception IllegalStateException Thrown if the player is in the STATE_CLOSED state.
     * @exception PMediaException Thrown if the <i>stop</i> request couldn't be
     * completed and the state of the player changed to STATE_FAILED.
     */
    public synchronized void stop()  throws PMediaException
    {
        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        int iStatus = JNI_stop(getHandle(), 0) ;
        if( iStatus != OSSTATUS_SUCCESS )
        {
            throw new PMediaException
                ("Player couldn't be stopped, internal status="+iStatus);
        }
    }

    /**
     * Get the current state of the player. The possible states are:</p>
     * <ul>
     *
     * <li>STATE_UNREALIZED
     * <li>STATE_REALIZED
     * <li>STATE_PREFETCHED
     * <li>STATE_STARTED
     * <li>STATE_END_OF_MEDIA
     * <li>STATE_PAUSED
     * <li>STATE_STOPPED
     * <li>STATE_FAILED
     * <li>STATE_CLOSED
     * </ul>
     * @return The player's current state.
     */
    public synchronized int getState()
    {
        int iState = STATE_CLOSED ;    // Assume closed if no handle

        if (getHandle() != 0)
        {
            iState = JNI_getState(getHandle(), 0) ;
        }

        return iState ;
    }

    /**
     * Close the media player and release the resources acquired by it. This method changes
     * the state of the player to STATE_CLOSED, and the player cannot be used again.
     *
     * <p>If <i>close</i> is called when the player is in the STATE_CLOSED state,
     * it is ignored.
     */
    public synchronized void close() throws PMediaException
    {
        if( getState() == STATE_CLOSED )
            return;

        int iStatus = JNI_destroyPlayer(getHandle(), m_tc.getHandle()) ;
        m_lHandle = 0 ;
        if( iStatus != PTSTATUS_SUCCESS )
        {
            throw new PMediaException
                ("Player couldn't be closed, internal status="+iStatus);
        }
    }

    /**
     * Set the number of times the player should play the source audio file
     * in a continuous loop.
     *@param iCount If iCount is set to -1, the player loops indefinitely. The default
     * loop value is 1.
     * <p>If set to 0, an IllegalArgumentException is thrown.
     *@exception IllegalStateException Thrown if the player is in the STATE_CLOSED state
     *@exception IllegalArgumentException Thrown if count is 0.
     */
    public synchronized void setLoopCount(int iCount)
    {
        if( getState() == STATE_CLOSED )
            throw new IllegalStateException("The state of the player is STATE_CLOSED");

        if( iCount == 0 )
            throw new IllegalArgumentException("loop count cannot be 0.");

        JNI_setLoopCount(getHandle(), iCount, 0);
    }


//////////////////////////////////////////////////////////////////////////////
// Protected Methods
////



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native static long JNI_createPlayer(long lTCHandle, String url, int flags) ;
    protected native static int JNI_destroyPlayer(long lHandle, long lTCHandle) ;

    protected native static int JNI_realize(long lHandle, boolean bBlock) ;
    protected native static int JNI_prefetch(long lHandle, boolean bBlock) ;
    protected native static int JNI_play(long lHandle, boolean bBlock) ;
    protected native static int JNI_rewind(long lHandle, boolean bBlock) ;
    protected native static int JNI_pause(long lHandle, int unused) ;
    protected native static int JNI_stop(long lHandle, int unused) ;
    protected native static int JNI_getState(long lHandle, int unused) ;
    protected native static int JNI_setLoopCount(long lHandle, int iLoopCount, int unused) ;

}
