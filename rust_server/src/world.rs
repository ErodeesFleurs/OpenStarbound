/// World Management Module
/// 
/// This module implements basic world management, entity tracking, and world simulation.

use crate::protocol::*;
use std::collections::{HashMap, BinaryHeap, HashSet};
use std::sync::Arc;
use std::cmp::Ordering;
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
#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct WorldMetadata {
    pub name: String,
    pub size: (u32, u32),  // Width, height in tiles
    pub spawn_point: (f32, f32),
    pub gravity: f32,
    pub breathable: bool,
    pub biome: String,
}

impl Default for WorldMetadata {
    fn default() -> Self {
        Self {
            name: "Default World".to_string(),
            size: (1000, 1000),
            spawn_point: (500.0, 100.0),
            gravity: 9.8,
            breathable: true,
            biome: "forest".to_string(),
        }
    }
}

impl WorldMetadata {
    /// Create world metadata from JSON string
    pub fn from_json(_json: &str) -> Result<Self, String> {
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

/// World file I/O operations
impl World {
    /// Load a world from a file path (simplified implementation)
    /// In a real implementation, this would parse Starbound's world file format
    pub async fn load_from_file(path: &std::path::Path) -> Result<Self, std::io::Error> {
        use tokio::fs;
        
        // For MVP, we'll create a basic structure
        // Real implementation would parse binary world format
        let filename = path.file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("unknown");
        
        // Try to read metadata file if it exists
        let metadata_path = path.with_extension("json");
        let metadata = if metadata_path.exists() {
            let content = fs::read_to_string(&metadata_path).await?;
            serde_json::from_str::<WorldMetadata>(&content).unwrap_or_default()
        } else {
            WorldMetadata::default()
        };
        
        Ok(Self {
            id: filename.to_string(),
            template_data: format!(r#"{{"name":"{}","biome":"{}"}}"#, metadata.name, metadata.biome),
            sky_data: Vec::new(),
            weather_data: Vec::new(),
            spawn_position: metadata.spawn_point,
            properties: format!(
                r#"{{"gravity":{},"breathable":{}}}"#,
                metadata.gravity, metadata.breathable
            ),
            entities: EntityManager::new(),
            tick: 0,
        })
    }
    
    /// Save world to a file (simplified implementation)
    pub async fn save_to_file(&self, path: &std::path::Path) -> Result<(), std::io::Error> {
        use tokio::fs;
        
        // Save metadata as JSON
        let metadata = WorldMetadata {
            name: self.id.clone(),
            size: (1000, 1000),  // Would be determined from actual world data
            spawn_point: self.spawn_position,
            gravity: 9.8,
            breathable: true,
            biome: "forest".to_string(),
        };
        
        let metadata_path = path.with_extension("json");
        let json = serde_json::to_string_pretty(&metadata).unwrap();
        fs::write(&metadata_path, json).await?;
        
        Ok(())
    }
}

/// Collision detection for entities
#[derive(Debug, Clone)]
pub struct CollisionBox {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

impl CollisionBox {
    pub fn new(x: f32, y: f32, width: f32, height: f32) -> Self {
        Self { x, y, width, height }
    }
    
    /// Check if this box intersects with another
    pub fn intersects(&self, other: &CollisionBox) -> bool {
        self.x < other.x + other.width
            && self.x + self.width > other.x
            && self.y < other.y + other.height
            && self.y + self.height > other.y
    }
    
    /// Check if a point is inside this box
    pub fn contains_point(&self, x: f32, y: f32) -> bool {
        x >= self.x && x <= self.x + self.width
            && y >= self.y && y <= self.y + self.height
    }
}

/// Collision system for entities
pub struct CollisionSystem {
    entity_boxes: HashMap<EntityId, CollisionBox>,
}

impl CollisionSystem {
    pub fn new() -> Self {
        Self {
            entity_boxes: HashMap::new(),
        }
    }
    
    /// Register an entity's collision box
    pub fn register_entity(&mut self, entity_id: EntityId, collision_box: CollisionBox) {
        self.entity_boxes.insert(entity_id, collision_box);
    }
    
    /// Unregister an entity
    pub fn unregister_entity(&mut self, entity_id: EntityId) {
        self.entity_boxes.remove(&entity_id);
    }
    
    /// Find all entities that collide with a given box
    pub fn find_collisions(&self, test_box: &CollisionBox) -> Vec<EntityId> {
        self.entity_boxes
            .iter()
            .filter(|(_, box_)| test_box.intersects(box_))
            .map(|(id, _)| *id)
            .collect()
    }
    
    /// Check if an entity at a position would collide with anything
    pub fn check_collision(&self, entity_id: EntityId, position: (f32, f32), size: (f32, f32)) -> bool {
        let test_box = CollisionBox::new(position.0, position.1, size.0, size.1);
        
        self.entity_boxes
            .iter()
            .any(|(id, box_)| *id != entity_id && test_box.intersects(box_))
    }
}

// ==================== Phase 8: AI and Pathfinding ====================

/// A* pathfinding node for priority queue
#[derive(Clone, Eq, PartialEq)]
struct PathNode {
    position: (i32, i32),
    g_cost: i32,  // Cost from start
    h_cost: i32,  // Heuristic cost to goal
    parent: Option<(i32, i32)>,
}

impl PathNode {
    fn f_cost(&self) -> i32 {
        self.g_cost + self.h_cost
    }
}

impl Ord for PathNode {
    fn cmp(&self, other: &Self) -> Ordering {
        // Reverse ordering for min-heap
        other.f_cost().cmp(&self.f_cost())
    }
}

impl PartialOrd for PathNode {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

/// A* pathfinding implementation
pub struct Pathfinder {
    /// Map of walkable tiles
    walkable_map: HashSet<(i32, i32)>,
}

impl Pathfinder {
    pub fn new() -> Self {
        Self {
            walkable_map: HashSet::new(),
        }
    }
    
    /// Set a tile as walkable
    pub fn set_walkable(&mut self, x: i32, y: i32) {
        self.walkable_map.insert((x, y));
    }
    
    /// Set a tile as blocked
    pub fn set_blocked(&mut self, x: i32, y: i32) {
        self.walkable_map.remove(&(x, y));
    }
    
    /// Check if a position is walkable
    pub fn is_walkable(&self, x: i32, y: i32) -> bool {
        self.walkable_map.contains(&(x, y))
    }
    
    /// Manhattan distance heuristic
    fn heuristic(pos: (i32, i32), goal: (i32, i32)) -> i32 {
        (pos.0 - goal.0).abs() + (pos.1 - goal.1).abs()
    }
    
    /// Get neighboring positions (4-directional)
    fn neighbors(pos: (i32, i32)) -> Vec<(i32, i32)> {
        vec![
            (pos.0 + 1, pos.1),
            (pos.0 - 1, pos.1),
            (pos.0, pos.1 + 1),
            (pos.0, pos.1 - 1),
        ]
    }
    
    /// Find path from start to goal using A*
    pub fn find_path(&self, start: (i32, i32), goal: (i32, i32)) -> Option<Vec<(i32, i32)>> {
        if !self.is_walkable(start.0, start.1) || !self.is_walkable(goal.0, goal.1) {
            return None;
        }
        
        let mut open_set = BinaryHeap::new();
        let mut closed_set = HashSet::new();
        let mut came_from = HashMap::new();
        let mut g_scores = HashMap::new();
        
        g_scores.insert(start, 0);
        open_set.push(PathNode {
            position: start,
            g_cost: 0,
            h_cost: Self::heuristic(start, goal),
            parent: None,
        });
        
        while let Some(current) = open_set.pop() {
            if current.position == goal {
                // Reconstruct path
                let mut path = vec![goal];
                let mut current_pos = goal;
                
                while let Some(&parent) = came_from.get(&current_pos) {
                    path.push(parent);
                    current_pos = parent;
                }
                
                path.reverse();
                return Some(path);
            }
            
            if closed_set.contains(&current.position) {
                continue;
            }
            
            closed_set.insert(current.position);
            
            for neighbor in Self::neighbors(current.position) {
                if !self.is_walkable(neighbor.0, neighbor.1) || closed_set.contains(&neighbor) {
                    continue;
                }
                
                let tentative_g = current.g_cost + 1;
                
                if tentative_g < *g_scores.get(&neighbor).unwrap_or(&i32::MAX) {
                    came_from.insert(neighbor, current.position);
                    g_scores.insert(neighbor, tentative_g);
                    
                    open_set.push(PathNode {
                        position: neighbor,
                        g_cost: tentative_g,
                        h_cost: Self::heuristic(neighbor, goal),
                        parent: Some(current.position),
                    });
                }
            }
        }
        
        None
    }
}

/// Behavior tree node result
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BehaviorStatus {
    Success,
    Failure,
    Running,
}

/// Behavior tree node trait
pub trait BehaviorNode: Send + Sync {
    fn execute(&mut self, entity: &mut Entity, world_tick: u64) -> BehaviorStatus;
}

/// Sequence node - runs children until one fails
pub struct SequenceNode {
    children: Vec<Box<dyn BehaviorNode>>,
    current_child: usize,
}

impl SequenceNode {
    pub fn new(children: Vec<Box<dyn BehaviorNode>>) -> Self {
        Self {
            children,
            current_child: 0,
        }
    }
}

impl BehaviorNode for SequenceNode {
    fn execute(&mut self, entity: &mut Entity, world_tick: u64) -> BehaviorStatus {
        while self.current_child < self.children.len() {
            match self.children[self.current_child].execute(entity, world_tick) {
                BehaviorStatus::Success => {
                    self.current_child += 1;
                }
                BehaviorStatus::Failure => {
                    self.current_child = 0;
                    return BehaviorStatus::Failure;
                }
                BehaviorStatus::Running => {
                    return BehaviorStatus::Running;
                }
            }
        }
        
        self.current_child = 0;
        BehaviorStatus::Success
    }
}

/// Selector node - runs children until one succeeds
pub struct SelectorNode {
    children: Vec<Box<dyn BehaviorNode>>,
    current_child: usize,
}

impl SelectorNode {
    pub fn new(children: Vec<Box<dyn BehaviorNode>>) -> Self {
        Self {
            children,
            current_child: 0,
        }
    }
}

impl BehaviorNode for SelectorNode {
    fn execute(&mut self, entity: &mut Entity, world_tick: u64) -> BehaviorStatus {
        while self.current_child < self.children.len() {
            match self.children[self.current_child].execute(entity, world_tick) {
                BehaviorStatus::Success => {
                    self.current_child = 0;
                    return BehaviorStatus::Success;
                }
                BehaviorStatus::Failure => {
                    self.current_child += 1;
                }
                BehaviorStatus::Running => {
                    return BehaviorStatus::Running;
                }
            }
        }
        
        self.current_child = 0;
        BehaviorStatus::Failure
    }
}

/// Simple action: move towards target
pub struct MoveTowardsNode {
    target: (f32, f32),
    speed: f32,
}

impl MoveTowardsNode {
    pub fn new(target: (f32, f32), speed: f32) -> Self {
        Self { target, speed }
    }
}

impl BehaviorNode for MoveTowardsNode {
    fn execute(&mut self, entity: &mut Entity, _world_tick: u64) -> BehaviorStatus {
        let dx = self.target.0 - entity.position.0;
        let dy = self.target.1 - entity.position.1;
        let distance = (dx * dx + dy * dy).sqrt();
        
        if distance < 1.0 {
            return BehaviorStatus::Success;
        }
        
        let move_x = (dx / distance) * self.speed;
        let move_y = (dy / distance) * self.speed;
        
        entity.position.0 += move_x;
        entity.position.1 += move_y;
        
        BehaviorStatus::Running
    }
}

/// Simple action: wait for a duration
pub struct WaitNode {
    duration: u64,
    start_tick: Option<u64>,
}

impl WaitNode {
    pub fn new(duration: u64) -> Self {
        Self {
            duration,
            start_tick: None,
        }
    }
}

impl BehaviorNode for WaitNode {
    fn execute(&mut self, _entity: &mut Entity, world_tick: u64) -> BehaviorStatus {
        if self.start_tick.is_none() {
            self.start_tick = Some(world_tick);
        }
        
        if world_tick - self.start_tick.unwrap() >= self.duration {
            self.start_tick = None;
            return BehaviorStatus::Success;
        }
        
        BehaviorStatus::Running
    }
}

/// AI Controller for managing entity behavior
pub struct AIController {
    behavior_tree: Box<dyn BehaviorNode>,
}

impl AIController {
    pub fn new(behavior_tree: Box<dyn BehaviorNode>) -> Self {
        Self { behavior_tree }
    }
    
    pub fn update(&mut self, entity: &mut Entity, world_tick: u64) -> BehaviorStatus {
        self.behavior_tree.execute(entity, world_tick)
    }
}

/// Monster AI behavior
pub struct MonsterAI {
    target_position: Option<(f32, f32)>,
    wander_timer: u64,
    last_wander_tick: u64,
}

impl MonsterAI {
    pub fn new() -> Self {
        Self {
            target_position: None,
            wander_timer: 120,  // Wander every 120 ticks
            last_wander_tick: 0,
        }
    }
    
    pub fn set_target(&mut self, target: (f32, f32)) {
        self.target_position = Some(target);
    }
    
    pub fn clear_target(&mut self) {
        self.target_position = None;
    }
    
    pub fn update(&mut self, entity: &mut Entity, world_tick: u64) {
        if let Some(target) = self.target_position {
            // Move towards target
            let dx = target.0 - entity.position.0;
            let dy = target.1 - entity.position.1;
            let distance = (dx * dx + dy * dy).sqrt();
            
            if distance > 1.0 {
                let speed = 2.0;
                entity.position.0 += (dx / distance) * speed;
                entity.position.1 += (dy / distance) * speed;
            }
        } else {
            // Wander behavior
            if world_tick - self.last_wander_tick >= self.wander_timer {
                self.last_wander_tick = world_tick;
                
                // Random wander (simplified)
                let wander_x = ((world_tick % 100) as f32 - 50.0) / 10.0;
                let wander_y = ((world_tick % 50) as f32 - 25.0) / 10.0;
                
                entity.position.0 += wander_x;
                entity.position.1 += wander_y;
            }
        }
    }
}

/// NPC AI behavior
pub struct NpcAI {
    idle_position: (f32, f32),
    conversation_active: bool,
}

impl NpcAI {
    pub fn new(idle_position: (f32, f32)) -> Self {
        Self {
            idle_position,
            conversation_active: false,
        }
    }
    
    pub fn start_conversation(&mut self) {
        self.conversation_active = true;
    }
    
    pub fn end_conversation(&mut self) {
        self.conversation_active = false;
    }
    
    pub fn update(&mut self, entity: &mut Entity, _world_tick: u64) {
        if !self.conversation_active {
            // Return to idle position
            let dx = self.idle_position.0 - entity.position.0;
            let dy = self.idle_position.1 - entity.position.1;
            let distance = (dx * dx + dy * dy).sqrt();
            
            if distance > 1.0 {
                let speed = 1.0;
                entity.position.0 += (dx / distance) * speed;
                entity.position.1 += (dy / distance) * speed;
            }
        }
    }
}

// ============================================================================
// Phase 9: Advanced Physics & Spatial Optimization
// ============================================================================

/// Physics component for entities with velocity and acceleration
#[derive(Debug, Clone)]
pub struct PhysicsBody {
    pub velocity: (f32, f32),
    pub acceleration: (f32, f32),
    pub mass: f32,
    pub friction: f32,
    pub bounciness: f32,
}

impl PhysicsBody {
    pub fn new(mass: f32) -> Self {
        Self {
            velocity: (0.0, 0.0),
            acceleration: (0.0, 0.0),
            mass,
            friction: 0.1,
            bounciness: 0.3,
        }
    }
    
    pub fn apply_force(&mut self, force_x: f32, force_y: f32) {
        self.acceleration.0 += force_x / self.mass;
        self.acceleration.1 += force_y / self.mass;
    }
    
    pub fn apply_gravity(&mut self, gravity: f32) {
        self.acceleration.1 += gravity;
    }
    
    pub fn update(&mut self, delta_time: f32) {
        // Update velocity with acceleration
        self.velocity.0 += self.acceleration.0 * delta_time;
        self.velocity.1 += self.acceleration.1 * delta_time;
        
        // Apply friction
        self.velocity.0 *= 1.0 - self.friction;
        self.velocity.1 *= 1.0 - self.friction;
        
        // Reset acceleration
        self.acceleration = (0.0, 0.0);
    }
    
    pub fn get_position_delta(&self, delta_time: f32) -> (f32, f32) {
        (
            self.velocity.0 * delta_time,
            self.velocity.1 * delta_time,
        )
    }
}

/// Spatial hash grid for efficient spatial queries
pub struct SpatialGrid {
    cell_size: f32,
    grid: HashMap<(i32, i32), Vec<EntityId>>,
}

impl SpatialGrid {
    pub fn new(cell_size: f32) -> Self {
        Self {
            cell_size,
            grid: HashMap::new(),
        }
    }
    
    fn get_cell(&self, pos: (f32, f32)) -> (i32, i32) {
        (
            (pos.0 / self.cell_size).floor() as i32,
            (pos.1 / self.cell_size).floor() as i32,
        )
    }
    
    pub fn insert(&mut self, entity_id: EntityId, position: (f32, f32)) {
        let cell = self.get_cell(position);
        self.grid.entry(cell).or_insert_with(Vec::new).push(entity_id);
    }
    
    pub fn remove(&mut self, entity_id: EntityId, position: (f32, f32)) {
        let cell = self.get_cell(position);
        if let Some(entities) = self.grid.get_mut(&cell) {
            entities.retain(|&id| id != entity_id);
        }
    }
    
    pub fn query_radius(&self, center: (f32, f32), radius: f32) -> Vec<EntityId> {
        let min_cell = self.get_cell((center.0 - radius, center.1 - radius));
        let max_cell = self.get_cell((center.0 + radius, center.1 + radius));
        
        let mut results = Vec::new();
        for x in min_cell.0..=max_cell.0 {
            for y in min_cell.1..=max_cell.1 {
                if let Some(entities) = self.grid.get(&(x, y)) {
                    results.extend_from_slice(entities);
                }
            }
        }
        results
    }
    
    pub fn clear(&mut self) {
        self.grid.clear();
    }
}

/// Universe coordinator for managing multiple celestial objects
#[derive(Debug, Clone)]
pub struct CelestialCoordinate {
    pub sector_x: i32,
    pub sector_y: i32,
    pub system: i32,
    pub planet: i32,
    pub satellite: Option<i32>,
}

impl CelestialCoordinate {
    pub fn new(sector_x: i32, sector_y: i32, system: i32, planet: i32) -> Self {
        Self {
            sector_x,
            sector_y,
            system,
            planet,
            satellite: None,
        }
    }
    
    pub fn to_world_id(&self) -> String {
        if let Some(sat) = self.satellite {
            format!("CelestialWorld:{}:{}:{}:{}:{}", 
                self.sector_x, self.sector_y, self.system, self.planet, sat)
        } else {
            format!("CelestialWorld:{}:{}:{}:{}", 
                self.sector_x, self.sector_y, self.system, self.planet)
        }
    }
    
    pub fn from_world_id(world_id: &str) -> Option<Self> {
        let parts: Vec<&str> = world_id.split(':').collect();
        if parts.len() < 5 || parts[0] != "CelestialWorld" {
            return None;
        }
        
        Some(Self {
            sector_x: parts[1].parse().ok()?,
            sector_y: parts[2].parse().ok()?,
            system: parts[3].parse().ok()?,
            planet: parts[4].parse().ok()?,
            satellite: if parts.len() > 5 { parts[5].parse().ok() } else { None },
        })
    }
}

/// Universe manager for coordinating multiple worlds
pub struct UniverseManager {
    worlds: Arc<RwLock<HashMap<String, Arc<RwLock<World>>>>>,
    player_locations: Arc<RwLock<HashMap<EntityId, String>>>,
}

impl UniverseManager {
    pub fn new() -> Self {
        Self {
            worlds: Arc::new(RwLock::new(HashMap::new())),
            player_locations: Arc::new(RwLock::new(HashMap::new())),
        }
    }
    
    pub async fn get_or_create_world(&self, world_id: String) -> Arc<RwLock<World>> {
        let mut worlds = self.worlds.write().await;
        
        if let Some(world) = worlds.get(&world_id) {
            return Arc::clone(world);
        }
        
        let world = Arc::new(RwLock::new(World::new(world_id.clone())));
        worlds.insert(world_id, Arc::clone(&world));
        world
    }
    
    pub async fn transfer_entity(&self, entity_id: EntityId, from_world: &str, to_world: &str) -> Result<(), String> {
        // Get both worlds
        let from = self.get_or_create_world(from_world.to_string()).await;
        let to = self.get_or_create_world(to_world.to_string()).await;
        
        // Get entity from source world first
        let entity = {
            let from_lock = from.read().await;
            from_lock.entities.get_entity(entity_id).cloned()
        };
        
        if let Some(entity) = entity {
            // Remove from source world
            {
                let mut from_lock = from.write().await;
                from_lock.entities.remove_entity(entity_id, false);
            }
            
            // Add entity to destination world
            let mut to_lock = to.write().await;
            to_lock.add_entity(entity);
            
            // Update player location tracking
            let mut locations = self.player_locations.write().await;
            locations.insert(entity_id, to_world.to_string());
            
            Ok(())
        } else {
            Err(format!("Entity {} not found in world {}", entity_id, from_world))
        }
    }
    
    pub async fn get_player_world(&self, entity_id: EntityId) -> Option<String> {
        let locations = self.player_locations.read().await;
        locations.get(&entity_id).cloned()
    }
    
    pub async fn list_worlds(&self) -> Vec<String> {
        let worlds = self.worlds.read().await;
        worlds.keys().cloned().collect()
    }
    
    pub async fn unload_world(&self, world_id: &str) -> bool {
        let mut worlds = self.worlds.write().await;
        worlds.remove(world_id).is_some()
    }
}

/// Planet generation parameters
#[derive(Debug, Clone)]
pub struct PlanetParams {
    pub seed: u64,
    pub size: (u32, u32),
    pub biome: String,
    pub threat_level: u8,
    pub has_atmosphere: bool,
    pub temperature: f32,
}

impl PlanetParams {
    pub fn generate(coordinate: &CelestialCoordinate) -> Self {
        // Simple deterministic generation based on coordinates
        let seed = ((coordinate.sector_x as u64) << 48) 
                 | ((coordinate.sector_y as u64) << 32)
                 | ((coordinate.system as u64) << 16)
                 | (coordinate.planet as u64);
        
        // Simple pseudo-random generation
        let size = (1000 + (seed % 3000) as u32, 600 + (seed % 400) as u32);
        let biome_idx = (seed % 10) as usize;
        let biomes = ["forest", "desert", "tundra", "volcanic", "ocean", 
                     "toxic", "alien", "midnight", "savannah", "jungle"];
        let biome = biomes[biome_idx].to_string();
        
        Self {
            seed,
            size,
            biome,
            threat_level: ((seed % 10) as u8) + 1,
            has_atmosphere: (seed % 3) != 0,
            temperature: ((seed % 100) as f32 - 50.0) * 2.0,
        }
    }
    
    pub fn to_world(&self, world_id: String) -> World {
        let mut world = World::new(world_id);
        world.template_data = format!(
            r#"{{"biome":"{}","seed":{},"threatLevel":{}}}"#,
            self.biome, self.seed, self.threat_level
        );
        world.properties = format!(
            r#"{{"gravity":9.8,"breathable":{},"temperature":{}}}"#,
            self.has_atmosphere, self.temperature
        );
        world
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
        let _world2 = manager.get_or_create_world("world2".to_string()).await;
        
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

    #[test]
    fn test_collision_box() {
        let box1 = CollisionBox::new(0.0, 0.0, 10.0, 10.0);
        let box2 = CollisionBox::new(5.0, 5.0, 10.0, 10.0);
        let box3 = CollisionBox::new(20.0, 20.0, 10.0, 10.0);
        
        // Test intersection
        assert!(box1.intersects(&box2));
        assert!(box2.intersects(&box1));
        assert!(!box1.intersects(&box3));
        
        // Test point containment
        assert!(box1.contains_point(5.0, 5.0));
        assert!(!box1.contains_point(15.0, 15.0));
    }

    #[test]
    fn test_collision_system() {
        let mut system = CollisionSystem::new();
        
        // Register entities
        system.register_entity(1, CollisionBox::new(0.0, 0.0, 10.0, 10.0));
        system.register_entity(2, CollisionBox::new(20.0, 0.0, 10.0, 10.0));
        
        // Test collision detection
        let test_box = CollisionBox::new(5.0, 0.0, 10.0, 10.0);
        let collisions = system.find_collisions(&test_box);
        
        assert_eq!(collisions.len(), 1);
        assert!(collisions.contains(&1));
        
        // Test position check
        assert!(system.check_collision(3, (5.0, 5.0), (5.0, 5.0)));
        assert!(!system.check_collision(3, (50.0, 50.0), (5.0, 5.0)));
    }

    #[tokio::test]
    async fn test_world_save_load() {
        let temp_dir = std::env::temp_dir();
        let world_path = temp_dir.join("test_world.dat");
        
        // Create and save a world
        let world = World::new("test_save_world".to_string());
        world.save_to_file(&world_path).await.unwrap();
        
        // Load it back
        let loaded = World::load_from_file(&world_path).await.unwrap();
        assert_eq!(loaded.id, "test_world");
        
        // Cleanup
        let _ = tokio::fs::remove_file(&world_path.with_extension("json")).await;
    }

    #[test]
    fn test_pathfinder() {
        let mut pathfinder = Pathfinder::new();
        
        // Create a simple grid
        for x in 0..10 {
            for y in 0..10 {
                pathfinder.set_walkable(x, y);
            }
        }
        
        // Block some tiles to create an obstacle
        pathfinder.set_blocked(5, 3);
        pathfinder.set_blocked(5, 4);
        pathfinder.set_blocked(5, 5);
        
        // Find path around obstacle
        let path = pathfinder.find_path((0, 4), (9, 4));
        assert!(path.is_some());
        
        let path = path.unwrap();
        assert!(path.len() > 10); // Should go around the obstacle
        assert_eq!(path.first(), Some(&(0, 4)));
        assert_eq!(path.last(), Some(&(9, 4)));
    }

    #[test]
    fn test_pathfinder_no_path() {
        let mut pathfinder = Pathfinder::new();
        
        // Create two separate walkable areas
        pathfinder.set_walkable(0, 0);
        pathfinder.set_walkable(1, 0);
        pathfinder.set_walkable(9, 9);
        pathfinder.set_walkable(8, 9);
        
        // No path between disconnected areas
        let path = pathfinder.find_path((0, 0), (9, 9));
        assert!(path.is_none());
    }

    #[test]
    fn test_behavior_tree_sequence() {
        let mut entity = Entity::new_player(1, (0.0, 0.0));
        
        let children: Vec<Box<dyn BehaviorNode>> = vec![
            Box::new(WaitNode::new(5)),
            Box::new(MoveTowardsNode::new((10.0, 10.0), 2.0)),
        ];
        
        let mut sequence = SequenceNode::new(children);
        
        // First execution should run wait
        let status = sequence.execute(&mut entity, 0);
        assert_eq!(status, BehaviorStatus::Running);
        
        // After wait completes, should move to MoveTowards
        let status = sequence.execute(&mut entity, 5);
        assert_eq!(status, BehaviorStatus::Running);
    }

    #[test]
    fn test_behavior_tree_selector() {
        let mut entity = Entity::new_player(1, (0.0, 0.0));
        
        let children: Vec<Box<dyn BehaviorNode>> = vec![
            Box::new(MoveTowardsNode::new((1.0, 1.0), 2.0)),
            Box::new(WaitNode::new(5)),
        ];
        
        let mut selector = SelectorNode::new(children);
        
        // Should start with first child (MoveTowards)
        let status = selector.execute(&mut entity, 0);
        assert_eq!(status, BehaviorStatus::Running);
    }

    #[test]
    fn test_move_towards_node() {
        let mut entity = Entity::new_player(1, (0.0, 0.0));
        let mut node = MoveTowardsNode::new((10.0, 0.0), 2.0);
        
        // Move towards target
        let status = node.execute(&mut entity, 0);
        assert_eq!(status, BehaviorStatus::Running);
        assert!(entity.position.0 > 0.0);
        
        // Eventually reaches target
        entity.position = (9.5, 0.0);
        let status = node.execute(&mut entity, 1);
        assert_eq!(status, BehaviorStatus::Success);
    }

    #[test]
    fn test_wait_node() {
        let mut entity = Entity::new_player(1, (0.0, 0.0));
        let mut node = WaitNode::new(10);
        
        // Should be running initially
        let status = node.execute(&mut entity, 0);
        assert_eq!(status, BehaviorStatus::Running);
        
        // Still running before duration
        let status = node.execute(&mut entity, 5);
        assert_eq!(status, BehaviorStatus::Running);
        
        // Success after duration
        let status = node.execute(&mut entity, 10);
        assert_eq!(status, BehaviorStatus::Success);
    }

    #[test]
    fn test_monster_ai() {
        let mut entity = Entity::new(1, EntityType::Monster);
        entity.position = (0.0, 0.0);
        let mut ai = MonsterAI::new();
        
        // Set target
        ai.set_target((10.0, 10.0));
        ai.update(&mut entity, 0);
        
        // Should move towards target
        assert!(entity.position.0 > 0.0);
        assert!(entity.position.1 > 0.0);
        
        // Clear target for wander
        ai.clear_target();
        let old_pos = entity.position;
        ai.update(&mut entity, 120);
        
        // Should have wandered
        assert_ne!(entity.position, old_pos);
    }

    #[test]
    fn test_npc_ai() {
        let mut entity = Entity::new(1, EntityType::Npc);
        entity.position = (10.0, 10.0);
        let mut ai = NpcAI::new((5.0, 5.0));
        
        // Should return to idle position
        ai.update(&mut entity, 0);
        assert!(entity.position.0 < 10.0 || entity.position.1 < 10.0);
        
        // During conversation, should stay put
        entity.position = (10.0, 10.0);
        ai.start_conversation();
        let old_pos = entity.position;
        ai.update(&mut entity, 1);
        assert_eq!(entity.position, old_pos);
        
        // After conversation, return to idle
        ai.end_conversation();
        ai.update(&mut entity, 2);
        assert!(entity.position.0 < 10.0 || entity.position.1 < 10.0);
    }

    #[test]
    fn test_physics_body() {
        let mut body = PhysicsBody::new(1.0);
        
        // Apply force
        body.apply_force(10.0, 0.0);
        body.update(0.1);
        
        // Should have velocity
        assert!(body.velocity.0 > 0.0);
        
        // Apply gravity
        body.apply_gravity(-9.8);
        body.update(0.1);
        
        // Should have negative Y velocity
        assert!(body.velocity.1 < 0.0);
    }

    #[test]
    fn test_spatial_grid() {
        let mut grid = SpatialGrid::new(10.0);
        
        // Insert entities
        grid.insert(1, (5.0, 5.0));
        grid.insert(2, (15.0, 15.0));
        grid.insert(3, (35.0, 35.0));
        
        // Query radius - returns entities in cells within radius
        // (5, 5) and (15, 15) are in cells 0 and 1, within radius
        // (35, 35) is in cell 3, which should be outside query range
        let results = grid.query_radius((5.0, 5.0), 15.0);
        assert!(results.contains(&1));
        assert!(results.contains(&2));
        // Note: Spatial grid uses cell-based queries for efficiency
        // so it may return entities in nearby cells even if slightly outside radius
    }

    #[test]
    fn test_celestial_coordinate() {
        let coord = CelestialCoordinate::new(1, 2, 3, 4);
        let world_id = coord.to_world_id();
        assert_eq!(world_id, "CelestialWorld:1:2:3:4");
        
        // Parse back
        let parsed = CelestialCoordinate::from_world_id(&world_id);
        assert!(parsed.is_some());
        let parsed = parsed.unwrap();
        assert_eq!(parsed.sector_x, 1);
        assert_eq!(parsed.sector_y, 2);
        assert_eq!(parsed.system, 3);
        assert_eq!(parsed.planet, 4);
    }

    #[tokio::test]
    async fn test_universe_manager() {
        let universe = UniverseManager::new();
        
        // Create worlds
        let world1 = universe.get_or_create_world("world1".to_string()).await;
        let world2 = universe.get_or_create_world("world2".to_string()).await;
        
        // Add entity to world1
        {
            let mut w1 = world1.write().await;
            let entity_id = w1.entities.allocate_id();
            let entity = Entity::new_player(entity_id, (0.0, 0.0));
            w1.add_entity(entity);
        }
        
        // Transfer entity
        let result = universe.transfer_entity(1, "world1", "world2").await;
        assert!(result.is_ok());
        
        // Check player location
        let location = universe.get_player_world(1).await;
        assert_eq!(location, Some("world2".to_string()));
    }

    #[test]
    fn test_planet_generation() {
        let coord = CelestialCoordinate::new(0, 0, 1, 1);
        let params = PlanetParams::generate(&coord);
        
        // Should generate deterministic parameters
        assert!(params.size.0 >= 1000);
        assert!(params.size.1 >= 600);
        assert!(params.threat_level >= 1 && params.threat_level <= 10);
        
        // Same coordinates should generate same planet
        let params2 = PlanetParams::generate(&coord);
        assert_eq!(params.seed, params2.seed);
        assert_eq!(params.biome, params2.biome);
    }

    #[test]
    fn test_planet_to_world() {
        let coord = CelestialCoordinate::new(1, 2, 3, 4);
        let params = PlanetParams::generate(&coord);
        let world = params.to_world(coord.to_world_id());
        
        // Should have generated properties
        assert!(world.template_data.contains("biome"));
        assert!(world.properties.contains("gravity"));
    }

    #[test]
    fn test_spatial_grid_remove() {
        let mut grid = SpatialGrid::new(10.0);
        
        grid.insert(1, (5.0, 5.0));
        grid.insert(2, (5.0, 5.0));
        
        let results = grid.query_radius((5.0, 5.0), 5.0);
        assert_eq!(results.len(), 2);
        
        grid.remove(1, (5.0, 5.0));
        let results = grid.query_radius((5.0, 5.0), 5.0);
        assert_eq!(results.len(), 1);
        assert!(results.contains(&2));
    }

    #[test]
    fn test_physics_friction() {
        let mut body = PhysicsBody::new(1.0);
        body.velocity = (10.0, 0.0);
        
        // Update multiple times
        for _ in 0..10 {
            body.update(0.1);
        }
        
        // Velocity should decrease due to friction
        assert!(body.velocity.0 < 10.0);
        assert!(body.velocity.0 > 0.0);
    }
}
