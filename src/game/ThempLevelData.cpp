#include "ThempSystem.h"
#include "ThempLevelData.h"
#include "ThempGame.h"
#include "ThempFileManager.h"
#include "ThempObject2D.h"
#include "ThempGUIButton.h"
#include "ThempFont.h"
#include "ThempResources.h"
#include "ThempCreatureTaskManager.h"
#include "ThempAudio.h"
#include "ThempVoxelObject.h"
#include "../Library/imgui.h"
#include "../Engine/ThempCamera.h"
#include "../Engine/ThempObject3D.h"
#include "../Engine/ThempMesh.h"
#include "../Engine/ThempMaterial.h"
#include "../Engine/ThempD3D.h"
#include "../Engine/ThempFunctions.h"
#include "../Engine/ThempDebugDraw.h"
#include "ThempLevelConfig.h"
#include <DirectXMath.h>
using namespace Themp;

TileMap LevelData::m_Map;
bool LevelData::PathsInvalidated;

uint32_t NextAreaCode = 0;
LevelData::~LevelData()
{

}


LevelData::LevelData(int levelIndex)
{
	m_CurrentLevelNr = levelIndex;
	LoadLevelFileData();
	PathsInvalidated = true;
}
void LevelData::Init()
{
	UpdateArea(0, 84, 0, 84);
	NextAreaCode = 1;
	for (int y = 1; y < 84; y++)
	{
		for (int x = 1; x < 84; x++)
		{
			uint16_t cType = m_Map.m_Tiles[y][x].type & 0xFF;
			if (m_Map.m_Tiles[y][x].areaCode == 0 && !(cType >= Type_Rock && cType <= Type_Wall5 || cType == Type_Lava || cType == Type_Gem))
			{
				UpdateAreaCode(NextAreaCode, cType, y, x);
				NextAreaCode++;
			}
		}
	}
}

float mod(float val, float mod)
{
	return fmod(fmod(val, mod) + mod, mod);
}
float intbound(float s, float ds)
{
	// Find the smallest positive t such that s+t*ds is an integer.
	if (ds < 0)
	{
		return intbound(-s, -ds);
	}
	else
	{
		s = mod(s, 1.0f);
		// problem is now s+t*ds = 1
		return (1 - s) / ds;
	}
}
int signum(float x)
{
	return x >= 0.0 ? 1 : x < 0.0 ? -1 : 0;
}

LevelData::HitData LevelData::Raycast(XMFLOAT3 origin, XMFLOAT3 direction, float range)
{
	int xPos = (int)floor(origin.x);
	int yPos = (int)floor(origin.y);
	int zPos = (int)floor(origin.z);
	int stepX = signum(direction.x);
	int stepY = signum(direction.y);
	int stepZ = signum(direction.z);
	XMFLOAT3 tMax(intbound(origin.x, direction.x), intbound(origin.y, direction.y), intbound(origin.z, direction.z));
	XMFLOAT3 tDelta((float)stepX / direction.x, (float)stepY / direction.y, (float)stepZ / direction.z);

	HitData hitData;
	hitData.hit = false;
	hitData.distance = 99999;
	hitData.posX = 0;
	hitData.posY = 0;
	hitData.posZ = 0;

	while (!(xPos < MAP_SIZE_SUBTILES_RENDER && xPos >= 0 &&
		yPos < MAP_SIZE_HEIGHT && yPos >= 0 &&
		zPos < MAP_SIZE_SUBTILES_RENDER && zPos >= 0))
	{
		if (stepX < 0 && xPos < 0) goto END;
		if (stepY < 0 && yPos < 0) goto END;
		if (stepZ < 0 && zPos < 0) goto END;
		if (xPos >= MAP_SIZE_SUBTILES_RENDER && stepX > 0) goto END;
		if (yPos >= MAP_SIZE_HEIGHT && stepY > 0) goto END;
		if (zPos >= MAP_SIZE_SUBTILES_RENDER && stepZ > 0) goto END;
		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				xPos += stepX;
				tMax.x += tDelta.x;
			}
			else
			{
				zPos += stepZ;
				tMax.z += tDelta.z;
			}
		}
		else
		{
			if (tMax.y < tMax.z)
			{
				yPos += stepY;
				tMax.y += tDelta.y;
			}
			else
			{
				zPos += stepZ;
				tMax.z += tDelta.z;
			}
		}
	}

	while (true)
	{
		if ((xPos < MAP_SIZE_SUBTILES_RENDER && xPos >= 0 &&
			yPos < MAP_SIZE_HEIGHT && yPos >= 0 &&
			zPos < MAP_SIZE_SUBTILES_RENDER && zPos >= 0))
		{
			if (m_BlockMap[(int)round(yPos)][(int)round(zPos)][(int)round(xPos)].active)
			{
				//HitData hitData;
				hitData.hit = true;
				float dist = sqrt(tDelta.x + tDelta.y + tDelta.z);
			
				hitData.distance = (int)round(dist);
				hitData.posX = xPos;
				hitData.posY = yPos;
				hitData.posZ = zPos;
				return hitData;
			}
		}
		if (tMax.x < tMax.y) 
		{
			if (tMax.x < tMax.z) 
			{
				if (tMax.x > range) break;

				xPos += stepX;
				tMax.x += tDelta.x;
			}
			else
			{
				if (tMax.z > range) break;
				zPos += stepZ;
				tMax.z += tDelta.z;
			}
		}
		else
		{
			if (tMax.y < tMax.z) 
			{
				if (tMax.y > range) break;
				yPos += stepY;
				tMax.y += tDelta.y;
			}
			else 
			{
				if (tMax.z > range) break;
				zPos += stepZ;
				tMax.z += tDelta.z;
			}
		}
	}

END:
	return hitData;
}

//Selects a belonging RenderTile for the inputted tile (takes care of selecting the specific pieces of a room)
int LevelData::CreateFromTile(const Tile& tile, RenderTile& out)
{
	int numSolidBlocks = 0;
	uint8_t usingType = (tile.type >> 8) == 10 ? 5 : (tile.type >> 8);
	if (usingType > 0)usingType--;
	switch (tile.type & 0xFF)
	{
	case Type_Rock:
		memcpy(&out, &RenderTileMap[Tile_ROCK], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_ROCK].activeBlocks;
		break;
	case Type_Gold:
	case Type_Earth:
	case Type_Earth_Torch:
		memcpy(&out, &RenderTileMap[Tile_FULLBLOCK], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_FULLBLOCK].activeBlocks;
		break;
	case Type_Wall0:
	case Type_Wall1:
	case Type_Wall2:
	case Type_Wall3:
	case Type_Wall4:
	case Type_Wall5:
		memcpy(&out, &RenderTileMap[Tile_FULLBLOCK], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_FULLBLOCK].activeBlocks;
		break;
	case Type_Unclaimed_Path:
	case Type_Claimed_Land:
		memcpy(&out, &RenderTileMap[Tile_GROUND], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_GROUND].activeBlocks;
		break;
	case Type_Water:
	case Type_Lava:
		memcpy(&out, &RenderTileMap[Tile_LIQUID], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_LIQUID].activeBlocks;
		break;
	case Type_Portal:
		memcpy(&out, &RenderTileMap[Tile_PORTAL + usingType], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_PORTAL + usingType].activeBlocks;
		break;
	case Type_Dungeon_Heart:
		memcpy(&out, &RenderTileMap[Tile_DUNGEON_HEART + usingType], sizeof(RenderTile));
		numSolidBlocks += RenderTileMap[Tile_DUNGEON_HEART + usingType].activeBlocks;
		break;
	}

	return numSolidBlocks;
}

uint8_t LevelData::GetNeighbourInfo(uint16_t currentType, uint16_t nType)
{
	if (currentType == nType || currentType >= Type_Wall0 && currentType <= Type_Wall5 && nType >= Type_Wall0 && nType <= Type_Wall5)
	{
		return N_SAME;
	}
	else
	{
		if (nType == Type_Lava)
		{
			return N_LAVA;
		}
		else if (nType == Type_Water)
		{
			return N_WATER;
		}
		else if (nType != Type_Rock && nType != Type_Gold && nType != Type_Gem && nType != Type_Earth && nType != Type_Earth_Torch && (nType < Type_Wall0 || nType > Type_Wall5))
		{
			return N_WALKABLE;
		}
		return N_DIFF;
	}
	return N_DIFF;
}

TileNeighbours LevelData::CheckNeighbours(uint16_t type, int y, int x)
{
	TileNeighbours nInfo = { 0 };
	uint16_t nType = 0;
	if (y + 1 < MAP_SIZE_TILES)
	{
		nType = m_Map.m_Tiles[y + 1][x].type;
		nInfo.North = GetNeighbourInfo(type, nType);
		if (x + 1 < MAP_SIZE_TILES)
		{
			nType = m_Map.m_Tiles[y + 1][x + 1].type;
			nInfo.NorthEast = GetNeighbourInfo(type, nType);
		}
		if (x - 1 >= 0)
		{
			nType = m_Map.m_Tiles[y + 1][x - 1].type;
			nInfo.NorthWest = GetNeighbourInfo(type, nType);
		}
	}
	if (y - 1 >= 0)
	{
		nType = m_Map.m_Tiles[y - 1][x].type;
		nInfo.South = GetNeighbourInfo(type, nType);
		if (x + 1 < MAP_SIZE_TILES)
		{
			nType = m_Map.m_Tiles[y - 1][x + 1].type;
			nInfo.SouthEast = GetNeighbourInfo(type, nType);
		}
		if (x - 1 >= 0)
		{
			nType = m_Map.m_Tiles[y - 1][x - 1].type;
			nInfo.SouthWest = GetNeighbourInfo(type, nType);
		}
	}
	if (x + 1 < MAP_SIZE_TILES)
	{
		nType = m_Map.m_Tiles[y][x + 1].type;
		nInfo.East = GetNeighbourInfo(type, nType);
	}
	if (x - 1 >= 0)
	{
		nType = m_Map.m_Tiles[y][x - 1].type;
		nInfo.West = GetNeighbourInfo(type, nType);
	}
	return nInfo;
}

