//
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "utl/UtlSListIterator.h"
#include "net/XmlRpcDispatch.h"
#include "configrpc/ConfigRPC.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATICS

OsRWMutex* ConfigRPC::spDatabaseLock = new OsRWMutex(OsBSem::Q_PRIORITY);
UtlHashBag ConfigRPC::sDatabases;
bool       ConfigRPC::sRegistered = false;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/// Construct an instance to allow RPC access to a database.
ConfigRPC::ConfigRPC( const char*         dbName     ///< dbName known to XMLRPC methods
                     ,const char*         versionId  ///< version of this database
                     ,const UtlString&    dbPath     ///< path to persistent store for this db
                     ,ConfigRPC_Callback* callback   ///< connection to controlling application
                     )
   :UtlString(dbName)
   ,mVersion(versionId)
   ,mPath(dbPath)
   ,mCallback(callback)
{
   assert(dbName && *dbName != '\000');
   assert(versionId && *versionId != '\000');
   assert(!dbPath.isNull());
   assert(callback);
   
   OsWriteLock lock(*spDatabaseLock);

   if ( ! sDatabases.find(this) )
   {
      sDatabases.insert(this);
   }
   else
   {
      OsSysLog::add( FAC_KERNEL, PRI_CRIT, "ConfigRPC:: duplicate db name '%s'", dbName);
   }
}

OsStatus ConfigRPC::load(OsConfigDb& dataset)
{
   OsStatus status = dataset.loadFromFile(mPath);
   if ( OS_SUCCESS != status )
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "ConfigRPC failed to load '%s' from '%s'",
                    data(), mPath.data()
                    );
   }
   return status;
}


OsStatus ConfigRPC::store(OsConfigDb& dataset)
{
   OsStatus status = dataset.storeToFile(mPath);
   if ( OS_SUCCESS != status )
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "ConfigRPC failed to store '%s' to '%s'",
                    data(), mPath.data()
                    );
   }
   return status;
}

/// Destroy the instance to disconnect access to the database.
ConfigRPC::~ConfigRPC()
{
   OsWriteLock lock(*spDatabaseLock);
   sDatabases.remove(this);
}

/*****************************************************************
 * Default ConfigRPC_Callback
 *****************************************************************/

ConfigRPC_Callback::ConfigRPC_Callback()
{
}
   
// Access check function 
XmlRpcMethod::ExecutionStatus ConfigRPC_Callback::accessAllowed(
   const HttpRequestContext& requestContext,
   Method                    method
                                                                )
{
   return XmlRpcMethod::OK; // :TODO: should check request context and do something smarter
}

/// Invoked after the database has been modified
void ConfigRPC_Callback::modified()
{
}
   
/// Invoked after the database has been deleted
void ConfigRPC_Callback::deleted()
{
}

ConfigRPC_Callback::~ConfigRPC_Callback()
{
}


/// Implements the XML-RPC method configurationParameter.version
/**
 * Inputs:
 *  string    dbName           Name of the database.
 *  Outputs:
 *  string    version_id       The version identifier.
 *
 *  This method should be called by a configuration application to
 *  confirm that it is using a compatible definition of the database
 *  definition.  If the returned version_id does not match what the 
 *  configuration application expects, it should not modify the configuration..
 */
class ConfigRPC_version : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new ConfigRPC_version();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);
            ConfigRPC* db = ConfigRPC::find(*dbName);

            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Version);
               if ( XmlRpcMethod::OK == status )
               {
                  response.setResponse(&db->mVersion);
               }
            }
            else
            {
               UtlString faultMsg;               
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }
         
         return true;
      }
};

/// Implements the XML-RPC method configurationParameter.set method
/**
 *  Parameters  Type      Name             Description
 *  Inputs:
 *              string    db_name          configuration data set name
 *              struct
 *                string  parameter      parameter name (key)
 *                string  value          parameter value
 *    ...
 *  Outputs:
 *     none
 *
 *  Sets each 'parameter' / 'value' pair in 'db_name'.  Either all
 *  sets are made or none are made.
 */
class ConfigRPC_set : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get()
      {
         return new ConfigRPC_set();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);

            ConfigRPC* db = ConfigRPC::find(*dbName);
            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Set);
               if ( XmlRpcMethod::OK == status )
               {
                  // :TODO: actually get the database and set stuff
               }
            }
            else
            {
               UtlString faultMsg;               
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }
         
         return true;
      }
};

/*
  Method: configurationParameter.get

  Parameters  Type      Name             Description
  Inputs:
              string    db_name          configuration data set name
              array
                string  parameter        name of parameter to return
  Outputs:
              struct
                string  parameter      parameter name (key)
                string  value          parameter value

  Returns the name and value for each parameter in the input array
  of parameter names.  If any parameter in the set is undefined, a
  PARAMETER_UNDEFINED fault is returned.
*/
class ConfigRPC_get : public XmlRpcMethod
{
public:

