// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie (dpetrie AT SIPez DOT com)


#ifndef _PidfBody_h_
#define _PidfBody_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <net/HttpBody.h>
#include <utl/UtlDList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! class to contain an PIDF presense state body
/*! 
 */
class PidfBody : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

    //! Default constructor
    PidfBody(const char* resourceAor = NULL);

    //! Construct an PidfBody from body bytes
    PidfBody(const char* bytes, 
             int length,
             const char* contentEncodingValueString);

    //! Copy constructor
    PidfBody(const PidfBody& rPidfBody);

    //! Destructor
    virtual ~PidfBody();

/* ============================ MANIPULATORS ============================== */

    //! Assignment operator
    PidfBody& operator=(const PidfBody& rhs);

    //! Render the PIDF data into the XML format of the body
    /*! The PidfBody MUST be rendered if you set or change any
     *  of the PIDF information.  getBytes does this automatically
     *  if the data has changed.
     */
    void render();


    static int parseXmlToPidfProperties(const char* bytes, 
                                        int length, 
                                        const char* contentEncodingValueString,
                                        UtlString& entityAor,
                                        UtlDList& tuples);

/* ============================ ACCESSORS ================================= */

    //! Get the AOR (PIDF presence element, entity attribute) for the PIDF doc.
    void getEntityAor(UtlString& entityAor) const;

    //! get the body content of the PIDF body.
    /*! Renders the body into the mBody member if PIDF content
     *  has been changed via the accessors.
     */
    virtual void getBytes(const char** bytes, int* length) const;
    virtual void getBytes(UtlString* bytes, int* length) const;

       //! Get the presense status/state
    /*! Get the value of the status element of the tuple indicated by
     *  tupleIndex.
     *  \param tupleIndex - indicates which tuple to get the status from
     *  \param tuple - the value of the tupleId attribute of the indicated tuple
     *  \param basicStatus - the value of the <basic> element in the <status>
     *         element of the tuple.
     */
    UtlBoolean getBasicStatus(int tupleIndex, 
                                UtlString& tupleId, 
                                UtlBoolean& isTupleStatusBasicOpen,
                                UtlString& contact,
                                UtlString& tupleNote) const;

    //! Set the presence status/state
    void addBasicStatus(const UtlString& tupleId, 
                        const UtlBoolean& isTupleStatusBasicOpen,
                        const UtlString& contact,
                        const UtlString& tupleNote);

    //! Change the basic element value in the indicated tuple element
    UtlBoolean changeBasicStatus(int tupleIndex,
                                 UtlBoolean isTupleStatusBasicOpen);

/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // These will probably need to be lists
    UtlString mEntityAor; // Address of Record
    UtlDList mTuples;

    UtlBoolean mRendered;  // Has been rendered to XML body string in mBody

    void renderPidfXmlHeader(const UtlString& entityAor);
    void renderPidfTuple(const UtlString& tupleId,
                         const UtlBoolean isTupleStatusBasicOpen,
                         const UtlString& tupleContact,
                         const UtlString& tupleNote);
    void renderPidfXmlFooter();

    void copyTuples(const UtlDList& sourceList, UtlDList& targetList);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PidfBody_h_
