//! JSON type compatible with C++ Star::Json
//!
//! This module provides a JSON type that matches the functionality
//! of the C++ implementation and integrates with serde_json.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fmt;

/// JSON value type enumeration matching C++ Star::Json::Type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[repr(u8)]
pub enum JsonType {
    Null = 0,
    Float = 1,
    Bool = 2,
    Int = 3,
    String = 4,
    Array = 5,
    Object = 6,
}

impl JsonType {
    /// Get the name of this type
    pub fn name(&self) -> &'static str {
        match self {
            JsonType::Null => "null",
            JsonType::Float => "float",
            JsonType::Bool => "bool",
            JsonType::Int => "int",
            JsonType::String => "string",
            JsonType::Array => "array",
            JsonType::Object => "object",
        }
    }

    /// Parse a type from its name
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "null" => Some(JsonType::Null),
            "float" | "double" | "number" => Some(JsonType::Float),
            "bool" | "boolean" => Some(JsonType::Bool),
            "int" | "integer" => Some(JsonType::Int),
            "string" => Some(JsonType::String),
            "array" | "list" => Some(JsonType::Array),
            "object" | "map" => Some(JsonType::Object),
            _ => None,
        }
    }
}

impl fmt::Display for JsonType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// A JSON value compatible with C++ Star::Json
///
/// This wraps serde_json::Value but provides an API compatible with
/// the C++ implementation.
#[derive(Clone, PartialEq, Serialize, Deserialize)]
#[serde(transparent)]
pub struct Json(serde_json::Value);

impl Default for Json {
    fn default() -> Self {
        Self::null()
    }
}

impl Json {
    // Constructors

    /// Create a null value
    pub fn null() -> Self {
        Self(serde_json::Value::Null)
    }

    /// Create a boolean value
    pub fn bool(b: bool) -> Self {
        Self(serde_json::Value::Bool(b))
    }

    /// Create an integer value
    pub fn int(i: i64) -> Self {
        Self(serde_json::Value::Number(serde_json::Number::from(i)))
    }

    /// Create a float value
    pub fn float(f: f64) -> Self {
        Self(serde_json::Value::Number(
            serde_json::Number::from_f64(f).unwrap_or_else(|| serde_json::Number::from(0)),
        ))
    }

    /// Create a string value
    pub fn string<S: Into<String>>(s: S) -> Self {
        Self(serde_json::Value::String(s.into()))
    }

    /// Create an array value
    pub fn array(arr: Vec<Json>) -> Self {
        Self(serde_json::Value::Array(
            arr.into_iter().map(|j| j.0).collect(),
        ))
    }

    /// Create an empty array
    pub fn empty_array() -> Self {
        Self(serde_json::Value::Array(Vec::new()))
    }

    /// Create an object value
    pub fn object(obj: HashMap<String, Json>) -> Self {
        Self(serde_json::Value::Object(
            obj.into_iter().map(|(k, v)| (k, v.0)).collect(),
        ))
    }

    /// Create an empty object
    pub fn empty_object() -> Self {
        Self(serde_json::Value::Object(serde_json::Map::new()))
    }

    /// Create a value of the given type with default value
    pub fn of_type(t: JsonType) -> Self {
        match t {
            JsonType::Null => Self::null(),
            JsonType::Float => Self::float(0.0),
            JsonType::Bool => Self::bool(false),
            JsonType::Int => Self::int(0),
            JsonType::String => Self::string(""),
            JsonType::Array => Self::empty_array(),
            JsonType::Object => Self::empty_object(),
        }
    }

    // Parsing

    /// Parse JSON from a string
    pub fn parse(s: &str) -> Result<Self, crate::error::Error> {
        serde_json::from_str(s)
            .map(Self)
            .map_err(|e| crate::error::Error::Json(e))
    }

    // Type checking

    /// Get the type of this value
    pub fn get_type(&self) -> JsonType {
        match &self.0 {
            serde_json::Value::Null => JsonType::Null,
            serde_json::Value::Bool(_) => JsonType::Bool,
            serde_json::Value::Number(n) => {
                if n.is_i64() || n.is_u64() {
                    JsonType::Int
                } else {
                    JsonType::Float
                }
            }
            serde_json::Value::String(_) => JsonType::String,
            serde_json::Value::Array(_) => JsonType::Array,
            serde_json::Value::Object(_) => JsonType::Object,
        }
    }

    /// Check if this is a null value
    pub fn is_null(&self) -> bool {
        self.0.is_null()
    }

