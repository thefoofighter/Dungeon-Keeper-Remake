#pragma once
#include "Creature/ThempCreatureData.h"
namespace Themp
{
	struct RoomData
	{
		uint8_t Number;
		uint16_t Cost;
		uint16_t Health;
	};
	struct MagicData
	{
		uint8_t Number;
		uint16_t Cost[9];
		uint16_t Time;
		uint16_t Power[9];
	};
	struct GameData
	{
		uint8_t Number;
		int32_t Value;
	};
	struct SpellData
	{
		uint8_t Number;
		uint16_t Duration;
	};
	struct ShotData
	{
		uint8_t Number;
		uint16_t Health;
		uint16_t Damage;
		uint16_t Speed;
	};
	struct TrapData
	{
		uint8_t Number;
		uint8_t ManufactureLevel;
		uint8_t Shots;
		uint16_t ManufactureRequired;
		uint16_t TimeBetweenShots;
		uint16_t SellingValue;
	};
	struct DoorData
	{
		uint8_t Number;
		uint8_t ManufactureLevel;
		uint16_t ManufactureRequired;
		uint16_t SellingValue;
		uint16_t Health;
	};
	struct HealthData
	{
		uint8_t Number;
		uint16_t Value;
	};
	struct BlockData
	{
		uint8_t Number;
		uint16_t Value;
	};
	struct InstanceData
	{
		uint8_t Number;
		uint8_t Time;
		uint8_t ActionTime;
		uint8_t FPTime;
		uint8_t FPActionTime;
		uint8_t ForceVisibility;
		uint16_t ResetTime;
		uint16_t FPResetTime;
	};
	struct ResearchData
	{
		uint8_t Model;
		uint32_t Amount;
	};

