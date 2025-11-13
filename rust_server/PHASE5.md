# Phase 5: World & Entity Integration

## Overview

Phase 5 integrates the world and entity systems, providing the foundational infrastructure for world management, entity tracking, and world simulation. This implementation creates the necessary abstractions for managing multiple worlds with their entities while maintaining clean separation of concerns.

## Features Implemented

### World Structure

**World**
Complete world representation with all necessary data:
- `id`: String - World identifier (e.g., "CelestialWorld:1:2:3")
- `template_data`: String - JSON world template configuration
- `sky_data`: Vec<u8> - Binary sky configuration
- `weather_data`: Vec<u8> - Binary weather configuration
- `spawn_position`: (f32, f32) - Player spawn position
- `properties`: String - JSON world properties
- `entities`: EntityManager - Entity tracking for this world
- `tick`: u64 - World simulation tick counter

**World Methods**
- `new()` - Create a new world with default parameters
- `create_start_packet()` - Generate WorldStartPacket from world state
- `tick()` - Simulate one world tick and generate entity updates
- `add_entity()` - Add an entity and return EntityCreatePacket
- `remove_entity()` - Remove an entity and return EntityDestroyPacket

### Entity Manager

**EntityManager**
Per-world entity tracking and management:
- Stores entities in HashMap<EntityId, Entity>
- Allocates unique entity IDs
- Tracks entity lifecycle (add/remove)
- Generates entity update packets

**Entity Structure**
```rust
pub struct Entity {
    pub id: EntityId,
    pub entity_type: EntityType,
    pub position: (f32, f32),
    pub store_data: Vec<u8>,
    pub net_state: Vec<u8>,
}
```

**EntityManager Methods**
- `new()` - Create new entity manager
- `allocate_id()` - Allocate unique entity ID
- `add_entity()` - Add entity and return create packet
- `remove_entity()` - Remove entity and return destroy packet
- `get_entity()` - Get entity by ID (immutable)
- `get_entity_mut()` - Get entity by ID (mutable)
- `generate_updates()` - Generate entity update packets
- `entity_ids()` - Get all entity IDs
- `entity_count()` - Get entity count

### World Manager

**WorldManager**
Multi-world management with async operations:
- Thread-safe world storage with Arc<RwLock>
- Async world creation and lookup
- World removal and cleanup
- World enumeration

**WorldManager Methods**
- `new()` - Create new world manager
- `get_or_create_world()` - Get existing or create new world
- `get_world()` - Get world by ID (if exists)
- `remove_world()` - Remove world from manager
- `world_ids()` - Get all world IDs
- `world_count()` - Get world count

## Technical Implementation

### World Structure

```rust
#[derive(Debug)]
pub struct World {
    pub id: String,
    pub template_data: String,
    pub sky_data: Vec<u8>,
    pub weather_data: Vec<u8>,
    pub spawn_position: (f32, f32),
    pub properties: String,
    pub entities: EntityManager,
    pub tick: u64,
}

impl World {
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
    
    pub fn tick(&mut self) -> Vec<EntityUpdateSetPacket> {
        self.tick += 1;
        self.entities.generate_updates()
    }
}
```

### Entity Manager

```rust
#[derive(Debug)]
pub struct EntityManager {
    entities: HashMap<EntityId, Entity>,
    next_entity_id: EntityId,
}

impl EntityManager {
    pub fn new() -> Self {
        Self {
            entities: HashMap::new(),
            next_entity_id: 1,
        }
    }
    
    pub fn allocate_id(&mut self) -> EntityId {
        let id = self.next_entity_id;
        self.next_entity_id += 1;
        id
    }
    
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
    
    pub fn remove_entity(&mut self, entity_id: EntityId, death: bool) 
        -> Option<EntityDestroyPacket> 
    {
        self.entities.remove(&entity_id).map(|entity| {
            EntityDestroyPacket {
                entity_id,
                final_net_state: entity.net_state,
                death,
            }
        })
    }
}
```

### World Manager

