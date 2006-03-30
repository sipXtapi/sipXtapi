#!/bin/sh
###
### Utility script functions common to server startup scripts
###

sipx_config_value() { # ( CONFIG-FILE, DIRECTIVE )
#   prints the value of DIRECTIVE from CONFIG-FILE 
#   list values have any commas removed
    perl -n \
        -e 'use English;' \
        -e 's/#.*$//;' \
        -e "/^\\s*$2\\s*:\\s*/ && print join( ' ', split( /[\\s,]+/, \$POSTMATCH ));" \
        $1
}

dns_sipsrv () { # ( TRANSPORT, DOMAIN )
  # returns the SRV host name for DOMAIN over TRANSPORT 
  target=`echo _sip._$1.$2 | tr A-Z a-z`
  dig  -t srv +noidentify +nocmd +nocomments +noquestion +nostats +noauthority ${target} \
  | tr A-Z a-z \
  | awk "\$1 == \"${target}.\" { print \$NF }" \
  | sed 's/\.$//'
  }

dns_cname () { # ( DOMAIN )
  # returns the CNAME resolution for DOMAIN
  target=`echo $1 | tr A-Z a-z`
  dig  -t cname +noidentify +nocmd +nocomments +noquestion +nostats +noauthority ${target} \
  | tr A-Z a-z \
  | awk "\$1 == \"${target}.\" { print \$NF }" \
  | sed 's/\.$//'
  }

sip_resolves_to () { # ( unresolved, target )
    # returns true (0) if the unresolved name resolves to the target name by sip rules
     AliasMatched=1

     unresolvedName=$1
     targetName=$2

     if [ "${targetName}" == "${unresolvedName}" ]
     then
         AliasMatched=0
     else
         tcpSrv=`dns_sipsrv tcp ${unresolvedName}`
         udpSrv=`dns_sipsrv udp ${unresolvedName}`
         cName=`dns_cname ${unresolvedName}`
         for tryName in ${tcpSrv} ${udpSrv} ${cName}
         do
           if [ "${targetName}" = "${tryName}" ] 
           then
               AliasMatched=0
               break
           fi
         done

     fi

     return $AliasMatched
}

