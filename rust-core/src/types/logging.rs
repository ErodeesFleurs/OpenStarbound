//! Logging system compatible with C++ Star::Logger
//!
//! Provides multi-sink logging with Debug, Info, Warn, and Error levels.

use std::collections::HashMap;
use std::fs::{File, OpenOptions};
use std::io::{BufWriter, Write};
use std::sync::{Arc, Mutex, RwLock, OnceLock};

/// Log levels for the logging system
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
}

impl LogLevel {
    /// Parse a log level from string
    pub fn from_str(s: &str) -> Option<LogLevel> {
        match s.to_lowercase().as_str() {
            "debug" => Some(LogLevel::Debug),
            "info" => Some(LogLevel::Info),
            "warn" | "warning" => Some(LogLevel::Warn),
            "error" => Some(LogLevel::Error),
            _ => None,
        }
    }
    
    /// Get the string name of this log level
    pub fn name(&self) -> &'static str {
        match self {
            LogLevel::Debug => "Debug",
            LogLevel::Info => "Info",
            LogLevel::Warn => "Warn",
            LogLevel::Error => "Error",
        }
    }
}

impl std::fmt::Display for LogLevel {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// A sink for log messages
pub trait LogSink: Send + Sync {
    /// Log a message at the given level
    fn log(&self, msg: &str, level: LogLevel);
    
    /// Set the minimum log level for this sink
    fn set_level(&self, level: LogLevel);
    
    /// Get the current minimum log level
    fn level(&self) -> LogLevel;
}

/// Log sink that writes to stdout
pub struct StdoutLogSink {
    level: Mutex<LogLevel>,
}

impl StdoutLogSink {
    /// Create a new stdout log sink
    pub fn new() -> Self {
        StdoutLogSink {
            level: Mutex::new(LogLevel::Info),
        }
    }
}

impl Default for StdoutLogSink {
    fn default() -> Self {
        Self::new()
    }
}

impl LogSink for StdoutLogSink {
    fn log(&self, msg: &str, level: LogLevel) {
        if level >= *self.level.lock().unwrap() {
            let timestamp = chrono_lite_now();
            println!("[{}] [{}] {}", timestamp, level.name(), msg);
        }
    }
    
    fn set_level(&self, level: LogLevel) {
        *self.level.lock().unwrap() = level;
    }
    
    fn level(&self) -> LogLevel {
        *self.level.lock().unwrap()
    }
}

/// Log sink that writes to a file
pub struct FileLogSink {
    level: Mutex<LogLevel>,
    writer: Mutex<BufWriter<File>>,
}

impl FileLogSink {
    /// Create a new file log sink
    /// 
    /// If `truncate` is true, the file is truncated. Otherwise, logs are appended.
    pub fn new(filename: &str, level: LogLevel, truncate: bool) -> std::io::Result<Self> {
        let file = OpenOptions::new()
            .create(true)
            .write(true)
            .truncate(truncate)
            .append(!truncate)
            .open(filename)?;
        
        Ok(FileLogSink {
            level: Mutex::new(level),
            writer: Mutex::new(BufWriter::new(file)),
        })
    }
}

impl LogSink for FileLogSink {
    fn log(&self, msg: &str, level: LogLevel) {
        if level >= *self.level.lock().unwrap() {
            let timestamp = chrono_lite_now();
            let mut writer = self.writer.lock().unwrap();
            let _ = writeln!(writer, "[{}] [{}] {}", timestamp, level.name(), msg);
            let _ = writer.flush();
        }
    }
    
    fn set_level(&self, level: LogLevel) {
        *self.level.lock().unwrap() = level;
    }
    
