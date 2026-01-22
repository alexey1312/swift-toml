#define TOML_HEADER_ONLY 1
// Disable assertions to handle invalid input gracefully
#define NDEBUG 1

#include "include/ctoml.h"
#include "toml.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstring>

// ============================================================================
// Internal structures
// ============================================================================

// Forward declaration
struct ctoml_node;

// Internal node representation that wraps toml++ nodes
struct ctoml_node {
    ctoml_type_t type;

    // Scalar values
    std::string string_value;
    int64_t int_value;
    double float_value;
    bool bool_value;
    ctoml_date_t date_value;
    ctoml_time_t time_value;
    ctoml_datetime_t datetime_value;

    // Container values
    std::vector<std::unique_ptr<ctoml_node>> array_elements;
    std::vector<std::string> table_keys;
    std::vector<std::unique_ptr<ctoml_node>> table_values;

    ctoml_node() : type(CTOML_TYPE_NONE), int_value(0), float_value(0), bool_value(false) {
        date_value = {0, 0, 0};
        time_value = {0, 0, 0, 0};
        datetime_value = {{0, 0, 0}, {0, 0, 0, 0}, false, 0};
    }
};

// Document structure
struct ctoml_document {
    std::unique_ptr<ctoml_node> root;
    std::string error_message;
    int64_t error_line;
    int64_t error_column;
    bool is_valid;

    ctoml_document() : root(nullptr), error_line(0), error_column(0), is_valid(false) {}
};

// ============================================================================
// Conversion from toml++ to internal representation
// ============================================================================

static std::unique_ptr<ctoml_node> convert_node(const toml::node& node);

static std::unique_ptr<ctoml_node> convert_table(const toml::table& table) {
    auto node = std::make_unique<ctoml_node>();
    node->type = CTOML_TYPE_TABLE;

    for (auto& [k, v] : table) {
        node->table_keys.push_back(std::string(k));
        node->table_values.push_back(convert_node(v));
    }

    return node;
}

static std::unique_ptr<ctoml_node> convert_array(const toml::array& arr) {
    auto node = std::make_unique<ctoml_node>();
    node->type = CTOML_TYPE_ARRAY;

    for (size_t i = 0; i < arr.size(); ++i) {
        if (auto* elem = arr.get(i)) {
            node->array_elements.push_back(convert_node(*elem));
        }
    }

    return node;
}

static std::unique_ptr<ctoml_node> convert_node(const toml::node& node) {
    auto result = std::make_unique<ctoml_node>();

    if (node.is_string()) {
        result->type = CTOML_TYPE_STRING;
        result->string_value = std::string(node.as_string()->get());
    }
    else if (node.is_integer()) {
        result->type = CTOML_TYPE_INTEGER;
        result->int_value = node.as_integer()->get();
    }
    else if (node.is_floating_point()) {
        result->type = CTOML_TYPE_FLOAT;
        result->float_value = node.as_floating_point()->get();
    }
    else if (node.is_boolean()) {
        result->type = CTOML_TYPE_BOOLEAN;
        result->bool_value = node.as_boolean()->get();
    }
    else if (node.is_date()) {
        result->type = CTOML_TYPE_DATE;
        auto d = node.as_date()->get();
        result->date_value.year = d.year;
        result->date_value.month = static_cast<int32_t>(d.month);
        result->date_value.day = static_cast<int32_t>(d.day);
    }
    else if (node.is_time()) {
        result->type = CTOML_TYPE_TIME;
        auto t = node.as_time()->get();
        result->time_value.hour = static_cast<int32_t>(t.hour);
        result->time_value.minute = static_cast<int32_t>(t.minute);
        result->time_value.second = static_cast<int32_t>(t.second);
        result->time_value.nanosecond = static_cast<int32_t>(t.nanosecond);
    }
    else if (node.is_date_time()) {
        result->type = CTOML_TYPE_DATETIME;
        auto dt = node.as_date_time()->get();
        result->datetime_value.date.year = dt.date.year;
        result->datetime_value.date.month = static_cast<int32_t>(dt.date.month);
        result->datetime_value.date.day = static_cast<int32_t>(dt.date.day);
        result->datetime_value.time.hour = static_cast<int32_t>(dt.time.hour);
        result->datetime_value.time.minute = static_cast<int32_t>(dt.time.minute);
        result->datetime_value.time.second = static_cast<int32_t>(dt.time.second);
        result->datetime_value.time.nanosecond = static_cast<int32_t>(dt.time.nanosecond);
        result->datetime_value.has_offset = dt.offset.has_value();
        result->datetime_value.offset_minutes = result->datetime_value.has_offset ? dt.offset->minutes : 0;
    }
    else if (node.is_array()) {
        return convert_array(*node.as_array());
    }
    else if (node.is_table()) {
        return convert_table(*node.as_table());
    }

    return result;
}

