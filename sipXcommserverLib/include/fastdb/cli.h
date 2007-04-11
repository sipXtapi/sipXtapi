/*-< CLI.H >---------------------------------------------------------*--------*
 * FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
 * (Main Memory Database Management System)                          *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:     13-Jan-2000 K.A. Knizhnik   * / [] \ *
 *                          Last update: 13-Jan-2000 K.A. Knizhnik   * GARRET *
 *-------------------------------------------------------------------*--------*
 * Call level interface to FastDB server
 *-------------------------------------------------------------------*--------*/

#ifndef __CLI_H__
#define __CLI_H__

#include <stdlib.h>

#ifndef FASTDB_DLL_ENTRY
#ifdef FASTDB_DLL
#ifdef INSIDE_FASTDB
#define FASTDB_DLL_ENTRY __declspec(dllexport)
#else
#define FASTDB_DLL_ENTRY __declspec(dllimport)
#endif
#else
#define FASTDB_DLL_ENTRY
#endif
#endif

#ifdef __cplusplus
extern "C" { 
#endif

enum cli_result_code { 
    cli_ok = 0,
    cli_bad_address = -1,
    cli_connection_refused = -2,
    cli_database_not_found = -3, 
    cli_bad_statement = -4,
    cli_parameter_not_found = -5,
    cli_unbound_parameter = -6,
    cli_column_not_found = -7,
    cli_incompatible_type = -8,
    cli_network_error = -9,
    cli_runtime_error = -10,
    cli_bad_descriptor = -11,
    cli_unsupported_type = -12,
    cli_not_found        = -13,
    cli_not_update_mode  = -14,
    cli_table_not_found  = -15,
    cli_not_all_columns_specified = -16, 
    cli_not_fetched = -17,
    cli_already_updated = -18, 
    cli_table_already_exists = -19, 
    cli_not_implemented = -20
};
    
enum cli_var_type { 
    cli_oid,
    cli_bool, 
    cli_int1, 
    cli_int2,
    cli_int4,
    cli_int8,
    cli_real4,
    cli_real8,
    cli_decimal, /* not supported */
    cli_asciiz,  /* zero terminated string */
    cli_pasciiz, /* pointer to zero terminated string */
    cli_cstring, /* not supported */
    cli_array_of_oid,
    cli_array_of_bool, 
    cli_array_of_int1, 
    cli_array_of_int2,
    cli_array_of_int4,
    cli_array_of_int8,
    cli_array_of_real4,
    cli_array_of_real8,
    cli_array_of_decimal, 
    cli_array_of_string,
    cli_any,      /* not supported */
    cli_datetime, /* not supported */
    cli_autoincrement, 
    cli_rectangle,/* not supported */
    cli_unknown
};

typedef char         cli_bool_t;
typedef signed char  cli_int1_t;
typedef signed short cli_int2_t;
typedef signed int   cli_int4_t;
typedef float        cli_real4_t;
typedef double       cli_real8_t;
    
#if (defined(_WIN32) || defined(__BORLANDC__)) && !defined(__MINGW32__)
typedef __int64      cli_int8_t;
#else
#if defined(__osf__ )
typedef signed long  cli_int8_t;
#else
#if defined(__GNUC__) || defined(__SUNPRO_CC)
typedef signed long long cli_int8_t;
#else
#error "integer 8 byte type is not defined" 
#endif
#endif
#endif

#ifndef CLI_OID_DEFINED
typedef long cli_oid_t;
#endif

typedef struct cli_array_t { 
    size_t size;
    void*  data;
    size_t allocated;
} cli_array_t;
    
/*********************************************************************
 * cli_open
 *     Establish connection with the server 
 * Parameters:
 *     server_url - zero terminated string with server address and port, 
 *                  for example "localhost:5101", "195.239.208.240:6100",...
 *     max_connect_attempts  - number of attempts to establish connection
 *     reconnect_timeout_sec - timeput in seconds between connection attempts
 * Returns:
 *     >= 0 - connectiondescriptor to be used in all other cli calls
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_open(char const* server_url, 
                  int         max_connect_attempts,
                  int         reconnect_timeout_sec);

enum cli_open_attributes { 
    cli_open_default    = 0x0, 
    cli_open_readonly   = 0x1, 
    cli_open_truncate   = 0x2,
    cli_open_concurrent = 0x4
};
/*********************************************************************
 * cli_create
 *     Create connection to the local database
 * Parameters:
 *     databaseName - name of the database 
 *     fileName - path to the database file 
 *     transactionCommitDelay - trasnaction commit delay (specify 0 to disable)
 *     openAttr - mask of cli_open_attributes
 *     initDatabaseSize - initial size of the database
 *     extensionQuantum - database extension quantum
 *     initIndexSize - initial size of object index
 *     fileSizeLimit - limit for file size (0 - unlimited)
 * Returns:
 *     >= 0 - connection descriptor to be used in all other cli calls
 *     <  0 - error code as described in cli_result_code enum
 */

int FASTDB_DLL_ENTRY cli_create(char const* databaseName, 
                char const* filePath, 
                unsigned    transactionCommitDelay, 
                int         openAttr, 
                size_t      initDatabaseSize,
                                size_t      extensionQuantum,
                                size_t      initIndexSize,
                                size_t      fileSizeLimit);
    
/*********************************************************************
 * cli_create_replication_node
 *     Create connection to the local database with support of replication
 * Parameters:
 *     nodeId - node identifier: 0 <= nodeId < nServers
 *     nServers - number of replication nodes (primary + standby)
 *     nodeNames - array with URLs of the nodes (address:port)
 *     databaseName - name of the database 
 *     fileName - path to the database file 
 *     transactionCommitDelay - trasnaction commit delay (specify 0 to disable)
 *     openAttr - mask of cli_open_attributes (to allow concurrent read access to replication node, 
 *                cli_open_concurrent attribute should be set) 
 *     initDatabaseSize - initial size of the database
 *     extensionQuantum - database extension quantum
 *     initIndexSize - initial size of object index
 *     fileSizeLimit - limit for file size (0 - unlimited)
 * Returns:
 *     >= 0 - connection descriptor to be used in all other cli calls
 *     <  0 - error code as described in cli_result_code enum
 */

int FASTDB_DLL_ENTRY cli_create_replication_node(int         nodeId,
                                                 int         nServers,
                                                 char*       nodeNames[],
                                                 char const* databaseName, 
                                                 char const* filePath, 
                                                 int         openAttr, 
                                                 size_t      initDatabaseSize,
                                                 size_t      extensionQuantum,
                                                 size_t      initIndexSize,
                                                 size_t      fileSizeLimit);

/*********************************************************************
 * cli_close
 *     Close session
 * Parameters:
 *     session - session descriptor returned by cli_open
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_close(int session);

/*********************************************************************
 * cli_statement
 *     Specify SubSQL statement to be executed at server
 *     Binding to the parameters and columns can be established       
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     stmt    - zero terminated string with SubSQL statement  
 * Returns:
 *     >= 0 - statement descriptor
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_statement(int session, char const* stmt);

/*********************************************************************
 * cli_parameter
 *     Bind parameter to the statement
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 *     param_name - zero terminated string with parameter name  
 *                  Paramter name should start with '%'
 *     var_type   - type of variable as described in cli_var_type enum.
 *                  Only scalar and zero terminated string types are supported.
 *     var_ptr    - pointer to the variable
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_parameter(int         statement,
                   char const* param_name, 
                   int         var_type,
                   void*       var_ptr);

/*********************************************************************
 * cli_column
 *     Bind extracted column of select or insert statement
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 *     column_name - zero terminated string with column name  
 *     var_type    - type of variable as described in cli_var_type enum
 *     var_len     - pointer to the variable to hold length of array variable.
 *                   This variable should be assigned the maximal length
 *                   of the array/string buffer, pointed by var_ptr.
 *                   After the execution of the statement it is assigned the 
 *                   real length of the fetched array/string. If it is large 
 *                   than length of the buffer, then only part of the array
 *                   will be placed in the buffer, but var_len still will 
 *                   contain the actual array length. 
 *     var_ptr     - pointer to the variable
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_column(int         statement,
                char const* column_name, 
                int         var_type, 
                int*        var_len, 
                void*       var_ptr);


typedef void* (*cli_column_set)(int var_type, void* var_ptr, int len);
typedef void* (*cli_column_get)(int var_type, void* var_ptr, int* len);

typedef void* (*cli_column_set_ex)(int var_type, void* var_ptr, int len, 
                   char const* column_name, int statement, void const* data_ptr);
typedef void* (*cli_column_get_ex)(int var_type, void* var_ptr, int* len, 
                   char const* column_name, int statemen);

/*********************************************************************
 * cli_array_column
 *     Specify get/set functions for the array column
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 *     column_name - zero terminated string with column name  
 *     var_type    - type of variable as described in cli_var_type enum
 *     var_ptr     - pointer to the variable
 *     set         - function which will be called to construct fetched 
 *                   field. It receives pointer to the variable, 
 *                   length of the fetched array and returns pointer to th 
 *                   array's elements
 *     get         - function which will be called to update the field in the 
 *                   database. Given pointer to the variable, it should return 
 *                   pointer to the array elements and store length of the
 *                   array to the variable pointer by len parameter
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_array_column(int            statement,
                      char const*    column_name, 
                      int            var_type,
                      void*          var_ptr,
                      cli_column_set set,
                      cli_column_get get);
    
int FASTDB_DLL_ENTRY cli_array_column_ex(int               statement,
                     char const*       column_name, 
                     int               var_type,
                     void*             var_ptr,
                     cli_column_set_ex set,
                     cli_column_get_ex get);
    
enum { 
    cli_view_only, 
    cli_for_update
};

/*********************************************************************
 * cli_fetch
 *     Execute select statement.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 *     for_update - not zero if fetched rows will be updated 
 * Returns:
 *     >= 0 - success, for select statements number of fetched rows is returned
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_fetch(int statement, int for_update);

/*********************************************************************
 * cli_insert
 *     Execute insert statement.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 *     oid        - object identifier of created record. 
 * Returns:
 *     status code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_insert(int statement, cli_oid_t* oid);

/*********************************************************************
 * cli_get_first
 *     Get first row of the selection.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_get_first(int statement);

/*********************************************************************
 * cli_get_last
 *     Get last row of the selection.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_get_last(int statement);

/*********************************************************************
 * cli_get_next
 *     Get next row of the selection. If get_next records is called
 *     exactly after cli_fetch function call, is will fetch the first record
 *     in selection.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_get_next(int statement);

/*********************************************************************
 * cli_get_prev
 *     Get previous row of the selection. If get_next records is called
 *     exactly after cli_fetch function call, is will fetch the last record
 *     in selection.
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_get_prev(int statement);

/*********************************************************************
 * cli_skip
 *     Skip specified number of rows. 
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 *     n          - number of objects to be skipped
 *                - if "n" is positive, then this function has the same effect as
 *                     executing cli_get_next() function "n" times.
 *                - if "n" is negative, then this function has the same effect as
 *                     executing cli_get_prev() function "-n" times.
 *                - if "n"  is zero, this method just reloads current record
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_skip(int statement, int n);

/*********************************************************************
 * cli_seek
 *    Position cursor to the record with specified OID
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 *     oid         - object identifier of the record to which cursor should be positioned
 * Returns:
 *     >= 0 - success, position of the record in the selection
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_seek(int statement, cli_oid_t oid);


/*********************************************************************
 * cli_get_oid
 *     Get object identifier of the current record
 * Parameters:
 *     statement  - statememt descriptor returned by cli_statement
 * Returns:
 *     object identifier or 0 if no object is seleected
 */
cli_oid_t FASTDB_DLL_ENTRY cli_get_oid(int statement);

/*********************************************************************
 * cli_update
 *     Update the current row in the selection. You have to set
 *     for_update parameter of cli_fetch to 1 in order to be able 
 *     to perform updates. Updated value of row fields will be taken
 *     from bound column variables. 
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_update(int statement);

/*********************************************************************
 * cli_remove
 *     Remove all selected records. You have to set
 *     for_update parameter of cli_fetch to 1 in order to be able 
 *     to remove records. 
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_remove(int statement);

/*********************************************************************
 * cli_free
 *     Deallocate statement and all associated data
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_free(int statement);

/*********************************************************************
 * cli_commit
 *     Commit current database transaction
 * Parameters:
 *     session - session descriptor as returned by cli_open
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_commit(int session);

/*********************************************************************
 * cli_precommit
 *     Release all locks set by transaction. This methods allows other clients
 *     to proceed, but it doesn't flush transaction to the disk.
 * Parameters:
 *     session - session descriptor as returned by cli_open
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_precommit(int session);

/*********************************************************************
 * cli_abort
 *     Abort current database transaction
 * Parameters:
 *     session - session descriptor as returned by cli_open
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_abort(int session);


enum cli_field_flags { 
    cli_hashed           = 1, /* field should be indexed usnig hash table */
    cli_indexed          = 2  /* field should be indexed using B-Tree */
};

