//
// Copyright (C) 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include <getopt.h>

#include "os/OsDefs.h"
#if defined(_WIN32)
#   include <windows.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlHashMapIterator.h>
#include <net/XmlRpcRequest.h>
#include <os/OsSysLog.h>
#include <os/OsTask.h>

// CONSTANTS
#define MEMCHECK_DELAY 45
#define HTTP_PORT               8200    // Default HTTP port

int HttpPort = HTTP_PORT;
enum 
{
   Quiet,
   Normal,
   Verbose
} Feedback = Quiet;

const char* LogFile = "xmlrpcClient.log";
const char* xmlrpcURI;
int MemCheckDelay = 0;

void initLogger(char* argv[])
{
   OsSysLog::initialize(0, // do not cache any log messages in memory
                        argv[0]); // name for messages from this program
   OsSysLog::setOutputFile(0, // no cache period
                           LogFile); // log file name
   switch (Feedback)
   {
   case Quiet:
      OsSysLog::setLoggingPriority(PRI_WARNING);
      break;
   case Normal:
      OsSysLog::setLoggingPriority(PRI_INFO);
      break;
   case Verbose:
      OsSysLog::setLoggingPriority(PRI_DEBUG);
      break;
   }
}

void showHelp(char* argv[])
{
   printf("Usage: \n"
          "   %s:\n"
          "   [ {-v|--verbose} | {-q|--quiet} ]\n"
          "   [ {-h|-?|--help} ]\n"
          "   [ --version <xmlrpc URI> <dataset> ]\n"
          "   [ {-g|--get} <xmlrpc URI> <dataset> <name> ... ]\n"
          "   [ {-s|--set} <xmlrpc URI> <dataset> <name> <value> [ <name> <value> ] ... ]\n"
          "   [ {-d|--delete} <xmlrpc URI> <dataset> <name> ... ]\n"
          , argv[0]
          );
}

typedef enum
{
   Version,         ///< configurationParameter.version
   Get,             ///< configurationParameter.get
   Set,             ///< configurationParameter.set
   Delete           ///< configurationParameter.delete
} MethodType;

MethodType Method;
UtlString  DataSet;

void parseArgs(int argc, char* argv[])
{
   int optResult = 0;
   
   const char* short_options = "p:l:vqmhg";
   
   const struct option long_options[] =
      {
         {"verbose", no_argument, NULL, 'v'},
         {"quiet",   no_argument, NULL, 'q'},
         {"memcheck",no_argument, &MemCheckDelay, MEMCHECK_DELAY},
         {"help",    no_argument, NULL, 'h'},
         {"port",    required_argument, NULL, 'p'},
         {"log",     required_argument, NULL, 'l'},
         {"version", no_argument, (int*)&Method, Version },
         {"get",     no_argument, (int*)&Method, Get },
         {"set",     no_argument, (int*)&Method, Set },
         {"delete",  no_argument, (int*)&Method, Delete },
         {0, 0, 0, 0}
      };

   while (  (optResult = getopt_long (argc, argv, short_options, long_options, NULL))
          >= 0
          )
   {
      char* optend;

      switch (optResult)
      {
      case 'p':
         HttpPort = strtoul(optarg, &optend, 10);
         if ( '\0' != *optend )
         {
            fprintf( stderr, "Invalid HTTP port %s\n", optarg );
            exit(1);
         }
         break;

      case 'l':
         LogFile = optarg;
         break;

      case 'v':
         Feedback = Verbose;
         break;

      case 'q':
         Feedback = Quiet;
         break;

      case 'h':
      case '?':
         showHelp(argv);
         exit(0);
         break;

      default:
         break;
      }
   }

   if (optind < argc)
   {
      xmlrpcURI = argv[optind++];
   }

   if (optind < argc)
   {
      DataSet = argv[optind++];
   }
}

void exitFault(XmlRpcResponse& response)
{
   UtlString reason;
   int code;
   response.getFault(&code,reason);
   fprintf(stderr, "XML-RPC Fault %d: %s\n", code, reason.data() );
   exit(1);
}

