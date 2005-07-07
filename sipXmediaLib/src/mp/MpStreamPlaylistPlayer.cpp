// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpStreamPlaylistPlayer.h"
#include "mp/MpStreamMsg.h"
#include "os/OsEventMsg.h"
#include "os/OsEvent.h"
#include "os/OsQueuedEvent.h"
#include "os/OsMsgQ.h"

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
{
   mpMsgQ = pMsgQ ;
   if (pTarget != NULL)
      mTarget = pTarget ;
   mpQueueEvent = NULL ;

   for (int i=0; i<MAX_PLAYLIST_LENGTH; i++)
   {
      mPlayListDb[i].sourceType = 0 ;
      mPlayListDb[i].pBuffer = NULL ;
      mPlayListDb[i].handle = NULL ;
      mPlayListDb[i].state = PlayerUnrealized ;
      mPlayListDb[i].flags = 0 ;
      mPlayListDb[i].pQueuedEvent = NULL ;
   }

   mNumPlayListElements = 0 ;
   mCurrentElement = 0 ;
   mPlayingElement = -1 ;
   mbAutoAdvance = FALSE ;
   mAggregateState = PlayerUnrealized ;

   start() ;

   // Wait for the stream player to startup.
   while (!isStarted())
   {
      OsTask::yield() ;
   }
}


// Destructor
MpStreamPlaylistPlayer::~MpStreamPlaylistPlayer()
{   
   reset() ;

   if (mpQueueEvent != NULL)
   {
      delete mpQueueEvent ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Adds a url to the playlist 
OsStatus MpStreamPlaylistPlayer::add(Url& url, int flags)
{
   OsStatus status = OS_LIMIT_REACHED ;

   if (mNumPlayListElements < MAX_PLAYLIST_LENGTH)
   {
      int index = mNumPlayListElements ;
      mNumPlayListElements++ ;

      mPlayListDb[index].sourceType = SourceUrl ;
      mPlayListDb[index].url = url ;
      mPlayListDb[index].flags = flags;       
      mPlayListDb[index].pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), index) ;

      status = OS_SUCCESS ;
   }

   return status ;
}


// Adds a buffer to the playlist  
OsStatus MpStreamPlaylistPlayer::add(UtlString* pBuffer, int flags)
{
   OsStatus status = OS_LIMIT_REACHED ;

   if (mNumPlayListElements < MAX_PLAYLIST_LENGTH)
   {
      int index = mNumPlayListElements ;
      mNumPlayListElements++ ;

      mPlayListDb[index].sourceType = SourceBuffer ;
      mPlayListDb[index].pBuffer = pBuffer ;
      mPlayListDb[index].flags = flags;       
      mPlayListDb[index].pQueuedEvent = new OsQueuedEvent(*getMessageQueue(), index) ;

      status = OS_SUCCESS ;
   }
   return status ;   
}


// Realizes the player by initiating a connection to the target, allocates 
// buffers, etc.
OsStatus MpStreamPlaylistPlayer::realize(UtlBoolean bBlock)
{
   OsStatus status = OS_FAILED ;
   int i ;

   // Start prefetching all of the elements
   for (i=0; i<mNumPlayListElements; i++)
   {         
      if (mPlayListDb[i].state == PlayerUnrealized)
      {
         OsEvent eventHandle ;

         // Realize the stream
         if (mPlayListDb[i].sourceType == SourceUrl)
         {            
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_URL, mTarget, NULL, &eventHandle, mPlayListDb[i].pQueuedEvent, mPlayListDb[i].flags, (int) new Url(mPlayListDb[i].url)) ;                   
            status = mpMsgQ->send(msg) ;
            // assert(status == OS_SUCCESS);
         }
         else if (mPlayListDb[i].sourceType == SourceBuffer)
         {
            MpStreamMsg msg(MpStreamMsg::STREAM_REALIZE_URL, mTarget, NULL, &eventHandle, mPlayListDb[i].pQueuedEvent, mPlayListDb[i].flags, (int) mPlayListDb[i].pBuffer) ;                            
            status = mpMsgQ->send(msg) ;
            // assert(status == OS_SUCCESS);
         }

         if (status == OS_SUCCESS)
         {
            // Wait for a response
            int eventData ;
            eventHandle.wait() ;            
            if (eventHandle.getEventData(eventData) == OS_SUCCESS)
            {
               mPlayListDb[i].handle = (StreamHandle) eventData ;
            }
            else
            {
               setEntryState(i, PlayerFailed) ;
               mPlayListDb[i].handle = NULL ;   
            }
         }
      }

      // Block if requested
      if ((status == OS_SUCCESS) && bBlock)
      {
         for (i=0; i<mNumPlayListElements; i++)
         {                        
            while (mPlayListDb[i].state == PlayerUnrealized)
            {
               mSemStateChange.acquire();
            }
         }
      }
   }
   return status ;
}
     