	enum Jobs {
		JOB_NULL = 0,
		JOB_TUNNEL = 1,
		JOB_DIG = 2,
		JOB_RESEARCH = 4,
		JOB_TRAIN = 8,
		JOB_MANUFACTURE = 16,
		JOB_SCAVENGE = 32,
		JOB_KINKY_TORTURE = 64,
		JOB_FIGHT = 128,
		JOB_SEEK_THE_ENEMY = 256,
		JOB_GUARD = 512,
		JOB_GROUP = 1024,
		JOB_BARRACK = 2048,
		JOB_TEMPLE = 4096,
		JOB_FREEZE_PRISONERS = 8192,
		JOB_EXPLORE = 16384,
	};
	enum Anger_Job
	{
		ANGER_JOB_NULL = 0,
		ANGER_JOB_KILL_CREATURES = 1,
		ANGER_JOB_DESTROY_ROOMS = 2,
		ANGER_JOB_LEAVE_DUNGEON = 4,
		ANGER_JOB_STEAL_GOLD = 8,
		ANGER_JOB_DAMAGE_WALLS = 16,
		ANGER_JOB_MAD_PSYCHO = 32,
		ANGER_JOB_PERSUADE = 64,
		ANGER_JOB_JOIN_ENEMY = 128,
	};
	enum Rooms
	{
		ROOM_NULL,
		ROOM_ENTRANCE,
		ROOM_TREASURE_ROOM,
		ROOM_RESEARCH_ROOM,
		ROOM_PRISON_ROOM,
		ROOM_TORTURE_ROOM,
		ROOM_TRAINING_AREA,
		ROOM_DUNGEON_HEART,
		ROOM_WORKSHOP_ROOM,
		ROOM_SCAVENGER,
		ROOM_TEMPLE,
		ROOM_GRAVEYARD,
		ROOM_BARRACK_ROOM,
		ROOM_HATCHERY,
		ROOM_LAIR,
		ROOM_BRIDGE,
		ROOM_GUARD_POST,
	};
	enum MagicConfig
	{
		MAGIC_NULL,
		MAGIC_HAND,
		MAGIC_CREATE_IMP,
		MAGIC_MUST_OBEY,
		MAGIC_SLAP,
		MAGIC_SIGHT,
		MAGIC_CALL_TO_ARMS,
		MAGIC_CAVE_IN,
		MAGIC_HEAL_CREATURE,
		MAGIC_HOLD_AUDIENCE,
		MAGIC_LIGHTNING,
		MAGIC_SPEED,
		MAGIC_PROTECT,
		MAGIC_CONCEAL,
		MAGIC_DISEASE,
		MAGIC_CHICKEN,
		MAGIC_DESTROY_WALLS,
		MAGIC_TIME_BOMB,
		MAGIC_POSSESS,
		MAGIC_ARMAGEDON,
	};
	enum GameSettings
	{
		GAME_GOLD_PER_GOLD_BLOCK,
		GAME_POT_OF_GOLD_HOLDS,
		GAME_GOLD_PILE_VALUE,
		GAME_GOLD_PILE_MAXIMUM,
		GAME_RECOVERY_FREQUENCY,
		GAME_FIGHT_MAX_HATE,
		GAME_FIGHT_BORDERLINE,
		GAME_FIGHT_MAX_LOVE,
		GAME_FOOD_LIFE_OUT_OF_HATCHERY,
		GAME_FIGHT_HATE_KILL_VALUE,
		GAME_DEATH_MATCH_STATUE_REAPPER_TIME,
		GAME_DEATH_MATCH_OBJECT_REAPPER_TIME,
		GAME_HITS_PER_SLAB,
		GAME_COLLAPSE_DUNGEON_DAMAGE,
		GAME_TURNS_PER_COLLAPSE_DUNGEON_DAMAGE,
		GAME_IMP_JOB_NOT_SO_CLOSE_UNUSED,
		GAME_IMP_JOB_FAR_AWAY_UNUSED,
		GAME_TIME_SPENT_IN_PRISON_UNUSED,
		GAME_AUTO_DIG_LIMIT_UNUSED,
		GAME_DOUBLE_CLICK_SPEED_UNUSED,
		GAME_IMP_GENERATE_TIME_UNUSED,
		GAME_GHOST_CONVERT_CHANCE,
		GAME_ARMORY_TIME,
		GAME_WORKSHOP_TIME,
		GAME_OBSERVATORY_TIME,
		GAME_OBSERVATORY_GENERATE,
		GAME_IMPROVE_AREA,
		GAME_DOOR_OPEN_FOR,
		GAME_BOULDER_REDUCE_HEALTH_SLAP,
		GAME_BOULDER_REDUCE_HEALTH_WALL,
		GAME_BOULDER_REDUCE_HEALTH_ROOM,
		GAME_TILE_STRENGTH,
		GAME_GOLD_TILE_STRENGTH,
		GAME_ARMEGEDDON_COUNT_DOWN,
		GAME_ARMEGEDDON_DURATION,
		GAME_MINIMUM_GOLD,
		GAME_MAX_GOLD_LOOKUP,
		GAME_MIN_GOLD_TO_RECORD,
		GAME_WAIT_FOR_ROOM_TIME,
		GAME_CHECK_EXPAND_TIME,
		GAME_MAX_DISTANCE_TO_DIG,
		GAME_WAIT_AFTER_ROOM_AREA,
		GAME_PER_IMP_GOLD_DIG_SIZE,
		GAME_DEFAULT_GENERATE_SPEED,
		GAME_BARRACK_TIME,
		GAME_FOOD_GENERATION_SPEED,
		GAME_DEFAULT_NEUTRAL_ENTRANCE_LEVEL,
		GAME_DEFAULT_MAX_CREATURES_GENERATE_ENTRANCE,
		GAME_DEFAULT_IMP_DIG_DAMAGE,
		GAME_DEFAULT_IMP_DIG_OWN_DAMAGE,
		GAME_PRISON_SKELETON_CHANCE,
		GAME_DEFAULT_SACRIFICE_SCORE_FOR_HORNY,
		GAME_GAME_TURNS_IN_FLEE,
		GAME_TRAIN_COST_FREQUENCY,
		GAME_TORTURE_CONVERT_CHANCE,
		GAME_PAY_DAY_GAP,
		GAME_CHEST_GOLD_HOLD,
		GAME_SLAB_COLLAPSE_TIME,
		GAME_DUNGEON_HEART_HEALTH,
		GAME_POWER_HAND_GOLD_GRAB_AMOUNT,
		GAME_DUNGEON_HEART_HEAL_TIME,
		GAME_DUNGEON_HEART_HEAL_HEALTH,
		GAME_HERO_DOOR_WAIT_TIME,
		GAME_DISEASE_TRANSFER_PERCENTAGE,
		GAME_DISEASE_LOSE_PERCENTAGE_HEALTH,
		GAME_DISEASE_LOSE_HEALTH_TIME,
		GAME_HOLD_AUDIENCE_TIME,
		GAME_ARMAGEDON_TELEPORT_YOUR_TIME_GAP,
		GAME_ARMAGEDON_TELEPORT_ENEMY_TIME_GAP,
		GAME_SCAVENGE_COST_FREQUENCY,
		GAME_TEMPLE_SCAVENGE_PROTECTION_TIME,
		GAME_BODIES_FOR_VAMPIRE,
		GAME_BODY_REMAINS_FOR,
		GAME_GRAVEYARD_CONVERT_TIME,
		GAME_MIN_DISTANCE_FOR_TELEPORT,
		GAME_NULL,
	};
	enum Spells
	{
		SPELL_NULL,
		SPELL_FIREBALL,
		SPELL_FIRE_BOMB,
		SPELL_FREEZE,
		SPELL_ARMOUR,
		SPELL_LIGHTNING,
		SPELL_REBOUND,
		SPELL_HEAL,
		SPELL_POISON_CLOUD,
		SPELL_INVISIBILITY,
		SPELL_TELEPORT,
		SPELL_SPEED,
		SPELL_SLOW,
		SPELL_DRAIN,
		SPELL_FEAR,
		SPELL_MISSILE,
		SPELL_NAVIGATING_MISSILE,
		SPELL_FLAME_BREATH,
		SPELL_WIND,
		SPELL_LIGHT,
		SPELL_FLY,
		SPELL_SIGHT,
		SPELL_GRENADE,
		SPELL_HAILSTORM,
		SPELL_WORD_OF_POWER,
		SPELL_CRAZY_GAS,
		SPELL_DISEASE,
		SPELL_CHICKEN,
		SPELL_TIME_BOMB,
	};
	enum Shots
	{
		SHOT_NULL,
		SHOT_FIREBALL,
		SHOT_FIREBOMB,
		SHOT_FREEZE,
		SHOT_LIGHTNING,
		SHOT_POISON_CLOUD,
		SHOT_NAVIGATING_MISSILE,
		SHOT_FLAME_BREATHE,
		SHOT_WIND,
		SHOT_MISSILE,
		SHOT_SLOW,
		SHOT_GRENADE,
		SHOT_DRAIN,
		SHOT_HAIL_STORM,
		SHOT_ARROW,
		SHOT_BOULDER,
		SHOT_GOD_LIGHTNING,
		SHOT_SPIKE,
		SHOT_VORTEX,
		SHOT_ALARM,
		SHOT_SOLID_BOULDER,
		SHOT_SWING_WEAPON_SWORD,
		SHOT_SWING_WEAPON_FIST,
		SHOT_DIG,
		SHOT_GOD_LIGHTNING_BALL,
		SHOT_GROUP,
		SHOT_DISEASE,
		SHOT_CHICKEN,
		SHOT_TIME_BOMB,
		SHOT_TRAP_LIGHTNING,
	};
	enum Traps
	{
		TRAP_NULL,
		TRAP_BOULDER,
		TRAP_ALARM,
		TRAP_POISON_GAS,
		TRAP_LIGHTNING,
		TRAP_WORD_OF_POWER,
		TRAP_LAVA,
	};
	enum Doors
	{
		DOOR_NULL,
		DOOR_WOODEN,
		DOOR_BRACE,
		DOOR_STEEL,
		DOOR_MAGIC,
	};
	enum HealthConfig
	{
		HEALTH_HUNGER_HEALTH_LOSS,
		HEALTH_GAME_TURNS_PER_HUNGER_HEALTH_LOSS,
		HEALTH_FOOD_HEALTH_GAIN,
		HEALTH_UNUSED_PRISON_HEALTH_GAIN,
		HEALTH_UNUSED_GAME_TURNS_PER_PRISON_HEALTH_GAIN,
		HEALTH_TORTURE_HEALTH_LOSS,
		HEALTH_GAME_TURNS_PER_TORTURE_HEALTH_LOSS,
		HEALTH_NULL,
	};
	enum BlockHealth
	{
		BLOCK_HEALTH_ROCK,
		BLOCK_HEALTH_GOLD,
		BLOCK_HEALTH_PRETTY,
		BLOCK_HEALTH_FLOOR,
		BLOCK_HEALTH_ROOM,
		BLOCK_HEALTH_DOOR1,
		BLOCK_HEALTH_DOOR2,
		BLOCK_HEALTH_DOOR3,
		BLOCK_HEALTH_DOOR4,
		BLOCK_HEALTH_NULL,
	};
	enum Instance
	{
		INSTANCE_NULL,
		INSTANCE_SWING_WEAPON_SWORD,
		INSTANCE_SWING_WEAPON_FIST,
		INSTANCE_ESCAPE,
		INSTANCE_FIRE_ARROW,
		INSTANCE_FIREBALL,
		INSTANCE_FIRE_BOMB,
		INSTANCE_FREEZE,
		INSTANCE_ARMOUR,
		INSTANCE_LIGHTNING,
		INSTANCE_REBOUND,
		INSTANCE_HEAL,
		INSTANCE_POISON_CLOUD,
		INSTANCE_INVISIBILITY,
		INSTANCE_TELEPORT,
		INSTANCE_SPEED,
		INSTANCE_SLOW,
		INSTANCE_DRAIN,
		INSTANCE_FEAR,
		INSTANCE_MISSILE,
		INSTANCE_NAVIGATING_MISSILE,
		INSTANCE_FLAME_BREATH,
		INSTANCE_WIND,
		INSTANCE_LIGHT,
		INSTANCE_FLY,
		INSTANCE_SIGHT,
		INSTANCE_GRENADE,
		INSTANCE_HAILSTORM,
		INSTANCE_WORD_OF_POWER,
		INSTANCE_FART,
		INSTANCE_DIG,
		INSTANCE_PRETTY_PATH,
		INSTANCE_DESTROY_AREA,
		INSTANCE_TUNNEL,
		INSTANCE_WAVE,
		INSTANCE_REINFORCE,
		INSTANCE_EAT,
		INSTANCE_ATTACK_ROOM_SLAB,
		INSTANCE_DAMAGE_WALL,
		INSTANCE_FIRST_PERSON_DIG,
		INSTANCE_CAST_SPELL_GROUP,
		INSTANCE_CAST_SPELL_DISEASE,
		INSTANCE_CAST_SPELL_CHICKEN,
		INSTANCE_CAST_SPELL_TIME_BOMB,
		INSTANCE_MOAN,
		INSTANCE_TORTURED,
	};
	enum SlabType
	{
		SLAB_TYPE_HARD		 = 0,
		SLAB_TYPE_GOLD		 = 1,
		SLAB_TYPE_ROCK		 = 2,
		SLAB_TYPE_PRETTY_PATH= 11,
		SLAB_TYPE_LAVA		 = 12,
		SLAB_TYPE_WATER		 = 13,
		SLAB_TYPE_NULL		 = 0xFF,
	};
	enum AttackPreference
	{
		ATTACK_PREFERENCE_NULL = 0,
		ATTACK_PREFERENCE_MELEE = 1,
		ATTACK_PREFERENCE_RANGED = 2,
	};
	enum EyeLenses
	{
		EYE_NULL					 ,
		EYE_WIBBLE					 ,
		EYE_FISH_EYE				 ,
		EYE_COMPOUND				 ,
		EYE_MIST_WATER				 ,
		EYE_MIST_TWO_LAYERED_WATER	 ,
		EYE_MIST_DARK_WATER			 ,
		EYE_MIST_BLOB				 ,
		EYE_MIST_TWO_LAYERED_SMOKE	 ,
		EYE_MIST_TWO_LAYED_SMOKE_EDGE,
		EYE_MIST_TWO_LAYER_SHIT		 ,
		EYE_MIST_QUAKE_SHIT			 ,
		EYE_MIST_SMOKE				 ,
		EYE_LENS_DOG				 ,
		EYE_LENS_VAMPIRE			 ,
	};
	enum Research 
	{
		RESEARCH_NULL,
		RESEARCH_MAGIC_HAND				   ,
		RESEARCH_MAGIC_POSSESS			   ,
		RESEARCH_MAGIC_SLAP				   ,
		RESEARCH_MAGIC_CREATE_IMP		   ,
		RESEARCH_ROOM_TREASURE			   ,
		RESEARCH_ROOM_HATCHERY			   ,
		RESEARCH_ROOM_LAIR				   ,
		RESEARCH_ROOM_RESEARCH			   ,
		RESEARCH_ROOM_TRAINING			   ,
		RESEARCH_MAGIC_POWER_SIGHT		   ,
		RESEARCH_ROOM_BRIDGE			   ,
		RESEARCH_MAGIC_POWER_SPEED		   ,
		RESEARCH_MAGIC_POWER_OBEY		   ,
		RESEARCH_ROOM_GUARD_POST		   ,
		RESEARCH_MAGIC_POWER_CALL_TO_ARMS  ,
		RESEARCH_ROOM_WORKSHOP			   ,
		RESEARCH_MAGIC_POWER_CONCEAL	   ,
		RESEARCH_ROOM_BARRACKS			   ,
		RESEARCH_MAGIC_POWER_HOLD_AUDIENCE ,
		RESEARCH_ROOM_PRISON			   ,
		RESEARCH_MAGIC_POWER_CAVE_IN	   ,
		RESEARCH_ROOM_TORTURE			   ,
		RESEARCH_MAGIC_POWER_HEAL_CREATURE ,
		RESEARCH_ROOM_TEMPLE			   ,
		RESEARCH_MAGIC_POWER_LIGHTNING	   ,
		RESEARCH_ROOM_GRAVEYARD			   ,
		RESEARCH_MAGIC_POWER_PROTECT	   ,
		RESEARCH_ROOM_SCAVENGER			   ,
		RESEARCH_MAGIC_POWER_CHICKEN	   ,
		RESEARCH_MAGIC_POWER_DISEASE	   ,
		RESEARCH_MAGIC_POWER_ARMAGEDDON	   ,
		RESEARCH_MAGIC_POWER_DESTROY_WALLS ,
	};
	class LevelConfig
	{
	public:

		~LevelConfig();
		LevelConfig();

		static bool LoadConfiguration();

		static int TypeToRoom(uint16_t type);

		static CreatureData creatureData[32];
		static RoomData roomData[17];
		static MagicData magicData[20];
		static GameData gameSettings[76];
		static SpellData spellData[29];
		static ShotData shotData[30];
		static TrapData trapData[7];
		static DoorData doorData[5];
		static HealthData healthConfig[8];
		static BlockData blockHealth[10];
		static InstanceData instanceData[46];
		static ResearchData researchData[33];
	};
};
