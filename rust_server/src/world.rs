/// World Management Module
/// 
/// This module implements basic world management, entity tracking, and world simulation.

use crate::protocol::*;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;

/// World structure representing a game world
#[derive(Debug)]
pub struct World {
    /// World identifier (coordinate string, e.g., "CelestialWorld:1:2:3")
    pub id: String,
    
    /// World template data (JSON)
    pub template_data: String,
    
    /// Sky configuration data
    pub sky_data: Vec<u8>,
    
    /// Weather configuration data
    pub weather_data: Vec<u8>,
    
    /// Player spawn position
    pub spawn_position: (f32, f32),
    
    /// World properties (JSON)
    pub properties: String,
    
    /// Entity manager for this world
    pub entities: EntityManager,
    
    /// World simulation tick counter
    pub tick: u64,
}

impl World {
    /// Create a new world with default parameters
    pub fn new(id: String) -> Self {
        Self {
            id,
            template_data: r#"{"type":"terrestrial"}"#.to_string(),
            sky_data: Vec::new(),
            weather_data: Vec::new(),
            spawn_position: (0.0, 100.0),
            properties: r#"{"gravity":9.8,"breathable":true}"#.to_string(),
            entities: EntityManager::new(),
            tick: 0,
        }
    }
    
    /// Create a WorldStartPacket for a client
    pub fn create_start_packet(&self, client_id: ConnectionId) -> WorldStartPacket {
        WorldStartPacket {
            template_data: self.template_data.clone(),
            sky_data: self.sky_data.clone(),
            weather_data: self.weather_data.clone(),
            player_start: self.spawn_position,
            player_respawn: self.spawn_position,
            respawn_in_world: true,
            world_properties: self.properties.clone(),
            client_id,
            local_interpolation_mode: true,
        }
    }
    
    /// Simulate one world tick
    pub fn tick(&mut self) -> Vec<EntityUpdateSetPacket> {
        self.tick += 1;
        
        // Generate entity updates (simplified for MVP)
        let updates = self.entities.generate_updates();
        
        updates
    }
    
    /// Add an entity to the world
    pub fn add_entity(&mut self, entity: Entity) -> EntityCreatePacket {
        self.entities.add_entity(entity)
    }
    
    /// Remove an entity from the world
    pub fn remove_entity(&mut self, entity_id: EntityId, death: bool) -> Option<EntityDestroyPacket> {
        self.entities.remove_entity(entity_id, death)
    }
}

/// Entity structure representing a game entity
#[derive(Debug, Clone)]
pub struct Entity {
    pub id: EntityId,
    pub entity_type: EntityType,
    pub position: (f32, f32),
    pub store_data: Vec<u8>,
    pub net_state: Vec<u8>,
}

impl Entity {
    /// Create a new entity
    pub fn new(id: EntityId, entity_type: EntityType) -> Self {
        Self {
            id,
            entity_type,
            position: (0.0, 0.0),
            store_data: Vec::new(),
            net_state: Vec::new(),
        }
    }
    
    /// Create a player entity
    pub fn new_player(id: EntityId, position: (f32, f32)) -> Self {
        Self {
            id,
            entity_type: EntityType::Player,
            position,
            store_data: Vec::new(),
            net_state: Vec::new(),
        }
    }
}

/// Entity Manager for tracking entities in a world
#[derive(Debug)]
pub struct EntityManager {
    entities: HashMap<EntityId, Entity>,
    next_entity_id: EntityId,
}

impl EntityManager {
    /// Create a new entity manager
    pub fn new() -> Self {
        Self {
            entities: HashMap::new(),
            next_entity_id: 1,
        }
    }
    
    /// Allocate a new entity ID
    pub fn allocate_id(&mut self) -> EntityId {
        let id = self.next_entity_id;
        self.next_entity_id += 1;
        id
    }
    
    /// Add an entity and return the create packet
    pub fn add_entity(&mut self, entity: Entity) -> EntityCreatePacket {
        let packet = EntityCreatePacket {
            entity_type: entity.entity_type,
            store_data: entity.store_data.clone(),
            first_net_state: entity.net_state.clone(),
            entity_id: entity.id,
        };
        
        self.entities.insert(entity.id, entity);
        packet
    }
    
    /// Remove an entity and return the destroy packet
    pub fn remove_entity(&mut self, entity_id: EntityId, death: bool) -> Option<EntityDestroyPacket> {
        self.entities.remove(&entity_id).map(|entity| {
            EntityDestroyPacket {
                entity_id,
                final_net_state: entity.net_state,
                death,
            }
        })
    }
    
    /// Get an entity by ID
    pub fn get_entity(&self, entity_id: EntityId) -> Option<&Entity> {
        self.entities.get(&entity_id)
    }
    
