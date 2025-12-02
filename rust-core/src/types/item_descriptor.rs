//! Item descriptor for item specification and serialization.
//!
//! Compatible with C++ Star::ItemDescriptor from StarItemDescriptor.hpp

use crate::types::Json;
use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::error::{Error, Result};
use std::io::{Read, Write};
use std::hash::{Hash, Hasher};
use std::fmt;
use std::collections::hash_map::DefaultHasher;

/// Describes an item with name, count, and parameters.
#[derive(Clone)]
pub struct ItemDescriptor {
    name: String,
    count: u64,
    parameters: Json,
    parameters_hash: Option<u64>,
}

impl Default for ItemDescriptor {
    fn default() -> Self {
        Self {
            name: String::new(),
            count: 0,
            parameters: Json::null(),
            parameters_hash: None,
        }
    }
}

impl ItemDescriptor {
    /// Create a new item descriptor.
    pub fn new(name: impl Into<String>, count: u64) -> Self {
        Self {
            name: name.into(),
            count,
            parameters: Json::null(),
            parameters_hash: None,
        }
    }

    /// Create a new item descriptor with parameters.
    pub fn with_parameters(name: impl Into<String>, count: u64, parameters: Json) -> Self {
        Self {
            name: name.into(),
            count,
            parameters,
            parameters_hash: None,
        }
    }

    /// Load from store format (compact binary format).
    pub fn load_store(store: &Json) -> Result<Self> {
        Self::from_json(store)
    }

    /// Parse from JSON specification.
    /// 
    /// Accepts either:
    /// - An array: `[name, count, parameters]`
    /// - An object: `{"name": "...", "count": N, "parameters": {...}}`
    pub fn from_json(spec: &Json) -> Result<Self> {
        if spec.is_null() {
            return Ok(Self::default());
        }

        if let Some(arr) = spec.as_array() {
            // Array format: [name, count, parameters]
            let name = arr.first()
                .and_then(|v| v.as_str())
                .ok_or_else(|| Error::Parse("ItemDescriptor array requires name as first element".into()))?
                .to_string();
            
            let count = arr.get(1)
                .and_then(|v| v.to_uint())
                .unwrap_or(1);
            
            let parameters = arr.get(2).cloned().unwrap_or(Json::null());
            
            Ok(Self::with_parameters(name, count, parameters))
        } else if let Some(obj) = spec.as_object() {
            // Object format: {name, count, parameters}
            let name = obj.get("name")
                .and_then(|v| v.as_str())
                .unwrap_or("")
                .to_string();
            
            let count = obj.get("count")
                .and_then(|v| v.to_uint())
                .unwrap_or(1);
            
            let parameters = obj.get("parameters")
                .cloned()
                .unwrap_or(Json::null());
            
            Ok(Self::with_parameters(name, count, parameters))
        } else if let Some(name) = spec.as_str() {
            // Just a string name
            Ok(Self::new(name, 1))
        } else {
            Err(Error::Parse("Invalid ItemDescriptor format".into()))
        }
    }

    /// Get the item name.
    pub fn name(&self) -> &str {
        &self.name
    }

    /// Get the item count.
    pub fn count(&self) -> u64 {
        self.count
    }

    /// Get the item parameters.
    pub fn parameters(&self) -> &Json {
        &self.parameters
    }

    /// Create a singular version (count = 1).
    pub fn singular(&self) -> Self {
        Self::with_parameters(self.name.clone(), 1, self.parameters.clone())
    }

    /// Create a copy with a different count.
    pub fn with_count(&self, count: u64) -> Self {
        Self::with_parameters(self.name.clone(), count, self.parameters.clone())
    }

    /// Multiply the count.
    pub fn multiply(&self, factor: u64) -> Self {
        Self::with_parameters(self.name.clone(), self.count * factor, self.parameters.clone())
    }

