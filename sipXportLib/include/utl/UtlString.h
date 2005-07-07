// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _UtlString_h_
#define _UtlString_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
#define DEFAULT_UTLSTRING_CAPACITY 100

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlString is a resizable string container which is also containable in 
 * any of the UtlContainable-based containers.
 */
class UtlString : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const char* ssNull;
    static const size_t UTLSTRING_NOT_FOUND;

    /**
     * Compare case sensitivity constants
     */
    typedef enum CompareCase
    {
        matchCase,    /** < Case sensitive comparison */
        ignoreCase    /** < Case insensitive comparison */
    } CompareCase;


    /**
     * Flags to the various strip methods
     */ 
    typedef enum StripType
    {
        leading=1,  /** < only strip from the beginning of the string */
        trailing,   /** < only strip from the end of the string */
        both        /** < strip from both ends of the string */
    } StripType;

/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlString();

    
    /**
     * Constructor accepting an initial capacity.  The initial capacity is 
     * used to preallocate storage allowing for multiple appends without
     * the need to reallocate memory.
     */
    UtlString(int capacity);


    /**
     * Constructor accepting a null terminated source string.  The source
     * string is copied to this object.
     */
    UtlString(const char* szSource);

    /**
     * Constructor accepting a source string.  null terminations in
     * the source string are ignored.  length chars of the source
     * string are copied to this object.
     */
    UtlString(const char* szSource, size_t length);

    /**
     * Constructor accepting a reference to another UtlString.  The source
     * string is copied to this object.
     */
    UtlString(const UtlString& source);


    /**
     * Constructor accepting a reference to another string and a length.
     * Up to length characters are copied from the source string into
     * this object.  If length is greater than the length of source
     * this becomes a copy of source with the same data and length.
     */
    UtlString(const UtlString& source, size_t length);


    /**
     * Destructor
     */
    virtual ~UtlString();

