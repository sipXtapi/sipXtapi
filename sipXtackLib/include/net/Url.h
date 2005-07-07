// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Url_h_
#define _Url_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>
#include <utl/UtlDList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class NameValuePair ;

//! URL parser and constructor
/*! This object is used to parse and construct URL strings.  This object
 * contains all of the parsed components of a URL.  It has the ability
 * to construct a serialized string of the object using the toString()
 * method.  It can also be used as a parser using the constructor 
 * which accepts a string as input.  This is intended to be a
 * generic URL parser for all schema/protocol types.  It is currently
 * tested and known to work for sip, sips, http, https and file type
 * URLs. There are accessors for the various parts of the URL as defined
 * below:
 * - display name - getDisplayName(), setDisplayName()
 * - protocol - getUrlType(), setUrlType()
 * - user - getUserId(), setUserId()
 * - password - getPassword(), setPassword()
 * - host - getHostAddress(), setHostAddress()
 * - port - getHostPort(), setHostPort()
 * - URL parameter names and values - getUrlParameter(), getUrlParameters(), setUrlParameter(), removeUrlParameter(), removeUrlParameters()
 * - header (or CGI) parameter names and values - getHeaderParameter(), getHeaderParameters(), setHeaderParameter(), removeHeaderParameter(), removeHeaderParameters()
 * - field parameter names and values - getFieldParameter(), getFieldParameters(), setFieldParameter(), removeFieldParameter(), removeFieldParameters()
 * \par
 * These parts appear in URLs such as the following:
 * "display name"<protocol:user:password@host:port;urlparm=value?headerParam=value>;fieldParam=value
 */

class Url
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //! Default constructor from string
    /*! \param urlString - optional string to parse URL from
     * \param isAddrSpec - TRUE if addrSpec, FALSE if nameAddr. Typically
     *           TRUE if this is a URI, FALSE if this is a URL from a header field
     */
    Url(const char* urlString = NULL, UtlBoolean isAddrSpec = FALSE);

    //! Copy constructor
    Url(const Url& rUrl);

    //! Destructor
    virtual
    ~Url();

/* ============================ MANIPULATORS ============================== */

     //! Assignment operator
    Url& operator=(const Url& rhs);

    //! Assignment from string
    /*! Parse the given null terminated string and set the
     * URL parts as found in the string.
     */
    Url& operator=(const char* urlString);

    //! Serialize this URL to a string
    void toString(UtlString& urlString) const;

    //! Serialize this URL to a string
    UtlString toString() const;

    //! Debug dump to STDOUT
    void dump();

    //! Clear the contents of this URL
    void reset();

    //! Remove all of the URL, header and field parameters and values
    void removeParameters();

