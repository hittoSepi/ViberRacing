# Networking Architecture

## Overview

ViberRacing uses a P2P architecture with one peer acting as the authoritative host for game state.

## Connection Model

```
┌─────────────────────────────────────────┐
│              Host (Peer 0)              │
│  ┌───────────┐ ┌───────────────────┐   │
│  │ Game State│ │ Track Data        │   │
│  │ Authority │ │ (Editor Changes)  │   │
│  └─────┬─────┘ └─────────┬─────────┘   │
└────────┼─────────────────┼─────────────┘
         │                 │
    ┌────┴────┐       ┌────┴────┐
    │ Client1 │       │ Client2 │
    └─────────┘       └─────────┘
```

## ENet Configuration

- **Channels**:
  - Channel 0: Reliable (track data, events)
  - Channel 1: Unreliable (vehicle state, position)
  - Channel 2: State sync (tick-based updates)
  - Channel 3: Voice (future)

- **Default Ports**: 7777 (configurable)

## Message Types

| Type | Direction | Reliable | Frequency |
|------|-----------|----------|-----------|
| VehicleState | Bidirectional | No | 30 Hz |
| VehicleInput | Client→Host | No | 60 Hz |
| TrackEdit | Bidirectional | Yes | On change |
| TrackData | Host→Client | Yes | On join |
| PlayerJoin/Leave | Host→Client | Yes | On event |

## State Synchronization

### Vehicle State (30 Hz from Host)
```
struct VehicleStateSync {
    u32 playerId;
    u32 sequence;
    vec3 position;
    quat rotation;
    vec3 velocity;
    vec3 angularVelocity;
    float steeringAngle;
    float throttle;
    float brake;
}
```

### Input Prediction
Clients send input to host at 60 Hz. The host validates and broadcasts authoritative state back. Clients interpolate between received states for smooth visuals.

### Track Editor Sync
All track modifications go through the host:
1. Client sends `TrackEdit` command (reliable)
2. Host validates and applies
3. Host broadcasts to all clients
4. Clients apply same edit locally

## Latency Compensation

- **Client-side**: Input prediction for responsive controls
- **Server-side**: State validation, anti-cheat light checks
- **Interpolation**: 100ms buffer for remote vehicles

## Future Improvements

- [ ] Rollback netcode for competitive racing
- [ ] Lag compensation for collision detection
- [ ] Spectator mode with delayed sync
- [ ] Dedicated server support
