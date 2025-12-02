//! Damage types compatible with C++ Star::DamageTypes
//!
//! Provides damage-related enums and types used in combat.

use crate::error::Error;
use crate::types::game_types::ConnectionId;
use crate::types::Json;

/// Damage type enum
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum DamageType {
    #[default]
    NoDamage = 0,
    Damage = 1,
    IgnoresDef = 2,
    Knockback = 3,
    Environment = 4,
    Status = 5,
}

impl DamageType {
    /// Parse damage type from string
    pub fn from_str(s: &str) -> Option<DamageType> {
        match s.to_lowercase().as_str() {
            "nodamage" => Some(DamageType::NoDamage),
            "damage" => Some(DamageType::Damage),
            "ignoresdef" => Some(DamageType::IgnoresDef),
            "knockback" => Some(DamageType::Knockback),
            "environment" => Some(DamageType::Environment),
            "status" => Some(DamageType::Status),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            DamageType::NoDamage => "NoDamage",
            DamageType::Damage => "Damage",
            DamageType::IgnoresDef => "IgnoresDef",
            DamageType::Knockback => "Knockback",
            DamageType::Environment => "Environment",
            DamageType::Status => "Status",
        }
    }
}

impl std::fmt::Display for DamageType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Hit type enum
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum HitType {
    #[default]
    Hit = 0,
    StrongHit = 1,
    WeakHit = 2,
    ShieldHit = 3,
    Kill = 4,
}

impl HitType {
    /// Parse hit type from string
    pub fn from_str(s: &str) -> Option<HitType> {
        match s.to_lowercase().as_str() {
            "hit" => Some(HitType::Hit),
            "stronghit" => Some(HitType::StrongHit),
            "weakhit" => Some(HitType::WeakHit),
            "shieldhit" => Some(HitType::ShieldHit),
            "kill" => Some(HitType::Kill),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            HitType::Hit => "Hit",
            HitType::StrongHit => "StrongHit",
            HitType::WeakHit => "WeakHit",
            HitType::ShieldHit => "ShieldHit",
            HitType::Kill => "Kill",
        }
    }
}

impl std::fmt::Display for HitType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Team type for damage calculation
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
#[repr(u8)]
pub enum TeamType {
    #[default]
    Null = 0,
    /// Non-PvP-enabled players and player allied NPCs
    Friendly = 1,
    /// Hostile and neutral NPCs and monsters
    Enemy = 2,
    /// PvP-enabled players
    PVP = 3,
    /// Cannot damage anything, can be damaged by Friendly/PVP/Assistant
    Passive = 4,
    /// Cannot damage or be damaged
    Ghostly = 5,
    /// Cannot damage enemies, can be damaged by anything except enemy
    Environment = 6,
    /// Damages anything except ghostly, damaged by anything except ghostly/passive
    /// Used for self damage
    Indiscriminate = 7,
    /// Cannot damage friendlies and cannot be damaged by anything
    Assistant = 8,
}

impl TeamType {
    /// Parse team type from string
    pub fn from_str(s: &str) -> Option<TeamType> {
        match s.to_lowercase().as_str() {
            "null" => Some(TeamType::Null),
            "friendly" => Some(TeamType::Friendly),
            "enemy" => Some(TeamType::Enemy),
            "pvp" => Some(TeamType::PVP),
            "passive" => Some(TeamType::Passive),
            "ghostly" => Some(TeamType::Ghostly),
            "environment" => Some(TeamType::Environment),
            "indiscriminate" => Some(TeamType::Indiscriminate),
            "assistant" => Some(TeamType::Assistant),
            _ => None,
        }
    }
    
    /// Get string name
    pub fn name(&self) -> &'static str {
        match self {
            TeamType::Null => "null",
            TeamType::Friendly => "friendly",
            TeamType::Enemy => "enemy",
            TeamType::PVP => "pvp",
            TeamType::Passive => "passive",
            TeamType::Ghostly => "ghostly",
            TeamType::Environment => "environment",
            TeamType::Indiscriminate => "indiscriminate",
            TeamType::Assistant => "assistant",
        }
    }
}