// Prefetch enough of the data source to ensure a smooth playback.
OsStatus MpStreamPlaylistPlayer::prefetch(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_FAILED ;
   int i ;

   // Start prefetching all of the elements
   for (i=0; i<mNumPlayListElements; i++)
   {         
      if (mPlayListDb[i].state == PlayerRealized)
      {

         MpStreamMsg msg(MpStreamMsg::STREAM_PREFETCH, mTarget, mPlayListDb[i].handle);
         status = mpMsgQ->send(msg) ;         
         if (status != OS_SUCCESS)
         {
            setEntryState(i, PlayerFailed) ;
         }

         while (mPlayListDb[i].state == PlayerRealized)
         {
            mSemStateChange.acquire();
         }

      }
   }

   // Block if requested
   if ((status == OS_SUCCESS)  && bBlock)
   {
      for (i=0; i<mNumPlayListElements; i++)
      {                        
         while (mPlayListDb[i].state == PlayerRealized)
         {
            mSemStateChange.acquire();
         }
      }
   }

   return OS_SUCCESS ;
}



// Plays the media stream.  This will play all play lists from start to 
// finish.
OsStatus MpStreamPlaylistPlayer::play(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus rc = OS_INVALID ;

   if (bBlock) 
   {      
      first() ;
      do
      {
         mbAutoAdvance = FALSE ;         
         rc = playNext() ; 
      }
      while ((rc == OS_SUCCESS) && (mCurrentElement < mNumPlayListElements));      
   }
   else
   {
      mbAutoAdvance = TRUE ;
      if ((mAggregateState == PlayerPaused) && (mPlayingElement != -1))
      {
         rc = playEntry(mPlayingElement, bBlock) ;
      }
      else
         rc = playNext(FALSE) ;
   }

   return rc ;
}


// Plays the media stream.  This will play all play lists from start to 
// finish.
OsStatus MpStreamPlaylistPlayer::rewind(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_SUCCESS ;

   stop() ;
   mAggregateState = PlayerUnrealized ;

   // Start prefetching all of the elements
   for (int i=0; i<mNumPlayListElements; i++)
   {
      rewindEntry(i, bBlock) ;
   }

   return status ;
}


// Resets the playlist player state by stopping and removing all entries.
OsStatus MpStreamPlaylistPlayer::reset()
{
   OsStatus status = OS_SUCCESS ;

   for (int i=0; i<mNumPlayListElements; i++)
   {
      destroyEntry(i) ;
   }

   mNumPlayListElements = 0 ;
   mCurrentElement = 0 ;
   mPlayingElement = -1 ;
   mbAutoAdvance = FALSE ;
   mAggregateState = PlayerUnrealized ;

   return status ;
}



// Stops play the media stream and resources used for buffering and streaming.
OsStatus MpStreamPlaylistPlayer::stop()
{
   OsStatus status = OS_SUCCESS ;
   mbAutoAdvance = FALSE ;

   mCurrentElement = mNumPlayListElements ;
   for (int i=0; i<mNumPlayListElements; i++)
   {
      stopEntry(i) ;      
   }

   return status ;
}