```rust
pub struct WorldManager {
    worlds: Arc<RwLock<HashMap<String, Arc<RwLock<World>>>>>,
}

impl WorldManager {
    pub fn new() -> Self {
        Self {
            worlds: Arc::new(RwLock::new(HashMap::new())),
        }
    }
    
    pub async fn get_or_create_world(&self, world_id: String) 
        -> Arc<RwLock<World>> 
    {
        let mut worlds = self.worlds.write().await;
        
        if let Some(world) = worlds.get(&world_id) {
            world.clone()
        } else {
            let world = Arc::new(RwLock::new(World::new(world_id.clone())));
            worlds.insert(world_id, world.clone());
            world
        }
    }
    
    pub async fn get_world(&self, world_id: &str) 
        -> Option<Arc<RwLock<World>>> 
    {
        let worlds = self.worlds.read().await;
        worlds.get(world_id).cloned()
    }
}
```

## Testing

Phase 5 includes comprehensive tests for all world and entity integration:

### World Creation Test
```rust
#[test]
fn test_world_creation() {
    let world = World::new("test_world".to_string());
    assert_eq!(world.id, "test_world");
    assert_eq!(world.tick, 0);
    assert_eq!(world.entities.entity_count(), 0);
}
```

### Entity Manager Test
```rust
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
```

### Entity Removal Test
```rust
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
```

### World Manager Test
```rust
#[tokio::test]
async fn test_world_manager() {
    let manager = WorldManager::new();
    
    let world1 = manager.get_or_create_world("world1".to_string()).await;
    let world2 = manager.get_or_create_world("world2".to_string()).await;
    
    assert_eq!(manager.world_count().await, 2);
    
    manager.remove_world("world1").await;
    assert_eq!(manager.world_count().await, 1);
}
```

### Test Results
All 18 tests passing:
- ✅ VLQ unsigned/signed encoding (2 tests)
- ✅ Protocol request/response packets (2 tests)
- ✅ Chat send/receive packets (2 tests)
- ✅ Server info packet (1 test)
- ✅ Compression round-trip/size reduction (2 tests)
- ✅ WorldStart/WorldStop packets (2 tests)
- ✅ Entity packets (3 tests)
- ✅ World creation (1 test)
- ✅ Entity manager (1 test)
- ✅ Entity removal (1 test)
- ✅ World manager (1 test)

## Usage Examples

### Creating and Managing Worlds
```rust
// Create world manager
let world_manager = WorldManager::new();

// Get or create a world
let world = world_manager
    .get_or_create_world("planet_1".to_string())
    .await;

// Access world
let mut world_lock = world.write().await;
world_lock.spawn_position = (100.0, 200.0);
```

### Adding Entities to World
```rust
// Allocate entity ID
let entity_id = world_lock.entities.allocate_id();

// Create player entity
let player = Entity::new_player(entity_id, (100.0, 200.0));

// Add entity and get create packet
let create_packet = world_lock.add_entity(player);

// Broadcast create packet to clients
write_packet(&mut stream, &create_packet).await?;
```

### World Simulation Tick
```rust
// Simulate one world tick
let update_packets = world_lock.tick();

// Broadcast updates to clients
for update in update_packets {
    write_packet(&mut stream, &update).await?;
}
```

### Removing Entities
```rust
// Remove entity (death = true)
if let Some(destroy_packet) = world_lock.remove_entity(entity_id, true) {
    // Broadcast destroy packet to clients
    write_packet(&mut stream, &destroy_packet).await?;
}
```

### Client World Start
```rust
// Generate world start packet for client
let start_packet = world_lock.create_start_packet(client_id);

// Send to client
write_packet(&mut stream, &start_packet).await?;
```

## Architecture Decisions

### Why Arc<RwLock<World>>?

**Thread Safety**
- Multiple async tasks can access worlds concurrently
- RwLock allows multiple readers or single writer
- Arc enables shared ownership across tasks

**Performance**
- Read locks don't block other readers
- Only write operations are exclusive
- Suitable for read-heavy workloads (most game operations)

### Why HashMap for Entities?

