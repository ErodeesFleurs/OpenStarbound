//! String utilities compatible with C++ Star::String
//!
//! Provides string manipulation functions matching the C++ implementation.

use std::borrow::Cow;

/// Case sensitivity mode for string operations.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CaseSensitivity {
    CaseSensitive,
    CaseInsensitive,
}

/// Check if a character is whitespace (space, tab, newline, carriage return).
pub fn is_space(c: char) -> bool {
    matches!(c, ' ' | '\t' | '\n' | '\r' | '\u{FEFF}')
}

/// Check if a character is an ASCII digit.
pub fn is_ascii_number(c: char) -> bool {
    c.is_ascii_digit()
}

/// Check if a character is an ASCII letter.
pub fn is_ascii_letter(c: char) -> bool {
    c.is_ascii_alphabetic()
}

/// Convert a character to lowercase (ASCII only).
pub fn to_lower(c: char) -> char {
    c.to_ascii_lowercase()
}

/// Convert a character to uppercase (ASCII only).
pub fn to_upper(c: char) -> char {
    c.to_ascii_uppercase()
}

/// Compare two characters with optional case sensitivity.
pub fn char_equal(c1: char, c2: char, cs: CaseSensitivity) -> bool {
    match cs {
        CaseSensitivity::CaseSensitive => c1 == c2,
        CaseSensitivity::CaseInsensitive => c1.to_ascii_lowercase() == c2.to_ascii_lowercase(),
    }
}

/// Join two strings with a separator, ensuring no duplicate separators.
///
/// # Example
/// ```
/// use starbound_core::types::string_util::join_with;
///
/// assert_eq!(join_with("/", "foo", "bar"), "foo/bar");
/// assert_eq!(join_with("/", "foo/", "bar"), "foo/bar");
/// assert_eq!(join_with("/", "foo", "/bar"), "foo/bar");
/// assert_eq!(join_with("/", "foo/", "/bar"), "foo/bar");
/// assert_eq!(join_with("/", "", "bar"), "bar");
/// ```
pub fn join_with(joiner: &str, left: &str, right: &str) -> String {
    if left.is_empty() {
        return right.to_string();
    }
    if right.is_empty() {
        return left.to_string();
    }

    let left_ends = left.ends_with(joiner);
    let right_starts = right.starts_with(joiner);

    match (left_ends, right_starts) {
        (true, true) => format!("{}{}", left, &right[joiner.len()..]),
        (true, false) | (false, true) => format!("{}{}", left, right),
        (false, false) => format!("{}{}{}", left, joiner, right),
    }
}

/// Trim whitespace from both ends of a string.
pub fn trim(s: &str) -> &str {
    s.trim_matches(|c| is_space(c))
}

/// Trim whitespace from the left of a string.
pub fn trim_left(s: &str) -> &str {
    s.trim_start_matches(|c| is_space(c))
}

/// Trim whitespace from the right of a string.
pub fn trim_right(s: &str) -> &str {
    s.trim_end_matches(|c| is_space(c))
}

/// Convert a string to lowercase (ASCII only).
pub fn to_lowercase(s: &str) -> String {
    s.to_ascii_lowercase()
}

/// Convert a string to uppercase (ASCII only).
pub fn to_uppercase(s: &str) -> String {
    s.to_ascii_uppercase()
}

/// Check if two strings are equal with optional case sensitivity.
pub fn string_equal(s1: &str, s2: &str, cs: CaseSensitivity) -> bool {
    match cs {
        CaseSensitivity::CaseSensitive => s1 == s2,
        CaseSensitivity::CaseInsensitive => s1.eq_ignore_ascii_case(s2),
    }
}

/// Check if a string starts with a prefix with optional case sensitivity.
pub fn starts_with(s: &str, prefix: &str, cs: CaseSensitivity) -> bool {
    if s.len() < prefix.len() {
        return false;
    }
    match cs {
        CaseSensitivity::CaseSensitive => s.starts_with(prefix),
        CaseSensitivity::CaseInsensitive => {
            s[..prefix.len()].eq_ignore_ascii_case(prefix)
        }
    }
}

/// Check if a string ends with a suffix with optional case sensitivity.
pub fn ends_with(s: &str, suffix: &str, cs: CaseSensitivity) -> bool {
    if s.len() < suffix.len() {
        return false;
    }
    match cs {
        CaseSensitivity::CaseSensitive => s.ends_with(suffix),
        CaseSensitivity::CaseInsensitive => {
            s[s.len() - suffix.len()..].eq_ignore_ascii_case(suffix)
        }
    }
}

/// Check if a string contains a substring with optional case sensitivity.
pub fn contains(s: &str, needle: &str, cs: CaseSensitivity) -> bool {
    match cs {
        CaseSensitivity::CaseSensitive => s.contains(needle),
        CaseSensitivity::CaseInsensitive => {
            s.to_ascii_lowercase().contains(&needle.to_ascii_lowercase())
        }
    }
}

/// Split a string by a delimiter.
pub fn split(s: &str, delimiter: &str) -> Vec<String> {
    s.split(delimiter).map(|s| s.to_string()).collect()
}

/// Split a string by whitespace.
pub fn split_whitespace(s: &str) -> Vec<String> {
    s.split(is_space)
        .filter(|s| !s.is_empty())
        .map(|s| s.to_string())
        .collect()
}

/// Replace all occurrences of a pattern with a replacement.
pub fn replace_all(s: &str, pattern: &str, replacement: &str) -> String {
    s.replace(pattern, replacement)
}

