// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <os/OsStatus.h>
#include <sipdb/ResultSet.h>
#include <utl/UtlDefs.h>
#include "SipAaa.h"

class SipAaaTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipAaaTest);
   CPPUNIT_TEST(testIsAuthorized);
   CPPUNIT_TEST_SUITE_END();

public:
   void testIsAuthorized()
      {
         // Construct a SipAaa using the empty constructor so that we don't have to fire
         // up a SipUserAgent, which isn't needed.
         SipAaa* aaa = new SipAaa();

         // Authorization should succeed because all required permissions have been granted
         ResultSet huntingAndFishingPermissions;
         getPermissions(huntingAndFishingPermissions, true, true);
         UtlString matchedPermission;
         UtlString unmatchedPermissions;
         CPPUNIT_ASSERT(aaa->isAuthorized(
                           huntingAndFishingPermissions,    // requiredPermissions
                           huntingAndFishingPermissions,    // grantedPermissions
                           matchedPermission,
                           unmatchedPermissions));

         // Authorization should fail, not all required permissions have been granted
         ResultSet huntingPermissionOnly;
         getPermissions(huntingPermissionOnly, true, false);
         CPPUNIT_ASSERT(aaa->isAuthorized(
                           huntingAndFishingPermissions,    // requiredPermissions
                           huntingPermissionOnly,           // grantedPermissions
                           matchedPermission,
                           unmatchedPermissions));
      }
private:
   // Return hunting and/or fishing permissions in the ResultSet.
   // Use a dummy identity.
   void getPermissions(ResultSet& permissions, bool hunting, bool fishing)
      {
         if (hunting)
         {
            addPermission(permissions, "hunting");
         }
         if (fishing)
         {
            addPermission(permissions, "fishing");
         }
      }

   void addPermission(ResultSet& permissions, const char* permissionValue)
      {
         UtlHashMap record;
         record.insertKeyAndValue(new UtlString("identity"),
                                  new UtlString("dummy identity"));
         record.insertKeyAndValue(new UtlString("permission"),
                                  new UtlString(permissionValue));
         permissions.addValue(record);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipAaaTest);