// Marks the player as destroy and frees all allocated resources in media 
// processing.
OsStatus MpStreamPlaylistPlayer::destroy()
{
   OsStatus status = OS_SUCCESS ;   // This is non-blocking, assume success.
 
   mCurrentElement = mNumPlayListElements ;
   for (int i=0; i<mNumPlayListElements; i++)
   {
      destroyEntry(i, false) ;      
   }

   return status ;
}


// Pauses the media stream temporarily.  
OsStatus MpStreamPlaylistPlayer::pause()
{
   OsStatus status = OS_FAILED ;

   int iPlayingElement = mPlayingElement ;
   if (iPlayingElement != -1)
   {
      status = pauseEntry(iPlayingElement) ;
   }

   return status ;
}


/* ============================ ACCESSORS ================================= */

// Gets the number of play list entries
OsStatus MpStreamPlaylistPlayer::getCount(int& count) const
{
   count = mNumPlayListElements ;
   return OS_SUCCESS ;
}


// Gets the source type for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceType(int index, 
                                               int& type) const
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      type = mPlayListDb[index].sourceType ;
      status = OS_SUCCESS ;
   }

   return status ;
}


// Gets the source url for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceUrl(int index, Url url) const
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      if (mPlayListDb[index].sourceType == SourceUrl)
      {
         url = mPlayListDb[index].url ;
         status = OS_SUCCESS ;
      }
   }

   return status ;
}


// Gets the source buffer for playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceBuffer(int index, 
                                                 UtlString*& netBuffer) const
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      if (mPlayListDb[index].sourceType == SourceBuffer)
      {
         netBuffer = mPlayListDb[index].pBuffer ;
         status = OS_SUCCESS ;
      }
   }

   return status ;
}

// Gets the state for the playlist entry 'index'.
OsStatus MpStreamPlaylistPlayer::getSourceState(int index, 
                                                PlayerState& state) const

{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      state = mPlayListDb[index].state ;
      status = OS_SUCCESS ;
   }

   return status ;
}

// Gets the current playing index if playing or the next index to play if 
// playNext() was invoked.
OsStatus MpStreamPlaylistPlayer::getCurrentIndex(int& index) const
{
   OsStatus status = OS_SUCCESS ;

   if (mPlayingElement == -1)
      index = mCurrentElement ;
   else
      index = mPlayingElement ;

   return status ;
}

// Gets the aggregate playerlist player state 
OsStatus MpStreamPlaylistPlayer::getState(PlayerState& state) 
{
   state = mAggregateState ;
   
   return OS_SUCCESS ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Copy constructor
MpStreamPlaylistPlayer::MpStreamPlaylistPlayer(const MpStreamPlaylistPlayer& rMpStreamPlaylistPlayer)
   : mSemStateChange(OsBSem::Q_PRIORITY, OsBSem::EMPTY)

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
      stop() ;

   mCurrentElement = 0 ;

   return OS_SUCCESS ;
}

// Selects the last playlist entry as the next index to play.
OsStatus MpStreamPlaylistPlayer::last()
{
   stop() ;

   if (mNumPlayListElements > 0)
      mCurrentElement = mNumPlayListElements - 1 ;
   else
      mCurrentElement = 0 ;

   return OS_SUCCESS ;
}


// Plays the next playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playNext(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED ;

   if (mCurrentElement < mNumPlayListElements)
   {
      int iPlayElement = mCurrentElement++ ;
      status = playEntry(iPlayElement, bBlock) ;
   }

   return status ;
}


// Plays the previous playlist entry without wrapping.
OsStatus MpStreamPlaylistPlayer::playPrevious(UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_LIMIT_REACHED ;

   if (mCurrentElement > 0)
   {  
      mCurrentElement-- ;
      status = playEntry(mCurrentElement, bBlock) ;
      mCurrentElement++ ;
   }

   return status ;
}