typedef struct cli_field_descriptor { 
    enum cli_var_type type;
    int               flags;
    char const*       name;
    char const*       refTableName;
    char const*       inverseRefFieldName;
} cli_field_descriptor;

/*********************************************************************
 * cli_describe
 *     Describe fileds of specified table
 * Parameters:
 *     session - session descriptor as returned by cli_open
 *     table   - name of the table
 *     fields  - address of the pointer to the array of fields descriptors, 
 *               this array should be later deallocated by application by cli_free_memory()
 * Returns:
 *     >= 0 - number of fields in the table
 *     < 0  - result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_describe(int session, char const* table, cli_field_descriptor** fields);


typedef struct cli_table_descriptor {
    char const*       name;
} cli_table_descriptor;

/*********************************************************************
 * cli_show_tables
 *     Show all tables of specified database
 * Parameters:
 *     session - session descriptor as returned by cli_open
 *     tables  - address of the pointer to the array of tables descriptors,
 *               this array should be later deallocated by application by cli_free_memory()
 * Returns:
 *     >= 0 - number of tables in the database (Metatable is not returned/counted)
 *     < 0  - result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_show_tables(int session, cli_table_descriptor** tables);


/*********************************************************************
 * cli_create_table
 *     Create new table
 * Parameters:
 *     session   - session descriptor as returned by cli_open
 *     tableName - name of new table
 *     nFields   - number of columns in the table
 *     fields    - array with table columns descriptors
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_create_table(int session, char const* tableName, int nFields, 
                    cli_field_descriptor* fields);

/*********************************************************************
 * cli_drop_table
 *     drop the table
 * Parameters:
 *     session   - session descriptor as returned by cli_open
 *     tableName - name of deleted table
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_drop_table(int session, char const* tableName);


/*********************************************************************
 * cli_alter_index
 *     add or remove column index
 * Parameters:
 *     session   - session descriptor as returned by cli_open
 *     tableName - name of the table
 *     fieldName - name of field
 *     newFlags  - new flags of the field, if index exists for this field, but is not specified in 
 *                 <code>newFlags</code> mask, then it will be removed; if index not exists, but is 
 *                 specified in <code>newFlags</code> mask, then it will be created. *                   
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_alter_index(int session, char const* tableName, char const* fieldName, 
                                     int newFlags);


/*********************************************************************
 * cli_set_error_handler
 *     Set FastDB erro handler. Handler should be no-return function which perform stack unwind.
 * Parameters:
 *     session   - session descriptor as returned by cli_open
 *     handler   - error handler
 * Returns:
 *     previous handler
 */
