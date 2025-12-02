//! LRU Cache implementation compatible with C++ Star::LruCache
//!
//! This module provides a least-recently-used cache with configurable size limits.

use std::collections::HashMap;
use std::hash::Hash;

/// A node in the LRU linked list
struct LruNode<K, V> {
    key: K,
    value: V,
    prev: Option<usize>,
    next: Option<usize>,
}

/// Least-Recently-Used cache with O(1) lookup and eviction
///
/// Compatible with C++ Star::LruCache
pub struct LruCache<K, V> {
    /// Map from keys to node indices
    map: HashMap<K, usize>,
    /// Storage for nodes
    nodes: Vec<Option<LruNode<K, V>>>,
    /// Free list indices
    free_list: Vec<usize>,
    /// Head of the LRU list (most recently used)
    head: Option<usize>,
    /// Tail of the LRU list (least recently used)
    tail: Option<usize>,
    /// Maximum number of entries
    max_size: usize,
}

impl<K, V> LruCache<K, V>
where
    K: Eq + Hash + Clone,
{
    /// Create a new LRU cache with the given capacity
    pub fn new(max_size: usize) -> Self {
        Self {
            map: HashMap::with_capacity(max_size),
            nodes: Vec::with_capacity(max_size),
            free_list: Vec::new(),
            head: None,
            tail: None,
            max_size,
        }
    }

    /// Get the maximum size of the cache
    pub fn max_size(&self) -> usize {
        self.max_size
    }

    /// Set the maximum size, evicting entries if necessary
    pub fn set_max_size(&mut self, max_size: usize) {
        self.max_size = max_size;
        while self.len() > max_size {
            self.evict_lru();
        }
    }

    /// Get the current number of entries
    pub fn len(&self) -> usize {
        self.map.len()
    }

    /// Check if the cache is empty
    pub fn is_empty(&self) -> bool {
        self.map.is_empty()
    }

    /// Check if the cache is full
    pub fn is_full(&self) -> bool {
        self.len() >= self.max_size
    }

    /// Check if a key exists in the cache
    pub fn contains(&self, key: &K) -> bool {
        self.map.contains_key(key)
    }

    /// Get a reference to a value, marking it as recently used
    pub fn get(&mut self, key: &K) -> Option<&V> {
        if let Some(&index) = self.map.get(key) {
            self.move_to_head(index);
            self.nodes[index].as_ref().map(|n| &n.value)
        } else {
            None
        }
    }

    /// Get a mutable reference to a value, marking it as recently used
    pub fn get_mut(&mut self, key: &K) -> Option<&mut V> {
        if let Some(&index) = self.map.get(key) {
            self.move_to_head(index);
            self.nodes[index].as_mut().map(|n| &mut n.value)
        } else {
            None
        }
    }

    /// Peek at a value without marking it as recently used
    pub fn peek(&self, key: &K) -> Option<&V> {
        self.map
            .get(key)
            .and_then(|&index| self.nodes[index].as_ref().map(|n| &n.value))
    }

    /// Insert a key-value pair, returning the old value if present
    ///
    /// If the cache is full, the least recently used entry is evicted.
    pub fn insert(&mut self, key: K, value: V) -> Option<V> {
        if let Some(&index) = self.map.get(&key) {
            // Update existing entry
            let node = self.nodes[index].as_mut().unwrap();
            let old_value = std::mem::replace(&mut node.value, value);
            self.move_to_head(index);
            return Some(old_value);
        }

        // Evict if at capacity
        if self.len() >= self.max_size && self.max_size > 0 {
            self.evict_lru();
        }

        // Get a free index
        let index = if let Some(free_index) = self.free_list.pop() {
            free_index
        } else {
            let index = self.nodes.len();
            self.nodes.push(None);
            index
        };

        // Create new node
        let node = LruNode {
            key: key.clone(),
            value,
            prev: None,
            next: self.head,
        };

        // Update head's prev pointer
        if let Some(old_head) = self.head {
            if let Some(head_node) = &mut self.nodes[old_head] {
                head_node.prev = Some(index);
            }
        }

        // Insert node
        self.nodes[index] = Some(node);
        self.map.insert(key, index);
        self.head = Some(index);

        if self.tail.is_none() {
            self.tail = Some(index);
        }

        None
    }

    /// Remove a key from the cache, returning its value
    pub fn remove(&mut self, key: &K) -> Option<V> {
        if let Some(index) = self.map.remove(key) {
            let node = self.nodes[index].take().unwrap();
            self.unlink(index, node.prev, node.next);
            self.free_list.push(index);
            Some(node.value)
        } else {
            None
        }
    }

    /// Clear all entries from the cache
    pub fn clear(&mut self) {
        self.map.clear();
        self.nodes.clear();
        self.free_list.clear();
        self.head = None;
        self.tail = None;
    }

    /// Get an iterator over keys in order from most to least recently used
    pub fn keys(&self) -> impl Iterator<Item = &K> {
        LruIterator {
            cache: self,
            current: self.head,
        }
    }

    /// Get an iterator over values in order from most to least recently used
    pub fn values(&self) -> impl Iterator<Item = &V> {
        self.keys().filter_map(move |k| self.peek(k))
    }

    /// Get an iterator over (key, value) pairs
    pub fn iter(&self) -> impl Iterator<Item = (&K, &V)> {
        LruPairIterator {
            cache: self,
            current: self.head,
        }
    }

    /// Move a node to the head of the list
    fn move_to_head(&mut self, index: usize) {
        if self.head == Some(index) {
            return;
        }

        let (prev, next) = {
            let node = self.nodes[index].as_ref().unwrap();
            (node.prev, node.next)
        };

        // Unlink from current position
        self.unlink(index, prev, next);

        // Link at head
        if let Some(old_head) = self.head {
            if let Some(head_node) = &mut self.nodes[old_head] {
                head_node.prev = Some(index);
            }
        }

        if let Some(node) = &mut self.nodes[index] {
            node.prev = None;
            node.next = self.head;
        }

        self.head = Some(index);
    }

    /// Unlink a node from the list
    fn unlink(&mut self, _index: usize, prev: Option<usize>, next: Option<usize>) {
        if let Some(prev_idx) = prev {
            if let Some(prev_node) = &mut self.nodes[prev_idx] {
                prev_node.next = next;
            }
        } else {
            self.head = next;
        }

        if let Some(next_idx) = next {
            if let Some(next_node) = &mut self.nodes[next_idx] {
                next_node.prev = prev;
            }
        } else {
            self.tail = prev;
        }
    }

    /// Evict the least recently used entry
    fn evict_lru(&mut self) -> Option<(K, V)> {
        if let Some(tail_index) = self.tail {
            let node = self.nodes[tail_index].take().unwrap();
            self.map.remove(&node.key);
            self.unlink(tail_index, node.prev, node.next);
            self.free_list.push(tail_index);
            Some((node.key, node.value))
        } else {
            None
        }
    }
}