    fn level(&self) -> LogLevel {
        *self.level.lock().unwrap()
    }
}

/// Simple timestamp function (no external chrono dependency)
fn chrono_lite_now() -> String {
    use std::time::{SystemTime, UNIX_EPOCH};
    
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default();
    
    let secs = now.as_secs();
    let millis = now.subsec_millis();
    
    // Simple formatting - just show seconds since epoch and millis
    // In a real implementation, you'd convert to a proper date/time
    format!("{}.{:03}", secs, millis)
}

type LogSinkPtr = Arc<dyn LogSink>;

/// Global logger that manages multiple log sinks
pub struct Logger {
    sinks: RwLock<Vec<LogSinkPtr>>,
    loggable: [std::sync::atomic::AtomicBool; 4],
}

impl Logger {
    fn new() -> Self {
        Logger {
            sinks: RwLock::new(Vec::new()),
            loggable: [
                std::sync::atomic::AtomicBool::new(true),
                std::sync::atomic::AtomicBool::new(true),
                std::sync::atomic::AtomicBool::new(true),
                std::sync::atomic::AtomicBool::new(true),
            ],
        }
    }
    
    fn global() -> &'static Logger {
        static LOGGER: OnceLock<Logger> = OnceLock::new();
        LOGGER.get_or_init(|| {
            let mut logger = Logger::new();
            // Add default stdout sink
            logger.sinks.write().unwrap().push(Arc::new(StdoutLogSink::new()));
            logger
        })
    }
    
    /// Add a log sink
    pub fn add_sink(sink: LogSinkPtr) {
        Self::global().sinks.write().unwrap().push(sink);
        Self::refresh_loggable();
    }
    
    /// Remove a log sink
    pub fn remove_sink(sink: &LogSinkPtr) {
        let mut sinks = Self::global().sinks.write().unwrap();
        sinks.retain(|s| !Arc::ptr_eq(s, sink));
        drop(sinks);
        Self::refresh_loggable();
    }
    
    /// Log a message at the given level
    pub fn log(level: LogLevel, msg: &str) {
        let logger = Self::global();
        if !logger.loggable[level as usize].load(std::sync::atomic::Ordering::Relaxed) {
            return;
        }
        
        let sinks = logger.sinks.read().unwrap();
        for sink in sinks.iter() {
            if sink.level() <= level {
                sink.log(msg, level);
            }
        }
    }
    
    /// Log a debug message
    pub fn debug(msg: &str) {
        Self::log(LogLevel::Debug, msg);
    }
    
    /// Log an info message
    pub fn info(msg: &str) {
        Self::log(LogLevel::Info, msg);
    }
    
    /// Log a warning message
    pub fn warn(msg: &str) {
        Self::log(LogLevel::Warn, msg);
    }
    
    /// Log an error message
    pub fn error(msg: &str) {
        Self::log(LogLevel::Error, msg);
    }
    
    /// Check if a given log level is loggable
    pub fn loggable(level: LogLevel) -> bool {
        Self::global().loggable[level as usize].load(std::sync::atomic::Ordering::Relaxed)
    }
    
    /// Refresh the loggable flags based on current sinks
    pub fn refresh_loggable() {
        let logger = Self::global();
        let sinks = logger.sinks.read().unwrap();
        
        for level_idx in 0..4 {
            let level = match level_idx {
                0 => LogLevel::Debug,
                1 => LogLevel::Info,
                2 => LogLevel::Warn,
                _ => LogLevel::Error,
            };
            
            let is_loggable = sinks.iter().any(|s| s.level() <= level);
            logger.loggable[level_idx].store(is_loggable, std::sync::atomic::Ordering::Relaxed);
        }
    }
}

/// Convenience macros for logging
#[macro_export]
macro_rules! log_debug {
    ($($arg:tt)*) => {
        $crate::types::logging::Logger::debug(&format!($($arg)*))
    };
}

#[macro_export]
macro_rules! log_info {
    ($($arg:tt)*) => {
        $crate::types::logging::Logger::info(&format!($($arg)*))
    };
}

#[macro_export]
macro_rules! log_warn {
    ($($arg:tt)*) => {
        $crate::types::logging::Logger::warn(&format!($($arg)*))
    };
}

#[macro_export]
macro_rules! log_error {
    ($($arg:tt)*) => {
        $crate::types::logging::Logger::error(&format!($($arg)*))
    };
}