// Sets the state for a specific entry.
void MpStreamPlaylistPlayer::setEntryState(int index, PlayerState newState)
{
   PlayerState oldState = mPlayListDb[index].state ;

   if (oldState != newState)
   {
      // Store the new state
      mPlayListDb[index].state = newState ;

      // Updated aggregate state given the new entry state
      switch (newState)
      {
         case PlayerUnrealized:            
            break ;
         case PlayerRealized:
            handleRealizedState(index, oldState, newState) ;
            break ;
         case PlayerPrefetched:
            handlePrefetchedState(index, oldState, newState) ;
            break ;
         case PlayerPlaying:
            handlePlayingState(index, oldState, newState) ;
            break ;
         case PlayerPaused:
            handlePausedState(index, oldState, newState) ;
            break ;
         case PlayerStopped:
         case PlayerAborted:
            handleStoppedState(index, oldState, newState) ;
            break ;
         case PlayerFailed:
            handleFailedState(index, oldState, newState) ;
            break ;
      }
      
      // Update any blocking calls
      mSemStateChange.release() ;
   }
}


// Starts playing a specific entry
OsStatus MpStreamPlaylistPlayer::playEntry(int iEntry, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {      
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (mPlayListDb[iEntry].state == PlayerRealized) || 
            (mPlayListDb[iEntry].state == PlayerPaused) ||
            (mPlayListDb[iEntry].state == PlayerPrefetched))
      {
         mPlayingElement = iEntry ;         
         MpStreamMsg msg(MpStreamMsg::STREAM_PLAY, mTarget, mPlayListDb[iEntry].handle);
         status = mpMsgQ->send(msg) ;
         if (status != OS_SUCCESS)
         {
            setEntryState(iEntry, PlayerFailed) ;
         }
         else 
         {
            if (mPlayListDb[iEntry].state == PlayerPaused)
               setEntryState(iEntry, PlayerPlaying) ;

            // Block while waiting for play to complete (if requested)
            if ((status == OS_SUCCESS)  && bBlock)
            {
               while (  (mPlayListDb[iEntry].state == PlayerRealized)
                     || (mPlayListDb[iEntry].state == PlayerPrefetched) 
                     || (mPlayListDb[iEntry].state == PlayerPlaying) 
                     || (mPlayListDb[iEntry].state == PlayerPaused))
               {
                  mSemStateChange.acquire();
               }
            }
         }
      }
      else
      {
         status = OS_INVALID_STATE ;
      }      
   }
   return status ;
}


// Rewinds a previously played media stream.  In some cases this may result
// in a re-connect/refetch.
OsStatus MpStreamPlaylistPlayer::rewindEntry(int iEntry, UtlBoolean bBlock /*= TRUE*/)
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((iEntry >= 0) && (iEntry < mNumPlayListElements))
   {      
      if (mPlayListDb[iEntry].state == PlayerPrefetched)
      {
         status = OS_SUCCESS ;
      }
      else
      {         
         // Only proceed if we have a flow graph and the player is realized.
         // NOTE: The player doesn't need to be prefetched
         if ((mPlayListDb[iEntry].state == PlayerStopped) || (mPlayListDb[iEntry].state == PlayerAborted))
         {         
            MpStreamMsg msg(MpStreamMsg::STREAM_REWIND, mTarget, mPlayListDb[iEntry].handle);
            status = mpMsgQ->send(msg) ;
            if (status != OS_SUCCESS)
            {
               setEntryState(iEntry, PlayerFailed) ;
            }
            else 
            {
               // Block while waiting for play to complete (if requested)
               if ((status == OS_SUCCESS)  && bBlock)
               {
                  while ((mPlayListDb[iEntry].state == PlayerStopped) || 
                        (mPlayListDb[iEntry].state == PlayerAborted))
                  {
                     mSemStateChange.acquire();
                  }
               }
            }
         }
         else
         {
            status = OS_INVALID_STATE ;
         }
      }
   }
   return status ;
}


