//! Network element synchronization compatible with C++ Star::NetElement
//!
//! This module provides network synchronization primitives for multiplayer game state.

use crate::error::{Error, Result};
use std::sync::atomic::{AtomicU64, Ordering};

/// Network compatibility version - use AnyVersion to match any version
pub type VersionNumber = u32;

/// Special version number that matches any version
pub const ANY_VERSION: VersionNumber = 0;

/// Monotonically increasing NetElementVersion shared between all NetElements in a network
#[derive(Debug, Default)]
pub struct NetElementVersion {
    version: AtomicU64,
}

impl NetElementVersion {
    /// Create a new version tracker
    pub fn new() -> Self {
        Self {
            version: AtomicU64::new(0),
        }
    }

    /// Get the current version
    pub fn current(&self) -> u64 {
        self.version.load(Ordering::Acquire)
    }

    /// Increment and return the new version
    pub fn increment(&self) -> u64 {
        self.version.fetch_add(1, Ordering::AcqRel) + 1
    }
}

impl Clone for NetElementVersion {
    fn clone(&self) -> Self {
        Self {
            version: AtomicU64::new(self.version.load(Ordering::Acquire)),
        }
    }
}

/// Network compatibility rules for versioning
#[derive(Debug, Clone, Default)]
pub struct NetCompatibilityRules {
    version: VersionNumber,
}

impl NetCompatibilityRules {
    /// Create new rules with the given version
    pub fn new(version: VersionNumber) -> Self {
        Self { version }
    }

    /// Get the version
    pub fn version(&self) -> VersionNumber {
        self.version
    }

    /// Create rules that match any version
    pub fn any() -> Self {
        Self { version: ANY_VERSION }
    }
}

/// Base implementation for NetElement with common functionality
#[derive(Debug, Clone, Default)]
pub struct NetElementBase {
    /// Last updated version
    last_updated: u64,
    /// Compatibility version
    compatibility_version: VersionNumber,
    /// Whether interpolation is enabled
    interpolation_enabled: bool,
    /// Extrapolation hint time
    extrapolation_hint: f32,
}

impl NetElementBase {
    /// Create a new base element
    pub fn new() -> Self {
        Self::default()
    }

    /// Mark the element as updated
    pub fn mark_updated(&mut self, version: &NetElementVersion) {
        self.last_updated = version.current();
    }

    /// Check if updated since the given version
    pub fn updated_since(&self, version: u64) -> bool {
        self.last_updated >= version
    }

    /// Get the last updated version
    pub fn last_updated(&self) -> u64 {
        self.last_updated
    }

    /// Get the compatibility version
    pub fn compatibility_version(&self) -> VersionNumber {
        self.compatibility_version
    }

    /// Set the compatibility version
    pub fn set_compatibility_version(&mut self, version: VersionNumber) {
        self.compatibility_version = version;
    }

    /// Check if this element should be synced with the given rules
    pub fn check_with_rules(&self, rules: &NetCompatibilityRules) -> bool {
        if self.compatibility_version != ANY_VERSION {
            rules.version() >= self.compatibility_version
        } else {
            true
        }
    }
}

/// A simple boolean net element
#[derive(Debug, Clone, Default)]
pub struct NetElementBool {
    base: NetElementBase,
    value: bool,
    interpolated_value: Option<bool>,
    target_value: Option<bool>,
    interpolation_time: f32,
}

impl NetElementBool {
    /// Create a new boolean element
    pub fn new(initial: bool) -> Self {
        Self {
            base: NetElementBase::new(),
            value: initial,
            interpolated_value: None,
            target_value: None,
            interpolation_time: 0.0,
        }
    }

    /// Get the value
    pub fn get(&self) -> bool {
        self.interpolated_value.unwrap_or(self.value)
    }

    /// Set the value
    pub fn set(&mut self, value: bool, version: &NetElementVersion) {
        if self.value != value {
            self.value = value;
            self.base.mark_updated(version);
        }
    }

    /// Store to bytes
    pub fn store(&self) -> Vec<u8> {
        vec![if self.value { 1 } else { 0 }]
    }

