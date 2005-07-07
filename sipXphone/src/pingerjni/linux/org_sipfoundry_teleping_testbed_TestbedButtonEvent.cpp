// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/linux/com_pingtel_teleping_testbed_TestbedButtonEvent.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <jni.h>
#include "os/OsDefs.h"

#include "ptapi/PtPhoneButton.h"
#include "pingerjni/debug_jni.h"
#include "ps/PsMsg.h"
#include "ps/PsPhoneTask.h"
#include "ps/PsButtonTask.h"
#include "ps/PsButtonId.h"
#include "ps/PsButtonInfo.h"

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneButton
 * Method:    JNI_buttonPress
 * Signature: (J)V
 *
 *	This method takes any softphone button event, translates it
 *  and posts it to psPhoneTask using the postMessage method.
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedButtonEvent_JNI_1postButtonEvent
  (JNIEnv *, jclass, jint iButtonId, jint iEventType)
{

	enum { J_KEY_UP = 4, J_KEY_DOWN = 3, J_BUTTON_UP = 2, J_BUTTON_DOWN = 1};
	PsMsg        msg(PsMsg::BUTTON_DOWN, NULL, 0, 0); //the message we are going to create
	PsPhoneTask* pPhoneTask;
	OsStatus     res;
	int iLookupId = -1;
	int iParam1 = -1;
	bool bGotString = false; //used by NT keys to tell it not to convert the number to a string
	char buf[80];

    pPhoneTask = PsPhoneTask::getPhoneTask();

	// Button up or down
	switch (iEventType)
    {
	    case J_BUTTON_UP	: msg.setMsgSubType(PsMsg::BUTTON_UP);		break;
		case J_BUTTON_DOWN: msg.setMsgSubType(PsMsg::BUTTON_DOWN);	break;
	    case J_KEY_UP	: msg.setMsgSubType(PsMsg::BUTTON_UP);		break;
		case J_KEY_DOWN: msg.setMsgSubType(PsMsg::BUTTON_DOWN);	break;
		//default:  osPrintf("PTPhoneButton: Incorrect button type\n"); break;
    }
	
	//if on Linux systems
//osPrintf("postButtonEvent: before checking if it's a Linux keybaord event ButtonID = %X, iEventType = %d\n", iButtonId, iEventType);
	

	if (iEventType == J_KEY_UP || iEventType == J_KEY_DOWN)
	{

//osPrintf("postButtonEvent: got Linux keyboard event ButtonID = %X\n", iButtonId);

		switch(iButtonId)
		{
			case 0x224:				//  HOME
//									osPrintf("postButtonEvent: got HOME from NT\n");
								iLookupId = FKEY_HOME;
								iParam1 = 1;
								bGotString = true;  //dont do it later
								msg.setStringParam1("HOME");
								iEventType = J_KEY_DOWN;
								msg.setMsgSubType(PsMsg::BUTTON_DOWN);
								break;	
			case 0x223:				//  END
//									osPrintf("postButtonEvent: got END from NT\n");
								iLookupId = FKEY_END;
								iParam1 = 1;
								bGotString = true;  //dont do it later
								msg.setStringParam1("END");
								iEventType = J_KEY_DOWN;
								msg.setMsgSubType(PsMsg::BUTTON_DOWN);
								break;	
			case 0x225:				// LEFT ARROW
//									osPrintf("postButtonEvent: got LEFT from NT\n");
								iParam1 = 24;
								iLookupId	= 	FKEY_SKEY_L2;
								bGotString = true;	//dont do it later
								msg.setStringParam1("SKEYL2");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x227:				//RIGHT ARROW
//									osPrintf("postButtonEvent: got RIGHT from NT\n");
								iParam1 = 25;
								iLookupId	= 	FKEY_SKEY_L3;
								bGotString = true;	//dont do it later
								msg.setStringParam1("SKEYL3");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x226		:	// UP ARROW
//									osPrintf("postButtonEvent: got SCROLL UP from NT\n");
								iParam1 = 513;
								iLookupId = FKEY_SCROLL_UP;
								bGotString = true;	//dont do it later
								msg.setStringParam1("SCROLL_UP");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x228		:	//DOWN ARROW
//									osPrintf("postButtonEvent: got SCROLL DOWN from NT\n");
								iParam1 = 514;
								iLookupId = FKEY_SCROLL_DOWN;		
								bGotString = true;	//dont do it later
								msg.setStringParam1("SCROLL_DOWN");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x270:			
//									osPrintf("postButtonEvent: got F1 from NT\n");
								iParam1 = 22;
								iLookupId	= 	FKEY_MORE;
								bGotString = true;	//dont do it later
								msg.setStringParam1("MORE");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x271:			
//osPrintf("postButtonEvent: got F2 from NT\n");
								iLookupId	= 	FKEY_HEADSET;
								bGotString = true;	//dont do it later
								msg.setStringParam1("HEADSET");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x272:			
//osPrintf("postButtonEvent: got F3 from NT\n");
								iLookupId	= 	FKEY_TRANSFER;
								bGotString = true;	//dont do it later
								msg.setStringParam1("TRANSFER");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x273:			
//osPrintf("postButtonEvent: got F4 from NT\n");
								iLookupId	= 	FKEY_MUTE;
								bGotString = true;	//dont do it later
								msg.setStringParam1("MUTE");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x274:			
//osPrintf("postButtonEvent: got F5 from NT\n");
								iLookupId	= 	FKEY_CONFERENCE;
								bGotString = true;	//dont do it later
								msg.setStringParam1("CONFERENCE");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x275:			
//osPrintf("postButtonEvent: got F6 from NT\n");
								iLookupId	= 	FKEY_SPEAKER;
								bGotString = true;	//dont do it later
								msg.setStringParam1("SPEAKER");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			case 0x276:			
//osPrintf("postButtonEvent: got F7 from NT\n");
								iLookupId	= 	FKEY_HOLD;
								bGotString = true;	//dont do it later
								msg.setStringParam1("HOLD");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;

			case 0x209:				//  TAB KEY
//osPrintf("postButtonEvent: got TAB from NT\n");
								iLookupId = FKEY_TAB;
								iParam1 = 1;
								iEventType = J_KEY_DOWN;
								bGotString = true;	//dont do it later
								msg.setStringParam1("TAB");
								break;

			case 0x1B:				//	ESC KEY
//osPrintf("postButtonEvent: got ESC from NT\n");
								iLookupId = FKEY_ESC;
								iParam1 = 1;
								bGotString = true;	//dont do it later
								msg.setStringParam1("ESC");
								break;
			case 0x208:				//  BACKSPACE
//osPrintf("postButtonEvent: got BACKSPACE from NT\n");
								iLookupId = FKEY_BACKSPACE;
								iParam1 = 1;
								bGotString = true;  //dont do it later
								msg.setStringParam1("BACKSPACE");
								break;	

			case 0x20A		:	//ENTER key on NT keyboard maps to B3 key
//osPrintf("postButtonEvent: got SPEAKER from NT\n");
								
								iLookupId = FKEY_SKEY_B3;
								bGotString = true;	//dont do it later
								msg.setStringParam1("SKEYB3");
								if (iEventType == J_KEY_DOWN)
									iEventType = J_BUTTON_DOWN;
								else
									iEventType = J_BUTTON_UP;
								break;
			default			:	iLookupId = -1;
		}

		if (iLookupId == -1)
		{
			iLookupId = iButtonId;
			sprintf(buf, "KBD%c", iButtonId);
			msg.setStringParam1(buf);
			bGotString = true;	//dont do it later
		}
	}
	else
	
	// Hex values for keys are different in C++ and Java, so need to translate them
	{
		//if we got here, then it must be a mouse click from the softphone

		switch (iButtonId)
		{

			case '1'	:	iLookupId = DIAL_1;
							break;
			case '2'	: iLookupId = DIAL_2;			break;
							break;
			case '3'	: iLookupId = DIAL_3;			break;
							break;
			case '4'	: iLookupId = DIAL_4;			break;
							break;
			case '5'	: iLookupId = DIAL_5;			break;
							break;
			case '6'	: iLookupId = DIAL_6;			break;
							break;
			case '7'	: iLookupId = DIAL_7;			break;
							break;
			case '8'	: iLookupId = DIAL_8;			break;
							break;
			case '9'	: iLookupId = DIAL_9;			break;
							break;
			case '0'	: iLookupId = DIAL_0;			break;
							break;
			case '#'	: iLookupId = DIAL_POUND;		break;
							break;
			case '*'	: iLookupId = DIAL_STAR;		break;
							break;
			
			case 0x0201	: iLookupId = FKEY_VOL_UP;		break;
			case 0x0202	: iLookupId = FKEY_VOL_DOWN;	break;
			case 0x1000	: iLookupId = FKEY_SCROLL_UP;	break;
			case 0x1001	: iLookupId = FKEY_SCROLL_DOWN; break;
			case 0x0100	: iLookupId = FKEY_MORE;		break;
			case 0x0101	: iLookupId = FKEY_REDIAL;		break;
			case 0x0102	: iLookupId = FKEY_CONFERENCE;	break;
			case 0x0103	: iLookupId = FKEY_TRANSFER;	break;
			case 0x0104	: iLookupId = FKEY_HOLD;		break;
			case 0x0105	: iLookupId = FKEY_SPEAKER;		break;
			case 0x0106	: iLookupId = FKEY_MUTE ;		break;
			case 0x0107	: iLookupId = FKEY_HEADSET;		break;
			
			case 0x0300	: iLookupId = FKEY_SKEY_L1;		break;
			case 0x0301	: iLookupId = FKEY_SKEY_L2;		break;
			case 0x0302	: iLookupId = FKEY_SKEY_L3;		break;
			case 0x0303	: iLookupId = FKEY_SKEY_L4;		break;
			case 0x0400	: iLookupId = FKEY_SKEY_R1;		break;
			case 0x0401	: iLookupId = FKEY_SKEY_R2;		break;
			case 0x0402	: iLookupId = FKEY_SKEY_R3;		break;
			case 0x0403	: iLookupId = FKEY_SKEY_R4;		break;
			case 0x0500	: iLookupId = FKEY_SKEY_B1;		break;
			case 0x0501	: iLookupId = FKEY_SKEY_B2;		break;
			case 0x0502	: iLookupId = FKEY_SKEY_B3;		break;
			default:	  iLookupId = -1;			    break;
		}		
	}
	

	if (iLookupId != -1)
	{
		PsButtonTask *psButtonTask = PsButtonTask::getButtonTask();
		

		//only lookup button index if it didn't come from NT keyboard
		if (iEventType != J_KEY_UP && iEventType != J_KEY_DOWN)
			iParam1 = psButtonTask->getButtonIndex(iLookupId);
		else
		{
			switch(iLookupId)
			{
				case '1' :
				case '2' : 
				case '3' : 
				case '4' : 
				case '5' : 
				case '6' : 
				case '7' : 
				case '8' : 
				case '9' : 
				case '0' :  iParam1 = iLookupId - 0x30;
							break;
				case '*' :  iParam1 = 11;
							break;
				case '#' :  iParam1 = 10;
							break;
				default :  iParam1 = 1;
							break;
			}
		}

		if (iParam1 != -1)
		{
//			osPrintf("postButtonEvent: setParm1 %d\n",iParam1);
			msg.setParam1(iParam1);
//			osPrintf("postButtonEvent: setParm2 %d\n",iLookupId);
			msg.setParam2(iLookupId);
	
			char arButtonIndex[10];

			//only lookup button info if it didn't come from NT keyboard
			if (iEventType != J_KEY_UP && iEventType != J_KEY_DOWN)
			{
//osPrintf("postButtonEvent: setStringParm1 %s\n",psButtonTask->getButtonInfo(iParam1).getName());
				msg.setStringParam1(psButtonTask->getButtonInfo(iParam1).getName());
			}
			else
			{
				if (!bGotString)
				{
					sprintf(arButtonIndex, "%d", iLookupId);
//osPrintf("postButtonEvent: setStringParm1 %s\n",arButtonIndex);
					msg.setStringParam1(arButtonIndex);
				}
			}
		
			sprintf(arButtonIndex, "%d", iLookupId);
//osPrintf("postButtonEvent: setStringParm2 %d\n",iLookupId);
			msg.setStringParam2(arButtonIndex);

			// post the message to pPhoneTask
//osPrintf("PTPhonebutton: START Posting message\n");
			res = pPhoneTask->postMessage(msg);
//osPrintf("PTPhonebutton: DONE Posting message\n");
		}
		else 
		{
			osPrintf("PTPhonebutton: Incorrect value from getButtonIndex\n");
		}
	} 
	else 
	{
		osPrintf("PTPhoneButton: Incorrect button ID\n");
	}
}
