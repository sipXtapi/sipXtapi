/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/DatabaseVersionTag.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.patch.PatchManager;
import com.pingtel.pds.pgs.patch.PatchManagerHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class DatabaseVersionTag extends StyleTagSupport {

    private PatchManagerHome m_pmHome;

    public int doStartTag() throws JspException {
        try {
            PatchManager advocate = null;

            if ( m_pmHome == null ) {
                m_pmHome = ( PatchManagerHome )
                                    EJBHomeFactory.getInstance().getHomeInterface(  PatchManagerHome.class,
                                                                                    "PatchManager" );

                advocate = m_pmHome.create();
            }


            Integer currentVersion = new Integer ( "0" );

            currentVersion = advocate.getDatabaseVersion();

            Element root = new Element ( "dbversioninfo" );
            root.addContent( new Element ( "currentversion" ).setText( currentVersion.toString() ) );

            Element patchInfoElement = advocate.getUpgradeInfo();
            Collection versions = patchInfoElement.getChildren( "version" );
            Element newVersions = new Element ( "newversions" );
            root.addContent( newVersions );

            if ( currentVersion.intValue() != 0 ) {

                Element runPatches = new Element ( "appliedpatches" );
                root.addContent( runPatches );

                for ( Iterator iVersion = versions.iterator(); iVersion.hasNext(); ) {
                    Element versionElement = (Element) iVersion.next();
                    Integer versionNumber =
                        Integer.valueOf ( versionElement.getAttributeValue( "number" ) );

                    String status = "N";

                    Collection patches = versionElement.getChildren( "patch" );

                    for ( Iterator iPatch = patches.iterator(); iPatch.hasNext(); ) {
                        Element patchElement = (Element) iPatch.next();

                        Integer patchNumber =
                            Integer.valueOf( patchElement.getAttributeValue( "number" ) );

                        status = advocate.getPatchStatus( patchNumber );
                        String patchDescription = patchElement.getAttributeValue( "description");

                        Element patchStatus = new Element ( "patch" );
                        patchStatus.addContent( new Element ("number").setText( patchNumber.toString() ) );
                        patchStatus.addContent( new Element ("status").setText( status ) );
                        patchStatus.addContent( new Element ("description").setText( patchDescription ) );
                        runPatches.addContent( patchStatus );
                    }

                } // for

                Element newVersion = new Element ( "newversion" );
                newVersions.addContent( newVersion );
                Element number = new Element ( "versionnumber" );
                newVersion.addContent( number );
            } // if
            else {
                for ( Iterator iVersion = versions.iterator(); iVersion.hasNext(); ) {
                    Element versionElement = (Element) iVersion.next();
                    Integer versionNumber =
                        Integer.valueOf ( versionElement.getAttributeValue( "number" ) );

                    Element newVersion = new Element ( "newversion" );
                    newVersions.addContent( newVersion );
                    newVersion.setText( versionNumber.toString() );
                }
            }

            outputTextToBrowser ( root );
        }

        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }

     /**
      * Post Process the XML and insert friendly sub elements where
      * we have primmary keys
      */
    protected Element postProcessElement( Element inputElement ) {
        return inputElement;
    }

    protected void clearProperties() {
        super.clearProperties();
    }
}