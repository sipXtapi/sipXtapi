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

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.StringReader;

import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

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
        return escapeForBrowser( m_content );
    }

    private static String[][] REPLACEMENTS =
    {
        { "&lt;", "&amp;lt;" },
        { "&gt;", "&amp;gt;" },
    };
    
    /**
     * Browser is going to interpret HTML entities - need to escape them
     * @return escaped text
     */
    public static String escapeForBrowser( String input )
    {
        String output = input;
        for (int i = 0; i < REPLACEMENTS.length; i++) {
            String[] repl = REPLACEMENTS[i];
            String pattern = repl[0];
            String replacement = repl[1];
            output = output.replaceAll(pattern,replacement);
        }
        return output;
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
        FileReader rdr = null;
        try
        {
            rdr = new FileReader(filename);
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
        finally
        {
            if (rdr != null)
            {
                try
                {
                    rdr.close();
                }
                catch (IOException ignore) {}
            }
        }
    }

    /**
     * Save xml content to file. Will not let you save xml unless it's valid
     */
    public void save(String filename)
        throws IOException
    {
        if (!isValid())
            throw new IOException(getErrorMessage());
        
        FileOutputStream stream = null;
        try
        {
            stream = new FileOutputStream(filename);
            Format f = Format.getPrettyFormat();        
            XMLOutputter outputter = new XMLOutputter( f );
            outputter.output(m_doc, stream);
            stream.flush();
            stream.close();
        }
        finally
        {
            if (stream != null)
            {
                try
                {
                    stream.close();
                }
                catch (IOException ignore) {}
            }
        }
    }
}