TileNeighbourTiles LevelData::GetNeighbourTiles(int y, int x)
{
	TileNeighbourTiles nInfo = { 0 };
	nInfo.NorthWest.owner = m_Map.m_Tiles[y + 1][x - 1].owner;
	nInfo.NorthWest.type = m_Map.m_Tiles[y + 1][x - 1].type;
	nInfo.NorthWest.areaCode = m_Map.m_Tiles[y + 1][x - 1].areaCode;
	nInfo.North.owner = m_Map.m_Tiles[y + 1][x].owner;
	nInfo.North.type = m_Map.m_Tiles[y + 1][x].type;
	nInfo.North.areaCode = m_Map.m_Tiles[y + 1][x].areaCode;
	nInfo.NorthEast.owner = m_Map.m_Tiles[y + 1][x + 1].owner;
	nInfo.NorthEast.type = m_Map.m_Tiles[y + 1][x + 1].type;
	nInfo.NorthEast.areaCode = m_Map.m_Tiles[y + 1][x + 1].areaCode;
	nInfo.SouthWest.owner = m_Map.m_Tiles[y - 1][x - 1].owner;
	nInfo.SouthWest.type = m_Map.m_Tiles[y - 1][x - 1].type;
	nInfo.SouthWest.areaCode = m_Map.m_Tiles[y - 1][x - 1].areaCode;
	nInfo.South.owner = m_Map.m_Tiles[y - 1][x].owner;
	nInfo.South.type = m_Map.m_Tiles[y - 1][x].type;
	nInfo.South.areaCode = m_Map.m_Tiles[y - 1][x].areaCode;
	nInfo.SouthEast.owner = m_Map.m_Tiles[y - 1][x + 1].owner;
	nInfo.SouthEast.type = m_Map.m_Tiles[y - 1][x + 1].type;
	nInfo.SouthEast.areaCode = m_Map.m_Tiles[y - 1][x + 1].areaCode;
	nInfo.East.owner = m_Map.m_Tiles[y][x + 1].owner;
	nInfo.East.type = m_Map.m_Tiles[y][x + 1].type;
	nInfo.East.areaCode = m_Map.m_Tiles[y][x + 1].areaCode;
	nInfo.West.owner = m_Map.m_Tiles[y][x - 1].owner;
	nInfo.West.type = m_Map.m_Tiles[y][x - 1].type;
	nInfo.West.areaCode = m_Map.m_Tiles[y][x - 1].areaCode;
	return nInfo;
}

NeighbourSubTiles LevelData::GetNeighbourSubTiles(int y, int x)
{
	NeighbourSubTiles nInfo = { 0 };
	int xPos = x / 3;
	int yPos = y / 3;
	int xSub = x % 3;
	int ySub = y % 3;

	int xPosPOne = (x+1) / 3;
	int yPosPOne = (y+1) / 3;
	int xSubPOne = (x+1) % 3;
	int ySubPOne = (y+1) % 3;

	int xPosMOne = (x-1) / 3;
	int yPosMOne = (y-1) / 3;
	int xSubMOne = (x-1) % 3;
	int ySubMOne = (y-1) % 3;

	nInfo.NorthWest.owner = m_Map.m_Tiles[yPosPOne][xPosMOne].owner;
	nInfo.NorthWest.type = m_Map.m_Tiles[yPosPOne][xPosMOne].type;
	nInfo.NorthWest.areaCode = m_Map.m_Tiles[yPosPOne][xPosMOne].areaCode;
	nInfo.NorthWest.height = m_Map.m_Tiles[yPosPOne][xPosMOne].pathSubTiles[ySubPOne][xSubMOne].height;
	nInfo.NorthWest.cost = m_Map.m_Tiles[yPosPOne][xPosMOne].pathSubTiles[ySubPOne][xSubMOne].cost;
	nInfo.North.owner = m_Map.m_Tiles[yPosPOne][xPos].owner;
	nInfo.North.type = m_Map.m_Tiles[yPosPOne][xPos].type;
	nInfo.North.areaCode = m_Map.m_Tiles[yPosPOne][xPos].areaCode;
	nInfo.North.height = m_Map.m_Tiles[yPosPOne][xPos].pathSubTiles[ySubPOne][xSub].height;
	nInfo.North.cost = m_Map.m_Tiles[yPosPOne][xPos].pathSubTiles[ySubPOne][xSub].cost;
	nInfo.NorthEast.owner = m_Map.m_Tiles[yPosPOne][xPosPOne].owner;
	nInfo.NorthEast.type = m_Map.m_Tiles[yPosPOne][xPosPOne].type;
	nInfo.NorthEast.areaCode = m_Map.m_Tiles[yPosPOne][xPosPOne].areaCode;
	nInfo.NorthEast.height = m_Map.m_Tiles[yPosPOne][xPosPOne].pathSubTiles[ySubPOne][xSubPOne].height;
	nInfo.NorthEast.cost = m_Map.m_Tiles[yPosPOne][xPosPOne].pathSubTiles[ySubPOne][xSubPOne].cost;
	nInfo.SouthWest.owner = m_Map.m_Tiles[yPosMOne][xPosMOne].owner;
	nInfo.SouthWest.type = m_Map.m_Tiles[yPosMOne][xPosMOne].type;
	nInfo.SouthWest.areaCode = m_Map.m_Tiles[yPosMOne][xPosMOne].areaCode;
	nInfo.SouthWest.height = m_Map.m_Tiles[yPosMOne][xPosMOne].pathSubTiles[ySubMOne][xSubMOne].height;
	nInfo.SouthWest.cost = m_Map.m_Tiles[yPosMOne][xPosMOne].pathSubTiles[ySubMOne][xSubMOne].cost;
	nInfo.South.owner = m_Map.m_Tiles[yPosMOne][xPos].owner;
	nInfo.South.type = m_Map.m_Tiles[yPosMOne][xPos].type;
	nInfo.South.areaCode = m_Map.m_Tiles[yPosMOne][xPos].areaCode;
	nInfo.South.height = m_Map.m_Tiles[yPosMOne][xPos].pathSubTiles[ySubMOne][xSub].height;
	nInfo.South.cost = m_Map.m_Tiles[yPosMOne][xPos].pathSubTiles[ySubMOne][xSub].cost;
	nInfo.SouthEast.owner = m_Map.m_Tiles[yPosMOne][xPosPOne].owner;
	nInfo.SouthEast.type = m_Map.m_Tiles[yPosMOne][xPosPOne].type;
	nInfo.SouthEast.areaCode = m_Map.m_Tiles[yPosMOne][xPosPOne].areaCode;
	nInfo.SouthEast.height = m_Map.m_Tiles[yPosMOne][xPosPOne].pathSubTiles[ySubMOne][xSubPOne].height;
	nInfo.SouthEast.cost = m_Map.m_Tiles[yPosMOne][xPosPOne].pathSubTiles[ySubMOne][xSubPOne].cost;
	nInfo.East.owner = m_Map.m_Tiles[yPos][xPosPOne].owner;
	nInfo.East.type = m_Map.m_Tiles[yPos][xPosPOne].type;
	nInfo.East.areaCode = m_Map.m_Tiles[yPos][xPosPOne].areaCode;
	nInfo.East.height = m_Map.m_Tiles[yPos][xPosPOne].pathSubTiles[ySub][xSubPOne].height;
	nInfo.East.cost = m_Map.m_Tiles[yPos][xPosPOne].pathSubTiles[ySub][xSubPOne].cost;
	nInfo.West.owner = m_Map.m_Tiles[yPos][xPosMOne].owner;
	nInfo.West.type = m_Map.m_Tiles[yPos][xPosMOne].type;
	nInfo.West.areaCode = m_Map.m_Tiles[yPos][xPosMOne].areaCode;
	nInfo.West.height = m_Map.m_Tiles[yPos][xPosMOne].pathSubTiles[ySub][xSubMOne].height;
	nInfo.West.cost = m_Map.m_Tiles[yPos][xPosMOne].pathSubTiles[ySub][xSubMOne].cost;
	return nInfo;
}

uint8_t Themp::LevelData::GetSubtileHeight(int tileY, int tileX, int subTileY, int subTileX)
{
	return m_Map.m_Tiles[tileY][tileX].pathSubTiles[subTileY][subTileX].height;
}

