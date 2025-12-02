//! Celestial types for star system and planet representation.
//!
//! Compatible with C++ Star::CelestialTypes from StarCelestialTypes.hpp

use crate::math::{Vec2I, Vec3I};
use crate::types::{Json, Either};
use crate::serialization::{DataReader, DataWriter, Readable, Writable};
use crate::error::Result;
use std::io::{Read, Write};
use std::collections::HashMap;
use std::fmt;

/// Celestial coordinate for identifying celestial bodies.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct CelestialCoordinate {
    /// X position in the universe.
    pub x: i32,
    /// Y position in the universe.
    pub y: i32,
    /// Z position (system coordinate).
    pub z: i32,
    /// Planet index (0 for star system itself).
    pub planet: i32,
    /// Satellite index (0 for planet itself).
    pub satellite: i32,
}

impl Default for CelestialCoordinate {
    fn default() -> Self {
        Self {
            x: 0,
            y: 0,
            z: 0,
            planet: 0,
            satellite: 0,
        }
    }
}

impl CelestialCoordinate {
    /// Create a new celestial coordinate.
    pub fn new(x: i32, y: i32, z: i32, planet: i32, satellite: i32) -> Self {
        Self { x, y, z, planet, satellite }
    }

    /// Create a coordinate for a star system.
    pub fn system(location: Vec3I) -> Self {
        Self {
            x: location.x(),
            y: location.y(),
            z: location.z(),
            planet: 0,
            satellite: 0,
        }
    }

    /// Create a coordinate for a planet.
    pub fn planet(location: Vec3I, planet: i32) -> Self {
        Self {
            x: location.x(),
            y: location.y(),
            z: location.z(),
            planet,
            satellite: 0,
        }
    }

    /// Create a coordinate for a satellite.
    pub fn satellite(location: Vec3I, planet: i32, satellite: i32) -> Self {
        Self {
            x: location.x(),
            y: location.y(),
            z: location.z(),
            planet,
            satellite,
        }
    }

    /// Get the system location.
    pub fn system_location(&self) -> Vec3I {
        Vec3I::new(self.x, self.y, self.z)
    }

    /// Check if this is a system coordinate (not a planet or satellite).
    pub fn is_system(&self) -> bool {
        self.planet == 0 && self.satellite == 0
    }

    /// Check if this is a planet coordinate (not a satellite).
    pub fn is_planet(&self) -> bool {
        self.planet != 0 && self.satellite == 0
    }

    /// Check if this is a satellite coordinate.
    pub fn is_satellite(&self) -> bool {
        self.satellite != 0
    }

    /// Get the parent system coordinate.
    pub fn parent_system(&self) -> Self {
        Self::system(self.system_location())
    }

    /// Get the parent planet coordinate (if this is a satellite).
    pub fn parent_planet(&self) -> Option<Self> {
        if self.is_satellite() {
            Some(Self::planet(self.system_location(), self.planet))
        } else {
            None
        }
    }

    /// Parse from a string like "x:y:z:planet:satellite" or "x:y:z".
    pub fn from_string(s: &str) -> Option<Self> {
        let parts: Vec<&str> = s.split(':').collect();
        if parts.len() < 3 {
            return None;
        }

        let x = parts[0].parse().ok()?;
        let y = parts[1].parse().ok()?;
        let z = parts[2].parse().ok()?;
        let planet = parts.get(3).and_then(|s| s.parse().ok()).unwrap_or(0);
        let satellite = parts.get(4).and_then(|s| s.parse().ok()).unwrap_or(0);

        Some(Self { x, y, z, planet, satellite })
    }
}

impl fmt::Display for CelestialCoordinate {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.satellite != 0 {
            write!(f, "{}:{}:{}:{}:{}", self.x, self.y, self.z, self.planet, self.satellite)
        } else if self.planet != 0 {
            write!(f, "{}:{}:{}:{}", self.x, self.y, self.z, self.planet)
        } else {
            write!(f, "{}:{}:{}", self.x, self.y, self.z)
        }
    }
}

/// Celestial parameters for a celestial body.
#[derive(Debug, Clone, Default)]
pub struct CelestialParameters {
    /// The seed for procedural generation.
    pub seed: u64,
    /// The celestial type name.
    pub celestial_type: String,
    /// The celestial name.
    pub name: String,
    /// Additional parameters as JSON.
    pub parameters: Json,
}

