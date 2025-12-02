//! Threading utilities compatible with C++ Star::Thread.
//!
//! This module provides threading primitives that match the C++ implementation.

use std::sync::{
    atomic::{AtomicBool, AtomicUsize, Ordering},
    Arc, Condvar, Mutex as StdMutex, RwLock,
};
use std::thread::{self, JoinHandle};
use std::time::Duration;

/// Thread sleep utilities
pub struct Thread;

impl Thread {
    /// Sleep for at least the given number of milliseconds.
    ///
    /// # Arguments
    /// * `millis` - Number of milliseconds to sleep
    pub fn sleep(millis: u64) {
        thread::sleep(Duration::from_millis(millis));
    }

    /// Sleep for a more precise amount of time using spin-waiting.
    ///
    /// This uses more CPU but is more accurate for short durations.
    ///
    /// # Arguments
    /// * `millis` - Number of milliseconds to sleep
    pub fn sleep_precise(millis: u64) {
        let duration = Duration::from_millis(millis);
        let start = std::time::Instant::now();

        // For very short durations, spin-wait
        if millis < 10 {
            while start.elapsed() < duration {
                std::hint::spin_loop();
            }
        } else {
            // Sleep for most of the time, then spin for the remainder
            let sleep_time = Duration::from_millis(millis.saturating_sub(2));
            thread::sleep(sleep_time);
            while start.elapsed() < duration {
                std::hint::spin_loop();
            }
        }
    }

    /// Yield this thread, offering the opportunity to reschedule.
    pub fn yield_now() {
        thread::yield_now();
    }

    /// Get the number of available processors/cores.
    pub fn number_of_processors() -> usize {
        thread::available_parallelism()
            .map(|p| p.get())
            .unwrap_or(1)
    }
}

/// A thread function that wraps a function call in another thread.
///
/// This is a lightweight alternative to deriving from Thread.
/// Exceptions (panics) are captured and re-thrown during finish().
pub struct ThreadFunction<T> {
    handle: Option<JoinHandle<T>>,
    name: String,
    is_finished: Arc<AtomicBool>,
}

impl<T: Send + 'static> ThreadFunction<T> {
    /// Create a new thread function with the given name.
    ///
    /// # Arguments
    /// * `name` - Name for the thread
    /// * `func` - Function to execute in the thread
    pub fn new<F>(name: impl Into<String>, func: F) -> Self
    where
        F: FnOnce() -> T + Send + 'static,
    {
        let name = name.into();
        let is_finished = Arc::new(AtomicBool::new(false));
        let is_finished_clone = is_finished.clone();

        let handle = thread::Builder::new()
            .name(name.clone())
            .spawn(move || {
                let result = func();
                is_finished_clone.store(true, Ordering::SeqCst);
                result
            })
            .expect("Failed to spawn thread");

        ThreadFunction {
            handle: Some(handle),
            name,
            is_finished,
        }
    }

    /// Wait for the function to finish and return the result.
    ///
    /// If the function panicked, this will propagate the panic.
    /// May only be called once.
    pub fn finish(mut self) -> T {
        self.handle
            .take()
            .expect("ThreadFunction::finish called twice")
            .join()
            .expect("Thread panicked")
    }

    /// Check if the thread has finished running.
    pub fn is_finished(&self) -> bool {
        self.is_finished.load(Ordering::SeqCst)
    }

    /// Check if the thread is still running.
    pub fn is_running(&self) -> bool {
        self.handle.is_some() && !self.is_finished()
    }

    /// Get the name of this thread.
    pub fn name(&self) -> &str {
        &self.name
    }
}

impl<T> Drop for ThreadFunction<T> {
    fn drop(&mut self) {
        if let Some(handle) = self.handle.take() {
            // Wait for thread to finish on drop
            let _ = handle.join();
        }
    }
}

/// A spin lock for low-latency synchronization.
///
/// Use this when you need very short critical sections.
pub struct SpinLock {
    locked: AtomicBool,
}

impl SpinLock {
    /// Create a new unlocked spin lock.
    pub const fn new() -> Self {
        SpinLock {
            locked: AtomicBool::new(false),
        }
    }

    /// Acquire the lock, spinning until it's available.
    pub fn lock(&self) {
        while self
            .locked
            .compare_exchange_weak(false, true, Ordering::Acquire, Ordering::Relaxed)
            .is_err()
        {
            std::hint::spin_loop();
        }
    }

    /// Try to acquire the lock without blocking.
    ///
    /// Returns `true` if the lock was acquired.
    pub fn try_lock(&self) -> bool {
        self.locked
            .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
            .is_ok()
    }

    /// Release the lock.
    pub fn unlock(&self) {
        self.locked.store(false, Ordering::Release);
    }

    /// Execute a function while holding the lock.
    pub fn with_lock<F, R>(&self, f: F) -> R
    where
        F: FnOnce() -> R,
    {
        self.lock();
        let result = f();
        self.unlock();
        result
    }
}

impl Default for SpinLock {
    fn default() -> Self {
        Self::new()
    }
}

// SpinLock is safe to share between threads
unsafe impl Send for SpinLock {}
unsafe impl Sync for SpinLock {}

/// A readers-writer lock that allows multiple readers or one writer.
pub struct ReadersWriterLock<T> {
    inner: RwLock<T>,
}

impl<T> ReadersWriterLock<T> {
    /// Create a new readers-writer lock.
    pub fn new(value: T) -> Self {
        ReadersWriterLock {
            inner: RwLock::new(value),
        }
    }

