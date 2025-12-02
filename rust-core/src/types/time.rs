//! Time utilities compatible with C++ Star::Time, Clock, and Timer
//!
//! Provides time measurement, monotonic clocks, and countdown timers.

use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

/// Get time since Unix epoch in seconds (floating point)
pub fn time_since_epoch() -> f64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or(Duration::ZERO)
        .as_secs_f64()
}

/// Get milliseconds since Unix epoch
pub fn milliseconds_since_epoch() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or(Duration::ZERO)
        .as_millis() as i64
}

/// Get monotonic time in seconds (floating point)
/// 
/// Uses a cached start time to provide consistent monotonic measurements.
pub fn monotonic_time() -> f64 {
    static START: std::sync::OnceLock<Instant> = std::sync::OnceLock::new();
    let start = START.get_or_init(Instant::now);
    start.elapsed().as_secs_f64()
}

/// Get monotonic time in milliseconds
pub fn monotonic_milliseconds() -> i64 {
    static START: std::sync::OnceLock<Instant> = std::sync::OnceLock::new();
    let start = START.get_or_init(Instant::now);
    start.elapsed().as_millis() as i64
}

/// Get monotonic time in microseconds
pub fn monotonic_microseconds() -> i64 {
    static START: std::sync::OnceLock<Instant> = std::sync::OnceLock::new();
    let start = START.get_or_init(Instant::now);
    start.elapsed().as_micros() as i64
}

/// Pretty print a duration of time (In days, hours, minutes, seconds, and milliseconds)
pub fn print_duration(time: f64) -> String {
    let negative = time < 0.0;
    let time = time.abs();
    
    let total_millis = (time * 1000.0) as u64;
    let millis = total_millis % 1000;
    let total_seconds = total_millis / 1000;
    let seconds = total_seconds % 60;
    let total_minutes = total_seconds / 60;
    let minutes = total_minutes % 60;
    let total_hours = total_minutes / 60;
    let hours = total_hours % 24;
    let days = total_hours / 24;
    
    let mut result = String::new();
    if negative {
        result.push('-');
    }
    
    if days > 0 {
        result.push_str(&format!("{}d ", days));
    }
    if hours > 0 || days > 0 {
        result.push_str(&format!("{}h ", hours));
    }
    if minutes > 0 || hours > 0 || days > 0 {
        result.push_str(&format!("{}m ", minutes));
    }
    result.push_str(&format!("{}.{:03}s", seconds, millis));
    
    result
}

/// Pretty print a given date and time from epoch ticks
/// 
/// Format supports: `<year>`, `<month>`, `<day>`, `<hours>`, `<minutes>`, `<seconds>`, `<millis>`
pub fn print_date_and_time(epoch_millis: i64, format: &str) -> String {
    use std::time::{Duration, UNIX_EPOCH};
    
    let duration = if epoch_millis >= 0 {
        Duration::from_millis(epoch_millis as u64)
    } else {
        Duration::ZERO
    };
    
    let datetime = UNIX_EPOCH + duration;
    
    // Simple date/time calculation (no external dependency)
    let secs = epoch_millis / 1000;
    let millis = (epoch_millis % 1000).abs();
    
    // Calculate date components (simplified, doesn't handle leap seconds)
    let days_since_epoch = secs / 86400;
    let time_of_day = (secs % 86400).abs();
    
    let hours = time_of_day / 3600;
    let minutes = (time_of_day % 3600) / 60;
    let seconds = time_of_day % 60;
    
    // Calculate year, month, day (simplified algorithm)
    let (year, month, day) = days_to_ymd(days_since_epoch as i32);
    
    format
        .replace("<year>", &format!("{:04}", year))
        .replace("<month>", &format!("{:02}", month))
        .replace("<day>", &format!("{:02}", day))
        .replace("<hours>", &format!("{:02}", hours))
        .replace("<minutes>", &format!("{:02}", minutes))
        .replace("<seconds>", &format!("{:02}", seconds))
        .replace("<millis>", &format!("{:03}", millis))
}

/// Pretty print current date and time
pub fn print_current_date_and_time(format: &str) -> String {
    print_date_and_time(milliseconds_since_epoch(), format)
}