impl CelestialParameters {
    /// Create new celestial parameters.
    pub fn new(seed: u64, celestial_type: impl Into<String>, name: impl Into<String>) -> Self {
        Self {
            seed,
            celestial_type: celestial_type.into(),
            name: name.into(),
            parameters: Json::null(),
        }
    }

    /// Create from JSON.
    pub fn from_json(json: &Json) -> Option<Self> {
        Some(Self {
            seed: json.get_key("seed")?.to_uint()?,
            celestial_type: json.get_key("celestialType")?.as_str()?.to_string(),
            name: json.get_key("name")?.as_str()?.to_string(),
            parameters: json.get_key("parameters").unwrap_or(Json::null()),
        })
    }

    /// Convert to JSON.
    pub fn to_json(&self) -> Json {
        let mut obj = serde_json::Map::new();
        obj.insert("seed".to_string(), serde_json::Value::Number(self.seed.into()));
        obj.insert("celestialType".to_string(), serde_json::Value::String(self.celestial_type.clone()));
        obj.insert("name".to_string(), serde_json::Value::String(self.name.clone()));
        if !self.parameters.is_null() {
            obj.insert("parameters".to_string(), self.parameters.clone().into_inner());
        }
        Json::from(serde_json::Value::Object(obj))
    }
}

/// Celestial orbit region configuration.
#[derive(Debug, Clone)]
pub struct CelestialOrbitRegion {
    /// Region name.
    pub region_name: String,
    /// Orbit range (min, max).
    pub orbit_range: (i32, i32),
    /// Probability of a body appearing.
    pub body_probability: f32,
    /// Weighted pool of planetary types.
    pub planetary_types: Vec<(String, f32)>,
    /// Weighted pool of satellite types.
    pub satellite_types: Vec<(String, f32)>,
}

/// Celestial planet with satellites.
#[derive(Debug, Clone, Default)]
pub struct CelestialPlanet {
    /// Planet parameters.
    pub planet_parameters: CelestialParameters,
    /// Satellite parameters by index.
    pub satellite_parameters: HashMap<i32, CelestialParameters>,
}

impl CelestialPlanet {
    /// Create a new celestial planet.
    pub fn new(parameters: CelestialParameters) -> Self {
        Self {
            planet_parameters: parameters,
            satellite_parameters: HashMap::new(),
        }
    }

    /// Add a satellite.
    pub fn add_satellite(&mut self, index: i32, parameters: CelestialParameters) {
        self.satellite_parameters.insert(index, parameters);
    }

    /// Get a satellite.
    pub fn get_satellite(&self, index: i32) -> Option<&CelestialParameters> {
        self.satellite_parameters.get(&index)
    }
}

/// System objects for a celestial system.
#[derive(Debug, Clone, Default)]
pub struct CelestialSystemObjects {
    /// System location.
    pub system_location: Vec3I,
    /// Planets by index.
    pub planets: HashMap<i32, CelestialPlanet>,
}

impl CelestialSystemObjects {
    /// Create new system objects.
    pub fn new(location: Vec3I) -> Self {
        Self {
            system_location: location,
            planets: HashMap::new(),
        }
    }

    /// Add a planet.
    pub fn add_planet(&mut self, index: i32, planet: CelestialPlanet) {
        self.planets.insert(index, planet);
    }

    /// Get a planet.
    pub fn get_planet(&self, index: i32) -> Option<&CelestialPlanet> {
        self.planets.get(&index)
    }
}

/// Constellation as a list of line segments.
pub type CelestialConstellation = Vec<(Vec2I, Vec2I)>;

/// Chunk of celestial data.
#[derive(Debug, Clone, Default)]
pub struct CelestialChunk {
    /// Chunk index.
    pub chunk_index: Vec2I,
    /// Constellations in this chunk.
    pub constellations: Vec<CelestialConstellation>,
    /// System parameters by location.
    pub system_parameters: HashMap<Vec3I, CelestialParameters>,
    /// System objects (planets and satellites) by location.
    pub system_objects: HashMap<Vec3I, HashMap<i32, CelestialPlanet>>,
}

impl CelestialChunk {
    /// Create a new empty chunk.
    pub fn new(index: Vec2I) -> Self {
        Self {
            chunk_index: index,
            constellations: Vec::new(),
            system_parameters: HashMap::new(),
            system_objects: HashMap::new(),
        }
    }

