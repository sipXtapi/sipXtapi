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
 
package com.pingtel.pds.pgs.profile;

import java.io.ByteArrayInputStream;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import javax.naming.NamingException;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.EJBHomeFactory;

/**
 * RefPropertyCache is a Singleton cache which is used to cache certain
 * attributes of RefProperties.
 *
 * @author ibutcher
 * 
 */
class RefPropertyCache {

//////////////////////////////////////////////////////////////////////////
// Constants
////  
      

//////////////////////////////////////////////////////////////////////////
// Attributes
////    
    private static RefPropertyCache mInstance;

    private Map mCardinalityMap;
    private RefPropertyHome mRefPropertyHome;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    private RefPropertyCache()  {
        mCardinalityMap = Collections.synchronizedMap(new HashMap());

        try {
            mRefPropertyHome = (RefPropertyHome)
                EJBHomeFactory.getInstance().getHomeInterface(
                        RefPropertyHome.class,
                        "RefProperty");
        }
        catch(NamingException e) {
            throw new RuntimeException(e.getMessage());
        }
    }
    
    
//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * getInstance returns a reference to a RefPropertyCache object
     *
     * @return refence to RefPropertyCache
     */
    public synchronized static RefPropertyCache getInstance() {
        if(mInstance == null) {
            mInstance = new RefPropertyCache();
        }
        return mInstance;
    }
    

//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * flush empties all of the cached data.
     *
     */
    public void flush() {
        mCardinalityMap.clear();
    }

    /**
     * getCardinality returns the cardinality value for the given
     * RefProperty.
     *
     * @param refPropertyId PK of the RefProperty that you wish to get
     * the cardinality for.
     * @return the cardinality of the RefProperty.
     */
    public synchronized String getCardinality(Integer refPropertyId) {

        SAXBuilder saxBuilder = new SAXBuilder();
        if(!mCardinalityMap.containsKey(refPropertyId)){
            RefProperty refProperty = null;
            String cardinality = null;

            try {
                refProperty = mRefPropertyHome.findByPrimaryKey(refPropertyId);

                String content = refProperty.getContent();
                Document rpContent =
                    saxBuilder.build(
                            new ByteArrayInputStream (content.getBytes()));

                Element definition = rpContent.getRootElement();

                cardinality = definition.getAttribute("cardinality").getValue();

                mCardinalityMap.put(refPropertyId, cardinality);
            }
            catch (Exception ex ) {
                throw new RuntimeException(ex);
            }
        }

        return (String) mCardinalityMap.get(refPropertyId);
    }



//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////    


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}

