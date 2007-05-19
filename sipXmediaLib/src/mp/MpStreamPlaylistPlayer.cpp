//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpStreamPlaylistPlayer.h"
#include "mp/MpStreamMsg.h"
#include "os/OsEventMsg.h"
#include "os/OsEvent.h"
#include "os/OsQueuedEvent.h"
#include "os/OsMsgQ.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Contructor accepting a flow graph
MpStreamPlaylistPlayer::MpStreamPlaylistPlayer(OsMsgQ* pMsgQ, const char* pTarget)  
   : OsServerTask("PlaylistPlay-%d")
   , MpPlayer()
   , mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemWaitSync(OsBSem::Q_FIFO, OsBSem::EMPTY)
   , mRealizeTimeout(REALIZE_TIMEOUT, 0)
   , mPrefetchTimeout(PREFETCH_TIMEOUT, 0)
   , mPlayTimeout(PLAY_TIMEOUT, 0)
   , mRewindTimeout(REWIND_TIMEOUT, 0)
   , mStopTimeout(STOP_TIMEOUT, 0)
   , mDestroyTimeout(DESTROY_TIMEOUT, 0)

{
   mpMsgQ = pMsgQ;
   if (pTarget != NULL)
      mTarget = pTarget;
   mpQueueEvent = NULL;

   for (int i = 0; i < MAX_PLAYLIST_LENGTH; i++)
   {
      mPlayListDb[i].sourceType = 0;
      mPlayListDb[i].pBuffer = NULL;
      mPlayListDb[i].handle = NULL;
      mPlayListDb[i].state = PlayerUnrealized;
      mPlayListDb[i].flags = 0;
      mPlayListDb[i].pQueuedEvent = NULL;
   }

   mNumPlayListElements = 0;
   mCurrentElement = 0;
   mPlayingElement = -1;
   mbAutoAdvance = FALSE;
   mAggregateState = PlayerUnrealized;

   start();

   // Wait for the stream player to startup.
   while (!isStarted())
   {
      OsTask::yield();
   }
}


// Destructor
MpStreamPlaylistPlayer::~MpStreamPlaylistPlayer()
{   
   reset();

   // Wake up anyone waiting on this class
   mSemWaitSync.release();

   if (mpQueueEvent != NULL)
   {
      delete mpQueueEvent;
   }
}

/* ============================ MANIPULATORS ============================== */

// Adds a url to the playlist 
OsStatus MpStreamPlaylistPlayer::add(Url& url, int flags)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mNumPlayListElements < MAX_PLAYLIST_LENGTH)
   {
      int index = mNumPlayListElements;
      mNumPlayListElements++;

      mPlayListDb[index].sourceType = SourceUrl;
      mPlayListDb[index].url = url;
      mPlayListDb[index].flags = flags;       
      mPlayListDb[index].pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), index);

      status = OS_SUCCESS;
   }

   return status;
}


// Adds a buffer to the playlist  
OsStatus MpStreamPlaylistPlayer::add(UtlString* pBuffer, int flags)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mNumPlayListElements < MAX_PLAYLIST_LENGTH)
   {
      int index = mNumPlayListElements;
      mNumPlayListElements++;

      mPlayListDb[index].sourceType = SourceBuffer;
      mPlayListDb[index].pBuffer = pBuffer;
      mPlayListDb[index].flags = flags;       
      mPlayListDb[index].pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), index);

      status = OS_SUCCESS;
   }
   return status;   
}