/* ============================ MANIPULATORS ============================== */

    /**
     * Assignment operator; replace the contents of this string with the
     * designated null terminated string.
     */
    UtlString& operator=(const char* szStr);


    /**
     * Assignment operator; replace the contents of this string with the
     * designated string.
     */
    UtlString& operator=(const UtlString& str);


    /**
     * Plus equals operator; append the designated string to this string
     */
    UtlString& operator+=(const char *);


    /**
     * Plus equals operator; append the designated string to this string
     */
    UtlString& operator+=(const UtlString& s);


    /**
     * Plus equals operator; append the designated char to this string
     */
    UtlString& operator+=(const char c);


    /**
     * Get the character at position N
     */
    char operator()(size_t N);


    /**
     * Prepend the designated string to this beginning of this string
     */
    UtlString& prepend(const char* szStr);


    /**
     * Append the designated string to the end of this string
     */
    UtlString& append(const char* szStr);


    /**
     * Append the designated character to the end of this string
     */
    UtlString& append(const char c);


    /**
     * Append upto N bytes of the designated string to the end of this
     * string.
     */
    UtlString& append(const char* szStr, size_t N);


    /**
     * Append the designated string to the end of this string.
     */
    UtlString& append(const UtlString& str);


    /**
     * Insert the designated string starting at character position.  If
     * an invalid position is specified, nothing is performed.
     *
     * @param position starting character position in this string to insert at.
     */
    UtlString& insert(size_t position, const UtlString& src);

    /**
     * Insert the designated char starting at character position.  If
     * an invalid position is specified, nothing is performed.
     *
     * @param position starting character position in this char to insert at.
     * @param newChar the character to insert
     */
    UtlString& insert(size_t position, const char newChar);

    /**
     * Insert the designated string starting at character position.  If
     * an invalid position is specified, nothing is performed.
     *
     * @param position starting character position in this string to insert at.
     * @param src null terminated string to insert at
     * this given position.
     */
    UtlString& insert(size_t position, const char* src);

    /**
     * Insert the designated string starting at character position.  If
     * an invalid position is specified, nothing is performed.
     *
     * @param position starting character position in this string to insert at.
     * @param src string (which may contain null chars) to insert at
     * this given position.
     * @param sourceLength the number of characters to be copied (null
     * chars are ignored).
     */
    UtlString& insert(size_t position, const char* src, size_t sourceLenth);

    /**
     * Remove all characters after the specified position.  Nothing is performed 
     * if the position is invalid.
     */
    UtlString& remove(size_t pos);


    /**
     * Remove N characters from this string starting at designated position.
     * Invalid position or length results in no changes.
     */
    UtlString& remove(size_t pos, size_t N);


    /**
     * Replace N characters starting at the designated position with the 
     * designated replacement string.  Invalid position or length results
     * in no changes.
     * 
     * @param pos starting postion of the replacement
     * @param N number of characters to replace
     * @param replaceStr target replacement string
     */
    UtlString& replace(size_t pos, size_t N, const char* replaceStr);
    
    /**
     * Replace N characters starting at the designated position with the 
     * designated replacement string.  Invalid position or length results
     * in no changes.
     * 
     * @param pos starting postion of the replacement
     * @param N number of characters to replace
     * @param replaceStr target replacement string
     */
    UtlString& replace(size_t pos, size_t N, const UtlString& replaceStr);
    
    /**
     * Replace N characters starting at the designated position with a subset
     * of the designated replacement string.  Invalid position or length results 
     * in no changes.
     * 
     * @param pos starting postion of the replacement
     * @param N number of characters to replace
     * @param replaceStr target replacement string
     * @param L maximum number of characters of the replacement string to use.
     */
    UtlString& replace(size_t pos, size_t N, const char* replaceStr, size_t L);


    /**
     * Replace a single character at the designated position.
     * @param pos      offset into data to replace - must be < the length
     * @param newChar  character to be put into that offset
     */
    void UtlString::replaceAt(size_t pos, char newChar);

	/**
	 * Replace all instances of character src with character tgt
	 */
	UtlString& replace(const char src, const char tgt);
	

    /**
     * Removes whitespace (space, tab, Cr, Lf) from the end of the 
     * string
     */
    UtlString strip();


    /**
     * Removes whitespace (space, tab, Cr, Lf) from the beginning of the string,
     * from the end of the string, or from both the beginning and end of the 
     * string.
     */
    UtlString strip(StripType );


    /**
     * Remove the designated character from the beginning of the string, 
     * from the end of the string, or from both the beginning and end of 
     * the string.
     */
    UtlString strip(StripType , char );


    /**
     * Convert the string to all lower case characters (e.g. AbC1 -> abc1)
     */
    void toLower();


    /**
     * Convert the string to all upper case characters (e.g. AbC1 -> ABC1)
     */
    void toUpper();


    /**
     * Resize the string to the specified size.  If the requested size is less
     * then the the current size (string length), the string will be truncated.
     * If larger, the string will be padded with nulls.
     */
    void resize(size_t );


    /**
     * Set the string's storage capacity to the designated value.  This does
     * not modify the string, but rather adjusts the dynamic memory allocated 
     * for this string.  A size less then the current string length is 
     * ignored.
     * 
     * @return new capacity, which may be equal or greater the input one in
     *         case of success, and less than the input one in case of failure.
     */
    size_t capacity(size_t );    

    
    /**
     * Addition operator a = b + c 
     */
    friend UtlString operator+(const UtlString& , const UtlString& );


    /**
     * Addition operator a = b + c 
     */
    friend UtlString operator+(const UtlString& ,  const char* );


    /**
     * Addition operator a = b + c 
     */
    friend UtlString operator+(const char* , const UtlString& );