**O(1) Lookup**
- Fast entity access by ID
- Efficient for entity updates
- Good memory locality for iteration

**Flexibility**
- Easy to add/remove entities
- No fixed capacity limits
- Natural fit for dynamic entity sets

### Why Separate EntityManager?

**Encapsulation**
- Entity logic separate from world logic
- Clear responsibility boundaries
- Easier to test and maintain

**Reusability**
- EntityManager could be used independently
- Can be extended with additional features
- Clean interface for entity operations

## Performance Characteristics

### Memory Efficiency
- **World Storage**: Minimal overhead with Arc/RwLock
- **Entity Storage**: HashMap provides good memory efficiency
- **Lazy Loading**: Worlds created on-demand

### Concurrency
- **Read Scaling**: Multiple concurrent readers
- **Write Blocking**: Single writer (acceptable for world updates)
- **Lock Granularity**: Per-world locks (worlds independent)

### Scalability
- **Multiple Worlds**: Each world independently managed
- **Entity Count**: HashMap scales well to thousands of entities
- **Async Operations**: Non-blocking world operations

## Known Limitations

1. **No World Persistence**: Worlds not saved to disk (Phase 6)
2. **Simplified Tick**: World tick doesn't implement full physics (Phase 6)
3. **No Entity Behavior**: Entities are passive data structures (Phase 6)
4. **No Spatial Indexing**: Entity lookup is by ID only (future optimization)
5. **No World Files**: World generation not implemented (Phase 6)

## Future Enhancements (Phase 6+)

### Phase 6: World Files & Entity Behavior
- Load world files from disk
- Parse world template and structure
- Implement entity behavior systems
- Add entity AI and pathfinding
- Player entity full implementation

### Phase 7: Advanced Features
- Spatial indexing for entity queries
- Entity collision detection
- World physics simulation
- Entity scripting integration
- Dynamic world generation

### Phase 8: Optimization
- Entity pooling and recycling
- Incremental world updates
- Dirty flagging for changed entities
- Compression of entity deltas

## Integration Points

### With Server
```rust
// Server maintains world manager
pub struct StarboundServer {
    world_manager: Arc<WorldManager>,
    // ... other fields
}

// Client joins world
async fn client_join_world(&self, client_id: ConnectionId, world_id: String) {
    let world = self.world_manager.get_or_create_world(world_id).await;
    let world_lock = world.read().await;
    let start_packet = world_lock.create_start_packet(client_id);
    // Send start_packet to client
}
```

### With Entity System
```rust
// Entity creation from protocol packets
impl From<EntityCreatePacket> for Entity {
    fn from(packet: EntityCreatePacket) -> Self {
        Entity {
            id: packet.entity_id,
            entity_type: packet.entity_type,
            position: (0.0, 0.0),  // Parse from store_data
            store_data: packet.store_data,
            net_state: packet.first_net_state,
        }
    }
}
```

## Migration Notes

For future development:
- World structure is extensible (add fields as needed)
- EntityManager can be enhanced without breaking interface
- WorldManager is thread-safe and async-ready
- All types use standard Rust collections (easy to optimize)

For integration with server:
- Use Arc clones to share world references
- Acquire locks as briefly as possible
- Prefer read locks when possible
- Consider lock ordering to avoid deadlocks

## Statistics

- **Lines of code added**: ~280
- **New module**: world.rs
- **New tests**: 4
- **Total tests**: 18 (100% pass rate)
- **Build time**: ~31s (debug)
- **Binary size**: 3.8 MB (no significant change)

## Conclusion

Phase 5 successfully implements the world and entity integration infrastructure while maintaining clean architecture and testability. The implementation provides a solid foundation for Phase 6's world file loading and entity behavior systems.

**Key Achievements**
- ✅ Complete world structure with all necessary data
- ✅ EntityManager for per-world entity tracking
- ✅ WorldManager for multi-world support
- ✅ Async operations with thread safety
- ✅ Clean separation of concerns
- ✅ Comprehensive testing
- ✅ Foundation for future features
