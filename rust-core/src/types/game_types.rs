//! Game types compatible with C++ Star::GameTypes
//!
//! Provides fundamental game enums and types used throughout Starbound.

use std::ops::Neg;

/// Direction enum (Left or Right)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum Direction {
    #[default]
    Left = 0,
    Right = 1,
}

impl Direction {
    /// Parse direction from string
    pub fn from_str(s: &str) -> Option<Direction> {
        match s.to_lowercase().as_str() {
            "left" => Some(Direction::Left),
            "right" => Some(Direction::Right),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            Direction::Left => "left",
            Direction::Right => "right",
        }
    }
    
    /// Get numerical direction (-1 for Left, 1 for Right)
    pub fn numerical(&self) -> i32 {
        match self {
            Direction::Left => -1,
            Direction::Right => 1,
        }
    }
}

impl Neg for Direction {
    type Output = Direction;
    
    fn neg(self) -> Direction {
        match self {
            Direction::Left => Direction::Right,
            Direction::Right => Direction::Left,
        }
    }
}

impl std::fmt::Display for Direction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Get direction from a numerical value
pub fn direction_of<T: PartialOrd + Default>(n: T) -> Option<Direction> {
    if n == T::default() {
        None
    } else if n < T::default() {
        Some(Direction::Left)
    } else {
        Some(Direction::Right)
    }
}

/// Get numerical direction from optional direction (0 if None)
pub fn numerical_direction(direction: Option<Direction>) -> i32 {
    direction.map(|d| d.numerical()).unwrap_or(0)
}

/// Gender enum
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum Gender {
    #[default]
    Male = 0,
    Female = 1,
}

impl Gender {
    /// Parse gender from string
    pub fn from_str(s: &str) -> Option<Gender> {
        match s.to_lowercase().as_str() {
            "male" => Some(Gender::Male),
            "female" => Some(Gender::Female),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            Gender::Male => "male",
            Gender::Female => "female",
        }
    }
}

impl std::fmt::Display for Gender {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Fire mode for weapons
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum FireMode {
    #[default]
    None = 0,
    Primary = 1,
    Alt = 2,
}

impl FireMode {
    /// Parse fire mode from string
    pub fn from_str(s: &str) -> Option<FireMode> {
        match s.to_lowercase().as_str() {
            "none" => Some(FireMode::None),
            "primary" => Some(FireMode::Primary),
            "alt" => Some(FireMode::Alt),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            FireMode::None => "none",
            FireMode::Primary => "primary",
            FireMode::Alt => "alt",
        }
    }
}

/// Tool hand (primary or alt)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum ToolHand {
    #[default]
    Primary = 0,
    Alt = 1,
}

impl ToolHand {
    /// Parse tool hand from string
    pub fn from_str(s: &str) -> Option<ToolHand> {
        match s.to_lowercase().as_str() {
            "primary" => Some(ToolHand::Primary),
            "alt" => Some(ToolHand::Alt),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            ToolHand::Primary => "primary",
            ToolHand::Alt => "alt",
        }
    }
}

/// Tile layer (foreground or background)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum TileLayer {
    #[default]
    Foreground = 0,
    Background = 1,
}

impl TileLayer {
    /// Parse tile layer from string
    pub fn from_str(s: &str) -> Option<TileLayer> {
        match s.to_lowercase().as_str() {
            "foreground" => Some(TileLayer::Foreground),
            "background" => Some(TileLayer::Background),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            TileLayer::Foreground => "foreground",
            TileLayer::Background => "background",
        }
    }
}

/// Movement control types
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum MoveControlType {
    Left = 0,
    Right = 1,
    Down = 2,
    Up = 3,
    Jump = 4,
}

impl MoveControlType {
    /// Parse move control type from string
    pub fn from_str(s: &str) -> Option<MoveControlType> {
        match s.to_lowercase().as_str() {
            "left" => Some(MoveControlType::Left),
            "right" => Some(MoveControlType::Right),
            "down" => Some(MoveControlType::Down),
            "up" => Some(MoveControlType::Up),
            "jump" => Some(MoveControlType::Jump),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            MoveControlType::Left => "left",
            MoveControlType::Right => "right",
            MoveControlType::Down => "down",
            MoveControlType::Up => "up",
            MoveControlType::Jump => "jump",
        }
    }
}

