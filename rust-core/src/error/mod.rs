//! Error types for Starbound Core
//!
//! This module provides error handling types that mirror the C++ exception hierarchy.

use thiserror::Error;

/// Main error type for Starbound Core operations
#[derive(Error, Debug)]
pub enum Error {
    /// General Starbound error
    #[error("Starbound error: {0}")]
    Star(String),

    /// Color parsing or manipulation error
    #[error("Color error: {0}")]
    Color(String),

    /// Math operation error
    #[error("Math error: {0}")]
    Math(String),

    /// Serialization error
    #[error("Serialization error: {0}")]
    Serialization(String),

    /// Parse error
    #[error("Parse error: {0}")]
    Parse(String),

    /// Network error
    #[error("Network error: {0}")]
    Network(String),

    /// JSON parsing error
    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),

    /// I/O error
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),
}

impl Error {
    /// Create a parse error
    pub fn parse<S: Into<String>>(msg: S) -> Self {
        Error::Parse(msg.into())
    }

    /// Create a network error
    pub fn network<S: Into<String>>(msg: S) -> Self {
        Error::Network(msg.into())
    }
}

/// Result type alias for Starbound Core operations
pub type Result<T> = std::result::Result<T, Error>;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_error_display() {
        let err = Error::Star("test error".to_string());
        assert_eq!(format!("{}", err), "Starbound error: test error");
    }

    #[test]
    fn test_parse_error() {
        let err = Error::parse("invalid format");
        assert_eq!(format!("{}", err), "Parse error: invalid format");
    }
}
