#ifndef LINT
static char rcsid[] = "";
#endif

/* This routine for win32 found on web --GAT */

/* writev() emulations contained in this source file for the following systems:
 *
 *  WindowsNT
 */


#if defined(_WIN32)
#define OWN_WRITEV
#define TIMEOUT_SEC 120
#include <resparse/wnt/sys/uio.h> /* added for iovec definition --GAT */
#include <stdarg.h>
/*  #include "conf/portability.h"  we don't need this --GAT */


/*
 * writev --
 * simplistic writev implementation for WindowsNT using the WriteFile WIN32API.
 */
/* lgk win95 does not support overlapped/async file operations so change it to
   synchronous */
/* Zeev:  We don't compile the whole bindlib, so stay with Windows 95's implementation
 * (we dont have the hReadWriteEvent handle available)
 */

int
writev(fd, iov, iovcnt)
        int fd;
        const struct iovec *iov;
        int iovcnt;
{
        int i;
        char *base;
        DWORD BytesWritten, TotalBytesWritten = 0, len;
        BOOL ret;

        for (i=0; i<iovcnt; i++) {
                base = iov[i].iov_base;
                len = (DWORD)iov[i].iov_len;
                ret = WriteFile((HANDLE)fd, (char *)base, len,
                              (LPDWORD)&BytesWritten, NULL);
                if (ret == FALSE)
                   return (-1);
                TotalBytesWritten += BytesWritten;
        }
        return((int)TotalBytesWritten);
}

#endif // winnt

#ifndef OWN_WRITEV
int __bindcompat_writev;
#endif