// Stops playing a specific entry
OsStatus MpStreamPlaylistPlayer::stopEntry(int index, UtlBoolean bBlock /* = TRUE */)
{
   OsStatus status = OS_INVALID_ARGUMENT ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {      
      // Only proceed if we have a flow graph and the player is realized.
      // NOTE: The player doesn't need to be prefetched
      if (  (mPlayListDb[index].state == PlayerPrefetched) || 
            (mPlayListDb[index].state == PlayerPlaying) || 
            (mPlayListDb[index].state == PlayerPaused))
      {        
         MpStreamMsg msg(MpStreamMsg::STREAM_STOP, mTarget, mPlayListDb[index].handle);
         status = mpMsgQ->send(msg) ;         
         if ((status == OS_SUCCESS) && bBlock)
         {
            while ((mPlayListDb[index].state == PlayerStopped) || 
                  (mPlayListDb[index].state == PlayerAborted))
                
            {
               mSemStateChange.acquire();
            }
         }
      }
      else if ((mPlayListDb[index].state == PlayerStopped) || 
            (mPlayListDb[index].state == PlayerAborted))
         status = OS_SUCCESS ;
   }
   return status ;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::pauseEntry(int index)
{
   OsStatus status = OS_INVALID_ARGUMENT   ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if ( (mPlayListDb[index].state == PlayerPrefetched) ||
           (mPlayListDb[index].state == PlayerPlaying))
      {
         MpStreamMsg msg(MpStreamMsg::STREAM_PAUSE, mTarget, mPlayListDb[index].handle);
         status = mpMsgQ->send(msg) ;         
      }
   }

   return status ;
}


// Pauses a specific entry
OsStatus MpStreamPlaylistPlayer::destroyEntry(int index, UtlBoolean bBlockAndClean)
{
   OsStatus status = OS_INVALID_ARGUMENT   ;

   if ((index >= 0) && (index < mNumPlayListElements))
   {
      // Only proceed if we have a flow graph and the player is prefetched or
      // playing.
      if (mPlayListDb[index].state != PlayerUnrealized)
      {
         int iState = mPlayListDb[index].state ;
         
         if (  (iState != PlayerStopped) && 
               (iState != PlayerAborted) && 
               (iState != PlayerDestroyed)  )
         {
            MpStreamMsg msgStop(MpStreamMsg::STREAM_STOP, mTarget, mPlayListDb[index].handle);
            status = mpMsgQ->send(msgStop) ;
         }

         if ((iState != PlayerDestroyed) && (status == OS_SUCCESS))
         {
            MpStreamMsg msgDestroy(MpStreamMsg::STREAM_DESTROY, mTarget, mPlayListDb[index].handle);
            status = mpMsgQ->send(msgDestroy) ;
         }         

         
         if (bBlockAndClean)
         {
            while (mPlayListDb[index].state != PlayerDestroyed)
            {
               mSemStateChange.acquire();
            }         

            mPlayListDb[index].sourceType = 0 ;
            mPlayListDb[index].pBuffer = NULL ;
            mPlayListDb[index].handle = NULL ;
            mPlayListDb[index].state = PlayerUnrealized ;
            mPlayListDb[index].flags = 0 ;
            if (mPlayListDb[index].pQueuedEvent != NULL)
            {
               delete mPlayListDb[index].pQueuedEvent ;
               mPlayListDb[index].pQueuedEvent = NULL ;   
            }
         }
      }
   }

   return status ;
}