    /// Create from JSON.
    pub fn from_json(json: &Json) -> Option<Self> {
        let chunk_index = json.get_key("chunkIndex")
            .and_then(|v| {
                let arr = v.as_array()?;
                Some(Vec2I::new(
                    arr.first()?.to_int()? as i32,
                    arr.get(1)?.to_int()? as i32,
                ))
            })?;

        Some(Self {
            chunk_index,
            constellations: Vec::new(), // Would need more complex parsing
            system_parameters: HashMap::new(),
            system_objects: HashMap::new(),
        })
    }

    /// Convert to JSON.
    pub fn to_json(&self) -> Json {
        let mut obj = serde_json::Map::new();
        obj.insert("chunkIndex".to_string(), serde_json::json!([self.chunk_index.x(), self.chunk_index.y()]));
        Json::from(serde_json::Value::Object(obj))
    }
}

/// Celestial request type.
pub type CelestialRequest = Either<Vec2I, Vec3I>;

/// Celestial response type.
pub type CelestialResponse = Either<CelestialChunk, CelestialSystemObjects>;

/// Base celestial information.
#[derive(Debug, Clone)]
pub struct CelestialBaseInformation {
    /// Number of orbital levels for planets.
    pub planet_orbital_levels: i32,
    /// Number of orbital levels for satellites.
    pub satellite_orbital_levels: i32,
    /// Size of celestial chunks.
    pub chunk_size: i32,
    /// XY coordinate range.
    pub xy_coord_range: (i32, i32),
    /// Z coordinate range.
    pub z_coord_range: (i32, i32),
    /// Whether to enforce coordinate range.
    pub enforce_coord_range: bool,
}

impl Default for CelestialBaseInformation {
    fn default() -> Self {
        Self {
            planet_orbital_levels: 8,
            satellite_orbital_levels: 3,
            chunk_size: 64,
            xy_coord_range: (-1000000, 1000000),
            z_coord_range: (-100, 100),
            enforce_coord_range: true,
        }
    }
}

// Serialization implementations

impl Readable for CelestialCoordinate {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            x: reader.read_var_i32()?,
            y: reader.read_var_i32()?,
            z: reader.read_var_i32()?,
            planet: reader.read_var_i32()?,
            satellite: reader.read_var_i32()?,
        })
    }
}

impl Writable for CelestialCoordinate {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_var_i32(self.x)?;
        writer.write_var_i32(self.y)?;
        writer.write_var_i32(self.z)?;
        writer.write_var_i32(self.planet)?;
        writer.write_var_i32(self.satellite)?;
        Ok(())
    }
}

impl Readable for CelestialParameters {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            seed: reader.read_u64()?,
            celestial_type: reader.read_string()?,
            name: reader.read_string()?,
            parameters: {
                let s = reader.read_string()?;
                if s.is_empty() { Json::null() } else { Json::parse(&s)? }
            },
        })
    }
}

impl Writable for CelestialParameters {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_u64(self.seed)?;
        writer.write_string(&self.celestial_type)?;
        writer.write_string(&self.name)?;
        if self.parameters.is_null() {
            writer.write_string("")?;
        } else {
            writer.write_string(&self.parameters.to_string())?;
        }
        Ok(())
    }
}

impl Readable for CelestialBaseInformation {
    fn read<R: Read>(reader: &mut DataReader<R>) -> Result<Self> {
        Ok(Self {
            planet_orbital_levels: reader.read_var_i32()?,
            satellite_orbital_levels: reader.read_var_i32()?,
            chunk_size: reader.read_var_i32()?,
            xy_coord_range: (reader.read_var_i32()?, reader.read_var_i32()?),
            z_coord_range: (reader.read_var_i32()?, reader.read_var_i32()?),
            enforce_coord_range: reader.read_bool()?,
        })
    }
}

