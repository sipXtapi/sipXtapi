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

#ifndef _VOICEENGINE_H
#define _VOICEENGINE_H

#include "VoiceEngineDefs.h"

//////////////////////////////////////////////////////////////////////////////

class VoiceEngine
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /* ============================ CREATORS ================================== */
    VoiceEngine(GIPSVoiceEngine* pGIPSVoiceEngine) ;

    virtual ~VoiceEngine() ;

    /* ============================ MANIPULATORS ============================== */

    /* ============================ ACCESSORS ================================= */

    GIPSVoiceEngine* getVE() ;
	GIPSVEBase* getBase() ;
	GIPSVEVQE* getVQE() ;
	GIPSVENetwork* getNetwork() ;
	GIPSVECodec* getCodec() ;
    GIPSVEEncryption* getEncryption();
    GIPSVERTP_RTCP* getRTCP();
    GIPSVEHardware* getHardware() ;
    GIPSVEVolumeControl* getVolumeControl() ;
    GIPSVEDTMF* getDTMF() ;
    GIPSVEFile* getFile() ;
    GIPSVEExternalMedia* getExternalMedia() ;

	
    /* ============================ INQUIRY =================================== */

    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
	GIPSVoiceEngine* m_pGIPSVoiceEngine ;
	GIPSVEBase* m_pBase ;
	GIPSVEVQE* m_pVqe ;
	GIPSVENetwork* m_pNetwork;
	GIPSVECodec* m_pCodec ;
    GIPSVEEncryption* m_pEncryption;
    GIPSVERTP_RTCP* m_pRTCP;
    GIPSVEHardware* m_pHardware;
    GIPSVEVolumeControl* m_pVolumnControl;
    GIPSVEDTMF* m_pDTMF ;
    GIPSVEFile* m_pFile ;
    GIPSVEExternalMedia* m_pExternalMedia;

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
} ;

#endif