uint16_t LevelData::HandleSpecialCases(int yPos, int xPos)
{
	uint16_t type = m_Map.m_Tiles[yPos][xPos].type & 0xFF;
	if (type != Type_Dungeon_Heart && type != Type_Portal) return type;
	if (yPos >= 1 && yPos < MAP_SIZE_TILES && xPos >= 1 && xPos < MAP_SIZE_TILES)
	{
		if (L8(m_Map.m_Tiles[yPos - 1][xPos].type) != type && L8(m_Map.m_Tiles[yPos + 1][xPos].type) == type) // U
		{
			if (L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos + 1].type) != type) // LU
			{
				return type + (3 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type) // MU
			{
				return type + (2 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) != type) // RU
			{
				return type + (1 << 8);
			}
		}
		if (L8(m_Map.m_Tiles[yPos + 1][xPos].type) != type && L8(m_Map.m_Tiles[yPos - 1][xPos].type) == type) // L
		{
			if (L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos + 1].type) != type) // LL
			{
				return type + (9 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type) // ML
			{
				return type + (8 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) != type) // RL
			{
				return type + (7 << 8);
			}
		}
		if (L8(m_Map.m_Tiles[yPos + 1][xPos].type) == type && L8(m_Map.m_Tiles[yPos - 1][xPos].type) == type) // M
		{
			if (L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos + 1].type) != type) // LM
			{
				return type + (6 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) == type) // MM
			{
				if (L8(m_Map.m_Tiles[yPos + 1][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos - 1][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos + 1][xPos - 1].type) == type && L8(m_Map.m_Tiles[yPos - 1][xPos - 1].type) == type)
				{
					return type + (10 << 8);
				}
				return type + (5 << 8);
			}
			if (L8(m_Map.m_Tiles[yPos][xPos + 1].type) == type && L8(m_Map.m_Tiles[yPos][xPos - 1].type) != type) // RM
			{
				return type + (4 << 8);
			}
		}
	}
	return type + (5 << 8); //Middle tile
}

void LevelData::UpdateWalls(int y, int x)
{
	uint16_t type = m_Map.m_Tiles[y][x].type;
	TileNeighbours neighbour = CheckNeighbours(type, y, x);

	bool northWalkable = m_Map.m_Tiles[y + 1][x].type > Type_Wall5;
	bool southWalkable = m_Map.m_Tiles[y - 1][x].type > Type_Wall5;
	bool eastWalkable = m_Map.m_Tiles[y][x + 1].type > Type_Wall5;
	bool westWalkable = m_Map.m_Tiles[y][x - 1].type > Type_Wall5;
	uint8_t numWalkable = northWalkable + southWalkable + eastWalkable + westWalkable;

	if (type == Type_Wall3)
	{
		if (northWalkable && southWalkable || eastWalkable && westWalkable)
		{
			m_Map.m_Tiles[y][x].type = Type_Wall0;
			m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
		}
		else if(numWalkable >= 1)
		{
			m_Map.m_Tiles[y][x].type = Type_Wall2;
			m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
		}
	}
	else if (type == Type_Wall2)
	{
		if (northWalkable && southWalkable || eastWalkable && westWalkable)
		{
			m_Map.m_Tiles[y][x].type = Type_Wall0;
			m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
		}
	}
}
bool Themp::LevelData::MineTile(int y, int x)
{
	if((m_Map.m_Tiles[y][x].type & 0xFF) != Type_Gem)
		m_Map.m_Tiles[y][x].health -= LevelConfig::gameSettings[GameSettings::GAME_DEFAULT_IMP_DIG_DAMAGE].Value;
	if (m_Map.m_Tiles[y][x].health <= 0)
	{
		DestroyTile(y, x);
		return true;
	}
	else
	{
		const static std::string miningSounds[6] =
		{
			"DIG1.WAV",
			"DIG2.WAV",
			"DIG3.WAV",
			"DIG4.WAV",
			"DIG5.WAV",
			"DIG6.WAV",
		};
		System::tSys->m_Audio->PlayOneShot(FileManager::GetSound(miningSounds[rand() % 6]));
	}
	return false;
}
bool Themp::LevelData::ReinforceTile(uint8_t owner, int y, int x)
{
	m_Map.m_Tiles[y][x].health -= LevelConfig::gameSettings[GameSettings::GAME_DEFAULT_IMP_DIG_DAMAGE].Value;
	if (m_Map.m_Tiles[y][x].health <= 0)
	{
		ClaimTile(owner, y, x);
		return true;
	}
	return false;
}
void LevelData::DestroyTile(int y, int x)
{
	uint16_t type = m_Map.m_Tiles[y][x].type;
	if (type == Type_Earth || type == Type_Earth_Torch || type >= Type_Wall0 && type <= Type_Wall5 || type == Type_Gold)
	{
		m_Map.m_Tiles[y][x].type = Type_Unclaimed_Path;
		m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROCK].Value;
		m_Map.m_Tiles[y][x].owner = Owner_PlayerNone;
		m_Map.m_Tiles[y][x].marked[0] = false;
		m_Map.m_Tiles[y][x].marked[1] = false;
		m_Map.m_Tiles[y][x].marked[2] = false;
		m_Map.m_Tiles[y][x].marked[3] = false;

		//first we loop to update all the walls
		for (int sy = -1; sy <= 1; sy++)
		{
			for (int sx = -1; sx <= 1; sx++)
			{
				UpdateWalls(y + sy, x + sx);
			}
		}

		TileNeighbourTiles neighbours = GetNeighbourTiles(y, x);
		uint32_t lowestAreaCode = NextAreaCode;
		for (int i = 0; i < 4; i++)
		{
			if (neighbours.Axii[i].areaCode > 0 && (neighbours.Axii[i].type & 0xFF) != Type_Lava)
			{
				if (neighbours.Axii[i].areaCode < lowestAreaCode)
				{
					lowestAreaCode = neighbours.Axii[i].areaCode;
				}
			}
		}
		if (lowestAreaCode == NextAreaCode)
		{
			lowestAreaCode = NextAreaCode;
			NextAreaCode++;
		}
		UpdateAreaCode(lowestAreaCode, Type_Unclaimed_Path, y, x);

		//then update the area surrounding this
		UpdateArea(y - 2, y + 2, x - 2, x + 2);

		const static std::string rockSounds[3] =
		{
			"ROCKS1.WAV",
			"ROCKS2.WAV",
			"ROCKS3.WAV",
		};
		System::tSys->m_Audio->PlayOneShot(FileManager::GetSound(rockSounds[rand() % 3]));
		LevelData::PathsInvalidated = true;
	}
}

bool LevelData::MarkTile(uint8_t player, int y, int x)
{
	if (m_Map.m_Tiles[y][x].owner == Owner_PlayerRed || m_Map.m_Tiles[y][x].owner == Owner_PlayerNone)
	{
		uint16_t type = m_Map.m_Tiles[y][x].type & 0xFF;
		if (type >= Type_Gold && type <= Type_Wall5 || type == Type_Gem) //If the tile is a full block (mineable)
		{
			if (m_Map.m_Tiles[y][x].marked[player])return true;
			m_Map.m_Tiles[y][x].marked[player] = true;

			TileNeighbours nb = CheckNeighbours(type, y, x);
			if (nb.North == N_WALKABLE || nb.North == N_WATER || nb.East == N_WALKABLE || nb.East == N_WATER || nb.South == N_WALKABLE || nb.South == N_WATER || nb.West == N_WALKABLE || nb.West == N_WATER)
			{
				CreatureTaskManager::AddMiningTask(player, XMINT2(x, y),&m_Map.m_Tiles[y][x]);
			}
			System::tSys->m_Audio->PlayOneShot(FileManager::GetSound("DIGMARK.WAV"));
			return true;
		}
	}
	return false;
}
void LevelData::UnMarkTile(uint8_t player, int y, int x)
{
	if (!m_Map.m_Tiles[y][x].marked[player]) return;
	m_Map.m_Tiles[y][x].marked[player] = false;
	CreatureTaskManager::RemoveMiningTask(player, &m_Map.m_Tiles[y][x]);
	System::tSys->m_Audio->PlayOneShot(FileManager::GetSound("DIGMARK.WAV"));
}
bool IsClaimableRoom(uint16_t type)
{
	return ((type >= Type_Portal && type <= Type_Barracks || type == Type_Bridge || type == Type_Guardpost) && type != Type_Dungeon_Heart);
}