    /// Check if this is a boolean value
    pub fn is_bool(&self) -> bool {
        self.0.is_boolean()
    }

    /// Check if this is an integer value
    pub fn is_int(&self) -> bool {
        self.0.is_i64() || self.0.is_u64()
    }

    /// Check if this is a float value
    pub fn is_float(&self) -> bool {
        self.0.is_f64()
    }

    /// Check if this is a number (int or float)
    pub fn is_number(&self) -> bool {
        self.0.is_number()
    }

    /// Check if this is a string value
    pub fn is_string(&self) -> bool {
        self.0.is_string()
    }

    /// Check if this is an array value
    pub fn is_array(&self) -> bool {
        self.0.is_array()
    }

    /// Check if this is an object value
    pub fn is_object(&self) -> bool {
        self.0.is_object()
    }

    // Conversion

    /// Convert to boolean
    pub fn to_bool(&self) -> Option<bool> {
        self.0.as_bool()
    }

    /// Convert to i64
    pub fn to_int(&self) -> Option<i64> {
        self.0.as_i64()
    }

    /// Convert to u64
    pub fn to_uint(&self) -> Option<u64> {
        self.0.as_u64()
    }

    /// Convert to f64
    pub fn to_float(&self) -> Option<f64> {
        self.0.as_f64()
    }

    /// Convert to f32
    /// Note: May lose precision for very large or very small values
    pub fn to_float32(&self) -> Option<f32> {
        self.0.as_f64().map(|f| {
            // Handle special cases
            if f.is_nan() {
                f32::NAN
            } else if f.is_infinite() {
                if f.is_sign_positive() { f32::INFINITY } else { f32::NEG_INFINITY }
            } else {
                f as f32
            }
        })
    }

    /// Get as string reference
    pub fn as_str(&self) -> Option<&str> {
        self.0.as_str()
    }

    /// Convert to owned String
    pub fn to_string_value(&self) -> Option<String> {
        self.0.as_str().map(|s| s.to_string())
    }

    /// Get as array reference
    pub fn as_array(&self) -> Option<Vec<Json>> {
        self.0.as_array().map(|arr| arr.iter().map(|v| Json(v.clone())).collect())
    }

    /// Get as object reference
    pub fn as_object(&self) -> Option<HashMap<String, Json>> {
        self.0.as_object().map(|obj| {
            obj.iter()
                .map(|(k, v)| (k.clone(), Json(v.clone())))
                .collect()
        })
    }

    // Array operations

    /// Get array length (0 if not an array)
    pub fn len(&self) -> usize {
        match &self.0 {
            serde_json::Value::Array(arr) => arr.len(),
            serde_json::Value::Object(obj) => obj.len(),
            _ => 0,
        }
    }

    /// Check if empty
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Get element at index (for arrays)
    pub fn get(&self, index: usize) -> Option<Json> {
        self.0.get(index).map(|v| Json(v.clone()))
    }

    /// Get element by key (for objects)
    pub fn get_key(&self, key: &str) -> Option<Json> {
        self.0.get(key).map(|v| Json(v.clone()))
    }

    /// Check if object contains key
    pub fn contains(&self, key: &str) -> bool {
        match &self.0 {
            serde_json::Value::Object(obj) => obj.contains_key(key),
            _ => false,
        }
    }

    /// Get keys of object
    pub fn keys(&self) -> Vec<String> {
        match &self.0 {
            serde_json::Value::Object(obj) => obj.keys().cloned().collect(),
            _ => Vec::new(),
        }
    }

    // Serialization

    /// Serialize to JSON string
    pub fn to_json_string(&self) -> String {
        serde_json::to_string(&self.0).unwrap_or_else(|_| "null".to_string())
    }

    /// Serialize to pretty JSON string
    pub fn to_json_pretty(&self) -> String {
        serde_json::to_string_pretty(&self.0).unwrap_or_else(|_| "null".to_string())
    }

    /// Get underlying serde_json::Value
    pub fn into_inner(self) -> serde_json::Value {
        self.0
    }

    /// Get reference to underlying serde_json::Value
    pub fn as_inner(&self) -> &serde_json::Value {
        &self.0
    }
}

// From implementations for convenient construction

impl From<bool> for Json {
    fn from(b: bool) -> Self {
        Self::bool(b)
    }
}

impl From<i32> for Json {
    fn from(i: i32) -> Self {
        Self::int(i as i64)
    }
}

impl From<i64> for Json {
    fn from(i: i64) -> Self {
        Self::int(i)
    }
}