// Realizes the player by initiating a connection to the target, allocates 
// buffers, etc.
OsStatus MpStreamPlaylistPlayer::realize(UtlBoolean bBlock)
{
   OsStatus status = OS_FAILED;
   int i;

   if (mAggregateState == PlayerFailed)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize failure, mAggregateState == PlayerFailed");
      return status;
   }

   // Start prefetching all of the elements
   for (i = 0; (i < mNumPlayListElements) && (mAggregateState != PlayerFailed); i++)
   {         
      if (mPlayListDb[i].state == PlayerUnrealized)
      {
         OsEvent eventHandle;

         // Realize the stream
         if (mPlayListDb[i].sourceType == SourceUrl)
         {            
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_URL,
                            mTarget,
                            NULL,
                            &eventHandle,
                            mPlayListDb[i].pQueuedEvent,
                            mPlayListDb[i].flags,
                            (int) new Url(mPlayListDb[i].url));                   
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(i, PlayerFailed);
               mPlayListDb[i].handle = NULL;
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize failed on send of MpStreamMsg::STREAM_REALIZE_URL message");
            }
         }
         else if (mPlayListDb[i].sourceType == SourceBuffer)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_BUFFER,
                            mTarget,
                            NULL,
                            &eventHandle,
                            mPlayListDb[i].pQueuedEvent,
                            mPlayListDb[i].flags,
                            (int) mPlayListDb[i].pBuffer);                            
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(i, PlayerFailed);
               mPlayListDb[i].handle = NULL;
               delete mPlayListDb[i].pBuffer;
               mPlayListDb[i].pBuffer = NULL;
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize failed on send of MpStreamMsg::STREAM_REALIZE_BUFFER message");
            }
         }

         if (status == OS_SUCCESS)
         {
            // Wait for a response
            int eventData;
            status = eventHandle.wait(mRealizeTimeout);
            if (status == OS_SUCCESS)
               status = eventHandle.getEventData(eventData);
            if (status == OS_SUCCESS)
            {
               mPlayListDb[i].handle = (StreamHandle) eventData;
            }
            else
            {
               setEntryState(i, PlayerFailed);
               mPlayListDb[i].handle = NULL;
               if (mPlayListDb[i].sourceType == SourceBuffer)
               {
                  delete mPlayListDb[i].pBuffer;
                  mPlayListDb[i].pBuffer = NULL;
               }
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize STREAM_REALIZE_ request failed");
            }
         }
      }
   }

   // Block if requested
   if ((status == OS_SUCCESS) && bBlock)
   {
      for (i = 0; (i < mNumPlayListElements) && (mAggregateState != PlayerFailed); i++)
      {
         while (mPlayListDb[i].state == PlayerUnrealized)
         {
            status = mSemStateChange.acquire(mRealizeTimeout);
            if (status == OS_WAIT_TIMEOUT)
            {
               setEntryState(i, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::realize timed out waiting for Realize to complete");
            }
         }
      }
   }

   return status;
}
     

