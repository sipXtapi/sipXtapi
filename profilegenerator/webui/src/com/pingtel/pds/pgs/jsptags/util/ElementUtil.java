/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/ElementUtil.java#5 $
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

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import org.jdom.Element;
import com.pingtel.pds.common.ElementUtilException;

/**
 * <p>Title: DMSTags</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public final class ElementUtil {
    private static ElementUtil g_instance = null;

    /** Singleton private constructor */
    private ElementUtil() {}

    /**
     * Singleton Accessor
     *
     * @return the Singleton
     */
    public static ElementUtil getInstance() {
        if (g_instance == null) {
            g_instance = new ElementUtil();
        }
        return g_instance;
    }

    /**
     * This helper method creates a JDOM Element container for an EJB or any other
     * object passed in.  The method introspects the gettable methods and
     * created lower case element names from them and stores them in an outer element
     * which is returned to the user.  The algorithm also filters out EJB specific
     * attributes (Primary Key, Handle and EJBHome as these are not part of the
     * business attributes)
     *
     * @param userObject
     *
     * @return an element containing field mapped sub-elements.
     */
    public Element createElementFromObject (String nodeElementName, Object userObject)
        throws ElementUtilException {
        // this is the collection class group
        Element groupElement = new Element ( nodeElementName );

        Method[] methods = userObject.getClass().getDeclaredMethods();
        try {
            for (int i=0; i<methods.length; i++) {
                String methodName = methods[i].getName();
                int getPos = methodName.indexOf("get");
                boolean skip = false;
                if ( methodName.equals("getEJBHome") ||
                     methodName.equals("getHandle") ||
                     methodName.equals("getPrimaryKey") ) {
                     skip = true;
                }

                // The Method starts with get<Field>, note that the
                // EJB Object fields must be excluded from this
                // these are 'getHome' 'getHandle' and 'getPrimaryKey'
                if ( (getPos != -1) && !skip ) {
                    String fieldName = methodName.
                        substring(getPos + 3).toLowerCase();
                    Element fieldElement = new Element(fieldName);
                    // Get takes no parameters and returns an Object (integer or string)
                    Object fieldValue = methods[i].invoke(userObject, new Object[0]);
                    // Only set the attribuge value if it is not null
                    if (fieldValue != null)
                        fieldElement.setText(fieldValue.toString());

                    groupElement.addContent(fieldElement);
                }
            }
        } catch (Exception ex) {
            throw new ElementUtilException (ex.getMessage());
        }
        return groupElement;
    }

    /**
     * All Elements in the Collection are nodes in a tree.  The order of
     * these nodes is not specified.
     *
     * @return the root node for a JDOM document
     */
    public Element sortCollection ( String rootElementName,
                                    String nodeElementName,
                                    Collection input )
        throws ElementUtilException  {
        // Populate a convenience HashMap with the ID/UserGroup Element pairs
        if (input != null) {
            // This is where we build the XML output from a the roots collection
            Element rootElement = new Element( rootElementName );

            HashMap elementMap = new HashMap ( input.size() );
            try {
                for ( Iterator iter = input.iterator(); iter.hasNext(); ) {
                    Object next = iter.next();
                    // Using Reflection Get the ID of the Object
                    Method getIDMethod = next.getClass().getDeclaredMethod( "getID", new Class[0] );
                    Object idObject = getIDMethod.invoke(next, new Object[0]);
                    Element element = createElementFromObject( nodeElementName, next );
                    elementMap.put( (Integer)idObject, element );
                }
            } catch ( Exception ex ) {
                // Wrap the exception in our specialized exception
                throw new ElementUtilException (ex.getMessage());
            }
            // Create an initially empty all branches collection, each branch should
            // consist of a branch specific root node.  This is not the global root
            // node as these are specific to an organization
            ArrayList allBranches = new ArrayList();

            // Iterate through the Elements  building Branches as we do
            for ( Iterator iter =  elementMap.values().iterator(); iter.hasNext(); ) {
                Element nextElement = (Element)iter.next();
                // build the branches recusively moving the results to the roots
                buildTreeFromElement ( nextElement, elementMap, allBranches );
            }

            // Add all branches to the Document
            for ( Iterator iter = allBranches.iterator(); iter.hasNext(); ) {
                Element nextElement = (Element)iter.next();
                rootElement.addContent( nextElement );
            }
            return rootElement;
        }
        return null;
    }


    /**
     * This Method inserts the userObject into a hierarchical tree and returns the
     * TreeNode containing the userObject
     *
     * @param userGroup this is the object we need to put into the tree structure
     * @param mappedCollection a hashmap of ID/Object pairs, note that the Object in this
     * case must support a getParent method.
     *
     * @return the node containing the UserGroup
     * @exception RemoteException
     */
    private Element buildTreeFromElement ( Element inputElement,
                                           HashMap elementMap,
                                           ArrayList allBranches )
        throws ElementUtilException {
        // search all branches starting from their roots to see if the
        // Element has been processed and moved
        Element element = findElement ( inputElement, allBranches );

        if ( element != null ) {
            // Found the containing node so return it
            return element;
        } else {
            // Crude Method to search an Element for a contained element
            Integer parentID = null;
            String parentIDStringVal = inputElement.getChild( "parentid" ).getText();
            if ( parentIDStringVal != "" ) {
                parentID = new Integer ( parentIDStringVal );
            }

            // if there is no parent add it to the list of branch roots
            // returning this element (since it has been now mapped)
            if ( parentID == null ) {
                // this is really a root node
                allBranches.add( inputElement );
                return inputElement;
            } else {
                // Get the parent element to search for in the branches
                Element parentElement = (Element) elementMap.get( parentID );
                // This recursive method will eventually return with parentElement
                // there is no point in doing anything with the return value as
                // a result of this.
                buildTreeFromElement ( parentElement,
                                       elementMap,
                                       allBranches );
                // Add this node as a child of the parent
                parentElement.addContent( inputElement );
            }
            return inputElement;
        }
    }

    /**
     * Searches through all Roots down through their children to find a
     * matching userObject
     *
     * @param userObject
     * @param allRootNodes
     *
     * @return the parent container for the inputElement
     */
    private Element findElement ( Element inputElement, Collection allBranches ) {
        // Start Searching all the RootNodes, assume does not have parent
        // => this is a root element until we can show otherwise
        for (Iterator iter = allBranches.iterator(); iter.hasNext(); ) {
            // Start at the root
            Element nextBranch = ( Element )iter.next();
            Element result = findElementInBranch ( inputElement, nextBranch );
            // break out when we find our first match
            if (result != null)
                return result;
        }
        // Failed to find match in any tree
        return null;
    }

    /**
     * Searches the branch (Top down for a matching Element)
     *
     * @param startNode
     * @param userObject
     *
     * @return
     */
    private Element findElementInBranch ( Element inputElement, Element searchBranch ) {
        // Double equals is the correct one to use here as we are using a single copy
        // of the collection class
        if ( searchBranch == inputElement ) {
            return inputElement;
        } else {
            // Recursively search only "group" children elements of this Element
            for (Iterator iter = searchBranch.getChildren("group").iterator(); iter.hasNext(); ) {
                Element nextGroupElement = (Element) iter.next();
                Element result = findElementInBranch ( inputElement, nextGroupElement );
                // only terminate the serarch at a match
                if ( result != null )
                    return result;
            }
        }
        // could find no match
        return null;
    }
}
