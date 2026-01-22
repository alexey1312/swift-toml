#ifndef CTOML_H
#define CTOML_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle types
typedef struct ctoml_document* ctoml_document_t;
typedef struct ctoml_node* ctoml_node_t;

// Node types
typedef enum {
    CTOML_TYPE_NONE = 0,
    CTOML_TYPE_STRING,
    CTOML_TYPE_INTEGER,
    CTOML_TYPE_FLOAT,
    CTOML_TYPE_BOOLEAN,
    CTOML_TYPE_DATE,
    CTOML_TYPE_TIME,
    CTOML_TYPE_DATETIME,
    CTOML_TYPE_ARRAY,
    CTOML_TYPE_TABLE
} ctoml_type_t;

// Date/Time structures
typedef struct {
    int32_t year;
    int32_t month;
    int32_t day;
} ctoml_date_t;

typedef struct {
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t nanosecond;
} ctoml_time_t;

typedef struct {
    ctoml_date_t date;
    ctoml_time_t time;
    bool has_offset;
    int32_t offset_minutes;
} ctoml_datetime_t;

// Error information
typedef struct {
    int64_t line;
    int64_t column;
    const char* message;  // Valid until document is freed
} ctoml_error_t;

// ============================================================================
// Document functions
// ============================================================================

/// Parse a TOML string and return a document handle.
/// @param input UTF-8 encoded TOML string
/// @param length Length of input in bytes
/// @return Document handle, or NULL on allocation failure
ctoml_document_t ctoml_parse(const char* input, size_t length);

/// Check if parsing was successful.
/// @param doc Document handle
/// @return true if parsing succeeded, false if there was a syntax error
bool ctoml_document_is_valid(ctoml_document_t doc);

/// Get the error information for a failed parse.
/// @param doc Document handle
/// @return Error information (valid until document is freed)
ctoml_error_t ctoml_document_get_error(ctoml_document_t doc);

/// Get the root table of the document.
/// @param doc Document handle
/// @return Root node handle (valid until document is freed)
ctoml_node_t ctoml_document_get_root(ctoml_document_t doc);

/// Free a document and all associated resources.
/// @param doc Document handle
void ctoml_document_free(ctoml_document_t doc);

// ============================================================================
// Node type functions
// ============================================================================

/// Get the type of a node.
/// @param node Node handle
/// @return Node type
ctoml_type_t ctoml_node_get_type(ctoml_node_t node);

// ============================================================================
// Scalar value functions
// ============================================================================

/// Get the string value of a string node.
/// @param node Node handle
/// @param length Out parameter for string length (can be NULL)
/// @return Pointer to string data (valid until document is freed)
const char* ctoml_node_get_string(ctoml_node_t node, size_t* length);

/// Get the integer value of an integer node.
/// @param node Node handle
/// @return Integer value
int64_t ctoml_node_get_integer(ctoml_node_t node);

/// Get the float value of a float node.
/// @param node Node handle
/// @return Float value
double ctoml_node_get_float(ctoml_node_t node);

/// Get the boolean value of a boolean node.
/// @param node Node handle
/// @return Boolean value
bool ctoml_node_get_boolean(ctoml_node_t node);

/// Get the date value of a date node.
/// @param node Node handle
/// @return Date value
ctoml_date_t ctoml_node_get_date(ctoml_node_t node);

/// Get the time value of a time node.
/// @param node Node handle
/// @return Time value
ctoml_time_t ctoml_node_get_time(ctoml_node_t node);

/// Get the datetime value of a datetime node.
/// @param node Node handle
/// @return DateTime value
ctoml_datetime_t ctoml_node_get_datetime(ctoml_node_t node);

// ============================================================================
// Array functions
// ============================================================================

/// Get the number of elements in an array.
/// @param node Array node handle
/// @return Number of elements
size_t ctoml_node_array_count(ctoml_node_t node);

/// Get an element from an array by index.
/// @param node Array node handle
/// @param index Index of element
/// @return Node handle for the element (valid until document is freed)
ctoml_node_t ctoml_node_array_get(ctoml_node_t node, size_t index);

// ============================================================================
// Table functions
// ============================================================================

/// Get the number of key-value pairs in a table.
/// @param node Table node handle
/// @return Number of key-value pairs
size_t ctoml_node_table_count(ctoml_node_t node);

/// Get the key at a specific index in a table.
/// @param node Table node handle
/// @param index Index of key-value pair
/// @param length Out parameter for key length (can be NULL)
/// @return Pointer to key string (valid until document is freed)
const char* ctoml_node_table_key_at(ctoml_node_t node, size_t index, size_t* length);

/// Get the value at a specific index in a table.
/// @param node Table node handle
/// @param index Index of key-value pair
/// @return Node handle for the value (valid until document is freed)
ctoml_node_t ctoml_node_table_value_at(ctoml_node_t node, size_t index);

/// Get a value from a table by key.
/// @param node Table node handle
/// @param key Key to look up
/// @return Node handle for the value, or NULL if not found
ctoml_node_t ctoml_node_table_get(ctoml_node_t node, const char* key);

#ifdef __cplusplus
}
#endif

#endif // CTOML_H
