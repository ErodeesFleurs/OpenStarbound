# Threading Architecture

## Overview

OpenStarbound uses a multi-threaded architecture to maximize performance on modern multi-core processors. This document describes the threading model and parallelization strategies.

## Thread Types

### 1. Main Thread (Universe Server)
**Purpose**: Central coordination and management

**Responsibilities:**
- Universe-level updates and clock management
- Client connection handling and authentication
- Player warping and ship coordination
- Chat processing and command handling
- World lifecycle management (creation/shutdown)
- Celestial database coordination

**Update Frequency**: Configurable via `mainWakeupInterval` (default ~10ms)

### 2. Worker Pool
**Purpose**: CPU-intensive background tasks

**Automatic Sizing:**
```cpp
// Default: 75% of CPU cores, clamped to [2, 16]
unsigned hwThreads = std::thread::hardware_concurrency();
size_t threads = clamp((hwThreads * 3) / 4, 2, 16);
```

**Tasks:**
- World generation and loading
- Celestial coordinate calculations
- Planet and star system queries
- Asynchronous world promises
- Database operations

**Configuration:**
```json
{
  "workerPoolThreads": 8  // Override automatic detection
}
```

### 3. Network Worker Threads
**Purpose**: Network I/O and packet processing

**Automatic Sizing:**
```cpp
// Default: 25% of CPU cores, minimum 2
size_t threads = max(2, hardware_concurrency() / 4);
```

**Tasks:**
- Packet serialization/deserialization
- Connection management
- Network protocol handling
- Compression/decompression

**Configuration:**
```json
{
  "networkWorkerThreads": 4  // Override automatic detection
}
```

### 4. World Server Threads
**Purpose**: Independent world simulation

**Thread Count**: One per active world (dynamic)

**Responsibilities:**
- Entity updates (players, NPCs, objects, projectiles)
- Liquid simulation
- Tile updates and damage
- Weather simulation
- Script execution (Lua)
- World storage I/O
- Client packet processing

**Update Rate**: 60Hz (configurable via `ServerGlobalTimestep`)

### 5. System World Threads
**Purpose**: Star system simulation

**Thread Count**: One per active system (dynamic)

**Responsibilities:**
- System object updates
- Ship rendering in systems
- Orbital mechanics
- System-level interactions

## Parallelization Strategies

### World Loading
Worlds are loaded in parallel using the worker pool:

```cpp
// Async world creation
auto promise = m_workerPool.addProducer<WorldServerThreadPtr>([...]() {
  return createWorldThread(worldId);
});
```

**Benefits:**
- Multiple worlds load simultaneously
- Non-blocking main thread
- Efficient CPU utilization

### Celestial Queries
Star system and planet lookups use parallel processing:

```cpp
// Process multiple celestial requests in parallel
for (auto request : requests) {
  m_pendingCelestialRequests[clientId].append(
    m_workerPool.addProducer<CelestialResponse>([this, request]() {
      return m_celestialDatabase->respondToRequest(request);
    })
  );
}
```

**Benefits:**
- Non-blocking universe exploration
- Fast system generation
- Concurrent planet queries

### World Storage
Sector generation prioritizes by player proximity:

```cpp
m_worldStorage->generateQueue(limit, [this](Sector a, Sector b) {
  return distanceToClosestPlayer(a) < distanceToClosestPlayer(b);
});
```

**Benefits:**
- Generates terrain near players first
- Background processing during gameplay
- Smooth exploration experience

## Thread Safety

### Locking Strategy

**Main Lock (`m_mainLock`):**
- Universe-level state changes
- World list modifications
- System world management

**Clients Lock (`m_clientsLock`):**
- Client list access (read/write lock)
- Player context updates
- Connection state

**World-Specific Locks:**
- Each world has independent locking
- Minimal lock contention between worlds
- Entity map has spatial locking

### Lock Ordering
To prevent deadlocks, locks must be acquired in this order:
1. Main lock
2. Clients lock (read or write)
3. World-specific locks
4. Entity locks

## Performance Monitoring

### LogMap Metrics

