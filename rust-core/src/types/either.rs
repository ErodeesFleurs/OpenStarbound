//! Either type compatible with C++ Star::Either
//!
//! A container that holds exactly one of two possible types.

use std::fmt;

/// A container that holds exactly one of two possible types.
///
/// Similar to Rust's `Result` but without the semantic meaning of success/failure.
///
/// # Example
/// ```
/// use starbound_core::types::either::Either;
///
/// let left: Either<i32, String> = Either::new_left(42);
/// let right: Either<i32, String> = Either::new_right("hello".to_string());
///
/// assert!(left.is_left());
/// assert!(right.is_right());
/// assert_eq!(left.left(), Some(&42));
/// assert_eq!(right.right(), Some(&"hello".to_string()));
/// ```
#[derive(Clone, PartialEq, Eq, Hash)]
pub enum Either<L, R> {
    /// Left variant
    Left(L),
    /// Right variant
    Right(R),
}

impl<L, R> Either<L, R> {
    /// Create a Left variant.
    pub fn new_left(value: L) -> Self {
        Either::Left(value)
    }

    /// Create a Right variant.
    pub fn new_right(value: R) -> Self {
        Either::Right(value)
    }

    /// Returns true if this is a Left variant.
    pub fn is_left(&self) -> bool {
        matches!(self, Either::Left(_))
    }

    /// Returns true if this is a Right variant.
    pub fn is_right(&self) -> bool {
        matches!(self, Either::Right(_))
    }

    /// Get a reference to the left value, if present.
    pub fn left(&self) -> Option<&L> {
        match self {
            Either::Left(ref l) => Some(l),
            Either::Right(_) => None,
        }
    }

    /// Get a reference to the right value, if present.
    pub fn right(&self) -> Option<&R> {
        match self {
            Either::Left(_) => None,
            Either::Right(ref r) => Some(r),
        }
    }

    /// Get a mutable reference to the left value, if present.
    pub fn left_mut(&mut self) -> Option<&mut L> {
        match self {
            Either::Left(ref mut l) => Some(l),
            Either::Right(_) => None,
        }
    }

    /// Get a mutable reference to the right value, if present.
    pub fn right_mut(&mut self) -> Option<&mut R> {
        match self {
            Either::Left(_) => None,
            Either::Right(ref mut r) => Some(r),
        }
    }

    /// Unwrap the left value, panicking if this is a Right.
    ///
    /// # Panics
    /// Panics if this is a Right variant.
    pub fn unwrap_left(self) -> L {
        match self {
            Either::Left(l) => l,
            Either::Right(_) => panic!("Called unwrap_left on Right variant"),
        }
    }

    /// Unwrap the right value, panicking if this is a Left.
    ///
    /// # Panics
    /// Panics if this is a Left variant.
    pub fn unwrap_right(self) -> R {
        match self {
            Either::Left(_) => panic!("Called unwrap_right on Left variant"),
            Either::Right(r) => r,
        }
    }

    /// Map the left value with a function.
    pub fn map_left<L2, F: FnOnce(L) -> L2>(self, f: F) -> Either<L2, R> {
        match self {
            Either::Left(l) => Either::Left(f(l)),
            Either::Right(r) => Either::Right(r),
        }
    }

    /// Map the right value with a function.
    pub fn map_right<R2, F: FnOnce(R) -> R2>(self, f: F) -> Either<L, R2> {
        match self {
            Either::Left(l) => Either::Left(l),
            Either::Right(r) => Either::Right(f(r)),
        }
    }

    /// Flip the Either, swapping Left and Right.
    pub fn flip(self) -> Either<R, L> {
        match self {
            Either::Left(l) => Either::Right(l),
            Either::Right(r) => Either::Left(r),
        }
    }

    /// Apply a function to either variant.
    pub fn either<T, FL: FnOnce(L) -> T, FR: FnOnce(R) -> T>(self, f_left: FL, f_right: FR) -> T {
        match self {
            Either::Left(l) => f_left(l),
            Either::Right(r) => f_right(r),
        }
    }

    /// Get the left value or a default.
    pub fn left_or(self, default: L) -> L {
        match self {
            Either::Left(l) => l,
            Either::Right(_) => default,
        }
    }

    /// Get the right value or a default.
    pub fn right_or(self, default: R) -> R {
        match self {
            Either::Left(_) => default,
            Either::Right(r) => r,
        }
    }

