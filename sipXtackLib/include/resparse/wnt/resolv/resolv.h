/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *		PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *		Copyright Notice
 *
 * Notice of copyright on this source code product does not indicate
 * publication.
 *
 *	(c) 1986,1987,1988,1989,1996  Sun Microsystems, Inc
 *	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *		All rights reserved.
 *
 * BIND 4.9.3:
 *
 * Copyright (c) 1983, 1987, 1989, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * --Copyright--
 *
 * End BIND 4.9.3
 */

/*
 * Global defines and variables for resolver stub.
 */

#ifndef	_RESOLV_H
#define	_RESOLV_H

/* BIND 4.9.3 */
#include <sys/types.h>
/* End BIND 4.9.3 */

#include <stdio.h>
#include <winsock2.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* BIND 4.9.3 */
#define	__RES	19951031
/* End BIND 4.9.3 */

/*
 * Resolver configuration file.
 * Normally not present, but may contain the address of the
 * inital name server(s) to query and the domain search list.
 */

#ifndef	_PATH_RESCONF
#define	_PATH_RESCONF	"resolv.conf"
#endif

/* ADDRSORT and MAXADDR retained for compatibility; not used */
#define	ADDRSORT	1		/* enable the address-sorting option */
#define	MAXADDR		10		/* max # addresses to sort by */

#define	MAXNS			3	/* max # name servers we'll track */
#define	MAXDFLSRCH		3	/* # default domain levels to try */
#define	MAXDNSRCH		6	/* max # domains in search path */
#define	LOCALDOMAINPARTS	2	/* min levels in name that is "local" */

/* BIND 4.9.3 */
#define	RES_TIMEOUT		5	/* min. seconds between retries */
#define	MAXRESOLVSORT		10	/* number of net to sort on */
#define	RES_MAXNDOTS		15	/* should reflect bit field size */
/* End BIND 4.9.3 */

/* BIND 4.9.3 */
/*
 * NOTE: the "__res_state" struct replaces the "state" struct used in
 * previous versions of BIND. These two structures are only partially
 * compatible.
 */

struct __res_state {
	int	retrans;		/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	u_long	options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct sockaddr_in
		nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr	nsaddr_list[0]		/* for backward compatibility */
	u_short	id;			/* current packet id */
	char	*dnsrch[MAXDNSRCH+1];	/* components of domain to search */
	char	defdname[MAXDNAME];	/* default domain */
	u_long	pfcode;			/* RES_PRF_ flags - see below. */
	unsigned ndots:4;		/* threshold for initial abs. query */
	unsigned nsort:4;		/* number of elements in sort_list[] */
	char	unused[3];
	struct {
		struct in_addr	addr;
/* BIND 4.9.3 original:	u_int32_t	mask; */
		unsigned int	mask;
	} sort_list[MAXRESOLVSORT];
	char	pad[72];		/* On an i38this means 512b total. */
};
/* End BIND 4.9.3 */


/*
 * Resolver options
 */
#define	RES_INIT	0x00000001	/* address initialized */
#define	RES_DEBUG	0x00000002	/* print debug messages */
#define	RES_AAONLY	0x00000004	/* authoritative answers only (!IMPL) */
#define	RES_USEVC	0x00000008	/* use virtual circuit */
#define	RES_PRIMARY	0x00000010	/* query primary server only (!IMPL) */
#define	RES_IGNTC	0x00000020	/* ignore trucation errors */
#define	RES_RECURSE	0x00000040	/* recursion desired */
#define	RES_DEFNAMES	0x00000080	/* use default domain name */
#define	RES_STAYOPEN	0x00000100	/* Keep TCP socket open */
#define	RES_DNSRCH	0x00000200	/* search up local domain tree */
/* BIND 4.9.3 */
#define	RES_INSECURE1	0x00000400	/* type 1 security disabled */
#define	RES_INSECURE2	0x00000800	/* type 2 security disabled */
#define	RES_NOALIASES	0x00001000	/* shuts off HOSTALIASES feature */
/* End BIND 4.9.3 */

/* Gotten from BSD resolv.h */
#define	RES_USE_INET6	0x00002000	/* use/map IPv6 in gethostbyname() */
/* Define by myself */
#define RES_NOTLDQUERY  0               /* Don't know where to find it */

#define	RES_DEFAULT	(RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

/* BIND 4.9.3 */

/*
 * Resolver "pfcode" values.  Used by dig.
 */
