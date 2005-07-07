#!/bin/sh
###
### Check date in ssl certificate
###


ExitStatus=0
ShowCert="NO"
Feedback=NORMAL
ExpireWarn=30
ExpireFail=0

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
               [ {-f | -fail} <failure-days> ]
               <certificate-file>

    Checks expiration of x509 certificate.

    Prints a warning if the certificate will expire
      in less than <warning-days> (default is 30)

    Returns failure (and prints message) if the certificate expires 
      in less than <failure-days> (default is 0)

    Always returns a failure if the certificate has expired.
    
USAGE
    exit
fi

warnSeconds=$((${ExpireWarn} * 3600 * 24))
failSeconds=$((${ExpireFail} * 3600 * 24))

if [ -r ${Certificate} ]
then
    cert_expires=`openssl x509 -in ${Certificate} -noout -dates | awk -F = '$1 == "notAfter" {print $2}'`
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

        elif [ "${Feedback}" = "VERBOSE" ]
        then
            echo "SSL certificate expires: ${cert_expires}" 
        fi

    else
        echo "Your 'date' command does not support %s format - cannot calculate expiration." 1>&2
        echo "SSL certificate expires: ${cert_expires}" 1>&2
        ShowCert="SHOW"
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
