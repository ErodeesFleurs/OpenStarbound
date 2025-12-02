//! Entity types compatible with C++ Star::Entity
//!
//! This module provides entity types and traits for game entities.

use crate::error::Result;
use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::types::damage_types::{EntityDamageTeam, TeamType};
use crate::types::game_types::{ConnectionId, EntityId, EntityMode, NULL_ENTITY_ID};

/// How the client should treat an entity created on the client.
///
/// Matches C++ `ClientEntityMode` enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum ClientEntityMode {
    /// Always a slave on the client
    #[default]
    ClientSlaveOnly = 0,
    /// Can be a master on the client
    ClientMasterAllowed = 1,
    /// Can be a master on the client, and contributes to client presence
    ClientPresenceMaster = 2,
}

impl ClientEntityMode {
    /// Creates from a string name.
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "clientslaveonly" | "slave" => Some(ClientEntityMode::ClientSlaveOnly),
            "clientmasterallowed" | "masterallowed" => Some(ClientEntityMode::ClientMasterAllowed),
            "clientpresencemaster" | "presencemaster" => Some(ClientEntityMode::ClientPresenceMaster),
            _ => None,
        }
    }

    /// Gets the name of this mode.
    pub fn name(&self) -> &'static str {
        match self {
            ClientEntityMode::ClientSlaveOnly => "ClientSlaveOnly",
            ClientEntityMode::ClientMasterAllowed => "ClientMasterAllowed",
            ClientEntityMode::ClientPresenceMaster => "ClientPresenceMaster",
        }
    }
}

impl Readable for ClientEntityMode {
    fn read(reader: &mut DataReader) -> Result<Self> {
        match reader.read_u8()? {
            0 => Ok(ClientEntityMode::ClientSlaveOnly),
            1 => Ok(ClientEntityMode::ClientMasterAllowed),
            2 => Ok(ClientEntityMode::ClientPresenceMaster),
            _ => Ok(ClientEntityMode::ClientSlaveOnly),
        }
    }
}

impl Writable for ClientEntityMode {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u8(*self as u8)
    }
}

/// The top-level entity type.
///
/// The enum order is intended to be in the order in which entities should be
/// updated every tick. Matches C++ `EntityType` enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum EntityType {
    Plant = 0,
    Object = 1,
    Vehicle = 2,
    ItemDrop = 3,
    PlantDrop = 4,
    Projectile = 5,
    Stagehand = 6,
    Monster = 7,
    Npc = 8,
    Player = 9,
}

impl Default for EntityType {
    fn default() -> Self {
        EntityType::Object
    }
}

impl EntityType {
    /// Creates from a string name.
    pub fn from_name(name: &str) -> Option<Self> {
        match name.to_lowercase().as_str() {
            "plant" => Some(EntityType::Plant),
            "object" => Some(EntityType::Object),
            "vehicle" => Some(EntityType::Vehicle),
            "itemdrop" | "item" => Some(EntityType::ItemDrop),
            "plantdrop" => Some(EntityType::PlantDrop),
            "projectile" => Some(EntityType::Projectile),
            "stagehand" => Some(EntityType::Stagehand),
            "monster" => Some(EntityType::Monster),
            "npc" => Some(EntityType::Npc),
            "player" => Some(EntityType::Player),
            _ => None,
        }
    }

    /// Gets the name of this entity type.
    pub fn name(&self) -> &'static str {
        match self {
            EntityType::Plant => "Plant",
            EntityType::Object => "Object",
            EntityType::Vehicle => "Vehicle",
            EntityType::ItemDrop => "ItemDrop",
            EntityType::PlantDrop => "PlantDrop",
            EntityType::Projectile => "Projectile",
            EntityType::Stagehand => "Stagehand",
            EntityType::Monster => "Monster",
            EntityType::Npc => "Npc",
            EntityType::Player => "Player",
        }
    }

    /// Returns true if this entity type can be a master on the client.
    pub fn can_be_client_master(&self) -> bool {
        matches!(self, EntityType::Player | EntityType::Projectile)
    }

    /// Returns true if this entity type is persistent (saved to disk).
    pub fn is_persistent(&self) -> bool {
        matches!(
            self,
            EntityType::Plant | EntityType::Object | EntityType::Monster | EntityType::Npc | EntityType::Player
        )
    }

    /// Returns true if this entity is an actor (can move and interact).
    pub fn is_actor(&self) -> bool {
        matches!(
            self,
            EntityType::Monster | EntityType::Npc | EntityType::Player | EntityType::Vehicle
        )
    }
}

