//! Material types for tile-based world representation.
//!
//! Compatible with C++ Star::MaterialTypes from StarMaterialTypes.hpp

/// Material identifier (u16).
pub type MaterialId = u16;

/// Material hue shift (u8).
pub type MaterialHue = u8;

/// Material color variant (u8).
pub type MaterialColorVariant = u8;

/// Mod identifier (u16).
pub type ModId = u16;

// Special material IDs

/// Empty and non-colliding.
pub const EMPTY_MATERIAL_ID: MaterialId = 65535;

/// Empty and colliding. Also used as placeholder in world generation.
pub const NULL_MATERIAL_ID: MaterialId = 65534;

/// Invisible colliding material for pre-drawn world structures.
pub const STRUCTURE_MATERIAL_ID: MaterialId = 65533;

/// Placeholder for biome native ground material (variant 5).
pub const BIOME5_MATERIAL_ID: MaterialId = 65532;

/// Placeholder for biome native ground material (variant 4).
pub const BIOME4_MATERIAL_ID: MaterialId = 65531;

/// Placeholder for biome native ground material (variant 3).
pub const BIOME3_MATERIAL_ID: MaterialId = 65530;

/// Placeholder for biome native ground material (variant 2).
pub const BIOME2_MATERIAL_ID: MaterialId = 65529;

/// Placeholder for biome native ground material (variant 1).
pub const BIOME1_MATERIAL_ID: MaterialId = 65528;

/// Placeholder for biome native ground material.
pub const BIOME_MATERIAL_ID: MaterialId = 65527;

/// Invisible walls that can't be connected to or grappled.
pub const BOUNDARY_MATERIAL_ID: MaterialId = 65526;

/// Default generic object solid metamaterial.
pub const OBJECT_SOLID_MATERIAL_ID: MaterialId = 65500;

/// Default generic object platform metamaterial.
pub const OBJECT_PLATFORM_MATERIAL_ID: MaterialId = 65501;

/// Material IDs 65500 and above are reserved for engine-specified metamaterials.
pub const FIRST_ENGINE_META_MATERIAL_ID: MaterialId = 65500;

/// Material IDs 65000 - 65499 are reserved for configurable metamaterials.
pub const FIRST_META_MATERIAL_ID: MaterialId = 65000;

/// Default material color variant.
pub const DEFAULT_MATERIAL_COLOR_VARIANT: MaterialColorVariant = 0;

/// Maximum material color variant.
pub const MAX_MATERIAL_COLOR_VARIANT: MaterialColorVariant = 8;

// Special mod IDs

/// Tile has no tilemod.
pub const NO_MOD_ID: ModId = 65535;

/// Placeholder mod for biome native ground mod.
pub const BIOME_MOD_ID: ModId = 65534;

/// Placeholder mod for underground biome native ground mod.
pub const UNDERGROUND_BIOME_MOD_ID: ModId = 65533;

/// First mod ID reserved for special hard-coded mod values.
pub const FIRST_META_MOD: ModId = 65520;

/// Convert material hue (0-255) to degrees (0-360).
#[inline]
pub fn material_hue_to_degrees(hue: MaterialHue) -> f32 {
    hue as f32 * 360.0 / 255.0
}

/// Convert degrees (0-360) to material hue (0-255).
#[inline]
pub fn material_hue_from_degrees(degrees: f32) -> MaterialHue {
    ((degrees % 360.0) * 255.0 / 360.0) as MaterialHue
}

/// Check if material ID represents a real (non-meta) material.
#[inline]
pub fn is_real_material(material: MaterialId) -> bool {
    material < FIRST_META_MATERIAL_ID
}

/// Check if material can be connected to other materials.
#[inline]
pub fn is_connectable_material(material: MaterialId) -> bool {
    material != NULL_MATERIAL_ID && material != EMPTY_MATERIAL_ID && material != BOUNDARY_MATERIAL_ID
}

/// Check if material is a biome placeholder material.
#[inline]
pub fn is_biome_material(material: MaterialId) -> bool {
    material == BIOME_MATERIAL_ID || (material >= BIOME1_MATERIAL_ID && material <= BIOME5_MATERIAL_ID)
}

/// Check if mod ID represents a real (non-meta) mod.
#[inline]
pub fn is_real_mod(mod_id: ModId) -> bool {
    mod_id < FIRST_META_MOD
}

/// Check if mod is a biome placeholder mod.
#[inline]
pub fn is_biome_mod(mod_id: ModId) -> bool {
    mod_id == BIOME_MOD_ID || mod_id == UNDERGROUND_BIOME_MOD_ID
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_material_hue_conversion() {
        assert!((material_hue_to_degrees(0) - 0.0).abs() < 0.01);
        assert!((material_hue_to_degrees(127) - 179.29).abs() < 0.1);
        assert!((material_hue_to_degrees(255) - 360.0).abs() < 0.01);
        
        assert_eq!(material_hue_from_degrees(0.0), 0);
        assert_eq!(material_hue_from_degrees(180.0), 127);
        assert_eq!(material_hue_from_degrees(360.0), 0); // wraps
    }

    #[test]
    fn test_is_real_material() {
        assert!(is_real_material(0));
        assert!(is_real_material(1000));
        assert!(is_real_material(64999));
        assert!(!is_real_material(65000));
        assert!(!is_real_material(EMPTY_MATERIAL_ID));
    }

    #[test]
    fn test_is_connectable_material() {
        assert!(is_connectable_material(0));
        assert!(is_connectable_material(1000));
        assert!(!is_connectable_material(NULL_MATERIAL_ID));
        assert!(!is_connectable_material(EMPTY_MATERIAL_ID));
        assert!(!is_connectable_material(BOUNDARY_MATERIAL_ID));
    }

    #[test]
    fn test_is_biome_material() {
        assert!(is_biome_material(BIOME_MATERIAL_ID));
        assert!(is_biome_material(BIOME1_MATERIAL_ID));
        assert!(is_biome_material(BIOME5_MATERIAL_ID));
        assert!(!is_biome_material(0));
        assert!(!is_biome_material(NULL_MATERIAL_ID));
    }

    #[test]
    fn test_is_real_mod() {
        assert!(is_real_mod(0));
        assert!(is_real_mod(1000));
        assert!(!is_real_mod(FIRST_META_MOD));
        assert!(!is_real_mod(NO_MOD_ID));
    }

    #[test]
    fn test_is_biome_mod() {
        assert!(is_biome_mod(BIOME_MOD_ID));
        assert!(is_biome_mod(UNDERGROUND_BIOME_MOD_ID));
        assert!(!is_biome_mod(0));
        assert!(!is_biome_mod(NO_MOD_ID));
    }
}
