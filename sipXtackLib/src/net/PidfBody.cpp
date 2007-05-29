// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/PidfBody.h>
#include <os/OsSysLog.h>
#include <utl/UtlDListIterator.h>
#include <xmlparser/tinyxml.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES

class PidfTuple : public UtlString
{
public:
    PidfTuple(){};
    PidfTuple(const PidfTuple& rPidfTuple);
    virtual ~PidfTuple(){};
    PidfTuple& operator=(const PidfTuple& rhs);
    UtlBoolean mTupleStatusBasic;
    UtlString mTupleContact;  // TODO: should be an array
    UtlString mTupleNote;
};

// Copy constructor
PidfTuple::PidfTuple(const PidfTuple& rPidfTuple)
{
    append(rPidfTuple);
    mTupleStatusBasic = rPidfTuple.mTupleStatusBasic;
    mTupleContact = rPidfTuple.mTupleContact;
    mTupleNote = rPidfTuple.mTupleNote;
}

// Assignment operator
PidfTuple& PidfTuple::operator=(const PidfTuple& rPidfTuple)
{
    if(this != &rPidfTuple)
    {
        remove(0);
        append(rPidfTuple);
        mTupleStatusBasic = rPidfTuple.mTupleStatusBasic;
        mTupleContact = rPidfTuple.mTupleContact;
        mTupleNote = rPidfTuple.mTupleNote;

    }

    return(*this);
}
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default constructor 
PidfBody::PidfBody(const char* resourceAor) :
HttpBody()
{
    if(resourceAor)
    {
        mEntityAor = resourceAor;
    }

    mClassType = PIDF_BODY_CLASS;
    mRendered = FALSE;

    // Remove to make sure base class did not already set it
    remove(0);
    append(CONTENT_TYPE_PIDF);
}

// Constructor
PidfBody::PidfBody(const char* bytes, 
                     int length,
                     const char* contentEncodingValueString) :
                     HttpBody()
{
    mClassType = PIDF_BODY_CLASS;
    parseXmlToPidfProperties(bytes, length, contentEncodingValueString,
        mEntityAor, mTuples);
    mRendered = TRUE;

    mBody.append(bytes, length);
    bodyLength = length;

    // Remove to make sure base class did not already set it
    remove(0);
    append(CONTENT_TYPE_PIDF);
}

// Copy constructor
PidfBody::PidfBody(const PidfBody& rPidfBody)
{
    // Copy the parent
    *((HttpBody*)this) = rPidfBody;

    // Remove to make sure base class did not already set it
    remove(0);
    append(CONTENT_TYPE_PIDF);
    mClassType = PIDF_BODY_CLASS;
    mEntityAor = rPidfBody.mEntityAor;
    copyTuples(rPidfBody.mTuples, mTuples);
    mRendered = rPidfBody.mRendered;
}