impl Readable for EntityType {
    fn read(reader: &mut DataReader) -> Result<Self> {
        match reader.read_u8()? {
            0 => Ok(EntityType::Plant),
            1 => Ok(EntityType::Object),
            2 => Ok(EntityType::Vehicle),
            3 => Ok(EntityType::ItemDrop),
            4 => Ok(EntityType::PlantDrop),
            5 => Ok(EntityType::Projectile),
            6 => Ok(EntityType::Stagehand),
            7 => Ok(EntityType::Monster),
            8 => Ok(EntityType::Npc),
            9 => Ok(EntityType::Player),
            _ => Ok(EntityType::Object),
        }
    }
}

impl Writable for EntityType {
    fn write(&self, writer: &mut DataWriter) -> Result<()> {
        writer.write_u8(*self as u8)
    }
}

/// Common entity state data.
///
/// This is not a trait like in C++ but a data structure that can be embedded in entities.
#[derive(Debug, Clone)]
pub struct EntityState {
    /// The entity ID (unique within a world)
    entity_id: EntityId,
    /// Current entity mode (master/slave)
    entity_mode: Option<EntityMode>,
    /// Whether the entity is persistent (saved to disk)
    persistent: bool,
    /// Whether this entity keeps the sector alive
    keep_alive: bool,
    /// Unique identifier for cross-world references
    unique_id: Option<String>,
    /// Damage team
    team: EntityDamageTeam,
}

impl Default for EntityState {
    fn default() -> Self {
        Self {
            entity_id: NULL_ENTITY_ID,
            entity_mode: None,
            persistent: false,
            keep_alive: false,
            unique_id: None,
            team: EntityDamageTeam::new(TeamType::Null, 0),
        }
    }
}

impl EntityState {
    /// Creates a new entity state.
    pub fn new() -> Self {
        Self::default()
    }

    /// Gets the entity ID.
    pub fn entity_id(&self) -> EntityId {
        self.entity_id
    }

    /// Returns true if the entity is in a world.
    pub fn in_world(&self) -> bool {
        self.entity_mode.is_some()
    }

    /// Gets the entity mode.
    pub fn entity_mode(&self) -> Option<EntityMode> {
        self.entity_mode
    }

    /// Returns true if this entity is a master.
    pub fn is_master(&self) -> bool {
        self.entity_mode == Some(EntityMode::Master)
    }

    /// Returns true if this entity is a slave.
    pub fn is_slave(&self) -> bool {
        self.entity_mode == Some(EntityMode::Slave)
    }

    /// Gets whether the entity is persistent.
    pub fn persistent(&self) -> bool {
        self.persistent
    }

    /// Sets whether the entity is persistent.
    pub fn set_persistent(&mut self, persistent: bool) {
        self.persistent = persistent;
    }

    /// Gets whether the entity keeps the sector alive.
    pub fn keep_alive(&self) -> bool {
        self.keep_alive
    }

    /// Sets whether the entity keeps the sector alive.
    pub fn set_keep_alive(&mut self, keep_alive: bool) {
        self.keep_alive = keep_alive;
    }

    /// Gets the unique ID.
    pub fn unique_id(&self) -> Option<&str> {
        self.unique_id.as_deref()
    }

    /// Sets the unique ID.
    pub fn set_unique_id(&mut self, unique_id: Option<String>) {
        self.unique_id = unique_id;
    }

    /// Gets the damage team.
    pub fn team(&self) -> &EntityDamageTeam {
        &self.team
    }

    /// Sets the damage team.
    pub fn set_team(&mut self, team: EntityDamageTeam) {
        self.team = team;
    }

    /// Initializes the entity in a world.
    pub fn init(&mut self, entity_id: EntityId, mode: EntityMode) {
        self.entity_id = entity_id;
        self.entity_mode = Some(mode);
    }