    /// Get a mutable entity by ID
    pub fn get_entity_mut(&mut self, entity_id: EntityId) -> Option<&mut Entity> {
        self.entities.get_mut(&entity_id)
    }
    
    /// Generate entity updates (simplified)
    pub fn generate_updates(&self) -> Vec<EntityUpdateSetPacket> {
        // For MVP, return empty updates
        // Full implementation would track entity state changes
        Vec::new()
    }
    
    /// Get all entity IDs
    pub fn entity_ids(&self) -> Vec<EntityId> {
        self.entities.keys().copied().collect()
    }
    
    /// Get entity count
    pub fn entity_count(&self) -> usize {
        self.entities.len()
    }
}

/// World Manager for managing multiple worlds
pub struct WorldManager {
    worlds: Arc<RwLock<HashMap<String, Arc<RwLock<World>>>>>,
}

impl WorldManager {
    /// Create a new world manager
    pub fn new() -> Self {
        Self {
            worlds: Arc::new(RwLock::new(HashMap::new())),
        }
    }
    
    /// Get or create a world
    pub async fn get_or_create_world(&self, world_id: String) -> Arc<RwLock<World>> {
        let mut worlds = self.worlds.write().await;
        
        if let Some(world) = worlds.get(&world_id) {
            world.clone()
        } else {
            let world = Arc::new(RwLock::new(World::new(world_id.clone())));
            worlds.insert(world_id, world.clone());
            world
        }
    }
    
    /// Get a world by ID
    pub async fn get_world(&self, world_id: &str) -> Option<Arc<RwLock<World>>> {
        let worlds = self.worlds.read().await;
        worlds.get(world_id).cloned()
    }
    
    /// Remove a world
    pub async fn remove_world(&self, world_id: &str) -> Option<Arc<RwLock<World>>> {
        let mut worlds = self.worlds.write().await;
        worlds.remove(world_id)
    }
    
    /// Get all world IDs
    pub async fn world_ids(&self) -> Vec<String> {
        let worlds = self.worlds.read().await;
        worlds.keys().cloned().collect()
    }
    
    /// Get world count
    pub async fn world_count(&self) -> usize {
        let worlds = self.worlds.read().await;
        worlds.len()
    }
}

/// World file metadata structure
#[derive(Debug, Clone)]
pub struct WorldMetadata {
    pub name: String,
    pub size: (u32, u32),  // Width, height in tiles
    pub spawn_point: (f32, f32),
    pub gravity: f32,
    pub breathable: bool,
    pub biome: String,
}

impl WorldMetadata {
    /// Create default world metadata
    pub fn default() -> Self {
        Self {
            name: "Default World".to_string(),
            size: (1000, 1000),
            spawn_point: (500.0, 100.0),
            gravity: 9.8,
            breathable: true,
            biome: "forest".to_string(),
        }
    }
    
    /// Create world metadata from JSON string
    pub fn from_json(json: &str) -> Result<Self, String> {
        // Simplified JSON parsing for MVP
        // Full implementation would use serde_json
        Ok(Self::default())
    }
}

/// Entity behavior trait - defines how entities update
pub trait EntityBehavior: Send + Sync {
    /// Update entity state for one tick
    fn update(&mut self, entity: &mut Entity, world_tick: u64);
    
    /// Check if entity should be removed
    fn should_remove(&self) -> bool {
        false
    }
}

/// Player entity state
#[derive(Debug, Clone)]
pub struct PlayerState {
    pub entity_id: EntityId,
    pub name: String,
    pub health: f32,
    pub max_health: f32,
    pub energy: f32,
    pub max_energy: f32,
    pub position: (f32, f32),
    pub velocity: (f32, f32),
    pub facing_direction: i8,  // -1 = left, 1 = right
}

impl PlayerState {
    /// Create a new player state
    pub fn new(entity_id: EntityId, name: String) -> Self {
        Self {
            entity_id,
            name,
            health: 100.0,
            max_health: 100.0,
            energy: 100.0,
            max_energy: 100.0,
            position: (0.0, 0.0),
            velocity: (0.0, 0.0),
            facing_direction: 1,
        }
    }
    
    /// Apply damage to player
    pub fn take_damage(&mut self, damage: f32) -> bool {
        self.health -= damage;
        if self.health < 0.0 {
            self.health = 0.0;
        }
        self.health <= 0.0  // Returns true if dead
    }
    
    /// Heal player
    pub fn heal(&mut self, amount: f32) {
        self.health += amount;
        if self.health > self.max_health {
            self.health = self.max_health;
        }
    }
    
    /// Update position
    pub fn update_position(&mut self, new_pos: (f32, f32)) {
        self.position = new_pos;
    }
    
    /// Update velocity
    pub fn update_velocity(&mut self, new_vel: (f32, f32)) {
        self.velocity = new_vel;
    }
}

