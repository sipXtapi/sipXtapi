// Copyright 2009 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
// USA. 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 
// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "include/VoiceEngine.h"

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
VoiceEngine::VoiceEngine(GIPSVoiceEngine* pGIPSVoiceEngine) 
{
	m_pGIPSVoiceEngine = pGIPSVoiceEngine ;

	m_pBase = GIPSVEBase::GIPSVE_GetInterface(m_pGIPSVoiceEngine);
	m_pVqe = GIPSVEVQE::GetInterface(m_pGIPSVoiceEngine) ;
	m_pNetwork = GIPSVENetwork::GetInterface(m_pGIPSVoiceEngine) ;
	m_pCodec = GIPSVECodec::GetInterface(m_pGIPSVoiceEngine) ;
    m_pEncryption = GIPSVEEncryption::GetInterface(m_pGIPSVoiceEngine) ;
    m_pRTCP = GIPSVERTP_RTCP::GetInterface(m_pGIPSVoiceEngine) ;
    m_pHardware = GIPSVEHardware::GetInterface(m_pGIPSVoiceEngine) ;
    m_pVolumnControl = GIPSVEVolumeControl::GetInterface(m_pGIPSVoiceEngine) ;
    m_pDTMF = GIPSVEDTMF::GetInterface(m_pGIPSVoiceEngine) ;
    m_pFile = GIPSVEFile::GetInterface(m_pGIPSVoiceEngine) ;
    m_pExternalMedia = GIPSVEExternalMedia::GetInterface(m_pGIPSVoiceEngine) ;
}

VoiceEngine::~VoiceEngine() 
{
    m_pExternalMedia->Release() ;
    m_pFile->Release() ;
    m_pDTMF->Release() ;
    m_pVolumnControl->Release() ;
    m_pHardware->Release() ;
    m_pRTCP->Release() ;
    m_pEncryption->Release() ;
    m_pCodec->Release() ;
	m_pNetwork->Release() ;
	m_pVqe->Release() ;	
	m_pBase->GIPSVE_Release();

    GIPSVoiceEngine::Delete(m_pGIPSVoiceEngine) ;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

GIPSVoiceEngine* VoiceEngine::getVE() 
{
	return m_pGIPSVoiceEngine ;
}

GIPSVEBase* VoiceEngine::getBase() 
{
	return m_pBase ;
}

GIPSVEVQE* VoiceEngine::getVQE() 
{
	return m_pVqe ;
}

GIPSVENetwork* VoiceEngine::getNetwork()
{
	return m_pNetwork ;
}

GIPSVECodec* VoiceEngine::getCodec() 
{
	return m_pCodec ;
}

GIPSVEEncryption* VoiceEngine::getEncryption()
{
	return m_pEncryption ;
}

GIPSVERTP_RTCP* VoiceEngine::getRTCP()
{
    return m_pRTCP ;
}

GIPSVEHardware* VoiceEngine::getHardware() 
{
    return m_pHardware ;
}

GIPSVEVolumeControl* VoiceEngine::getVolumeControl()
{
    return m_pVolumnControl ;
}

GIPSVEDTMF* VoiceEngine::getDTMF()
{
    return m_pDTMF ;
}

GIPSVEFile* VoiceEngine::getFile() 
{
    return m_pFile ;
}

GIPSVEExternalMedia* VoiceEngine::getExternalMedia() 
{
    return m_pExternalMedia ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