//in Tile Positions
void LevelData::UpdateArea(int minY, int maxY, int minX, int maxX)
{
	RenderTile tileOut = {};
	const XMINT2 directions[4] = { XMINT2(0,1), XMINT2(0,-1), XMINT2(-1,0), XMINT2(1,0), };
	for (int y = minY; y <= maxY; y++)
	{
		for (int x = minX; x <= maxX; x++)
		{
			m_Map.m_Tiles[y][x].type = HandleSpecialCases(y, x);
			uint16_t numBlocks = CreateFromTile(m_Map.m_Tiles[y][x], tileOut);
			m_Map.m_Tiles[y][x].numBlocks = numBlocks;
			//Update area surrounding this
			for (size_t yy = 0; yy < 3; yy++)
			{
				for (size_t xx = 0; xx < 3; xx++)
				{
					assert(y * 3 + yy < 256 && y * 3 + yy >= 0);
					assert(x * 3 + xx < 256 && x * 3 + xx >= 0);
					for (size_t z = 0; z < MAP_SIZE_HEIGHT; z++)
					{
						Block& b = tileOut.subTile[yy][xx][z];
						m_Map.m_Tiles[y][x].pathSubTiles[yy][xx] = tileOut.pathSubTiles[yy][xx];
						b.randValue = m_BlockMap[z][y * 3 + yy][x * 3 + xx].randValue;
						m_BlockMap[z][y * 3 + yy][x * 3 + xx] = b;
					}
				}
			}
			uint16_t currentTileType = m_Map.m_Tiles[y][x].type & 0xFF;
			DoUVs(currentTileType, y, x);

			TileNeighbourTiles neighbourTiles = GetNeighbourTiles(y, x);
			if (currentTileType == Type_Unclaimed_Path)
			{
				if (m_Map.m_Tiles[y][x].health <= 0.0f)
				{
					m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROCK].Value;
				}
				for (int i = 0; i < 4; i++)
				{
					if(neighbourTiles.Axii[i].owner != Owner_PlayerWhite && neighbourTiles.Axii[i].owner != Owner_PlayerNone && ( (neighbourTiles.Axii[i].type & 0xFF) == Type_Claimed_Land || IsClaimableRoom(neighbourTiles.Axii[i].type & 0xFF)))
					{
						CreatureTaskManager::AddClaimingTask(neighbourTiles.Axii[i].owner, XMINT2(x, y), &m_Map.m_Tiles[y][x]);
					}
				}
			}
			else if (currentTileType == Type_Claimed_Land)
			{
				if (m_Map.m_Tiles[y][x].health <= 0.0f)
				{
					m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				}
				for (int i = 0; i < 4; i++)
				{
					if (neighbourTiles.Axii[i].owner != m_Map.m_Tiles[y][x].owner && neighbourTiles.Axii[i].owner != Owner_PlayerWhite && neighbourTiles.Axii[i].owner != Owner_PlayerNone && ((neighbourTiles.Axii[i].type & 0xFF) == Type_Claimed_Land || IsClaimableRoom(neighbourTiles.Axii[i].type & 0xFF)))
					{
						CreatureTaskManager::AddClaimingTask(neighbourTiles.Axii[i].owner, XMINT2(x, y), &m_Map.m_Tiles[y][x]);
					}
				}
			}
			else if (IsClaimableRoom(currentTileType))
			{
				if (m_Map.m_Tiles[y][x].health <= 0.0f)
				{
					m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROOM].Value;
				}
				for (int i = 0; i < 4; i++)
				{
					if (neighbourTiles.Axii[i].owner != m_Map.m_Tiles[y][x].owner && neighbourTiles.Axii[i].owner != Owner_PlayerWhite && neighbourTiles.Axii[i].owner != Owner_PlayerNone && ((neighbourTiles.Axii[i].type & 0xFF) == Type_Claimed_Land || IsClaimableRoom(neighbourTiles.Axii[i].type & 0xFF)))
					{
						CreatureTaskManager::AddClaimingTask(neighbourTiles.Axii[i].owner, XMINT2(x, y), &m_Map.m_Tiles[y][x]);
					}
				}
			}
			else if (currentTileType == Type_Earth || currentTileType == Type_Earth_Torch)
			{
				if (m_Map.m_Tiles[y][x].health <= 0.0f)
				{
					m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROCK].Value;
				}
				bool* markers = m_Map.m_Tiles[y][x].marked;
				for (int player = 0; player < 4; player++)
				{
					for (int i = 0; i < 4; i++)
					{
						bool* bnMarkers = m_Map.m_Tiles[y + directions[i].y][x + directions[i].x].marked;
					
						if (bnMarkers[player])
							CreatureTaskManager::AddMiningTask(player, XMINT2(x + directions[i].x, y + directions[i].y), &m_Map.m_Tiles[y][x]);

						if (!markers[player])
						{
							if (neighbourTiles.Axii[i].owner == player && ((neighbourTiles.Axii[i].type & 0xFF) == Type_Claimed_Land || IsClaimableRoom(neighbourTiles.Axii[i].type & 0xFF)))
							{
								CreatureTaskManager::AddReinforcingTask(player, XMINT2(x, y), &m_Map.m_Tiles[y][x]);
							}
						}
					}
				}
			}
			else if (currentTileType == Type_Gold || currentTileType == Type_Gem)
			{
				if (m_Map.m_Tiles[y][x].health <= 0.0f)
				{
					m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_GOLD].Value;
				}
				for (int i = 0; i < 4; i++)
				{
					bool* markers = m_Map.m_Tiles[y + directions[i].y][x + directions[i].x].marked;
					for (int player = 0; player < 4; player++)
					{
						if (markers[player])
							CreatureTaskManager::AddMiningTask(player, XMINT2(x + directions[i].x, y + directions[i].y), &m_Map.m_Tiles[y][x]);
					}
				}
			}
		}
	}
}
bool LevelData::HasLandNeighbour(const int pos[4], int position, int y, int x)
{
	//for (size_t i = 0; i < 4; i++)
	{
		//if (abs(pos[i] - position) < 2)
		{
			TileNeighbourTiles n = GetNeighbourTiles(y, x);
			for (size_t j = 0; j < 4; j++)
			{
				if (n.Axii[j].type == Type_Claimed_Land || n.Axii[j].type == Type_Unclaimed_Path)
				{
					return true;
				}
			}
		}
	}
	return false;
}
void LevelData::UpdateAreaCode(uint32_t newCode, uint16_t currentType, int ty, int tx)
{
	//if this tile is already the same code, return
	if (newCode == m_Map.m_Tiles[ty][tx].areaCode) return;

	//tile is of a space seperating type, return
	uint16_t nType = m_Map.m_Tiles[ty][tx].type & 0xFF;
	if (nType >= Type_Rock && nType <= Type_Wall5 || nType == Type_Lava || nType == Type_Gem) return;

	m_Map.m_Tiles[ty][tx].areaCode = newCode;

	UpdateAreaCode(newCode, currentType, ty - 1, tx);
	UpdateAreaCode(newCode, currentType, ty + 1, tx);
	UpdateAreaCode(newCode, currentType, ty, tx - 1);
	UpdateAreaCode(newCode, currentType, ty, tx + 1);
}
bool LevelData::CollectiveClaimRoom(uint16_t type, int ty, int tx)
{
	//tile is of a different type, return
	uint16_t nType = m_Map.m_Tiles[ty][tx].type & 0xFF;
	if (type != nType) return false;

	//if this tile has less than or equal to 0 health we return true 
	m_Map.m_Tiles[ty][tx].health -= LevelConfig::gameSettings[GameSettings::GAME_DEFAULT_IMP_DIG_DAMAGE].Value;
	if (m_Map.m_Tiles[ty][tx].health <= 0)
		return true;

	//if our neighbours return true it means they have all depleted health
	if (!CollectiveClaimRoom(type, ty - 1, tx)) return false;
	if (!CollectiveClaimRoom(type, ty + 1, tx)) return false;
	if (!CollectiveClaimRoom(type, ty, tx - 1)) return false;
	if (!CollectiveClaimRoom(type, ty, tx + 1)) return false;

	//we can recurse the stack back giving true as the final end result, meaning all tiles ran out of health (and thus we claimed the room)
	return true;
}
void LevelData::ClaimRoom(uint8_t newOwner, uint16_t type, int ty, int tx)
{
	//if this tile is already ours, return
	if (newOwner == m_Map.m_Tiles[ty][tx].owner) return;

	//tile is of a different type, return
	uint16_t nType = m_Map.m_Tiles[ty][tx].type & 0xFF;
	if (type != nType) return;

	CreatureTaskManager::RemoveClaimingTask(newOwner, &m_Map.m_Tiles[ty][tx]);
	m_Map.m_Tiles[ty][tx].owner = newOwner;
	m_Map.m_Tiles[ty][tx].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROOM].Value;
	ClaimRoom(newOwner, type, ty - 1, tx);
	ClaimRoom(newOwner, type, ty + 1, tx);
	ClaimRoom(newOwner, type, ty, tx - 1);
	ClaimRoom(newOwner, type, ty, tx + 1);

	UpdateArea(ty - 1, ty + 1, tx - 1, tx + 1);
}
bool LevelData::IsClaimableCorner(int y, int x)
{
	TileNeighbourTiles n = GetNeighbourTiles(y, x);
	if ((   n.SouthEast.type == Type_Claimed_Land || IsOwnedRoom(n.SouthEast.owner,y-1,x+1)) && (IsWall(n.South.type) && IsWall(n.East.type))
		|| (n.SouthWest.type == Type_Claimed_Land || IsOwnedRoom(n.SouthWest.owner, y - 1, x - 1)) && (IsWall(n.South.type) && IsWall(n.West.type))
		|| (n.NorthEast.type == Type_Claimed_Land || IsOwnedRoom(n.NorthEast.owner, y + 1, x + 1)) && (IsWall(n.North.type) && IsWall(n.East.type))
		|| (n.NorthWest.type == Type_Claimed_Land || IsOwnedRoom(n.NorthWest.owner, y + 1, x - 1)) && (IsWall(n.North.type) && IsWall(n.West.type)))
	{
		return true;
	}
	return false;
}