// Handle messages directed to this server task.
UtlBoolean MpStreamPlaylistPlayer::handleMessage(OsMsg& rMsg) 
{
   switch (rMsg.getMsgType())
   {
      case OsMsg::OS_EVENT:
         OsEventMsg* pMsg = (OsEventMsg*) &rMsg ;
         int status ;
         PlayerState oldState ;         
         int index ;

         pMsg->getUserData(index);
         if (pMsg->getEventData(status) == OS_SUCCESS)
         {
            getSourceState(index, oldState) ;
            switch (status)
            {
               case FeederRealizedEvent:
                  setEntryState(index, PlayerRealized) ;                  
                  break ;

               case FeederPrefetchedEvent:
                  setEntryState(index, PlayerPrefetched) ;
                  break;

               case FeederStoppedEvent:
                  if (mAggregateState != PlayerPlaying)
                  {
                     setEntryState(index, PlayerPrefetched) ;
                  }
                  break ;

               case FeederRenderingEvent:
                  break ;

               case FeederFailedEvent:                     
                  setEntryState(index, PlayerFailed) ;
                  break ;

               case FeederStreamPlayingEvent:
                  setEntryState(index, PlayerPlaying) ;
                  break ;

               case FeederStreamPausedEvent:
                  setEntryState(index, PlayerPaused) ;
                  break ;               
               case FeederStreamStoppedEvent:
                  setEntryState(index, PlayerStopped) ;
                  break ;
               
               case FeederStreamDestroyedEvent:
                  setEntryState(index, PlayerDestroyed) ;
                  break ;

            }
         }
         break ;
   }

   return TRUE ;
}


// Handles processing for the realized state
void MpStreamPlaylistPlayer::handleRealizedState(int index, PlayerState oldState, PlayerState newState)
{
   //
   // Updated the mAggregateState if all play list items are realized
   //
   if (mAggregateState == PlayerUnrealized)
   {
      UtlBoolean bAnyUnRealized = FALSE ;
      for (int i=0;i<mNumPlayListElements; i++)
      {
         if (mPlayListDb[i].state == PlayerUnrealized)
         {
            bAnyUnRealized = TRUE ;
            break ;
         }
      }

      // If everything is PlayerRealized or better, then updated the aggregate
      // state and fire off the event to listeners
      if (!bAnyUnRealized)
      {
         mAggregateState = PlayerRealized ;
         fireEvent(PlayerRealized) ;
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
      UtlBoolean bAllPrefetched = TRUE ;
      for (int i=0;i<mNumPlayListElements; i++)
      {
         if (  (mPlayListDb[i].state != PlayerPrefetched) && 
               (mPlayListDb[i].state != PlayerFailed))
         {
            bAllPrefetched = FALSE ;
            break ;
         }
      }

      // If everything is prefetched, then updated the aggregate state and
      // fire off the event to listeners
      if (bAllPrefetched)
      {
         mAggregateState = PlayerPrefetched ;
         fireEvent(PlayerPrefetched) ;
      }
   }
}


// Handles processing for the playing state
void MpStreamPlaylistPlayer::handlePlayingState(int index, PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPlaying)
   {
      mAggregateState = PlayerPlaying ;
      fireEvent(PlayerPlaying) ;
   }
}


// Handles processing for the paused state
void MpStreamPlaylistPlayer::handlePausedState(int index, PlayerState oldState, PlayerState newState)
{
   if (mAggregateState != PlayerPaused)
   {
     mAggregateState = PlayerPaused ;
      fireEvent(PlayerPaused) ;
   }
}


// Handles processing for the stopped state
void MpStreamPlaylistPlayer::handleStoppedState(int index, PlayerState oldState, PlayerState newState)
{
   if (  (mbAutoAdvance) && 
         (mCurrentElement < mNumPlayListElements) && 
         (newState != PlayerAborted))
   {
      playNext(FALSE) ;
   }
   else if (newState == PlayerAborted)
   {
      if (mAggregateState != PlayerAborted)
      {
         mAggregateState = PlayerAborted ;
         fireEvent(PlayerAborted) ;
      }
   }
   else
   {
      if (mCurrentElement >= mNumPlayListElements)
      {
         mbAutoAdvance = FALSE ;

         if (mAggregateState != PlayerStopped)
         {
            mAggregateState = PlayerStopped ;
            fireEvent(PlayerStopped) ;
         }
      }
   }
}


// Handles processing for the failed state
void MpStreamPlaylistPlayer::handleFailedState(int index, PlayerState oldState, PlayerState newState)
{

}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

