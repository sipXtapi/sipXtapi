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
import java.util.Iterator;
import java.util.List;


/**
 * ProcessDialPlansBean replaces the dialplans tag in the destinations XML
 * file and regenerates the derived comm server config XML files.
 *
 * @author dbrown
 *
 */
public class ProcessDialPlansBean extends ProcessDestinationsBean {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mDID = null;
    private String mInternalextensions = null;
    private String mAutoattendant = null;
    private String mRetrievevoicemail = null;
    private String mPSTNprefix = null;
    private String mVMprefixfromextension = null;
    private String mIntldialprefix = null;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public ProcessDialPlansBean () { }


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    public String getInternalextensions() {
        return mInternalextensions;
    }

    public void setInternalextensions(String internalextensions ) {
        this.mInternalextensions = internalextensions;
    }
    public String getDid () {
        return mDID;
    }

    public void setDid (String did ) {
        this.mDID = did;
    }

    public String getAutoattendant() {
        return mAutoattendant;
    }

    public void setAutoattendant(String autoattendant) {
        this.mAutoattendant = autoattendant;
    }

    public String getRetrievevoicemail() {
        return mRetrievevoicemail;
    }

    public void setRetrievevoicemail(String retrievevoicemail) {
        this.mRetrievevoicemail = retrievevoicemail;
    }

    public String getPstnprefix() {
        return mPSTNprefix;
    }

    public void setPstnprefix(String pstnprefix) {
        this.mPSTNprefix = pstnprefix;
    }

    public String getVmprefixfromextension() {
        return mVMprefixfromextension;
    }

    public void setVmprefixfromextension(String vmprefixfromextension) {
        this.mVMprefixfromextension = vmprefixfromextension;
    }

    public String getIntldialprefix() {
        return mIntldialprefix;
    }

    public void setIntldialprefix(String intldialprefix) {
        this.mIntldialprefix = intldialprefix;
    }

    public void updateXML() throws RedirectServletException {
        FileOutputStream out = null;
        try {
            synchronized (mDestinationsFileLock) {
                Document doc = mSAXbuilder.build(new File( DESTINATIONS_FILE ));
                //mXMLOutputter.output( doc, System.out );
                List dialplans = doc.getRootElement().getChild("dialplans").getChildren("dialplan");
                Iterator dpIterator = dialplans.iterator();
                Element em = null;
                while ( dpIterator.hasNext() ) {
                    em = ( Element ) dpIterator.next();
                    if ( em.getChild("name").getText().equals("autoattendant") ) {
                        em.getChild("value").setText( mAutoattendant );
                    }
                    else if ( em.getChild("name").getText().equals("retrievevoicemail") ) {
                        em.getChild("value").setText( mRetrievevoicemail );
                    }
                    else if ( em.getChild("name").getText().equals("pstnprefix") ) {
                        em.getChild("value").setText( mPSTNprefix );
                    }
                    else if ( em.getChild("name").getText().equals("vmprefixfromextension") ) {
                        em.getChild("value").setText( mVMprefixfromextension );
                    }
                    else if ( em.getChild("name").getText().equals("intldialprefix") ) {
                        em.getChild("value").setText( mIntldialprefix );
                    }
                    else if ( em.getChild("name").getText().equals("internalextensions") ) {
                        em.getChild("value").setText( mInternalextensions );
                    }
                    else if ( em.getChild("name").getText().equals("did") ) {
                        em.getChild("value").setText( mDID );
                    }
                }
                out = new FileOutputStream( DESTINATIONS_FILE );
                org.jdom.output.Format f = org.jdom.output.Format.getPrettyFormat();        
                XMLOutputter xmlOut = new XMLOutputter( f );
                xmlOut.output( doc, out );
            }  // synchronized


            generateMappingRules2();
            generateAuthRules();
        }
        catch( Exception e ) {
            throw new RedirectServletException (
                    e.getMessage(), "../dialplan_details.jsp", null);
        }
        finally {
            if ( out != null ) {
                try {
                    out.flush();
                    out.close();
                }
                catch (IOException e) {
                    throw new RedirectServletException (
                            e.getMessage(), "../dialplan_details.jsp", null);
                }
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