// Prefetch enough of the data source to ensure a smooth playback.
OsStatus MpStreamPlaylistPlayer::prefetch(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED;
   int i;

   // Start prefetching all of the elements
   for (i = 0; (i < mNumPlayListElements) && (mAggregateState != PlayerFailed); i++)
   {         
      if (mPlayListDb[i].state == PlayerRealized)
      {

         MpStreamMsg msg(MpStreamMsg::STREAM_PREFETCH, mTarget, mPlayListDb[i].handle);
         status = mpMsgQ->send(msg);         
         if (status != OS_SUCCESS)
         {
            setEntryState(i, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::prefetch failed on send of MpStreamMsg::STREAM_PREFETCH message");
         }
      }
   }

   // Block if requested
   if ((status == OS_SUCCESS) && bBlock)
   {
      for (i = 0; (i < mNumPlayListElements) && (mAggregateState != PlayerFailed); i++)
      {
         while (mPlayListDb[i].state == PlayerRealized)
         {
            status = mSemStateChange.acquire(mPrefetchTimeout);
            if (status == OS_WAIT_TIMEOUT)
            {
               setEntryState(i, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::prefetch timed out waiting for Prefetch to complete");
            }
         }
      }
   }

   return status;
}



// Plays the media stream.  This will play all play lists from start to 
// finish.
OsStatus MpStreamPlaylistPlayer::play(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED;

   // If the player is in a failed or unrealized state, abort.
   if ((mAggregateState == PlayerFailed) || (mAggregateState == PlayerUnrealized))
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::play request failed due to player being in invalid state");
      return OS_INVALID_STATE;
   }

   if (bBlock) 
   {      
      first();
      do
      {
         mbAutoAdvance = FALSE;         
         status = playNext(); 
      }
      while ((status == OS_SUCCESS) && (mCurrentElement < mNumPlayListElements));      
   }
   else
   {
      mbAutoAdvance = TRUE;
      if ((mAggregateState == PlayerPaused) && (mPlayingElement != -1))
      {
         status = playEntry(mPlayingElement, bBlock);
      }
      else
         status = playNext(FALSE);
   }

   return status;
}


// Wait until all play list items are finished playing
OsStatus MpStreamPlaylistPlayer::wait(const OsTime& rTimeout) 
{
   OsStatus status = OS_SUCCESS;

   while (  (mAggregateState == PlayerRealized)
         || (mAggregateState == PlayerPrefetched) 
         || (mAggregateState == PlayerPlaying) 
         || (mAggregateState == PlayerPaused))
   {
      status = mSemWaitSync.acquire(rTimeout);
      if (status != OS_SUCCESS)
         break;
   }

   return status;
}


// Plays the media stream.  This will play all play lists from start to 
// finish.
OsStatus MpStreamPlaylistPlayer::rewind(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_SUCCESS;

   stop();
   mAggregateState = PlayerUnrealized;

   // Start prefetching all of the elements
   for (int i = 0; i < mNumPlayListElements; i++)
   {
      rewindEntry(i, bBlock);
   }

   mCurrentElement = 0;
   mPlayingElement = -1;

   return status;
}


// Resets the playlist player state by stopping and removing all entries.
OsStatus MpStreamPlaylistPlayer::reset()
{
   OsStatus status = OS_SUCCESS;

   for (int i = 0; i < mNumPlayListElements; i++)
   {
      destroyEntry(i);
   }

   mNumPlayListElements = 0;
   mCurrentElement = 0;
   mPlayingElement = -1;
   mbAutoAdvance = FALSE;
   mAggregateState = PlayerUnrealized;

   return status;
}



// Stops play the media stream and resources used for buffering and streaming.
OsStatus MpStreamPlaylistPlayer::stop()
{
   OsStatus status = OS_SUCCESS;
   mbAutoAdvance = FALSE;

   mCurrentElement = mNumPlayListElements;
   for (int i = 0; i < mNumPlayListElements; i++)
   {
      stopEntry(i);      
   }

   return status;
}



// Marks the player as destroy and frees all allocated resources in media 
// processing.
OsStatus MpStreamPlaylistPlayer::destroy()
{
   OsStatus status = OS_SUCCESS;   // This is non-blocking, assume success.
 
   mCurrentElement = mNumPlayListElements;
   for (int i = 0; i < mNumPlayListElements; i++)
   {
      destroyEntry(i, false);      
   }

   return status;
}


// Pauses the media stream temporarily.  
OsStatus MpStreamPlaylistPlayer::pause()
{
   OsStatus status = OS_FAILED;

   int iPlayingElement = mPlayingElement;
   if (iPlayingElement != -1)
   {
      status = pauseEntry(iPlayingElement);
   }

   return status;
}


/* ============================ ACCESSORS ================================= */

// Gets the number of play list entries
OsStatus MpStreamPlaylistPlayer::getCount(int& count) const
{
   count = mNumPlayListElements;
   return OS_SUCCESS;
}


// Gets the source type for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceType(int index, int& type) const
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      type = mPlayListDb[index].sourceType;
      status = OS_SUCCESS;
   }

   return status;
}


// Gets the source url for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceUrl(int index, Url url) const
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      if (mPlayListDb[index].sourceType == SourceUrl)
      {
         url = mPlayListDb[index].url;
         status = OS_SUCCESS;
      }
   }

   return status;
}


// Gets the source buffer for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceBuffer(int index, UtlString*& netBuffer) const
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      if (mPlayListDb[index].sourceType == SourceBuffer)
      {
         netBuffer = mPlayListDb[index].pBuffer;
         status = OS_SUCCESS;
      }
   }

   return status;
}

// Gets the state for the playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceState(int index, PlayerState& state) const

{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      state = mPlayListDb[index].state;
      status = OS_SUCCESS;
   }

   return status;
}

// Gets the current playing index if playing or the next index to play if 
// playNext() was invoked.
OsStatus MpStreamPlaylistPlayer::getCurrentIndex(int& index) const
{
   OsStatus status = OS_SUCCESS;

   if (mPlayingElement == -1)
      index = mCurrentElement;
   else
      index = mPlayingElement;

   return status;
}