    /// Acquire a read lock.
    pub fn read(&self) -> std::sync::RwLockReadGuard<'_, T> {
        self.inner.read().expect("RwLock poisoned")
    }

    /// Try to acquire a read lock without blocking.
    pub fn try_read(&self) -> Option<std::sync::RwLockReadGuard<'_, T>> {
        self.inner.try_read().ok()
    }

    /// Acquire a write lock.
    pub fn write(&self) -> std::sync::RwLockWriteGuard<'_, T> {
        self.inner.write().expect("RwLock poisoned")
    }

    /// Try to acquire a write lock without blocking.
    pub fn try_write(&self) -> Option<std::sync::RwLockWriteGuard<'_, T>> {
        self.inner.try_write().ok()
    }
}

/// A condition variable for thread synchronization.
pub struct ConditionVariable {
    cvar: Condvar,
    mutex: StdMutex<bool>,
}

impl ConditionVariable {
    /// Create a new condition variable.
    pub fn new() -> Self {
        ConditionVariable {
            cvar: Condvar::new(),
            mutex: StdMutex::new(false),
        }
    }

    /// Wait on the condition variable.
    ///
    /// # Arguments
    /// * `timeout_millis` - Optional timeout in milliseconds
    pub fn wait(&self, timeout_millis: Option<u64>) {
        let guard = self.mutex.lock().expect("Mutex poisoned");

        if let Some(millis) = timeout_millis {
            drop(self
                .cvar
                .wait_timeout(guard, Duration::from_millis(millis)));
        } else {
            drop(self.cvar.wait(guard));
        }
    }

    /// Signal one waiting thread.
    pub fn signal(&self) {
        self.cvar.notify_one();
    }

    /// Signal all waiting threads.
    pub fn broadcast(&self) {
        self.cvar.notify_all();
    }
}

impl Default for ConditionVariable {
    fn default() -> Self {
        Self::new()
    }
}

/// An atomic counter for thread-safe counting.
pub struct AtomicCounter {
    value: AtomicUsize,
}

impl AtomicCounter {
    /// Create a new counter with the given initial value.
    pub const fn new(initial: usize) -> Self {
        AtomicCounter {
            value: AtomicUsize::new(initial),
        }
    }

    /// Get the current value.
    pub fn get(&self) -> usize {
        self.value.load(Ordering::SeqCst)
    }

    /// Set the value.
    pub fn set(&self, value: usize) {
        self.value.store(value, Ordering::SeqCst);
    }

    /// Increment and return the new value.
    pub fn increment(&self) -> usize {
        self.value.fetch_add(1, Ordering::SeqCst) + 1
    }

    /// Decrement and return the new value.
    pub fn decrement(&self) -> usize {
        self.value.fetch_sub(1, Ordering::SeqCst) - 1
    }

    /// Add to the counter and return the previous value.
    pub fn fetch_add(&self, val: usize) -> usize {
        self.value.fetch_add(val, Ordering::SeqCst)
    }

    /// Subtract from the counter and return the previous value.
    pub fn fetch_sub(&self, val: usize) -> usize {
        self.value.fetch_sub(val, Ordering::SeqCst)
    }
}

impl Default for AtomicCounter {
    fn default() -> Self {
        Self::new(0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_thread_sleep() {
        let start = std::time::Instant::now();
        Thread::sleep(10);
        assert!(start.elapsed() >= Duration::from_millis(10));
    }

    #[test]
    fn test_thread_yield() {
        Thread::yield_now(); // Should not panic
    }

    #[test]
    fn test_number_of_processors() {
        let count = Thread::number_of_processors();
        assert!(count >= 1);
    }

    #[test]
    fn test_thread_function() {
        let tf = ThreadFunction::new("test", || 42);
        let result = tf.finish();
        assert_eq!(result, 42);
    }

    #[test]
    fn test_thread_function_is_finished() {
        let tf = ThreadFunction::new("test", || {
            Thread::sleep(50);
            42
        });

        // May or may not be finished immediately
        Thread::sleep(100);
        assert!(tf.is_finished());

        let result = tf.finish();
        assert_eq!(result, 42);
    }

    #[test]
    fn test_spin_lock() {
        let lock = SpinLock::new();

        lock.lock();
        assert!(!lock.try_lock());
        lock.unlock();

        assert!(lock.try_lock());
        lock.unlock();
    }

    #[test]
    fn test_spin_lock_with_lock() {
        let lock = SpinLock::new();
        let result = lock.with_lock(|| 42);
        assert_eq!(result, 42);
    }

    #[test]
    fn test_readers_writer_lock() {
        let lock = ReadersWriterLock::new(42);

        {
            let read = lock.read();
            assert_eq!(*read, 42);
        }

        {
            let mut write = lock.write();
            *write = 100;
        }

        {
            let read = lock.read();
            assert_eq!(*read, 100);
        }
    }

    #[test]
    fn test_condition_variable() {
        let cv = ConditionVariable::new();

        // Test signal (should not block indefinitely with timeout)
        cv.wait(Some(10));
        cv.signal();
        cv.broadcast();
    }

    #[test]
    fn test_atomic_counter() {
        let counter = AtomicCounter::new(0);

        assert_eq!(counter.get(), 0);
        assert_eq!(counter.increment(), 1);
        assert_eq!(counter.increment(), 2);
        assert_eq!(counter.get(), 2);
        assert_eq!(counter.decrement(), 1);
        assert_eq!(counter.get(), 1);

        counter.set(100);
        assert_eq!(counter.get(), 100);
    }
}
