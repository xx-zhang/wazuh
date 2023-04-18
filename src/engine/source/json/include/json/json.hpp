#ifndef _JSON_H
#define _JSON_H

// #define JSON_USE_RAPIDJSON
#define JSON_USE_NLOHMANN

#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef JSON_USE_RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#endif

#ifdef JSON_USE_NLOHMANN
#include <nlohmann/json.hpp>
#endif

#include <error.hpp>

namespace json
{

constexpr bool RECURSIVE {true};
constexpr bool NOT_RECURSIVE {false};

#ifdef JSON_USE_RAPIDJSON

class Json
{
public:
    enum class Type
    {
        Null,
        Object,
        Array,
        String,
        Number,
        Boolean
    };

private:
    rapidjson::Document m_document;

    /**
     * @brief Construct a new Json object form a rapidjason::Value.
     * Copies the value.
     *
     * @param value The rapidjson::Value to copy.
     */
    Json(const rapidjson::Value& value);

    /**
     * @brief Construct a new Json object form a rapidjason::GenericObject.
     * Copies the object.
     *
     * @param object The rapidjson::GenericObject to copy.
     */
    Json(const rapidjson::GenericObject<true, rapidjson::Value>& object);

    /**
     * @brief Get Json type from internal rapidjason type.
     *
     * @param t rapidjson::Type to convert.
     * @return constexpr Type The converted type.
     *
     * @throw std::runtime_error if the type is not supported.
     */
    constexpr static Type rapidTypeToJsonType(rapidjson::Type t)
    {
        switch (t)
        {
            case rapidjson::kNullType: return Type::Null;
            case rapidjson::kObjectType: return Type::Object;
            case rapidjson::kArrayType: return Type::Array;
            case rapidjson::kStringType: return Type::String;
            case rapidjson::kNumberType: return Type::Number;
            case rapidjson::kFalseType:
            case rapidjson::kTrueType: return Type::Boolean;
            default: throw std::runtime_error("Unknown rapidjson::Type");
        }
    }

    void merge(const bool isRecursive, rapidjson::Value& source, std::string_view path);

public:
    /**
     * @brief Construct a new Json empty json object.
     *
     */
    Json();

    /**
     * @brief Construct a new Json object from a rapidjason Document.
     * Moves the document.
     *
     * @param document The rapidjson::Document to move.
     */
    explicit Json(rapidjson::Document&& document);

    /**
     * @brief Construct a new Json object from a json string
     *
     * @param json The json string to parse.
     */
    explicit Json(const char* json);

    /**
     * @brief Copy constructs a new Json object.
     * Value is copied.
     *
     * @param other The Json to copy.
     */
    Json(const Json& other);

    /**
     * @brief Copy assignment operator.
     * Value is copied.
     *
     * @param other The Json to copy.
     * @return Json& The new Json object.
     */
    Json& operator=(const Json& other);

    bool operator==(const Json& other) const;

    /************************************************************************************/
    // Static Helpers
    /************************************************************************************/

    /**
     * @brief Transform dot path string to pointer path string.
     *
     * @param dotPath The dot path string.
     * @return std::string The pointer path string.
     */
    static std::string formatJsonPath(std::string_view dotPath, bool skipDot = false);

    /************************************************************************************/
    // Runtime functionality, used only by our operations.
    // TODO: Move runtime functionality to separate class.
    /************************************************************************************/

    /**
     * @brief Move copy constructor.
     * Value is moved.
     *
     * @param other The Json to move, which is left in an empty state.
     */
    Json(Json&& other) noexcept;

    /**
     * @brief Move copy assignment operator.
     * Value is moved.
     *
     * @param other The Json to move, which is left in an empty state.
     * @return Json& The new Json object.
     */
    Json& operator=(Json&& other) noexcept;

