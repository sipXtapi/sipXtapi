#!/bin/sh
###
### Check date in ssl certificate
###


ExitStatus=0
ShowCert="NO"
Feedback=NORMAL
ExpireWarn=30
ExpireFail=0
Names=""

while [ $# -ne 0 ]
do
    case ${1} in
        ##
        ## Verbose
        ##
        -v|--verbose)
            Feedback=VERBOSE
            ;;

        ##
        ## 
        ##
        -w|--warning)
            if [ $# -lt 2 ]
            then
                echo "Must specify <warning-days> with ${1}" 1>&2
                Action=USAGE
                break
            else
                ExpireWarn=${2}
                shift # consume the switch
            fi
            ;;

        -f|--fail)
            if [ $# -lt 2 ]
            then
                echo "Must specify <failure-days> with ${1}" 1>&2
                Action=USAGE
                break
            else
                ExpireFail=${2}
                shift # consume the switch
            fi
            ;;

        -n|--name)
            if [ $# -lt 2 ]
            then
                echo "Must specify <name> with ${1}" 1>&2
                Action=USAGE
                break
            else
                Names="${Names} ${2}"
                shift # consume the switch
            fi
            ;;

        ##
        ## handle the 'end of options' marker
        ##
        --)
            ;;

        ##
        ## handle an unknown switch
        ##
        -*)
            Action=USAGE
            break
            ;;

        *)
            if [ -z "${Certificate}" ]
            then
                Certificate=${1}
            else
                echo "Too many arguments supplied: $@" 1>&2
                Action=USAGE
                break
            fi
            ;;
    esac           

    shift # always consume 1
done

if [ "${Action}" = "USAGE" ]
then
    cat <<USAGE

Usage:
    
    check-cert [ -v | --verbose ]
               [ {-w | --warning} <warning-days> ]
               [ {-f | --fail} <failure-days> ]
               [ {-n | --name} <name> ]...
               <certificate-file>

    Checks name and expiration of x509 certificate.

    Prints a warning if the certificate will expire
      in less than <warning-days> (default is 30)

    Returns failure (and prints message) if the certificate expires 
      in less than <failure-days> (default is 0)

    Always returns a failure if the certificate has expired.
    
    The --name option may be repeated any number of times, and if it is
    given, the commonName attribute of the certificate must equal one of
    the names.
    
USAGE
    exit
fi

warnSeconds=$((${ExpireWarn} * 3600 * 24))
failSeconds=$((${ExpireFail} * 3600 * 24))

if [ -r ${Certificate} ]
then

    # Check subject name if any correct answers are provided
    cert_name=`openssl x509 -in "${Certificate}" -subject -nameopt RFC2253,multiline -noout | perl -ne 'use English; m/^ +commonName += / && print $POSTMATCH'`

    if [ -n "${Names}" ]
    then
        UniqueNames=`for n in ${Names}; do echo $n; done | sort -u`
        Matched="NO"
        for name in ${UniqueNames}
        do 
          if [ "${name}" = "${cert_name}" ]; then Matched="YES"; fi
        done
        if [ "${Matched}" = "NO" ]
        then
            echo -n "SSL certificate name '${cert_name}' is not one of: " 1>&2
            for name in ${UniqueNames}; do echo -n "'${name}' " 1>&2; done
            echo "" 1>&2
            ExitStatus=1
            ShowCert="SHOW"
        fi
    fi

    # Check expiration
    cert_expires=`openssl x509 -in "${Certificate}" -noout -dates | awk -F = '$1 == "notAfter" {print $2}'`
    now=`date +%s`
    if [ $? -eq 0 ] # date command support epoch format
    then
        exp=`date --date="${cert_expires}" +%s`
        remaining=$(($exp - $now))

        if [ ${remaining} -le 0 ] # cert expired
        then
            echo "SSL certificate expired: ${cert_expires}" 1>&2
            ExitStatus=1

        elif [ ${failSeconds} -gt 0 -a ${remaining} -le ${failSeconds} ]
        then
            echo "SSL certificate expires in less than ${ExpireFail} days: ${cert_expires}" 1>&2
            ExitStatus=1

        elif [ ${remaining} -le ${warnSeconds} ]
        then
            echo "SSL certificate expires in less than ${ExpireWarn} days: ${cert_expires}" 1>&2
            ShowCert="SHOW"

        fi
    else
        echo "Your 'date' command does not support %s format - cannot calculate expiration." 1>&2
        echo "SSL certificate expires: ${cert_expires}" 1>&2
        ShowCert="SHOW"
    fi

    if [ "${Feedback}" = "VERBOSE" ]
    then
        echo "SSL certificate name:    ${cert_name}" 
        echo "SSL certificate expires: ${cert_expires}" 
    fi

else
    echo "SSL certificate not found." 1>&2
    ExitStatus=1
fi

if [ ${ExitStatus} -ne 0 -o "${ShowCert}" = "SHOW" ]
then
    echo "SSL certificate: ${Certificate}" 1>&2
fi

exit ${ExitStatus}