    /// Load from bytes
    pub fn load(&mut self, data: &[u8]) -> Result<()> {
        if data.is_empty() {
            return Err(Error::Serialization("Empty data for NetElementBool".into()));
        }
        self.value = data[0] != 0;
        Ok(())
    }

    /// Enable interpolation
    pub fn enable_interpolation(&mut self, extrapolation_hint: f32) {
        self.base.interpolation_enabled = true;
        self.base.extrapolation_hint = extrapolation_hint;
    }

    /// Disable interpolation
    pub fn disable_interpolation(&mut self) {
        self.base.interpolation_enabled = false;
        self.interpolated_value = None;
        self.target_value = None;
    }

    /// Tick interpolation
    pub fn tick_interpolation(&mut self, dt: f32) {
        if !self.base.interpolation_enabled {
            return;
        }

        if let Some(target) = self.target_value {
            self.interpolation_time -= dt;
            if self.interpolation_time <= 0.0 {
                self.interpolated_value = Some(target);
                self.target_value = None;
            }
        }
    }
}

/// A simple integer net element
#[derive(Debug, Clone, Default)]
pub struct NetElementInt {
    base: NetElementBase,
    value: i64,
    interpolated_value: Option<i64>,
    target_value: Option<i64>,
    interpolation_time: f32,
}

impl NetElementInt {
    /// Create a new integer element
    pub fn new(initial: i64) -> Self {
        Self {
            base: NetElementBase::new(),
            value: initial,
            interpolated_value: None,
            target_value: None,
            interpolation_time: 0.0,
        }
    }

    /// Get the value
    pub fn get(&self) -> i64 {
        self.interpolated_value.unwrap_or(self.value)
    }

    /// Set the value
    pub fn set(&mut self, value: i64, version: &NetElementVersion) {
        if self.value != value {
            self.value = value;
            self.base.mark_updated(version);
        }
    }

    /// Store to bytes (little-endian)
    pub fn store(&self) -> Vec<u8> {
        self.value.to_le_bytes().to_vec()
    }

    /// Load from bytes
    pub fn load(&mut self, data: &[u8]) -> Result<()> {
        if data.len() < 8 {
            return Err(Error::Serialization("Not enough data for NetElementInt".into()));
        }
        let mut buf = [0u8; 8];
        buf.copy_from_slice(&data[..8]);
        self.value = i64::from_le_bytes(buf);
        Ok(())
    }

    /// Enable interpolation
    pub fn enable_interpolation(&mut self, extrapolation_hint: f32) {
        self.base.interpolation_enabled = true;
        self.base.extrapolation_hint = extrapolation_hint;
    }

    /// Disable interpolation
    pub fn disable_interpolation(&mut self) {
        self.base.interpolation_enabled = false;
        self.interpolated_value = None;
        self.target_value = None;
    }

    /// Tick interpolation
    pub fn tick_interpolation(&mut self, dt: f32) {
        if !self.base.interpolation_enabled {
            return;
        }

        if let Some(target) = self.target_value {
            self.interpolation_time -= dt;
            if self.interpolation_time <= 0.0 {
                self.interpolated_value = Some(target);
                self.target_value = None;
            } else {
                let current = self.interpolated_value.unwrap_or(self.value);
                // Use safe division avoiding potential overflow
                let t = if self.interpolation_time > 0.0 {
                    (dt / self.interpolation_time).min(1.0)
                } else {
                    1.0
                };
                let interp = current + ((target - current) as f32 * t) as i64;
                self.interpolated_value = Some(interp);
            }
        }
    }
}

/// A floating point net element with interpolation
#[derive(Debug, Clone, Default)]
pub struct NetElementFloat {
    base: NetElementBase,
    value: f64,
    interpolated_value: Option<f64>,
    target_value: Option<f64>,
    interpolation_time: f32,
    velocity: f64,
}

impl NetElementFloat {
    /// Create a new float element
    pub fn new(initial: f64) -> Self {
        Self {
            base: NetElementBase::new(),
            value: initial,
            interpolated_value: None,
            target_value: None,
            interpolation_time: 0.0,
            velocity: 0.0,
        }
    }

