// $Id: //depot/OPENDEV/sipXphone/src/pinger/PingerConfig.cpp#3 $
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
#include <assert.h>

#include <os/OsDefs.h>
#include <net/SipLineMgr.h>
#include <pinger/Pinger.h>

//: Pinger/Phone related configuration related functions and utilities
// This is intended as a container for all Phone product
// specific functions and utilities realted to configuration.
// PLEASE think twice before making a phone spscific utility.
// Try to generalize to make if more reusable.  Hoever if this
// is unavoidable, this is the place to put it.

// APPLICATION INCLUDES
#include <pinger/PingerConfig.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* ============================ CREATORS ================================== */


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

//:Serialize all of the configuration settings in a SipLineMgr to/from persistent storage
//!param: (in) bSave - Controls whether data is being stored (TRUE) or
//        retrieved (FALSE).
//!returns: TRUE if any items are saved or restored, otherwise FALSE
UtlBoolean SipLineMgrSerialize(SipLineMgr& lineMgr, UtlBoolean bSave)
{
    UtlBoolean bSuccess = false ;
    OsConfigDb* pConfigDb = Pinger::getPingerTask()->getConfigDb() ;
    UtlString strUserDefaultOutboundLine ;
    UtlString strDefaultIdentity ;

    int iActualLines = 0 ;
    int iUserLineCount = 1 ;
    int i ;

    OsConfigDb dbDeviceConfig ;
    OsConfigDb dbUserConfig ;

    dbDeviceConfig.loadFromFile(CONFIGDB_NAME_IN_FLASH) ;
    dbUserConfig.loadFromFile(CONFIGDB_USER_IN_FLASH) ;

    lineMgr.getDefaultOutboundLine(strDefaultIdentity) ;

    if (bSave)
    {
        // Remove all line parameters and then rewrite them.  Changes
        // are made to both the actual files and runtime cache
        lineMgr.purgeLines(&dbDeviceConfig) ;
        lineMgr.purgeLines(&dbUserConfig) ;
        lineMgr.purgeLines(pConfigDb) ;

        int noOfLines =  lineMgr.getNumLines() ;

        // Allocate Lines
        SipLine* lines = new SipLine[noOfLines] ;
        // Get Actual lines
        if (lineMgr.getLines(noOfLines, iActualLines, lines))
        {
            for (i=0; i < iActualLines; i++)
            {
                   //save the line only if it is persitent line
                   //for 1.3 all the lines are Persistent by default and should have
                   //autoenable set to true
               osPrintf("Trying to save line: auto enable=%d\n", lines[i].getAutoEnableStatus()) ;
                   if (lines[i].getAutoEnableStatus() == TRUE)
                   {
                       if (lines[i].isDeviceLine() )
                       {
                           // We only support a single device line
                           UtlString strKey(BASE_PHONESET_LINE_KEY) ;
                           lineMgr.storeLine(&dbDeviceConfig, strKey, lines[i]) ;
                           lineMgr.storeLine(pConfigDb, strKey, lines[i]) ;
                           bSuccess = true ;

                           // Note if this is the default line
                           Url canonicalUrl = lines[i].getCanonicalUrl() ;
                           UtlString strIdentity  = canonicalUrl.toString() ;
                           if (strDefaultIdentity.compareTo(strIdentity) == 0)
                           {
                               strUserDefaultOutboundLine = DEFAULT_LINE_PARAM_PHONESET_LINE ;
                           }
                       }
                       else
                       {
                           // Multiple User Lines are stored with a unique count
                           // embedded into the key.  For example USER_LINE.<n>.URL
                           UtlString strKey(BASE_USER_LINE_KEY) ;

                           char szTempBuf[32] ;
                           sprintf(szTempBuf, "%d", iUserLineCount++) ;
                           strKey.append(szTempBuf) ;
                           strKey.append(".") ;

                           lineMgr.storeLine(&dbUserConfig, strKey, lines[i]) ;
                           lineMgr.storeLine(pConfigDb, strKey, lines[i]) ;
                           bSuccess = true ;

                           // Note if this is the default line
                           Url canonicalUrl = lines[i].getCanonicalUrl() ;
                           UtlString strIdentity  = canonicalUrl.toString() ;
                           if (strDefaultIdentity.compareTo(strIdentity) == 0)
                           {
                               strUserDefaultOutboundLine = DEFAULT_LINE_PARAM_BASE_USER_LINE ;
                               strUserDefaultOutboundLine .append(szTempBuf) ;
                           }
                       }
              }
           }
        }
        // Store default line parameter
        if (!strUserDefaultOutboundLine.isNull())
        {
            dbUserConfig.set(USER_DEFAULT_OUTBOUND_LINE, strUserDefaultOutboundLine) ;
            pConfigDb->set(USER_DEFAULT_OUTBOUND_LINE, strUserDefaultOutboundLine) ;
        }

        // Free Lines
        delete[] lines;
        dbDeviceConfig.storeToFile(CONFIGDB_NAME_IN_FLASH) ;
        dbUserConfig.storeToFile(CONFIGDB_USER_IN_FLASH) ;
    }
    else
    {
        // Load the default outbound line
        dbUserConfig.get(USER_DEFAULT_OUTBOUND_LINE, strUserDefaultOutboundLine) ;

        // Load Device Line
        SipLine lineDevice ;
        if (lineMgr.loadLine(&dbDeviceConfig, BASE_PHONESET_LINE_KEY, lineDevice))
        {
            lineDevice.setUser("Device") ;
            lineMgr.addLine(lineDevice, FALSE) ;
            bSuccess = true ;

            // If the outbound line is set to the device line, record the identity.
            if (strUserDefaultOutboundLine.compareTo(DEFAULT_LINE_PARAM_PHONESET_LINE) == 0)
            {
                lineMgr.setDefaultOutboundLine(lineDevice.getCanonicalUrl()) ;
            }
        }

        // Load User Lines.  Attempt to load the max number of lines, however,
        // if any lines fail to load, kick out.  This is a safe optimization
        // assuming that lines numbers are sequential.
        UtlBoolean nextLineFound = TRUE;
        int i = 1;
        while( nextLineFound)
        {
            UtlString strKey(BASE_USER_LINE_KEY) ;
            char szTempBuf[32] ;
            sprintf(szTempBuf, "%d", i) ;
            strKey.append(szTempBuf) ;
            strKey.append(".") ;

            SipLine lineUser ;
            nextLineFound = lineMgr.loadLine(&dbUserConfig, strKey, lineUser);
            if (nextLineFound)
            {
                lineUser.setUser("User") ;
                lineMgr.addLine(lineUser, FALSE) ;
                bSuccess = true ;
                i++;
                // Check to see if this is 'the' default outbound user line.
                // If so, set it as the default outbound line.
                UtlString strLineId(BASE_USER_LINE_KEY) ;
                strLineId.append(szTempBuf) ;
                if (strUserDefaultOutboundLine.compareTo(strLineId) == 0)
                {
                    lineMgr.setDefaultOutboundLine(lineUser.getCanonicalUrl()) ;
                }
            }
        }
        //check if default poutbound is set?
        UtlString outbound;
        lineMgr.getDefaultOutboundLine(outbound);
        Url outboundUrl(outbound);
        UtlString host;
        outboundUrl.getHostAddress(host);
        if( host.isNull())
        {
           lineMgr.setFirstLineAsDefaultOutBound();
        }
    }
    return bSuccess ;
}
