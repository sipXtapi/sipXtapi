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
import java.util.Iterator;
import java.util.Collection;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Date;

import java.io.Serializable;

import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.Element;
import org.jdom.Attribute;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.XMLSupport;

import com.pingtel.pds.pgs.profile.Projection;
import com.pingtel.pds.pgs.profile.ProjectionInput;
import com.pingtel.pds.pgs.profile.ProjectionRule;


public class StandardTopDown implements Projection, Serializable {

    public ProjectionInput project( final Collection projectionInputs, final Collection validRefPropertyIDs )
        throws PDSException {


        for ( Iterator iInputs = projectionInputs.iterator(); iInputs.hasNext(); ) {
            ProjectionInput input = (ProjectionInput) iInputs.next();

            if ( input.getDocument() == null || input.getFinalRules() == null ) {
                continue;
            }

            Document doc = input.getDocument();
            Element root = doc.getRootElement();

            Collection rules = input.getFinalRules();
            setCardinalities ( rules );

            ///////////////////////////////////////////////////////////////////////
            //
            // Decision table on what how substitutions are made:
            //
            //  ________________________________________________________________________________________________
            //  |                               | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15| 16|
            //  |                               |___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
            //  |   Is Final?                   | Y | Y | Y | Y | Y | Y | Y | Y | N | N | N | N | N | N | N | N |
            //  |   Cardinality = '1' or '0..1' | Y | Y | Y | Y | N | N | N | N | Y | Y | Y | Y | N | N | N | N |
            //  |   Exists with none or diff ID?| Y | Y | N | N | Y | Y | N | N | Y | Y | N | N | Y | Y | N | N |
            //  |   Exists with same ID?        | Y | N | Y | N | Y | N | Y | N | Y | N | Y | N | Y | N | Y | N |
            //  |                               |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
            //  |   Do nothing                  | X | X | X | X | X | X | X | X |   |   |   |   |   |   |   |   |
            //  |   Replace existing            |   |   |   |   |   |   |   |   | X | X | X |   | X |   | X |   |
            //  |   Add                         |   |   |   |   |   |   |   |   |   |   |   | X |   | X |   | X |
            //  |_______________________________|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
            //
            // Notes: cardinality can be:
            //
            //      '1' - one and only one per profile
            //      '0..1' - zero or one per profile
            //      '1..N' - one or more per profile
            //      '0..N' - zero or more per profile
            //      NEW: A - aggregates accross config sets regardless; used for additional parameters
            //
            ///////////////////////////////////////////////////////////////////////


            for (   Iterator iProperties = root.getChildren().iterator();
                    iProperties.hasNext(); ) {

                Element property = (Element) iProperties.next();

                Integer refPropertyID =
                    new Integer ( property.getAttributeValue( "ref_property_id" ) );

                if ( !validRefPropertyIDs.contains( refPropertyID ) ) {
                    continue;
                }

                String cardinality = this.getCardinality( refPropertyID );

                ///////////////////////////////////////////////////////////////////////
                //
                // cases 1-8 cut out - if the Ref Property has been set as 'final' in a
                // prior ProjectionInput Projection Rule then we just 'Do nothing'.
                //
                ///////////////////////////////////////////////////////////////////////
                if ( !isPropertyFinal( refPropertyID ) ) {

                    // cases 13-16
                    if (    cardinality.equals( "0..N" ) ||
                            cardinality.equals( "1..N" ) ) {

                        m_propertyMap.replaceAddProperty(   refPropertyID,
                                                            property.getAttributeValue ( "id" ),
                                                            property );

                    }
                    else if ( cardinality.equals( "A" ) ) {
                        m_propertyMap.addProperty(  refPropertyID,
                                                    property );
                    }
                    else { // cases 9-12
                        m_propertyMap.replaceAddAllProperties(  refPropertyID,
                                                                property );

                    }

                } // if not

            } // for properties

            setProjectionRules ( rules );
            setFinals ( rules );
        } // for inputs

        Element root = new Element ( "PROFILE");

        Document doc = new Document ( root );
        Collection projectedElements = m_propertyMap.values();

        for (   Iterator iElements = XMLSupport.detachableIterator(projectedElements.iterator());
                iElements.hasNext(); ) {

            Element e = (Element) iElements.next();
            e = (Element)e.detach();
            root.addContent( e );
        }

        return new ProjectionInput ( doc, collateProjectionRules() );
    }



    ///////////////////////////////////////////////////////////////////////
    //
    // ProjectionRules can define a particular ref property as being final
    // in projection.
    //
    ///////////////////////////////////////////////////////////////////////
    private void setProjectionRules(Collection projectionRules ) {

        for ( Iterator i = projectionRules.iterator(); i.hasNext() ; ) {
            ProjectionRule rule = (ProjectionRule) i.next();
            Integer refPropertyID = rule.getRefPropertyID();

            if (m_projectionRules.containsKey(refPropertyID)) {

                ProjectionRule existing =
                        (ProjectionRule) m_projectionRules.get( refPropertyID );

                m_projectionRules.remove(existing);
            }

            m_projectionRules.put(refPropertyID, rule);
        }
    }


    private void setFinals ( Collection projectionRules ) {

        for ( Iterator i = projectionRules.iterator(); i.hasNext(); ) {
            ProjectionRule rule = (ProjectionRule) i.next();

            Integer refPropertyID = rule.getRefPropertyID();

            if ( !m_finalMap.containsKey( refPropertyID ) ) {
                if ( rule.getIsFinal() )
                    m_finalMap.put( refPropertyID, null );
            }
        }
    }


    private Collection collateProjectionRules () {

        ArrayList newRules = new ArrayList();

        for ( Iterator i = m_projectionRules.values().iterator(); i.hasNext(); ) {
            ProjectionRule rule = (ProjectionRule) i.next();
            ProjectionRule newRule = new ProjectionRule (   rule.getRefPropertyID(),
                                                            isPropertyFinal ( rule.getRefPropertyID() ),
                                                            rule.getIsReadOnly(),
                                                            rule.getCardinality() );

            newRules.add( newRule );
        }

        return newRules;
    }

    private boolean isPropertyFinal(Integer refPropertyID) {
        boolean result = false;

        if (m_finalMap.containsKey(refPropertyID))
            result = true;

        return result;
    }


    private void setCardinalities ( Collection projectionRules ) {

        for ( Iterator i = projectionRules.iterator(); i.hasNext(); ) {
            ProjectionRule rule = (ProjectionRule) i.next();
            Integer refPropertyID = rule.getRefPropertyID();

            if ( !m_propertyCardinalities.containsKey( refPropertyID ) ) {

                String cardinality = rule.getCardinality();
                m_propertyCardinalities.put( refPropertyID, cardinality );
            }
        }
    }


    private String getCardinality ( Integer refPropertyID ) {

        String result = (String) m_propertyCardinalities.get( refPropertyID );
        return result;
    }


    private HashMap m_projectionRules = new HashMap();
    private HashMap m_finalMap = new HashMap();

    private HashMap m_propertyCardinalities = new HashMap();

    // instance of PropertyMap (defined below)
    private PropertyMap m_propertyMap = new PropertyMap();

} // class