    /// Apply additional parameters (merge with existing).
    /// Note: This creates a new JSON object instead of mutating in place.
    pub fn apply_parameters(&self, new_params: &serde_json::Map<String, serde_json::Value>) -> Self {
        let params = if let Some(obj) = self.parameters.as_object() {
            let mut merged: serde_json::Map<String, serde_json::Value> = obj.iter()
                .map(|(k, v)| (k.clone(), v.clone().into_inner()))
                .collect();
            for (k, v) in new_params {
                merged.insert(k.clone(), v.clone());
            }
            Json::from(serde_json::Value::Object(merged))
        } else if !new_params.is_empty() {
            Json::from(serde_json::Value::Object(new_params.clone()))
        } else {
            self.parameters.clone()
        };
        
        Self::with_parameters(self.name.clone(), self.count, params)
    }

    /// Check if this is a null descriptor.
    pub fn is_null(&self) -> bool {
        self.name.is_empty()
    }

    /// Check if this is empty (null or count 0).
    pub fn is_empty(&self) -> bool {
        self.is_null() || self.count == 0
    }

    /// Check if this descriptor matches another.
    /// 
    /// If `exact_match` is true, parameters must match exactly.
    /// Otherwise, only name is compared.
    pub fn matches(&self, other: &ItemDescriptor, exact_match: bool) -> bool {
        if self.name != other.name {
            return false;
        }
        
        if exact_match {
            self.parameters_hash() == other.parameters_hash()
        } else {
            true
        }
    }

    /// Store to disk format (versioned structure).
    pub fn disk_store(&self) -> Json {
        self.to_json()
    }

    /// Convert to JSON specification format.
    pub fn to_json(&self) -> Json {
        if self.is_null() {
            return Json::null();
        }

        let mut obj = serde_json::Map::new();
        obj.insert("name".to_string(), serde_json::Value::String(self.name.clone()));
        obj.insert("count".to_string(), serde_json::Value::Number(self.count.into()));
        
        if !self.parameters.is_null() {
            obj.insert("parameters".to_string(), self.parameters.clone().into_inner());
        }
        
        Json::from(serde_json::Value::Object(obj))
    }

    /// Get the parameters hash for comparison.
    fn parameters_hash(&self) -> u64 {
        if let Some(hash) = self.parameters_hash {
            return hash;
        }
        
        let mut hasher = DefaultHasher::new();
        let json_str = self.parameters.to_string();
        json_str.hash(&mut hasher);
        hasher.finish()
    }
}

impl PartialEq for ItemDescriptor {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name 
            && self.count == other.count 
            && self.parameters_hash() == other.parameters_hash()
    }
}

impl Eq for ItemDescriptor {}

impl Hash for ItemDescriptor {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.name.hash(state);
        self.count.hash(state);
        self.parameters_hash().hash(state);
    }
}

impl fmt::Debug for ItemDescriptor {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("ItemDescriptor")
            .field("name", &self.name)
            .field("count", &self.count)
            .field("parameters", &self.parameters)
            .finish()
    }
}

impl fmt::Display for ItemDescriptor {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.is_null() {
            write!(f, "(null)")
        } else if self.parameters.is_null() {
            write!(f, "{}x{}", self.count, self.name)
        } else {
            write!(f, "{}x{} {}", self.count, self.name, self.parameters)
        }
    }
}

impl Readable for ItemDescriptor {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        let name = reader.read_string()?;
        let count = reader.read_var_u64()?;
        let params_str = reader.read_string()?;
        
        let parameters = if params_str.is_empty() {
            Json::null()
        } else {
            Json::parse(&params_str)?
        };
        
        Ok(Self::with_parameters(name, count, parameters))
    }
}

impl Writable for ItemDescriptor {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_string(&self.name)?;
        writer.write_var_u64(self.count)?;
        
        if self.parameters.is_null() {
            writer.write_string("")?;
        } else {
            writer.write_string(&self.parameters.to_string())?;
        }
        
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_item_descriptor_default() {
        let desc = ItemDescriptor::default();
        assert!(desc.is_null());
        assert!(desc.is_empty());
        assert_eq!(desc.name(), "");
        assert_eq!(desc.count(), 0);
    }

    #[test]
    fn test_item_descriptor_new() {
        let desc = ItemDescriptor::new("sword", 5);
        assert!(!desc.is_null());
        assert!(!desc.is_empty());
        assert_eq!(desc.name(), "sword");
        assert_eq!(desc.count(), 5);
    }