enum cli_error_class { 
    cli_no_error, 
    cli_query_error,
    cli_arithmetic_error,
    cli_index_out_of_range_error,
    cli_database_open_error,
    cli_file_error,
    cli_out_of_memory_error,
    cli_deadlock,
    cli_null_reference_error,
    cli_lock_revoked,
    cli_file_limit_exeeded        
};
typedef void (*cli_error_handler)(int error, char const* msg, int msgarg); 
cli_error_handler FASTDB_DLL_ENTRY cli_set_error_handler(int session, cli_error_handler new_handler);

/*********************************************************************
 * cli_freeze
 *    Freeze cursor. Make it possible to reused cursor after commit of the current transaction.
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_freeze(int statement);

/*********************************************************************
 * cli_unfreeze
 *    Unfreeze cursor. Reuse previously frozen cursor.
 * Parameters:
 *     statement   - statememt descriptor returned by cli_statement
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_unfreeze(int statement);


/*********************************************************************
 * cli_attach
 *    Attach thread to the database. Each thread except one opened the database should first
 *    attach to the database before any access to the database, and detach after end of the work with database
 * Parameters:
 *     session - session descriptor returned by cli_open
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_attach(int session);

/*********************************************************************
 * cli_detach
 *    Detach thread from the database. Each thread except one opened the database should perform 
 *    attach to the database before any access to the database, and detach after end of the work with database
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     detach_mode - bit mask representing detach mode
 * Returns:
 *     result code as described in cli_result_code enum
 */