// Gets the aggregate playerlist player state 
OsStatus MpStreamPlaylistPlayer::getState(PlayerState& state) 
{
   state = mAggregateState;
   
   return OS_SUCCESS;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
MpStreamPlaylistPlayer::MpStreamPlaylistPlayer(const MpStreamPlaylistPlayer& rMpStreamPlaylistPlayer)
   : mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
   , mSemWaitSync(OsBSem::Q_FIFO, OsBSem::EMPTY)
{
}

// Assignment operator
MpStreamPlaylistPlayer& 
MpStreamPlaylistPlayer::operator=(const MpStreamPlaylistPlayer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Selects the first playlist entry as the next index to play.
OsStatus MpStreamPlaylistPlayer::first()
{   
   if (mPlayingElement != -1)
      stop();

   mCurrentElement = 0;

   return OS_SUCCESS;
}

// Selects the last playlist entry as the next index to play.
OsStatus MpStreamPlaylistPlayer::last()
{
   stop();

   if (mNumPlayListElements > 0)
      mCurrentElement = mNumPlayListElements - 1;
   else
      mCurrentElement = 0;

   return OS_SUCCESS;
}


// Plays the next playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playNext(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mCurrentElement < mNumPlayListElements)
   {
      int iPlayElement = mCurrentElement++;
      status = playEntry(iPlayElement, bBlock);
   }

   return status;
}


// Plays the previous playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playPrevious(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED;

   if (mCurrentElement > 0)
   {  
      mCurrentElement--;
      status = playEntry(mCurrentElement, bBlock);
      mCurrentElement++;
   }

   return status;
}


// Sets the state for a specific entry.
void MpStreamPlaylistPlayer::setEntryState(int iEntry, PlayerState newState)
{
   PlayerState oldState = mPlayListDb[iEntry].state;

   if (oldState != newState)
   {
#ifdef MP_STREAM_DEBUG /* [ */
      osPrintf("MpStreamPlaylistPlayer::setEntryState(%p): Setting mPlayListDb[%d].state = %s\n",
               this, iEntry, getEventString(newState));
#endif /* MP_STREAM_DEBUG ] */
      // Store the new state
      mPlayListDb[iEntry].state = newState;

      // Updated aggregate state given the new entry state
      switch (newState)
      {
         case PlayerUnrealized:            
            break;
         case PlayerRealized:
            handleRealizedState(iEntry, oldState, newState);
            break;
         case PlayerPrefetched:
            handlePrefetchedState(iEntry, oldState, newState);
            break;
         case PlayerPlaying:
            handlePlayingState(iEntry, oldState, newState);
            break;
         case PlayerPaused:
            handlePausedState(iEntry, oldState, newState);
            break;
         case PlayerStopped:
         case PlayerAborted:
            handleStoppedState(iEntry, oldState, newState);
            break;
         case PlayerDestroyed:
            break;
         case PlayerFailed:
            handleFailedState(iEntry, oldState, newState);
            break;
      }
      
      // Update any blocking calls
      mSemStateChange.release();
   }
}


// Starts playing a specific entry
OsStatus MpStreamPlaylistPlayer::playEntry(int iEntry, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {      
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (mPlayListDb[iEntry].state == PlayerRealized) || 
            (mPlayListDb[iEntry].state == PlayerPaused) ||
            (mPlayListDb[iEntry].state == PlayerPrefetched))
      {
         mPlayingElement = iEntry;         
         MpStreamMsg msg(MpStreamMsg::STREAM_PLAY, mTarget, mPlayListDb[iEntry].handle);
         status = mpMsgQ->send(msg);
         if (status != OS_SUCCESS)
         {
            setEntryState(iEntry, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::playEntry failed on send of MpStreamMsg::STREAM_PLAY message");
         }
         else 
         {
            if (mPlayListDb[iEntry].state == PlayerPaused)
               setEntryState(iEntry, PlayerPlaying);

            // Block while waiting for play to complete (if requested)
            if ((status == OS_SUCCESS)  && bBlock)
            {
               while (  (mPlayListDb[iEntry].state == PlayerRealized)
                     || (mPlayListDb[iEntry].state == PlayerPrefetched) 
                     || (mPlayListDb[iEntry].state == PlayerPlaying) 
                     || (mPlayListDb[iEntry].state == PlayerPaused))
               {
                  status = mSemStateChange.acquire(mPlayTimeout);
                  if (status == OS_WAIT_TIMEOUT)
                  {
                     setEntryState(iEntry, PlayerFailed);
                     OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::playEntry timed out waiting for play to complete");
                  }
               }
            }
         }
      }
      else
      {
         status = OS_INVALID_STATE;
         OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::playEntry failed due to current state being invalid");
      }      
   }
   return status;
}