bool LevelData::IsOwnedRoom(uint8_t player, int y, int x)
{
	uint16_t type = m_Map.m_Tiles[y][x].type & 0xFF;
	uint8_t owner = m_Map.m_Tiles[y][x].owner;

	bool isRoom = type >= Type_Portal && type <= Type_Barracks || type == Type_Bridge || type == Type_Guardpost || type == Type_Dungeon_Heart;
	
	return (isRoom && owner == player);
}
//This function should never be able to be called on y/x == 0 or y/x == MAP_SIZE_TILES
void LevelData::ClaimTile(uint8_t player, int y, int x)
{
	assert(y > 0 && y < MAP_SIZE_TILES);
	assert(x > 0 && x < MAP_SIZE_TILES);
	uint16_t type = m_Map.m_Tiles[y][x].type & 0xFF;
	TileNeighbourTiles neighbourTiles = GetNeighbourTiles(y, x);

	if ((neighbourTiles.North.type == Type_Claimed_Land && neighbourTiles.North.owner == player) || IsOwnedRoom(player,y+1,x)
		|| (neighbourTiles.South.type == Type_Claimed_Land && neighbourTiles.South.owner == player) || IsOwnedRoom(player, y - 1, x)
		|| (neighbourTiles.East.type == Type_Claimed_Land && neighbourTiles.East.owner == player) || IsOwnedRoom(player, y, x + 1)
		|| (neighbourTiles.West.type == Type_Claimed_Land && neighbourTiles.West.owner == player) || IsOwnedRoom(player, y, x - 1))
	{
		if (type == Type_Unclaimed_Path)
		{
			m_Map.m_Tiles[y][x].type = Type_Claimed_Land;
			m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
			m_Map.m_Tiles[y][x].owner = player;
			UpdateArea(y - 1, y + 1, x - 1, x + 1);

			System::tSys->m_Audio->PlayOneShot(FileManager::GetSound("STARS3.WAV")); 
		}
		else if (type == Type_Earth || type == Type_Earth_Torch)
		{
			m_Map.m_Tiles[y][x].type = Type_Wall2;
			m_Map.m_Tiles[y][x].owner = player;
			m_Map.m_Tiles[y][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
			TileNeighbours neighbour = CheckNeighbours(Type_Wall2, y, x);

			int xPos[4] = { -100 }; //some tile far enough to never have a neigbour
			int yPos[4] = { -100 }; //some tile far enough to never have a neigbour
			if (neighbourTiles.North.owner == player && neighbour.North == N_WALKABLE)
			{
				yPos[0] = y + 1;
				xPos[0] = x;
			}
			if (neighbourTiles.South.owner == player && neighbour.South == N_WALKABLE)
			{
				yPos[2] = y - 1;
				xPos[2] = x;
			}
			if (neighbourTiles.East.owner == player && neighbour.East == N_WALKABLE)
			{
				yPos[1] = y;
				xPos[1] = x + 1;
			}
			if (neighbourTiles.West.owner == player && neighbour.West == N_WALKABLE)
			{
				yPos[3] = y;
				xPos[3] = x - 1;
			}
			if ((neighbourTiles.North.type == Type_Earth || neighbourTiles.North.type == Type_Earth_Torch) && (neighbour.NorthEast == N_SAME || neighbour.NorthWest == N_SAME))
			{
				if (IsClaimableCorner(y+1,x))
				{
					m_Map.m_Tiles[y + 1][x].type = Type_Wall3;
					m_Map.m_Tiles[y + 1][x].owner = player;
					m_Map.m_Tiles[y + 1][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				}
			}
			if ((neighbourTiles.South.type == Type_Earth || neighbourTiles.South.type == Type_Earth_Torch) && (neighbour.SouthEast == N_SAME || neighbour.SouthWest == N_SAME))
			{
				if (IsClaimableCorner(y - 1, x))
				{
					m_Map.m_Tiles[y - 1][x].type = Type_Wall3;
					m_Map.m_Tiles[y - 1][x].owner = player;
					m_Map.m_Tiles[y - 1][x].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				}
			}
			if ((neighbourTiles.East.type == Type_Earth || neighbourTiles.East.type == Type_Earth_Torch) && (neighbour.NorthEast == N_SAME || neighbour.SouthEast == N_SAME))
			{
				if (IsClaimableCorner(y, x + 1))
				{
					m_Map.m_Tiles[y][x + 1].type = Type_Wall3;
					m_Map.m_Tiles[y][x + 1].owner = player;
					m_Map.m_Tiles[y][x + 1].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				}
			}
			if ((neighbourTiles.West.type == Type_Earth || neighbourTiles.West.type == Type_Earth_Torch) && (neighbour.NorthWest == N_SAME || neighbour.SouthWest == N_SAME))
			{
				if (IsClaimableCorner(y, x - 1))
				{
					m_Map.m_Tiles[y][x - 1].type = Type_Wall3;
					m_Map.m_Tiles[y][x - 1].owner = player;
					m_Map.m_Tiles[y][x - 1].health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				}
			}
			UpdateWalls(y, x);

			UpdateArea(y - 1, y + 1, x - 1, x + 1);

			System::tSys->m_Audio->PlayOneShot(FileManager::GetSound("STARS3.WAV"));
		}
		else if (IsClaimableRoom(type))
		{
			System::tSys->m_Audio->PlayOneShot(FileManager::GetSound("TAKEOVER.WAV"));
			ClaimRoom(player, type, y, x);
		}
	}
}

XMINT2 LevelData::WorldToTile(XMFLOAT3 pos)
{
	return XMINT2(((int)pos.x) / 3, ((int)pos.z) / 3);
}
XMFLOAT3 LevelData::TileToWorld(XMINT2 tPos)
{
	return XMFLOAT3((tPos.x) * 3 + 1, 3, (tPos.y) * 3 +1);
}	
XMFLOAT3 LevelData::WorldToSubtileFloat(XMFLOAT3 pos)
{
	return XMFLOAT3(pos.x, pos.y, pos.z);
}
XMINT3 LevelData::WorldToSubtile(XMFLOAT3 pos)
{
	return XMINT3(pos.x, pos.y, pos.z);
}
XMFLOAT3 LevelData::SubtileToWorld(XMINT3 pos)
{
	return XMFLOAT3(pos.x, pos.y, pos.z);
}

void LevelData::LoadLevelFileData()
{
	//"MAP00001"
	std::wstring LevelPath = L"LEVELS\\MAP0";

	//local scope so we don't waste the variable names
	{
		wchar_t buf[32]; //should only ever have 4 or less chars, but safety first..
		memset(buf, 0, 32);
		_itow(m_CurrentLevelNr, buf, 10); //Though I guess I shouldn't be using such a unsafe function then heh...
		std::wstring levelNr = L"";

		//Now we add the pre-trailing 0's since itoa 1 == "1" and we need "0001" or 10 being "0010"
		if (m_CurrentLevelNr < 10)
		{
			levelNr.append(3, L'0'); //000
		}
		else if (m_CurrentLevelNr < 100)
		{
			levelNr.append(2, L'0'); //00
		}
		else if (m_CurrentLevelNr < 1000)
		{
			levelNr.append(1, L'0'); //0
		}
		levelNr.append(buf); //append the resulting number to the zero's resulting in XXXN (X being a 0 or number)
		LevelPath.append(levelNr); //append it to the complete path "LEVELS\\MAP0NNNN" 
	}

	//All these files are needed to completely load a single level, so I guess we'll just grab em already.
	//Source: http://keeper.lubiki.pl/dk1_docs/dk_mapfiles_ref.htm


	//Description: Gives location of action points for reference in script (.txt) files.
	//Format:
	//4 bytes : Number of 8 byte entries.
	//Each entry :
	//	4 bytes : XY Location
	//	2 bytes : Range
	//	2 bytes : Action point number(points are usually in order, but they don't have to be.
	FileData map_apt = FileManager::GetFileData(LevelPath + L".apt");

	//Description:Along with the.dat file, this determines the graphics for the level.Each entry gives the data for a subtile.
	//Format :
	//8 bytes : Number of 24 byte entries(always 2048).
	//Each entry :
	//24 bytes :
	//	0 - 1 : Use - Counter of the column uses for non - permanent columns; For permanent colums, this value has an offset.
	//	2: Byte with bitfields :
	//Bit 0 : Permanent - if set, the column is never overwritten
	//	Bits 1 - 3 : Lintel(unknown)
	//	Bits 4 - 7 : Height - in subtiles; proper values are 0..7
	//	3 - 4: Solidmask - appears to be a bitmask for which of the cubes are non - zero, with the lowest cube being represented by the least significant bit.
	//	5 - 6 : Baseblock - determines the very lowest level(below the lowest cube).I think the block numbers here are different to the ones used by the cubes, but I'm not sure yet. This entry is mostly useful for water, lava, and hatcheries.
	//	7 : Orientation(always 0)
	//	8 - 23 : Cubes - there are 8 cubes, each having a 2 byte entry.The first cube usually forms the floor, etc, up to the ceiling.
	FileData map_clm = FileManager::GetFileData(LevelPath + L".clm");

	//Description: Determines the graphics at the start of the game.When new areas are dug out or rooms are changed, computer automatically generates different graphics for these areas, but otherwise everything is in this file.
	//Format : Subtile map format, with 2 bytes per entry.
	FileData map_dat = FileManager::GetFileData(LevelPath + L".dat");

	//Description: Specifies the landscape style, or in other words, texture file number(making the dungeon icy, for instance).
	//Format : Always 1 byte, which is a 0 for all but the following maps :
	//	 01 for maps 13, 14, 15, 16, 17, 104
	//	 02 for maps 9, 10
	//	 03 for maps 19, 20
	FileData map_inf = FileManager::GetFileData(LevelPath + L".inf");

	//Description: Provides extra lighting.
	//Format :
	//4 bytes : Number of 20 byte entries.
	//Each entry :
	//	 2 bytes : Range
	//	 1 byte : Light intensivity
	//	 7 bytes : unknown
	//	 6 bytes : XYZ Location
	//	 2 bytes : unknown.
	//	 2 bytes : always - 1 (0xffff).
	FileData map_lgt = FileManager::GetFileData(LevelPath + L".lgt");

	//Description: Determine ownership of tiles.
	//Notes: in subtile format. (each tile = 3x3 subtiles)
	FileData map_own = FileManager::GetFileData(LevelPath + L".own");

	//Description: Determines type of "room" each tile is.
	//Format: 85 lines of 85 tiles, each having a two byte entry.Each entry only uses the first byte, the second byte being 0.
	FileData map_slb = FileManager::GetFileData(LevelPath + L".slb");

	//Description: List of "things" in the dungeon.
	//Format:
	//2 bytes : Number of 21 byte entries
	//Each entry :
	//	0 - 5 : Location
	//	6 : Thing type
	//	7 : Thing subtype(eg creature race, trap type)
	//	8 : Owner
	//	9 - 20 : Type specific data
	FileData map_tng = FileManager::GetFileData(LevelPath + L".tng");

	//Description: Level script which determines victory conditions, enemy keepers actions, training mode, etc.
	//Format: Text file.
	//This is a pretty simple scripting language. There are no loops or jumps, and condition testing is very basic. There are only three statement forms - conditions, commands, and condition terminators. Conditions are naturally nested, but that's about as complicated as it gets.
	//Each line contains a single instruction.Whitespace in the form of spaces and tabs is ignored.Comments begin with REM.All unconditional commands(ie ones outside any condition statements) are executed at the beginning of the game, then ignored.All conditions are checked at regular intervals.
	FileData map_txt = FileManager::GetFileData(LevelPath + L".txt");

	//Description: Determines which subtiles are animated.
	//Format : Subtile map with single byte entries.Each byte is 00, 01, or 02 :
	//00 : Display the subtile without any change
	//01 : Skew the subtile slightly(default)
	//02 : Animate the subtile
	FileData map_wib = FileManager::GetFileData(LevelPath + L".wib");

	//Description: Determines type of liquid each tile is. (Water/Lava)
	//Format: 85 lines of 85 tiles, each having one byte entry.
	FileData map_wlb = FileManager::GetFileData(LevelPath + L".wlb");


	int slb_index = 0;
	for (int y = MAP_SIZE_TILES - 1; y >= 0; y--)
	{
		for (int x = 0; x < MAP_SIZE_TILES; x++)
		{
			BYTE tile = map_slb.data[slb_index++];
			slb_index++; //second byte is always 0 so we skip it

			Tile& mapTile = m_Map.m_Tiles[y][x];
			if (IsWall(tile))
			{
				tile = Type_Wall0;
			}
			else if (IsClaimableRoom(tile))
			{
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROOM].Value;
			}
			switch (tile)
			{
			case Type_Wall0:
			case Type_Wall1:
			case Type_Wall2:
			case Type_Wall3:
			case Type_Wall4:
			case Type_Wall5:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				break;
			case Type_Earth:
			case Type_Earth_Torch:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_ROCK].Value;
				break;
			case Type_Gold:
			case Type_Gem:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_GOLD].Value;
				break;
			case Type_Wooden_DoorH:
			case Type_Wooden_DoorV:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_DOOR1].Value;
				break;
			case Type_Braced_DoorH:
			case Type_Braced_DoorV:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_DOOR2].Value;
				break;
			case Type_Iron_DoorH:
			case Type_Iron_DoorV:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_DOOR3].Value;
				break;
			case Type_Magic_DoorH:
			case Type_Magic_DoorV:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_DOOR4].Value;
				break;
			case Type_Unclaimed_Path:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_FLOOR].Value;
				break; 
			case Type_Claimed_Land:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				break;
			default:
				mapTile.health = LevelConfig::blockHealth[BlockHealth::BLOCK_HEALTH_PRETTY].Value;
				break;
			}

			mapTile.type = tile;
			mapTile.areaCode = 0;
			mapTile.marked[0] = false;
			mapTile.marked[1] = false;
			mapTile.marked[2] = false;
			mapTile.marked[3] = false;
		}
	}
	Tile m_AdjustedMap[85][85];
	memcpy(m_AdjustedMap, m_Map.m_Tiles, sizeof(m_Map.m_Tiles));

	int own_index = 0; //skip the first rown of subtiles
	int yIndex = 0;
	int xIndex = 0;
	for (int y = MAP_SIZE_SUBTILES - 1; y >= 0;)
	{
		if (y == 255)
		{
			//skip this line of subtiles
			y--;
			continue;
		}
		for (int x = 0; x < MAP_SIZE_SUBTILES;)
		{
			if (x == 255)
			{
				//skip this subtile
				own_index++;
				x++;
				continue;
			}
			assert(own_index < map_own.size);
			uint8_t owner = map_own.data[own_index];
			m_AdjustedMap[yIndex][xIndex].owner = owner;
			own_index += 3; //skip a whole tile
			x += 3;
			xIndex++;
		}
		y -= 3;
		own_index = MAP_SIZE_SUBTILES * y;
		xIndex = 0;
		yIndex++;
	}

	for (int y = 0; y < MAP_SIZE_TILES; y++)
	{
		for (int x = 0; x < MAP_SIZE_TILES; x++)
		{
			Tile& mapTile = m_Map.m_Tiles[y][x];

			//Walls NEVER occur along map borders, so checking x-1 or x+1 or y-1 or y+1 without boundary checks is fine
			if (mapTile.type == Type_Wall0)
			{
				m_AdjustedMap[y][x].type = Type_Wall0;
				bool northWall = m_Map.m_Tiles[y + 1][x].type == Type_Wall0;
				bool southWall = m_Map.m_Tiles[y - 1][x].type == Type_Wall0;
				bool westWall = m_Map.m_Tiles[y][x - 1].type == Type_Wall0;
				bool eastWall = m_Map.m_Tiles[y][x + 1].type == Type_Wall0;
				bool northRock = m_Map.m_Tiles[y + 1][x].type == Type_Rock || m_Map.m_Tiles[y + 1][x].type == Type_Earth;
				bool southRock = m_Map.m_Tiles[y - 1][x].type == Type_Rock || m_Map.m_Tiles[y - 1][x].type == Type_Earth;
				bool westRock = m_Map.m_Tiles[y][x - 1].type == Type_Rock || m_Map.m_Tiles[y][x - 1].type == Type_Earth;
				bool eastRock = m_Map.m_Tiles[y][x + 1].type == Type_Rock || m_Map.m_Tiles[y][x + 1].type == Type_Earth;

				if (southWall)
				{//southern piece is a walltype
					if (eastWall)
					{//eastern piece is a walltype
						if (northRock && westRock)
						{ //both other sides are not walls
							m_AdjustedMap[y][x].type = Type_Wall3; //This is a corner piece
						}
						else if (!northWall || !westWall)
						{//only 1 side is not a wall
							m_AdjustedMap[y][x].type = Type_Wall2; //this is a wall AND a cornerpiece
						}
						else
						{//otherwise this is a wall
							m_AdjustedMap[y][x].type = Type_Wall0;
						}
					}
					else if (westWall)
					{//Western piece is a walltype
						if (northRock && eastRock)
						{ //both other sides are not walls
							m_AdjustedMap[y][x].type = Type_Wall3; //This is a corner piece
						}
						else if (!northWall || !eastWall)
						{//only 1 northRock is not a wall
							m_AdjustedMap[y][x].type = Type_Wall2; //this is a wall AND a cornerpiece
						}
						else
						{//otherwise this is a wall
							m_AdjustedMap[y][x].type = Type_Wall0;
						}
					}
				}
				if (northWall)
				{//northern piece is a walltype
					if (eastWall)
					{//eastern piece is a walltype
						if (southRock && westRock)
						{ //both other sides are not walls
							m_AdjustedMap[y][x].type = Type_Wall3; //This is a corner piece
						}
						else if (!southWall || !eastWall)
						{//only 1 side is not a wall
							m_AdjustedMap[y][x].type = Type_Wall2; //this is a wall AND a cornerpiece
						}
						else
						{//otherwise this is a wall
							m_AdjustedMap[y][x].type = Type_Wall0;
						}
					}
					else if (westWall)
					{//Western piece is a walltype
						if (southRock && eastRock)
						{ //both other sides are not walls
							m_AdjustedMap[y][x].type = Type_Wall3; //This is a corner piece
							continue;
						}
						else if (!southWall || !eastWall)
						{//only 1 side is not a wall
							m_AdjustedMap[y][x].type = Type_Wall2; //this is a wall AND a cornerpiece
							continue;
						}
						else
						{//otherwise this is a wall
							m_AdjustedMap[y][x].type = Type_Wall0;
							continue;

						}
					}
				}

			}
		}
	}

	memcpy(m_Map.m_Tiles, m_AdjustedMap, sizeof(m_Map.m_Tiles));
	memcpy(m_OriginalMap.m_Tiles, m_Map.m_Tiles, sizeof(m_Map.m_Tiles));
}