#define	RES_PRF_STATS	0x00000001
/*			0x00000002	*/
#define	RES_PRF_CLASS   0x00000004
#define	RES_PRF_CMD	0x00000008
#define	RES_PRF_QUES	0x00000010
#define	RES_PRF_ANS	0x00000020
#define	RES_PRF_AUTH	0x00000040
#define	RES_PRF_ADD	0x00000080
#define	RES_PRF_HEAD1	0x00000100
#define	RES_PRF_HEAD2	0x00000200
#define	RES_PRF_TTLID	0x00000400
#define	RES_PRF_HEADX	0x00000800
#define	RES_PRF_QUERY	0x00001000
#define	RES_PRF_REPLY	0x00002000
#define	RES_PRF_INIT    0x00004000
/*			0x00008000	*/

#ifdef __STDC__
#define	__P(x) x
#else
#define	__P(x) ()
#endif /* __STDC__ */


/* hooks are still experimental as of 4.9.2 */
typedef enum { res_goahead, res_nextns, res_modified, res_done, res_error }
	res_sendhookact;

typedef res_sendhookact (*res_send_qhook)(struct sockaddr_in * const *ns,
						const u_char **query,
						int *querylen,
						u_char *ans,
						int anssiz,
						int *resplen);

typedef res_sendhookact (*res_send_rhook)(const struct sockaddr_in *ns,
						const u_char *query,
						int querylen,
						u_char *ans,
						int anssiz,
						int *resplen);

/*
 * NOTE: _res used to be "struct state", which is only partially compatible
 * with the __res_state struct.
 */
extern struct __res_state _res;

/* Private routines shared between libc/net, named, nslookup and others. */
#define	dn_skipname	__dn_skipname
#define	fp_query	__fp_query
#define	fp_nquery	__fp_nquery
#define	hostalias	__hostalias
#define	putlong		__putlong
#define	putshort	__putshort
#define	p_class		__p_class
#define	p_time		__p_time
#define	p_type		__p_type
#define	p_cdnname	__p_cdnname
#define	p_cdname	__p_cdname
#define	p_fqname	__p_fqname
#define	p_query		__p_query
#define	p_rr		__p_rr
#define	p_option	__p_option
#define	res_randomid	__res_randomid
#define	res_isourserver	__res_isourserver
#define	res_nameinquery	__res_nameinquery
#define	res_queriesmatch __res_queriesmatch

int	 __dn_skipname(const u_char *, const u_char *);
void	 __fp_resstat(struct __res_state *, FILE *);
void	 __fp_query(const u_char *, FILE *);
void	 __fp_nquery(const u_char *, int, FILE *);
char	*__hostalias(const char *);
/* BIND 4.9.3 original:	void	 __putlong(u_int32_t, u_char *); */
void	 __putlong(unsigned int, u_char *);
/* BIND 4.9.3 original:	void	 __putshort(u_int16_t, u_char *); */
void	 __putshort(unsigned short, u_char *);
/* BIND 4.9.3 original:	char	*__p_time(u_int32_t); */
char	*__p_time(unsigned int);
void	 __p_query(const u_char *);
const u_char *__p_cdnname(const u_char *, const u_char *, int, FILE *);
const u_char *__p_cdname(const u_char *, const u_char *, FILE *);
const u_char *__p_fqname(const u_char *, const u_char *, FILE *);
const u_char *__p_rr(const u_char *, const u_char *, FILE *);
const char *__p_type(int);
const char *__p_class(int);
const char *__p_option(u_long option);
int	 dn_comp(const char *, u_char *, int, u_char **, u_char **);
int	 dn_expand(const u_char *, const u_char *, const u_char *,
			char *, int);
int	 res_init(void);
/* Bind 4.9.3 original:	u_int16_t res_randomid(void); */
unsigned short	res_randomid(void);
int	 res_query(const char *, int, int, u_char *, int);
int	 res_search(const char *, int, int, u_char *, int);
int	 res_local_querydomain(const char *, const char *, int, int,
				u_char *, int);
int	 res_mkquery(int, const char *, int, int, const u_char *, int,
				const u_char *, u_char *, int);
int	 res_send(const u_char *, int, u_char *, int);
int	 res_isourserver(const struct sockaddr_in *);
int	 res_nameinquery(const char *, int, int,
				const u_char *, const u_char *);
int	 res_queriesmatch(const u_char *, const u_char *,
				const u_char *, const u_char *);
/* XXX - these last two don't belong in the resolver */
u_int	 inet_nsap_addr(const char *, u_char *, int maxlen);
char	*inet_nsap_ntoa(int, const u_char *, char *ascii);

#ifdef __cplusplus
}
#endif

#endif	/* _RESOLV_H */
