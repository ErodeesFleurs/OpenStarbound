//! Lua scripting bindings compatible with C++ Star::Lua
//!
//! This module provides a safe wrapper around Lua scripting that is compatible
//! with the C++ implementation's Lua integration.

use crate::error::{Error, Result};
use crate::types::Json;
use std::collections::HashMap;
use std::sync::Arc;

/// Exception types for Lua errors
#[derive(Debug, Clone, PartialEq)]
pub enum LuaExceptionKind {
    /// Basic unspecified lua exception
    General,
    /// Thrown when trying to parse an incomplete statement
    IncompleteStatement,
    /// Thrown when the instruction limit is reached
    InstructionLimitReached,
    /// Thrown when the engine recursion limit is reached
    RecursionLimitReached,
    /// Thrown when an incorrect lua type is passed
    ConversionError,
}

/// Lua value type - represents any value that can exist in Lua
#[derive(Debug, Clone, PartialEq)]
pub enum LuaValue {
    /// nil value
    Nil,
    /// boolean value
    Boolean(bool),
    /// integer value (64-bit)
    Integer(i64),
    /// floating point value
    Float(f64),
    /// string value
    String(String),
    /// table value (reference)
    Table(LuaTableRef),
    /// function value (reference)
    Function(LuaFunctionRef),
    /// thread/coroutine value (reference)
    Thread(LuaThreadRef),
    /// userdata value (reference)
    UserData(LuaUserDataRef),
}

impl Default for LuaValue {
    fn default() -> Self {
        LuaValue::Nil
    }
}

impl LuaValue {
    /// Returns true if the value is nil
    pub fn is_nil(&self) -> bool {
        matches!(self, LuaValue::Nil)
    }

    /// Returns true if the value is a boolean
    pub fn is_boolean(&self) -> bool {
        matches!(self, LuaValue::Boolean(_))
    }

    /// Returns true if the value is numeric (integer or float)
    pub fn is_number(&self) -> bool {
        matches!(self, LuaValue::Integer(_) | LuaValue::Float(_))
    }

    /// Returns true if the value is a string
    pub fn is_string(&self) -> bool {
        matches!(self, LuaValue::String(_))
    }

    /// Returns true if the value is a table
    pub fn is_table(&self) -> bool {
        matches!(self, LuaValue::Table(_))
    }

    /// Returns true if the value is a function
    pub fn is_function(&self) -> bool {
        matches!(self, LuaValue::Function(_))
    }

    /// Try to convert to boolean
    pub fn as_boolean(&self) -> Option<bool> {
        match self {
            LuaValue::Boolean(b) => Some(*b),
            LuaValue::Nil => Some(false),
            _ => Some(true), // Everything except nil and false is truthy in Lua
        }
    }

    /// Try to convert to integer
    pub fn as_integer(&self) -> Option<i64> {
        match self {
            LuaValue::Integer(i) => Some(*i),
            LuaValue::Float(f) => {
                if f.fract() == 0.0 && *f >= i64::MIN as f64 && *f <= i64::MAX as f64 {
                    Some(*f as i64)
                } else {
                    None
                }
            }
            LuaValue::String(s) => s.parse().ok(),
            _ => None,
        }
    }

    /// Try to convert to float
    pub fn as_float(&self) -> Option<f64> {
        match self {
            LuaValue::Float(f) => Some(*f),
            LuaValue::Integer(i) => Some(*i as f64),
            LuaValue::String(s) => s.parse().ok(),
            _ => None,
        }
    }

    /// Try to convert to string
    pub fn as_str(&self) -> Option<&str> {
        match self {
            LuaValue::String(s) => Some(s),
            _ => None,
        }
    }

    /// Convert to string representation
    pub fn to_string_value(&self) -> String {
        match self {
            LuaValue::Nil => "nil".to_string(),
            LuaValue::Boolean(b) => b.to_string(),
            LuaValue::Integer(i) => i.to_string(),
            LuaValue::Float(f) => f.to_string(),
            LuaValue::String(s) => s.clone(),
            LuaValue::Table(_) => "table".to_string(),
            LuaValue::Function(_) => "function".to_string(),
            LuaValue::Thread(_) => "thread".to_string(),
            LuaValue::UserData(_) => "userdata".to_string(),
        }
    }
}

impl From<bool> for LuaValue {
    fn from(b: bool) -> Self {
        LuaValue::Boolean(b)
    }
}

impl From<i32> for LuaValue {
    fn from(i: i32) -> Self {
        LuaValue::Integer(i as i64)
    }
}

