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
 
package com.pingtel.pds.pgs.plugins.projection;


import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import org.jdom.Element;

///////////////////////////////////////////////////////////////////////
//
// PropertyMap is a home brew MultiMap - it uses a HashMap as it's
// backbone for storing 'buckets' of ArrayLists.   The map uses
// Ref Property IDs as its keys; the buckets store properties which
// correspond to those keys.
//
///////////////////////////////////////////////////////////////////////
public class PropertyMap {


    public PropertyMap () {
        m_map = new HashMap( 256 );
    }


    public void addProperty ( Integer refPropertyID, Element property ) {
        ArrayList bucket = null;

        if ( m_map.containsKey( refPropertyID ) ) {
            bucket = (ArrayList) m_map.get ( refPropertyID );
        }
        else {
            bucket = new ArrayList();
        }

        bucket.add( property.clone() );
        m_map.put( refPropertyID, bucket );
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // replaceAddAllProperties is used to add a property to the data
    // structure.   If one exists it is removed before adding the new one.
    //
    ///////////////////////////////////////////////////////////////////////
    public void replaceAddAllProperties (   Integer refPropertyID,
                                            Element property ) {

        if ( m_map.containsKey( refPropertyID ) ) {
            ArrayList bucket = (ArrayList) m_map.get ( refPropertyID );
            bucket.clear();
        }

        addProperty ( refPropertyID, property );
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // replaceAddProperty performs much the same function as
    // replaceAddAllProperties except that replaces and existing property
    // only if it has the same 'id' attribute.   The id attribute is used
    // to distinguish instances of ref propeties which have cardinalities of
    // '1..N' or '0..N'.
    //
    ///////////////////////////////////////////////////////////////////////
    public void replaceAddProperty (    Integer refPropertyID,
                                        String id,
                                        Element property ) {

        if ( m_map.containsKey( refPropertyID ) ) {
            ArrayList bucket = (ArrayList) m_map.get ( refPropertyID );

            for ( Iterator i = bucket.iterator(); i.hasNext(); ) {
                Element e = (Element) i.next();

                String elementID = e.getAttributeValue( "id" );

                if ( elementID != null ) {
                    if ( elementID.equals( id ) ) {
                        bucket.remove( e );
                        break;
                    }
                }
            }
        }

        addProperty ( refPropertyID, property );
    }



    ///////////////////////////////////////////////////////////////////////
    //
    // values does produces a full list of all of the properties in the
    // data structure - same idea as HashMap's values() operation.
    //
    ///////////////////////////////////////////////////////////////////////
    public Collection values() {

        ArrayList elements = new ArrayList();
        Collection buckets = this.m_map.values();

        for ( Iterator iBucket = buckets.iterator(); iBucket.hasNext(); ) {
            ArrayList bucket = (ArrayList) iBucket.next();

            for ( Iterator iProperty = bucket.iterator(); iProperty.hasNext(); ) {
                Element e = (Element) iProperty.next();
                elements.add( e );
            }
        }

        return elements;
    }


    public void clearSpecificProperty ( Integer refPropertyID ) {
        if ( m_map.containsKey( refPropertyID ) )
            m_map.remove( refPropertyID );
    }


    private HashMap m_map; // backbone of the multimap.
} // class