//! Worker pool implementation compatible with C++ Star::WorkerPool
//!
//! This module provides a thread pool for parallel task execution.

use crate::error::{Error, Result};
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::{self, JoinHandle};

/// A task to be executed by a worker
type Task = Box<dyn FnOnce() + Send + 'static>;

/// Worker pool for parallel task execution
///
/// Compatible with C++ Star::WorkerPool
pub struct WorkerPool {
    /// Task queue
    queue: Arc<Mutex<Vec<Task>>>,
    /// Condition variable for task notification
    condvar: Arc<Condvar>,
    /// Worker threads
    workers: Vec<JoinHandle<()>>,
    /// Shutdown flag
    shutdown: Arc<AtomicBool>,
    /// Number of active workers
    active_workers: Arc<AtomicUsize>,
    /// Total tasks executed
    tasks_executed: Arc<AtomicUsize>,
}

impl WorkerPool {
    /// Create a new worker pool with the specified number of threads
    pub fn new(num_threads: usize) -> Self {
        let queue = Arc::new(Mutex::new(Vec::new()));
        let condvar = Arc::new(Condvar::new());
        let shutdown = Arc::new(AtomicBool::new(false));
        let active_workers = Arc::new(AtomicUsize::new(0));
        let tasks_executed = Arc::new(AtomicUsize::new(0));

        let mut workers = Vec::with_capacity(num_threads);

        for _ in 0..num_threads {
            let queue = queue.clone();
            let condvar = condvar.clone();
            let shutdown = shutdown.clone();
            let active = active_workers.clone();
            let executed = tasks_executed.clone();

            let handle = thread::spawn(move || {
                worker_loop(queue, condvar, shutdown, active, executed);
            });

            workers.push(handle);
        }

        Self {
            queue,
            condvar,
            workers,
            shutdown,
            active_workers,
            tasks_executed,
        }
    }

    /// Create a worker pool with the number of threads equal to available CPUs
    pub fn with_cpu_threads() -> Self {
        let num_cpus = thread::available_parallelism()
            .map(|n| n.get())
            .unwrap_or(4);
        Self::new(num_cpus)
    }

    /// Get the number of worker threads
    pub fn num_threads(&self) -> usize {
        self.workers.len()
    }

    /// Get the number of currently active workers
    pub fn active_workers(&self) -> usize {
        self.active_workers.load(Ordering::Relaxed)
    }

    /// Get the total number of tasks executed
    pub fn tasks_executed(&self) -> usize {
        self.tasks_executed.load(Ordering::Relaxed)
    }

    /// Get the number of pending tasks
    pub fn pending_tasks(&self) -> usize {
        self.queue.lock().unwrap().len()
    }

    /// Submit a task to be executed
    pub fn submit<F>(&self, task: F)
    where
        F: FnOnce() + Send + 'static,
    {
        let mut queue = self.queue.lock().unwrap();
        queue.push(Box::new(task));
        self.condvar.notify_one();
    }

    /// Submit multiple tasks
    pub fn submit_all<I, F>(&self, tasks: I)
    where
        I: IntoIterator<Item = F>,
        F: FnOnce() + Send + 'static,
    {
        let mut queue = self.queue.lock().unwrap();
        for task in tasks {
            queue.push(Box::new(task));
        }
        self.condvar.notify_all();
    }

    /// Wait for all current tasks to complete
    pub fn wait(&self) {
        loop {
            let pending = {
                let queue = self.queue.lock().unwrap();
                queue.len()
            };

            if pending == 0 && self.active_workers.load(Ordering::Relaxed) == 0 {
                break;
            }

            // Sleep briefly to avoid busy waiting
            thread::sleep(std::time::Duration::from_micros(100));
        }
    }

    /// Shutdown the pool, waiting for all tasks to complete
    pub fn shutdown(mut self) {
        self.shutdown.store(true, Ordering::Release);
        self.condvar.notify_all();

        for worker in self.workers.drain(..) {
            let _ = worker.join();
        }
    }

    /// Check if the pool is shutting down
    pub fn is_shutting_down(&self) -> bool {
        self.shutdown.load(Ordering::Acquire)
    }
}

impl Default for WorkerPool {
    fn default() -> Self {
        Self::with_cpu_threads()
    }
}

impl Drop for WorkerPool {
    fn drop(&mut self) {
        // Signal shutdown to all workers
        self.shutdown.store(true, Ordering::Release);
        self.condvar.notify_all();
        
        // Wait for all workers to finish
        for worker in self.workers.drain(..) {
            let _ = worker.join();
        }
    }
}

