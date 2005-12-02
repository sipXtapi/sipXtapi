/* 
 * @(#)FlatCtx.java	1.4 99/10/15
 * 
 * Copyright 1997, 1998, 1999 Sun Microsystems, Inc. All Rights
 * Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free,
 * license to use, modify and redistribute this software in source and
 * binary code form, provided that i) this copyright notice and license
 * appear on all copies of the software; and ii) Licensee does not 
 * utilize the software in a manner which is disparaging to Sun.
 *
 * This software is provided "AS IS," without a warranty of any
 * kind. ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE 
 * HEREBY EXCLUDED.  SUN AND ITS LICENSORS SHALL NOT BE LIABLE 
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, 
 * MODIFYING OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN 
 * NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL,
 * CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER 
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT 
 * OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS 
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line
 * control of aircraft, air traffic, aircraft navigation or aircraft
 * communications; or in the design, construction, operation or
 * maintenance of any nuclear facility. Licensee represents and warrants
 * that it will not use or redistribute the Software for such purposes.  
 */
package org.sipfoundry.sipxphone.sys.jndi;

import javax.naming.*;
import java.util.*;
import java.io.Serializable;

import org.sipfoundry.util.* ; 

/**
 * This is a dummy class
 */

class Bindings extends Hashtable implements Serializable
{
}

/**
 * This is the SPI for JNDI that is integrated with the PropertyManager.
 *
 * JNDI is the Java Naming and Directory Interface that layers on
 * top of SPIs such as LDAP. We have provided our own SPI for JNDI
 * that talks to the PropertyManager. When the PropertyManager interfaces
 * with the ConfigManager, the directory we are providing in our SPI
 * will be Roamable, that is to say that directory information can
 * be propagated between phones that a user logs in to. The directory
 * information stored in this SPI is intended to be used for Roamable
 * personal directory information.
 *
 */

class FlatCtx implements Context {
    
    
    PropertyManager pm = PropertyManager.getInstance();
    
    Hashtable myEnv;
    static NameParser myParser = new FlatNameParser();
    
    private Bindings bindings = null;

    FlatCtx(Hashtable environment) {
        myEnv = (environment != null) ? (Hashtable)(environment.clone()) : null;

        Bindings dummyBindings = new Bindings();
        
        if (pm.exists(dummyBindings))
        {
            try
            {
                bindings = (Bindings) pm.getOwnedBean(dummyBindings);
                        
            } catch (Exception e) {
                System.out.println("Error getting the bindings " + e);
            }
            
        }
        else
        {
            bindings = dummyBindings;
        }
        
        pm.registerOwnedBean(this, bindings);
        
    }
    
    /**
     * This method is called each time that there is a change to
     * the directory. It tells the PropertyManager that there hs been a
     * change to the bindings.
     *
     * Todo: This is not an optimized implementation. It would be nice to
     * be able to batch these changes up, without losing them. The issue here
     * is that the current implementation of the PM will write out the entire
     * repository each time anything changes in it. Ick.
     */
    private void flushChanges()
    {
        try {
            pm.ownedBeanChanged(bindings);
        } catch (Exception e) {
            SysLog.log(e) ;            
        }
    }
    
    public Object lookup(String name) throws NamingException {
        if (name.equals("")) {
            // Asking to look up this context itself.  Create and return
            // a new instance with its own independent environment.
            return (new FlatCtx(myEnv));
        }
        Object answer = bindings.get(name);
        if (answer == null) {
            throw new NameNotFoundException(name + " not found");
        }
        return answer;
    }

    public Object lookup(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        return lookup(name.toString()); 
    }

    public void bind(String name, Object obj) throws NamingException {
        
        if (name.equals("")) {
            throw new InvalidNameException("Cannot bind empty name");
        }
        if (bindings.get(name) != null) {
            throw new NameAlreadyBoundException(
                    "Use rebind to override");
        }
        bindings.put(name, obj);
        flushChanges();
    }

    public void bind(Name name, Object obj) throws NamingException {
        // Flat namespace; no federation; just call string version
        bind(name.toString(), obj);
    }

    public void rebind(String name, Object obj) throws NamingException {
        if (name.equals("")) {
            throw new InvalidNameException("Cannot bind empty name");
        }
        bindings.put(name, obj);
        flushChanges();
    }

    public void rebind(Name name, Object obj) throws NamingException {
        // Flat namespace; no federation; just call string version
        rebind(name.toString(), obj);
    }

    public void unbind(String name) throws NamingException {
        if (name.equals("")) {
            throw new InvalidNameException("Cannot unbind empty name");
        }
        
        Object obj = bindings.remove(name);
        flushChanges();
        
        if (obj == null) {
            throw new InvalidNameException("Cannot find name " + name);
        }
    }

    public void unbind(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        unbind(name.toString());
    }

