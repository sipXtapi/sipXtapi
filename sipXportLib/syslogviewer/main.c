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

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#if defined(_WIN32)
#   include <io.h>
#elif defined(__linux__)
#   include <unistd.h>
#endif

#define BUFFER_SIZE 8192

static char buffer[BUFFER_SIZE];
static int buffer_full = 0;
static enum {
   NORMAL,
   BACKSLASH
} state;
// If 1, continuation lines produced by \n should be indented with a TAB.
static int indent = 0;

static void flush_buffer(int fd)
{
   write(fd, buffer, buffer_full);
   buffer_full = 0;
}

static void process_char(char ch, int fd)
{
   switch(state)
   {
   case NORMAL:
      if(ch == '\\')
         state = BACKSLASH;
      else
         buffer[buffer_full++] = ch;
      break;
   case BACKSLASH:
      if (ch == '\\')
      {
         buffer[buffer_full++] = '\\';
      }
      else if (ch == 'n')
      {
         buffer[buffer_full++] = '\n';
         // Indent the continuation line if it was requested.
         if (indent)
         {
            if (buffer_full == BUFFER_SIZE)
            {
               flush_buffer(fd);
            }
            buffer[buffer_full++] = '\t';
         }
      }
      else if (ch != 'r')
      {
         buffer[buffer_full++] = '\\';
         if(buffer_full == BUFFER_SIZE)
         {
            flush_buffer(fd);
         }
         buffer[buffer_full++] = ch;
      }
      state = NORMAL;
      break;
   }
   if(buffer_full == BUFFER_SIZE)
      flush_buffer(fd);
}

int main(int argc, char * argv[])
{
   char input_buffer[BUFFER_SIZE];
   int i, ifd = 0, ofd = 1;

   for(i = 1; i < argc; i++)
   {
      if(!strcmp(argv[i], "-h"))
      {
         fprintf(stderr, "Usage:\n\t%s [-h] [-i] [if=input] [of=output]\n",
                 argv[0]);
         fprintf(stderr, "\n");
         fprintf(stderr, "\t-h\tPrint this help message.\n");
         fprintf(stderr, "\t-i\tIndent continued lines.\n");
         fprintf(stderr, "\tif=\tSpecify input file name.\n");
         fprintf(stderr, "\tof=\tSpecify output file name.\n");
         return 0;
      }
      else if (strcmp(argv[i], "-i") == 0)
      {
         indent = 1;
      }
      else if(!strncmp(argv[i], "if=", 3))
      {
         ifd = open(&argv[i][3], O_RDONLY);
         if(ifd == -1)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else if(!strncmp(argv[i], "of=", 3))
      {
         ofd = open(&argv[i][3], O_WRONLY | O_CREAT, 0644);
         if(ofd == -1)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else
      {
         fprintf(stderr, "Unknown option: %s\n", argv[i]);
         return 1;
      }
   }

   i = read(ifd, input_buffer, BUFFER_SIZE);
   while(i && i != -1)
   {
      int j;
      for(j = 0; j != i; j++)
         process_char(input_buffer[j], ofd);
      i = read(ifd, input_buffer, BUFFER_SIZE);
   }

   flush_buffer(ofd);

   return 0;
}
