/*
 * Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
 * Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
 *
 *  res_parse.c
 *
 *  Parse the result of a DNS query
 *      into an accessible structure
 *
 *  v1.0
 *
 *  Dave Shield         January 1994
 *
 *  v1.1        Cast all malloc calls
 *              Cleaner handling of header
 *                      February 1994
 *
 *  v1.2        Fixed incorrect calls to free_response
 *              Use memcpy rather than bcopy
 *                      November 1995
 *
 *  v1.3        Allocate response dynamically
 *              Handle network/host ordering
 *                      July 1997
 *
 *  v1.4        Track the length of the response buffer and bounds check
 *              every field read.  A crafted DNS response with inflated
 *              record counts or field lengths could previously drive the
 *              parser past the end of the response buffer, reading adjacent
 *              heap memory (CWE-125).  All reads are now bounded by 'eom',
 *              one byte past the last valid byte of the response.
 */
#include <stdio.h>
#include "resparse/rr.h"
#ifdef __pingtel_on_posix__
#include <resolv.h>
/* These are a cheezy hack from ns_netint.c */
#define _getshort _pingtel_getshort
#define _getlong _pingtel_getlong

u_int _getshort(const u_char *src);
u_long _getlong(const u_char *src);
char * expand_cdname(char **cpp, char *msg, char *eom);
char * expand_charstring(char **cpp, char *msg, char *eom);
s_question * parse_question(char **cpp, char *msg, char *eom);
s_rr * parse_rr( char **cpp, char *msg, char *eom);

u_int _getshort(const u_char *src)
{
        u_int dst;
        NS_GET16(dst, src);
        return dst;
}
u_long _getlong(const u_char *src)
{
        u_long dst;
        NS_GET32(dst, src);
        return dst;
}
#endif



extern int dn_expand(const u_char *a,
                        const u_char *b, const u_char *c, char *d, int e);




        /*
         *  expand_cdname:
         *      Extract and uncompress a domain name
         *
         *      'eom' points one byte past the end of the response buffer and
         *      bounds the (possibly compression-pointer-following) expansion.
         *
         *  returns a pointer to the domain name (or NULL on failure).
         */
char *
expand_cdname(
        char    **cpp,
        char    *msg,
        char    *eom)
{
        char    name[MAXDNAME];
        int     i;
        char    *ptr;

        /* The current position must lie within the message. */
        if (*cpp < msg || *cpp >= eom)
                return(NULL);

        if ((i = dn_expand((const u_char *)msg, (const u_char *)eom,
                           (const u_char *)*cpp, name, MAXDNAME)) < 0)
                return(NULL);
        if (name[0] == '\0' ) {
                name[0] = '.';
                name[1] = '\0';
        }
        *cpp += i;      /* Step over the name */

        if ((ptr=(char *)malloc(strlen(name)+1)) == NULL ) {
                        /* Failed - undo everything */
                *cpp -= i;
                return(NULL);
        }
        strcpy(ptr, name);
        return(ptr);
}


        /*
         *  expand_charstring:
         *      Extract a character string
         *
         *      Bounded by 'eom'.  Both the length octet and the string data
         *      it describes must lie within the response buffer.
         *
         *  returns a pointer to the string (or NULL on failure).
         */
char *
expand_charstring(
        char    **cpp,
        char    *msg,
        char    *eom)
{
        int     i;
        char    *ptr;

        /* Need at least the length octet. */
        if (*cpp < msg || *cpp >= eom)
                return(NULL);

        i = (unsigned char)**cpp;       /* Extract length of string */
        (*cpp)++;                       /*  and move on */

        /* The string data must also lie within the message. */
        if (*cpp + i > eom) {
                (*cpp)--;
                return(NULL);
        }

        if ((ptr=(char *)malloc(i+1)) == NULL ) {
                        /* Failed - undo everything */
                (*cpp)--;
                return(NULL);
        }
        strncpy(ptr, *cpp, i);
        ptr[i] = '\0';
        *cpp += i;

        return(ptr);
}


        /*
         *  parse_question:
         *      Extract and parse a question record
         *
         *  returns a pointer to the question (or NULL on failure).
         */
