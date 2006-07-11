//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include <stdarg.h>
#include <resparse/wnt/sys/uio.h>


/* writev() emulation for WIN32 environment */

int writev(int fd, const struct iovec *vector, int count)
{
   int block;
   void *iov_base;
   unsigned int iov_len;
   unsigned int write_count, total_write_count;

   total_write_count = 0;

   for (block = 0; block < count; block++) {
      iov_base = vector[block].iov_base;
      iov_len = (unsigned int)vector[block].iov_len;
      if (WriteFile((HANDLE)fd, (char *)iov_base, iov_len, (unsigned int *)&write_count, NULL) == FALSE)
         return (-1);
      total_write_count += write_count;
   }
   return(total_write_count);
}