    public void rename(String oldname, String newname)
            throws NamingException {
        if (oldname.equals("") || newname.equals("")) {
            throw new InvalidNameException("Cannot rename empty name");
        }

        // Check if new name exists
        if (bindings.get(newname) != null) {
            throw new NameAlreadyBoundException(newname +
                                                " is already bound");
        }

        // Check if old name is bound
        Object oldBinding = bindings.remove(oldname);
        if (oldBinding == null) {
            throw new NameNotFoundException(oldname + " not bound");
        }

        bindings.put(newname, oldBinding);
        flushChanges();
    }

    public void rename(Name oldname, Name newname)
            throws NamingException {
        // Flat namespace; no federation; just call string version
        rename(oldname.toString(), newname.toString());
    }

    public NamingEnumeration list(String name)
            throws NamingException {
        
        if (name.equals("")) {
            // listing this context
            return new FlatNames(bindings.keys());
        } 

        // Perhaps 'name' names a context
        Object target = lookup(name);
        if (target instanceof Context) {
            return ((Context)target).list("");
        }
        throw new NotContextException(name + " cannot be listed");
    }

    public NamingEnumeration list(Name name)
            throws NamingException {
        
        // Flat namespace; no federation; just call string version
        return list(name.toString());
    }

    public NamingEnumeration listBindings(String name)
            throws NamingException {
        
        if (name.equals("")) {
            // listing this context
            return new FlatBindings(bindings.keys());
        } 

        // Perhaps 'name' names a context
        Object target = lookup(name);
        if (target instanceof Context) {
            return ((Context)target).listBindings("");
        }
        throw new NotContextException(name + " cannot be listed");
    }

    public NamingEnumeration listBindings(Name name)
            throws NamingException {
        
        // Flat namespace; no federation; just call string version
        return listBindings(name.toString());
    }

    public void destroySubcontext(String name) throws NamingException {
        throw new OperationNotSupportedException(
                "FlatCtx does not support subcontexts");
    }

    public void destroySubcontext(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        destroySubcontext(name.toString());
    }

    public Context createSubcontext(String name)
            throws NamingException {
        throw new OperationNotSupportedException(
                "FlatCtx does not support subcontexts");
    }

    public Context createSubcontext(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        return createSubcontext(name.toString());
    }

    public Object lookupLink(String name) throws NamingException {
        // This flat context does not treat links specially
        return lookup(name);
    }

    public Object lookupLink(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        return lookupLink(name.toString());
    }

    public NameParser getNameParser(String name)
            throws NamingException {
        return myParser;
    }

    public NameParser getNameParser(Name name) throws NamingException {
        // Flat namespace; no federation; just call string version
        return getNameParser(name.toString());
    }

    public String composeName(String name, String prefix)
            throws NamingException {
        Name result = composeName(new CompositeName(name),
                                  new CompositeName(prefix));
        return result.toString();
    }

    public Name composeName(Name name, Name prefix)
            throws NamingException {
        Name result = (Name)(prefix.clone());
        result.addAll(name);
        return result;
    }

    public Object addToEnvironment(String propName, Object propVal)
            throws NamingException {
        if (myEnv == null) {
            myEnv = new Hashtable(5, 0.75f);
        } 
        return myEnv.put(propName, propVal);
    }

    public Object removeFromEnvironment(String propName) 
            throws NamingException {
        if (myEnv == null)
            return null;

        return myEnv.remove(propName);
    }

    public Hashtable getEnvironment() throws NamingException {
        if (myEnv == null) {
            // Must return non-null
            return new Hashtable(3, 0.75f);
        } else {
            return (Hashtable)myEnv.clone();
        }
    }

    public String getNameInNamespace() throws NamingException {
        return ""; 
    }

    public void close() throws NamingException {
        myEnv = null;
        bindings = null;
    }

    // Class for enumerating name/class pairs
    class FlatNames implements NamingEnumeration {
        Enumeration names;

        FlatNames (Enumeration names) {
            this.names = names;
        }

        public boolean hasMoreElements() {
            return names.hasMoreElements();
        }

        public boolean hasMore() throws NamingException {
            return hasMoreElements();
        }

        public Object nextElement() {
            String name = (String)names.nextElement();
            String className = bindings.get(name).getClass().getName();
            return new NameClassPair(name, className);
        }

        public Object next() throws NamingException {
            return nextElement();
        }
        public void close() {
        }
    }

    // Class for enumerating bindings
    class FlatBindings implements NamingEnumeration {
        Enumeration names;

        FlatBindings (Enumeration names) {
            this.names = names;
        }

        public boolean hasMoreElements() {
            return names.hasMoreElements();
        }

        public boolean hasMore() throws NamingException {
            return hasMoreElements();
        }

        public Object nextElement() {
            String name = (String)names.nextElement();
            return new Binding(name, bindings.get(name));
        }

        public Object next() throws NamingException {
            return nextElement();
        }
        public void close() {
        }
    }
    
    private void dumpKeys()
    {
        Enumeration keyEnum = bindings.keys();
        
        while (keyEnum.hasMoreElements())
        {
            String key = (String) keyEnum.nextElement();
            System.out.println("    Key: " + key);
        }
    }
    
};
