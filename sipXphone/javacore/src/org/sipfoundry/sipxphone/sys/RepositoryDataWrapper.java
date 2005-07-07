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

import java.io.* ;

/**
 * This wrapper is sort of a hack, but an important hack.  Java kicks and 
 * screams if we tell it to deserialize an object and we don't have the 
 * class loaded (think about our applet style class loaders). 
 * <p>
 * This is very bad.  Our repository is currently a big hash table populated 
 * with both user and system objects.  We ran into problems where this list
 * could not be deserialized because we did not have the user class around.
 * <p>
 * Well, to solve this problem, we basically wrap our objects in this thin 
 * data wrapper.   This allows us to store the data as bytes, get the data as
 * bytes and massage it into an object only when needed.  In theory (hopefully),
 * the appropriate class loader is initialized and we can actually deserialize
 * the object.  If not- an exception is thrown.
 *
 * @author Robert J. Andreasen, Jr.
 */
class RepositoryDataWrapper implements Serializable
{
    /** our object stored as bytes */
    private byte m_data[] ;
    
    /**
     * public no argument constructor needed purely for serialization.
     */
    public RepositoryDataWrapper()
    {
        m_data = null ;        
    }


    /**
     * Constructor taking a serializable object.  The constructor will turn 
     * the object into a byte array.
     */
    public RepositoryDataWrapper(Serializable object)
        throws IOException
    {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream() ;        
        ObjectOutputStream objOutputStream = new ObjectOutputStream(outputStream) ;
        objOutputStream.writeObject(object) ;        
        m_data = outputStream.toByteArray() ;        
    }


    /**
     * Get an Object from the data wrapper.  This pulls the object out of the 
     * byte array.
     */
    public Object getObject()
        throws IOException, ClassNotFoundException, OptionalDataException
    {
        Object objRC = null ;
        
        if (m_data != null) {
            ByteArrayInputStream inputStream = new ByteArrayInputStream(m_data) ;
            ObjectInputStream objInputStream = new ObjectInputStream(inputStream) ;
            objRC = objInputStream.readObject() ;
        }        
        return objRC ;
    }
}