/// A map for logging high-frequency debug values
pub struct LogMap {
    values: RwLock<HashMap<String, String>>,
}

impl LogMap {
    fn global() -> &'static LogMap {
        static LOG_MAP: OnceLock<LogMap> = OnceLock::new();
        LOG_MAP.get_or_init(|| LogMap {
            values: RwLock::new(HashMap::new()),
        })
    }
    
    /// Get a value from the log map
    pub fn get_value(key: &str) -> Option<String> {
        Self::global().values.read().unwrap().get(key).cloned()
    }
    
    /// Set a value in the log map
    pub fn set_value(key: &str, value: &str) {
        Self::global().values.write().unwrap().insert(key.to_string(), value.to_string());
    }
    
    /// Set a value using Display trait
    pub fn set<T: std::fmt::Display>(key: &str, value: &T) {
        Self::set_value(key, &value.to_string());
    }
    
    /// Get all values in the log map
    pub fn get_values() -> HashMap<String, String> {
        Self::global().values.read().unwrap().clone()
    }
    
    /// Clear all values in the log map
    pub fn clear() {
        Self::global().values.write().unwrap().clear();
    }
}

/// Spatial logger for debugging spatial data
/// 
/// Stores lines, points, and text for rendering in debug overlays.
pub struct SpatialLogger {
    lines: RwLock<HashMap<String, Vec<Line>>>,
    points: RwLock<HashMap<String, Vec<Point>>>,
    text: RwLock<HashMap<String, Vec<LogText>>>,
    observed: std::sync::atomic::AtomicBool,
}

/// A line for spatial logging
#[derive(Debug, Clone)]
pub struct Line {
    pub begin: [f32; 2],
    pub end: [f32; 2],
    pub color: [u8; 4],
}

/// A point for spatial logging
#[derive(Debug, Clone)]
pub struct Point {
    pub position: [f32; 2],
    pub color: [u8; 4],
}

/// Text for spatial logging
#[derive(Debug, Clone)]
pub struct LogText {
    pub text: String,
    pub position: [f32; 2],
    pub color: [u8; 4],
}

/// Maximum number of items per space
pub const MAX_LINES: usize = 200000;
pub const MAX_POINTS: usize = 200000;
pub const MAX_TEXT: usize = 10000;