// Rewinds a previously played media stream.  In some cases this may result
// in a re-connect/refetch.
OsStatus MpStreamPlaylistPlayer::rewindEntry(int iEntry, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {      
      if (mPlayListDb[iEntry].state == PlayerPrefetched)
      {
         status = OS_SUCCESS;
      }
      else
      {         
         // Only proceed if we have a flow graph and the player is realized.
         // NOTE: The player doesn't need to be prefetched
         if ((mPlayListDb[iEntry].state == PlayerStopped) || (mPlayListDb[iEntry].state == PlayerAborted))
         {         
            MpStreamMsg msg(MpStreamMsg::STREAM_REWIND, mTarget, mPlayListDb[iEntry].handle);
            status = mpMsgQ->send(msg);
            if (status != OS_SUCCESS)
            {
               setEntryState(iEntry, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::rewindEntry failed on send of MpStreamMsg::STREAM_REWIND message");
            }
            else 
            {
               // Block while waiting for rewind to complete (if requested)
               if (bBlock)
               {
                  while ((mPlayListDb[iEntry].state == PlayerStopped) || 
                        (mPlayListDb[iEntry].state == PlayerAborted)  ||
                        (mPlayListDb[iEntry].state == PlayerRealized))
                  {
                     status = mSemStateChange.acquire(mRewindTimeout);
                     if (status == OS_WAIT_TIMEOUT)
                     {
                        setEntryState(iEntry, PlayerFailed);
                        OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::rewindEntry timed out waiting for Rewind to complete");
                     }
                  }
               }
            }
         }
         else
         {
            status = OS_INVALID_STATE;
         }
      }
   }
   return status;
}


// Stops playing a specific entry
OsStatus MpStreamPlaylistPlayer::stopEntry(int iEntry, UtlBoolean bBlock /* = TRUE */)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {      
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (mPlayListDb[iEntry].state == PlayerPrefetched) || 
            (mPlayListDb[iEntry].state == PlayerPlaying) || 
            (mPlayListDb[iEntry].state == PlayerPaused))
      {        
         MpStreamMsg msg(MpStreamMsg::STREAM_STOP, mTarget, mPlayListDb[iEntry].handle);
         status = mpMsgQ->send(msg);         
         if (status != OS_SUCCESS)
         {
            setEntryState(iEntry, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::stopEntry failed on send of MpStreamMsg::STREAM_STOP message");
         }
         else 
         {
            if (bBlock)
            {
               while ((mPlayListDb[iEntry].state != PlayerStopped) && 
                      (mPlayListDb[iEntry].state != PlayerAborted))
                
               {
                  status = mSemStateChange.acquire(mStopTimeout);
                  if (status == OS_WAIT_TIMEOUT)
                  {
                     setEntryState(iEntry, PlayerFailed);
                     OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::stopEntry timed out waiting for Stop to complete");
                     break;
                  }
               }
            }
         }
      }
      else if ((mPlayListDb[iEntry].state == PlayerStopped) || 
               (mPlayListDb[iEntry].state == PlayerAborted))
         status = OS_SUCCESS;
   }
   return status;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::pauseEntry(int iEntry)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {
      status = OS_SUCCESS;

      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if ( (mPlayListDb[iEntry].state == PlayerPrefetched) ||
           (mPlayListDb[iEntry].state == PlayerPlaying))
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PAUSE, mTarget, mPlayListDb[iEntry].handle);
         status = mpMsgQ->send(msg);         
         if (status != OS_SUCCESS)
         {
            setEntryState(iEntry, PlayerFailed);
            OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::pauseEntry failed on send of MpStreamMsg::STREAM_PAUSE message");
         }
      }
   }

   return status;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::destroyEntry(int iEntry, UtlBoolean bBlockAndClean /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {
      status = OS_SUCCESS;

      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if (mPlayListDb[iEntry].state != PlayerUnrealized)
      {
         int iState = mPlayListDb[iEntry].state;
         if (  (iState != PlayerStopped) && 
               (iState != PlayerAborted) && 
               (iState != PlayerDestroyed)  )
         {
            MpStreamMsg msgStop(MpStreamMsg::STREAM_STOP, mTarget, mPlayListDb[iEntry].handle);
            status = mpMsgQ->send(msgStop);
            if (status != OS_SUCCESS)
            {
               setEntryState(iEntry, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::deleteEntry failed on send of MpStreamMsg::STREAM_STOP message");
            }
         }

         if ((iState != PlayerDestroyed) && (status == OS_SUCCESS))
         {
            MpStreamMsg msgDestroy(MpStreamMsg::STREAM_DESTROY, mTarget, mPlayListDb[iEntry].handle);
            status = mpMsgQ->send(msgDestroy);
            if (status != OS_SUCCESS)
            {
               setEntryState(iEntry, PlayerFailed);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::deleteEntry failed on send of MpStreamMsg::STREAM_DESTROY message");
            }
         }         

         
         if (bBlockAndClean)
         {
            while (mPlayListDb[iEntry].state != PlayerDestroyed)
            {
               status = mSemStateChange.acquire(mDestroyTimeout);
               if (status == OS_WAIT_TIMEOUT)
               {
                  OsSysLog::add(FAC_MP, PRI_DEBUG, "MpStreamPlaylistPlayer::deleteEntry timed out waiting for Delete to complete");
                  break;
               }
            }         

            mPlayListDb[iEntry].sourceType = 0;
            mPlayListDb[iEntry].pBuffer = NULL;
            mPlayListDb[iEntry].handle = NULL;
            mPlayListDb[iEntry].state = PlayerUnrealized;
            mPlayListDb[iEntry].flags = 0;
            if (mPlayListDb[iEntry].pQueuedEvent != NULL)
            {
               delete mPlayListDb[iEntry].pQueuedEvent;
               mPlayListDb[iEntry].pQueuedEvent = NULL;   
            }
         }
      }
   }

   return status;
}


// Handle messages directed to this server task.
UtlBoolean MpStreamPlaylistPlayer::handleMessage(OsMsg& rMsg) 
{
   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         OsEventMsg* pMsg = (OsEventMsg*) &rMsg;
         int status;
         PlayerState oldState;         
         int index;

         pMsg->getUserData(index);
         if (pMsg->getEventData(status) == OS_SUCCESS)
         {
#ifdef MP_STREAM_DEBUG /* [ */
            osPrintf("MpStreamPlaylistPlayer::handleMessage(%p): Received Feeder event: %s \n", 
                     this, getFeederEventString(status));
#endif /* MP_STREAM_DEBUG ] */

            getSourceState(index, oldState);
            switch (status)
            {
               case FeederRealizedEvent:
                  setEntryState(index, PlayerRealized);                  
                  break;

               case FeederPrefetchedEvent:
                  setEntryState(index, PlayerPrefetched);
                  break;

               case FeederStoppedEvent:
                  if (mAggregateState != PlayerPlaying)
                  {
                     setEntryState(index, PlayerPrefetched);
                  }
                  break;

               case FeederRenderingEvent:
                  break;

               case FeederFailedEvent:                     
                  setEntryState(index, PlayerFailed);
                  break;

               case FeederStreamPlayingEvent:
                  setEntryState(index, PlayerPlaying);
                  break;

               case FeederStreamPausedEvent:
                  setEntryState(index, PlayerPaused);
                  break;               

               case FeederStreamStoppedEvent:
                  setEntryState(index, PlayerStopped);
                  break;
               
               case FeederStreamDestroyedEvent:
                  setEntryState(index, PlayerDestroyed);
                  break;
               
               case FeederStreamAbortedEvent:
                  setEntryState(index, PlayerStopped);
                  break;

            }
         }
         break;
   }

   return TRUE;
}