/// Portrait mode for character portraits
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum PortraitMode {
    #[default]
    Head = 0,
    Bust = 1,
    Full = 2,
    FullNeutral = 3,
    FullNude = 4,
    FullNeutralNude = 5,
}

impl PortraitMode {
    /// Parse portrait mode from string
    pub fn from_str(s: &str) -> Option<PortraitMode> {
        match s.to_lowercase().as_str() {
            "head" => Some(PortraitMode::Head),
            "bust" => Some(PortraitMode::Bust),
            "full" => Some(PortraitMode::Full),
            "fullneutral" => Some(PortraitMode::FullNeutral),
            "fullnude" => Some(PortraitMode::FullNude),
            "fullneutralnude" => Some(PortraitMode::FullNeutralNude),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            PortraitMode::Head => "head",
            PortraitMode::Bust => "bust",
            PortraitMode::Full => "full",
            PortraitMode::FullNeutral => "fullNeutral",
            PortraitMode::FullNude => "fullNude",
            PortraitMode::FullNeutralNude => "fullNeutralNude",
        }
    }
}

/// Item rarity
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, PartialOrd, Ord)]
#[repr(u8)]
pub enum Rarity {
    #[default]
    Common = 0,
    Uncommon = 1,
    Rare = 2,
    Legendary = 3,
    Essential = 4,
}

impl Rarity {
    /// Parse rarity from string
    pub fn from_str(s: &str) -> Option<Rarity> {
        match s.to_lowercase().as_str() {
            "common" => Some(Rarity::Common),
            "uncommon" => Some(Rarity::Uncommon),
            "rare" => Some(Rarity::Rare),
            "legendary" => Some(Rarity::Legendary),
            "essential" => Some(Rarity::Essential),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            Rarity::Common => "common",
            Rarity::Uncommon => "uncommon",
            Rarity::Rare => "rare",
            Rarity::Legendary => "legendary",
            Rarity::Essential => "essential",
        }
    }
}

