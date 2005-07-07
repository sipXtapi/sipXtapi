/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/DialingTemplateMatch.java#2 $
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
 * @author Robert J. Andreasen, Jr.
 */
public class DialingTemplateMatch
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String m_strTemplate ;          // Original Dialing Template
    private String m_strSource ;            // Source String
    private String m_strRendered = null ;   // Rendered (applied) String


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor requiring both a template adn source string
     */
    protected DialingTemplateMatch(String strTemplate,
                                   String strSource)
    {
        m_strTemplate = strTemplate ;
        m_strSource = strSource ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the original template used to render
     */
    public String getTemplate()
    {
        return m_strTemplate ;
    }


    /**
     * Get the source string that to be rendered
     */
    public String getSource()
    {
        return m_strSource ;

    }


    /**
     * Get the rendered (applied) String
     */
    public String getRenderedString()
    {
        if ((m_strRendered == null) && (m_strTemplate != null))
            m_strRendered = render(m_strSource, m_strTemplate) ;

        return m_strRendered ;
    }


    /**
     * Determine the character position where the iCharIndex's char would be
     * placed in this matches' template.
     *
     * @return the position of the specified index or -1 if it would not be
     *         appended to the string
     */
    public int getIndex(int iCharIndex)
    {
        int iRC = -1 ;

        if (m_strTemplate != null)
        {
            iRC = index(iCharIndex, m_strTemplate) ;
        }
        return iRC ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
   private String render(String strSource, String strTemplate)
    {
        StringBuffer strResults = new StringBuffer() ;

        boolean bEscapeChar = false ;
        boolean bOutOfData = false ;

        int iSourceLength = 0 ;
        int iSourcePosition = 0 ;

        if (m_strSource != null)
            iSourceLength = strSource.length() ;

        for (int i=0; i<strTemplate.length() && !bOutOfData; i++)
        {
            char ch = strTemplate.charAt(i) ;
            if (bEscapeChar)
            {
                bEscapeChar = false ;
                strResults.append(ch) ;
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
                        if (iSourcePosition < iSourceLength)
                        {
                            strResults.append(strSource.charAt(iSourcePosition++)) ;
                        }
                        else
                        {
                            bOutOfData = true ;
                        }
                        break ;
                    default:
                        strResults.append(ch) ;
                        break ;
                }
            }
        }

        // Append any additional characters not handled in the source string ...
        if (iSourcePosition < iSourceLength)
        {
            strResults.append(strSource.substring(iSourcePosition)) ;
        }

        return strResults.toString() ;
    }


    /**
     * Determine the character position where the iCharIndex's char would be
     * placed using the specified template.
     *
     * @return the position of the specified index or -1 if it would not be
     *         appended to the string
     */
    private int index(int iCharIndex, String strTemplate)
    {
        int iIndex = 0 ;

        if (strTemplate != null)
        {
            boolean bEscapeChar = false ;

            for (int i=0; i<strTemplate.length() && (iCharIndex >= 0); i++)
            {
                char ch = strTemplate.charAt(i) ;
                if (bEscapeChar)
                {
                    iIndex++ ;
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
                            iIndex++ ;
                            iCharIndex-- ;
                            break ;
                        default:
                            iIndex++ ;
                            break ;
                    }
                }
            }
        }
        return (iIndex+iCharIndex)-1 ;
    }
}