/* ============================ ACCESSORS ================================= */
    //! Construct and cannonical identity 
    /*! In some applications this is used to compare if this
     * URL refers to the same distination.  The identity is
     * constructed as:
     * \par
     * "user@host:port"
     */
    void getIdentity(UtlString& identity) const;

    //! Get the URL application layer protocol 
    void getUrlType(UtlString& urlProtocol) const;

    //! Set the URL application layer protocol
    void setUrlType(const char* urlProtocol);

    //! Get the URL display name if present
    void getDisplayName(UtlString& displayName) const;

    //! Set the URL display name
    void setDisplayName(const char* displayName);

    //! Get the URL user identity if present
    void getUserId(UtlString& userId) const;

    //! Set the URL user identity
    void setUserId(const char* userId);

    //! Get the users password if present in the URL
    UtlBoolean getPassword(UtlString& userId) const;

    //! Set the users password in the URL
    void setPassword(const char* userId);

    //! Get the URL host name or IP address
    void getHostAddress(UtlString& address) const;

    //! Set the URL host name or IP address
    void setHostAddress(const char* address);

    //! Get the URL host port
    int getHostPort() const;

    //! Set the URL host port
    void setHostPort(int port);

    //! Get the file path from the URL
    /*! \param getStyle TRUE will put header (or CGI) parameters in path 
     *         in the format needed for an HTTP GET.  FALSE will 
     *         form the path without the header parameters as formated
     *         for a HTTP POST.
     */
    UtlBoolean getPath(UtlString& path, UtlBoolean getStyle = FALSE);


    //! Set the file path
    /*! \note the path should \a not contain header (or CGI) parameters
     */
    void setPath(const char* path);

    //! Get the named URL parameter value
    /*! \param name - the parameter name to get
     * \param value - the value of the named parameter
     * \param index - gets the \a index occurance of the named parameter 
     *        (the same parameter name may occur multiple times in the URL).
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getUrlParameter(const char* name, 
                              UtlString& value, 
                              int index = 0);

    //! Get the name and value of the URL parameter at the indicated
    //! index
    /*! \param urlIndex - the index indicting which URL parameter to 
     *          get (starting at 0 for the first one).
     * \param name - the parameter name at urlIndex
     * \param value - the value of the parameter at urlIndex
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getUrlParameter(int urlIndex, 
                              UtlString& name, 
                              UtlString& value);

    //! Set the named URL parameter to the given value
    /*! Adds the parameter if it does not exist, sets the value if
     * it does exist.
     * \param name - the parameter name
     * \param value - the value of the parameter
     */
    void setUrlParameter(const char* name, const char* value);

    //! Removes all of the URL parameters
    void removeUrlParameters();

    //! Removes all of the URL parameters with the given name
    void removeUrlParameter(const char* name);

    //! Gets all of the URL parameters and values
    /*! \param iMaxReturn (in) - the maximum number of items to return
     * \param pNames (out) - Pointer to a preallocated array of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated array of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the \a iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \returns TRUE if values are returned otherwise FALSE   
     */
    UtlBoolean getUrlParameters(int iMaxReturn, 
                               UtlString *pNames, 
                               UtlString *pValues, 
                               int& iActualReturn);

    //! Get the named header parameter value
    /*! \param name - the parameter name to get
     * \param value - the value of the named parameter
     * \param index - gets the \a index occurance of the named parameter 
     *        (the same parameter name may occur multiple times in the URL).
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getHeaderParameter(const char* name, 
                                 UtlString& value, 
                                 int index = 0);

    //! Get the name and value of the header parameter at the indicated
    //! index
    /*! \param headerIndex - the index indicting which header parameter to 
     *          get (starting at 0 for the first one).
     * \param name - the parameter name at headerIndex
     * \param value - the value of the parameter at headerIndex
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getHeaderParameter(int headerIndex, 
                                 UtlString& headerName, 
                                 UtlString& headerValue);

    //! Set the named header parameter to the given value
    /*! Adds the parameter if it does not exist, sets the value if
     * it does exist.
     * \param name - the parameter name
     * \param value - the value of the parameter
     */
    void setHeaderParameter(const char* name, 
                            const char* value);

    //! Removes all of the header parameters
    void removeHeaderParameters();

    //! Removes all of the header parameters with the given name
    void removeHeaderParameter(const char* name);

    //!Gets all of the Header parameters
    /*! \param iMaxReturn (in) - the maximum number of items to return
     * \param pNames (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \return TRUE if values are returned otherwise FALSE
     */
    UtlBoolean getHeaderParameters(int iMaxReturn, UtlString *pNames, UtlString *pValues, int& iActualReturn);


    //! Get the named field parameter value
    /*! \param name - the parameter name to get
     * \param value - the value of the named parameter
     * \param index - gets the \a index occurance of the named parameter 
     *        (the same parameter name may occur multiple times in the URL).
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getFieldParameter(const char* name, 
                                UtlString& value, 
                                int index = 0) const;

    //! Get the name and value of the field parameter at the indicated
    //! index
    /*! \param fieldIndex - the index indicting which field parameter to 
     *          get (starting at 0 for the first one).
     * \param name - the parameter name at fieldIndex
     * \param value - the value of the parameter at fieldIndex
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getFieldParameter(int fieldIndex, 
                                UtlString& name, 
                                UtlString& value);

    //! Set the named field parameter to the given value
    /*! Adds the parameter if it does not exist, sets the value if
     * it does exist.
     * \param name - the parameter name
     * \param value - the value of the parameter
     */
    void setFieldParameter(const char* name, 
                           const char* value);

    //! Removes all of the field parameters
    void removeFieldParameters();

    //! Removes all of the field parameters with the given name
    void removeFieldParameter(const char* name);

    //! Gets all of the Header parameters
    /*! \param iMaxReturn (in) - the maximum number of items to return
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \return TRUE if values are returned otherwise FALSE
     */
    UtlBoolean getFieldParameters(int iMaxReturn, 
                                 UtlString *pNames, 
                                 UtlString *pValues, 
                                 int& iActualReturn);

    //! Forces the presense of angle brackets (i.e. <>) in the URL 
    //! when serialized
    void includeAngleBrackets();

    //! Remove the angle brackets (i.e. <>) from the URL
    void removeAngleBrackets();

    //! Gets the serialized URL as a string (with no display name or 
    //! field parameters)
    void getUri(UtlString& Uri);

/* ============================ INQUIRY =================================== */

    //! Is string all digits
    /*! \param dialedCharacters - null terminated string containing 
     *       ascii test
     * \ return TRUE if the dialedCharacters are all digits
     */
   static UtlBoolean isDigitString(const char* dialedCharacters);

   //! Compare two URLs to see if the have the same user, host and port
   /* Assumes that no port set is the same as the default port for
    * the URL type/protocol.  Also assumes that host is \a not case 
    * sensative, but that user id \a is case sensative.
    * \return TRUE if the user Id, host and port are the same
    */
   UtlBoolean isUserHostPortEqual(const Url& uri) const ;

   //! Are angle brackets explicitly included
   /*! \note does not test if angle brackets are required or will be added
    * implicitly.
    * \return TRUE if angle brackets were found when parsing or if they are
    *         explicitly set to be inserted during serialization.
    */
   UtlBoolean isIncludeAngleBracketsSet() const ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void parseString(const char* urlString);

    UtlString mUrlType;
    UtlString mDisplayName;
    UtlString mUserId;
    UtlString mPassword;
    UtlBoolean mPasswordSet;
    UtlString mHostAddress;
    int mHostPort;
    UtlString mPath;
    UtlDList* mpUrlParameters;
    UtlDList* mpHeaderParameters;
    UtlDList* mpFieldParameters;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    UtlBoolean mAngleBracketsIncluded;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _Url_h_
