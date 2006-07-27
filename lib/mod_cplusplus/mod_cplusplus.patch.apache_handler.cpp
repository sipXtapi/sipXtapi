--- src/apache_handler.cpp.orig	2004-05-23 21:34:27.000000000 -0400
+++ src/apache_handler.cpp	2004-09-19 11:43:51.000000000 -0400
@@ -1,10 +1,11 @@
 #define EXPORT_MODCPP
 
 #include "apr_dso.h"
 #include "apache_handler.h"
 #include "apache_protocol.h"
+#include "apache_filters.h"
 
 extern "C" {
     
 #define CALL_REQ_FUNCTION(function_name) \
     int result = DECLINED; \
