/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ReadLogFileTag.java#5 $
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

import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ByteArrayInputStream;

import javax.servlet.jsp.JspException;

import org.jdom.Element;
import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.common.ElementUtilException;

import com.pingtel.pds.common.PathLocatorUtil;


public class ReadLogFileTag extends StyleTagSupport {


    private SAXBuilder m_saxBuilder = new SAXBuilder();

    /**
     *
     * @return
     * @exception JspException
     */
    public int doStartTag() throws JspException {

        String fileName = null;

        try {
            fileName =
                PathLocatorUtil.getInstance().getPath(  PathLocatorUtil.LOGS_FOLDER,
                                                        PathLocatorUtil.PGS );

            fileName = fileName + "PDSMessages.log";
            File f = new File ( fileName );
            Element root = null;

            try {
                if ( !f.exists() )
                    throw new JspException ( "log file: " + fileName + " does not exist." );

                StringBuffer logFileContent = new StringBuffer();
                logFileContent.append( "<root>" );
                byte [] bytes = new byte [64000];

                FileInputStream fis = new FileInputStream ( f );

                while ( true ) {
                    int len = fis.read( bytes );
                    if ( len == -1 )
                        break;

                    logFileContent.append( new String ( bytes, 0, len ) );
                }

                logFileContent.append( "</root>" );

                Document doc =
                    m_saxBuilder.build(
                        new ByteArrayInputStream ( logFileContent.toString().getBytes() ) );

                root = doc.getRootElement();
                root.detach();
            }
            catch ( JDOMException ex ) {
                throw new JspException ( ex.toString() );
            }

            outputTextToBrowser ( root );
        }
        catch ( IOException ex ) {
            throw new JspException ( ex.toString() );
        }
        catch ( ElementUtilException ex ) {
            throw new JspException ( ex.toString() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        super.clearProperties();
    }
}