// Handles processing for the realized state
void MpStreamPlaylistPlayer::handleRealizedState(int index, PlayerState oldState, PlayerState newState)
{
   //
   // Updated the mAggregateState if all play list items are realized
   //
   if (mAggregateState == PlayerUnrealized)
   {
      UtlBoolean bAnyUnRealized = FALSE;
      for (int i = 0; i < mNumPlayListElements; i++)
      {
         if (mPlayListDb[i].state == PlayerUnrealized)
         {
            bAnyUnRealized = TRUE;
            break;
         }
      }

      // If everything is PlayerRealized or better, then updated the aggregate
      // state and fire off the event to listeners
      if (!bAnyUnRealized)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handleRealizedState(%p): Changed from %s to PlayerRealized.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerRealized;
         fireEvent(PlayerRealized);
      }
   }
}

  
// Handles processing for the prefetched state
void MpStreamPlaylistPlayer::handlePrefetchedState(int index, PlayerState oldState, PlayerState newState)
{   
   //
   // Updated the mAggregateState if all play list items are prefetched
   //
   if ((mAggregateState == PlayerUnrealized) || (mAggregateState == PlayerRealized))
   {
      UtlBoolean bAllPrefetched = TRUE;
      for (int i = 0; i < mNumPlayListElements; i++)
      {
         if (  (mPlayListDb[i].state != PlayerPrefetched) && 
               (mPlayListDb[i].state != PlayerFailed))
         {
            bAllPrefetched = FALSE;
            break;
         }
      }

      // If everything is prefetched, then updated the aggregate state and
      // fire off the event to listeners
      if (bAllPrefetched)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePrefetchedState(%p): Changed from %s to PlayerPrefetched.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerPrefetched;
         fireEvent(PlayerPrefetched);
      }
   }
}


