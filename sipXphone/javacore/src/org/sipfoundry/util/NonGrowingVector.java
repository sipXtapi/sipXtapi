/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/NonGrowingVector.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util;

import java.util.Vector ;
/**
 * NonGrowingVector.java
 * A vector whose size cannot grow beyond specified after its created.
 *
 * Its size is determined by the param you pass in the constructor. 
 * It also has a method to tell if the limit
 * was tried to be exceeded( i.e. if addElement() was called after the 
 * limit was reached).
 *
 * Created: Thu Aug 30 10:23:14 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class NonGrowingVector {
    
        private int m_iNumberOfElements ;
        private Vector m_vector ;
        private boolean m_bSizeTriedToBeExceeded;
        public NonGrowingVector( int numberOfElements ){
            m_iNumberOfElements = numberOfElements;
            m_vector = new Vector();
        }
        
        public void addElement(Object obj){
            if( m_iNumberOfElements != -1 ){
                if( m_vector.size() >= m_iNumberOfElements ){
                    m_bSizeTriedToBeExceeded = true;
                     }
            }
            
            if( !m_bSizeTriedToBeExceeded ){
                //debug("****ADDED WAS " + obj );
                m_vector.addElement( obj );
            }
            
        }
        
        public int size(){
            return m_vector.size();
        }
        
        public boolean wasSizeTriedToBeExceeded(){
            return m_bSizeTriedToBeExceeded;
        } 
        
        public void removeElementAt(int i ){
            m_vector.removeElementAt(i);
        }
        
        public Object elementAt(int i){
            return m_vector.elementAt(i);
        }
        
        public java.util.Enumeration elements(){
            return m_vector.elements();
        }
        
        public Vector getVector(){
            return m_vector;
        }
    
}// NonGrowingVector
