//! Bidirectional map compatible with C++ Star::BiMap
//!
//! A map that allows lookup by either key or value in O(1) time.

use std::collections::HashMap;
use std::hash::Hash;

/// A bidirectional map that allows O(1) lookup from left to right and right to left.
///
/// Both left and right values must be unique within their respective sets.
///
/// # Example
/// ```
/// use starbound_core::types::bimap::BiMap;
///
/// let mut map: BiMap<String, i32> = BiMap::new();
/// map.insert("one".to_string(), 1);
/// map.insert("two".to_string(), 2);
///
/// assert_eq!(map.get_right(&"one".to_string()), Some(&1));
/// assert_eq!(map.get_left(&2), Some(&"two".to_string()));
/// ```
#[derive(Debug, Clone)]
pub struct BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
    left_to_right: HashMap<L, R>,
    right_to_left: HashMap<R, L>,
}

impl<L, R> Default for BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
    fn default() -> Self {
        Self::new()
    }
}

impl<L, R> BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
    /// Create a new empty BiMap.
    pub fn new() -> Self {
        Self {
            left_to_right: HashMap::new(),
            right_to_left: HashMap::new(),
        }
    }

    /// Create a BiMap with the given capacity.
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            left_to_right: HashMap::with_capacity(capacity),
            right_to_left: HashMap::with_capacity(capacity),
        }
    }

    /// Returns the number of key-value pairs.
    pub fn len(&self) -> usize {
        self.left_to_right.len()
    }

    /// Returns true if the map is empty.
    pub fn is_empty(&self) -> bool {
        self.left_to_right.is_empty()
    }

    /// Clear all entries from the map.
    pub fn clear(&mut self) {
        self.left_to_right.clear();
        self.right_to_left.clear();
    }

    /// Insert a left-right pair into the map.
    ///
    /// Returns `true` if the pair was inserted successfully.
    /// Returns `false` if either the left or right value already exists.
    pub fn insert(&mut self, left: L, right: R) -> bool {
        if self.left_to_right.contains_key(&left) || self.right_to_left.contains_key(&right) {
            return false;
        }
        self.left_to_right.insert(left.clone(), right.clone());
        self.right_to_left.insert(right, left);
        true
    }

    /// Insert a pair, overwriting any existing mappings.
    ///
    /// If the left value already exists, its old right mapping is removed.
    /// If the right value already exists, its old left mapping is removed.
    pub fn insert_overwrite(&mut self, left: L, right: R) {
        // Remove existing mappings if any
        if let Some(old_right) = self.left_to_right.remove(&left) {
            self.right_to_left.remove(&old_right);
        }
        if let Some(old_left) = self.right_to_left.remove(&right) {
            self.left_to_right.remove(&old_left);
        }

        self.left_to_right.insert(left.clone(), right.clone());
        self.right_to_left.insert(right, left);
    }

    /// Check if a left value exists.
    pub fn has_left(&self, left: &L) -> bool {
        self.left_to_right.contains_key(left)
    }

    /// Check if a right value exists.
    pub fn has_right(&self, right: &R) -> bool {
        self.right_to_left.contains_key(right)
    }

    /// Get the right value for a left key.
    pub fn get_right(&self, left: &L) -> Option<&R> {
        self.left_to_right.get(left)
    }

    /// Get the left value for a right key.
    pub fn get_left(&self, right: &R) -> Option<&L> {
        self.right_to_left.get(right)
    }

    /// Get the right value for a left key, returning a default if not found.
    pub fn value_right(&self, left: &L, default: R) -> R {
        self.left_to_right.get(left).cloned().unwrap_or(default)
    }

    /// Get the left value for a right key, returning a default if not found.
    pub fn value_left(&self, right: &R, default: L) -> L {
        self.right_to_left.get(right).cloned().unwrap_or(default)
    }

    /// Remove an entry by left key, returning the removed right value.
    pub fn remove_left(&mut self, left: &L) -> Option<R> {
        if let Some(right) = self.left_to_right.remove(left) {
            self.right_to_left.remove(&right);
            Some(right)
        } else {
            None
        }
    }

    /// Remove an entry by right key, returning the removed left value.
    pub fn remove_right(&mut self, right: &R) -> Option<L> {
        if let Some(left) = self.right_to_left.remove(right) {
            self.left_to_right.remove(&left);
            Some(left)
        } else {
            None
        }
    }

    /// Get all left values.
    pub fn left_values(&self) -> impl Iterator<Item = &L> {
        self.left_to_right.keys()
    }

    /// Get all right values.
    pub fn right_values(&self) -> impl Iterator<Item = &R> {
        self.right_to_left.keys()
    }

    /// Get all pairs as an iterator.
    pub fn iter(&self) -> impl Iterator<Item = (&L, &R)> {
        self.left_to_right.iter()
    }
}

