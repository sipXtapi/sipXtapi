//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBridgeAlgSimple.h"
#include "mp/MpMisc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpBridgeAlgSimple::MpBridgeAlgSimple(int inputs, int outputs,
                                     UtlBoolean mixSilence,
                                     int samplesPerFrame)
: MpBridgeAlgBase(inputs, outputs, mixSilence)
, mpGainMatrix(NULL)
, mpMixAccumulator(NULL)
{
   // Allocate mix matrix.
   mpGainMatrix = new MpBridgeGain[maxInputs()*maxOutputs()];
   assert(mpGainMatrix != NULL);

   // Initially set matrix to inversed unity matrix, with zeros along
   // main diagonal.
   MpBridgeGain *pGain = mpGainMatrix;
   *pGain = MP_BRIDGE_GAIN_MUTED;
   pGain++;
   for (int row=0; row<maxOutputs()-1; row++)
   {
      for (int i=0; i<maxInputs(); i++)
      {
         *pGain = MP_BRIDGE_GAIN_PASSTHROUGH;
         pGain++;
      }
      *pGain = MP_BRIDGE_GAIN_MUTED;
      pGain++;
   }

   // Allocate temporary storage for mixing data.
   mpMixAccumulator = new MpBridgeAccum[samplesPerFrame];
   assert(mpMixAccumulator != NULL);
}

MpBridgeAlgSimple::~MpBridgeAlgSimple()
{
   delete[] mpGainMatrix;
   delete[] mpMixAccumulator;
}

/* ============================= MANIPULATORS ============================= */

UtlBoolean MpBridgeAlgSimple::doMix(MpBufPtr inBufs[], int inBufsSize,
                                    MpBufPtr outBufs[], int outBufsSize,
                                    int samplesPerFrame)
{
   // Loop over all outputs and mix
   for (int outputNum=0; outputNum<outBufsSize; outputNum++)
   {
      MpBridgeGain *pInputGains = &mpGainMatrix[outputNum*maxInputs()];

      // Initialize accumulator
      for (int i=0; i<samplesPerFrame; i++)
      {
         mpMixAccumulator[i] = MPF_BRIDGE_FLOAT(0.0f);
      }

      // Get buffer for output data.
      MpAudioBufPtr pOutBuf = MpMisc.RawAudioPool->getBuffer();
      assert(pOutBuf.isValid());
      pOutBuf->setSamplesNumber(samplesPerFrame);
      pOutBuf->setSpeechType(MP_SPEECH_UNKNOWN);

      // Mix input data to accumulator
      for (int inputNum=0; inputNum<inBufsSize; inputNum++)
      {
         if (inBufs[inputNum].isValid() &&
             pInputGains[inputNum] != MP_BRIDGE_GAIN_MUTED)
         {
            MpAudioBufPtr pFrame = inBufs[inputNum];
            assert(pFrame->getSamplesNumber() == samplesPerFrame);
            if (pInputGains[inputNum] == MP_BRIDGE_GAIN_PASSTHROUGH)
            {
               MpDspUtils::add_IGain(pFrame->getSamplesPtr(), mpMixAccumulator,
                                     samplesPerFrame, MP_BRIDGE_FRAC_LENGTH);
            }
            else
            {
               MpDspUtils::addMul_I(pFrame->getSamplesPtr(),
                                    pInputGains[inputNum],
                                    mpMixAccumulator,
                                    samplesPerFrame);
            }
         }
      }

      // Move data from accumulator to output.
      MpDspUtils::convert_Att(mpMixAccumulator, pOutBuf->getSamplesWritePtr(),
                              samplesPerFrame, MP_BRIDGE_FRAC_LENGTH);
      outBufs[outputNum].swap(pOutBuf);
   }

   return TRUE;
}

void MpBridgeAlgSimple::setGainMatrixValue(int column, int row, MpBridgeGain val)
{
   mpGainMatrix[row*maxInputs() + column] = val;
}

void MpBridgeAlgSimple::setGainMatrixRow(int row, int numValues, const MpBridgeGain val[])
{
   // Copy gain data to mix matrix row.
   MpBridgeGain *pCurGain = &mpGainMatrix[row*maxInputs()];
   for (int i=0; i<numValues; i++)
   {
      if (val[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         *pCurGain = val[i];
      }
      pCurGain++;
   }
}

void MpBridgeAlgSimple::setGainMatrixColumn(int column, int numValues, const MpBridgeGain val[])
{
   // Copy gain data to mix matrix column.
   MpBridgeGain *pCurGain = &mpGainMatrix[column];
   for (int i=0; i<numValues; i++)
   {
      if (val[i] != MP_BRIDGE_GAIN_UNDEFINED)
      {
         *pCurGain = val[i];
      }
      pCurGain += maxInputs();
   }
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

