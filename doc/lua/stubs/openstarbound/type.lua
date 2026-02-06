--- @meta

--- @alias Json boolean|integer|number|string|table|JsonObject
--- @alias JsonObject table<string, Json>
---
--- @alias Optional<T> T?
--- @alias List<T> T[]
--- @alias Map<K, V> table<K, V>
--- @alias Set<T> T[]
--- @alias Pair<K, V> [K, V]

--- @alias Vec2F [number, number]
--- @alias Vec2I [integer, integer]
--- @alias Vec2U [integer, integer]
--- @alias Vec3F [number, number, number]
--- @alias Vec3I [integer, integer, integer]
--- @alias Vec4F [number, number, number, number]
--- @alias Vec4I [integer, integer, integer, integer]
--- @alias RectF [number, number, number, number]
--- @alias RectI [integer, integer, integer, integer]
--- @alias RectU [integer, integer, integer, integer]
--- @alias Mat3F [[number, number, number], [number, number, number], [number, number, number]]
---
--- @alias PolyF Vec2F[]
--- @alias PolyI Vec2I[]

--- @alias Color Vec3I|Vec4I

--- @alias Drawable
--- | {filepath:string, position: Vec2F, rotation:number?, transformation: Mat3F?, fullbright:boolean?, centered: boolean?, mirrored: boolean?, color:Color?,}
--- | {line: [Vec2F, Vec2F], width: number, rotation:number?, transformation: Mat3F?, fullbright:boolean?, centered: boolean?, mirrored: boolean?, color:Color?,}
--- | {poly: PolyF, rotation:number?, transformation: Mat3F?, fullbright:boolean?, centered: boolean?, mirrored: boolean?, color:Color?,}

--- @alias PlayerMode "casual"|"survival"|"hardcore"
--- @alias EssentialItem "essentialItem"|"essentialItemWithTag
--- @alias TeamType "friendly"|"enemy"|"passive"|"ghostly"|"assistant"|"environment"|"indiscriminate"|"pvp"
--- @alias DamageTeam {type: TeamType, team: integer}
--- @alias ItemDescriptor {name: string, count: integer, parameters: JsonObject?}
--- @alias Personality {}
--- @alias PhysicsForceRegion table<string, any>
--- @alias ActorMovementParameters table<string, any>
--- @alias ActorMovementModifiers table<string, any>
--- @alias MovementParameters table<string, any>
--- @alias SystemLocation table<string, any>
--- @alias CelestialCoordinate table<string, any>
--- @alias CelestialParameters table<string, any>
--- @alias VisitableParameters table<string, any>
--- @alias Orbit table<string, any>
--- @alias playerShipPosition table<string, any>
--- @alias DamageSource
--- | {poly: PolyF[], damage: integer, trackSourceEntity: boolean, sourceEntity: integer, team: DamageTeam, damageType: string, damageSourceKind: string, statusEffects: table[], knockback: integer, rayCheck: boolean, damageRepeatTimeout: number, damageRepeatGroup: string}
--- | {line: PolyF[], damage: integer, trackSourceEntity: boolean, sourceEntity: integer, team: DamageTeam, damageType: string, damageSourceKind: string, statusEffects: table[], knockback: integer, rayCheck: boolean, damageRepeatTimeout: number, damageRepeatGroup: string}
--- @alias QuestParameters table<string, any>
--- @alias Collection table<string, any>
--- @alias Collectable table<string, any>
--- @alias DamageRequest {damageType: string, damage: integer, sourceEntity: integer, targetEntityId: integer?, position: Vec2F?, healthLost: integer?, hitType: string?, damageSourceKind: string?, targetMaterialKind: string?, killed: boolean?}
--- @alias StatModifier table<string, any>
--- @alias ItemBag table<string, any>
--- @alias CollisionSet string[]
--- @alias LiquidLevel {liquidId: integer, amount: number}
--- @alias PlatformerAStar_Path Vec2F[]
--- @alias PlatformerAStar_Parameters table<string, any>
--- @alias TextPositioning {position: Vec2F, horizontalAnchor: "left"|"mid"|"right", verticalAnchor: "top"|"mid"|"bottom", wrapWidth: integer?}

