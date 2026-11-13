//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Linux implementation of utlRscGetProcessStats (see utl/UtlRscTrace.h).
// On other POSIX systems /proc is absent and every field stays -1.

// SYSTEM INCLUDES
#include <stdio.h>
#include <string.h>
#include <dirent.h>

// APPLICATION INCLUDES
#include "utl/UtlRscTrace.h"

void utlRscGetProcessStats(UtlRscProcessStats& stats)
{
   stats.handles = -1;
   stats.privateKB = -1;
   stats.threads = -1;

   // Open file descriptors: entries in /proc/self/fd, less the one
   // opendir itself holds while we read it.
   DIR* dir = opendir("/proc/self/fd");
   if (dir != NULL)
   {
      long count = 0;
      struct dirent* entry;
      while ((entry = readdir(dir)) != NULL)
      {
         if (entry->d_name[0] != '.')
         {
            count++;
         }
      }
      closedir(dir);
      stats.handles = count > 0 ? count - 1 : 0;
   }

   FILE* status = fopen("/proc/self/status", "r");
   if (status != NULL)
   {
      char line[256];
      long value;
      while (fgets(line, sizeof(line), status) != NULL)
      {
         if (sscanf(line, "VmRSS: %ld", &value) == 1)
         {
            stats.privateKB = value;
         }
         else if (sscanf(line, "Threads: %ld", &value) == 1)
         {
            stats.threads = value;
         }
      }
      fclose(status);
   }
}