    /// Get the value
    pub fn get(&self) -> f64 {
        self.interpolated_value.unwrap_or(self.value)
    }

    /// Set the value
    pub fn set(&mut self, value: f64, version: &NetElementVersion) {
        if (self.value - value).abs() > f64::EPSILON {
            self.value = value;
            self.base.mark_updated(version);
        }
    }

    /// Store to bytes
    pub fn store(&self) -> Vec<u8> {
        self.value.to_le_bytes().to_vec()
    }

    /// Load from bytes
    pub fn load(&mut self, data: &[u8]) -> Result<()> {
        if data.len() < 8 {
            return Err(Error::Serialization("Not enough data for NetElementFloat".into()));
        }
        let mut buf = [0u8; 8];
        buf.copy_from_slice(&data[..8]);
        self.value = f64::from_le_bytes(buf);
        Ok(())
    }

    /// Enable interpolation
    pub fn enable_interpolation(&mut self, extrapolation_hint: f32) {
        self.base.interpolation_enabled = true;
        self.base.extrapolation_hint = extrapolation_hint;
    }

    /// Disable interpolation
    pub fn disable_interpolation(&mut self) {
        self.base.interpolation_enabled = false;
        self.interpolated_value = None;
        self.target_value = None;
        self.velocity = 0.0;
    }

    /// Tick interpolation
    pub fn tick_interpolation(&mut self, dt: f32) {
        if !self.base.interpolation_enabled {
            return;
        }

        if let Some(target) = self.target_value {
            // First compute the step while time is still positive
            let current = self.interpolated_value.unwrap_or(self.value);
            
            if self.interpolation_time <= dt {
                // We'll reach the target this tick
                self.interpolated_value = Some(target);
                self.target_value = None;
                self.velocity = 0.0;
            } else {
                // Interpolate proportionally
                let t = dt as f64 / self.interpolation_time as f64;
                let interp = current + (target - current) * t;
                self.interpolated_value = Some(interp);
                self.interpolation_time -= dt;
                self.velocity = (target - current) / self.interpolation_time as f64;
            }
        } else if self.base.extrapolation_hint > 0.0 && self.velocity.abs() > f64::EPSILON {
            if let Some(current) = self.interpolated_value {
                self.interpolated_value = Some(current + self.velocity * dt as f64);
            }
        }
    }
}

/// A string net element
#[derive(Debug, Clone, Default)]
pub struct NetElementString {
    base: NetElementBase,
    value: String,
}

impl NetElementString {
    /// Create a new string element
    pub fn new(initial: impl Into<String>) -> Self {
        Self {
            base: NetElementBase::new(),
            value: initial.into(),
        }
    }

    /// Get the value
    pub fn get(&self) -> &str {
        &self.value
    }

    /// Set the value
    pub fn set(&mut self, value: impl Into<String>, version: &NetElementVersion) {
        let new_value = value.into();
        if self.value != new_value {
            self.value = new_value;
            self.base.mark_updated(version);
        }
    }

    /// Store to bytes (length-prefixed)
    pub fn store(&self) -> Vec<u8> {
        let len = self.value.len() as u32;
        let mut result = Vec::with_capacity(4 + self.value.len());
        result.extend_from_slice(&len.to_le_bytes());
        result.extend_from_slice(self.value.as_bytes());
        result
    }

    /// Load from bytes
    pub fn load(&mut self, data: &[u8]) -> Result<()> {
        if data.len() < 4 {
            return Err(Error::Serialization("Not enough data for string length".into()));
        }
        let mut len_buf = [0u8; 4];
        len_buf.copy_from_slice(&data[..4]);
        let len = u32::from_le_bytes(len_buf) as usize;
        
        if data.len() < 4 + len {
            return Err(Error::Serialization("Not enough data for string content".into()));
        }
        
        self.value = String::from_utf8(data[4..4+len].to_vec())
            .map_err(|e| Error::Serialization(e.to_string()))?;
        Ok(())
    }
}

/// Group of net elements synchronized together
#[derive(Default)]
pub struct NetElementGroup {
    base: NetElementBase,
    bool_elements: Vec<NetElementBool>,
    int_elements: Vec<NetElementInt>,
    float_elements: Vec<NetElementFloat>,
    string_elements: Vec<NetElementString>,
}