void LevelData::DoUVs(uint16_t type, int y, int x)
{
	const int yP = y * 3;
	const int xP = x * 3;
	TileNeighbours neighbour = CheckNeighbours(type, y, x);

	uint16_t texIndex = TypeToTexture(type);
	if (type == Type_Earth)
	{
		for (int sy = 0; sy < 3; sy++)
			for (int sx = 0; sx < 3; sx++)
			{
				const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndex].top[0];
				m_BlockMap[5][yP + sy][xP + sx].uv[2] = tex1[m_BlockMap[5][yP + sy][xP + sx].randValue % tex1.size()];
			}
	}
	else if (type == Type_Unclaimed_Path)
	{
		bool muddyPath = false;
		if (neighbour.North != N_SAME || neighbour.South != N_SAME)
		{
			if (neighbour.East != N_SAME || neighbour.West != N_SAME)
			{
				const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[1];
				muddyPath = true;
				for (int sy = 0; sy < 3; sy++)
					for (int sx = 0; sx < 3; sx++)
					{
						m_BlockMap[1][yP + sy][xP + sx].uv[2] = tex0[m_BlockMap[1][yP + sy][xP + sx].randValue % tex0.size()];
					}
			}
		}
		if (!muddyPath)
		{
			const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[0];
			for (int sy = 0; sy < 3; sy++)
				for (int sx = 0; sx < 3; sx++)
				{
					m_BlockMap[1][yP + sy][xP + sx].uv[2] = tex0[m_BlockMap[1][yP + sy][xP + sx].randValue % tex0.size()];
				}
		}
	}
	else if (type == Type_Claimed_Land)
	{
		const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[0];
		const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndex].top[1];
		if (neighbour.North == N_SAME)
		{
			m_BlockMap[1][yP + 2][xP + 1].uv[2] = tex1[m_BlockMap[1][yP + 2][xP + 1].randValue % tex1.size()];
			if (neighbour.East == N_SAME)
			{
				m_BlockMap[1][yP + 2][xP + 2].uv[2] = tex1[m_BlockMap[1][yP + 2][xP + 2].randValue % tex1.size()];
			}
			else
			{
				m_BlockMap[1][yP + 2][xP + 2].uv[2] = tex0[5];
			}
			if (neighbour.West == N_SAME)
			{
				m_BlockMap[1][yP + 2][xP].uv[2] = tex1[m_BlockMap[1][yP + 2][xP].randValue % tex1.size()];
			}
			else
			{
				m_BlockMap[1][yP + 2][xP].uv[2] = tex0[3];
			}
		}
		else
		{
			m_BlockMap[1][yP + 2][xP + 1].uv[2] = tex0[7];
			if (neighbour.East == N_SAME)
			{
				m_BlockMap[1][yP + 2][xP + 2].uv[2] = tex0[7];
			}
			else
			{
				m_BlockMap[1][yP + 2][xP + 2].uv[2] = tex0[8];
			}
			if (neighbour.West == N_SAME)
			{
				m_BlockMap[1][yP + 2][xP].uv[2] = tex0[7];
			}
			else
			{
				m_BlockMap[1][yP + 2][xP].uv[2] = tex0[6];
			}
		}
		if (neighbour.South == N_SAME)
		{
			m_BlockMap[1][yP][xP + 1].uv[2] = tex1[m_BlockMap[1][yP][xP + 1].randValue % tex1.size()];
			if (neighbour.East == N_SAME)
			{
				m_BlockMap[1][yP][xP + 2].uv[2] = tex1[m_BlockMap[1][yP][xP + 2].randValue % tex1.size()];
			}
			else
			{
				m_BlockMap[1][yP][xP + 2].uv[2] = tex0[5];
			}
			if (neighbour.West == N_SAME)
			{
				m_BlockMap[1][yP][xP].uv[2] = tex1[m_BlockMap[1][yP][xP].randValue % tex1.size()];
			}
			else
			{
				m_BlockMap[1][yP][xP].uv[2] = tex0[3];
			}
		}
		else
		{
			m_BlockMap[1][yP][xP + 1].uv[2] = tex0[1];
			if (neighbour.East == N_SAME)
			{
				m_BlockMap[1][yP][xP + 2].uv[2] = tex0[1];
			}
			else
			{
				m_BlockMap[1][yP][xP + 2].uv[2] = tex0[2];
			}
			if (neighbour.West == N_SAME)
			{
				m_BlockMap[1][yP][xP].uv[2] = tex0[1];
			}
			else
			{
				m_BlockMap[1][yP][xP].uv[2] = tex0[0];
			}
		}
		if (neighbour.East == N_SAME)
		{
			m_BlockMap[1][yP + 1][xP + 2].uv[2] = tex1[m_BlockMap[1][yP + 1][xP + 2].randValue % tex1.size()];
		}
		else
		{
			m_BlockMap[1][yP + 1][xP + 2].uv[2] = tex0[5];
		}
		if (neighbour.West == N_SAME)
		{
			m_BlockMap[1][yP + 1][xP].uv[2] = tex1[m_BlockMap[1][yP + 1][xP].randValue % tex1.size()];
		}
		else
		{
			m_BlockMap[1][yP + 1][xP].uv[2] = tex0[3];
		}
		m_BlockMap[1][yP + 1][xP + 1].uv[2] = t_FloorOwners[m_Map.m_Tiles[y][x].owner];
	}
	DoWallUVs(neighbour, type, texIndex, x, y);

	//debugging ownership
	//m_BlockMap[1][yP + 1][xP + 1].uv[2] = t_FloorOwners[m_Map.m_Tiles[y][x].owner];
	//m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_FloorOwners[m_Map.m_Tiles[y][x].owner];
}
void LevelData::DoWallUVs(TileNeighbours neighbour, int type, int texIndex, int x, int y)
{
	const int yP = y * 3;
	const int xP = x * 3;
	if (type == Type_Wall0)
	{
		const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[0];
		for (int sy = 0; sy < 3; sy++)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + 2][xP + sy].uv[2] = tex0[2];
			}
		}
		m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[m_Map.m_Tiles[y][x].owner];
		//for debugging
		//m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[Owner_PlayerWhite];
		if (neighbour.North != N_SAME && neighbour.North != N_DIFF)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP + 2][xP + sy].uv[2] = tex0[1];
			}
		}
		if (neighbour.South != N_SAME && neighbour.South != N_DIFF)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP][xP + sy].uv[2] = tex0[1];
			}
		}
		if (neighbour.East != N_SAME && neighbour.East != N_DIFF)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP + 2].uv[2] = tex0[1];
			}
		}
		if (neighbour.West != N_SAME && neighbour.West != N_DIFF)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP].uv[2] = tex0[1];
			}
		}
		if (neighbour.West == N_WALKABLE || neighbour.West == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x - 1].type);
			int randValue = m_BlockMap[7][yP][xP + 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.East == N_WALKABLE || neighbour.East == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x + 1].type);
			int randValue = m_BlockMap[7][yP][xP - 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP + 2].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.North == N_WALKABLE || neighbour.North == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y + 1][x].type);
			int randValue = m_BlockMap[7][yP + 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + 2][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.South == N_WALKABLE || neighbour.South == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y - 1][x].type);
			int randValue = m_BlockMap[7][yP - 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}

		bool northWalkable = (neighbour.North != N_DIFF && neighbour.North != N_SAME);
		bool northEastWalkable = (neighbour.NorthEast != N_DIFF && neighbour.NorthEast != N_SAME);
		bool northWestWalkable = (neighbour.NorthWest != N_DIFF && neighbour.NorthWest != N_SAME);
		bool southWalkable = (neighbour.South != N_DIFF && neighbour.South != N_SAME);
		bool southEastWalkable = (neighbour.SouthEast != N_DIFF && neighbour.SouthEast != N_SAME);
		bool southWestWalkable = (neighbour.SouthWest != N_DIFF && neighbour.SouthWest != N_SAME);
		bool eastWalkable = (neighbour.East != N_DIFF && neighbour.East != N_SAME);
		bool westWalkable = (neighbour.West != N_DIFF && neighbour.West != N_SAME);

		//corner pieces
		if (northWalkable && eastWalkable || !northWalkable && !eastWalkable && northEastWalkable)
		{
			m_BlockMap[5][yP + 2][xP + 2].uv[2] = tex0[0];
		}
		if (northWalkable && westWalkable || !northWalkable && !westWalkable && northWestWalkable)
		{
			m_BlockMap[5][yP + 2][xP].uv[2] = tex0[0];
		}
		if (southWalkable && eastWalkable || !southWalkable && !eastWalkable && southEastWalkable)
		{
			m_BlockMap[5][yP][xP + 2].uv[2] = tex0[0];
		}
		if (southWalkable && westWalkable || !southWalkable && !westWalkable && southWestWalkable)
		{
			m_BlockMap[5][yP][xP].uv[2] = tex0[0];
		}

	}
	else if (type == Type_Wall2) //wall+corner, this tile can't ever exist on the edges so checking for X/Y -1 / +1 is totally fine
	{
		const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[0];
		for (int sy = 0; sy < 3; sy++)
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sy][xP + sx].uv[2] = tex0[2];
			}

		m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[m_Map.m_Tiles[y][x].owner];
		//for debugging
		//m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[Owner_PlayerBlue];
		bool northWall = m_Map.m_Tiles[y + 1][x].type == Type_Wall0;// && m_Map.m_Tiles[y + 1][x].type <= Type_Wall5;
		bool southWall = m_Map.m_Tiles[y - 1][x].type == Type_Wall0;// && m_Map.m_Tiles[y - 1][x].type <= Type_Wall5;
		bool eastWall = m_Map.m_Tiles[y][x + 1].type == Type_Wall0;//&& m_Map.m_Tiles[y][x + 1].type <= Type_Wall5;
		bool westWall = m_Map.m_Tiles[y][x - 1].type == Type_Wall0;//&& m_Map.m_Tiles[y][x - 1].type <= Type_Wall5;

														   //bool northWall = IsWall(m_Map.m_Tiles[y + 1][x].type);// && m_Map.m_Tiles[y + 1][x].type <= Type_Wall5;
														   //bool southWall = IsWall(m_Map.m_Tiles[y - 1][x].type);// && m_Map.m_Tiles[y - 1][x].type <= Type_Wall5;
														   //bool eastWall = IsWall(m_Map.m_Tiles[y][x + 1].type);//&& m_Map.m_Tiles[y][x + 1].type <= Type_Wall5;
														   //bool westWall = IsWall(m_Map.m_Tiles[y][x - 1].type);//&& m_Map.m_Tiles[y][x - 1].type <= Type_Wall5;
														   //corner pieces
		if (northWall && eastWall)
		{
			m_BlockMap[5][yP + 2][xP + 2].uv[2] = tex0[0];
		}
		if (northWall && westWall)
		{
			m_BlockMap[5][yP + 2][xP].uv[2] = tex0[0];
		}
		if (southWall && eastWall)
		{
			m_BlockMap[5][yP][xP + 2].uv[2] = tex0[0];
		}
		if (southWall && westWall)
		{
			m_BlockMap[5][yP][xP].uv[2] = tex0[0];
		}

		//uint8_t northInfo = GetNeighbourInfo(type, m_Map.m_Tiles[y + 1][x].type);
		//bool northWalkable = (northInfo != N_DIFF && northInfo != N_SAME);
		//uint8_t southInfo = GetNeighbourInfo(type, m_Map.m_Tiles[y - 1][x].type);
		//bool southWalkable = (southInfo != N_DIFF && southInfo != N_SAME);
		//uint8_t eastInfo = GetNeighbourInfo(type, m_Map.m_Tiles[y][x + 1].type);
		//bool eastWalkable = (eastInfo != N_DIFF && eastInfo != N_SAME);
		//uint8_t westInfo = GetNeighbourInfo(type, m_Map.m_Tiles[y][x - 1].type);
		//bool westWalkable = (westInfo != N_DIFF && westInfo != N_SAME);

		uint8_t northInfo = neighbour.North;
		bool northWalkable = (neighbour.North != N_DIFF && neighbour.North != N_SAME);
		uint8_t southInfo = neighbour.South;
		bool southWalkable = (neighbour.South != N_DIFF && neighbour.South != N_SAME);
		uint8_t eastInfo = neighbour.East;
		bool eastWalkable = (neighbour.East != N_DIFF && neighbour.East != N_SAME);
		uint8_t westInfo = neighbour.West;
		bool westWalkable = (neighbour.West != N_DIFF && neighbour.West != N_SAME);

		if (northWalkable)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP + 2][xP + sy].uv[2] = tex0[1];
			}
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y + 1][x].type);
			int randValue = m_BlockMap[7][yP + 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + 2][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (southWalkable)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP][xP + sy].uv[2] = tex0[1];
			}
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y - 1][x].type);
			int randValue = m_BlockMap[7][yP - 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (eastWalkable)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP + 2].uv[2] = tex0[1];
			}
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x + 1].type);
			int randValue = m_BlockMap[7][yP][xP - 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP + 2].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (westWalkable)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP].uv[2] = tex0[1];
			}
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x - 1].type);
			int randValue = m_BlockMap[7][yP][xP + 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}

		if (neighbour.West == N_WALKABLE || neighbour.West == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x - 1].type);
			int randValue = m_BlockMap[7][yP][xP + 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.East == N_WALKABLE || neighbour.East == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x + 1].type);
			int randValue = m_BlockMap[7][yP][xP - 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP + 2].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.North == N_WALKABLE || neighbour.North == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y + 1][x].type);
			int randValue = m_BlockMap[7][yP + 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + 2][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.South == N_WALKABLE || neighbour.South == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y - 1][x].type);
			int randValue = m_BlockMap[7][yP - 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}

		if ((neighbour.NorthEast != N_SAME && neighbour.NorthEast != N_DIFF) && ((northWalkable && eastWalkable) || (northInfo == N_SAME && eastInfo == N_SAME)))
		{
			m_BlockMap[5][yP + 2][xP + 2].uv[2] = tex0[0];
		}
		if (neighbour.NorthWest != N_SAME && neighbour.NorthWest != N_DIFF && ((northWalkable && westWalkable) || (northInfo == N_SAME && westInfo == N_SAME)))
		{
			m_BlockMap[5][yP + 2][xP].uv[2] = tex0[0];
		}
		if (neighbour.SouthEast != N_SAME && neighbour.SouthEast != N_DIFF && ((southWalkable && eastWalkable) || (southInfo == N_SAME && eastInfo == N_SAME)))
		{
			m_BlockMap[5][yP][xP + 2].uv[2] = tex0[0];
		}
		if (neighbour.SouthWest != N_SAME && neighbour.SouthWest != N_DIFF && ((southWalkable && westWalkable) || (southInfo == N_SAME && westInfo == N_SAME)))
		{
			m_BlockMap[5][yP][xP].uv[2] = tex0[0];
		}
	}
	else if (type == Type_Wall3) //corner
	{
		const std::vector<XMFLOAT2>& tex0 = BlockTextures[texIndex].top[0];
		for (int sy = 0; sy < 3; sy++)
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sy][xP + sx].uv[2] = tex0[2];
			}
		m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[m_Map.m_Tiles[y][x].owner];
		//for debugging
		//m_BlockMap[5][yP + 1][xP + 1].uv[2] = t_WallOwners[Owner_PlayerRed];
		bool northWall = neighbour.North == N_SAME;
		bool southWall = neighbour.South == N_SAME;
		bool eastWall = neighbour.East == N_SAME;
		bool westWall = neighbour.West == N_SAME;

		//mark corner pieces
		if (northWall)
		{
			if (eastWall && neighbour.NorthEast == N_WALKABLE)
			{
				m_BlockMap[5][yP + 2][xP + 2].uv[2] = tex0[0];
			}
			if (westWall && neighbour.NorthWest == N_WALKABLE)
			{
				m_BlockMap[5][yP + 2][xP].uv[2] = tex0[0];
			}
		}
		if (southWall)
		{
			if (eastWall && neighbour.SouthEast == N_WALKABLE)
			{
				m_BlockMap[5][yP][xP + 2].uv[2] = tex0[0];
			}
			if (westWall && neighbour.SouthWest == N_WALKABLE)
			{
				m_BlockMap[5][yP][xP].uv[2] = tex0[0];
			}
		}

		//top edges mark a walkable area next to it
		if (neighbour.North != N_SAME && neighbour.North != N_DIFF)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP + 2][xP + sy].uv[2] = tex0[1];
			}
		}
		if (neighbour.South != N_SAME && neighbour.South != N_DIFF)
		{
			for (int sy = 0; sy < 3; sy++)
			{
				m_BlockMap[5][yP][xP + sy].uv[2] = tex0[1];
			}
		}
		if (neighbour.East != N_SAME && neighbour.East != N_DIFF)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP + 2].uv[2] = tex0[1];
			}
		}
		if (neighbour.West != N_SAME && neighbour.West != N_DIFF)
		{
			for (int sx = 0; sx < 3; sx++)
			{
				m_BlockMap[5][yP + sx][xP].uv[2] = tex0[1];
			}
		}



		//Set the wall side to their respective texture
		if (neighbour.West == N_WALKABLE || neighbour.West == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x - 1].type);
			int randValue = m_BlockMap[7][yP][xP + 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					//maps | sx + (3 * (3 - (z - 2))) |  to a value within 3*4 area, upper left being 0, lower right being 8
					m_BlockMap[z][yP + sx][xP].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.East == N_WALKABLE || neighbour.East == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y][x + 1].type);
			int randValue = m_BlockMap[7][yP][xP - 3].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + sx][xP + 2].uv[0] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.North == N_WALKABLE || neighbour.North == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y + 1][x].type);
			int randValue = m_BlockMap[7][yP + 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP + 2][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
		if (neighbour.South == N_WALKABLE || neighbour.South == N_WATER)
		{
			uint16_t texIndexRoom = TypeToTexture(m_Map.m_Tiles[y - 1][x].type);
			int randValue = m_BlockMap[7][yP - 3][xP].randValue;//we use the random value of one block as the entire side will need to match
			const std::vector<XMFLOAT2>& tex1 = BlockTextures[texIndexRoom].wall[randValue % BlockTextures[texIndexRoom].wall.size()];
			for (int sx = 0; sx < 3; sx++)
			{
				for (int z = 2; z < MAP_SIZE_HEIGHT - 2; z++)
				{
					m_BlockMap[z][yP][xP + sx].uv[1] = tex1[sx + (3 * (3 - (z - 2)))];
				}
			}
		}
	}
}