// Helper function to convert days since epoch to year/month/day
fn days_to_ymd(days: i32) -> (i32, i32, i32) {
    // Civil calendar algorithm from Howard Hinnant
    let z = days + 719468;
    let era = if z >= 0 { z } else { z - 146096 } / 146097;
    let doe = (z - era * 146097) as u32;
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let y = yoe as i32 + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = doy - (153 * mp + 2) / 5 + 1;
    let m = if mp < 10 { mp + 3 } else { mp - 9 };
    let year = y + if m <= 2 { 1 } else { 0 };
    
    (year, m as i32, d as i32)
}

/// Convert ticks to seconds
pub fn ticks_to_seconds(ticks: i64, tick_frequency: i64) -> f64 {
    ticks as f64 / tick_frequency as f64
}

/// Convert ticks to milliseconds
pub fn ticks_to_milliseconds(ticks: i64, tick_frequency: i64) -> i64 {
    (ticks * 1000) / tick_frequency
}

/// Convert ticks to microseconds
pub fn ticks_to_microseconds(ticks: i64, tick_frequency: i64) -> i64 {
    (ticks * 1_000_000) / tick_frequency
}

/// Convert seconds to ticks
pub fn seconds_to_ticks(seconds: f64, tick_frequency: i64) -> i64 {
    (seconds * tick_frequency as f64) as i64
}

/// Convert milliseconds to ticks
pub fn milliseconds_to_ticks(milliseconds: i64, tick_frequency: i64) -> i64 {
    (milliseconds * tick_frequency) / 1000
}

/// Convert microseconds to ticks
pub fn microseconds_to_ticks(microseconds: i64, tick_frequency: i64) -> i64 {
    (microseconds * tick_frequency) / 1_000_000
}

/// A monotonically increasing clock that tracks elapsed time
/// 
/// Compatible with C++ Star::Clock. Thread-safe.
#[derive(Clone)]
pub struct Clock {
    inner: Arc<Mutex<ClockInner>>,
}

struct ClockInner {
    elapsed_micros: i64,
    last_instant: Option<Instant>,
    running: bool,
}

impl Clock {
    /// Create a new clock, optionally starting it immediately
    pub fn new(start: bool) -> Self {
        let clock = Clock {
            inner: Arc::new(Mutex::new(ClockInner {
                elapsed_micros: 0,
                last_instant: None,
                running: false,
            })),
        };
        
        if start {
            clock.start();
        }
        
        clock
    }
    
    /// Reset the clock to 0 elapsed time
    pub fn reset(&self) {
        let mut inner = self.inner.lock().unwrap();
        inner.elapsed_micros = 0;
        if inner.running {
            inner.last_instant = Some(Instant::now());
        } else {
            inner.last_instant = None;
        }
    }
    
    /// Stop the clock
    pub fn stop(&self) {
        let mut inner = self.inner.lock().unwrap();
        self.update_elapsed_locked(&mut inner);
        inner.running = false;
        inner.last_instant = None;
    }
    
    /// Start the clock
    pub fn start(&self) {
        let mut inner = self.inner.lock().unwrap();
        if !inner.running {
            inner.running = true;
            inner.last_instant = Some(Instant::now());
        }
    }
    
    /// Check if the clock is running
    pub fn running(&self) -> bool {
        let inner = self.inner.lock().unwrap();
        inner.running
    }
    
    /// Get elapsed time in seconds
    pub fn time(&self) -> f64 {
        let mut inner = self.inner.lock().unwrap();
        self.update_elapsed_locked(&mut inner);
        inner.elapsed_micros as f64 / 1_000_000.0
    }
    
    /// Get elapsed time in milliseconds
    pub fn milliseconds(&self) -> i64 {
        let mut inner = self.inner.lock().unwrap();
        self.update_elapsed_locked(&mut inner);
        inner.elapsed_micros / 1000
    }
    
    /// Set the elapsed time
    pub fn set_time(&self, time: f64) {
        let mut inner = self.inner.lock().unwrap();
        inner.elapsed_micros = (time * 1_000_000.0) as i64;
        if inner.running {
            inner.last_instant = Some(Instant::now());
        }
    }
    