impl NetElementGroup {
    /// Create a new group
    pub fn new() -> Self {
        Self::default()
    }

    /// Add a boolean element
    pub fn add_bool(&mut self, element: NetElementBool) -> usize {
        let idx = self.bool_elements.len();
        self.bool_elements.push(element);
        idx
    }

    /// Add an integer element
    pub fn add_int(&mut self, element: NetElementInt) -> usize {
        let idx = self.int_elements.len();
        self.int_elements.push(element);
        idx
    }

    /// Add a float element
    pub fn add_float(&mut self, element: NetElementFloat) -> usize {
        let idx = self.float_elements.len();
        self.float_elements.push(element);
        idx
    }

    /// Add a string element
    pub fn add_string(&mut self, element: NetElementString) -> usize {
        let idx = self.string_elements.len();
        self.string_elements.push(element);
        idx
    }

    /// Get a boolean element
    pub fn get_bool(&self, idx: usize) -> Option<&NetElementBool> {
        self.bool_elements.get(idx)
    }

    /// Get a boolean element mutably
    pub fn get_bool_mut(&mut self, idx: usize) -> Option<&mut NetElementBool> {
        self.bool_elements.get_mut(idx)
    }

    /// Get an integer element
    pub fn get_int(&self, idx: usize) -> Option<&NetElementInt> {
        self.int_elements.get(idx)
    }

    /// Get an integer element mutably
    pub fn get_int_mut(&mut self, idx: usize) -> Option<&mut NetElementInt> {
        self.int_elements.get_mut(idx)
    }

    /// Get a float element
    pub fn get_float(&self, idx: usize) -> Option<&NetElementFloat> {
        self.float_elements.get(idx)
    }

    /// Get a float element mutably
    pub fn get_float_mut(&mut self, idx: usize) -> Option<&mut NetElementFloat> {
        self.float_elements.get_mut(idx)
    }

    /// Get a string element
    pub fn get_string(&self, idx: usize) -> Option<&NetElementString> {
        self.string_elements.get(idx)
    }

    /// Get a string element mutably
    pub fn get_string_mut(&mut self, idx: usize) -> Option<&mut NetElementString> {
        self.string_elements.get_mut(idx)
    }

    /// Get total element count
    pub fn len(&self) -> usize {
        self.bool_elements.len() + 
        self.int_elements.len() + 
        self.float_elements.len() + 
        self.string_elements.len()
    }

    /// Check if empty
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Enable interpolation for all elements
    pub fn enable_interpolation(&mut self, extrapolation_hint: f32) {
        self.base.interpolation_enabled = true;
        self.base.extrapolation_hint = extrapolation_hint;
        for e in &mut self.bool_elements { e.enable_interpolation(extrapolation_hint); }
        for e in &mut self.int_elements { e.enable_interpolation(extrapolation_hint); }
        for e in &mut self.float_elements { e.enable_interpolation(extrapolation_hint); }
    }

    /// Disable interpolation for all elements
    pub fn disable_interpolation(&mut self) {
        self.base.interpolation_enabled = false;
        for e in &mut self.bool_elements { e.disable_interpolation(); }
        for e in &mut self.int_elements { e.disable_interpolation(); }
        for e in &mut self.float_elements { e.disable_interpolation(); }
    }