int main(int argc, char* argv[])
{
   parseArgs(argc, argv);
   initLogger(argv);

   Url url(xmlrpcURI);
    
   if (MemCheckDelay)
   {
      // Delay 45 seconds to allow memcheck start
      printf("Wating %d seconds for start of memcheck ...", MemCheckDelay);
      OsTask::delay(MemCheckDelay * 1000);
      printf("starting\n");
   }

   XmlRpcRequest* request;
   XmlRpcResponse response;

   switch (Method)
   {
   case Version: // --version <xmlrpc URI> <dataset>
      if (optind < argc)
      {
         fprintf(stderr, "Too many arguments: '%s'\n", argv[optind]);
         showHelp(argv);
         exit(1);
      }

      request = new XmlRpcRequest(url, "configurationParameter.version");
      request->addParam(&DataSet);

      if (!request->execute(response))
      {
         exitFault(response);
      }
      else
      {
         UtlContainable* value;
         if (response.getResponse(value))
         {
            UtlString* versionId = dynamic_cast<UtlString*>(value);
            if (versionId)
            {
               printf("%s\n", versionId->data());
            }
            else
            {
               fprintf(stderr, "Incorrect type returned.\n");
               exit(1);
            }
         }
         else
         {
            fprintf(stderr, "No value returned.\n");
         }
      }
      break;

   case Get: // --get <xmlrpc URI> <dataset> <name> ...
   {
      request = new XmlRpcRequest(url, "configurationParameter.get");
      request->addParam(&DataSet);

      UtlSList names;
      // copy remaining arguments into the names list
      while (optind < argc)
      {
         names.append(new UtlString(argv[optind++]));
      }
      if (!names.isEmpty())
      {
         request->addParam(&names);
      }
      
      if (!request->execute(response))
      {
         exitFault(response);
      }
      else
      {
         UtlContainable* value;
         if (response.getResponse(value))
         {
            UtlHashMap* paramList = dynamic_cast<UtlHashMap*>(value);
            if (paramList)
            {
               UtlHashMapIterator params(*paramList);
               UtlString* name;
               while ((name = dynamic_cast<UtlString*>(params())))
               {
                  UtlString* value = dynamic_cast<UtlString*>(paramList->findValue(name));
                  printf("%s : %s\n", name->data(), value->data());
               }
            }
            else
            {
               fprintf(stderr, "Incorrect type returned.\n");
               exit(1);
            }
         }
         else
         {
            fprintf(stderr, "No value returned.\n");
            exit(1);
         }
      }
   }
      break;

   case Set: // --set <xmlrpc URI> <dataset> <name> <value> [ <name> <value> ] ... 
   {
      request = new XmlRpcRequest(url, "configurationParameter.set");
      request->addParam(&DataSet);

      UtlHashMap parameters;
      // copy remaining arguments into the names list
      while (optind + 1 < argc)
      {
         UtlString* setName = new UtlString(argv[optind++]);
         UtlString* setValue = new UtlString(argv[optind++]);
         parameters.insertKeyAndValue(setName, setValue);
      }
      if (optind < argc)
      {
         fprintf(stderr, "name '%s' without a value\n", argv[optind]);
         showHelp(argv);
         exit(1);
      }
      
      if (parameters.isEmpty())
      {
         fprintf(stderr, "must specify at least one name and value\n");
         showHelp(argv);
         exit(1);
      }
      else
      {
         request->addParam(&parameters);
         parameters.destroyAll();
      }
      
      if (request->execute(response))
      {
         UtlContainable* value;
         if (response.getResponse(value))
         {
            UtlInt* numberSet = dynamic_cast<UtlInt*>(value);
            if (numberSet)
            {
               if (Verbose == Feedback)
               {
                  printf("set %d name/value pairs.\n", numberSet->getValue());
               }
            }
            else
            {
               fprintf(stderr, "Incorrect type returned.\n");
               exit(1);
            }
         }
         else
         {
            fprintf(stderr, "No value returned.\n");
            exit(1);
         }
      }
      else
      {
         exitFault(response);
      }
   }
      break;

   case Delete: // --delete <xmlrpc URI> <dataset> <name> ... 
   {
      request = new XmlRpcRequest(url, "configurationParameter.delete");
      request->addParam(&DataSet);

      UtlSList names;
      // copy remaining arguments into the names list
      while (optind < argc)
      {
         names.append(new UtlString(argv[optind++]));
      }
      if (!names.isEmpty())
      {
         request->addParam(&names);
      }
      
      if (!request->execute(response))
      {
         exitFault(response);
      }
      else
      {
         UtlContainable* value;
         if (response.getResponse(value))
         {
            UtlInt* deletedCount = dynamic_cast<UtlInt*>(value);
            if (deletedCount)
            {
               if (Verbose == Feedback)
               {
                  printf("deleted %d parameters.\n", deletedCount->getValue());
               }
            }
            else
            {
               fprintf(stderr, "Incorrect type returned.\n");
               exit(1);
            }
         }
         else
         {
            fprintf(stderr, "No value returned.\n");
            exit(1);
         }
      }
   }
      break;

   default:
      fprintf(stderr, "No method specified\n");
      showHelp(argv);
      exit(1);
   }
   
   delete request;
   request = NULL;

   if (MemCheckDelay)
   {
      // Delay 45 seconds to allow memcheck start
      printf("Wating %d seconds for stop of memcheck ...", MemCheckDelay);
      OsTask::delay(MemCheckDelay * 1000);
      printf("starting\n");
   }

   exit(0);
      
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}