/// Escape special characters in a string for safe display.
pub fn escape_string(s: &str) -> Cow<'_, str> {
    let mut result = String::new();
    let mut needs_escape = false;

    for c in s.chars() {
        match c {
            '\\' => { needs_escape = true; result.push_str("\\\\"); }
            '"' => { needs_escape = true; result.push_str("\\\""); }
            '\n' => { needs_escape = true; result.push_str("\\n"); }
            '\r' => { needs_escape = true; result.push_str("\\r"); }
            '\t' => { needs_escape = true; result.push_str("\\t"); }
            _ if c.is_control() => {
                needs_escape = true;
                result.push_str(&format!("\\x{:02x}", c as u32));
            }
            _ => result.push(c),
        }
    }

    if needs_escape {
        Cow::Owned(result)
    } else {
        Cow::Borrowed(s)
    }
}

/// Parse an escaped string, converting escape sequences to their characters.
pub fn unescape_string(s: &str) -> Option<String> {
    let mut result = String::new();
    let mut chars = s.chars().peekable();

    while let Some(c) = chars.next() {
        if c == '\\' {
            match chars.next()? {
                'n' => result.push('\n'),
                'r' => result.push('\r'),
                't' => result.push('\t'),
                '\\' => result.push('\\'),
                '"' => result.push('"'),
                'x' => {
                    let hex: String = chars.by_ref().take(2).collect();
                    if hex.len() != 2 {
                        return None;
                    }
                    let code = u8::from_str_radix(&hex, 16).ok()?;
                    result.push(code as char);
                }
                _ => return None,
            }
        } else {
            result.push(c);
        }
    }

    Some(result)
}

/// Format a size in bytes to a human-readable string.
pub fn format_byte_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB", "TB", "PB"];
    
    if bytes == 0 {
        return "0 B".to_string();
    }

    let mut size = bytes as f64;
    let mut unit_index = 0;

    while size >= 1024.0 && unit_index < UNITS.len() - 1 {
        size /= 1024.0;
        unit_index += 1;
    }

    if size.fract() < 0.01 {
        format!("{:.0} {}", size, UNITS[unit_index])
    } else {
        format!("{:.2} {}", size, UNITS[unit_index])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_space() {
        assert!(is_space(' '));
        assert!(is_space('\t'));
        assert!(is_space('\n'));
        assert!(is_space('\r'));
        assert!(!is_space('a'));
        assert!(!is_space('0'));
    }

    #[test]
    fn test_join_with() {
        assert_eq!(join_with("/", "foo", "bar"), "foo/bar");
        assert_eq!(join_with("/", "foo/", "bar"), "foo/bar");
        assert_eq!(join_with("/", "foo", "/bar"), "foo/bar");
        assert_eq!(join_with("/", "foo/", "/bar"), "foo/bar");
        assert_eq!(join_with("/", "", "bar"), "bar");
        assert_eq!(join_with("/", "foo", ""), "foo");
        assert_eq!(join_with("", "foo", "bar"), "foobar");
    }

    #[test]
    fn test_trim() {
        assert_eq!(trim("  hello  "), "hello");
        assert_eq!(trim("\t\nhello\r\n"), "hello");
        assert_eq!(trim("hello"), "hello");
    }

    #[test]
    fn test_case_sensitivity() {
        assert!(string_equal("Hello", "Hello", CaseSensitivity::CaseSensitive));
        assert!(!string_equal("Hello", "hello", CaseSensitivity::CaseSensitive));
        assert!(string_equal("Hello", "hello", CaseSensitivity::CaseInsensitive));
    }

    #[test]
    fn test_starts_ends_with() {
        assert!(starts_with("Hello World", "Hello", CaseSensitivity::CaseSensitive));
        assert!(starts_with("Hello World", "hello", CaseSensitivity::CaseInsensitive));
        assert!(!starts_with("Hello World", "World", CaseSensitivity::CaseSensitive));

        assert!(ends_with("Hello World", "World", CaseSensitivity::CaseSensitive));
        assert!(ends_with("Hello World", "world", CaseSensitivity::CaseInsensitive));
        assert!(!ends_with("Hello World", "Hello", CaseSensitivity::CaseSensitive));
    }

    #[test]
    fn test_contains() {
        assert!(contains("Hello World", "lo Wo", CaseSensitivity::CaseSensitive));
        assert!(!contains("Hello World", "lo wo", CaseSensitivity::CaseSensitive));
        assert!(contains("Hello World", "lo wo", CaseSensitivity::CaseInsensitive));
    }

    #[test]
    fn test_split() {
        assert_eq!(split("a,b,c", ","), vec!["a", "b", "c"]);
        assert_eq!(split("hello", ","), vec!["hello"]);
    }

    #[test]
    fn test_split_whitespace() {
        assert_eq!(split_whitespace("  a  b  c  "), vec!["a", "b", "c"]);
    }

    #[test]
    fn test_escape_unescape() {
        assert_eq!(escape_string("hello\nworld"), "hello\\nworld");
        assert_eq!(unescape_string("hello\\nworld"), Some("hello\nworld".to_string()));
        assert_eq!(unescape_string("\\t\\r\\n"), Some("\t\r\n".to_string()));
    }

    #[test]
    fn test_format_byte_size() {
        assert_eq!(format_byte_size(0), "0 B");
        assert_eq!(format_byte_size(512), "512 B");
        assert_eq!(format_byte_size(1024), "1 KB");
        assert_eq!(format_byte_size(1536), "1.50 KB");
        assert_eq!(format_byte_size(1048576), "1 MB");
        assert_eq!(format_byte_size(1073741824), "1 GB");
    }
}