impl std::fmt::Display for TeamType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.name())
    }
}

/// Team number type
pub type TeamNumber = u16;

/// Entity damage team for damage calculation
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct EntityDamageTeam {
    /// The team type
    pub team_type: TeamType,
    /// The team number (for sub-teams within a type)
    pub team: TeamNumber,
}

impl EntityDamageTeam {
    /// Create a new entity damage team
    pub fn new(team_type: TeamType, team: TeamNumber) -> Self {
        EntityDamageTeam { team_type, team }
    }
    
    /// Create from team type only (team number = 0)
    pub fn from_type(team_type: TeamType) -> Self {
        EntityDamageTeam { team_type, team: 0 }
    }
    
    /// Create from JSON
    pub fn from_json(json: &Json) -> Option<Self> {
        if let Some(obj) = json.as_object() {
            let team_type = obj.get("type")
                .and_then(|v| v.as_str())
                .and_then(|s| TeamType::from_str(s))
                .unwrap_or(TeamType::Null);
            
            let team = obj.get("team")
                .and_then(|v| v.to_int())
                .map(|n| n as TeamNumber)
                .unwrap_or(0);
            
            Some(EntityDamageTeam { team_type, team })
        } else {
            None
        }
    }
    
    /// Convert to JSON
    pub fn to_json(&self) -> Json {
        let mut obj = serde_json::Map::new();
        obj.insert("type".to_string(), serde_json::Value::String(self.team_type.name().to_string()));
        obj.insert("team".to_string(), serde_json::Value::Number(serde_json::Number::from(self.team)));
        Json::from(serde_json::Value::Object(obj))
    }
    
    /// Check if this team can damage another team
    pub fn can_damage(&self, victim: EntityDamageTeam, victim_is_self: bool) -> bool {
        use TeamType::*;
        
        match self.team_type {
            Null => false,
            
            Friendly => {
                match victim.team_type {
                    Enemy | Passive | Indiscriminate => true,
                    _ => false,
                }
            }
            
            Enemy => {
                match victim.team_type {
                    Friendly | PVP | Passive | Environment | Indiscriminate | Assistant => true,
                    _ => false,
                }
            }
            
            PVP => {
                match victim.team_type {
                    Enemy | Passive | Indiscriminate => true,
                    PVP if self.team != victim.team => true,
                    Friendly if victim_is_self => true,
                    _ => false,
                }
            }
            
            Passive => false,
            
            Ghostly => false,
            
            Environment => {
                match victim.team_type {
                    Friendly | PVP | Assistant => true,
                    _ => false,
                }
            }
            
            Indiscriminate => {
                match victim.team_type {
                    Ghostly | Passive => false,
                    _ => true,
                }
            }
            
            Assistant => {
                match victim.team_type {
                    Enemy | Passive | Indiscriminate => true,
                    _ => false,
                }
            }
        }
    }
    
    /// Read from a byte slice
    pub fn read_from_bytes(bytes: &[u8]) -> Result<(Self, usize), Error> {
        if bytes.len() < 3 {
            return Err(Error::Serialization("Not enough bytes for EntityDamageTeam".to_string()));
        }
        
        let team_type_byte = bytes[0];
        let team_type = match team_type_byte {
            0 => TeamType::Null,
            1 => TeamType::Friendly,
            2 => TeamType::Enemy,
            3 => TeamType::PVP,
            4 => TeamType::Passive,
            5 => TeamType::Ghostly,
            6 => TeamType::Environment,
            7 => TeamType::Indiscriminate,
            8 => TeamType::Assistant,
            _ => TeamType::Null,
        };
        
        let team = u16::from_le_bytes([bytes[1], bytes[2]]);
        
        Ok((EntityDamageTeam { team_type, team }, 3))
    }
    