s_question *
parse_question(
        char    **cpp,
        char    *msg,
        char    *eom)
{
        s_question      *ptr;

        if ((ptr = (s_question *)malloc(sizeof(s_question))) == NULL )
                return(NULL);
        if ((ptr->qname = expand_cdname(cpp,msg,eom)) == NULL ) {
                free( ptr );
                return(NULL);
        }
        /* qtype (2) + qclass (2) must be present. */
        if (*cpp + 2*sizeof(u_short) > eom) {
                free(ptr->qname);
                free(ptr);
                return(NULL);
        }
        ptr->qtype = _getshort(*cpp);
        *cpp += sizeof(u_short);
        ptr->qclass = _getshort(*cpp);
        *cpp += sizeof(u_short);

        return(ptr);
}



        /*
         *  parse_rr:
         *      Extract and parse a resource record
         *
         *      Every field read is bounded by 'eom'.  The RR-independent
         *      header (name, type, class, ttl, rdlength) must lie within the
         *      message, and the rdata region it describes must fit within the
         *      message as well.  Reads of individual rdata fields are bounded
         *      by the rdata region ('rdata_end'); on any shortfall the switch
         *      stops parsing that record but the record parsed so far is still
         *      returned, matching the historical lenient behaviour.
         *
         *  returns a pointer to the RR (or NULL on failure).
         */