impl<K, V> Default for LruCache<K, V>
where
    K: Eq + Hash + Clone,
{
    fn default() -> Self {
        Self::new(128)
    }
}

/// Iterator over keys
struct LruIterator<'a, K, V> {
    cache: &'a LruCache<K, V>,
    current: Option<usize>,
}

impl<'a, K, V> Iterator for LruIterator<'a, K, V> {
    type Item = &'a K;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(index) = self.current {
            let node = self.cache.nodes[index].as_ref()?;
            self.current = node.next;
            Some(&node.key)
        } else {
            None
        }
    }
}

/// Iterator over (key, value) pairs
struct LruPairIterator<'a, K, V> {
    cache: &'a LruCache<K, V>,
    current: Option<usize>,
}

impl<'a, K, V> Iterator for LruPairIterator<'a, K, V> {
    type Item = (&'a K, &'a V);

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(index) = self.current {
            let node = self.cache.nodes[index].as_ref()?;
            self.current = node.next;
            Some((&node.key, &node.value))
        } else {
            None
        }
    }
}

/// Time-To-Live cache that evicts entries after a timeout
pub struct TtlCache<K, V> {
    /// Inner LRU cache
    inner: LruCache<K, (V, std::time::Instant)>,
    /// Time-to-live for entries
    ttl: std::time::Duration,
}