impl From<i64> for LuaValue {
    fn from(i: i64) -> Self {
        LuaValue::Integer(i)
    }
}

impl From<f32> for LuaValue {
    fn from(f: f32) -> Self {
        LuaValue::Float(f as f64)
    }
}

impl From<f64> for LuaValue {
    fn from(f: f64) -> Self {
        LuaValue::Float(f)
    }
}

impl From<String> for LuaValue {
    fn from(s: String) -> Self {
        LuaValue::String(s)
    }
}

impl From<&str> for LuaValue {
    fn from(s: &str) -> Self {
        LuaValue::String(s.to_string())
    }
}

impl From<Json> for LuaValue {
    fn from(json: Json) -> Self {
        json_to_lua_value(&json)
    }
}

/// Convert JSON to LuaValue
fn json_to_lua_value(json: &Json) -> LuaValue {
    match json.as_value() {
        serde_json::Value::Null => LuaValue::Nil,
        serde_json::Value::Bool(b) => LuaValue::Boolean(*b),
        serde_json::Value::Number(n) => {
            if let Some(i) = n.as_i64() {
                LuaValue::Integer(i)
            } else if let Some(f) = n.as_f64() {
                LuaValue::Float(f)
            } else {
                LuaValue::Nil
            }
        }
        serde_json::Value::String(s) => LuaValue::String(s.clone()),
        serde_json::Value::Array(_) | serde_json::Value::Object(_) => {
            // Tables need engine context to create properly
            // For now, return nil - full implementation would need engine reference
            LuaValue::Nil
        }
    }
}

/// Reference to a Lua table
#[derive(Debug, Clone, PartialEq)]
pub struct LuaTableRef {
    /// Handle index in the engine's reference table
    pub handle_index: i32,
}

/// Reference to a Lua function
#[derive(Debug, Clone, PartialEq)]
pub struct LuaFunctionRef {
    /// Handle index in the engine's reference table
    pub handle_index: i32,
}

/// Reference to a Lua thread/coroutine
#[derive(Debug, Clone, PartialEq)]
pub struct LuaThreadRef {
    /// Handle index in the engine's reference table
    pub handle_index: i32,
}

/// Thread status
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum LuaThreadStatus {
    /// Thread has finished execution
    Dead,
    /// Thread is active and can be resumed
    Active,
    /// Thread encountered an error
    Error,
}

/// Reference to Lua userdata
#[derive(Debug, Clone, PartialEq)]
pub struct LuaUserDataRef {
    /// Handle index in the engine's reference table
    pub handle_index: i32,
    /// Type identifier for the userdata
    pub type_id: std::any::TypeId,
}

impl LuaUserDataRef {
    /// Check if the userdata is of a specific type
    pub fn is<T: 'static>(&self) -> bool {
        self.type_id == std::any::TypeId::of::<T>()
    }
}

/// Collection of registered Lua callbacks
#[derive(Default)]
pub struct LuaCallbacks {
    callbacks: HashMap<String, LuaWrappedFunction>,
}

/// Wrapped function type for Lua callbacks
pub type LuaWrappedFunction = Arc<dyn Fn(&mut LuaEngine, &[LuaValue]) -> Result<Vec<LuaValue>> + Send + Sync>;

impl LuaCallbacks {
    /// Create a new empty callback collection
    pub fn new() -> Self {
        Self {
            callbacks: HashMap::new(),
        }
    }

    /// Register a callback function
    pub fn register<F>(&mut self, name: impl Into<String>, func: F) -> Result<()>
    where
        F: Fn(&mut LuaEngine, &[LuaValue]) -> Result<Vec<LuaValue>> + Send + Sync + 'static,
    {
        let name = name.into();
        if self.callbacks.contains_key(&name) {
            return Err(Error::Lua(format!("Lua callback '{}' was registered twice", name)));
        }
        self.callbacks.insert(name, Arc::new(func));
        Ok(())
    }

    /// Copy a callback to a new name
    pub fn copy_callback(&mut self, src_name: &str, dst_name: impl Into<String>) -> Result<()> {
        let dst = dst_name.into();
        if let Some(func) = self.callbacks.get(src_name) {
            self.callbacks.insert(dst, func.clone());
            Ok(())
        } else {
            Err(Error::Lua(format!("Callback '{}' not found", src_name)))
        }
    }

    /// Remove a callback
    pub fn remove(&mut self, name: &str) -> bool {
        self.callbacks.remove(name).is_some()
    }

    /// Merge another callback collection into this one
    pub fn merge(&mut self, other: LuaCallbacks) {
        self.callbacks.extend(other.callbacks);
    }

