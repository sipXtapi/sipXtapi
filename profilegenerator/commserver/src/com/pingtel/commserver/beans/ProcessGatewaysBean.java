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

package com.pingtel.commserver.beans;

import com.pingtel.pds.common.RedirectServletException;
import org.jdom.output.*;
import org.jdom.Document;
import org.jdom.Element;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Hashtable;

/**
 * ProcessGatewaysBean replaces the gateways tag in the destinations XML
 * file and regenerates the derived comm server config XML files.
 *
 * @author dbrown
 *
 */
public class ProcessGatewaysBean extends ProcessDestinationsBean {

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    // Gateway Hashtable definition
    private Hashtable mGWHT = new Hashtable();
    // Gateway Alternate(s) Hashtable definition
    private Hashtable mGWaltHT = new Hashtable();
    // Gateway Q-Value HashTable definition
    private Hashtable mGWqHT = new Hashtable();
    // Gateway seq number Hashtable definition
    private Hashtable mGWseqHT = new Hashtable();
    // Gateway label
    private Hashtable mGWlabelHT = new Hashtable();
    // Emergency Gateway Hashtable definition
    private Hashtable mEGWHT = new Hashtable();
    // Emergency Gateway Alternate(s) Hashtable definition
    private Hashtable mEGWaltHT = new Hashtable();
    // Emergency Gateway Q-Value Hashtable definition
    private Hashtable mEGWqHT = new Hashtable();
    // Emergency Gateway seq number Hashtable definition
    private Hashtable mEGWseqHT = new Hashtable();
    // Emergency Gateway label
    private Hashtable mEGWlabelHT = new Hashtable();


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public ProcessGatewaysBean () {}

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    // Set Gateway Hashtable
    public void setGwht ( Hashtable gwht ) {
        this.mGWHT = gwht;
    }

    // Get Gateway Hashtable
    public Hashtable getGwht () {
        return mGWHT;
    }