    /**
     * @brief Check if the Json contains a field with the given pointer path.
     *
     * @param pointerPath The pointer path to check.
     * @return true The Json contains the field.
     * @return false The Json does not contain the field.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool exists(std::string_view pointerPath) const;

    /**
     * @brief Check if the Json contains a field with the given dot path, and if so, with
     * the given value.
     *
     * @param pointerPath
     * @param value
     * @return true The Json contains the field with the given value.
     * @return false The Json does not contain the field with the given value.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool equals(std::string_view pointerPath, const Json& value) const;

    /**
     * @brief Check if basePointerPath field's value is equal to referencePointerPath
     * field's value. If basePointerPath or referencePointerPath is not found, returns
     * false.
     *
     * @param basePointerPath The base pointer path to check.
     * @param referencePointerPath The reference pointer path to check.
     * @return true The base field's value is equal to the reference field's value.
     * @return false The base field's value is not equal to the reference field's value.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    bool equals(std::string_view basePointerPath,
                std::string_view referencePointerPath) const;

    /**
     * @brief Set the value of the field with the given pointer path.
     * Overwrites previous value.
     *
     * @param pointerPath The pointer path to set.
     * @param value The value to set.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    void set(std::string_view pointerPath, const Json& value);

    /**
     * @brief Set the value of the base field with the value of the reference field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     * @param referencePointerPath The reference pointer path.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    void set(std::string_view basePointerPath, std::string_view referencePointerPath);

    /************************************************************************************/
    // Getters
    /************************************************************************************/

    /**
     * @brief get the value of the string field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::string> getString(std::string_view path = "") const;

    /**
     * @brief get the value of the int field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<int> getInt(std::string_view path = "") const;

    /**
     * @brief get the value of the int64 field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<int64_t> getInt64(std::string_view path = "") const;

    /**
     * @brief get the value of the float field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<float_t> getFloat(std::string_view path = "") const;

    /**
     * @brief get the value of the double field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<double_t> getDouble(std::string_view path = "") const;

    /**
     * @brief get the value of either a double or int field as a double.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     *
     * @todo Develop tests for this method
     */
    std::optional<double> getNumberAsDouble(std::string_view path = "") const;

    /**
     * @brief get the value of the bool field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<bool> getBool(std::string_view path = "") const;

    /**
     * @brief get the value of the array field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::vector<Json>> getArray(std::string_view path = "") const;

    /**
     * @brief get the value of the object field.
     *
     * @param path The base pointer path to get.
     *
     * @return T The value of the field.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    std::optional<std::vector<std::tuple<std::string, Json>>> getObject(std::string_view path = "") const;

    /**
     * @brief Get Json prettyfied string.
     *
     * @return std::string The Json prettyfied string.
     */
    std::string prettyStr() const;

    /**
     * @brief Get Json string.
     *
     * @return std::string The Json string.
     */
    std::string str() const;

    /**
     * @brief Get Json string from an object.
     *
     * @param path The path to the object.
     * @return std::string The Json string or nothing if the path not found.
     * @throws std::runtime_error If the path is invalid.
     */
    std::optional<std::string> str(std::string_view path) const;

    /**
     * @brief Get a copy of the Json object or nothing if the path not found.c++ diagram
     *
     * @param path The path to the object, default value is root object ("").
     * @return std::optional<Json> The Json object if it exists, std::nullopt otherwise.
     * @throw std::runtime_error If path is invalid.
     */
    std::optional<Json> getJson(std::string_view path = "") const;

    friend std::ostream& operator<<(std::ostream& os, const Json& json);

    /************************************************************************************/
    // Query
    /************************************************************************************/

