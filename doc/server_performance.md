# Server Performance Optimization Guide

## Overview

OpenStarbound includes several optimizations to improve server performance and multi-core CPU utilization. These optimizations help the server handle more players and worlds efficiently.

## Automatic Thread Pool Sizing

The server automatically detects available CPU cores and configures thread pools accordingly:

### Worker Pool Threads
- **Automatic sizing**: Uses 75% of available CPU cores (minimum 2, maximum 16)
- **Purpose**: Handles world creation, celestial database queries, and background tasks
- **Manual override**: Set `workerPoolThreads` in `universe_server.config`

Example:
```json
{
  "workerPoolThreads": 8
}
```

### Network Worker Threads  
- **Automatic sizing**: Uses hardware_concurrency() / 4 (minimum 2)
- **Purpose**: Handles network packet processing
- **Manual override**: Set `networkWorkerThreads` in `universe_server.config`

Example:
```json
{
  "networkWorkerThreads": 4
}
```

## Performance Tuning

### Hardware Recommendations

For optimal performance:
- **2-4 players**: 4+ CPU cores, 2GB RAM
- **5-10 players**: 6+ CPU cores, 4GB RAM  
- **10+ players**: 8+ CPU cores, 8GB RAM

### Configuration Tips

1. **Let automatic detection work**: Unless you have specific needs, the automatic thread pool sizing provides good defaults

2. **Manual tuning**: If you need to manually configure:
   - Set `workerPoolThreads` to 50-75% of available cores
   - Set `networkWorkerThreads` to 10-25% of available cores
   - Leave some cores for the main thread and world threads

3. **Monitor performance**: Use server logs to check thread utilization:
   ```
   UniverseServer: Starting worker pool with 6 threads
   UniverseConnectionServer: Starting 2 network worker threads
   ```

## Background Processing

The server uses parallel processing for:

- **World generation**: Terrain and structure generation in worker threads
- **World loading**: Multiple worlds can be loaded simultaneously
- **Celestial queries**: Star system and planet queries processed in parallel
- **Network I/O**: Packet processing distributed across network threads

## Troubleshooting

### Server using too much CPU

If the server is using excessive CPU:
1. Reduce `workerPoolThreads` to limit parallel world generation
2. Adjust `serverFidelity` in configuration (automatic/minimum/low/medium/high)
3. Reduce `worldStorageGenerationLevelLimit` to limit terrain generation rate

### Server response is slow

If the server feels sluggish:
1. Increase `workerPoolThreads` if you have spare CPU cores
2. Increase `networkWorkerThreads` to improve packet processing
3. Check that `serverFidelity` is not set to "minimum"

### Out of Memory Errors

If the server runs out of memory:
1. Reduce the number of loaded worlds by decreasing player spread
2. Lower `worldStorageGenerationLevelLimit`
3. Increase server RAM allocation

## Technical Details

### Thread Pool Architecture

The server uses multiple thread pools:

1. **Main Thread**: Handles universe-level updates, client management, and coordination
2. **Worker Pool**: General-purpose thread pool for CPU-intensive background tasks
3. **Network Threads**: Dedicated threads for network I/O and packet processing  
4. **World Threads**: Each active world runs in its own thread

### Scalability

The threading improvements enable:
- Efficient use of multi-core processors (4-32+ cores)
- Parallel world updates when multiple worlds are active
- Non-blocking celestial database queries
- Concurrent world generation and loading

### Performance Monitoring

Monitor these log entries for performance insights:

```
UniverseServer: Starting worker pool with N threads
UniverseConnectionServer: Starting N network worker threads
server_{worldId}_update: XX.XXHz
server_{worldId}_fidelity: medium/high
```

## Future Optimizations

Planned improvements include:
- Parallel entity updates within worlds
- Improved sector-based processing
- Better load balancing across threads
- Enhanced world storage parallelization