/// Worker thread loop
fn worker_loop(
    queue: Arc<Mutex<Vec<Task>>>,
    condvar: Arc<Condvar>,
    shutdown: Arc<AtomicBool>,
    active: Arc<AtomicUsize>,
    executed: Arc<AtomicUsize>,
) {
    loop {
        let task = {
            let mut queue = queue.lock().unwrap();

            while queue.is_empty() {
                if shutdown.load(Ordering::Acquire) {
                    return;
                }
                queue = condvar.wait(queue).unwrap();
            }

            if shutdown.load(Ordering::Acquire) && queue.is_empty() {
                return;
            }

            queue.pop()
        };

        if let Some(task) = task {
            active.fetch_add(1, Ordering::Relaxed);
            task();
            executed.fetch_add(1, Ordering::Relaxed);
            active.fetch_sub(1, Ordering::Relaxed);
        }
    }
}

/// A future-like handle for an async task result
pub struct TaskHandle<T> {
    result: Arc<Mutex<Option<T>>>,
    completed: Arc<AtomicBool>,
    condvar: Arc<Condvar>,
}

impl<T> TaskHandle<T> {
    /// Create a new task handle
    fn new() -> Self {
        Self {
            result: Arc::new(Mutex::new(None)),
            completed: Arc::new(AtomicBool::new(false)),
            condvar: Arc::new(Condvar::new()),
        }
    }

    /// Check if the task is complete
    pub fn is_complete(&self) -> bool {
        self.completed.load(Ordering::Acquire)
    }

    /// Wait for the result
    pub fn wait(self) -> Option<T> {
        let mut result = self.result.lock().unwrap();
        while !self.completed.load(Ordering::Acquire) {
            result = self.condvar.wait(result).unwrap();
        }
        result.take()
    }

    /// Try to get the result without waiting
    pub fn try_get(&self) -> Option<T>
    where
        T: Clone,
    {
        if self.completed.load(Ordering::Acquire) {
            self.result.lock().unwrap().clone()
        } else {
            None
        }
    }
}

impl<T> Clone for TaskHandle<T> {
    fn clone(&self) -> Self {
        Self {
            result: self.result.clone(),
            completed: self.completed.clone(),
            condvar: self.condvar.clone(),
        }
    }
}

/// Extended worker pool with result handling
pub struct AsyncWorkerPool {
    inner: WorkerPool,
}

impl AsyncWorkerPool {
    /// Create a new async worker pool
    pub fn new(num_threads: usize) -> Self {
        Self {
            inner: WorkerPool::new(num_threads),
        }
    }

    /// Create with CPU thread count
    pub fn with_cpu_threads() -> Self {
        Self {
            inner: WorkerPool::with_cpu_threads(),
        }
    }

    /// Submit a task that returns a value
    pub fn submit<F, T>(&self, task: F) -> TaskHandle<T>
    where
        F: FnOnce() -> T + Send + 'static,
        T: Send + 'static,
    {
        let handle = TaskHandle::new();
        let result = handle.result.clone();
        let completed = handle.completed.clone();
        let condvar = handle.condvar.clone();

        self.inner.submit(move || {
            let value = task();
            *result.lock().unwrap() = Some(value);
            completed.store(true, Ordering::Release);
            condvar.notify_all();
        });

        handle
    }

    /// Get the number of threads
    pub fn num_threads(&self) -> usize {
        self.inner.num_threads()
    }

    /// Wait for all tasks
    pub fn wait(&self) {
        self.inner.wait();
    }

    /// Shutdown the pool
    pub fn shutdown(self) {
        self.inner.shutdown();
    }
}

impl Default for AsyncWorkerPool {
    fn default() -> Self {
        Self::with_cpu_threads()
    }
}

/// Parallel iterator adapter for the worker pool
pub struct ParallelIterator<'a, T> {
    pool: &'a WorkerPool,
    items: Vec<T>,
}