impl<L, R> FromIterator<(L, R)> for BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
    fn from_iter<I: IntoIterator<Item = (L, R)>>(iter: I) -> Self {
        let mut map = BiMap::new();
        for (left, right) in iter {
            map.insert(left, right);
        }
        map
    }
}

impl<L, R> PartialEq for BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
    fn eq(&self, other: &Self) -> bool {
        self.left_to_right == other.left_to_right
    }
}

impl<L, R> Eq for BiMap<L, R>
where
    L: Eq + Hash + Clone,
    R: Eq + Hash + Clone,
{
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_operations() {
        let mut map: BiMap<String, i32> = BiMap::new();

        assert!(map.insert("one".to_string(), 1));
        assert!(map.insert("two".to_string(), 2));
        assert!(map.insert("three".to_string(), 3));

        assert_eq!(map.get_right(&"one".to_string()), Some(&1));
        assert_eq!(map.get_right(&"two".to_string()), Some(&2));
        assert_eq!(map.get_left(&3), Some(&"three".to_string()));
    }

    #[test]
    fn test_duplicate_insert() {
        let mut map: BiMap<String, i32> = BiMap::new();

        assert!(map.insert("one".to_string(), 1));
        assert!(!map.insert("one".to_string(), 2)); // Duplicate left
        assert!(!map.insert("two".to_string(), 1)); // Duplicate right
    }

    #[test]
    fn test_insert_overwrite() {
        let mut map: BiMap<String, i32> = BiMap::new();

        map.insert_overwrite("one".to_string(), 1);
        map.insert_overwrite("one".to_string(), 2);

        assert_eq!(map.get_right(&"one".to_string()), Some(&2));
        assert_eq!(map.get_left(&1), None);
        assert_eq!(map.get_left(&2), Some(&"one".to_string()));
    }

    #[test]
    fn test_remove() {
        let mut map: BiMap<String, i32> = BiMap::new();

        map.insert("one".to_string(), 1);
        map.insert("two".to_string(), 2);

        assert_eq!(map.remove_left(&"one".to_string()), Some(1));
        assert!(!map.has_left(&"one".to_string()));
        assert!(!map.has_right(&1));

        assert_eq!(map.remove_right(&2), Some("two".to_string()));
        assert!(map.is_empty());
    }

    #[test]
    fn test_from_iterator() {
        let pairs = vec![("a".to_string(), 1), ("b".to_string(), 2)];
        let map: BiMap<String, i32> = pairs.into_iter().collect();

        assert_eq!(map.len(), 2);
        assert_eq!(map.get_right(&"a".to_string()), Some(&1));
        assert_eq!(map.get_left(&2), Some(&"b".to_string()));
    }

    #[test]
    fn test_value_with_default() {
        let mut map: BiMap<String, i32> = BiMap::new();
        map.insert("one".to_string(), 1);

        assert_eq!(map.value_right(&"one".to_string(), 0), 1);
        assert_eq!(map.value_right(&"missing".to_string(), 99), 99);
        assert_eq!(map.value_left(&1, "default".to_string()), "one".to_string());
        assert_eq!(map.value_left(&99, "default".to_string()), "default".to_string());
    }
}
