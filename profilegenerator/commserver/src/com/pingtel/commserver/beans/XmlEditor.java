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

import org.jdom.JDOMException;
import org.jdom.Document;
import org.jdom.output.XMLOutputter;
import org.jdom.input.SAXBuilder;

import java.io.*;

/**
 * Read and Writes XML files to support web gui of editing raw XML
 *
 * @author dhubler
 */
public class XmlEditor 
{
    /** xml stream */
    private String m_content;

    /** after validated */
    private String m_errorMessage;

    /** if stream is valid */
    private boolean m_valid = false;

    /** DOM object, valid after validate */
    private Document m_doc;

    /** 
     * entire xml stream as a string 
     */
    public void setContent(String content)
    {
        m_content = content;
        validate();
    }

    /**
     * @return null if not set
     */
    public String getContent()
    {
        return m_content;
    }

    /**
     * Run content thru XML routines
     */
    private void validate()
    {
        m_valid = false;
        if (m_content == null)
        {
            m_errorMessage = "empty content";            
        }
        else
        {
            try 
            {
                SAXBuilder builder = new SAXBuilder();
                m_doc = builder.build(new StringReader(m_content));
                m_valid = true;
            }
            catch (IOException e)
            {
                m_errorMessage = e.getMessage();
            }
            catch (JDOMException e)
            {
                m_errorMessage = e.getMessage();
            }
        }
    }

    /**
     * Is the XML set durring <code>setContent</code> valid
     */
    public boolean isValid()
    {
        return m_valid;
    }

    /**
     * After call to validate
     */
    public String getErrorMessage()
    {
        return m_errorMessage;
    }

    public void load(String filename)
        throws IOException
    {
        FileReader rdr = new FileReader(filename);
        BufferedReader buffRdr = new BufferedReader(rdr);
        String line = null;
        StringBuffer buff = new StringBuffer(1024);
        String eol = System.getProperty("line.separator");
        while ((line = buffRdr.readLine()) != null)
        {
            buff.append(line).append(eol);
        }

        setContent(buff.toString());
    }

    /**
     * Save xml content to file
     */
    public void save(String filename)
        throws IOException
    {
        if (!isValid())
            throw new IOException(getErrorMessage());
        
        FileOutputStream stream = new FileOutputStream(filename);
        org.jdom.output.Format f = org.jdom.output.Format.getPrettyFormat();        
        XMLOutputter outputter = new XMLOutputter( f );
        outputter.output(m_doc, stream);
        stream.flush();
        stream.close();
    }
}
