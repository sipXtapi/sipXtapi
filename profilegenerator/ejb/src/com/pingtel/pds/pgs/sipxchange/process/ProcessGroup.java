/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/sipxchange/process/ProcessGroup.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.sipxchange.process;

import java.util.Collection;

/**
 * ProcessGroup
 *
 * @author Pradeep Paudyal
 */
public class ProcessGroup
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
        private String m_strGroupID;
        private Collection m_colProcesses;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

        public ProcessGroup( String strGroupID, Collection colProcesses){
            m_strGroupID    = strGroupID;
            m_colProcesses  = colProcesses;
        }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
       public String getGroupID(){
         return m_strGroupID;
       }

       public Collection getProcesses(){
         return m_colProcesses;
       }



//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}