// ============================================================================
// C API Implementation
// ============================================================================

extern "C" {

ctoml_document_t ctoml_parse(const char* input, size_t length) {
    auto* doc = new ctoml_document();

    try {
        std::string_view sv(input, length);
        auto table = toml::parse(sv);
        doc->root = convert_table(table);
        doc->is_valid = true;
    } catch (const toml::parse_error& err) {
        doc->is_valid = false;
        doc->error_message = std::string(err.description());
        doc->error_line = err.source().begin.line;
        doc->error_column = err.source().begin.column;
    }

    return doc;
}

bool ctoml_document_is_valid(ctoml_document_t doc) {
    if (!doc) return false;
    return doc->is_valid;
}

ctoml_error_t ctoml_document_get_error(ctoml_document_t doc) {
    ctoml_error_t error = {0, 0, nullptr};
    if (!doc) return error;

    error.line = doc->error_line;
    error.column = doc->error_column;
    error.message = doc->error_message.c_str();
    return error;
}

ctoml_node_t ctoml_document_get_root(ctoml_document_t doc) {
    if (!doc || !doc->root) return nullptr;
    return doc->root.get();
}

void ctoml_document_free(ctoml_document_t doc) {
    delete doc;
}

ctoml_type_t ctoml_node_get_type(ctoml_node_t node) {
    if (!node) return CTOML_TYPE_NONE;
    return node->type;
}

const char* ctoml_node_get_string(ctoml_node_t node, size_t* length) {
    if (!node || node->type != CTOML_TYPE_STRING) {
        if (length) *length = 0;
        return "";
    }
    if (length) *length = node->string_value.size();
    return node->string_value.c_str();
}

int64_t ctoml_node_get_integer(ctoml_node_t node) {
    if (!node || node->type != CTOML_TYPE_INTEGER) return 0;
    return node->int_value;
}

double ctoml_node_get_float(ctoml_node_t node) {
    if (!node || node->type != CTOML_TYPE_FLOAT) return 0.0;
    return node->float_value;
}

bool ctoml_node_get_boolean(ctoml_node_t node) {
    if (!node || node->type != CTOML_TYPE_BOOLEAN) return false;
    return node->bool_value;
}

ctoml_date_t ctoml_node_get_date(ctoml_node_t node) {
    ctoml_date_t date = {0, 0, 0};
    if (!node || node->type != CTOML_TYPE_DATE) return date;
    return node->date_value;
}

ctoml_time_t ctoml_node_get_time(ctoml_node_t node) {
    ctoml_time_t time = {0, 0, 0, 0};
    if (!node || node->type != CTOML_TYPE_TIME) return time;
    return node->time_value;
}

ctoml_datetime_t ctoml_node_get_datetime(ctoml_node_t node) {
    ctoml_datetime_t datetime = {{0, 0, 0}, {0, 0, 0, 0}, false, 0};
    if (!node || node->type != CTOML_TYPE_DATETIME) return datetime;
    return node->datetime_value;
}

size_t ctoml_node_array_count(ctoml_node_t node) {
    if (!node || node->type != CTOML_TYPE_ARRAY) return 0;
    return node->array_elements.size();
}

ctoml_node_t ctoml_node_array_get(ctoml_node_t node, size_t index) {
    if (!node || node->type != CTOML_TYPE_ARRAY) return nullptr;
    if (index >= node->array_elements.size()) return nullptr;
    return node->array_elements[index].get();
}

size_t ctoml_node_table_count(ctoml_node_t node) {
    if (!node || node->type != CTOML_TYPE_TABLE) return 0;
    return node->table_keys.size();
}

const char* ctoml_node_table_key_at(ctoml_node_t node, size_t index, size_t* length) {
    if (!node || node->type != CTOML_TYPE_TABLE || index >= node->table_keys.size()) {
        if (length) *length = 0;
        return "";
    }
    if (length) *length = node->table_keys[index].size();
    return node->table_keys[index].c_str();
}

ctoml_node_t ctoml_node_table_value_at(ctoml_node_t node, size_t index) {
    if (!node || node->type != CTOML_TYPE_TABLE) return nullptr;
    if (index >= node->table_values.size()) return nullptr;
    return node->table_values[index].get();
}

ctoml_node_t ctoml_node_table_get(ctoml_node_t node, const char* key) {
    if (!node || node->type != CTOML_TYPE_TABLE || !key) return nullptr;

    for (size_t i = 0; i < node->table_keys.size(); ++i) {
        if (node->table_keys[i] == key) {
            return node->table_values[i].get();
        }
    }
    return nullptr;
}

} // extern "C"