// Destructor
PidfBody::~PidfBody()
{
    // Delete any Tuples in the list
    mTuples.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PidfBody&
PidfBody::operator=(const PidfBody& rPidfBody)
{
    if (this != &rPidfBody) // do not copy self
    {
        // Copy the parent
        *((HttpBody*)this) = rPidfBody;

        // Set the class type just 
        mClassType = PIDF_BODY_CLASS;
        *this = CONTENT_TYPE_PIDF;
        mClassType = PIDF_BODY_CLASS;
        mEntityAor = rPidfBody.mEntityAor;
        mTuples.destroyAll();
        copyTuples(rPidfBody.mTuples, mTuples);
    }
    return(*this);
}

void PidfBody::render()
{
    mBody.remove(0);

    // Add the common PIDF XML header
    renderPidfXmlHeader(mEntityAor);

    UtlDListIterator iterator(mTuples);

    // Loop through the Tuples and add to the body
    PidfTuple* tuple = NULL;
    while((tuple = (PidfTuple*) iterator()))
    {
        renderPidfTuple(*tuple,
                        tuple->mTupleStatusBasic,
                        tuple->mTupleContact,
                        tuple->mTupleNote);
    }

    // Add the common PIDF footer
    renderPidfXmlFooter();

    mRendered = TRUE;
}

int PidfBody::parseXmlToPidfProperties(const char* bytes, 
                                       int length, 
                                       const char* contentEncodingValueString,
                                       UtlString& entityAor,
                                       UtlDList& tuples)
{
    int tupleCount = 0;
    entityAor.remove(0);
    tuples.destroyAll();

    // Make sure we have a null terminated string
    UtlString byteString;
    byteString.append(bytes, length);

    // TODO: should pay attention to contentEncodingValueString

    TiXmlDocument pidfXmlDocument;
    pidfXmlDocument.Parse(byteString);

    // TODO: need to check for name spaces

    // Get the presense element and the AOR attribute
    TiXmlNode* presenceNode = pidfXmlDocument.FirstChild ("presence");
    if(presenceNode)
    {
        TiXmlElement* presenceElement = presenceNode->ToElement();
        if(presenceElement)
        {
            entityAor = presenceElement->Attribute("entity");
        }

        TiXmlNode *tupleNode = presenceNode->FirstChild("tuple");
        while(tupleNode)
        {
            PidfTuple* tuple = new PidfTuple;

            // Get the tuple id attribute
            TiXmlElement* tupleElement = tupleNode->ToElement();
            if(tupleElement)
            {
                tuple->append(tupleElement->Attribute("id"));
            }

            // The tuple should contain a <status>, one or more
            // <contacts> and optionally a <note>
            TiXmlNode* statusNode = tupleNode->FirstChild("status");
            if(statusNode)
            {
                TiXmlNode* basicNode = statusNode->FirstChild("basic");
                if(basicNode)
                {
                    UtlString basicString;
                    basicString = basicNode->FirstChild()->Value();
                    tuple->mTupleStatusBasic = FALSE;
                    if(basicString.compareTo("open", UtlString::ignoreCase) == 0)
                    {
                        tuple->mTupleStatusBasic = TRUE;
                    }                    
                }
            }
            TiXmlNode* contactNode = tupleNode->FirstChild("contact");
            if(contactNode)
            {
                tuple->mTupleContact = contactNode->FirstChild()->Value();
            }
            TiXmlNode* noteNode = tupleNode->FirstChild("note");
            if(noteNode)
            {
                tuple->mTupleNote = noteNode->FirstChild()->Value();
            }

            // Add the tuple to the list
            tuples.append(tuple);

            // check for next tuple
            tupleNode = presenceNode->NextSibling("tuple");
        }
    }

    return(tupleCount);
}

/* ============================ ACCESSORS ================================= */

void PidfBody::getEntityAor(UtlString& entityAor) const
{
    entityAor = mEntityAor;
}

void PidfBody::getBytes(const char** bytes, int* length) const
{
    if(!mRendered)
    {
        (*((PidfBody*)this)).render();
    }

    HttpBody::getBytes(bytes, length);
}

void PidfBody::getBytes(UtlString* bytes, int* length) const
{
    if(!mRendered)
    {
        (*((PidfBody*)this)).render();
    }

    HttpBody::getBytes(bytes, length);
}

UtlBoolean PidfBody::getBasicStatus(int tupleIndex, 
                                    UtlString& tupleId, 
                                    UtlBoolean& isTupleStatusBasicOpen,
                                    UtlString& contact,
                                    UtlString& tupleNote) const
{
    PidfTuple* tuple = (PidfTuple*) mTuples.at(tupleIndex);
    if(tuple)
    {
        tupleId = *tuple;
        isTupleStatusBasicOpen = tuple->mTupleStatusBasic;
        contact = tuple->mTupleContact;
        tupleNote = tuple->mTupleNote;
    }
    else
    {
        tuple->remove(0);
        isTupleStatusBasicOpen = FALSE;
        contact = "";
        tupleNote = "";
    }

    return(tuple != NULL);
}

UtlBoolean PidfBody::changeBasicStatus(int tupleIndex, 
                                       UtlBoolean isTupleStatusBasicOpen)
{
    PidfTuple* tuple = (PidfTuple*) mTuples.at(tupleIndex);
    if(tuple)
    {
        tuple->mTupleStatusBasic = isTupleStatusBasicOpen;
        mRendered = FALSE;
    }

    return(tuple != NULL);
}

void PidfBody::addBasicStatus(const UtlString& tupleId, 
                            const UtlBoolean& isTupleStatusBasicOpen,
                            const UtlString& contact,
                            const UtlString& tupleNote)
{
    PidfTuple* tuple = new PidfTuple();
    tuple->append(tupleId);
    tuple->mTupleStatusBasic = isTupleStatusBasicOpen;
    tuple->mTupleContact = contact;
    tuple->mTupleNote = tupleNote;

    mTuples.append(tuple);
    mRendered = FALSE;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/*
   <?xml version="1.0" encoding="UTF-8"?>
   <impp:presence xmlns:impp="urn:ietf:params:xml:ns:pidf"
       entity="pres:someone@example.com">
     <impp:tuple id="sg89ae">
       <impp:status>
         <impp:basic>open</impp:basic>
       </impp:status>
       <impp:contact priority="0.8">tel:+09012345678</impp:contact>
     </impp:tuple>
   </impp:presence>

   , using a default XML namespace:

   <?xml version="1.0" encoding="UTF-8"?>
   <presence xmlns="urn:ietf:params:xml:ns:pidf"
       entity="pres:someone@example.com">
     <tuple id="sg89ae">
       <status>
         <basic>open</basic>
       </status>
       <contact priority="0.8">tel:+09012345678</contact>
     </tuple>
   </presence>
*/

// Quick and dirty implementation of XML rendering
void PidfBody::renderPidfXmlHeader(const UtlString& entityAor)
{
    mBody = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"";
    mBody.append(entityAor);
    mBody.append("\">\n");
    bodyLength = mBody.length();
}

void PidfBody::renderPidfTuple(const UtlString& tupleId,
                         const UtlBoolean isTupleStatusBasicOpen,
                         const UtlString& tupleContact,
                         const UtlString& tupleNote)
{
    mBody.append("  <tuple id=\"");
    mBody.append(tupleId);
    mBody.append("\">\n    <status>\n      <basic>");

    // status basic
    if(isTupleStatusBasicOpen)
    {
        mBody.append("open");
    }
    else
    {
        mBody.append("closed");
    }
    mBody.append("</basic>\n    </status>\n  ");

    // contact
    if(!tupleContact.isNull())
    {
        mBody.append("  <contact>");
        mBody.append(tupleContact);
        mBody.append("</contact>\n");
    }

    // note
    if(!tupleNote.isNull())
    {
        mBody.append("    <note>");
        mBody.append(tupleNote);
        mBody.append("  </note>\n");
    }

    // End tuple
    mBody.append("  </tuple>\n");
    bodyLength = mBody.length();
}

void PidfBody::renderPidfXmlFooter()
{
    mBody.append("</presence>\n");
    bodyLength = mBody.length();
}

void PidfBody::copyTuples(const UtlDList& sourceList, UtlDList& targetList)
{
    UtlDListIterator iterator(sourceList);

    // Loop through the Tuples copy to target list
    PidfTuple* tuple = NULL;
    while((tuple = (PidfTuple*) iterator()))
    {
        PidfTuple* tupleCopy = new PidfTuple(*tuple);
        targetList.append(tupleCopy);
    }
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