s_rr *
parse_rr(
        char    **cpp,
        char    *msg,
        char    *eom)
{
        s_rr            *ptr;
        int             dlen;
        char            *rdata_end;
        union u_rdata   *rd;

/* Ensure at least 'n' bytes of rdata remain before a raw read; otherwise
 * abandon further rdata parsing for this record.  Expands to a bare
 * 'if (...) break;' so the break targets the enclosing switch/loop.  'n' is
 * always a non-negative compile-time size at the call sites. */
#define RR_NEED(n)  if (*cpp + (n) > rdata_end) break;

                /*
                 *  Set up the RR-independent information
                 */
        if ((ptr = (s_rr *)malloc(sizeof(s_rr))) == NULL )
                return(NULL);
        if ((ptr->name = expand_cdname(cpp,msg,eom)) == NULL ) {
                free(ptr);
                return(NULL);
        }

        /* type (2) + class (2) + ttl (4) + rdlength (2) = 10 bytes. */
        if (*cpp + 3*sizeof(u_short) + sizeof(u_int) > eom) {
                free(ptr->name);
                free(ptr);
                return(NULL);
        }
        ptr->type = _getshort(*cpp);
        *cpp += sizeof(u_short);
        ptr->rclass = _getshort(*cpp);
        *cpp += sizeof(u_short);
        ptr->ttl = _getlong(*cpp);
        /* Size on the network is 4 bytes so use u_int not u_long as
         * u_long is architecture dependent (e.g. 8 bytes on 64 bit). */
        *cpp += sizeof(u_int);

        dlen = _getshort(*cpp);
        ptr->dlen = dlen;
        *cpp += sizeof(u_short);

        /* The rdata region described by rdlength must fit within the
         * message.  Reject the whole response otherwise, since we cannot
         * reliably locate the next record. */
        if (dlen < 0 || *cpp + dlen > eom) {
                free(ptr->name);
                free(ptr);
                return(NULL);
        }
        rdata_end = *cpp + dlen;

        rd = &ptr->rdata;


                /*
                 *  Handle RR-specifics
                 *
                 *    (No indication of failures here is
                 *      passed back to the calling procedure)
                 */
        switch(ptr->type) {
        case T_A:                               /* Address */
                switch (ptr->rclass) {
                case C_IN:
                        RR_NEED(sizeof(struct in_addr));
                        memcpy((void *)&rd->address, (void *)*cpp, sizeof(struct in_addr));
                        break;

                default:
                        /* Can't really handle this - just skip it */
                        break;
                }
                break;

        case T_NS:                              /* Name Server */
        case T_MD:                              /* Mail Destination (OBS) */
        case T_MF:                              /* Mail Forwarder   (OBS) */
        case T_CNAME:                           /* Canonical Name */
                rd->string = expand_cdname(cpp, msg, eom);
                break;

        case T_SOA:                             /* Start of Authority */
                rd->soa.mname = expand_cdname(cpp, msg, eom);
                rd->soa.rname = expand_cdname(cpp, msg, eom);
                RR_NEED(sizeof(u_int));
                rd->soa.serial = _getlong(*cpp);
                *cpp += sizeof(u_int);
                RR_NEED(sizeof(u_int));
                rd->soa.refresh = _getlong(*cpp);
                *cpp += sizeof(u_int);
                RR_NEED(sizeof(u_int));
                rd->soa.retry = _getlong(*cpp);
                *cpp += sizeof(u_int);
                RR_NEED(sizeof(u_int));
                rd->soa.expire = _getlong(*cpp);
                *cpp += sizeof(u_int);
                RR_NEED(sizeof(u_int));
                rd->soa.minimum = _getlong(*cpp);
                *cpp += sizeof(u_int);
                break;

        case T_MB:                              /* Mail Box  */
        case T_MG:                              /* Mail Group */
        case T_MR:                              /* Mail Rename */
                rd->string = expand_cdname(cpp, msg, eom);
                break;

/* Following modification taken from VxWorks --GAT */
/* 01b,29apr97,jag Changed T_NULL to T_NULL_RR to fix conflict with loadCoffLib.h */
        case T_NULL_RR:                         /* Null RR */
                if (dlen > 0 && (rd->null.anything = (char *)malloc(dlen)) != NULL )
                        memcpy((void *)rd->null.anything, (void *)*cpp, dlen);
                rd->null.length = dlen;
                break;

        case T_WKS:                             /* Well Known Services */
                RR_NEED(sizeof(struct in_addr) + 1);
                memcpy((void *)&rd->wks.address, (void *)*cpp, sizeof(struct in_addr));
                *cpp += sizeof(struct in_addr);
                rd->wks.protocol = **cpp;
                (*cpp)++;
                rd->wks.maplength = dlen-(sizeof(struct in_addr) +1);
                /* Compare via subtraction (never pointer + u_long) so an
                 * underflowed maplength cannot overflow the pointer. */
                if (rd->wks.maplength > 0 &&
                    rd->wks.maplength <= (u_long)(rdata_end - *cpp) &&
                    (rd->wks.bitmap = (char *)malloc(rd->wks.maplength)) != NULL )
                        memcpy((void *)rd->wks.bitmap, (void *)*cpp, rd->wks.maplength);
                break;

        case T_PTR:                             /* Domain Name Pointer */
                rd->string = expand_cdname(cpp, msg, eom);
                break;

        case T_HINFO:                           /* Host Info */
                if (( rd->hinfo.cpu = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }
                if (( rd->hinfo.os = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }
                break;

        case T_MINFO:                           /* Mailbox Info */
                rd->minfo.rmailbx = expand_cdname(cpp, msg, eom);
                rd->minfo.emailbx = expand_cdname(cpp, msg, eom);
                break;

        case T_MX:                              /* Mail Exchanger */
                RR_NEED(sizeof(u_short));
                rd->mx.preference = _getshort(*cpp);
                *cpp += sizeof(u_short);
                rd->mx.exchange = expand_cdname(cpp, msg, eom);
                break;

        case T_SRV:                             /* Service location */
                RR_NEED(sizeof(u_short));
                rd->srv.priority = _getshort(*cpp);
                *cpp += sizeof(u_short);
                RR_NEED(sizeof(u_short));
                rd->srv.weight = _getshort(*cpp);
                *cpp += sizeof(u_short);
                RR_NEED(sizeof(u_short));
                rd->srv.port = _getshort(*cpp);
                *cpp += sizeof(u_short);
                rd->srv.target = expand_cdname(cpp, msg, eom);
                break;

        case T_NAPTR:                           /* Naming authority pointer */
        {
                RR_NEED(sizeof(u_short));
                rd->naptr.order = _getshort(*cpp);
                *cpp += sizeof(u_short);
                RR_NEED(sizeof(u_short));
                rd->naptr.preference = _getshort(*cpp);
                *cpp += sizeof(u_short);
                if (( rd->naptr.flags = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }
                if (( rd->naptr.services = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }
                if (( rd->naptr.regexp = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }
                rd->naptr.replacement = expand_cdname(cpp, msg, eom);
        }
                break;

        case T_TXT:                             /* Text string */
                RR_NEED(1);
                rd->txt.len = (unsigned char)**cpp;
                rd->txt.next = NULL;
                if ((rd->txt.text = expand_charstring(cpp, msg, eom)) == NULL ) {
                        break;
                }

                        /*
                         *  Multiple strings
                         *      add onto a linked list
                         */
                if ( rd->txt.len+1 < dlen ) {
                        struct s_TXT    *txtp;
                        char            **cpp2;
                        int             n;

                        n = rd->txt.len+1;
                        txtp = &(rd->txt);
                        cpp2 = cpp;
                        while ( n < dlen ) {
                                if (*cpp2 < msg || *cpp2 >= rdata_end)
                                        break;
                                if ((txtp->next = (struct s_TXT *)malloc(sizeof(struct s_TXT))) == NULL ) {
                                        break;
                                }
                                txtp = txtp->next;
                                txtp->len = (unsigned char)**cpp2;
                                n += txtp->len+1;
                                txtp->next = NULL;
                                if ((txtp->text = expand_charstring(cpp2, msg, eom)) == NULL ) {
                                        break;
                                }
                        }
                }
                break;


                        /*
                         *  RFC 1183  Additional types
                         */
        case T_AFSDB:                           /* AFS Server */
                RR_NEED(sizeof(u_short));
                rd->afsdb.subtype = _getshort(*cpp);
                *cpp += sizeof(u_short);
                rd->afsdb.hostname = expand_cdname(cpp, msg, eom);
                break;


        case T_RP:                              /* Responsible Person */
                rd->rp.mbox_dname = expand_cdname(cpp, msg, eom);
                rd->rp.txt_dname = expand_cdname(cpp, msg, eom);
                break;

        case T_X25:                             /* X25 Address */
                rd->string = expand_charstring(cpp, msg, eom);
                break;

        case T_ISDN:                            /* ISDN Address */
                RR_NEED(1);
                if ( **cpp == dlen ) {
                        rd->isdn.address = expand_charstring(cpp, msg, eom);
                        rd->isdn.sa = NULL;
                }
                else {
                        rd->isdn.address = expand_charstring(cpp, msg, eom);
                        rd->isdn.sa = expand_charstring(cpp, msg, eom);
                }
                break;

        case T_RT:                              /* Route Through */
                RR_NEED(sizeof(u_short));
                rd->rt.preference = _getshort(*cpp);
                *cpp += sizeof(u_short);
                rd->rt.int_host = expand_cdname(cpp, msg, eom);
                break;

                        /*
                         *  Additional Non-standard types
                         */
        case T_UINFO:                           /* User (finger) info */
                if ((rd->string = (char *)malloc(dlen+1)) != NULL ) {
                        memcpy((void *)rd->string, (void *)*cpp, dlen);
                        rd->string[dlen] = '\0';
                }
                break;

        case T_UID:                             /* User ID */
        case T_GID:                             /* Group ID */
                RR_NEED(sizeof(u_int));
                rd->number = _getlong(*cpp);
                *cpp += sizeof(u_int);
                break;

        case T_UNSPEC:                          /* Unspecified info */
        default:                                /* Unrecognised */
                if ((rd->string = (char *)malloc(dlen+1)) != NULL ) {
                        memcpy((void *)rd->string, (void *)*cpp, dlen);
                        rd->string[dlen] = '\0';
                }
                break;

        }

        /* Resynchronise to the end of this record's rdata regardless of how
         * far the individual field handlers advanced.  rdlength is
         * authoritative for the record boundary, so this both recovers from a
         * truncated rdata parse above and corrects any per-field advance
         * inconsistencies. */
        *cpp = rdata_end;

#undef RR_NEED

        return(ptr);
}




        /*
         *  res_parse:
         *      Parse a DNS response buffer
         *
         *      'msglen' is the number of valid bytes in 'msg' (the value
         *      returned by res_query()).  It bounds all parsing so a crafted
         *      response cannot drive the parser past the end of the buffer.
         *
         *  returns a pointer to the expanded tree (or NULL on failure).
         */
res_response *
res_parse(char *msg, int msglen)
{
        char *cp;
        char *eom;
        HEADER *hp;
        res_response *resp;
        int i;

        u_short qdcount, ancount, nscount, arcount;

        /* A valid response must contain at least a fixed header. */
        if (msg == NULL || msglen < (int)sizeof(HEADER))
                return(NULL);
        eom = msg + msglen;


        /*
         * Set up the response structure,
         *  and copy across the header fields.
         */
        if ((resp = (res_response *)malloc(sizeof(res_response))) == NULL )
                return(NULL);
        memcpy((void *)&(resp->header), (void *)msg, sizeof(HEADER));
        hp = &(resp->header);
        cp = msg + sizeof(HEADER);


        /*
         * Temporarily clear the number of records to expect
         *    (allows the tree to be freed in case of problems)
         * Also handle network/host ordering
         */
        qdcount = ntohs((u_short)resp->header.qdcount); resp->header.qdcount = 0;
        ancount = ntohs((u_short)resp->header.ancount); resp->header.ancount = 0;
        nscount = ntohs((u_short)resp->header.nscount); resp->header.nscount = 0;
        arcount = ntohs((u_short)resp->header.arcount); resp->header.arcount = 0;
        resp->question = NULL;
        resp->answer = NULL;
        resp->authority = NULL;
        resp->additional = NULL;

        /*
         * Handle question records.
         */
        if ( qdcount ) {
                if ((resp->question = (s_question **)malloc(qdcount*sizeof(s_question*))) == NULL )
                        return(NULL);
                for ( i=0 ; i<qdcount ; i++ )           /* Clear, in case of free */
                        resp->question[i] = NULL;
                resp->header.qdcount = qdcount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<qdcount ; i++ )
                        if ((resp->question[i] = parse_question(&cp, msg, eom)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle authoritative answer records
         */
        if ( ancount ) {
                if ((resp->answer = (s_rr **)malloc(ancount*sizeof(s_rr*))) == NULL ) {
                        resp->header.ancount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<ancount ; i++ )
                        resp->answer[i] = NULL;
                resp->header.ancount = ancount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<ancount ; i++ )
                        if ((resp->answer[i] = parse_rr(&cp, msg, eom)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle name server records
         */
        if ( nscount ) {
                if ((resp->authority = (s_rr **)malloc(nscount*sizeof(s_rr*))) == NULL ) {
                        resp->header.nscount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<nscount ; i++ )
                        resp->authority[i] = NULL;
                resp->header.nscount = nscount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<nscount ; i++ )
                        if ((resp->authority[i] = parse_rr(&cp, msg, eom)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }


        /*
         * Handle additional records
         */
        if ( arcount ) {
                if ((resp->additional = (s_rr **)malloc(arcount*sizeof(s_rr*))) == NULL ) {
                        resp->header.arcount = 0;
                        free_response(resp);
                        free(resp);
                        return(NULL);
                }
                for ( i=0 ; i<arcount ; i++ )
                        resp->additional[i] = NULL;
                resp->header.arcount = arcount;  /* Stores swapped byte order!  Requires change to free_response. --GAT */
                for ( i=0 ; i<arcount ; i++ )
                        if ((resp->additional[i] = parse_rr(&cp, msg, eom)) == NULL ) {
                                free_response(resp);
                                free(resp);
                                return(NULL);
                        }
        }

        return(resp);
}
