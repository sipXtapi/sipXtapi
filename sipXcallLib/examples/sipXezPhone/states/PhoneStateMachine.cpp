//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "PhoneStateMachine.h"
#include "PhoneStateIdle.h"
#include "utl/UtlSListIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
PhoneStateMachine* PhoneStateMachine::spStateMachine = NULL;
// MACRO CALLS


PhoneStateMachine::PhoneStateMachine(void) :
        mpState(NULL)
{

   mpState = new PhoneStateIdle(); // start the state machine off in the IDLE state
}

PhoneStateMachine::~PhoneStateMachine(void)
{
}

PhoneStateMachine& PhoneStateMachine::getInstance()
{
   if (NULL == spStateMachine)
   {
      spStateMachine = new PhoneStateMachine;
   }
   return *spStateMachine;
}

PhoneState* PhoneStateMachine::getState() const
{
        return mpState;
}

void PhoneStateMachine::setState(const PhoneState* pState)
{
        if (mpState && mpState != pState)
        {
           if (pState)
           {
               // delete old state
                delete mpState;
                // transition to new state
                mpState = (PhoneState*) pState;
                // execute it
                mpState->Execute();
           }
        }
}

PhoneState* PhoneStateMachine::OnDial(wxString phoneNumber)
{
   if (mpState)
   {
      // notify listeners that a Dial transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnDial(phoneNumber);
      }

      // set the state
      setState(mpState->OnDial(phoneNumber));
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnConnected()
{
   if (mpState)
   {
      // notify listeners that a Connected transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnConnected();
      }

      // set the state
      setState(mpState->OnConnected());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnDisconnected(const SIPX_CALL hCall)
{
   if (mpState)
   {
      // notify listeners that a Disconnected transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnDisconnected(hCall);
      }

      // set the state
      setState(mpState->OnDisconnected(hCall));
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnFlashButton()
{
   if (mpState)
   {
      // notify listeners that a FlashButton transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnFlashButton();
      }

      // set the state
      setState(mpState->OnFlashButton());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnRemoteAlerting()
{
   if (mpState)
   {
      // notify listeners that a RemoteAlerting transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnRemoteAlerting();
      }

      // set the state
      setState(mpState->OnRemoteAlerting());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnRemoteBusy()
{
   if (mpState)
   {
      // notify listeners that a RemoteBusy transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnRemoteBusy();
      }

      // set the state      setState(mpState->OnRemoteBusy());
      setState(mpState->OnRemoteBusy());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnRinging(SIPX_CALL hCall)
{
   if (mpState)
   {
      // notify listeners that a Ringing transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnRinging(hCall);
      }

      // set the state
      setState(mpState->OnRinging(hCall));
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnOutgoingCallRejected()
{
   if (mpState)
   {
      // notify listeners that a OutgoingCallRejected transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnOutgoingCallRejected();
      }

      // set the state
      setState(mpState->OnOutgoingCallRejected());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnError()
{
   if (mpState)
   {
      // notify listeners that a Error transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnError();
      }

      // set the state
      setState(mpState->OnError());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnTransferRequested(const wxString phoneNumber)
{
   if (mpState)
   {
      // notify listeners that a HoldButton transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnTransferRequested(phoneNumber);
      }

      // set the state
      setState(mpState->OnTransferRequested(phoneNumber));
   }
    return NULL; // the state machine should only return state pointers from the getState methods
}

PhoneState* PhoneStateMachine::OnTransferComplete(const bool bFailure)
{
   if (mpState)
   {
      // notify listeners that a HoldButton transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnTransferComplete(bFailure);
      }

      // set the state
      setState(mpState->OnTransferComplete(bFailure));
   }
    return NULL; // the state machine should only return state pointers from the getState methods
}

PhoneState* PhoneStateMachine::OnOffer(SIPX_CALL hCall)
{
   if (mpState)
   {
      // notify listeners that an Offer transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnOffer(hCall);
      }

      // set the state
      setState(mpState->OnOffer(hCall));
   }
    return NULL; // the state machine should only return state pointers from the getState methods
}

PhoneState* PhoneStateMachine::OnHoldButton()
{
   if (mpState)
   {
      // notify listeners that a OutgoingCallRejected transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnHoldButton();
      }

      // set the state
      setState(mpState->OnHoldButton());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

PhoneState* PhoneStateMachine::OnLocalHoldRequested()
{
   if (mpState)
   {
      // notify listeners that a OnLocalHoldRequested transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnLocalHoldRequested();
      }

      // set the state
      setState(mpState->OnLocalHoldRequested());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}


PhoneState* PhoneStateMachine::OnConnectedInactive()
{
   if (mpState)
   {
      // notify listeners that a OnConnectedInactive transition is about to occur
      UtlSListIterator iterator(mObservers);
      PhoneStateMachineObserver* pObserver;
      while (pObserver = (PhoneStateMachineObserver*)iterator())
      {
          pObserver->OnConnectedInactive();
      }

      // set the state
      setState(mpState->OnConnectedInactive());
   }
   return NULL; // the state machine should only return state pointers from the getState method
}

void PhoneStateMachine::addObserver(const PhoneStateMachineObserver* pObserver)
{
    // inspect the list of observers, and make sure its not there already
    if (mObservers.containsReference(pObserver))
    {
        return;
    }
    mObservers.insert((UtlContainable*)pObserver);
}

void PhoneStateMachine::removeObserver(const PhoneStateMachineObserver* pObserver)
{
    mObservers.removeReference((UtlContainable*) pObserver);
    return;
}
