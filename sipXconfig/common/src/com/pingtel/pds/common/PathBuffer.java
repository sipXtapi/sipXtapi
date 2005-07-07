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

package com.pingtel.pds.common;


/**
 * Helper for constructing file system paths
 */
public class PathBuffer
{
    private static final String FILE_SEPARATOR = System.getProperty("file.separator");

    private StringBuffer m_sb = new StringBuffer();

    public PathBuffer() {}

    /** @param path  initial path to start */
    public PathBuffer(String path) 
    {
        append(path);
    }

    /** append given string with no special processing */
    public PathBuffer append(String s)
    {
        m_sb.append(s);
        return this;
    }

    /** @return path so far */
    public String toString()
    {
        return m_sb.toString();
    }

    /** 
     * calls slash() then appends the given directory
     * @param directory to append
     */
    public PathBuffer dir(String dir)
    {
        slash().append(dir);
        return this;
    }


    /**
     * Append file separator to end
     */
    public PathBuffer slash()
    {
        m_sb.append(FILE_SEPARATOR);
        
        return this;
    }
}
