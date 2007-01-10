//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpStreamPlaylistPlayer_h_
#define _MpStreamPlaylistPlayer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpPlayer.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"
#include "os/OsBSem.h"
#include "os/OsEvent.h"
#include "os/OsDefs.h"
#include "os/OsServerTask.h"
#include "os/OsStatus.h"
#include "os/OsQueuedEvent.h"

// DEFINES
#define MAX_PLAYLIST_LENGTH      40    ///< Max number of play list entries
#define REALIZE_TIMEOUT          15    ///< Timeout after 15 seconds
#define PREFETCH_TIMEOUT         30    ///< Timeout after 30 seconds
#define PLAY_TIMEOUT            180    ///< Timeout after 180 seconds
#define REWIND_TIMEOUT           15    ///< Timeout after 15 seconds
#define STOP_TIMEOUT             15    ///< Timeout after 15 seconds
#define DESTROY_TIMEOUT          30    ///< Timeout after 30 seconds

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

class MpStreamPlaylistPlayer : public OsServerTask, public MpPlayer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      SourceUrl,
      SourceBuffer
   } SourceType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor accepting a flow graph
   MpStreamPlaylistPlayer(OsMsgQ* pMsgQ, const char* pTarget = NULL);


     /// Destructor
   virtual
   ~MpStreamPlaylistPlayer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Adds a url to the playlist 
   virtual OsStatus add( Url& url  ///< Url identifying the source data stream
                       , int flags ///< Playing flags (see StreamDefs.h)
                       );


     /// Adds a buffer to the playlist 
   virtual OsStatus add(UtlString* pBuffer, int flags);
     /**<
     *  @param pBuffer - Net Buffer containing buffered audio data.  The 
     *         MpStreamPlayer resource will delete the pBuffer upon destruction
     *         of itself.
     *  @param flags - Playing flags (see StreamDefs.h)
     */


     /// @brief Realizes the player by initiating a connection to the target,
     /// allocates buffers, etc.
   virtual OsStatus realize(UtlBoolean bBlock = TRUE); 
     /**<
     *  @param bBlock - TRUE if the method should block until completion, 
     *         otherwise FALSE.
     */

     /// Prefetch enough of the data source to ensure a smooth playback.
   virtual OsStatus prefetch(UtlBoolean bBlock = TRUE); 
     /**<
     *  @param bBlock - TRUE if the method should block until completion,
     *         otherwise FALSE.
     */

     /// @brief Plays the media stream.  This will play all play lists from start
     /// to finish.
   virtual OsStatus play(UtlBoolean bBlock = TRUE);
     /**<
     *  @param bBlock - TRUE if the method should block until completion,
     *         otherwise FALSE.
     */

     /// @brief Waits for the media stream(s) to finish playing.  This will block
     /// the caller until all play lists have finished or rTimeout is reached.
   virtual OsStatus wait(const OsTime& rTimeout = OsTime::OS_INFINITY);
     /**<
     *  @param rTimeout - Optional timeout.  If not specified, wait forever.
     */

     /// @brief Rewinds a previously played media stream.  In some cases this
     ///  may result in a re-connect/refetch.
   virtual OsStatus rewind(UtlBoolean bBlock = TRUE);
     /**<
     *  @param bBlock - TRUE if the method should block until completion,
     *         otherwise FALSE.
     */

     /// @brief Resets the playlist player state by stopping and removing all 
     /// entries.
   virtual OsStatus reset() ;

     /// @brief Stops play the media stream and resources used for buffering
     /// and streaming.
   virtual OsStatus stop() ;

     /// @briefMarks the player as destroy and frees all allocated resources
     ///  in media processing.
   virtual OsStatus destroy() ;

     /// Pauses the media stream temporarily.  
   virtual OsStatus pause() ;


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Gets the number of play list entries
   virtual OsStatus getCount(int& count) const ;

     /// Gets the source type for playlist entry 'index'.
   virtual OsStatus getSourceType(int index, int& type) const ;

     /// Gets the source url for playlist entry 'index'.
   virtual OsStatus getSourceUrl(int index, Url url) const ;

     /// Gets the source buffer for playlist entry 'index'.
   virtual OsStatus getSourceBuffer(int index, UtlString*& netBuffer) const ;

     /// Gets the state for the playlist entry 'index'.
   virtual OsStatus getSourceState(int index, PlayerState& state) const ;

     /// @brief Gets the current playing index if playing or the next index to
     /// play if playNext() was invoked.
   virtual OsStatus getCurrentIndex(int& iIndex) const ;

     /// Gets the aggregate playerlist player state 
   virtual OsStatus getState(PlayerState& state)  ;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Copy constructor
   MpStreamPlaylistPlayer(const MpStreamPlaylistPlayer& rMpStreamPlaylistPlayer);

     /// Assignment operator
   MpStreamPlaylistPlayer& operator=(const MpStreamPlaylistPlayer& rhs);

     /// Selects the first playlist entry as the next index to play.
   virtual OsStatus first() ;
     /**<
     *  @note If an entry was playing, it will be stopped
     */

     /// Selects the last playlist entry as the next index to play.
   virtual OsStatus last() ;   
     /**<
     *  @note If an entry was playing, it will be stopped
     */

     /// Plays the next playlist entry without wrapping.
   virtual OsStatus playNext(UtlBoolean bBlock = TRUE) ;

     /// Plays the previous playlist entry without wrapping.
   virtual OsStatus playPrevious(UtlBoolean bBlock = TRUE) ;

     /// Sets the state for a specific entry.
   void setEntryState(int index, PlayerState iState) ;

     /// Starts playing a specific entry
   OsStatus playEntry(int iEntry, UtlBoolean bBlock = TRUE);

     /// Rewinds a specific entry
   OsStatus rewindEntry(int iEntry, UtlBoolean bBlock = TRUE);

     /// Stops playing a specific entry
   OsStatus stopEntry(int index, UtlBoolean bBlock = TRUE);

     /// Pauses a specific entry
   OsStatus pauseEntry(int index);

     /// Destroys a specific entry
   OsStatus destroyEntry(int index, UtlBoolean bBlockAndClean = TRUE);

     /// Handle messages directed to this server task.
   virtual UtlBoolean handleMessage(OsMsg& rMsg) ;

     /// Handles processing for the realized state
   void handleRealizedState(int index, PlayerState oldState, PlayerState newState);

     /// Handles processing for the prefetched state
   void handlePrefetchedState(int index, PlayerState oldState, PlayerState newState);

     /// Handles processing for the playing state
   void handlePlayingState(int index, PlayerState oldState, PlayerState newState);

     /// Handles processing for the paused state
   void handlePausedState(int index, PlayerState oldState, PlayerState newState);

     /// Handles processing for the stopped state
   void handleStoppedState(int index, PlayerState oldState, PlayerState newState);

     /// Handles processing for the failed state
   void handleFailedState(int index, PlayerState oldState, PlayerState newState);

     /// Handles converting Feeder events into human readable form
   const char* getFeederEventString(int iEvent);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Definition for a playlist entry
   struct PlaylistDb
   {
      int            sourceType ;   ///< Source type (url or buffer)
      Url            url ;          ///< url if source type url
      UtlString*     pBuffer ;      ///< buffer if source type buffer
      StreamHandle   handle ;       ///< handle of the feeder
      PlayerState    state ;        ///< state of the entry
      int            flags ;        ///< flags for the entry
      OsQueuedEvent* pQueuedEvent ; ///< queued event for notifications
   }  ;


   int mNumPlayListElements;        ///< number of play list entries
   int mCurrentElement ;            ///< next item to  play
   int mPlayingElement ;            ///< current playing item

   OsQueuedEvent* mpQueueEvent;     ///< event for notifications
   OsBSem mSemStateChange;          ///< used to block for state changes
   OsMsgQ* mpMsgQ;                  ///< MsgQ to send commands
   UtlString mTarget;               ///< target used for MsgQ receive to help dispatch
   OsEvent mWaitEvent;              ///< used to block until player completes
   OsTime mRealizeTimeout;          ///< Timeout for Realize operation
   OsTime mPrefetchTimeout;         ///< Timeout for Prefetch operation
   OsTime mPlayTimeout;             ///< Timeout for Play operation
   OsTime mRewindTimeout;           ///< Timeout for Rewind operation
   OsTime mStopTimeout;             ///< Timeout for Stop operation
   OsTime mDestroyTimeout;          ///< Timeout for Destroy operation

   UtlBoolean mbAutoAdvance;        ///< used to play playlist lists

   struct PlaylistDb mPlayListDb[MAX_PLAYLIST_LENGTH];   ///< db of entries
   PlayerState mAggregateState ;    ///< Aggregate state of the player
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStreamPlaylistPlayer_h_
