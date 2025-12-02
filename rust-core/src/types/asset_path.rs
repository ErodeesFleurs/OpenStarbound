//! Asset path type compatible with C++ Star::AssetPath
//!
//! Asset paths are not filesystem paths. '/' is always the directory separator,
//! and it is not possible to escape any asset source directory. '\' is never a
//! valid directory separator. All asset paths are considered case-insensitive.

use serde::{Deserialize, Serialize};
use std::fmt;

/// An asset path with optional sub-path and directives
#[derive(Clone, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
pub struct AssetPath {
    /// The base path to the asset
    pub base_path: String,
    /// Optional sub-path (separated by ':')
    pub sub_path: Option<String>,
    /// Directives string (everything after '?')
    pub directives: String,
}

impl AssetPath {
    /// Create a new empty asset path
    pub fn new() -> Self {
        Self::default()
    }

    /// Create an asset path from a base path
    pub fn from_base<S: Into<String>>(base: S) -> Self {
        Self {
            base_path: base.into(),
            sub_path: None,
            directives: String::new(),
        }
    }

    /// Parse an asset path from a combined path string
    pub fn split(path: &str) -> Self {
        // Find directives (everything after first '?')
        let (path_part, directives_part) = match path.find('?') {
            Some(idx) => (&path[..idx], &path[idx..]),
            None => (path, ""),
        };
        let directives = directives_part.to_string();

        // Find sub-path (after ':')
        let (base_path, sub_path) = match path_part.find(':') {
            Some(idx) => {
                (path_part[..idx].to_string(), Some(path_part[idx + 1..].to_string()))
            }
            None => {
                (path_part.to_string(), None)
            }
        };

        Self {
            base_path,
            sub_path,
            directives,
        }
    }

    /// Join the path components into a single string
    pub fn join(&self) -> String {
        let mut result = self.base_path.clone();
        if let Some(ref sub) = self.sub_path {
            result.push(':');
            result.push_str(sub);
        }
        result.push_str(&self.directives);
        result
    }

    /// Set the sub-path on a joined path string
    pub fn set_sub_path_str(joined_path: &str, sub_path: &str) -> String {
        let mut path = Self::split(joined_path);
        path.sub_path = if sub_path.is_empty() {
            None
        } else {
            Some(sub_path.to_string())
        };
        path.join()
    }

    /// Remove the sub-path from a joined path string
    pub fn remove_sub_path_str(joined_path: &str) -> String {
        let mut path = Self::split(joined_path);
        path.sub_path = None;
        path.join()
    }

    /// Get directives from a joined path string
    pub fn get_directives_str(joined_path: &str) -> String {
        let path = Self::split(joined_path);
        path.directives
    }

    /// Add directives to a joined path string
    pub fn add_directives_str(joined_path: &str, directives: &str) -> String {
        let mut path = Self::split(joined_path);
        path.directives.push_str(directives);
        path.join()
    }

    /// Remove directives from a joined path string
    pub fn remove_directives_str(joined_path: &str) -> String {
        let mut path = Self::split(joined_path);
        path.directives.clear();
        path.join()
    }

    /// Get the directory portion of a path (with trailing '/')
    pub fn directory(path: &str) -> String {
        let path = Self::split(path);
        match path.base_path.rfind('/') {
            Some(idx) => path.base_path[..=idx].to_string(),
            None => String::new(),
        }
    }

    /// Get the filename portion of a path
    pub fn filename(path: &str) -> String {
        let path = Self::split(path);
        match path.base_path.rfind('/') {
            Some(idx) => path.base_path[idx + 1..].to_string(),
            None => path.base_path,
        }
    }

    /// Get the file extension of a path
    pub fn extension(path: &str) -> String {
        let filename = Self::filename(path);
        match filename.rfind('.') {
            Some(idx) => filename[idx + 1..].to_string(),
            None => String::new(),
        }
    }

    /// Compute an absolute asset path from a relative path
    pub fn relative_to(source_path: &str, given_path: &str) -> String {
        // If given path is absolute (starts with '/'), return as-is
        if given_path.starts_with('/') {
            return given_path.to_string();
        }

        // Get directory of source path
        let dir = Self::directory(source_path);
        
        // Combine and normalize
        let mut combined = dir + given_path;
        combined = Self::normalize(&combined);
        combined
    }

    /// Normalize a path (resolve '..' and '.')
    fn normalize(path: &str) -> String {
        let mut parts: Vec<&str> = Vec::new();
        
        for part in path.split('/') {
            match part {
                "" | "." => continue,
                ".." => {
                    parts.pop();
                }
                _ => parts.push(part),
            }
        }

        let result = parts.join("/");
        if path.starts_with('/') {
            format!("/{}", result)
        } else {
            result
        }
    }

    /// Check if this is an empty/unset path
    pub fn is_empty(&self) -> bool {
        self.base_path.is_empty()
    }

