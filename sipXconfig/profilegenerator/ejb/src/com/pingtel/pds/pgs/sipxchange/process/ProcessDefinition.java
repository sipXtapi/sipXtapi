/*
 * 
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
 * ProcessDefinition
 *
 * @author Pradeep Paudyal
 */
public class ProcessDefinition
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
        private String     m_strName;
        private String     m_strGroup;
        private String     m_strStatus;
        private String     m_strPID;

        //Dependencies are services that must be started for this
        //service to function properly.
        //Collection of process names
        private Collection m_colDependencies;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

        public ProcessDefinition( String name, String group, String status, String strPID ){
            m_strName   = name;
            m_strGroup  = group;
            m_strStatus = status;
            m_strPID    =  strPID;
        }

        public ProcessDefinition( String name, String group, String status, String strPID,
                                  Collection  dependencies ){
            m_strName         = name;
            m_strGroup        = group;
            m_strStatus       = status;
            m_strPID          =  strPID;
            m_colDependencies = dependencies;
        }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
       public String getName(){
         return m_strName;
       }

       public String getGroup(){
         return m_strGroup;
       }

       public String getStatus(){
         return m_strStatus;
       }

       public String getPID(){
         return m_strPID;
       }

       public void setDependencies(Collection dependencies ){
         m_colDependencies = dependencies;
       }

       public Collection getDependencies(){
         return m_colDependencies;
       }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    public String toString(){
        return ("Name: "+m_strName +" Group: "+ m_strGroup+ " Status: "+ m_strStatus );
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}

