This is a fixed version of Hashtable.class. The better solution 
would be to put in pj.jar but for now, we are including it in 
pinbtel.jar until we update to new Personal Java. More on this is
described below:


Instructions to modify pj.jar
to fix a bug with Hashtable implementation of jdk1.1 which prevents

java.lang.ClassCastException: sun.misc.WeakCache$Value

from being thrown( in our case  when fetching images using
Toolkit.getDefaultToolkit.getImage( strImageName ) ).



Modifying pj.jar to fix a JDK bug with Hashtable.
==================================================


This document outlines the procedure to make the changes 
in  Hashtable.java in jdk1.1 branch to fix the Exception 
java.lang.ClassCastException: sun.misc.WeakCache$Value
you occasionally get when trying to load images using 
the code 
Toolkit.getDefaultToolkit.getImage( strImageName ).

Please refer to java bug ID 4089896 
( http://developer.java.sun.com/developer/bugParade/bugs/4089896.html )
for more info.

Download jdk1.1.8 from the Sun Java site
(http://java.sun.com/products/jdk/1.1/download-jdk-windows.html)
and install the source under a root directory like c:\, D:\ etc , not inside
a folder under the root folder. Because we need to preserve the package name 
when we add the classfile using WinZip and if its underneath something 
like c:\blahblah\java\util , it will also put blahblah as a package/folder 
name. 

Open java\util\Hashtable.java in a editor.
===============================================================
Basically, the changes are to be made inside the 
public synchronized Object put(Object key, Object value) method.

Instead of this block of code 
        -------------------------------------------------------
        if (count >= threshold) {
            // Rehash the table if the threshold is exceeded
            rehash();
            return put(key, value);
        } 
        ------------------------------------------------------

Use this block of code inside the "put" method. 

        if (count >= threshold) {
            // Rehash the table if the threshold is exceeded
            rehash();
            tab = table;
            index = (hash & 0x7FFFFFFF) % tab.length;
        } 

==================================================================


After making those changes,
Compile this new Hashtable.java using the classpath of jdk1.1 classes
ie by putting classes.zip in your classpath. And then replace the 
Hashtable.class inside pj.jar with this Hashtable.class.  