    /// Set the elapsed time in milliseconds
    pub fn set_milliseconds(&self, millis: i64) {
        let mut inner = self.inner.lock().unwrap();
        inner.elapsed_micros = millis * 1000;
        if inner.running {
            inner.last_instant = Some(Instant::now());
        }
    }
    
    /// Adjust the elapsed time by the given amount (can be negative)
    pub fn adjust_time(&self, adjustment: f64) {
        let mut inner = self.inner.lock().unwrap();
        self.update_elapsed_locked(&mut inner);
        inner.elapsed_micros += (adjustment * 1_000_000.0) as i64;
    }
    
    /// Adjust the elapsed time in milliseconds
    pub fn adjust_milliseconds(&self, adjustment: i64) {
        let mut inner = self.inner.lock().unwrap();
        self.update_elapsed_locked(&mut inner);
        inner.elapsed_micros += adjustment * 1000;
    }
    
    fn update_elapsed_locked(&self, inner: &mut ClockInner) {
        if let Some(last) = inner.last_instant {
            let now = Instant::now();
            let elapsed = now.duration_since(last).as_micros() as i64;
            inner.elapsed_micros += elapsed;
            inner.last_instant = Some(now);
        }
    }
}

impl Default for Clock {
    fn default() -> Self {
        Clock::new(true)
    }
}

/// A countdown timer that tracks remaining time
/// 
/// Compatible with C++ Star::Timer. Thread-safe.
#[derive(Clone)]
pub struct Timer {
    clock: Clock,
    target_micros: Arc<Mutex<i64>>,
}

impl Timer {
    /// Create a timer with the given time remaining in seconds
    pub fn with_time(time_left: f64, start: bool) -> Self {
        let timer = Timer {
            clock: Clock::new(false),
            target_micros: Arc::new(Mutex::new((time_left * 1_000_000.0) as i64)),
        };
        
        if start {
            timer.clock.start();
        }
        
        timer
    }
    
    /// Create a timer with the given time remaining in milliseconds
    pub fn with_milliseconds(millis: i64, start: bool) -> Self {
        let timer = Timer {
            clock: Clock::new(false),
            target_micros: Arc::new(Mutex::new(millis * 1000)),
        };
        
        if start {
            timer.clock.start();
        }
        
        timer
    }
    
    /// Create a stopped timer whose time is up
    pub fn new() -> Self {
        Timer {
            clock: Clock::new(false),
            target_micros: Arc::new(Mutex::new(0)),
        }
    }
    
    /// Restart the timer with the given time left in seconds
    pub fn restart(&self, time_left: f64) {
        *self.target_micros.lock().unwrap() = (time_left * 1_000_000.0) as i64;
        self.clock.reset();
        self.clock.start();
    }
    
    /// Restart the timer with the given time left in milliseconds
    pub fn restart_with_milliseconds(&self, millis: i64) {
        *self.target_micros.lock().unwrap() = millis * 1000;
        self.clock.reset();
        self.clock.start();
    }
    
    /// Get the time remaining in seconds
    /// 
    /// If `negative` is true, returns negative values after time is up.
    /// If `negative` is false, stops at zero.
    pub fn time_left(&self, negative: bool) -> f64 {
        let target = *self.target_micros.lock().unwrap();
        let elapsed = self.clock.time();
        let target_secs = target as f64 / 1_000_000.0;
        let remaining = target_secs - elapsed;
        
        if negative || remaining >= 0.0 {
            remaining
        } else {
            0.0
        }
    }
    
    /// Get the time remaining in milliseconds
    pub fn milliseconds_left(&self, negative: bool) -> i64 {
        let target = *self.target_micros.lock().unwrap() / 1000;
        let elapsed = self.clock.milliseconds();
        let remaining = target - elapsed;
        
        if negative || remaining >= 0 {
            remaining
        } else {
            0
        }
    }
    
    /// Check if the time is up (remaining <= 0)
    pub fn time_up(&self) -> bool {
        self.time_left(false) <= 0.0
    }
    
    /// Stop the timer
    pub fn stop(&self) {
        self.clock.stop();
    }
    
    /// Start the timer
    pub fn start(&self) {
        self.clock.start();
    }
    
    /// Check if the timer is running
    pub fn running(&self) -> bool {
        self.clock.running()
    }
}