    // Get a Gateway from the Hashtable by key
    public String getGw ( String key ) {
        if ( mGWHT.get( key ) != null ) {
            return mGWHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Gateway Alt Hashtable
    public void setGwaltht( Hashtable gwaltht ) {
        this.mGWaltHT = gwaltht;
    }

    // Get Gateway Alt Hashtable
    public Hashtable getGwaltht () {
        return mGWaltHT;
    }

    // Get a Gateway Alt from the Hashtable by key
    public String getGwalt ( String key ) {
        if ( mGWaltHT.get( key ) != null ) {
            return mGWaltHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Gateway Q-Value Hashtable
    public void setGwqht ( Hashtable gwqht ) {
        this.mGWqHT = gwqht;
    }

    // Get Gateway Q-Value Hashtable
    public Hashtable getGwqht () {
        return mGWqHT;
    }

    // Get a Gateway Q-Value from the Hashtable by key
    public String getGwq ( String key ){
        if ( mGWqHT.get( key ) != null ){
            return mGWqHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Gateway seq Hashtable
    public void setGwseqht ( Hashtable gwseqht ) {
        this.mGWseqHT = gwseqht;
    }

    // Get Gateway seq Hashtable
    public Hashtable getGwseqht () {
        return mGWseqHT;
    }

    // Get a Gateway seq from the Hashtable by key
    public String getGwseq ( String key ){
        if ( mGWseqHT.get( key ) != null ){
            return mGWseqHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Gateway label Hashtable
    public void setGwlabelht ( Hashtable gwlabelht ) {
        this.mGWlabelHT = gwlabelht;
    }

    // Get Gateway label Hashtable
    public Hashtable getGwlabelHT () {
        return mGWlabelHT;
    }

    // Get a Gateway label from the Hashtable by key
    public String getGwlabel ( String key ){
        if ( mGWlabelHT.get( key ) != null ){
            return mGWlabelHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Emergency Gateway Hashtable
    public void setEgwht ( Hashtable egwht ) {
        this.mEGWHT = egwht;
    }

    // Get Emergency Gateway Hashtable
    public Hashtable getEgwht () {
        return mEGWHT;
    }

    // Get an Emergency Gateway from the Hashtable by key
    public String getEgw ( String key ) {
        if ( mEGWHT.get( key ) != null ) {
            return mEGWHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Emergency Gateway Alt Hashtable
    public void setEgwaltht( Hashtable egwaltht ) {
        this.mEGWaltHT = egwaltht;
    }

    // Get Emergency Gateway Alt Hashtable
    public Hashtable getEgwaltht () {
        return mEGWaltHT;
    }

    // Get an Emergency Gateway Alt from Hashtable by key
    public String getEgwalt ( String key ) {
        if ( mEGWaltHT.get( key ) != null ) {
            return mEGWaltHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Emergency Gateway Q-Value Hashtable
    public void setEgwqht ( Hashtable egwqht ) {
        this.mEGWqHT = egwqht;
    }

    // Get Emergency Gateway Q-Value Hashtable
    public Hashtable getEgwqht () {
        return mEGWqHT;
    }

    // Get an Emergency Gateway Q-Value from Hashtable by key
    public String getEgwq ( String key ){
        if ( mEGWqHT.get( key ) != null ){
            return mEGWqHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Emergency Gateway seq Hashtable
    public void setEgwseqht ( Hashtable egwseqht ) {
        this.mEGWseqHT = egwseqht;
    }

    // Get Emergency Gateway seq Hashtable
    public Hashtable getEgwseqht () {
        return mEGWseqHT;
    }

    // Get a Emergency Gateway seq from the Hashtable by key
    public String getEgwseq ( String key ){
        if ( mEGWseqHT.get( key ) != null ){
            return mEGWseqHT.get( key ).toString();
        }
        else {
            return "";
        }
    }

    // Set Emergency Gateway label Hashtable
    public void setEgwlabelht ( Hashtable egwlabelht ) {
        this.mEGWlabelHT = egwlabelht;
    }

    // Get Emergency Gateway label Hashtable
    public Hashtable getEgwlabelHT () {
        return mEGWlabelHT;
    }

    // Get a Emergency  label from the Hashtable by key
    public String getEgwlabel ( String key ){
        if ( mEGWlabelHT.get( key ) != null ){
            return mEGWlabelHT.get( key ).toString();
        }
        else {
            return "";
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    public void updateXML() throws RedirectServletException {
        FileOutputStream out = null;
        try {
            synchronized (mDestinationsFileLock) {
                Document doc = mSAXbuilder.build(new File( DESTINATIONS_FILE ));
                Element gateways = new Element( "gateways" );
                //Element gateway = new Element( "gateway" );
                //Element  primary = new Element( "primary_name" );
                //Element alias = new Element( "alias" );
                int idCounter = 0;
                Enumeration enum = null;
                Enumeration enumAlt = null;
                Enumeration enumQ = null;
                Enumeration enumLabel = null;
                Enumeration enumSeq = null;
                String key = null;
                String keyAlt = null;
                String keyQ = null;
                String keyLabel = null;
                String keySeq = null;

                //Start by checking to see if mGWHT is not null;
                if ( mGWHT != null ) {
                    enum = mGWHT.keys();
                    while ( enum.hasMoreElements() ) {
                        Element gateway = new Element( "gateway" );
                        Element primary = new Element( "primary_name" );
                        key = enum.nextElement().toString();
                        primary.setText( getGw( key ) );
                        gateway.addContent( primary );
                        gateway.setAttribute( "type", "normal" );
                        gateway.setAttribute( "id", ""+idCounter );
                        enumQ = mGWqHT.keys();
                        while ( enumQ.hasMoreElements() ) {
                            keyQ = enumQ.nextElement().toString();
                            if ( keyQ.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "qvalue", getGwq( keyQ ) );
                            }
                        }
                        enumLabel = mGWlabelHT.keys();
                        while ( enumLabel.hasMoreElements() ) {
                            keyLabel = enumLabel.nextElement().toString();
                            if ( keyLabel.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "label", getGwlabel( keyLabel ) );
                            }
                        }
                        enumSeq = mGWseqHT.keys();
                        while ( enumSeq.hasMoreElements() ) {
                            keySeq = enumSeq.nextElement().toString();
                            if ( keySeq.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "seq", getGwseq( keySeq ) );
                            }
                        }
                        enumAlt = mGWaltHT.keys();
                        while ( enumAlt.hasMoreElements() ) {
                            keyAlt = enumAlt.nextElement().toString();
                            if ( keyAlt.indexOf( key ) != -1 ) {
                                Element alias = new Element( "alias" );
                                alias.setText( getGwalt( keyAlt ) );
                                gateway.addContent( alias );
                            }
                        }
                        gateways.addContent( gateway );
                        ++ idCounter;
                    }
                }

                if ( mEGWHT != null ) {
                    enum = mEGWHT.keys();
                    while ( enum.hasMoreElements() ) {
                        Element gateway = new Element( "gateway" );
                        Element primary = new Element( "primary_name" );
                        key = enum.nextElement().toString();
                        primary.setText( getEgw( key ) );
                        gateway.addContent( primary );
                        gateway.setAttribute( "type", "emergency" );
                        gateway.setAttribute( "id", ""+idCounter );
                        enumQ = mEGWqHT.keys();
                        while ( enumQ.hasMoreElements() ) {
                            keyQ = enumQ.nextElement().toString();
                            if ( keyQ.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "qvalue", getEgwq( keyQ ) );
                            }
                        }
                        enumLabel = mEGWlabelHT.keys();
                        while ( enumLabel.hasMoreElements() ) {
                            keyLabel = enumLabel.nextElement().toString();
                            if ( keyLabel.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "label", getEgwlabel( keyLabel ) );
                            }
                        }
                        enumSeq = mEGWseqHT.keys();
                        while ( enumSeq.hasMoreElements() ) {
                            keySeq = enumSeq.nextElement().toString();
                            if ( keySeq.indexOf( key ) != -1 ) {
                                gateway.setAttribute( "seq", getEgwseq( keySeq ) );
                            }
                        }
                        enumAlt = mEGWaltHT.keys();
                        while ( enumAlt.hasMoreElements() ) {
                            keyAlt = enumAlt.nextElement().toString();
                            if ( keyAlt.indexOf( key ) != -1 ) {
                                Element alias = new Element( "alias" );
                                alias.setText( getEgwalt( keyAlt ) );
                                gateway.addContent( alias );
                            }
                        }
                        gateways.addContent( gateway );
                        ++ idCounter;
                    }
                }
                doc.getRootElement().removeChild( "gateways" );
                doc.getRootElement().addContent( gateways );
                out = new FileOutputStream( DESTINATIONS_FILE );
                org.jdom.output.Format f = org.jdom.output.Format.getPrettyFormat();        
                XMLOutputter xmlOut = new XMLOutputter( f );
                xmlOut.output( doc, out );
            }

            generateMappingRules2();
            generateAuthRules();
        }
        catch (Exception e) {
            throw new RedirectServletException (
                    e.getMessage(), "../gateway_details.jsp", null);
        }
        finally {
            if (out != null ) {
                try {
                    out.flush();
                    out.close();
                }
                catch (IOException e) {
                    throw new RedirectServletException (
                            e.getMessage(), "../gateway_details.jsp", null);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