enum cli_detach_mode {
    cli_commit_on_detach          = 1,
    cli_destroy_context_on_detach = 2
};

int FASTDB_DLL_ENTRY cli_detach(int session, int detach_mode);


/*********************************************************************
 * cli_free_memory
 *    Free memory allocated by cli_show_tables and cli_describe
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     ptr - pointer to the allocated buffer
 */
void FASTDB_DLL_ENTRY cli_free_memory(int session, void* ptr);


typedef struct cli_database_monitor {
    int n_readers;
    int n_writers;
    int n_blocked_readers;
    int n_blocked_writers;
    int n_users;
} cli_database_monitor;

/*********************************************************************
 * cli_get_database_state
 *    Obtain information about current state of the database
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     monitor - pointer to the monitor structure. The folloing fields are set:
 *       n_readers: number of granted shared locks
 *       n_writers: number of granted exclusive locks
 *       n_blocked_reader: number of threads which shared lock request was blocked
 *       n_blocked_writers: number of threads which exclusive lock request was blocked
 *       n_users: number of processes openned the database
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_get_database_state(int session, cli_database_monitor* monitor);



/*********************************************************************
 * cli_set_trace_function
 *    Set trace function which will be used to output FastDB trace messages
 * Parameters:
 *     func - pointer to trace function which receives trace message terminated with new line character
 */