impl std::fmt::Display for Rarity {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Entity mode (Master or Slave)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum EntityMode {
    #[default]
    Master = 0,
    Slave = 1,
}

/// Tile damage result
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum TileDamageResult {
    #[default]
    None = 0,
    Protected = 1,
    Normal = 2,
}

// === Game Constants ===

/// Number of pixels in one tile
pub const TILE_PIXELS: u32 = 8;

/// World sector size in tiles
pub const WORLD_SECTOR_SIZE: usize = 32;

/// Entity ID type
pub type EntityId = i32;

/// Null entity ID constant
pub const NULL_ENTITY_ID: EntityId = 0;
/// Minimum server entity ID
pub const MIN_SERVER_ENTITY_ID: EntityId = 1;
/// Maximum server entity ID
pub const MAX_SERVER_ENTITY_ID: EntityId = i32::MAX;

/// Connection ID type
pub type ConnectionId = u16;

/// Server connection ID
pub const SERVER_CONNECTION_ID: ConnectionId = 0;
/// Minimum client connection ID
pub const MIN_CLIENT_CONNECTION_ID: ConnectionId = 1;
/// Maximum client connection ID
pub const MAX_CLIENT_CONNECTION_ID: ConnectionId = 32767;

/// Dungeon ID type
pub type DungeonId = u16;

/// Special dungeon IDs
pub const NO_DUNGEON_ID: DungeonId = 65535;
pub const SPAWN_DUNGEON_ID: DungeonId = 65534;
pub const BIOME_MICRO_DUNGEON_ID: DungeonId = 65533;
pub const CONSTRUCTION_DUNGEON_ID: DungeonId = 65532;
pub const DESTROYED_BLOCK_DUNGEON_ID: DungeonId = 65531;
pub const ZERO_G_DUNGEON_ID: DungeonId = 65525;
pub const PROTECTED_ZERO_G_DUNGEON_ID: DungeonId = 65524;
pub const FIRST_META_DUNGEON_ID: DungeonId = 65520;

/// Check if a dungeon ID is a real (non-meta) dungeon
pub fn is_real_dungeon(dungeon: DungeonId) -> bool {
    dungeon < FIRST_META_DUNGEON_ID
}

/// Get the entity ID space for a given connection
pub fn connection_entity_space(connection_id: ConnectionId) -> (EntityId, EntityId) {
    if connection_id == SERVER_CONNECTION_ID {
        (MIN_SERVER_ENTITY_ID, MAX_SERVER_ENTITY_ID)
    } else {
        // Client entity spaces are in negative range
        let base = -(connection_id as i32) * (MAX_SERVER_ENTITY_ID / MAX_CLIENT_CONNECTION_ID as i32);
        let end = base - (MAX_SERVER_ENTITY_ID / MAX_CLIENT_CONNECTION_ID as i32) + 1;
        (base, end)
    }
}

/// Check if an entity ID belongs to a given connection
pub fn entity_id_in_space(entity_id: EntityId, connection_id: ConnectionId) -> bool {
    let (start, end) = connection_entity_space(connection_id);
    if connection_id == SERVER_CONNECTION_ID {
        entity_id >= start && entity_id <= end
    } else {
        entity_id <= start && entity_id >= end
    }
}

/// Get the connection that owns an entity
pub fn connection_for_entity(entity_id: EntityId) -> ConnectionId {
    if entity_id >= MIN_SERVER_ENTITY_ID {
        SERVER_CONNECTION_ID
    } else if entity_id < 0 {
        let index = (-entity_id) / (MAX_SERVER_ENTITY_ID / MAX_CLIENT_CONNECTION_ID as i32);
        index as ConnectionId
    } else {
        SERVER_CONNECTION_ID
    }
}

/// Get angle and direction side from an angle
/// 
/// Returns an angle in the range [-pi/2, pi/2] and the horizontal hemisphere.
/// If `cc_rotation` is true, angle is always positive (counter-clockwise).
pub fn get_angle_side(angle: f32, cc_rotation: bool) -> (f32, Direction) {
    use std::f32::consts::PI;
    
    let normalized = angle.rem_euclid(2.0 * PI);
    
    let (side_angle, direction) = if normalized <= PI / 2.0 {
        (normalized, Direction::Right)
    } else if normalized <= PI {
        (PI - normalized, Direction::Left)
    } else if normalized <= 3.0 * PI / 2.0 {
        (normalized - PI, Direction::Left)
    } else {
        (2.0 * PI - normalized, Direction::Right)
    };
    
    if cc_rotation {
        (side_angle, direction)
    } else {
        let signed_angle = if direction == Direction::Left && side_angle > 0.0 {
            -side_angle
        } else {
            side_angle
        };
        (signed_angle, direction)
    }
}

/// Get the center position of a tile
pub fn center_of_tile(tile_x: i32, tile_y: i32) -> (f32, f32) {
    (tile_x as f32 + 0.5, tile_y as f32 + 0.5)
}

// Global timescale and timestep (normally would be mutable statics)
// For Rust, we use thread-local or atomic values

use std::sync::atomic::{AtomicU32, Ordering};

static GLOBAL_TIMESCALE_BITS: AtomicU32 = AtomicU32::new(0x3f800000); // 1.0f32 as bits
static GLOBAL_TIMESTEP_BITS: AtomicU32 = AtomicU32::new(0x3c888889); // 1/60 as bits
static SERVER_GLOBAL_TIMESTEP_BITS: AtomicU32 = AtomicU32::new(0x3c888889); // 1/60 as bits

/// Get the global timescale
pub fn global_timescale() -> f32 {
    f32::from_bits(GLOBAL_TIMESCALE_BITS.load(Ordering::Relaxed))
}

/// Set the global timescale
pub fn set_global_timescale(value: f32) {
    GLOBAL_TIMESCALE_BITS.store(value.to_bits(), Ordering::Relaxed);
}

/// Get the global timestep
pub fn global_timestep() -> f32 {
    f32::from_bits(GLOBAL_TIMESTEP_BITS.load(Ordering::Relaxed))
}

/// Set the global timestep
pub fn set_global_timestep(value: f32) {
    GLOBAL_TIMESTEP_BITS.store(value.to_bits(), Ordering::Relaxed);
}

/// Get the server global timestep
pub fn server_global_timestep() -> f32 {
    f32::from_bits(SERVER_GLOBAL_TIMESTEP_BITS.load(Ordering::Relaxed))
}

/// Set the server global timestep
pub fn set_server_global_timestep(value: f32) {
    SERVER_GLOBAL_TIMESTEP_BITS.store(value.to_bits(), Ordering::Relaxed);
}

/// System world timestep constant (1/20)
pub const SYSTEM_WORLD_TIMESTEP: f32 = 1.0 / 20.0;

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_direction() {
        assert_eq!(Direction::Left.numerical(), -1);
        assert_eq!(Direction::Right.numerical(), 1);
        assert_eq!(-Direction::Left, Direction::Right);
        assert_eq!(-Direction::Right, Direction::Left);
    }
    