    /// Write to a byte vector
    pub fn write_to_bytes(&self, bytes: &mut Vec<u8>) {
        bytes.push(self.team_type as u8);
        bytes.extend_from_slice(&self.team.to_le_bytes());
    }
    
    /// Convert to bytes
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::with_capacity(3);
        self.write_to_bytes(&mut bytes);
        bytes
    }
}

/// Get solo PvP team number for a connection
pub fn solo_pvp_team(client_id: ConnectionId) -> TeamNumber {
    // Each client gets a unique team number based on their connection ID
    client_id
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_damage_type() {
        assert_eq!(DamageType::from_str("damage"), Some(DamageType::Damage));
        assert_eq!(DamageType::from_str("IgnoresDef"), Some(DamageType::IgnoresDef));
        assert_eq!(DamageType::Knockback.name(), "Knockback");
    }
    
    #[test]
    fn test_hit_type() {
        assert_eq!(HitType::from_str("hit"), Some(HitType::Hit));
        assert_eq!(HitType::from_str("Kill"), Some(HitType::Kill));
        assert_eq!(HitType::StrongHit.name(), "StrongHit");
    }
    
    #[test]
    fn test_team_type() {
        assert_eq!(TeamType::from_str("friendly"), Some(TeamType::Friendly));
        assert_eq!(TeamType::from_str("pvp"), Some(TeamType::PVP));
        assert_eq!(TeamType::Ghostly.name(), "ghostly");
    }
    
    #[test]
    fn test_entity_damage_team() {
        let friendly = EntityDamageTeam::from_type(TeamType::Friendly);
        let enemy = EntityDamageTeam::from_type(TeamType::Enemy);
        let passive = EntityDamageTeam::from_type(TeamType::Passive);
        let ghostly = EntityDamageTeam::from_type(TeamType::Ghostly);
        
        // Friendly can damage enemies and passives
        assert!(friendly.can_damage(enemy, false));
        assert!(friendly.can_damage(passive, false));
        
        // Friendly cannot damage other friendlies
        assert!(!friendly.can_damage(friendly, false));
        
        // Enemy can damage friendlies
        assert!(enemy.can_damage(friendly, false));
        
        // Ghostly cannot damage or be damaged
        assert!(!ghostly.can_damage(friendly, false));
        assert!(!friendly.can_damage(ghostly, false));
        
        // Passive cannot damage anything
        assert!(!passive.can_damage(friendly, false));
        assert!(!passive.can_damage(enemy, false));
    }
    
    #[test]
    fn test_pvp_teams() {
        let pvp1 = EntityDamageTeam::new(TeamType::PVP, 1);
        let pvp2 = EntityDamageTeam::new(TeamType::PVP, 2);
        let pvp_same = EntityDamageTeam::new(TeamType::PVP, 1);
        
        // Different PvP teams can damage each other
        assert!(pvp1.can_damage(pvp2, false));
        
        // Same PvP team cannot damage each other
        assert!(!pvp1.can_damage(pvp_same, false));
    }
    
    #[test]
    fn test_self_damage() {
        let friendly = EntityDamageTeam::from_type(TeamType::Friendly);
        let pvp = EntityDamageTeam::from_type(TeamType::PVP);
        
        // PVP can damage friendly if victim is self
        assert!(pvp.can_damage(friendly, true));
        
        // PVP cannot damage friendly if victim is not self
        assert!(!pvp.can_damage(friendly, false));
    }
    
    #[test]
    fn test_solo_pvp_team() {
        assert_eq!(solo_pvp_team(1), 1);
        assert_eq!(solo_pvp_team(100), 100);
    }
    
    #[test]
    fn test_serialization() {
        let team = EntityDamageTeam::new(TeamType::Friendly, 42);
        
        let bytes = team.to_bytes();
        let (read_team, bytes_read) = EntityDamageTeam::read_from_bytes(&bytes).unwrap();
        
        assert_eq!(bytes_read, 3);
        assert_eq!(team, read_team);
    }
}