impl SpatialLogger {
    fn global() -> &'static SpatialLogger {
        static SPATIAL: OnceLock<SpatialLogger> = OnceLock::new();
        SPATIAL.get_or_init(|| SpatialLogger {
            lines: RwLock::new(HashMap::new()),
            points: RwLock::new(HashMap::new()),
            text: RwLock::new(HashMap::new()),
            observed: std::sync::atomic::AtomicBool::new(false),
        })
    }
    
    /// Log a line in the given space
    pub fn log_line(space: &str, begin: [f32; 2], end: [f32; 2], color: [u8; 4]) {
        if !Self::observed() {
            return;
        }
        
        let mut lines = Self::global().lines.write().unwrap();
        let space_lines = lines.entry(space.to_string()).or_insert_with(Vec::new);
        
        if space_lines.len() < MAX_LINES {
            space_lines.push(Line { begin, end, color });
        }
    }
    
    /// Log a point in the given space
    pub fn log_point(space: &str, position: [f32; 2], color: [u8; 4]) {
        if !Self::observed() {
            return;
        }
        
        let mut points = Self::global().points.write().unwrap();
        let space_points = points.entry(space.to_string()).or_insert_with(Vec::new);
        
        if space_points.len() < MAX_POINTS {
            space_points.push(Point { position, color });
        }
    }
    
    /// Log text in the given space
    pub fn log_text(space: &str, text: String, position: [f32; 2], color: [u8; 4]) {
        if !Self::observed() {
            return;
        }
        
        let mut texts = Self::global().text.write().unwrap();
        let space_text = texts.entry(space.to_string()).or_insert_with(Vec::new);
        
        if space_text.len() < MAX_TEXT {
            space_text.push(LogText { text, position, color });
        }
    }
    
    /// Get and optionally clear lines for a space
    pub fn get_lines(space: &str, and_clear: bool) -> Vec<Line> {
        let mut lines = Self::global().lines.write().unwrap();
        if and_clear {
            lines.remove(space).unwrap_or_default()
        } else {
            lines.get(space).cloned().unwrap_or_default()
        }
    }
    
    /// Get and optionally clear points for a space
    pub fn get_points(space: &str, and_clear: bool) -> Vec<Point> {
        let mut points = Self::global().points.write().unwrap();
        if and_clear {
            points.remove(space).unwrap_or_default()
        } else {
            points.get(space).cloned().unwrap_or_default()
        }
    }
    
    /// Get and optionally clear text for a space
    pub fn get_text(space: &str, and_clear: bool) -> Vec<LogText> {
        let mut text = Self::global().text.write().unwrap();
        if and_clear {
            text.remove(space).unwrap_or_default()
        } else {
            text.get(space).cloned().unwrap_or_default()
        }
    }
    
    /// Clear all spatial log data
    pub fn clear() {
        Self::global().lines.write().unwrap().clear();
        Self::global().points.write().unwrap().clear();
        Self::global().text.write().unwrap().clear();
    }
    
    /// Check if the spatial logger is being observed
    pub fn observed() -> bool {
        Self::global().observed.load(std::sync::atomic::Ordering::Relaxed)
    }
    
    /// Set whether the spatial logger is being observed
    pub fn set_observed(observed: bool) {
        Self::global().observed.store(observed, std::sync::atomic::Ordering::Relaxed);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_log_level_ordering() {
        assert!(LogLevel::Debug < LogLevel::Info);
        assert!(LogLevel::Info < LogLevel::Warn);
        assert!(LogLevel::Warn < LogLevel::Error);
    }
    
    #[test]
    fn test_log_level_from_str() {
        assert_eq!(LogLevel::from_str("debug"), Some(LogLevel::Debug));
        assert_eq!(LogLevel::from_str("INFO"), Some(LogLevel::Info));
        assert_eq!(LogLevel::from_str("Warning"), Some(LogLevel::Warn));
        assert_eq!(LogLevel::from_str("error"), Some(LogLevel::Error));
        assert_eq!(LogLevel::from_str("invalid"), None);
    }
    
    #[test]
    fn test_log_map() {
        LogMap::set_value("test_key", "test_value");
        assert_eq!(LogMap::get_value("test_key"), Some("test_value".to_string()));
        
        LogMap::set("test_num", &42);
        assert_eq!(LogMap::get_value("test_num"), Some("42".to_string()));
        
        LogMap::clear();
        assert_eq!(LogMap::get_value("test_key"), None);
    }
    
    #[test]
    fn test_spatial_logger() {
        SpatialLogger::set_observed(true);
        
        SpatialLogger::log_line("test", [0.0, 0.0], [1.0, 1.0], [255, 0, 0, 255]);
        SpatialLogger::log_point("test", [0.5, 0.5], [0, 255, 0, 255]);
        SpatialLogger::log_text("test", "Hello".to_string(), [0.0, 0.0], [0, 0, 255, 255]);
        
        let lines = SpatialLogger::get_lines("test", false);
        assert_eq!(lines.len(), 1);
        
        let points = SpatialLogger::get_points("test", true);
        assert_eq!(points.len(), 1);
        
        // Points should be cleared
        let points = SpatialLogger::get_points("test", false);
        assert_eq!(points.len(), 0);
        
        SpatialLogger::clear();
        SpatialLogger::set_observed(false);
    }
    
    #[test]
    fn test_stdout_sink() {
        let sink = StdoutLogSink::new();
        assert_eq!(sink.level(), LogLevel::Info);
        
        sink.set_level(LogLevel::Debug);
        assert_eq!(sink.level(), LogLevel::Debug);
    }
}