impl Writable for CelestialBaseInformation {
    fn write<W: Write>(&self, writer: &mut DataWriter<W>) -> Result<()> {
        writer.write_var_i32(self.planet_orbital_levels)?;
        writer.write_var_i32(self.satellite_orbital_levels)?;
        writer.write_var_i32(self.chunk_size)?;
        writer.write_var_i32(self.xy_coord_range.0)?;
        writer.write_var_i32(self.xy_coord_range.1)?;
        writer.write_var_i32(self.z_coord_range.0)?;
        writer.write_var_i32(self.z_coord_range.1)?;
        writer.write_bool(self.enforce_coord_range)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_celestial_coordinate_default() {
        let coord = CelestialCoordinate::default();
        assert_eq!(coord.x, 0);
        assert_eq!(coord.y, 0);
        assert_eq!(coord.z, 0);
        assert!(coord.is_system());
    }

    #[test]
    fn test_celestial_coordinate_system() {
        let coord = CelestialCoordinate::system(Vec3I::new(10, 20, 30));
        assert_eq!(coord.x, 10);
        assert_eq!(coord.y, 20);
        assert_eq!(coord.z, 30);
        assert!(coord.is_system());
    }

    #[test]
    fn test_celestial_coordinate_planet() {
        let coord = CelestialCoordinate::planet(Vec3I::new(10, 20, 30), 3);
        assert!(coord.is_planet());
        assert!(!coord.is_satellite());
        assert_eq!(coord.planet, 3);
    }

    #[test]
    fn test_celestial_coordinate_satellite() {
        let coord = CelestialCoordinate::satellite(Vec3I::new(10, 20, 30), 2, 1);
        assert!(coord.is_satellite());
        assert_eq!(coord.satellite, 1);
        
        let parent = coord.parent_planet().unwrap();
        assert!(parent.is_planet());
        assert_eq!(parent.planet, 2);
    }

    #[test]
    fn test_celestial_coordinate_from_string() {
        let coord = CelestialCoordinate::from_string("10:20:30").unwrap();
        assert_eq!(coord.x, 10);
        assert_eq!(coord.y, 20);
        assert_eq!(coord.z, 30);
        assert!(coord.is_system());

        let coord2 = CelestialCoordinate::from_string("10:20:30:2:1").unwrap();
        assert_eq!(coord2.planet, 2);
        assert_eq!(coord2.satellite, 1);
    }

    #[test]
    fn test_celestial_coordinate_display() {
        let system = CelestialCoordinate::system(Vec3I::new(10, 20, 30));
        assert_eq!(format!("{}", system), "10:20:30");

        let planet = CelestialCoordinate::planet(Vec3I::new(10, 20, 30), 3);
        assert_eq!(format!("{}", planet), "10:20:30:3");

        let satellite = CelestialCoordinate::satellite(Vec3I::new(10, 20, 30), 2, 1);
        assert_eq!(format!("{}", satellite), "10:20:30:2:1");
    }

    #[test]
    fn test_celestial_parameters() {
        let params = CelestialParameters::new(12345, "TerrestrialWorld", "Earth");
        assert_eq!(params.seed, 12345);
        assert_eq!(params.celestial_type, "TerrestrialWorld");
        assert_eq!(params.name, "Earth");
    }

    #[test]
    fn test_celestial_planet() {
        let mut planet = CelestialPlanet::new(CelestialParameters::new(100, "Planet", "Test"));
        planet.add_satellite(1, CelestialParameters::new(101, "Moon", "Moon1"));
        
        assert!(planet.get_satellite(1).is_some());
        assert!(planet.get_satellite(2).is_none());
    }

    #[test]
    fn test_celestial_system_objects() {
        let mut system = CelestialSystemObjects::new(Vec3I::new(1, 2, 3));
        system.add_planet(1, CelestialPlanet::new(CelestialParameters::new(100, "Planet", "P1")));
        
        assert!(system.get_planet(1).is_some());
        assert!(system.get_planet(2).is_none());
    }

    #[test]
    fn test_celestial_coordinate_serialization() {
        let original = CelestialCoordinate::satellite(Vec3I::new(100, 200, 300), 5, 2);
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: CelestialCoordinate = reader.read().unwrap();
        
        assert_eq!(read, original);
    }

    #[test]
    fn test_celestial_parameters_serialization() {
        let original = CelestialParameters::new(99999, "TestType", "TestName");
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: CelestialParameters = reader.read().unwrap();
        
        assert_eq!(read.seed, original.seed);
        assert_eq!(read.celestial_type, original.celestial_type);
        assert_eq!(read.name, original.name);
    }

    #[test]
    fn test_celestial_base_info_serialization() {
        let original = CelestialBaseInformation::default();
        
        let mut buf = Vec::new();
        {
            let mut writer = DataWriter::new(&mut buf);
            original.write(&mut writer).unwrap();
        }
        
        let mut reader = DataReader::new(std::io::Cursor::new(buf));
        let read: CelestialBaseInformation = reader.read().unwrap();
        
        assert_eq!(read.planet_orbital_levels, original.planet_orbital_levels);
        assert_eq!(read.chunk_size, original.chunk_size);
    }
}