```cpp
// Worker pool statistics
LogMap::set("worker_pool_threads", threadCount);
LogMap::set("worker_pool_pending", pendingWork);
LogMap::set("worker_pool_idle", idleThreads);

// Per-world metrics
LogMap::set("server_{worldId}_update", updateRate);
LogMap::set("server_{worldId}_fidelity", fidelity);
LogMap::set("server_{worldId}_entities", entityCount);
```

### Interpreting Metrics

**High pending work:**
- Worker pool may need more threads
- CPU bottleneck
- Consider increasing `workerPoolThreads`

**All threads idle:**
- Worker pool oversized for load
- Consider reducing threads to save memory
- Normal during low activity

**Low update rate:**
- World simulation bottleneck
- Check entity count and complexity
- May need fidelity adjustment

## Scalability

### CPU Core Scaling

| Cores | Worker Threads | Network Threads | Expected Performance |
|-------|---------------|-----------------|---------------------|
| 2     | 2             | 2               | 1-2 players         |
| 4     | 3             | 2               | 2-4 players         |
| 8     | 6             | 2               | 5-10 players        |
| 16    | 12            | 4               | 10-20 players       |
| 32+   | 16 (capped)   | 8               | 20+ players         |

### Memory Considerations

Each thread consumes stack memory:
- Worker thread: ~1MB stack
- World thread: ~2MB stack + world data
- Network thread: ~1MB stack

**Example (8-core system):**
- 6 worker threads: ~6MB
- 2 network threads: ~2MB
- 5 active worlds: ~20MB
- Total thread overhead: ~30MB

## Best Practices

### For Server Administrators

1. **Let auto-detection work** on modern systems
2. **Monitor metrics** via LogMap to identify bottlenecks
3. **Adjust manually** only if needed for specific workloads
4. **Consider memory** when increasing thread counts

### For Developers

1. **Use worker pool** for CPU-intensive tasks
2. **Avoid blocking** the main thread
3. **Minimize lock contention** with fine-grained locking
4. **Profile** before optimizing
5. **Test** on various CPU configurations

## Adaptive Load Balancing

**Status: Implemented ✅**

The server now includes dynamic thread pool resizing based on real-time load:

### How It Works

1. **Utilization Monitoring**
   - Calculates active threads / total threads
   - Sampled every main loop iteration
   - Logged as `worker_pool_utilization`

2. **Scaling Logic**
   ```
   IF utilization > 85% AND pending_work > thread_count:
     Scale up by 2 threads (max: 75% of CPU cores, capped at 16)
   
   IF utilization < 25% AND pending_work == 0:
     Scale down by 1 thread (min: 2 threads)
   ```

3. **Adjustment Frequency**
   - Checks every 30 seconds
   - Prevents rapid oscillation
   - Only active when `workerPoolThreads` not manually set

### Benefits

- **Automatic adaptation** to varying server load
- **Resource efficiency** during low activity
- **Increased capacity** during high demand
- **No configuration** required

### Example Behavior

**8-core system (max 6 worker threads):**
- Idle server: Scales down to 2 threads
- Light load (40% util): Maintains 2-3 threads
- Medium load (60% util): Scales to 4 threads
- Heavy load (90% util): Scales to 6 threads
- Load decreases: Gradually scales back down

## Future Improvements

Potential areas for further optimization:

1. **Parallel Entity Updates:**
   - Partition entities by spatial regions
   - Update non-interacting entities in parallel
   - Requires careful synchronization
   - **Status**: Research phase - high complexity

2. **Sector-Level Parallelization:**
   - Process world sectors independently
   - Parallel liquid simulation
   - Concurrent tile updates
   - **Status**: Under investigation - liquid engine already optimized

3. **NUMA Awareness:**
   - Thread affinity for large systems
   - Memory locality optimization
   - Core pinning for consistent performance
   - **Status**: Would require new dependencies (hwloc/libnuma)

## References

- [C++ std::thread documentation](https://en.cppreference.com/w/cpp/thread/thread)
- [Thread Pool Pattern](https://en.wikipedia.org/wiki/Thread_pool)
- [Lock-Free Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