    #[test]
    fn test_direction_of() {
        assert_eq!(direction_of(0), None);
        assert_eq!(direction_of(-5), Some(Direction::Left));
        assert_eq!(direction_of(5), Some(Direction::Right));
    }
    
    #[test]
    fn test_numerical_direction() {
        assert_eq!(numerical_direction(None), 0);
        assert_eq!(numerical_direction(Some(Direction::Left)), -1);
        assert_eq!(numerical_direction(Some(Direction::Right)), 1);
    }
    
    #[test]
    fn test_gender() {
        assert_eq!(Gender::from_str("male"), Some(Gender::Male));
        assert_eq!(Gender::from_str("FEMALE"), Some(Gender::Female));
        assert_eq!(Gender::from_str("other"), None);
    }
    
    #[test]
    fn test_rarity_ordering() {
        assert!(Rarity::Common < Rarity::Uncommon);
        assert!(Rarity::Uncommon < Rarity::Rare);
        assert!(Rarity::Rare < Rarity::Legendary);
        assert!(Rarity::Legendary < Rarity::Essential);
    }
    
    #[test]
    fn test_tile_layer() {
        assert_eq!(TileLayer::from_str("foreground"), Some(TileLayer::Foreground));
        assert_eq!(TileLayer::from_str("Background"), Some(TileLayer::Background));
    }
    
    #[test]
    fn test_fire_mode() {
        assert_eq!(FireMode::from_str("none"), Some(FireMode::None));
        assert_eq!(FireMode::from_str("primary"), Some(FireMode::Primary));
        assert_eq!(FireMode::from_str("alt"), Some(FireMode::Alt));
    }
    
    #[test]
    fn test_is_real_dungeon() {
        assert!(is_real_dungeon(0));
        assert!(is_real_dungeon(1000));
        assert!(!is_real_dungeon(NO_DUNGEON_ID));
        assert!(!is_real_dungeon(SPAWN_DUNGEON_ID));
        assert!(!is_real_dungeon(FIRST_META_DUNGEON_ID));
    }
    
    #[test]
    fn test_connection_entity_space() {
        let (start, end) = connection_entity_space(SERVER_CONNECTION_ID);
        assert_eq!(start, MIN_SERVER_ENTITY_ID);
        assert_eq!(end, MAX_SERVER_ENTITY_ID);
    }
    
    #[test]
    fn test_entity_id_in_space() {
        assert!(entity_id_in_space(100, SERVER_CONNECTION_ID));
        assert!(!entity_id_in_space(-100, SERVER_CONNECTION_ID));
    }
    
    #[test]
    fn test_connection_for_entity() {
        assert_eq!(connection_for_entity(100), SERVER_CONNECTION_ID);
    }
    
    #[test]
    fn test_center_of_tile() {
        assert_eq!(center_of_tile(0, 0), (0.5, 0.5));
        assert_eq!(center_of_tile(5, 10), (5.5, 10.5));
    }
    
    #[test]
    fn test_global_timescale() {
        let original = global_timescale();
        set_global_timescale(2.0);
        assert_eq!(global_timescale(), 2.0);
        set_global_timescale(original);
    }
    
    #[test]
    fn test_portrait_mode() {
        assert_eq!(PortraitMode::from_str("head"), Some(PortraitMode::Head));
        assert_eq!(PortraitMode::from_str("bust"), Some(PortraitMode::Bust));
        assert_eq!(PortraitMode::from_str("full"), Some(PortraitMode::Full));
    }
    
    #[test]
    fn test_move_control_type() {
        assert_eq!(MoveControlType::from_str("left"), Some(MoveControlType::Left));
        assert_eq!(MoveControlType::from_str("jump"), Some(MoveControlType::Jump));
    }
}