    /// Set the base path
    pub fn set_base_path<S: Into<String>>(&mut self, path: S) {
        self.base_path = path.into();
    }

    /// Set the sub-path
    pub fn set_sub_path<S: Into<String>>(&mut self, path: Option<S>) {
        self.sub_path = path.map(|s| s.into());
    }

    /// Set the directives
    pub fn set_directives<S: Into<String>>(&mut self, directives: S) {
        self.directives = directives.into();
    }

    /// Add a directive
    pub fn add_directive<S: AsRef<str>>(&mut self, directive: S) {
        if self.directives.is_empty() {
            self.directives.push('?');
        } else if !self.directives.ends_with('?') {
            self.directives.push('?');
        }
        self.directives.push_str(directive.as_ref());
    }
}

impl fmt::Display for AssetPath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.join())
    }
}

impl fmt::Debug for AssetPath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("AssetPath")
            .field("base_path", &self.base_path)
            .field("sub_path", &self.sub_path)
            .field("directives", &self.directives)
            .finish()
    }
}

impl From<&str> for AssetPath {
    fn from(s: &str) -> Self {
        Self::split(s)
    }
}

impl From<String> for AssetPath {
    fn from(s: String) -> Self {
        Self::split(&s)
    }
}

impl From<AssetPath> for String {
    fn from(path: AssetPath) -> Self {
        path.join()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_split_simple() {
        let path = AssetPath::split("/assets/image.png");
        assert_eq!(path.base_path, "/assets/image.png");
        assert_eq!(path.sub_path, None);
        assert_eq!(path.directives, "");
    }

    #[test]
    fn test_split_with_subpath() {
        let path = AssetPath::split("/assets/image.png:default");
        assert_eq!(path.base_path, "/assets/image.png");
        assert_eq!(path.sub_path, Some("default".to_string()));
        assert_eq!(path.directives, "");
    }

    #[test]
    fn test_split_with_directives() {
        let path = AssetPath::split("/assets/image.png?hueshift=30");
        assert_eq!(path.base_path, "/assets/image.png");
        assert_eq!(path.sub_path, None);
        assert_eq!(path.directives, "?hueshift=30");
    }

    #[test]
    fn test_split_full() {
        let path = AssetPath::split("/assets/image.png:default?hueshift=30?scale=2");
        assert_eq!(path.base_path, "/assets/image.png");
        assert_eq!(path.sub_path, Some("default".to_string()));
        assert_eq!(path.directives, "?hueshift=30?scale=2");
    }

    #[test]
    fn test_join() {
        let path = AssetPath {
            base_path: "/assets/image.png".to_string(),
            sub_path: Some("frame1".to_string()),
            directives: "?scale=2".to_string(),
        };
        assert_eq!(path.join(), "/assets/image.png:frame1?scale=2");
    }

    #[test]
    fn test_directory() {
        assert_eq!(AssetPath::directory("/assets/images/test.png"), "/assets/images/");
        assert_eq!(AssetPath::directory("test.png"), "");
    }

    #[test]
    fn test_filename() {
        assert_eq!(AssetPath::filename("/assets/images/test.png"), "test.png");
        assert_eq!(AssetPath::filename("test.png"), "test.png");
    }

    #[test]
    fn test_extension() {
        assert_eq!(AssetPath::extension("/assets/test.png"), "png");
        assert_eq!(AssetPath::extension("/assets/test"), "");
    }

    #[test]
    fn test_relative_to() {
        assert_eq!(
            AssetPath::relative_to("/assets/images/test.png", "other.png"),
            "/assets/images/other.png"
        );
        assert_eq!(
            AssetPath::relative_to("/assets/images/test.png", "/absolute/path.png"),
            "/absolute/path.png"
        );
        assert_eq!(
            AssetPath::relative_to("/assets/images/test.png", "../sounds/click.ogg"),
            "/assets/sounds/click.ogg"
        );
    }

    #[test]
    fn test_set_sub_path() {
        assert_eq!(
            AssetPath::set_sub_path_str("/test.png", "frame1"),
            "/test.png:frame1"
        );
        assert_eq!(
            AssetPath::set_sub_path_str("/test.png:old", "new"),
            "/test.png:new"
        );
    }

    #[test]
    fn test_remove_sub_path() {
        assert_eq!(
            AssetPath::remove_sub_path_str("/test.png:frame1"),
            "/test.png"
        );
    }

    #[test]
    fn test_directives() {
        assert_eq!(
            AssetPath::get_directives_str("/test.png?scale=2"),
            "?scale=2"
        );
        assert_eq!(
            AssetPath::add_directives_str("/test.png", "?scale=2"),
            "/test.png?scale=2"
        );
        assert_eq!(
            AssetPath::remove_directives_str("/test.png?scale=2"),
            "/test.png"
        );
    }
}
