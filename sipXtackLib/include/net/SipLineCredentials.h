//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////



#if !defined(AFX_SIPLINECREDENTIALS_H__8B43463B_8F7F_426B_94F5_B60164A76DF3__INCLUDED_)
#define AFX_SIPLINECREDENTIALS_H__8B43463B_8F7F_426B_94F5_B60164A76DF3__INCLUDED_


#include <net/Url.h>
#include <net/HttpMessage.h>

class SipLineCredentials : public UtlString
{
public:

        SipLineCredentials(const UtlString realm,
                const UtlString userId,
                const UtlString passwordToken,
                const UtlString type);

        virtual ~SipLineCredentials();
        void getRealm(UtlString* realm);
        void getUserId(UtlString* UserId);
        void getPasswordToken(UtlString* passToken);
        void getType(UtlString* type);

private:

        UtlString mType;
        UtlString mPasswordToken;
        UtlString mUserId;
        UtlString mRealm;

};

#endif // !defined(AFX_SIPLINECREDENTIALS_H__8B43463B_8F7F_426B_94F5_B60164A76DF3__INCLUDED_)