impl Default for Timer {
    fn default() -> Self {
        Timer::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::thread::sleep;
    use std::time::Duration;
    
    #[test]
    fn test_time_since_epoch() {
        let t = time_since_epoch();
        assert!(t > 0.0);
    }
    
    #[test]
    fn test_milliseconds_since_epoch() {
        let ms = milliseconds_since_epoch();
        assert!(ms > 0);
    }
    
    #[test]
    fn test_monotonic_time() {
        let t1 = monotonic_time();
        sleep(Duration::from_millis(10));
        let t2 = monotonic_time();
        assert!(t2 > t1);
    }
    
    #[test]
    fn test_print_duration() {
        assert_eq!(print_duration(0.0), "0.000s");
        assert_eq!(print_duration(1.5), "1.500s");
        assert_eq!(print_duration(61.0), "1m 1.000s");
        assert_eq!(print_duration(3661.5), "1h 1m 1.500s");
        assert_eq!(print_duration(90061.0), "1d 1h 1m 1.000s");
    }
    
    #[test]
    fn test_print_duration_negative() {
        let s = print_duration(-5.0);
        assert!(s.starts_with('-'));
    }
    
    #[test]
    fn test_clock_basic() {
        let clock = Clock::new(true);
        assert!(clock.running());
        
        sleep(Duration::from_millis(50));
        let t = clock.time();
        assert!(t >= 0.04 && t < 0.2);
    }
    
    #[test]
    fn test_clock_stop_start() {
        let clock = Clock::new(true);
        sleep(Duration::from_millis(20));
        clock.stop();
        
        let t1 = clock.time();
        sleep(Duration::from_millis(20));
        let t2 = clock.time();
        
        // Time should not advance while stopped
        assert!((t1 - t2).abs() < 0.001);
        
        clock.start();
        sleep(Duration::from_millis(20));
        let t3 = clock.time();
        
        // Time should advance after restart
        assert!(t3 > t1);
    }
    
    #[test]
    fn test_clock_reset() {
        let clock = Clock::new(true);
        sleep(Duration::from_millis(20));
        
        clock.reset();
        let t = clock.time();
        assert!(t < 0.01);
    }
    
    #[test]
    fn test_clock_set_time() {
        let clock = Clock::new(false);
        clock.set_time(5.0);
        
        let t = clock.time();
        assert!((t - 5.0).abs() < 0.01);
    }
    
    #[test]
    fn test_clock_adjust_time() {
        let clock = Clock::new(false);
        clock.set_time(5.0);
        clock.adjust_time(2.5);
        
        let t = clock.time();
        assert!((t - 7.5).abs() < 0.01);
    }
    
    #[test]
    fn test_timer_basic() {
        let timer = Timer::with_time(0.1, true);
        
        assert!(!timer.time_up());
        assert!(timer.time_left(false) > 0.0);
        
        sleep(Duration::from_millis(150));
        
        assert!(timer.time_up());
    }
    
    #[test]
    fn test_timer_negative() {
        let timer = Timer::with_time(0.05, true);
        sleep(Duration::from_millis(100));
        
        // With negative=false, should return 0
        assert_eq!(timer.time_left(false), 0.0);
        
        // With negative=true, should return negative value
        assert!(timer.time_left(true) < 0.0);
    }
    
    #[test]
    fn test_timer_restart() {
        let timer = Timer::with_time(0.05, true);
        sleep(Duration::from_millis(100));
        assert!(timer.time_up());
        
        timer.restart(1.0);
        assert!(!timer.time_up());
        assert!(timer.time_left(false) > 0.9);
    }
    
    #[test]
    fn test_timer_stopped() {
        let timer = Timer::new();
        assert!(!timer.running());
        assert!(timer.time_up());
    }
    
    #[test]
    fn test_tick_conversions() {
        let freq = 1000; // 1000 ticks per second
        
        assert!((ticks_to_seconds(500, freq) - 0.5).abs() < 0.001);
        assert_eq!(ticks_to_milliseconds(500, freq), 500);
        assert_eq!(ticks_to_microseconds(500, freq), 500000);
        
        assert_eq!(seconds_to_ticks(0.5, freq), 500);
        assert_eq!(milliseconds_to_ticks(500, freq), 500);
        assert_eq!(microseconds_to_ticks(500000, freq), 500);
    }
}