    /// Uninitializes the entity from a world.
    pub fn uninit(&mut self) {
        self.entity_id = NULL_ENTITY_ID;
        self.entity_mode = None;
    }
}

/// Trait for entities that can be rendered.
pub trait Renderable {
    /// Renders the entity.
    fn render(&self);

    /// Renders light sources.
    fn render_light_sources(&self);
}

/// Trait for entities that can receive messages.
pub trait MessageReceiver {
    /// Receives a message and optionally returns a response.
    fn receive_message(
        &mut self,
        sending_connection: ConnectionId,
        message: &str,
        args: &[serde_json::Value],
    ) -> Option<serde_json::Value>;
}

/// Entity factory function type.
pub type EntityFactoryFn = fn() -> Box<dyn std::any::Any>;

/// Entity factory registry.
#[derive(Default)]
pub struct EntityFactory {
    factories: std::collections::HashMap<EntityType, EntityFactoryFn>,
}

impl EntityFactory {
    /// Creates a new entity factory.
    pub fn new() -> Self {
        Self::default()
    }

    /// Registers an entity type factory.
    pub fn register(&mut self, entity_type: EntityType, factory: EntityFactoryFn) {
        self.factories.insert(entity_type, factory);
    }

    /// Creates an entity of the given type.
    pub fn create(&self, entity_type: EntityType) -> Option<Box<dyn std::any::Any>> {
        self.factories.get(&entity_type).map(|f| f())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_client_entity_mode() {
        assert_eq!(
            ClientEntityMode::from_name("ClientSlaveOnly"),
            Some(ClientEntityMode::ClientSlaveOnly)
        );
        assert_eq!(
            ClientEntityMode::from_name("presencemaster"),
            Some(ClientEntityMode::ClientPresenceMaster)
        );
        assert_eq!(ClientEntityMode::from_name("invalid"), None);

        assert_eq!(
            ClientEntityMode::ClientMasterAllowed.name(),
            "ClientMasterAllowed"
        );
    }

    #[test]
    fn test_entity_type() {
        assert_eq!(EntityType::from_name("player"), Some(EntityType::Player));
        assert_eq!(EntityType::from_name("Monster"), Some(EntityType::Monster));
        assert_eq!(EntityType::from_name("invalid"), None);

        assert_eq!(EntityType::Player.name(), "Player");
        assert!(EntityType::Player.can_be_client_master());
        assert!(!EntityType::Object.can_be_client_master());
        assert!(EntityType::Player.is_actor());
        assert!(!EntityType::Plant.is_actor());
    }

    #[test]
    fn test_entity_state() {
        let mut state = EntityState::new();
        assert_eq!(state.entity_id(), NULL_ENTITY_ID);
        assert!(!state.in_world());

        state.init(123, EntityMode::Master);
        assert_eq!(state.entity_id(), 123);
        assert!(state.in_world());
        assert!(state.is_master());
        assert!(!state.is_slave());

        state.set_persistent(true);
        assert!(state.persistent());

        state.set_unique_id(Some("test-entity".to_string()));
        assert_eq!(state.unique_id(), Some("test-entity"));

        state.uninit();
        assert!(!state.in_world());
    }

    #[test]
    fn test_entity_type_serialization() {
        let et = EntityType::Monster;
        let mut writer = DataWriter::new();
        et.write(&mut writer).unwrap();

        let mut reader = DataReader::new(writer.data());
        let et2 = EntityType::read(&mut reader).unwrap();
        assert_eq!(et, et2);
    }

    #[test]
    fn test_client_entity_mode_serialization() {
        let mode = ClientEntityMode::ClientPresenceMaster;
        let mut writer = DataWriter::new();
        mode.write(&mut writer).unwrap();

        let mut reader = DataReader::new(writer.data());
        let mode2 = ClientEntityMode::read(&mut reader).unwrap();
        assert_eq!(mode, mode2);
    }

    #[test]
    fn test_entity_factory() {
        let factory = EntityFactory::new();
        assert!(factory.create(EntityType::Player).is_none());
    }
}
