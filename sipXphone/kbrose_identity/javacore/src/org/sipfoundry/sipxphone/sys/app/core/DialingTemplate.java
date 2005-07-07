/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/DialingTemplate.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.app.core ;

import java.util.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.util.* ;

import com.oroinc.text.regex.* ;

/**
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DialingTemplate
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final boolean DEBUG = false ; // Are we in a debugging mode


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Vector of dialing match templates */
    Vector m_vTemplates ;
    /** Hashtable pre-compiled match patterns */
    Hashtable m_htCompiledPatterns ;
    /** perl5 style regex matcher */
    private PatternMatcher m_matcher ;
    /** perl5 style regex compiler */
    private PatternCompiler m_compiler ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor
     */
    public DialingTemplate()
    {
        m_htCompiledPatterns = new Hashtable() ;
        m_vTemplates = new Vector() ;
        m_matcher = new Perl5Matcher() ;
        m_compiler = new Perl5Compiler() ;

        initialize() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Apply a dialing string template to the designated source string
     *
     * @param strSource the unformatted source string
     * @
     */
    public synchronized DialingTemplateMatch apply(String strSource)
    {
        Vector vTemplates = getMatchingTemplates(strSource) ;
        String strShortest = null;
        int    iLength = Integer.MAX_VALUE ;


        System.out.println("Matching Templates: " ) ;
        System.out.println(vTemplates) ;

        // Find the shortest template
        for (int i=0; i<vTemplates.size(); i++)
        {
            String strTemplate = (String) vTemplates.elementAt(i) ;

            if (strTemplate.length() < iLength)
            {
                strShortest = strTemplate ;
                iLength = strTemplate.length() ;
            }
        }
        return new DialingTemplateMatch(strShortest, strSource) ;
    }





//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Initialize list of templates from either a hard coded list (DEBUG mode)
     * or the contents of the user-config/pinger-config.
     */
    private synchronized void initialize()
    {
        if (DEBUG)
        {
            // PHONESET_DIAL_TEMPLATE.1.  : 1-xxx-xxxx
            // PHONESET_DIAL_TEMPLATE.9.  : 9,xxx-xxx-xxxx
            // PHONESET_DIAL_TEMPLATE.91. : 9,1-xxx-xxx-xxxx
            // PHONESET_DIAL_TEMPLATE.xxxxx.   : xxx-xxxx
            // PHONESET_DIAL_TEMPLATE.x.   : xxxx
            // PHONESET_DIAL_TEMPLATE.xxxxxxx.   : xxx-xxx-xxxx \\xxxxx

            addMatchTemplate("1.",  "1-xxx-xxx-xxxx") ;
            addMatchTemplate("91.", "9,1-xxx-xxx-xxxx") ;
            addMatchTemplate(".",   "xxx-xxx-xxxx") ;
            addMatchTemplate(".",   "xxx-xxxx") ;
            addMatchTemplate(".",   "xxxx") ;
            addMatchTemplate(".",   "xxx-xxx-xxxx \\xxxxx") ;
        }
        else
        {
            Hashtable configFile = PingerConfig.getInstance().getConfig() ;

            System.out.println("") ;
            System.out.println("Initializing Dialing Template") ;
            System.out.println("") ;

            final String PHONESET_DIAL_TEMPLATE = "PHONESET_DIAL_TEMPLATE." ;

            Enumeration configFileKeys = configFile.keys() ;
            while (configFileKeys.hasMoreElements())
            {
                String key = (String) configFileKeys.nextElement();

                if (key.startsWith(PHONESET_DIAL_TEMPLATE))
                {
                    String strMatch = key.substring(PHONESET_DIAL_TEMPLATE.length()) ;
                    String strTemplate = (String) configFile.get(key) ;

                    addMatchTemplate(strMatch, strTemplate) ;
                }
            }
        }
        System.out.println("") ;
    }


    /**
     * Adds a match string / template to our list.
     */
    private void addMatchTemplate(String strMatch, String strTemplate)
    {
        String strMatchString = buildMatchString(strMatch) ;
        System.out.println("\t" + strMatch + "\t" + strTemplate + "\t" + strMatchString) ;
        m_vTemplates.addElement(new icTemplateContainer(strMatchString, strTemplate)) ;
    }


    /**
     * Converts a MGCP digit string into a perl5 regular expression
     */
    private String buildMatchString(String strSource)
    {
        // Escape Stars
        strSource = simpleSubsitutation(strSource, "\\*", "\\*") ;

        // Expand x to [0123456789aAbBcCdD#\*]
        strSource = simpleSubsitutation(strSource, "x", "[0-9ABCD#\\*]") ;

        // turn . into .*
        strSource = simpleSubsitutation(strSource, "\\.", ".*") ;

        return strSource ;
    }



    /**
     * Subsitute strMatch with strSubstituation in strSource.
     */
    private String simpleSubsitutation(String strSource, String strMatch, String strSubstituation)
    {
        Substitution substituation = new Perl5Substitution(strSubstituation) ;

        String strRC = strSource ;
        Pattern pattern ;

        try
        {
            pattern = m_compiler.compile(strMatch);
            strRC = Util.substitute(m_matcher, pattern, substituation, strSource, Util.SUBSTITUTE_ALL) ;
        }
        catch(MalformedPatternException e)
        {
            System.out.println("Bad pattern: " + strMatch) ;
            SysLog.log(e) ;
        }

        return strRC ;
    }


    /**
     * The the maximum number of digits within the template
     */
    private int getMaxDigitsInTemplate(String strTemplate)
    {
        int iRC = 0 ;

        if (strTemplate != null)
        {
            boolean bEscapeChar = false ;
            for (int i=0; i<strTemplate.length(); i++)
            {
                char ch = strTemplate.charAt(i) ;
                if (bEscapeChar)
                {
                    bEscapeChar = false ;
                }
                else
                {
                    switch (ch)
                    {
                        case '\\':
                            bEscapeChar = true ;
                            break ;
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                        case '#':
                        case '*':
                        case 'X':
                        case 'x':
                            iRC++ ;
                            break ;
                        default:
                            break ;
                    }
                }
            }
        }
        return iRC ;
    }


    /**
     * Get the cached/pre-compiled perl pattern for the specified match
     */
    private Pattern getPattern(String strMatch)
    {
        Pattern pattern = null ;

        pattern = (Pattern) m_htCompiledPatterns.get(strMatch) ;
        if (pattern == null)
        {
            try
            {
                pattern = m_compiler.compile(strMatch) ;
                m_htCompiledPatterns.put(strMatch, pattern) ;
            }
            catch(MalformedPatternException e)
            {
                System.out.println("Bad pattern: " + strMatch) ;
                SysLog.log(e) ;
            }
        }
        return pattern ;
    }


    /**
     * Get all of the templates that match the specified source
     */
    private Vector getMatchingTemplates(String strSource)
    {
        Vector vMatchingTemplates = new Vector() ;

        Enumeration enum = m_vTemplates.elements() ;
        while (enum.hasMoreElements())
        {
            icTemplateContainer holder = (icTemplateContainer) enum.nextElement() ;
            String strMatch = holder.strMatch ;
            String strTemplate = holder.strTemplate ;

            Pattern pattern = getPattern(strMatch) ;
            if (pattern != null)
            {
                if (m_matcher.matches(strSource, pattern))
                {
                    if (strSource.length() <= getMaxDigitsInTemplate(strTemplate))
                    {
                        vMatchingTemplates.addElement(strTemplate) ;
                    }
                    else
                        System.out.println(strSource + " !=  " + strMatch + ": too long") ;
                }
                else
                    System.out.println(strSource + " !=  " + strMatch + " no match") ;

            }
            else
            {
                // On error, drop the match/template
                vMatchingTemplates.removeElement(strMatch) ;
                m_vTemplates.removeElement(holder) ;
                m_htCompiledPatterns.remove(strMatch) ;
            }
        }
        return vMatchingTemplates ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner/Nested classes
////

    /**
     * Container for a match and template pair
     */
    private class icTemplateContainer
    {
        String strMatch ;
        String strTemplate ;

        icTemplateContainer(String strMatch, String strTemplate)
        {
            this.strMatch = strMatch ;
            this.strTemplate = strTemplate ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Test Main
////
    public static void main(String[] args)
    {
        DialingTemplate template = new DialingTemplate() ;

        DialingTemplateMatch match = template.apply(args[0]) ;

        System.out.println("source="+args[0]) ;
        System.out.println("template="+match.getTemplate()) ;
        System.out.println("rendered="+match.getRenderedString()) ;
    }
}