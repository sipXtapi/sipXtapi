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

package com.pingtel.pds.pgs.entity;


import com.pingtel.pds.pgs.plugins.projection.PropertyMap;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.util.Iterator;
import java.util.Collection;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;
import org.xml.sax.InputSource;

public class ConfigurationSetMerger {

    private XMLOutputter m_xmlOut = new XMLOutputter();
    private static SAXBuilder m_saxBuilder = new SAXBuilder ();

    public static final void main ( String args [] ) {
        String one = "<PROFILE><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"ffff\"><PHONESET_DIGITMAP><NUMBER>ffff</NUMBER><TARGET>44444</TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"wwwww\"><PHONESET_DIGITMAP><NUMBER>wwwww</NUMBER><TARGET>11111</TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP></PROFILE>";
        //String two = "<PROFILE><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"ffff\"><PHONESET_DIGITMAP><NUMBER>ffff</NUMBER><TARGET>44444</TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"wwwww\"><PHONESET_DIGITMAP><NUMBER>aaaaa</NUMBER><TARGET>fff</TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP></PROFILE>";
        String two = "<PROFILE><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"ffff\"><PHONESET_DIGITMAP><NUMBER></NUMBER><TARGET></TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP><PHONESET_DIGITMAP ref_property_id=\"4\" id=\"wwwww\"><PHONESET_DIGITMAP><NUMBER>aaaaa</NUMBER><TARGET>fff</TARGET></PHONESET_DIGITMAP></PHONESET_DIGITMAP></PROFILE>";

        ByteArrayInputStream bas =
                new ByteArrayInputStream ( one.getBytes() );
        try {

            Document formXML = m_saxBuilder.build( bas );
            Element root = formXML.getRootElement();

            ConfigurationSetMerger m = new ConfigurationSetMerger();

        }
        catch ( Exception ex) {
            ex.printStackTrace();
        }
    }


    public String merge ( Element configSet, String formValues )
        throws IOException, JDOMException {

        PropertyMap map = new PropertyMap ( );

        for ( Iterator iCSProps = configSet.getChildren().iterator(); iCSProps.hasNext(); ) {
            Element element = (Element) iCSProps.next();
            map.addProperty(
                new Integer ( element.getAttributeValue( "ref_property_id" ) ),
                element );
        }

        Document formXML = m_saxBuilder.build(new InputSource(new StringReader(formValues)));
        Element root = formXML.getRootElement();

        for (   Iterator iProperties = root.getChildren().iterator();
                iProperties.hasNext(); ) {

            Element property = (Element) iProperties.next();
                Integer refPropertyID =
                    new Integer ( property.getAttributeValue( "ref_property_id" ) );
                map.clearSpecificProperty( refPropertyID );
        }


        for (   Iterator iProperties = root.getChildren().iterator();
                iProperties.hasNext(); ) {

            Element property = (Element) iProperties.next();

            //////////////////////////////////////////////////////////////////////
            //
            // These next two lines deserve some explanation.   The first call
            // to removeEmptyNodes removes leaf elements which have no data.
            // This clears away the blank userid, realm, and passtoken elements
            // in a line credential for example which get sent back.
            //
            // The second pass removes containers which no longer (after the first
            // call) contain any child elements; the credentials element itself in
            // the above example.
            //
            //////////////////////////////////////////////////////////////////////
            removeEmptyNodes ( property );
            removeEmptyNodes ( property );

            // We get back elements from the form even if there is no value
            // specified for them.  Here we filter the blank ones out to save
            // database space.
            if ( this.isEmptySetting( property ) ) {
                continue;
            }

            Integer refPropertyID =
                new Integer ( property.getAttributeValue( "ref_property_id" ) );

            // This is for the N cardinalities.
            if ( property.getAttributeValue("id") != null ) {
                map.replaceAddProperty( refPropertyID,
                                        property.getAttributeValue("id"),
                                        property );
            }
            else {
                map.replaceAddAllProperties( refPropertyID, property);
            }
        }

        Element result = new Element ( "PROFILE" );

        for ( Iterator iValues = map.values().iterator(); iValues.hasNext(); ) {
            Element element = (Element) iValues.next();
            element = (Element)element.detach();
            result.addContent( element );
        }

        return m_xmlOut.outputString( result );
    }


    private void removeEmptyNodes ( Element node ) {

        if ( node.getChildren() == null &&
                ( node.getText() == null || node.getText().length() == 0 ) ) {
            node.getParent().removeContent( node );
            return;
        }

        Collection children = node.getChildren();

        for ( Iterator i = children.iterator(); i.hasNext(); ) {
            removeEmptyNodes ( (Element) i.next() );
        }
    }


    private boolean isEmptySetting ( Element property ) {
        String text = property.getTextTrim();
        boolean empty = false;
        if ( text ==  null || text.length() == 0 ) {
            empty = true;
        }

        Collection children = property.getChildren();
        if ( !children.isEmpty() ) {
            for ( Iterator i = children.iterator(); i.hasNext(); ) {
                Element child = (Element) i.next();
                if ( !isEmptySetting ( child ) )
                    empty = false;
            }
        }

        return empty;
    }

}
