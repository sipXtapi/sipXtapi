#!/usr/bin/perl
#
# ***********************************************************************
#
# Description: A perl script to perform various user & device operations
#              on the SIPxchange Config server, using the SOAP interface.
#              SOAP::Lite package provides an interface to SOAP, and is
#              required to run this script. The package can be found at
#              www.soaplite.com
#
#              config.dat must be present on the local directory and
#              contains the configuration of SIPxchange config server.
#
# Usage      : perl sipx_soap_update.pl <input_data_file>
#              where input_data_file is the comma separated data file
#              each line containing a record to be updated on the config
#              server. The format of the input file is:
#              Op_Code,<field1>,<field2>
#              where field1, field2 etc are the various fields
#              coresponding to the operation OpCode.  See the Op_Code
#              syntax described below.
#
#              Eg: USER_DELETE, userId6
#                  where USER_DELETE is the operation
#                        userId6 is the field
#
# Output     : The error records and failed updates are logged in a
#              separate file (name chosen by script using PID), so the
#              script can be re-run after correcting errors if any.
#
# ***********************************************************************
#
# The following is a list of the syntax for the current Op_Codes supported
# by the configuration server SOAP APIs
#
# USER_CREATE,userid,pin,firstname,lastname,extension,alias,group
# Example:
# USER_CREATE,userId5,12345678,firstName,lastName,333,alias1,soap
#
# USER_DELETE,userid
#
# USER_EDIT,olduserid,newuserid,pin,firstname,lastname,extension,alias,group
#
# USER_CREATELINE,userid,PROVISION|REGISTER,ENABLE|DISABLE,sipaddress
#
# USER_DELETELINE,userid
#
# CREATE_CREDENTIAL,userid,sipaddress,realm,realmid,realmpassword
#
# DELETE_CREDENTIAL --not tested yet--- (3 args)
#
# USER_SEND_PROFILES,userid
#
# SEND_ALL_PROFILES (no arguments)
#
# RESYNC_ALL_DATASETS (no arguments)
#
# DEVICE_CREATE,serial,devicetype,name,group,description
# Device type = ixpressa_x86_win32  | xpressa_strongarm_vxworks | 7960 | 7940
# Example:
# DEVICE_CREATE,123456789012,ixpressa_x86_win32,dev1,soap,desc
#
# DEVICE_ASSIGN,userid,serial
# Example:
# DEVICE_ASSIGN,userId5,123456789012
#
# DEVICE_DELETE,serial
#
# DEVICE_EDIT,oldserial,newserial,name,group,description
#
# DEVICE_UNASSIGN,serial
#
# ***********************************************************************

use SOAP::Lite;
require "./sipx_import.dat";