--- @class Image
--- @field size fun(self: Image): Vec2I
--- @field drawInto fun(self: Image, position: Vec2U, other: Image)
--- @field copyInto fun(self: Image, position: Vec2U, other: Image)
--- @field set fun(self: Image, x: integer, y: integer, color: Color)
--- @field get fun(self: Image, x: integer, y: integer): Color
--- @field subImage fun(self: Image, min: Vec2U, size: Vec2U): Image
--- @field process fun(self: Image, directives: string): Image

--- @class PaneId

--- @class CanvasWidget
--- @field size fun(self: CanvasWidget): Vec2I
--- @field clear fun(self: CanvasWidget)
--- @field mousePosition fun(self: CanvasWidget): Vec2I
--- @field drawDrawable fun(self: CanvasWidget, drawable: Drawable, position: Vec2F?)
--- @field drawDrawables fun(self: CanvasWidget, drawables: Drawable[], position: Vec2F?)
--- @field drawJsonDrawable fun(self: CanvasWidget, drawable: Json, position: Vec2F?)
--- @field drawJsonDrawables fun(self: CanvasWidget, drawables: Json[], position: Vec2F?)
--- @field drawImage fun(self: CanvasWidget, image: string, position: Vec2F, scale: number?, color: Color?, centered: boolean?)
--- @field drawImageDrawable fun(self: CanvasWidget, image: Image, position: Vec2F, scale: (number|Vec2F)?, color: Color?, rotation: number?)
--- @field drawImageRect fun(self: CanvasWidget, texName: string, texCoords: RectF, screenCoords: RectF, color: Color?)
--- @field drawTiledImage fun(self: CanvasWidget, image: string, offset: Vec2F, screenCoords: RectF, scale: number, color: Color?)
--- @field drawLine fun(self: CanvasWidget, startPos: Vec2F, endPos: Vec2F, color: Color?, lineWidth: number)
--- @field drawRect fun(self: CanvasWidget, rect: RectF, color: Color?)
--- @field drawPoly fun(self: CanvasWidget, poly: PolyF, color: Color?, lineWidth: number)
--- @field drawTriangles fun(self: CanvasWidget, triangles: PolyF[], color: Color?)
--- @field drawText fun(self: CanvasWidget, text: string, textPositioning: TextPositioning, fontSize: integer, color: Color?, lineSpacing: number?, font: string?, directives: string?)

--- @class RpcPromise<T>
--- @field finished fun(self: RpcPromise<T>): boolean
--- @field succeeded fun(self: RpcPromise<T>): boolean
--- @field result fun(self: RpcPromise<T>): T
--- @field error fun(self: RpcPromise<T>): string

--- @class BehaviorState

--- @class RandomSource
--- @field init fun(self: RandomSource, seed: integer)
--- @field addEntropy fun(self: RandomSource, seed: integer)
--- @field randu32 fun(self: RandomSource): integer
--- @field randu64 fun(self: RandomSource): integer
--- @field randi32 fun(self: RandomSource): integer
--- @field randi64 fun(self: RandomSource): integer
--- @field randf fun(self: RandomSource, min: number?, max: number?): number
--- @field randb fun(self: RandomSource): boolean

--- @class PerlinSource
--- @field get fun(self: PerlinSource, x: number, y: number?, z: number?): number

--- @class PlatformerAStar_PathFinder
--- @field explore fun(self: PlatformerAStar_PathFinder, nodeLimit: integer): boolean
--- @field result fun(self: PlatformerAStar_PathFinder): PlatformerAStar_Path?

--- @param list any[]?
--- @return [any, ...]
function jarray(list) end

---@param obj table<string, any>?
---@return table<string, any>
function jobject(obj) end