/* ============================ ACCESSORS ================================= */

    /**
     * Find the first instance of the designated character or UTL_NOT_FOUND if 
     * not found.
     */
    size_t first(char c) const;


    /**
     * Find the first instance of the designated string or UTL_NOT_FOUND if not
     * found.
     */
    size_t first(const char* ) const;


    /**
     * Return the string length
     */
    size_t length() const;


    /**
     * Return a read-only pointer to the underlying data.  This pointer should
     * not be stored.
     */
    const char* data() const;


    /**
     * Return the index of the designated character or UTL_NOT_FOUND  if not 
     * found.
     */
    size_t index(char ) const;


    /**
     * Return the index of the designated character starting at the 
     * designated postion or UTL_NOT_FOUND if not found. 
     */
    size_t index(char , size_t ) const;


    /**
     * Return the index of the designated substring or UTL_NOT_FOUND  if not
     * found.
     */
    size_t index(const char* ) const;


    /**
     * Return the index of the designated substring starting at the
     * designated position or UTL_NOT_FOUND if not found.
     */
    size_t index(const char* , size_t ) const;


    /**
     * Return the index of the designated substring starting at the
     * designated position or UTL_NOT_FOUND  if not found.  This method
     * also allows the ability to specify case insensitive or sensitive
     * match.
     */
    size_t index(const char* , size_t , CompareCase ) const;

    /**
     * Return the index of the designated substring or UTL_NOT_FOUND  if not
     * found. Optimization to avoid calling strlen and safe to use with
     * binary or opaque data.
     */
    size_t index(const UtlString& ) const;


    /**
     * Return the index of the designated substring starting at the
     * designated position or UTL_NOT_FOUND if not found.
     * Optimization to avoid calling strlen and safe to use with
     * binary or opaque data.
     */
    size_t index(const UtlString& , size_t ) const;


    /**
     * Return the index of the designated substring starting at the
     * designated position or UTL_NOT_FOUND  if not found.  This method
     * also allows the ability to specify case insensitive or sensitive
     * match.  Optimization to avoid calling strlen and safe to use with
     * binary or opaque data.
     */
    size_t index(const UtlString& , size_t , CompareCase ) const;


    /**
     * Find the last instance of the designated string or UTL_NOT_FOUND  if
     * not found
     */
    size_t last(char s) const;


    /**
     * Return the storage capacity allocated for this string
     */
    size_t capacity() const;


    /**
     * Return a subset of this string.  An empty string is returned if the 
     * start and/or len is invalid.
     * 
     * @param start Starting character position
     * @param len Number of characters to copy or -1 for "the rest of the
     *        string"
     */
    UtlString operator() (size_t start, size_t len) const;


    /* UtlString operator()(const RWCRegexp& re) const ; */


    /**
     * Cast this object to a const char* pointer.  This effectively is the 
     * same as calling data().
     * 
     * @see data()
     */
    operator const char*() const;


    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const;


    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

    /**
     * Not equals operator
     */
    friend UtlBoolean operator!=(const char *, const UtlString& );


    /**
     * Not equals operator
     */
    friend UtlBoolean operator!=(const char , const UtlString& );
    
    /**
     * Equals operator
     */
    friend UtlBoolean operator==(const char , const UtlString& );

    friend UtlBoolean operator==(const char *, const UtlString& );

    UtlBoolean operator==(const char *) const;

    UtlBoolean operator!=(const char *) const;

    UtlBoolean operator==(const UtlString&) const;

    UtlBoolean operator!=(const UtlString&) const;


    /**
     * Compare this object to another like-objects.  Results for 
     * designating a non-like object is not equal.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const;
    

    /**
     * Test this object to another like-object for equality.  This method 
     * returns false if unlike-objects are specified.
     */
    virtual UtlBoolean isEqual(UtlContainable const *) const;
    

    /**
     * Compare this object to the specified string (case sensitive).
     * 
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    int compareTo(const char *) const;


    /**
     * Compare this object to the specified string with option of forcing
     * either a case insensitive compare of a case sensitive compare.
     * 
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    int compareTo(UtlString const *, CompareCase type) const;


    /**
     * Compare this object to the specified string with option of forcing
     * either a case insensitive compare of a case sensitive compare.
     * 
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    int compareTo(const char *, CompareCase type) const;


    /**
     * Return true if this string contains the specified string.
     */
    UtlBoolean contains(const char *) const;


    /**
     * Return true if this is a empty (or null) string.
     */
    UtlBoolean isNull() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    char*  mpData;      //: The value of UtlString.
    size_t mSize;       //: The number of bytes of data used.
    size_t mCapacity;   //: The allocated size of data.
    size_t mIncrement;  //: Used as the minimum capacity resize step.
       
};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlString_h_