    /// Get the left value or compute it from the right.
    pub fn left_or_else<F: FnOnce(R) -> L>(self, f: F) -> L {
        match self {
            Either::Left(l) => l,
            Either::Right(r) => f(r),
        }
    }

    /// Get the right value or compute it from the left.
    pub fn right_or_else<F: FnOnce(L) -> R>(self, f: F) -> R {
        match self {
            Either::Left(l) => f(l),
            Either::Right(r) => r,
        }
    }
}

impl<L, R> Default for Either<L, R>
where
    L: Default,
{
    fn default() -> Self {
        Either::Left(L::default())
    }
}

impl<L: fmt::Debug, R: fmt::Debug> fmt::Debug for Either<L, R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Either::Left(l) => f.debug_tuple("Left").field(l).finish(),
            Either::Right(r) => f.debug_tuple("Right").field(r).finish(),
        }
    }
}

impl<L: fmt::Display, R: fmt::Display> fmt::Display for Either<L, R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Either::Left(l) => write!(f, "{}", l),
            Either::Right(r) => write!(f, "{}", r),
        }
    }
}

/// Convert from Result to Either (Ok -> Right, Err -> Left)
impl<L, R> From<Result<R, L>> for Either<L, R> {
    fn from(result: Result<R, L>) -> Self {
        match result {
            Ok(r) => Either::Right(r),
            Err(l) => Either::Left(l),
        }
    }
}

/// Convert Either to Result (Right -> Ok, Left -> Err)
impl<L, R> From<Either<L, R>> for Result<R, L> {
    fn from(either: Either<L, R>) -> Self {
        match either {
            Either::Left(l) => Err(l),
            Either::Right(r) => Ok(r),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_left_right() {
        let left: Either<i32, String> = Either::new_left(42);
        let right: Either<i32, String> = Either::new_right("hello".to_string());

        assert!(left.is_left());
        assert!(!left.is_right());
        assert!(!right.is_left());
        assert!(right.is_right());
    }

    #[test]
    fn test_accessors() {
        let left: Either<i32, String> = Either::new_left(42);
        let right: Either<i32, String> = Either::new_right("hello".to_string());

        assert_eq!(left.left(), Some(&42));
        assert_eq!(left.right(), None);
        assert_eq!(right.left(), None);
        assert_eq!(right.right(), Some(&"hello".to_string()));
    }

    #[test]
    fn test_unwrap() {
        let left: Either<i32, String> = Either::new_left(42);
        let right: Either<i32, String> = Either::new_right("hello".to_string());

        assert_eq!(left.unwrap_left(), 42);
        assert_eq!(right.unwrap_right(), "hello".to_string());
    }

    #[test]
    #[should_panic]
    fn test_unwrap_left_panic() {
        let right: Either<i32, String> = Either::new_right("hello".to_string());
        right.unwrap_left();
    }

    #[test]
    fn test_map() {
        let left: Either<i32, String> = Either::new_left(42);
        let right: Either<i32, String> = Either::new_right("hello".to_string());

        let left_mapped = left.map_left(|x| x * 2);
        let right_mapped = right.map_right(|s| s.to_uppercase());

        assert_eq!(left_mapped.left(), Some(&84));
        assert_eq!(right_mapped.right(), Some(&"HELLO".to_string()));
    }

    #[test]
    fn test_flip() {
        let left: Either<i32, String> = Either::new_left(42);
        let flipped = left.flip();

        assert!(flipped.is_right());
        assert_eq!(flipped.right(), Some(&42));
    }

    #[test]
    fn test_either_fn() {
        let left: Either<i32, String> = Either::new_left(42);
        let right: Either<i32, String> = Either::new_right("hello".to_string());

        let left_result = left.either(|x| x.to_string(), |s| s);
        let right_result = right.either(|x| x.to_string(), |s| s);

        assert_eq!(left_result, "42");
        assert_eq!(right_result, "hello");
    }

    #[test]
    fn test_from_result() {
        let ok: Result<i32, String> = Ok(42);
        let err: Result<i32, String> = Err("error".to_string());

        let either_ok: Either<String, i32> = ok.into();
        let either_err: Either<String, i32> = err.into();

        assert!(either_ok.is_right());
        assert!(either_err.is_left());
    }

    #[test]
    fn test_default() {
        let either: Either<i32, String> = Either::default();
        assert!(either.is_left());
        assert_eq!(either.left(), Some(&0));
    }
}