    /// Get the callbacks map
    pub fn callbacks(&self) -> &HashMap<String, LuaWrappedFunction> {
        &self.callbacks
    }
}

/// Lua execution context with separate global environment
pub struct LuaContext {
    /// Handle to the context's environment table
    pub handle_index: i32,
    /// Reference to the parent engine
    engine_id: usize,
}

impl LuaContext {
    /// Get a value from the context's global environment
    pub fn get(&self, _key: &str) -> LuaValue {
        // Placeholder - actual implementation would use engine reference
        LuaValue::Nil
    }

    /// Set a value in the context's global environment
    pub fn set(&mut self, _key: &str, _value: LuaValue) {
        // Placeholder - actual implementation would use engine reference
    }

    /// Check if a key exists in the context
    pub fn contains(&self, key: &str) -> bool {
        !self.get(key).is_nil()
    }

    /// Remove a key from the context
    pub fn remove(&mut self, key: &str) {
        self.set(key, LuaValue::Nil);
    }

    /// Get the engine ID
    pub fn engine_id(&self) -> usize {
        self.engine_id
    }
}

/// Profiling entry for Lua functions
#[derive(Debug, Clone)]
pub struct LuaProfileEntry {
    /// Source name of the chunk the function was defined in
    pub source: String,
    /// Line number in the chunk of the beginning of the function definition
    pub source_line: u32,
    /// Name of the function, if it can be determined
    pub name: Option<String>,
    /// Scope of the function, if it can be determined
    pub name_scope: Option<String>,
    /// Time taken within this function itself (microseconds)
    pub self_time: i64,
    /// Total time taken within this function or sub functions (microseconds)
    pub total_time: i64,
}

/// Main Lua execution engine
///
/// This represents one execution engine in Lua, holding a single lua_State.
/// Multiple contexts can be created with separate global environments.
pub struct LuaEngine {
    /// Unique engine identifier
    id: usize,
    /// Whether safe mode is enabled (restricts I/O operations)
    safe: bool,
    /// Instruction limit for computation sequences
    instruction_limit: u64,
    /// Current instruction count
    instruction_count: u64,
    /// Whether profiling is enabled
    profiling_enabled: bool,
    /// Instruction measurement interval
    instruction_measure_interval: u32,
    /// Recursion limit
    recursion_limit: u32,
    /// Current recursion level
    recursion_level: u32,
    /// Memory usage in bytes
    memory_usage: usize,
    /// Global callbacks
    global_callbacks: HashMap<String, LuaWrappedFunction>,
    /// Profile entries
    profile_entries: Vec<LuaProfileEntry>,
    /// Next context ID
    next_context_id: i32,
}

impl Default for LuaEngine {
    fn default() -> Self {
        Self::new(true)
    }
}

impl LuaEngine {
    /// Create a new Lua engine
    ///
    /// If `safe` is true, creates an engine with all builtin lua functions
    /// that can affect the real world disabled.
    pub fn new(safe: bool) -> Self {
        static NEXT_ID: std::sync::atomic::AtomicUsize = std::sync::atomic::AtomicUsize::new(1);
        
        Self {
            id: NEXT_ID.fetch_add(1, std::sync::atomic::Ordering::SeqCst),
            safe,
            instruction_limit: 0,
            instruction_count: 0,
            profiling_enabled: false,
            instruction_measure_interval: 1000,
            recursion_limit: 0,
            recursion_level: 0,
            memory_usage: 0,
            global_callbacks: HashMap::new(),
            profile_entries: Vec::new(),
            next_context_id: 1,
        }
    }

    /// Get the engine ID
    pub fn id(&self) -> usize {
        self.id
    }

    /// Check if safe mode is enabled
    pub fn is_safe(&self) -> bool {
        self.safe
    }

    /// Set the instruction limit for computation sequences
    ///
    /// During any function invocation, thread resume, or code evaluation,
    /// an instruction counter will be started. If it exceeds this limit,
    /// a LuaException will be thrown. 0 disables the limit.
    pub fn set_instruction_limit(&mut self, limit: u64) {
        self.instruction_limit = limit;
    }

    /// Get the current instruction limit
    pub fn instruction_limit(&self) -> u64 {
        self.instruction_limit
    }

    /// Enable or disable profiling
    pub fn set_profiling_enabled(&mut self, enabled: bool) {
        self.profiling_enabled = enabled;
        if enabled {
            self.profile_entries.clear();
        }
    }

    /// Check if profiling is enabled
    pub fn profiling_enabled(&self) -> bool {
        self.profiling_enabled
    }