impl<K, V> TtlCache<K, V>
where
    K: Eq + Hash + Clone,
{
    /// Create a new TTL cache
    pub fn new(max_size: usize, ttl: std::time::Duration) -> Self {
        Self {
            inner: LruCache::new(max_size),
            ttl,
        }
    }

    /// Get the TTL duration
    pub fn ttl(&self) -> std::time::Duration {
        self.ttl
    }

    /// Set the TTL duration
    pub fn set_ttl(&mut self, ttl: std::time::Duration) {
        self.ttl = ttl;
    }

    /// Get the current size
    pub fn len(&self) -> usize {
        self.inner.len()
    }

    /// Check if empty
    pub fn is_empty(&self) -> bool {
        self.inner.is_empty()
    }

    /// Get a value if it exists and hasn't expired
    pub fn get(&mut self, key: &K) -> Option<&V> {
        let now = std::time::Instant::now();
        if let Some((value, inserted)) = self.inner.get(key) {
            if now.duration_since(*inserted) < self.ttl {
                return Some(value);
            }
            // Expired - will be removed on next cleanup
        }
        None
    }

    /// Insert a value with the current timestamp
    pub fn insert(&mut self, key: K, value: V) -> Option<V> {
        self.inner
            .insert(key, (value, std::time::Instant::now()))
            .map(|(v, _)| v)
    }

    /// Remove a value
    pub fn remove(&mut self, key: &K) -> Option<V> {
        self.inner.remove(key).map(|(v, _)| v)
    }

    /// Remove all expired entries
    pub fn cleanup_expired(&mut self) {
        let now = std::time::Instant::now();
        let expired_keys: Vec<K> = self
            .inner
            .iter()
            .filter(|(_, (_, inserted))| now.duration_since(*inserted) >= self.ttl)
            .map(|(k, _)| k.clone())
            .collect();

        for key in expired_keys {
            self.inner.remove(&key);
        }
    }

    /// Clear the cache
    pub fn clear(&mut self) {
        self.inner.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lru_cache_basic() {
        let mut cache: LruCache<i32, &str> = LruCache::new(3);

        cache.insert(1, "one");
        cache.insert(2, "two");
        cache.insert(3, "three");

        assert_eq!(cache.len(), 3);
        assert_eq!(cache.get(&1), Some(&"one"));
        assert_eq!(cache.get(&2), Some(&"two"));
        assert_eq!(cache.get(&3), Some(&"three"));
    }

    #[test]
    fn test_lru_cache_eviction() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");
        cache.insert(2, "two");
        cache.insert(3, "three");

        // 1 should be evicted
        assert_eq!(cache.len(), 2);
        assert_eq!(cache.get(&1), None);
        assert_eq!(cache.get(&2), Some(&"two"));
        assert_eq!(cache.get(&3), Some(&"three"));
    }

    #[test]
    fn test_lru_cache_access_order() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");
        cache.insert(2, "two");

        // Access 1 to make it most recently used
        cache.get(&1);

        // Insert 3, should evict 2
        cache.insert(3, "three");

        assert_eq!(cache.get(&1), Some(&"one"));
        assert_eq!(cache.get(&2), None);
        assert_eq!(cache.get(&3), Some(&"three"));
    }

    #[test]
    fn test_lru_cache_update() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");
        let old = cache.insert(1, "ONE");

        assert_eq!(old, Some("one"));
        assert_eq!(cache.get(&1), Some(&"ONE"));
        assert_eq!(cache.len(), 1);
    }

    #[test]
    fn test_lru_cache_remove() {
        let mut cache: LruCache<i32, &str> = LruCache::new(3);

        cache.insert(1, "one");
        cache.insert(2, "two");
        cache.insert(3, "three");

        let removed = cache.remove(&2);
        assert_eq!(removed, Some("two"));
        assert_eq!(cache.len(), 2);
        assert_eq!(cache.get(&2), None);
    }

    #[test]
    fn test_lru_cache_clear() {
        let mut cache: LruCache<i32, &str> = LruCache::new(3);

        cache.insert(1, "one");
        cache.insert(2, "two");

        cache.clear();
        assert!(cache.is_empty());
    }

    #[test]
    fn test_lru_cache_contains() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");

        assert!(cache.contains(&1));
        assert!(!cache.contains(&2));
    }

    #[test]
    fn test_lru_cache_peek() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");
        cache.insert(2, "two");

        // Peek doesn't change order
        assert_eq!(cache.peek(&1), Some(&"one"));

        // Insert 3, should evict 1 (not 2) because peek didn't update order
        cache.insert(3, "three");

        assert_eq!(cache.get(&1), None);
        assert_eq!(cache.get(&2), Some(&"two"));
    }

    #[test]
    fn test_lru_cache_get_mut() {
        let mut cache: LruCache<i32, String> = LruCache::new(2);

        cache.insert(1, "one".to_string());

        if let Some(val) = cache.get_mut(&1) {
            val.push_str("!");
        }

        assert_eq!(cache.get(&1), Some(&"one!".to_string()));
    }

    #[test]
    fn test_lru_cache_resize() {
        let mut cache: LruCache<i32, &str> = LruCache::new(5);

        for i in 0..5 {
            cache.insert(i, "value");
        }
        assert_eq!(cache.len(), 5);

        cache.set_max_size(2);
        assert_eq!(cache.len(), 2);
        assert_eq!(cache.max_size(), 2);
    }

    #[test]
    fn test_lru_cache_iter() {
        let mut cache: LruCache<i32, &str> = LruCache::new(3);

        cache.insert(1, "one");
        cache.insert(2, "two");
        cache.insert(3, "three");

        // Access 1 to make it most recently used
        cache.get(&1);

        let keys: Vec<_> = cache.keys().cloned().collect();
        // Order should be 1, 3, 2 (most to least recently used)
        assert_eq!(keys, vec![1, 3, 2]);
    }

    #[test]
    fn test_lru_cache_empty() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        assert!(cache.is_empty());
        assert!(!cache.is_full());
        assert_eq!(cache.get(&1), None);
    }

    #[test]
    fn test_lru_cache_is_full() {
        let mut cache: LruCache<i32, &str> = LruCache::new(2);

        cache.insert(1, "one");
        assert!(!cache.is_full());

        cache.insert(2, "two");
        assert!(cache.is_full());
    }

    #[test]
    fn test_ttl_cache_basic() {
        let mut cache: TtlCache<i32, &str> = TtlCache::new(10, std::time::Duration::from_secs(60));

        cache.insert(1, "one");
        assert_eq!(cache.get(&1), Some(&"one"));
        assert_eq!(cache.len(), 1);
    }

    #[test]
    fn test_ttl_cache_remove() {
        let mut cache: TtlCache<i32, &str> = TtlCache::new(10, std::time::Duration::from_secs(60));

        cache.insert(1, "one");
        let removed = cache.remove(&1);

        assert_eq!(removed, Some("one"));
        assert!(cache.is_empty());
    }

    #[test]
    fn test_ttl_cache_clear() {
        let mut cache: TtlCache<i32, &str> = TtlCache::new(10, std::time::Duration::from_secs(60));

        cache.insert(1, "one");
        cache.insert(2, "two");
        cache.clear();

        assert!(cache.is_empty());
    }

    #[test]
    fn test_default() {
        let cache: LruCache<i32, i32> = LruCache::default();
        assert_eq!(cache.max_size(), 128);
    }
}