typedef void (*cli_trace_function_t)(char* msg);
void FASTDB_DLL_ENTRY cli_set_trace_function(cli_trace_function_t func);


/*********************************************************************
 * cli_prepare_query
 *     Prepare SubSQL query statement. 
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     query   - query string with optional parameters. Parameters are specified
 *               as '%T' where T is one or two character code of parameter type using the same notation
 *               as in printf: %d or %i - int, %f - float or double, %ld - int8, %s - string, %p - oid...
 * Returns:
 *     >= 0 - statement descriptor
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_prepare_query(int session, char const* query);

/**
 * cli_execute_query
 *     Execute query previously prepared by cli_prepare_query
 * Parameters:
 *     statement - statement descriptor returned by cli_prepare_query
 *     for_update - not zero if fetched rows will be updated 
 *     record_struct - structure to receive selected record fields
 *     ...     - varying list of query parameters
 * Returns:
 *     >= 0 - success, for select statements number of fetched rows is returned
 *     <  0 - error code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_execute_query(int statement, int for_update, void* record_struct, ...);

/**
 * cli_insert_struct
 *     Insert new record represented as C structure
 * Parameters:
 *     session - session descriptor returned by cli_open
 *     table_name - name of the destination table
 *     record_struct - structure specifying value of record fields
 *     oid - pointer to the location to receive OID of created record (may be NULL)
 * Returns:
 *     result code as described in cli_result_code enum
 */
int FASTDB_DLL_ENTRY cli_insert_struct(int session, char const* table_name, void* record_struct, cli_oid_t* oid);

#ifdef __cplusplus
}
#endif

#endif

