@ECHO OFF
svn info . | sed -n -e /Revision:/p | cut -c 11-