    #[test]
    fn test_item_descriptor_with_parameters() {
        let params = Json::parse(r#"{"damage": 10}"#).unwrap();
        let desc = ItemDescriptor::with_parameters("sword", 1, params);
        
        assert_eq!(desc.name(), "sword");
        assert!(!desc.parameters().is_null());
    }

    #[test]
    fn test_item_descriptor_from_json_array() {
        let json = Json::parse(r#"["sword", 5, {"damage": 10}]"#).unwrap();
        let desc = ItemDescriptor::from_json(&json).unwrap();
        
        assert_eq!(desc.name(), "sword");
        assert_eq!(desc.count(), 5);
    }

    #[test]
    fn test_item_descriptor_from_json_object() {
        let json = Json::parse(r#"{"name": "shield", "count": 2}"#).unwrap();
        let desc = ItemDescriptor::from_json(&json).unwrap();
        
        assert_eq!(desc.name(), "shield");
        assert_eq!(desc.count(), 2);
    }

    #[test]
    fn test_item_descriptor_from_json_string() {
        let json = Json::parse(r#""potion""#).unwrap();
        let desc = ItemDescriptor::from_json(&json).unwrap();
        
        assert_eq!(desc.name(), "potion");
        assert_eq!(desc.count(), 1);
    }

    #[test]
    fn test_item_descriptor_singular() {
        let desc = ItemDescriptor::new("arrow", 50);
        let single = desc.singular();
        
        assert_eq!(single.name(), "arrow");
        assert_eq!(single.count(), 1);
    }

    #[test]
    fn test_item_descriptor_multiply() {
        let desc = ItemDescriptor::new("coin", 10);
        let multiplied = desc.multiply(5);
        
        assert_eq!(multiplied.count(), 50);
    }

    #[test]
    fn test_item_descriptor_with_count() {
        let desc = ItemDescriptor::new("gem", 3);
        let more = desc.with_count(100);
        
        assert_eq!(more.count(), 100);
    }

    #[test]
    fn test_item_descriptor_matches() {
        let desc1 = ItemDescriptor::new("sword", 5);
        let desc2 = ItemDescriptor::new("sword", 10);
        let desc3 = ItemDescriptor::new("shield", 5);
        
        assert!(desc1.matches(&desc2, false));
        assert!(!desc1.matches(&desc3, false));
    }

    #[test]
    fn test_item_descriptor_to_json() {
        let desc = ItemDescriptor::new("item", 3);
        let json = desc.to_json();
        
        let name = json.get_key("name").map(|v| v.to_string_value()).flatten();
        assert_eq!(name.as_deref(), Some("item"));
        assert_eq!(json.get_key("count").and_then(|v| v.to_uint()), Some(3));
    }

    #[test]
    fn test_item_descriptor_display() {
        let desc = ItemDescriptor::new("sword", 5);
        assert_eq!(format!("{}", desc), "5xsword");
        
        let null_desc = ItemDescriptor::default();
        assert_eq!(format!("{}", null_desc), "(null)");
    }

    #[test]
    fn test_item_descriptor_serialization() {
        let original = ItemDescriptor::new("weapon", 10);
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: ItemDescriptor = reader.read().unwrap();
        
        assert_eq!(read.name(), original.name());
        assert_eq!(read.count(), original.count());
    }

    #[test]
    fn test_item_descriptor_equality() {
        let desc1 = ItemDescriptor::new("sword", 5);
        let desc2 = ItemDescriptor::new("sword", 5);
        let desc3 = ItemDescriptor::new("sword", 10);
        
        assert_eq!(desc1, desc2);
        assert_ne!(desc1, desc3);
    }

    #[test]
    fn test_item_descriptor_hash() {
        use std::collections::HashSet;
        
        let desc1 = ItemDescriptor::new("sword", 5);
        let desc2 = ItemDescriptor::new("sword", 5);
        
        let mut set = HashSet::new();
        set.insert(desc1);
        
        // Same descriptor should not be added again
        assert!(!set.insert(desc2));
    }
}