# Verify if input data file (comma-separated) is passed as command line parameter
if ($#ARGV < 0)
{
    die "Usage: perl sipx_import.pl <user_data_file>\n" ;
}

# Open input data file
$input_file = $ARGV[0] ;

# Open output file for writing for "bad" records
$error_file = err_adduser.$$;

open( FILE,  "< $input_file") or die "ERROR: Opening $input_file\n";
open( ERR,  "> $error_file") or die "ERROR: Opening $error_file\n";

$FOUND_ERROR = 0 ;

# Loop thru the input and process each Op-code
while($line = <FILE>)
{
    $cmd_result = 1;

    chomp($line);
    @input_record = split(/,/, $line);
    $op_code = $input_record[0];

    if ($op_code)
    {
        debug_print("\nProcessing record [$line]") ;

        my $cmd_result = execute_command( @input_record);
        if ($cmd_result == 0)
        {
            # Write failed record to output file
            print ERR "$line\n";
	    $FOUND_ERROR = 1 ;
        }
    }
    else
    {
        print ("No Op_Code found or bad record\n", "");
    }
}
if ($FOUND_ERROR == 1)
{
    print ("\n\nError(s) occured while processing $input_file. See $error_file for failed records. Correct errors and re-run the script with $error_file as input.\n", "");
}
else
{
    unlink $error_file;
}

close(FILE);
close(ERR);


# ***********************************************************************
# Execute the operation specified for the record
# ***********************************************************************
sub execute_command 
{
    ($op_code, $arg1, $arg2, 
     $arg3, $arg4, $arg5,
     $arg6, $arg7, $arg8) = @_;

    # Type cast all values to string bcuz numeric values extensions and passwords
    # are passed as integers by Perl

    $arg1 = SOAP::Data->type(string => $arg1) ;
    $arg2 = SOAP::Data->type(string => $arg2) ;
    $arg3 = SOAP::Data->type(string => $arg3) ;
    $arg4 = SOAP::Data->type(string => $arg4) ;
    $arg5 = SOAP::Data->type(string => $arg5) ;
    $arg6 = SOAP::Data->type(string => $arg6) ;
    $arg7 = SOAP::Data->type(string => $arg7) ;
    $arg8 = SOAP::Data->type(string => $arg8) ;

        if ($op_code eq "USER_CREATELINE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> addUserLine( $arg1, $arg2, $arg3, $arg4 );
        process_output($op_result->faultstring, $op_result->faultcode) ;
    }
    elsif ($op_code eq "USER_DELETELINE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> deleteUserLine( $arg1, $arg2 );
			   
        process_output($op_result->faultstring, $op_result->faultcode) ;
    }
    elsif ($op_code eq "CREATE_CREDENTIAL")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> addUserCredential( $arg1,
                           $arg2 ,
                           $arg3 ,
                           $arg4 ,
                           $arg5 );
        process_output($op_result->faultstring, $op_result->faultcode) ;
    }
    elsif ($op_code eq "DELETE_CREDENTIAL")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> deleteLineCredential( $arg1,
                           $arg2 ,
                           $arg3 );
        process_output($op_result->faultstring, $op_result->faultcode) ;
    }
    elsif ($op_code eq "USER_CREATE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> createUser( $arg1,
                           $arg2 ,
                           $arg3 ,
                           $arg4 ,
                           $arg5 ,
                           $arg6 ,
                           $arg7 );
        process_output($op_result->faultstring, $op_result->faultcode) ;
    }
    elsif ($op_code eq "USER_EDIT")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> editUser( $arg1,
                         $arg2 ,
                         $arg3 ,
                         $arg4 ,
                         $arg5 ,
                         $arg6 ,
                         $arg7 ,
                         $arg8 );
        process_output($op_result->faultstring, $op_result->faultcode); 
    }
    elsif ($op_code eq "USER_DELETE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> deleteUser( $arg1);

        process_output($op_result->faultstring, $op_result->faultcode);
    }
    elsif ($op_code eq "DEVICE_CREATE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_DEVICE )
            -> createDevice( $arg1,
                             $arg2 ,
                             $arg3 ,
                             $arg4 ,
                             $arg5 );
        process_output($op_result->faultstring, $op_result->faultcode);
    }
    elsif ($op_code eq "DEVICE_EDIT")
    {                  
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_DEVICE )
            -> editDevice( $arg1 ,
                           $arg2 ,
                           $arg3 ,
                           $arg4 ,
                           $arg5 );
        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "DEVICE_DELETE")
    {
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_DEVICE )
            -> deleteDevice( $arg1);

        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "DEVICE_ASSIGN")
    {                  
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> assignDevice( $arg1 , $arg2 );
        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "DEVICE_UNASSIGN")
    {                  
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_USER )
            -> unassignDevice( $arg1 );
        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "RESYNC_ALL_DATASETS")
    {                  
        my $op_result = SOAP::Lite
            -> proxy ($PROXY_CMD_DATASET )
            -> rebuildDataSets();

        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "SEND_ALL_PROFILES")
    {                  
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_PROFILE )
            -> sendAllProfiles();

        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    elsif ($op_code eq "USER_SEND_PROFILES")
    {                  
        my $op_result = SOAP::Lite
            -> proxy($PROXY_CMD_PROFILE )
            -> sendProfilesForUser($arg1);

        process_output($op_result->faultstring, $op_result->faultcode);
    }       
    else
    {
        print ("Invalid or unsupported command $op_code found in $input_file\n", "");
    }
    return $cmd_result;
}

# **************************************************************************
# Set flag to write "bad" record to output file and print logging
# information based on DEBUG mode
# **************************************************************************

sub process_output
{
    ($msg, $code) = @_;
    if ($code)
    {
        $cmd_result = 0;
        print ("ERROR: $code - $msg\n");
    }
    else
    {
        debug_print("SUCCESS: record successfully updated") ;
    }
}

# **************************************************************************
# print statements to be processed only if logging enabled
# **************************************************************************
sub debug_print
{
    if ($DEBUG_MODE)
    {
      ($print_info) = @_;
      printf( "%s\n" , $print_info ) ;
    }
}