    /**
     * @brief Get number of elements.
     * If array get number of elements. If object get number of pairs (key, value).
     *
     * @return size_t The number of elements.
     *
     * @throws std::runtime_error If the Json is not an array or object.
     */
    size_t size(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Null.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Null.
     * @return false if Json is not Null.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isNull(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Bool.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Bool.
     * @return false if Json is not Bool.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isBool(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Number.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Number.
     * @return false if Json is not Number.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isNumber(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is integer.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Int.
     * @return false if Json is not Int.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isInt(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is int64.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is int64.
     * @return false if Json is not int64.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isInt64(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is float.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is float.
     * @return false if Json is not float.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isFloat(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is double.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Double.
     * @return false if Json is not Double.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isDouble(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is String.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is String.
     * @return false if Json is not String.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isString(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Array.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Array.
     * @return false if Json is not Array.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isArray(std::string_view path = "") const;

    /**
     * @brief Check if the Json described by the path is Object.
     *
     * Ensure that the path exists before calling this function.
     *
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if Json is Object.
     * @return false if Json is not Object.
     *
     * @throws std::runtime_error If path is invalid or cannot be found.
     */
    bool isObject(std::string_view path = "") const;

    /**
     * @brief Get the type name of the Json.
     *
     * @return std::string The type name of the Json.
     */
    std::string typeName(std::string_view path = "") const;

    /**
     * @brief Get Type of the Json.
     *
     * @param path The path to the object, default value is root object ("").
     * @return Type The type of the Json.
     *
     * @throws std::runtime_error If:
     * - path is invalid or cannot be found.
     * - internal json type is not supported.
     */
    Type type(std::string_view path = "") const;

    /**
     * @brief Validate the Json agains the schema.
     *
     * @param schema The schema to validate against.
     * @return std::optional<base::Error> Error message if validation failed, std::nullopt
     * otherwise.
     */
    std::optional<base::Error> validate(const Json& schema) const;

    /**
     * @brief Check if the Json has duplicate keys.
     *
     * @return std::optional<base::Error> If the Json has duplicate keys, return the error
     */
    std::optional<base::Error> checkDuplicateKeys() const;

    /************************************************************************************/
    // Setters
    /************************************************************************************/

    /**
     * @brief Set the Null object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setNull(std::string_view path = "");

    /**
     * @brief Set the Boolean object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setBool(bool value, std::string_view path = "");

    /**
     * @brief Set the Integer object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setInt(int value, std::string_view path = "");

    /**
     * @brief Set the Integer object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setInt64(int64_t value, std::string_view path = "");

    /**
     * @brief Set the Double object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setDouble(double_t value, std::string_view path = "");

    /**
     * @brief Set the Double object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setFloat(float_t value, std::string_view path = "");

    /**
     * @brief Set the String object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setString(std::string_view value, std::string_view path = "");

    /**
     * @brief Set the Array object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setArray(std::string_view path = "");

    /**
     * @brief Set the Object object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setObject(std::string_view path = "");

    /**
     * @brief Append string to the Array object at the path.
     * Parents objects are created if they do not exist.
     * If the object is not an Array, it is converted to an Array.
     *
     * @param value The string to append.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void appendString(std::string_view value, std::string_view path = "");

    /**
     * @brief Append Json to the Array object at the path.
     *
     * @param value The Json to append.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void appendJson(const Json& value, std::string_view path = "");

    /**
     * @brief Erase Json object at the path.
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if object was erased, false if object was not found.
     *
     * @throws std::runtime_error If path is invalid.
     */
    bool erase(std::string_view path = "");

    /**
     * @brief Merge the Json Value at the path with the given Json Value.
     *
     * Objects are merged, arrays are appended.
     * Merges only first level of the Json Value.
     *
     * @param other The Json Value to merge.
     * @param path  The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error On the following conditions:
     * - If path is invalid.
     * - If either Json Values are not Object or Array.
     * - If Json Values are not the same type.
     */
    void merge(const bool isRecursive, Json& other, std::string_view path = "");

    /**
     * @brief Merge the Json Value at the path with the given Json Value at reference
     * path.
     *
     * Merges only first level of the Json Value.
     * Reference value is deleted after merge.
     *
     * @param other The Json path pointing to the value to be merged.
     * @param path  The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error On the following conditions:
     * - If either path are invalid.
     * - If either Json Values are not Object or Array.
     * - If Json Values are not the same type.
     */
    void merge(const bool isRecursive, std::string_view other, std::string_view path = "");

};

#endif
#ifdef JSON_USE_NLOHMANN
class Json
{
public:
    enum class Type
    {
        Null,
        Object,
        Array,
        String,
        Number,
        Boolean
    };

private:
    nlohmann::ordered_json m_document;

    /**
     * @brief Construct a new Json object form a rapidjason::GenericObject.
     * Copies the object.
     *
     * @param object The rapidjson::GenericObject to copy.
     */
    // Json(const rapidjson::GenericObject<true, rapidjson::Value>& object);
    Json(const std::string& key, const std::string& value)
        : m_document {{key, value}} {};

    /**
     * @brief Get Json type from internal rapidjason type.
     *
     * @param t rapidjson::Type to convert.
     * @return constexpr Type The converted type.
     *
     * @throw std::runtime_error if the type is not supported.
     */
    constexpr static Type nlohmannToJsonType(nlohmann::ordered_json::value_t t)
    {
        switch (t)
        {
            case nlohmann::ordered_json::value_t::null: return Type::Null;
            case nlohmann::ordered_json::value_t::object: return Type::Object;
            case nlohmann::ordered_json::value_t::array: return Type::Array;
            case nlohmann::ordered_json::value_t::string: return Type::String;
            case nlohmann::ordered_json::value_t::number_integer:
            case nlohmann::ordered_json::value_t::number_unsigned:
            case nlohmann::ordered_json::value_t::number_float: return Type::Number;
            case nlohmann::ordered_json::value_t::boolean: return Type::Boolean;
            default: throw std::runtime_error("Unknown nlohmann::ordered_json::value_t");
        }
    }

    bool is_index(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    // void merge(const bool isRecursive, rapidjson::Value& source, std::string_view path);

public:
    /**
     * @brief Construct a new Json empty json object.
     *
     */
    Json() = default;

    /**
     * @brief Copy constructs a new Json object.
     * Value is copied.
     *
     * @param other The Json to copy.
     */
    explicit Json(nlohmann::ordered_json& document) : m_document(document) {};

    /**
     * @brief Construct a new Json object from a nlohmann::ordered_json Document.
     * Moves the document.
     *
     * @param document The rapidjson::Document to move.
     */
    explicit Json(nlohmann::ordered_json&& document)
        : m_document(std::move(document)) {};

    explicit Json(const nlohmann::ordered_json::value_type& val)
        : m_document(val) {};

    /**
     * @brief Construct a new Json object from a json string
     *
     * @param json The json string to parse.
     */
    explicit Json(const char* cstr)
        : m_document(nlohmann::ordered_json::parse(cstr)) {};
    explicit Json(const std::string str)
        : m_document(nlohmann::ordered_json::parse(str.data())) {};
    explicit Json(const std::string_view str)
        : m_document(nlohmann::ordered_json::parse(str.data())) {};

    Json(const Json& other)
        : m_document(other.m_document) {};

    /**
     * @brief Copy assignment operator.
     * Value is copied.
     *
     * @param rvalue The Json to copy.
     * @return Json& The new Json object.
     */
    Json& operator=(const Json& other)
    {
        m_document = other.m_document;
        return *this;
    }

    bool operator==(const Json& other) const { return m_document == other.m_document; }

    /************************************************************************************/
    // Static Helpers
    /************************************************************************************/

    /**
     * @brief Transform dot path string to pointer path string.
     *
     * @param dotPath The dot path string.
     * @return std::string The pointer path string.
     */
    static std::string formatJsonPath(std::string_view dotPath, bool skipDot = false)
    {
        // TODO: Handle array indices and pointer path operators.
        std::string ptrPath {dotPath};

        // Some helpers may indiate that the field is root element
        // In this case the path will be defined as "."
        if ("." == ptrPath)
        {
            ptrPath = "";
        }
        else
        {
            // Replace ~ with ~0
            for (auto pos = ptrPath.find('~'); pos != std::string::npos; pos = ptrPath.find('~', pos + 2))
            {
                ptrPath.replace(pos, 1, "~0");
            }

            // Replace / with ~1
            for (auto pos = ptrPath.find('/'); pos != std::string::npos; pos = ptrPath.find('/', pos + 2))
            {
                ptrPath.replace(pos, 1, "~1");
            }

            // Replace . with /
            if (!skipDot)
            {
                std::replace(std::begin(ptrPath), std::end(ptrPath), '.', '/');
            }

            // Add / at the beginning
            if (ptrPath.front() != '/')
            {
                ptrPath.insert(0, "/");
            }
        }

        return ptrPath;
    }

    /************************************************************************************/
    // Runtime functionality, used only by our operations.
    // TODO: Move runtime functionality to separate class.
    /************************************************************************************/

    Json(Json&& other) noexcept
        : m_document(std::move(other.m_document))
    {
    }

    Json& operator=(Json&& other) noexcept
    {
        m_document = std::move(other.m_document);
        return *this;
    }

    /**
     * @brief Check if the Json contains a field with the given pointer path.
     *
     * @param pointerPath The pointer path to check.
     * @return true The Json contains the field.
     * @return false The Json does not contain the field.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool exists(std::string_view pointerPath) const
    {
        bool exists {false};
        try
        {
            exists = m_document.contains(nlohmann::ordered_json::json_pointer(pointerPath.data()));
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(e.what());
        }

        return exists;
    };

    /**
     * @brief Check if the Json contains a field with the given dot path, and if so, with
     * the given value.
     *
     * @param pointerPath
     * @param value
     * @return true The Json contains the field with the given value.
     * @return false The Json does not contain the field with the given value.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    bool equals(std::string_view pointerPath, const Json& value) const
    {
        nlohmann::ordered_json::json_pointer ptr;

        ptr = nlohmann::ordered_json::json_pointer(pointerPath.data());

        // check if the pointer path exists in the JSON object
        if (!m_document.contains(ptr))
        {
            return false;
        }

        // get the value at the pointer path
        try
        {
            const nlohmann::ordered_json& fieldValue = m_document[ptr];

            if (fieldValue.type() != value.m_document.type())
            {
                return false;
            }

            return fieldValue == value.m_document;
        }
        catch (const std::out_of_range& ex)
        {
            return false;
        }
    }

    /**
     * @brief Check if firstPointerPath field's value is equal to secondPointerPath
     * field's value. If firstPointerPath or secondPointerPath is not found, returns
     * false.
     *
     * @param firstPointerPath The base pointer path to check.
     * @param secondPointerPath The reference pointer path to check.
     * @return true The base field's value is equal to the reference field's value.
     * @return false The base field's value is not equal to the reference field's value.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    bool equals(std::string_view firstPointerPath, std::string_view secondPointerPath) const
    {
        nlohmann::ordered_json::json_pointer ptrFirst;
        ptrFirst = nlohmann::ordered_json::json_pointer(firstPointerPath.data());

        nlohmann::ordered_json::json_pointer ptrSecond;
        ptrSecond = nlohmann::ordered_json::json_pointer(secondPointerPath.data());

        // check if the pointer path exists in the JSON object
        if (!m_document.contains(ptrFirst) || !m_document.contains(ptrSecond))
        {
            return false;
        }

        // get the value at the pointer path
        try
        {
            const nlohmann::ordered_json& firstFieldValue = m_document[ptrFirst];
            const nlohmann::ordered_json& secondFieldValue = m_document[ptrSecond];

            if (firstFieldValue.type() != secondFieldValue.type())
            {
                return false;
            }

            return firstFieldValue == secondFieldValue;
        }
        catch (const std::out_of_range& ex)
        {
            return false;
        }
    }

    /**
     * @brief Set the value of the field with the given pointer path.
     * Overwrites previous value.
     *
     * @param pointerPath The pointer path to set.
     * @param value The value to set.
     *
     * @throws std::runtime_error If the pointer path is invalid.
     */
    void set(std::string_view pointerPath, const Json& value)
    {
        nlohmann::ordered_json::json_pointer ptr;
        ptr = nlohmann::ordered_json::json_pointer(pointerPath.data());

        // set the value at the pointer path
        try
        {
            m_document[ptr] = value.m_document;
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error("Failed to set value: " + std::string(ex.what()));
        }
    }

    /**
     * @brief Set the value of the base field with the value of the reference field.
     * Overwrites previous value. If reference field is not found, sets base field to
     * null.
     *
     * @param basePointerPath The base pointer path to set.
     * @param referencePointerPath The reference pointer path.
     *
     * @throws std::runtime_error If any pointer path is invalid.
     */
    void set(std::string_view basePointerPath, std::string_view referencePointerPath)
    {
        nlohmann::ordered_json::json_pointer ptrBase;
        ptrBase = nlohmann::ordered_json::json_pointer(basePointerPath.data());

        nlohmann::ordered_json::json_pointer ptrReference;
        ptrReference = nlohmann::ordered_json::json_pointer(referencePointerPath.data());

        if (!m_document.contains(ptrReference))
        {
            m_document[ptrBase] = nlohmann::ordered_json();
        }

        // set the value at the pointer path
        try
        {
            m_document[ptrBase] = m_document[ptrReference];
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error("Failed to set value: " + std::string(ex.what()));
        }
    }

    /************************************************************************************/
    // Getters
    /************************************************************************************/

    std::optional<std::string> getString(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_string())
            {
                return std::nullopt;
            }
            return value.get<std::string>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<int> getInt(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_number_integer())
            {
                return std::nullopt;
            }
            return value.get<int>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<int64_t> getInt64(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_number_integer())
            {
                return std::nullopt;
            }
            return value.get<int64_t>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<float_t> getFloat(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_number_float())
            {
                return std::nullopt;
            }
            return value.get<float_t>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<double_t> getDouble(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_number_float())
            {
                return std::nullopt;
            }
            return value.get<double_t>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<double> getNumberAsDouble(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_number())
            {
                return std::nullopt;
            }
            return value.get<double>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<bool> getBool(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        try
        {
            auto& value = m_document[ptr];

            if (value.is_null())
            {
                return std::nullopt;
            }
            if (!value.is_boolean())
            {
                return std::nullopt;
            }
            return value.get<bool>();
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<std::vector<Json>> getArray(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        const auto& field = m_document[ptr];

        if (field.is_array())
        {
            std::vector<Json> result {};

            for (const nlohmann::ordered_json element : field)
            {
                result.push_back(Json(element));
            }

            return result;
        }

        return std::nullopt;
    }

    std::optional<std::vector<std::tuple<std::string, Json>>> getObject(std::string_view path = "") const
    {

        std::optional<std::vector<std::tuple<std::string, Json>>> retval {std::nullopt};
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (m_document.contains(ptr))
        {
            auto field = m_document[ptr];

            if (field.is_object())
            {
                std::vector<std::tuple<std::string, Json>> result;
                for (auto it = field.begin(); it != field.end(); ++it)
                {
                    result.emplace_back(std::make_tuple(it.key(), it.value()));
                }
                retval = result;
            }

            return retval;
        }

        return std::nullopt;
    }

    /**
     * @brief Get Json prettyfied string.
     *
     * @return std::string The Json prettyfied string.
     */
    std::string prettyStr() const { return m_document.dump(4); }

    /**
     * @brief Get Json string.
     *
     * @return std::string The Json string.
     */
    std::string str() const { return m_document.dump(); }

    /**
     * @brief Get Json string from an object.
     *
     * @param path The path to the object.
     * @return std::string The Json string or nothing if the path not found.
     * @throws std::runtime_error If the path is invalid.
     */
    std::optional<std::string> str(std::string_view path) const
    {
        nlohmann::ordered_json::json_pointer ptr;
        ptr = nlohmann::ordered_json::json_pointer(path.data());

        if (!m_document.contains(ptr))
        {
            return std::nullopt;
        }

        return m_document[ptr].dump();
    }

    /**
     * @brief Get a copy of the Json object or nothing if the path not found.c++ diagram
     *
     * @param path The path to the object, default value is root object ("").
     * @return std::optional<Json> The Json object if it exists, std::nullopt otherwise.
     * @throw std::runtime_error If path is invalid.
     */
    std::optional<Json> getJson(std::string_view path = "") const
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (m_document.contains(ptr))
        {
            return Json(m_document.at(ptr));
        }

        return std::nullopt;
    }

    friend std::ostream& operator<<(std::ostream& os, const Json& json)
    {
        os << json.str();
        return os;
    }

    /************************************************************************************/
    // Query
    /************************************************************************************/

    /**
     * @brief Get number of elements.
     * If array get number of elements. If object get number of pairs (key, value).
     *
     * @return size_t The number of elements.
     *
     * @throws std::runtime_error If the Json is not an array or object.
     */
    size_t size(std::string_view path = "") const
    {
        if (path.empty() && (m_document.is_array() || m_document.is_object()))
        {
            return m_document.size();
        }
        else
        {
            nlohmann::ordered_json::json_pointer ptr {path.data()};

            if (m_document.contains(ptr) && (m_document[ptr].is_array() || m_document[ptr].is_object()))
            {
                return m_document[ptr].size();
            }
        }

        throw std::runtime_error("Json element is not an array or object.");
    }

    /**
     * @brief Check if the value is null.
     *
     * @return true if the value is null, false otherwise.
     */
    bool isNull(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_null());
    }

    /**
     * @brief Check if the value is a boolean.
     *
     * @return true if the value is a boolean, false otherwise.
     */
    bool isBool(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_boolean());
    }

    /**
     * @brief Check if the value is a number.
     *
     * @return true if the value is a number, false otherwise.
     */
    bool isNumber(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_number());
    }

    /**
     * @brief Check if the value is an integer.
     *
     * @return true if the value is an integer, false otherwise.
     */
    bool isInt(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_number_integer());
    }

    /**
     * @brief Check if the value is a 64-bit integer.
     *
     * @return true if the value is a 64-bit integer, false otherwise.
     */
    bool isInt64(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_number_integer());
    }

    /**
     * @brief Check if the value is a floating-point number.
     *
     * @return true if the value is a floating-point number, false otherwise.
     */
    bool isFloat(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_number_float());
    }

    /**
     * @brief Check if the value is a double-precision floating-point number.
     *
     * @return true if the value is a double-precision floating-point number, false otherwise.
     */
    bool isDouble(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_number_float());
    }

    /**
     * @brief Check if the value is a string.
     *
     * @return true if the value is a string, false otherwise.
     */
    bool isString(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_string());
    }

    /**
     * @brief Check if the value is an array.
     *
     * @return true if the value is an array, false otherwise.
     */
    bool isArray(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_array());
    }

    /**
     * @brief Check if Json is an object.
     *
     * @param path The path to the JSON element to check.
     *             If empty, checks the entire JSON.
     *
     * @return true If Json is an object.
     * @return false Otherwise.
     */
    bool isObject(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};
        return (m_document.contains(ptr) && m_document[ptr].is_object());
    }

    /**
     * @brief Get the type name of the Json.
     *
     * @return std::string The type name of the Json.
     */
    std::string typeName(std::string_view path = "") const
    {
        std::string type {""};

        const nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (path.empty())
        {
            type = m_document.type_name();
        }
        else
        {
            if (!m_document.contains(ptr))
            {
                throw std::runtime_error("Path does not exist on JSON");
            }

            type = m_document[ptr].type_name();
        }

        return type;
    }

    /**
     * @brief Get Type of the Json.
     *
     * @param path The path to the object, default value is root object ("").
     * @return Type The type of the Json.
     *
     * @throws std::runtime_error If:
     * - path is invalid or cannot be found.
     * - internal json type is not supported.
     */
    Type type(std::string_view path = "") const
    {
        const nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (m_document.contains(ptr))
        {
            return nlohmannToJsonType(m_document[ptr].type());
        }

        throw std::runtime_error("Json type could not be found.");
    }

    /**
     * @brief Validate the Json agains the schema.
     *
     * @param schema The schema to validate against.
     * @return std::optional<base::Error> Error message if validation failed, std::nullopt
     * otherwise.
     */
    // std::optional<base::Error> validate(const Json& schema) const;

    /**
     * @brief Check if the Json has duplicate keys.
     *
     * @return std::optional<base::Error> If the Json has duplicate keys, return the error
     */
    std::optional<base::Error> checkDuplicateKeys() const
    {
        if (!m_document.is_object())
        {
            return std::nullopt;
        }

        std::set<std::string> keys;

        for (const auto& [key, value] : m_document.items())
        {
            if (keys.find(key) != keys.end())
            {
                return base::Error {"Json object contains duplicate key: " + key};
            }
            keys.insert(key);
        }

        return std::nullopt;
    }

    /************************************************************************************/
    // Setters
    /************************************************************************************/

    /**
     * @brief Set the Null object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setNull(std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = nullptr;
    }

    /**
     * @brief Set the Boolean object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setBool(bool value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value;
    }

    /**
     * @brief Set the Integer object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setInt(int value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value;
    }

    /**
     * @brief Set the Integer object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setInt64(int64_t value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value;
    }

    /**
     * @brief Set the Double object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setDouble(double_t value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value;
    }

    /**
     * @brief Set the Double object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setFloat(float_t value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value;
    }

    /**
     * @brief Set the String object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param value The value to set.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setString(std::string_view value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = value.data();
    }

    /**
     * @brief Set the Array object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setArray(std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = nlohmann::ordered_json::array();
    }

    /**
     * @brief Set the Object object at the path.
     * Parents objects are created if they do not exist.
     *
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void setObject(std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!ptr.parent_pointer().empty() && m_document[ptr.parent_pointer()].is_array() && !is_index(ptr.back()))
        {
            m_document[ptr.parent_pointer()] = nlohmann::ordered_json::object();
        }

        m_document[ptr] = nlohmann::ordered_json::object();
    }

    /**
     * @brief Append string to the Array object at the path.
     * Parents objects are created if they do not exist.
     * If the object is not an Array, it is converted to an Array.
     *
     * @param value The string to append.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void appendString(std::string_view value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!exists(path) || !isArray(path))
        {
            m_document[ptr] = nlohmann::ordered_json::array();
        }

        m_document[ptr].emplace_back(value);
    }

    /**
     * @brief Append Json to the Array object at the path.
     *
     * @param value The Json to append.
     * @param path The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error If path is invalid.
     */
    void appendJson(const Json& value, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr {path.data()};

        if (!m_document.contains(ptr))
        {
            if (m_document.empty())
            {
                m_document = nlohmann::ordered_json::parse("{}");
            }
        }
        if (!m_document[ptr].is_array())
        {
            m_document[ptr] = nlohmann::ordered_json::array();
        }

        m_document[ptr].emplace_back(value.m_document);
    }

    /**
     * @brief Erase Json object at the path.
     *
     * @param path The path to the object, default value is root object ("").
     * @return true if object was erased, false if object was not found.
     *
     * @throws std::runtime_error If path is invalid.
     */
    bool erase(std::string_view path = "")
    {
        bool result {false};

        if (path.empty())
        {
            try
            {
                auto it = m_document.erase(m_document.begin());
                result = (it == m_document.end());
            }
            catch (...)
            {
            }
            m_document = nlohmann::ordered_json();
        }
        else
        {
            nlohmann::ordered_json::json_pointer ptr {path.data()};
            try
            {
                result = m_document[ptr.parent_pointer()].erase(ptr.back()) > 0 ? true : false;
            }
            catch (...)
            {
            }
        }

        return result;
    }

    /**
     * @brief Merge the Json Value at the path with the given Json Value.
     *
     * Objects are merged, arrays are appended.
     * Merges only first level of the Json Value.
     *
     * @param other The Json Value to merge.
     * @param path  The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error On the following conditions:
     * - If path is invalid.
     * - If either Json Values are not Object or Array.
     * - If Json Values are not the same type.
     */
    void merge(const bool isRecursive, Json& other, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr{path.data()};

        // Check if the path is valid
        if (!m_document.contains(ptr))
        {
            throw std::runtime_error("Path does not exists on Json object.");
        }

        auto& valueThis = m_document[ptr];
        auto& valueOther = other.m_document;

        if (!valueThis.is_object() && !valueThis.is_array())
        {
            throw std::runtime_error("Destination path is not an array nor an object.");
        }

        if (!other.isObject() && !other.isArray())
        {
            throw std::runtime_error("Json input is not an array nor an object.");
        }

        if (valueThis.type() != valueOther.type())
        {
            throw std::runtime_error("Json operands are not of the same type.");
        }

        // If the value is an object, merge it with the other object
        if (valueThis.is_object())
        {
            if (isRecursive)
            {
                valueThis.merge_patch(valueOther);
            }
            else
            {
                valueThis.update(valueOther);
            }
        }
        // If the value is an array, append the other array to it
        else if (valueThis.is_array())
        {
            // valueThis.insert(valueThis.end(), other.m_document.begin(), other.m_document.end());
            for (const auto value : valueOther)
            {
                if (std::find(valueThis.begin(), valueThis.end(), value) == valueThis.end())
                {
                    valueThis.insert(valueThis.end(), value);
                }
            }
        }
    }

    void merge(const bool isRecursive, nlohmann::ordered_json::value_type& source, std::string_view path = "")
    {
        Json otherJson {source};
        merge(isRecursive, otherJson, path);
    }

    /**
     * @brief Merge the Json Value at the path with the given Json Value at reference
     * path.
     *
     * Merges only first level of the Json Value.
     * Reference value is deleted after merge.
     *
     * @param source The Json path pointing to the value to be merged.
     * @param path  The path to the object, default value is root object ("").
     *
     * @throws std::runtime_error On the following conditions:
     * - If either path are invalid.
     * - If either Json Values are not Object or Array.
     * - If Json Values are not the same type.
     */
    void merge(const bool isRecursive, std::string_view source, std::string_view path = "")
    {
        nlohmann::ordered_json::json_pointer ptr{source.data()};

        if (!m_document.contains(ptr))
        {
            throw std::runtime_error("Path not found on JSON object");
        }

        Json sourceJson {m_document[ptr]};
        merge(isRecursive, sourceJson, path);

        // Delete reference value
        m_document[ptr.parent_pointer()].erase(ptr.back());
    }
};

#endif
} // namespace json

#endif // _JSON_H