/// Simple static entity behavior (doesn't move)
pub struct StaticBehavior;

impl EntityBehavior for StaticBehavior {
    fn update(&mut self, _entity: &mut Entity, _world_tick: u64) {
        // Static entities don't update
    }
}

/// Simple projectile behavior (moves in a direction)
pub struct ProjectileBehavior {
    velocity: (f32, f32),
    lifetime: u64,
    age: u64,
}

impl ProjectileBehavior {
    pub fn new(velocity: (f32, f32), lifetime: u64) -> Self {
        Self {
            velocity,
            lifetime,
            age: 0,
        }
    }
}

impl EntityBehavior for ProjectileBehavior {
    fn update(&mut self, entity: &mut Entity, _world_tick: u64) {
        // Update position based on velocity
        entity.position.0 += self.velocity.0;
        entity.position.1 += self.velocity.1;
        
        self.age += 1;
    }
    
    fn should_remove(&self) -> bool {
        self.age >= self.lifetime
    }
}

/// Player behavior (managed by client input)
pub struct PlayerBehavior {
    last_update: u64,
}

impl PlayerBehavior {
    pub fn new() -> Self {
        Self { last_update: 0 }
    }
}

impl EntityBehavior for PlayerBehavior {
    fn update(&mut self, _entity: &mut Entity, world_tick: u64) {
        // Player movement is controlled by client
        // Server just validates and updates state
        self.last_update = world_tick;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_world_creation() {
        let world = World::new("test_world".to_string());
        assert_eq!(world.id, "test_world");
        assert_eq!(world.tick, 0);
        assert_eq!(world.entities.entity_count(), 0);
    }

    #[test]
    fn test_entity_manager() {
        let mut manager = EntityManager::new();
        
        let id = manager.allocate_id();
        assert_eq!(id, 1);
        
        let entity = Entity::new(id, EntityType::Player);
        manager.add_entity(entity);
        
        assert_eq!(manager.entity_count(), 1);
        assert!(manager.get_entity(id).is_some());
    }

    #[test]
    fn test_entity_removal() {
        let mut manager = EntityManager::new();
        let id = manager.allocate_id();
        let entity = Entity::new(id, EntityType::Monster);
        
        manager.add_entity(entity);
        assert_eq!(manager.entity_count(), 1);
        
        let destroy_packet = manager.remove_entity(id, true);
        assert!(destroy_packet.is_some());
        assert_eq!(manager.entity_count(), 0);
    }

    #[tokio::test]
    async fn test_world_manager() {
        let manager = WorldManager::new();
        
        let world1 = manager.get_or_create_world("world1".to_string()).await;
        let world2 = manager.get_or_create_world("world2".to_string()).await;
        
        assert_eq!(manager.world_count().await, 2);
        
        {
            let w1 = world1.read().await;
            assert_eq!(w1.id, "world1");
        }
        
        manager.remove_world("world1").await;
        assert_eq!(manager.world_count().await, 1);
    }

    #[test]
    fn test_world_metadata() {
        let metadata = WorldMetadata::default();
        assert_eq!(metadata.name, "Default World");
        assert_eq!(metadata.size, (1000, 1000));
        assert_eq!(metadata.gravity, 9.8);
        assert_eq!(metadata.breathable, true);
    }

    #[test]
    fn test_player_state() {
        let mut player = PlayerState::new(1, "TestPlayer".to_string());
        
        assert_eq!(player.health, 100.0);
        assert_eq!(player.name, "TestPlayer");
        
        // Test damage
        let dead = player.take_damage(50.0);
        assert_eq!(player.health, 50.0);
        assert!(!dead);
        
        // Test healing
        player.heal(25.0);
        assert_eq!(player.health, 75.0);
        
        // Test death
        let dead = player.take_damage(100.0);
        assert_eq!(player.health, 0.0);
        assert!(dead);
    }

    #[test]
    fn test_projectile_behavior() {
        let mut behavior = ProjectileBehavior::new((1.0, 0.5), 10);
        let mut entity = Entity::new(1, EntityType::Projectile);
        entity.position = (0.0, 0.0);
        
        // Update once
        behavior.update(&mut entity, 1);
        assert_eq!(entity.position, (1.0, 0.5));
        assert!(!behavior.should_remove());
        
        // Simulate 10 updates
        for i in 2..=10 {
            behavior.update(&mut entity, i);
        }
        
        // Should be removed after lifetime
        assert!(behavior.should_remove());
    }

    #[test]
    fn test_player_behavior() {
        let mut behavior = PlayerBehavior::new();
        let mut entity = Entity::new_player(1, (0.0, 0.0));
        
        behavior.update(&mut entity, 1);
        assert_eq!(behavior.last_update, 1);
        
        behavior.update(&mut entity, 5);
        assert_eq!(behavior.last_update, 5);
    }
}
