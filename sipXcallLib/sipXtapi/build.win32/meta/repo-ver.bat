svn info . | sed -n -e /Revision:/p | cut -c 11-
exit /b %0