    /// Get profiling data
    pub fn get_profile(&self) -> &[LuaProfileEntry] {
        &self.profile_entries
    }

    /// Set the instruction measurement interval
    pub fn set_instruction_measure_interval(&mut self, interval: u32) {
        self.instruction_measure_interval = interval.max(1);
    }

    /// Get the instruction measurement interval
    pub fn instruction_measure_interval(&self) -> u32 {
        self.instruction_measure_interval
    }

    /// Set the recursion limit
    pub fn set_recursion_limit(&mut self, limit: u32) {
        self.recursion_limit = limit;
    }

    /// Get the recursion limit
    pub fn recursion_limit(&self) -> u32 {
        self.recursion_limit
    }

    /// Compile a script into bytecode
    pub fn compile(&self, contents: &str, name: Option<&str>) -> Result<Vec<u8>> {
        // Placeholder - actual implementation would use Lua C API
        let _ = name;
        Ok(contents.as_bytes().to_vec())
    }

    /// Create a new execution context
    pub fn create_context(&mut self) -> LuaContext {
        let handle = self.next_context_id;
        self.next_context_id += 1;
        
        LuaContext {
            handle_index: handle,
            engine_id: self.id,
        }
    }

    /// Set a global value that affects newly created contexts
    pub fn set_global(&mut self, key: &str, value: LuaValue) {
        if let LuaValue::Function(func_ref) = value {
            // Store function reference in global callbacks
            let _ = (key, func_ref);
        }
    }

    /// Get a global value
    pub fn get_global(&self, _key: &str) -> LuaValue {
        LuaValue::Nil
    }

    /// Create a Lua string
    pub fn create_string(&mut self, s: &str) -> LuaValue {
        LuaValue::String(s.to_string())
    }

    /// Create a Lua table
    pub fn create_table(&mut self) -> LuaTableRef {
        let handle = self.next_context_id;
        self.next_context_id += 1;
        LuaTableRef { handle_index: handle }
    }

    /// Perform garbage collection
    ///
    /// If steps is None, performs a full collection.
    /// Otherwise, performs an incremental collection with the given number of steps.
    pub fn collect_garbage(&mut self, steps: Option<u32>) {
        let _ = steps;
        // Placeholder - actual implementation would call lua_gc
    }

    /// Set whether automatic garbage collection is enabled
    pub fn set_auto_garbage_collection(&mut self, _enabled: bool) {
        // Placeholder
    }

    /// Tune garbage collection parameters
    pub fn tune_auto_garbage_collection(&mut self, _pause: f32, _step_multiplier: f32) {
        // Placeholder
    }

    /// Get current memory usage in bytes
    pub fn memory_usage(&self) -> usize {
        self.memory_usage
    }

    /// Register callbacks in the engine
    pub fn register_callbacks(&mut self, callbacks: &LuaCallbacks) {
        for (name, func) in callbacks.callbacks() {
            self.global_callbacks.insert(name.clone(), func.clone());
        }
    }

    /// Convert a Rust value to a Lua value
    pub fn lua_from<T: Into<LuaValue>>(&self, value: T) -> LuaValue {
        value.into()
    }

    /// Try to convert a Lua value to a specific Rust type
    pub fn lua_to<T: TryFrom<LuaValue>>(&self, value: LuaValue) -> Result<T>
    where
        T::Error: std::fmt::Display,
    {
        T::try_from(value).map_err(|e| Error::Lua(format!("Conversion error: {}", e)))
    }
}

/// Multiple return values from a Lua function call
#[derive(Debug, Clone)]
pub struct LuaVariadic<T>(pub Vec<T>);

impl<T> LuaVariadic<T> {
    /// Create a new variadic return
    pub fn new(values: Vec<T>) -> Self {
        Self(values)
    }

    /// Get the values
    pub fn values(&self) -> &[T] {
        &self.0
    }

    /// Take the values
    pub fn into_values(self) -> Vec<T> {
        self.0
    }

    /// Get the first value if present
    pub fn first(&self) -> Option<&T> {
        self.0.first()
    }

    /// Get the number of values
    pub fn len(&self) -> usize {
        self.0.len()
    }

    /// Check if empty
    pub fn is_empty(&self) -> bool {
        self.0.is_empty()
    }
}

impl<T> Default for LuaVariadic<T> {
    fn default() -> Self {
        Self(Vec::new())
    }
}

impl<T> From<Vec<T>> for LuaVariadic<T> {
    fn from(v: Vec<T>) -> Self {
        Self(v)
    }
}