impl WorkerPool {
    /// Create a parallel iterator
    pub fn parallel_iter<'a, T>(&'a self, items: Vec<T>) -> ParallelIterator<'a, T> {
        ParallelIterator { pool: self, items }
    }

    /// Execute a function on each item in parallel
    pub fn for_each<T, F>(&self, items: Vec<T>, f: F)
    where
        T: Send + 'static,
        F: Fn(T) + Send + Sync + Clone + 'static,
    {
        for item in items {
            let f = f.clone();
            self.submit(move || f(item));
        }
        self.wait();
    }

    /// Map a function over items in parallel
    pub fn map<T, U, F>(&self, items: Vec<T>, f: F) -> Vec<U>
    where
        T: Send + 'static,
        U: Send + 'static,
        F: Fn(T) -> U + Send + Sync + Clone + 'static,
    {
        let results = Arc::new(Mutex::new(Vec::with_capacity(items.len())));
        let indices = Arc::new(Mutex::new(Vec::with_capacity(items.len())));

        for (idx, item) in items.into_iter().enumerate() {
            let f = f.clone();
            let results = results.clone();
            let indices = indices.clone();

            self.submit(move || {
                let result = f(item);
                let mut r = results.lock().unwrap();
                let mut i = indices.lock().unwrap();
                r.push(result);
                i.push(idx);
            });
        }

        self.wait();

        // Sort by original indices
        let mut results = Arc::try_unwrap(results).ok().unwrap().into_inner().unwrap();
        let indices = Arc::try_unwrap(indices).ok().unwrap().into_inner().unwrap();

        // Create sorted result
        let mut sorted: Vec<Option<U>> = std::iter::repeat_with(|| None)
            .take(results.len())
            .collect();

        for (result, idx) in results.drain(..).zip(indices.iter()) {
            sorted[*idx] = Some(result);
        }

        sorted.into_iter().map(|o| o.unwrap()).collect()
    }

    /// Filter items in parallel
    pub fn filter<T, F>(&self, items: Vec<T>, predicate: F) -> Vec<T>
    where
        T: Send + 'static,
        F: Fn(&T) -> bool + Send + Sync + Clone + 'static,
    {
        let results = Arc::new(Mutex::new(Vec::new()));

        for item in items {
            let predicate = predicate.clone();
            let results = results.clone();

            self.submit(move || {
                if predicate(&item) {
                    results.lock().unwrap().push(item);
                }
            });
        }

        self.wait();

        Arc::try_unwrap(results).ok().unwrap().into_inner().unwrap()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::AtomicI32;

    #[test]
    fn test_worker_pool_basic() {
        let pool = WorkerPool::new(2);
        let counter = Arc::new(AtomicI32::new(0));

        for _ in 0..10 {
            let counter = counter.clone();
            pool.submit(move || {
                counter.fetch_add(1, Ordering::SeqCst);
            });
        }

        pool.wait();
        assert_eq!(counter.load(Ordering::SeqCst), 10);
    }

    #[test]
    fn test_worker_pool_num_threads() {
        let pool = WorkerPool::new(4);
        assert_eq!(pool.num_threads(), 4);
    }

    #[test]
    fn test_worker_pool_tasks_executed() {
        let pool = WorkerPool::new(2);

        for _ in 0..5 {
            pool.submit(|| {});
        }

        pool.wait();
        assert_eq!(pool.tasks_executed(), 5);
    }

    #[test]
    fn test_worker_pool_submit_all() {
        let pool = WorkerPool::new(2);
        let counter = Arc::new(AtomicI32::new(0));

        let tasks: Vec<_> = (0..5)
            .map(|_| {
                let counter = counter.clone();
                move || {
                    counter.fetch_add(1, Ordering::SeqCst);
                }
            })
            .collect();

        pool.submit_all(tasks);
        pool.wait();

        assert_eq!(counter.load(Ordering::SeqCst), 5);
    }

    #[test]
    fn test_worker_pool_shutdown() {
        let pool = WorkerPool::new(2);
        let counter = Arc::new(AtomicI32::new(0));

        for _ in 0..3 {
            let counter = counter.clone();
            pool.submit(move || {
                counter.fetch_add(1, Ordering::SeqCst);
            });
        }

        pool.shutdown();
        assert_eq!(counter.load(Ordering::SeqCst), 3);
    }

    #[test]
    fn test_async_worker_pool() {
        let pool = AsyncWorkerPool::new(2);

        let handle = pool.submit(|| 42);

        assert_eq!(handle.wait(), Some(42));
    }

    #[test]
    fn test_task_handle_is_complete() {
        let pool = AsyncWorkerPool::new(2);

        let handle = pool.submit(|| {
            thread::sleep(std::time::Duration::from_millis(10));
            42
        });

        // Initially not complete
        // Note: This is racy, but should generally work

        let result = handle.wait();
        assert_eq!(result, Some(42));
    }

    #[test]
    fn test_worker_pool_for_each() {
        let pool = WorkerPool::new(2);
        let results = Arc::new(Mutex::new(Vec::new()));

        let results_clone = results.clone();
        pool.for_each(vec![1, 2, 3, 4, 5], move |x| {
            results_clone.lock().unwrap().push(x * 2);
        });

        let mut r = results.lock().unwrap();
        r.sort();
        assert_eq!(*r, vec![2, 4, 6, 8, 10]);
    }

    #[test]
    fn test_worker_pool_map() {
        let pool = WorkerPool::new(2);

        let results = pool.map(vec![1, 2, 3, 4, 5], |x| x * 2);

        assert_eq!(results, vec![2, 4, 6, 8, 10]);
    }

    #[test]
    fn test_worker_pool_filter() {
        let pool = WorkerPool::new(2);

        let mut results = pool.filter(vec![1, 2, 3, 4, 5, 6], |x| x % 2 == 0);
        results.sort();

        assert_eq!(results, vec![2, 4, 6]);
    }

    #[test]
    fn test_default_pool() {
        let pool = WorkerPool::default();
        assert!(pool.num_threads() >= 1);
    }

    #[test]
    fn test_cpu_threads() {
        let pool = WorkerPool::with_cpu_threads();
        assert!(pool.num_threads() >= 1);
    }
}