impl From<u32> for Json {
    fn from(i: u32) -> Self {
        Self::int(i as i64)
    }
}

impl From<u64> for Json {
    fn from(i: u64) -> Self {
        // Handle u64 values that exceed i64::MAX
        if i <= i64::MAX as u64 {
            Self::int(i as i64)
        } else {
            // For values > i64::MAX, store as float to preserve value
            Self::float(i as f64)
        }
    }
}

impl From<f32> for Json {
    fn from(f: f32) -> Self {
        Self::float(f as f64)
    }
}

impl From<f64> for Json {
    fn from(f: f64) -> Self {
        Self::float(f)
    }
}

impl From<&str> for Json {
    fn from(s: &str) -> Self {
        Self::string(s)
    }
}

impl From<String> for Json {
    fn from(s: String) -> Self {
        Self::string(s)
    }
}

impl<T: Into<Json>> From<Vec<T>> for Json {
    fn from(v: Vec<T>) -> Self {
        Self::array(v.into_iter().map(Into::into).collect())
    }
}

impl From<serde_json::Value> for Json {
    fn from(v: serde_json::Value) -> Self {
        Self(v)
    }
}

impl From<Json> for serde_json::Value {
    fn from(j: Json) -> Self {
        j.0
    }
}

impl fmt::Display for Json {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_json_string())
    }
}

impl fmt::Debug for Json {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Json({})", self.to_json_pretty())
    }
}

impl std::str::FromStr for Json {
    type Err = crate::error::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Self::parse(s)
    }
}

// Index implementations

impl std::ops::Index<usize> for Json {
    type Output = serde_json::Value;

    fn index(&self, index: usize) -> &Self::Output {
        &self.0[index]
    }
}

impl std::ops::Index<&str> for Json {
    type Output = serde_json::Value;

    fn index(&self, key: &str) -> &Self::Output {
        &self.0[key]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_json_null() {
        let j = Json::null();
        assert!(j.is_null());
        assert_eq!(j.get_type(), JsonType::Null);
    }

    #[test]
    fn test_json_bool() {
        let j = Json::bool(true);
        assert!(j.is_bool());
        assert_eq!(j.to_bool(), Some(true));
    }

    #[test]
    fn test_json_int() {
        let j = Json::int(42);
        assert!(j.is_int());
        assert_eq!(j.to_int(), Some(42));
    }

    #[test]
    fn test_json_float() {
        let j = Json::float(3.14);
        assert!(j.is_number());
        assert!((j.to_float().unwrap() - 3.14).abs() < 0.001);
    }

    #[test]
    fn test_json_string() {
        let j = Json::string("hello");
        assert!(j.is_string());
        assert_eq!(j.as_str(), Some("hello"));
    }

    #[test]
    fn test_json_array() {
        let j = Json::array(vec![Json::int(1), Json::int(2), Json::int(3)]);
        assert!(j.is_array());
        assert_eq!(j.len(), 3);
        assert_eq!(j.get(0).unwrap().to_int(), Some(1));
    }

    #[test]
    fn test_json_object() {
        let mut obj = HashMap::new();
        obj.insert("key".to_string(), Json::string("value"));
        let j = Json::object(obj);
        assert!(j.is_object());
        assert!(j.contains("key"));
        assert_eq!(j.get_key("key").unwrap().as_str(), Some("value"));
    }

    #[test]
    fn test_json_parse() {
        let j: Json = r#"{"name": "test", "value": 42}"#.parse().unwrap();
        assert!(j.is_object());
        assert_eq!(j.get_key("name").unwrap().as_str(), Some("test"));
        assert_eq!(j.get_key("value").unwrap().to_int(), Some(42));
    }

    #[test]
    fn test_json_serialize() {
        let j = Json::object({
            let mut m = HashMap::new();
            m.insert("a".to_string(), Json::int(1));
            m
        });
        let s = j.to_json_string();
        assert!(s.contains("\"a\":1") || s.contains("\"a\": 1"));
    }

    #[test]
    fn test_json_type_name() {
        assert_eq!(JsonType::Null.name(), "null");
        assert_eq!(JsonType::Object.name(), "object");
        assert_eq!(JsonType::from_name("array"), Some(JsonType::Array));
    }

    #[test]
    fn test_json_from_types() {
        let _j1: Json = true.into();
        let _j2: Json = 42i32.into();
        let _j3: Json = 3.14f64.into();
        let _j4: Json = "hello".into();
        let _j5: Json = vec![1i32, 2, 3].into();
    }
}
