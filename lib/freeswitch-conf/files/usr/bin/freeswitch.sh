#!/bin/bash
#
# Copyright (C) 2004 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2004 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.

Action=RUN
Status=0
Args=""

iam=`whoami`

while [ $# -ne 0 ]
do
    case ${1} in
        --configtest)
            Action=CONFIGTEST
            ;;

        *)
            Args="$Args $1"
            ;;
    esac           

    shift # always consume 1
done

. /usr/libexec/sipx-utils.sh || exit 1

CONFIG_DEFS="/etc/sipxpbx/config.defs"
FS_CONFIG_DEFS="/etc/sipxpbx/freeswitch_config.defs"

CONFIG_FILES="\
  /usr/local/freeswitch/conf/freeswitch.xml \
  /usr/local/freeswitch/conf/sofia.conf.xml \
  /usr/local/freeswitch/conf/xml_rpc.conf.xml \
  /usr/local/freeswitch/conf/modules.conf.xml 
  "
# If the "config.defs" file exists and the <name>.in file exists for a
# configuration file, then run the config preprocessor to generate the
# fully resolved configuration file.

SIPXUID=`stat -c "%u" /etc/sipxpbx/sipxconfig.properties`
SIPXGID=`stat -c "%u" /etc/sipxpbx/sipxconfig.properties`
SIPXDIRS="/usr/local/freeswitch/log /usr/local/freeswitch/db /usr/local/freeswitch/conf"

if [ -f $CONFIG_DEFS -a -f $FS_CONFIG_DEFS ]
then
  cat $CONFIG_DEFS $FS_CONFIG_DEFS > /var/sipxdata/tmp/fs_config.defs
  for i in $CONFIG_FILES ; do
    if [ -f "${i}.in" ]
    then
       /usr/bin/configpp --defs /var/sipxdata/tmp/fs_config.defs --in "${i}.in" --out "$i"
    fi
  done
  rm -f /var/sipxdata/tmp/fs_config.defs
  chown -R $SIPXUID:$SIPXGID $SIPXDIRS
fi

case ${Action} in
   RUN)
     echo $$ > /var/run/sipxpbx/freeswitch.pid
     exec /usr/local/freeswitch/bin/freeswitch -nc -nf $Args
     exit 0
     ;;

   CONFIGTEST)
     WRONGPERM=`find $SIPXDIRS -not -uid $SIPXUID -or -not -gid $SIPXGID | wc -l`
     if [ $WRONGPERM -gt 0 ] 
     then
        if [ $iam = root ]
        then
           Status=0
           echo -e "\n Fixing FreeSWITCH folder permissions \n"
           chown -R $SIPXUID:$SIPXGID $SIPXDIRS
        else
           Status=1
           echo -e "\n Invalid FreeSWITCH folders & files permissions, run 'freeswitch.sh --configtest' as root to fix this \n"
        fi  
     fi
     # check validity of xml routing rules
#     /usr/bin/sipx-validate-xml /usr/local/freeswitch/conf/freeswitch.xml
#     Status=$?

     ;;
esac

exit $Status