   static XmlRpcMethod* get()
      {
         return new ConfigRPC_get();
      }

protected:
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        )
      {
         UtlString* dbName = dynamic_cast<UtlString*>(params.at(0));

         if (dbName && !dbName->isNull())
         {
            OsReadLock lock(*ConfigRPC::spDatabaseLock);

            ConfigRPC* db = ConfigRPC::find(*dbName);
            if (db)
            {
               status = db->mCallback->accessAllowed(requestContext, ConfigRPC_Callback::Get);
               if ( XmlRpcMethod::OK == status )
               {
                  OsConfigDb dataset;
                  OsStatus datasetStatus = db->load(dataset);

                  if ( OS_SUCCESS == datasetStatus )
                  {
                     UtlContainable* secondParam = params.at(1);
                     if ( secondParam )
                     {
                        UtlSList* nameList = dynamic_cast<UtlSList*>(secondParam);
                        
                        if (nameList)
                        {
                           UtlHashMap selectedParams;
                           UtlSListIterator requestedNames(*nameList);
                           UtlString* requestedName;
                           bool allNamesFound = true;
                           
                           while (   allNamesFound
                                  && (requestedName = dynamic_cast<UtlString*>(requestedNames()))
                                  )
                           {
                              UtlString* paramValue = new UtlString();
                              if ( OS_SUCCESS == dataset.get(*requestedName, *paramValue) )
                              {
                                 UtlString* paramName  = new UtlString(*requestedName);
                                 // put it into the results
                                 selectedParams.insertKeyAndValue(paramName, paramValue);
                              }
                              else
                              {
                                 allNamesFound = false;
                                 delete paramValue;
                              }
                           }
                           if (allNamesFound)
                           {
                              response.setResponse(&selectedParams);
                           }
                           else
                           {
                              // The second parameter was not a list
                              UtlString faultMsg;
                              faultMsg.append("parameter name '");
                              faultMsg.append(*requestedName);
                              faultMsg.append("' not found");
                              response.setFault(ConfigRPC::nameNotFound, faultMsg.data());
                              status = XmlRpcMethod::FAILED;
                           }
                           selectedParams.destroyAll();
                        }
                        else
                        {
                           // The second parameter was not a list
                           response.setFault( ConfigRPC::invalidType
                                             ,"namelist parameter is not an array"
                                             );
                           status = XmlRpcMethod::FAILED;
                        }
                     }
                     else // no parameter names specified
                     {
                        // return all names
                        UtlHashMap allParams;
                        UtlString  lastKey;
                        OsStatus   iterateStatus;
                        UtlString* paramName;
                        UtlString* paramValue;
                        bool       notEmpty = false;
  
                        for ( ( paramName  = new UtlString()
                               ,paramValue = new UtlString()
                               ,iterateStatus = dataset.getNext(lastKey, *paramName, *paramValue)
                               );
                              OS_SUCCESS == iterateStatus;
                              ( lastKey       = *paramName
                               ,paramName     = new UtlString()
                               ,paramValue    = new UtlString()
                               ,iterateStatus = dataset.getNext(lastKey, *paramName, *paramValue)
                               )
                             )
                        {
                           notEmpty = true; // got at least one parameter
                           // put it into the result array
                           allParams.insertKeyAndValue(paramName, paramValue);
                        }
                        // on the final iteration these were not used
                        delete paramName;
                        delete paramValue;
                        
                        if (notEmpty)
                        {
                           response.setResponse(&allParams);
                           allParams.destroyAll();
                        }
                        //:TODO: empty response?
                     }
                  }
                  else
                  {
                     UtlString faultMsg("dataset load failed");
                     response.setFault(ConfigRPC::loadFailed, faultMsg);
                     status = XmlRpcMethod::FAILED;
                  }
               }
            }
            else
            {
               UtlString faultMsg;               
               faultMsg.append("db lookup failed for '");
               faultMsg.append(*dbName);
               faultMsg.append("'");
               response.setFault( XmlRpcResponse::UnregisteredMethod, faultMsg.data());
               status = XmlRpcMethod::FAILED;
            }
         }
         else
         {
            response.setFault( XmlRpcResponse::EmptyParameterValue
                              ,"'dbname' parameter is missing or invalid type"
                              );
            status = XmlRpcMethod::FAILED;
         }
         
         return true;
      }
};

/*

  Method: configurationParameter.delete

  Parameters  Type      Name             Description
  Inputs:
  string    db_name          configuration data set name
  string    parameter        parameter name (key)
  Outputs:
  (none)

  Removes 'parameter' from 'db_name'.  This causes the value of
  'parameter' to be undefined.  It is not an error to invoke the
  delete method on an undefined parameter.


  Method: configurationParameter.datasetDelete

  Parameters  Type      Name             Description
  Inputs:
  string    db_name          configuration data set name
  Outputs:
  (none)

  Deletes the entire dataset; all parameters in the dataset become
  undefined.
*/


// Must be called once to connect the configurationParameter methods
void ConfigRPC::registerMethods(XmlRpcDispatch&     rpc /* xmlrpc dispatch service to use */)
{
   OsWriteLock lock(*spDatabaseLock);

   if (!sRegistered)
   {
      rpc.addMethod("configurationParameter.version", ConfigRPC_version::get, NULL);
      rpc.addMethod("configurationParameter.get",     ConfigRPC_get::get,     NULL);
      rpc.addMethod("configurationParameter.set",     ConfigRPC_set::get,     NULL);

      sRegistered = true;
   }
}