    /// Tick interpolation for all elements
    pub fn tick_interpolation(&mut self, dt: f32) {
        for e in &mut self.bool_elements { e.tick_interpolation(dt); }
        for e in &mut self.int_elements { e.tick_interpolation(dt); }
        for e in &mut self.float_elements { e.tick_interpolation(dt); }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_net_element_version() {
        let version = NetElementVersion::new();
        assert_eq!(version.current(), 0);

        assert_eq!(version.increment(), 1);
        assert_eq!(version.current(), 1);

        assert_eq!(version.increment(), 2);
        assert_eq!(version.current(), 2);
    }

    #[test]
    fn test_net_compatibility_rules() {
        let rules = NetCompatibilityRules::new(5);
        assert_eq!(rules.version(), 5);

        let any = NetCompatibilityRules::any();
        assert_eq!(any.version(), ANY_VERSION);
    }

    #[test]
    fn test_net_element_bool() {
        let version = NetElementVersion::new();
        let mut elem = NetElementBool::new(false);

        assert!(!elem.get());

        elem.set(true, &version);
        assert!(elem.get());

        // Store and load
        let data = elem.store();
        let mut elem2 = NetElementBool::new(false);
        elem2.load(&data).unwrap();
        assert!(elem2.get());
    }

    #[test]
    fn test_net_element_int() {
        let version = NetElementVersion::new();
        let mut elem = NetElementInt::new(0);

        assert_eq!(elem.get(), 0);

        elem.set(42, &version);
        assert_eq!(elem.get(), 42);

        // Store and load
        let data = elem.store();
        let mut elem2 = NetElementInt::new(0);
        elem2.load(&data).unwrap();
        assert_eq!(elem2.get(), 42);
    }

    #[test]
    fn test_net_element_float() {
        let version = NetElementVersion::new();
        let mut elem = NetElementFloat::new(0.0);

        assert!((elem.get() - 0.0).abs() < f64::EPSILON);

        elem.set(3.14159, &version);
        assert!((elem.get() - 3.14159).abs() < 0.00001);

        // Store and load
        let data = elem.store();
        let mut elem2 = NetElementFloat::new(0.0);
        elem2.load(&data).unwrap();
        assert!((elem2.get() - 3.14159).abs() < 0.00001);
    }

    #[test]
    fn test_net_element_string() {
        let version = NetElementVersion::new();
        let mut elem = NetElementString::new("");

        assert_eq!(elem.get(), "");

        elem.set("Hello, World!", &version);
        assert_eq!(elem.get(), "Hello, World!");

        // Store and load
        let data = elem.store();
        let mut elem2 = NetElementString::new("");
        elem2.load(&data).unwrap();
        assert_eq!(elem2.get(), "Hello, World!");
    }

    #[test]
    fn test_net_element_interpolation() {
        let mut elem = NetElementFloat::new(0.0);

        elem.enable_interpolation(1.0);
        elem.interpolated_value = Some(0.0);  // Start at 0
        elem.target_value = Some(100.0);
        elem.interpolation_time = 1.0;

        // Tick forward
        elem.tick_interpolation(0.5);
        let val = elem.get();
        assert!(val > 0.0 && val < 100.0, "Expected 0 < val < 100, got {}", val);

        // Tick to completion
        elem.tick_interpolation(0.6);
        assert!((elem.get() - 100.0).abs() < 0.1);
    }

    #[test]
    fn test_net_element_group() {
        let mut group = NetElementGroup::new();

        let bool_idx = group.add_bool(NetElementBool::new(true));
        let int_idx = group.add_int(NetElementInt::new(42));
        let float_idx = group.add_float(NetElementFloat::new(3.14));
        let string_idx = group.add_string(NetElementString::new("test"));

        assert_eq!(group.len(), 4);
        assert!(group.get_bool(bool_idx).unwrap().get());
        assert_eq!(group.get_int(int_idx).unwrap().get(), 42);
        assert!((group.get_float(float_idx).unwrap().get() - 3.14).abs() < 0.01);
        assert_eq!(group.get_string(string_idx).unwrap().get(), "test");
    }

    #[test]
    fn test_compatibility_version_check() {
        let mut base = NetElementBase::new();
        base.set_compatibility_version(5);

        assert!(base.check_with_rules(&NetCompatibilityRules::new(5)));
        assert!(base.check_with_rules(&NetCompatibilityRules::new(10)));
        assert!(!base.check_with_rules(&NetCompatibilityRules::new(4)));
    }

    #[test]
    fn test_any_version_compatibility() {
        let mut base = NetElementBase::new();
        base.set_compatibility_version(ANY_VERSION);

        assert!(base.check_with_rules(&NetCompatibilityRules::new(0)));
        assert!(base.check_with_rules(&NetCompatibilityRules::new(100)));
    }
}
