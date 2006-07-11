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



package org.sipfoundry.sipxphone.sys ;

import java.io.Serializable;

public class AudioLevels implements Serializable
{
    private int m_HandsetGain = 5;
    private int m_HeadsetGain = 5;
    private int m_MicrophoneGain = 5;
    private int m_HeadsetVolume = 5;
    private int m_RingerVolume = 5;
    private int m_HandsetVolume = 5;
    private int m_SpeakerVolume = 5;    
    private int m_ContrastLevel = -1 ;
    
    
    public AudioLevels()
    {
    }

	public int getHandsetGain()
	{
		return m_HandsetGain;
	}

	public void setHandsetGain(int propValue)
	{
	    m_HandsetGain = propValue;
	}
	
	public int getHeadsetGain()
	{
		return m_HeadsetGain;
	}

	public void setHeadsetGain(int propValue)
	{
	    m_HeadsetGain = propValue;
	}
	
	public int getMicrophoneGain()
	{
		return m_MicrophoneGain;
	}

	public void setMicrophoneGain(int propValue)
	{
	    m_MicrophoneGain = propValue;
	}
	
	public int getHeadsetVolume()
	{
		return m_HeadsetVolume;
	}

	public void setHeadsetVolume(int propValue)
	{
	    m_HeadsetVolume = propValue;
	}
	
	public int getRingerVolume()
	{
		return m_RingerVolume;
	}

	public void setRingerVolume(int propValue)
	{
	    m_RingerVolume = propValue;
	}

	public int getHandsetVolume()
	{
		return m_HandsetVolume;
	}

	public void setHandsetVolume(int propValue)
	{
	    m_HandsetVolume = propValue;
	}
	
	public int getSpeakerVolume()
	{
		return m_SpeakerVolume;
	}

	public void setSpeakerVolume(int propValue)
	{
	    m_SpeakerVolume = propValue;
	}	
	
	public int getLCDContrast()
	{
	    return m_ContrastLevel ;	    
	}
		
	public void setLCDContrast(int propValue)
	{
	    m_ContrastLevel = propValue ;   
	}
	
}