// Handles processing for the playing state
void MpStreamPlaylistPlayer::handlePlayingState(int index, PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPlaying)
   {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePlayingState(%p): Changed from %s to PlayerPlaying.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
      mAggregateState = PlayerPlaying;
      fireEvent(PlayerPlaying);
   }
}


// Handles processing for the paused state
void MpStreamPlaylistPlayer::handlePausedState(int index, PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPaused)
   {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handlePausedState(%p): Changed from %s to PlayerPaused.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerPaused;
         fireEvent(PlayerPaused);
   }
}


// Handles processing for the stopped state
void MpStreamPlaylistPlayer::handleStoppedState(int index, PlayerState oldState, PlayerState newState)
{
   if (  (mbAutoAdvance) && 
         (mCurrentElement < mNumPlayListElements) && 
         (newState != PlayerAborted))
   {
      playNext(FALSE);
   }
   else if (newState == PlayerAborted)
   {
      if (mAggregateState != PlayerAborted)
      {
#ifdef MP_STREAM_DEBUG /* [ */
         osPrintf("MpStreamPlaylistPlayer::handleStoppedState(%p): Changed from %s to PlayerAborted.\n",
                  this, getEventString(mAggregateState));
#endif /* ] */
         mAggregateState = PlayerAborted;
         fireEvent(PlayerAborted);
         // Wake up anyone waiting on play completion.
         mSemWaitSync.release();
      }
   }
   else
   {
      if (mCurrentElement >= mNumPlayListElements)
      {
         mbAutoAdvance = FALSE;

         if (mAggregateState != PlayerStopped)
         {
#ifdef MP_STREAM_DEBUG /* [ */
            osPrintf("MpStreamPlaylistPlayer::handleStoppedState(%p): Changed from %s to PlayerStopped.\n",
                     this, getEventString(mAggregateState));
#endif /* ] */
            mAggregateState = PlayerStopped;
            fireEvent(PlayerStopped);
            // Wake up anyone waiting on play completion.
            mSemWaitSync.release();
         }
      }
   }
}


// Handles processing for the failed state
void MpStreamPlaylistPlayer::handleFailedState(int index, PlayerState oldState, PlayerState newState)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("MpStreamPlaylistPlayer::handleFailedState(%p): Changed from %s to PlayerFailed.\n",
            this, getEventString(mAggregateState));
#endif /* ] */
   mAggregateState = PlayerFailed;
   // Wake up anyone waiting on play completion.
   mSemWaitSync.release();
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
const char* MpStreamPlaylistPlayer::getFeederEventString(int iEvent)
{
   const char* szRC = "ERROR_UNKNOWN";

   switch (iEvent)
   {
      case FeederRealizedEvent:
         szRC = "FeederRealizedEvent";
         break;
      case FeederPrefetchedEvent:
         szRC = "FeederPrefetchedEvent";
         break;
      case FeederRenderingEvent:
         szRC = "FeederRenderingEvent";
         break;
      case FeederStoppedEvent:
         szRC = "FeederStoppedEvent";
         break;
      case FeederFailedEvent:
         szRC = "FeederFailedEvent";
         break;
      case FeederStreamPlayingEvent:
         szRC = "FeederStreamPlayingEvent";
         break;
      case FeederStreamPausedEvent:
         szRC = "FeederStreamPausedEvent";
         break;
      case FeederStreamAbortedEvent:
         szRC = "FeederStreamAbortedEvent";
         break;
      case FeederStreamStoppedEvent:
         szRC = "FeederStreamStoppedEvent";
         break;
      case FeederStreamDestroyedEvent:
         szRC = "FeederStreamDestroyedEvent";
         break;
   }
   return szRC;
}

#endif

/* ============================ FUNCTIONS ================================= */