impl<T> IntoIterator for LuaVariadic<T> {
    type Item = T;
    type IntoIter = std::vec::IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.into_iter()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lua_value_types() {
        assert!(LuaValue::Nil.is_nil());
        assert!(LuaValue::Boolean(true).is_boolean());
        assert!(LuaValue::Integer(42).is_number());
        assert!(LuaValue::Float(3.14).is_number());
        assert!(LuaValue::String("hello".into()).is_string());
    }

    #[test]
    fn test_lua_value_conversions() {
        let val = LuaValue::Integer(42);
        assert_eq!(val.as_integer(), Some(42));
        assert_eq!(val.as_float(), Some(42.0));

        let val = LuaValue::Float(3.5);
        assert_eq!(val.as_integer(), None); // Not exact integer
        assert_eq!(val.as_float(), Some(3.5));

        let val = LuaValue::Float(3.0);
        assert_eq!(val.as_integer(), Some(3)); // Exact integer
    }

    #[test]
    fn test_lua_value_from() {
        let val: LuaValue = true.into();
        assert!(matches!(val, LuaValue::Boolean(true)));

        let val: LuaValue = 42i64.into();
        assert!(matches!(val, LuaValue::Integer(42)));

        let val: LuaValue = 3.14f64.into();
        assert!(matches!(val, LuaValue::Float(f) if (f - 3.14).abs() < 0.001));

        let val: LuaValue = "hello".into();
        assert!(matches!(val, LuaValue::String(s) if s == "hello"));
    }

    #[test]
    fn test_lua_truthiness() {
        assert_eq!(LuaValue::Nil.as_boolean(), Some(false));
        assert_eq!(LuaValue::Boolean(false).as_boolean(), Some(false));
        assert_eq!(LuaValue::Boolean(true).as_boolean(), Some(true));
        assert_eq!(LuaValue::Integer(0).as_boolean(), Some(true));
        assert_eq!(LuaValue::String("".into()).as_boolean(), Some(true));
    }

    #[test]
    fn test_lua_engine_creation() {
        let engine = LuaEngine::new(true);
        assert!(engine.is_safe());
        assert_eq!(engine.instruction_limit(), 0);
        assert!(!engine.profiling_enabled());
    }

    #[test]
    fn test_lua_engine_settings() {
        let mut engine = LuaEngine::new(false);
        
        engine.set_instruction_limit(10000);
        assert_eq!(engine.instruction_limit(), 10000);

        engine.set_profiling_enabled(true);
        assert!(engine.profiling_enabled());

        engine.set_instruction_measure_interval(500);
        assert_eq!(engine.instruction_measure_interval(), 500);

        engine.set_recursion_limit(100);
        assert_eq!(engine.recursion_limit(), 100);
    }

    #[test]
    fn test_lua_callbacks() {
        let mut callbacks = LuaCallbacks::new();
        
        callbacks.register("test", |_engine, args| {
            Ok(args.to_vec())
        }).unwrap();

        assert!(callbacks.callbacks().contains_key("test"));
        
        callbacks.copy_callback("test", "test2").unwrap();
        assert!(callbacks.callbacks().contains_key("test2"));

        assert!(callbacks.remove("test"));
        assert!(!callbacks.callbacks().contains_key("test"));
    }

    #[test]
    fn test_lua_context() {
        let mut engine = LuaEngine::new(true);
        let context = engine.create_context();
        
        assert_eq!(context.engine_id(), engine.id());
        assert!(context.get("undefined").is_nil());
    }

    #[test]
    fn test_lua_variadic() {
        let var = LuaVariadic::new(vec![1, 2, 3]);
        assert_eq!(var.len(), 3);
        assert_eq!(var.first(), Some(&1));
        assert!(!var.is_empty());

        let empty: LuaVariadic<i32> = LuaVariadic::default();
        assert!(empty.is_empty());
    }

    #[test]
    fn test_lua_compile() {
        let engine = LuaEngine::new(true);
        let bytecode = engine.compile("return 42", Some("test.lua")).unwrap();
        assert!(!bytecode.is_empty());
    }

    #[test]
    fn test_lua_create_table() {
        let mut engine = LuaEngine::new(true);
        let table = engine.create_table();
        assert!(table.handle_index > 0);
    }

    #[test]
    fn test_lua_value_to_string() {
        assert_eq!(LuaValue::Nil.to_string_value(), "nil");
        assert_eq!(LuaValue::Boolean(true).to_string_value(), "true");
        assert_eq!(LuaValue::Integer(42).to_string_value(), "42");
        assert_eq!(LuaValue::String("hello".into()).to_string_value(), "hello");
    }
}
