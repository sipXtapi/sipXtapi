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
package com.pingtel.pds.pgs.jsptags.util;

import java.util.Enumeration;

import javax.servlet.ServletContext;
import javax.servlet.ServletConfig;
import javax.servlet.jsp.PageContext;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.tagext.BodyTagSupport;

public abstract class ExBodyTagSupport 
    extends BodyTagSupport  {

    /** 
     * Make the tag run its state cleanup method.
     */    
    public int doEndTag() 
        throws JspException 
    {
        clearServiceState();
        return super.doEndTag();
    }

    /** 
     * Make the tag run its properties cleanup method.
     *
     * In some cases will also run the state cleanup method
     */    
    public void release()
    {
        clearServiceState();
        clearProperties();
        super.release();
    }
        
    public void resetCustomProperties()
    {
        clearProperties();
    }
    
    /** 
     * Clear the tag's properties.
     */
    protected void clearProperties()
    {
    }
        
    /** 
     * Clear the tag's service state.
     */
    protected void clearServiceState()
    {
    }

    protected void log(String msg)
    {
        getServletContext().log(msg);
    }
    
    protected void log(String msg, 
                       Throwable t)
    {
        getServletContext().log(msg, t);
    }    
    
    protected String getInitParameter(String name) 
    {
        return getInitParameter(name, 
                                PageContext.APPLICATION_SCOPE);
    }
    
    protected String findInitParameter(String name) 
    {
        String value = getInitParameter(name, 
                                PageContext.PAGE_SCOPE);
        if(null != value) {
            return value;
        }

        return getInitParameter(name, 
                                PageContext.APPLICATION_SCOPE);
    }

    protected Enumeration getInitParameterNames() 
    {
        return getInitParameterNamesForScope(PageContext.APPLICATION_SCOPE);
    }
    
    protected String getInitParameter(String name, 
                                      int scope)
    {
        switch(scope) {
            case PageContext.PAGE_SCOPE:
                return getServletConfig().getInitParameter(name);

            case PageContext.APPLICATION_SCOPE:
                return getServletContext().getInitParameter(name);

            default:
                throw new IllegalArgumentException("Illegal scope");
        }        
    }

    protected Enumeration getInitParameterNamesForScope(int scope)    
    {
        switch(scope) {
            case PageContext.PAGE_SCOPE:
                return getServletConfig().getInitParameterNames();

            case PageContext.APPLICATION_SCOPE:
                return getServletContext().getInitParameterNames();

            default:
                throw new IllegalArgumentException("Illegal scope");
        }        
    }
  
    protected ServletContext getServletContext()
    {
        return pageContext.getServletContext();
    }
  
    protected ServletConfig getServletConfig()
    {
        return pageContext.getServletConfig();
    }
}
