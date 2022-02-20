/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - maze container + generator
Inspiration : en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search
*******************************************************************************/
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <vector>
#include <random>
#include "maze.h"

// -- randomizer --

std::random_device randomDevice;
std::mt19937 randGenerator(randomDevice());
std::uniform_int_distribution<> directionDist( 0, 3 );

static inline MazeDirection __randDirection() noexcept {
  return static_cast<MazeDirection>(1 << (int)directionDist(randGenerator));
}


// -- helpers --

// Enum values                     0  1  2  3  4  5  6  7  8
// Direction                          U  R     D           L
static constexpr int g_offsetX[] { 0, 0,+1, 0, 0, 0, 0, 0,-1 };
static constexpr int g_offsetY[] { 0,-1, 0, 0,+1, 0, 0, 0, 0 };
static constexpr MazeDirection g_oppositeMask[] {
  (MazeDirection)0,
  MazeDirection::down | (MazeDirection)((int)MazeDirection::down << 4),
  MazeDirection::left | (MazeDirection)((int)MazeDirection::left << 4),
  (MazeDirection)0,
  MazeDirection::up | (MazeDirection)((int)MazeDirection::up << 4),
  (MazeDirection)0,
  (MazeDirection)0,
  (MazeDirection)0,
  MazeDirection::right | (MazeDirection)((int)MazeDirection::right << 4)
};

static inline int __cellIndex(uint32_t rows, int x, int y) noexcept {
  return ((y*(int)rows) + x);
}


// -- maze generator --

static inline bool __isDirectionAvailable(MazeDirection dir, CellPosition& currentPos, MazeDirection* cells,
                                          uint32_t lines, uint32_t rows) noexcept {
  int nextPosX = currentPos.x + g_offsetX[(int)dir];
  int nextPosY = currentPos.y + g_offsetY[(int)dir];

  if (dir == MazeDirection::invalid || nextPosX < 0 || nextPosY < 0 || nextPosX >= (int)rows || nextPosY >= (int)lines)
	return false;
  return (cells[__cellIndex(rows, nextPosX, nextPosY)] == (MazeDirection)0);
}

static MazeDirection __computeDirection(CellPosition& currentPos, MazeDirection* cells,
                                        uint32_t lines, uint32_t rows) noexcept {
  MazeDirection dir = __randDirection();
  do {
	for (int i = 0; i < 4; ++i) { // loop through all possible directions (with random first direction)
	  if (__isDirectionAvailable(dir, currentPos, cells, lines, rows))
		return dir;

	  dir = static_cast<MazeDirection>((uint8_t)dir << 1);
	  if (dir > __MAZE_DIRECTION_LAST)
		dir = __MAZE_DIRECTION_FIRST;
	}

	dir = static_cast<MazeDirection>( ((int)cells[__cellIndex(rows, currentPos.x, currentPos.y)]
	                                  & (int)0xF0) >> 4);

	if (dir != MazeDirection::invalid) {
	  currentPos.x += g_offsetX[(int)dir];
	  currentPos.y += g_offsetY[(int)dir];
	  dir = __randDirection();
	}
  } while (dir != MazeDirection::invalid);
  return MazeDirection::invalid; // no place left to go
}

static void __computeStartEndPositions(MazeDirection* cells, uint32_t lines, uint32_t rows,
                                       CellPosition& outStartPos, CellPosition& outEndPos) noexcept {
  if ((int)directionDist(randGenerator) & 0x1) { // top/bottom
	outStartPos.y = 0;
	outEndPos.y = (int)lines;
	if ((int)directionDist(randGenerator) >= 2) {
	  outStartPos.x = (int)(rows >> 1);
	  outEndPos.x = outStartPos.x - 1;
	}
	else {
	  outEndPos.x = (int)(rows >> 1);
	  outStartPos.x = outEndPos.x - 1;
	}
	cells[__cellIndex(rows, outStartPos.x, outStartPos.y)] |= MazeDirection::up;
	cells[__cellIndex(rows, outEndPos.x, outEndPos.y - 1)] |= MazeDirection::down;
  }
  else { // left/right
	outStartPos.x = 0;
	outEndPos.x = (int)rows;
	if ((int)directionDist(randGenerator) >= 2) {
	  outStartPos.y = (int)(lines >> 1);
	  outEndPos.y = outStartPos.y - 1;
	}
	else {
	  outEndPos.y = (int)(lines >> 1);
	  outStartPos.y = outEndPos.y - 1;
	}
	cells[__cellIndex(rows, outStartPos.x, outStartPos.y)] |= MazeDirection::left;
	cells[__cellIndex(rows, outEndPos.x - 1, outEndPos.y)] |= MazeDirection::right;
  }
}

// ---

static void __generateMaze(MazeDirection* cells, uint32_t lines, uint32_t rows,
                           CellPosition& outStartPos, CellPosition& outEndPos) {
  assert(cells != nullptr && lines > 0 && rows > 0);
  randGenerator.seed((unsigned int)time(NULL));
  
  CellPosition currentPos; // set beginning position for processing
  if (lines >= rows) {
	std::uniform_int_distribution<> cellDist(0, lines - 1);
	currentPos.x = static_cast<uint32_t>(cellDist(randGenerator)) % rows;
	currentPos.y = static_cast<uint32_t>(cellDist(randGenerator));
  }
  else {
	std::uniform_int_distribution<> cellDist(0, rows - 1);
	currentPos.x = static_cast<uint32_t>(cellDist(randGenerator));
	currentPos.y = static_cast<uint32_t>(cellDist(randGenerator)) % lines;
  }

  for (MazeDirection dir = __computeDirection(currentPos, cells, lines, rows);
	   dir != MazeDirection::invalid;
	   dir = __computeDirection(currentPos, cells, lines, rows)) {
	cells[__cellIndex(rows, currentPos.x, currentPos.y)] |= dir; // open new direction

	dir &= MazeDirection::all; // avoid out of bounds array access
	currentPos.x += g_offsetX[(int)dir];
	currentPos.y += g_offsetY[(int)dir];
	cells[__cellIndex(rows, currentPos.x, currentPos.y)] = g_oppositeMask[(int)dir]; // next cell: open dir to previous
  }
  __computeStartEndPositions(cells, lines, rows, outStartPos, outEndPos);
}

// ---

// Generate a new maze pattern
Maze::Maze(uint32_t baseDisplayWidth, uint32_t baseDisplayHeight)
  : _baseDisplayWidth(baseDisplayWidth), _baseDisplayHeight(baseDisplayHeight)
{
  // max integer odd number of lines that fit (minus 1 line for visible hedge side)
  uint32_t tilesY = ((baseDisplayHeight - __MAZE_TILE_PIXELS) / __MAZE_TILE_PIXELS);
  if ((tilesY & 0x1) == 0)
	--tilesY;
  // max integer odd number of rows that fit
  uint32_t tilesX = (baseDisplayWidth / __MAZE_TILE_PIXELS);
  if ((tilesX & 0x1) == 0)
	--tilesX;

  _lines = tilesY >> 1; // half the tiles are walls -> divide by 2 for cell count
  _rows = tilesX >> 1;   // half the tiles are walls -> divide by 2 for cell count

  _cells.reset(new MazeDirection[_lines*_rows]);
  memset(_cells.get(), 0, _lines*_rows*sizeof(MazeDirection));
  __generateMaze(_cells.get(), _lines, _rows, _startPosition, _endPosition);
}


// -- rendering --

// hedge.png:
// texcoord / size  / world offset
//     0; 0 / 32*64 / 0;+64: vertical top-end
//     0;64 / 32*32 / 0;+32: vertical
//     0;96 / 32*32 / 0;0: vertical bottom-end
//    32; 0 / 64*64 / -32;+32: horizontal left-end
//    64; 0 / 32*64 / 0;+32: horizontal
//    96; 0 / 32*64 / +32;+32: horizontal right-end

#define __TEX_TILE_WIDTH   ((float)__MAZE_TILE_PIXELS/(float)__MAZE_TILESET_SIZE)
#define __TEX_TILE_HEIGHT  ((float)__MAZE_TILE_PIXELS/(float)__MAZE_TILESET_SIZE)

enum class HedgeTiles : uint32_t {
  verticalTopEnd     = ((0 << 24) | (0 << 16) | (1 << 8) | 2),
  vertical           = ((0 << 24) | (2 << 16) | (1 << 8) | 1),
  verticalDouble     = ((0 << 24) | (1 << 16) | (1 << 8) | 2),
  verticalBottomEnd  = ((0 << 24) | (3 << 16) | (1 << 8) | 1),
  horizontalLeftEnd  = ((1 << 24) | (0 << 16) | (2 << 8) | 2),
  horizontal         = ((2 << 24) | (0 << 16) | (1 << 8) | 2),
  horizontalRightEnd = ((2 << 24) | (0 << 16) | (1 << 8) | 2)
};
static inline float __getTextureCoordX(HedgeTiles type) noexcept {
  return static_cast<float>(((uint32_t)type >> 24) & 0xFFu) * __TEX_TILE_WIDTH;
}
static inline float __getTextureCoordY(HedgeTiles type) noexcept {
  return static_cast<float>(((uint32_t)type >> 16) & 0xFFu) * __TEX_TILE_WIDTH;
}
static inline float __getTextureTileWidth(HedgeTiles type) noexcept {
  return static_cast<float>(((uint32_t)type >> 8) & 0x3u) * __TEX_TILE_WIDTH;
}
static inline float __getTextureTileHeight(HedgeTiles type) noexcept {
  return static_cast<float>((uint32_t)type & 0x3u) * __TEX_TILE_HEIGHT;
}
static inline float __getWorldTileWidth(HedgeTiles type, float worldTileSize) noexcept {
  return static_cast<float>(((uint32_t)type >> 8) & 0x3u) * worldTileSize;
}
static inline float __getWorldTileHeight(HedgeTiles type, float worldTileSize) noexcept {
  return static_cast<float>((uint32_t)type & 0x3u) * worldTileSize;
}

// ---

static void __computeVertices(HedgeTiles type, float worldPosX, float worldPosY, float worldTileSize[2],
							  std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
  float texCoordX = __getTextureCoordX(type);
  float texCoordY = __getTextureCoordY(type);
  float texWidth = __getTextureTileWidth(type);
  float texHeight = __getTextureTileHeight(type);
  float worldItemWidth = __getWorldTileWidth(type, worldTileSize[0]);
  float worldItemHeight = __getWorldTileHeight(type, worldTileSize[1]);
  
  uint32_t vertexIndex = static_cast<uint32_t>(outVertices.size() >> 2); // 4 floats per vertex
  outVertices.insert(outVertices.end(), { worldPosX,                  worldPosY, // top-left vertex
                                          texCoordX,                  texCoordY,
                                          worldPosX + worldItemWidth, worldPosY, // top-right vertex
                                          texCoordX + texWidth,       texCoordY,
                                          worldPosX,                  worldPosY - worldItemHeight, // bottom-left vertex
                                          texCoordX,                  texCoordY + texHeight,
                                          worldPosX + worldItemWidth, worldPosY - worldItemHeight, // bottom-right vertex
                                          texCoordX + texWidth,       texCoordY + texHeight });

  outIndices.insert(outIndices.end(), { vertexIndex,vertexIndex+1,vertexIndex+2,     // indices: 0,1,2
	                                    vertexIndex+1,vertexIndex+3,vertexIndex+2 });//          1,3,2
}

// ---

// Generate vertices/indices, based on texture file "hedge.png"
void Maze::computeRendering(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices, float outGameAreaOffset[2]) {
  uint32_t tilesX = (_rows << 1) + 1;  // 2 tiles per cell + right tile
  uint32_t tilesY = (_lines << 1) + 2; // 2 tiles per cell + bottom tiles

  // offset for centered hedge position
  float hedgeOffset[]{ (float)(_baseDisplayWidth - tilesX*__MAZE_TILE_PIXELS) / (float)_baseDisplayWidth, // percentage of unused space
                       (float)(_baseDisplayHeight - tilesY*__MAZE_TILE_PIXELS) / (float)_baseDisplayHeight };
  float worldTileSize[]{ (2.f*__MAZE_TILE_PIXELS) / (float)_baseDisplayWidth, // actual tile-size (on window)
	                     (2.f*__MAZE_TILE_PIXELS) / (float)_baseDisplayHeight };
  hedgeOffset[1] = -hedgeOffset[1] + worldTileSize[1]*0.25f; // add slight offset for hedge bottom side

  // game area offset (!= hedge offset: player isn't at same level as top of hedges)
  outGameAreaOffset[0] = hedgeOffset[0];
  outGameAreaOffset[1] = hedgeOffset[1] - worldTileSize[1]*0.375f;

  MazeDirection* mazeIt = _cells.get();
  for (uint32_t line = 0; line < _lines; ++line) {
	for (uint32_t row = 0; row < _rows; ++row, ++mazeIt) {
	  float worldPosX = hedgeOffset[0] + (float)(row<<1)*worldTileSize[0] - 1.f;
	  float worldPosY = hedgeOffset[1] + 1.f - (float)(line<<1)*worldTileSize[1];

	  // convolution
	  MazeDirection leftCell = (row > 0) ? *(mazeIt - 1) : (MazeDirection::up|MazeDirection::down|MazeDirection::left);
	  MazeDirection aboveCell = (line > 0) ? *(mazeIt - _rows) : (MazeDirection::up|MazeDirection::left|MazeDirection::right);
	  MazeDirection rightCell = (row + 1u != _rows)
		                      ? *(mazeIt + 1)
		                      : (((*mazeIt & MazeDirection::right) == true) ? MazeDirection::all : (MazeDirection::up|MazeDirection::down));
	  MazeDirection bottomCell = (line + 1u != _lines)
		                       ? *(mazeIt + _rows)
		                       : (((*mazeIt & MazeDirection::down) == true) ? MazeDirection::all : (MazeDirection::left|MazeDirection::right));

	  // tile association
	  if ((*mazeIt & MazeDirection::up) == false) {
		if ((leftCell & MazeDirection::up) == false) {
		  __computeVertices(HedgeTiles::horizontal, worldPosX, worldPosY,
			                worldTileSize, outVertices, outIndices);
		  __computeVertices(HedgeTiles::horizontal, worldPosX + worldTileSize[0], worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		else {
		  __computeVertices(HedgeTiles::horizontalLeftEnd, worldPosX, worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		if ((rightCell & MazeDirection::up) == true) {
		  __computeVertices(HedgeTiles::horizontalRightEnd, worldPosX + 2.f*worldTileSize[0], worldPosY,
							worldTileSize, outVertices, outIndices);
		}
	  }
	  if ((*mazeIt & MazeDirection::left) == false) {
		if ((aboveCell & MazeDirection::left) == false) {
		  __computeVertices(HedgeTiles::verticalDouble, worldPosX, worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		else {
		  __computeVertices(HedgeTiles::verticalTopEnd, worldPosX, worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		if ((bottomCell & MazeDirection::left) == true) {
		  __computeVertices(HedgeTiles::vertical, worldPosX, worldPosY - 2.f*worldTileSize[1],
							  worldTileSize, outVertices, outIndices);
		  if ((*mazeIt & MazeDirection::down) == true && (leftCell & MazeDirection::down) == true) {
			__computeVertices(HedgeTiles::verticalBottomEnd, worldPosX, worldPosY - 3.f*worldTileSize[1],
							  worldTileSize, outVertices, outIndices);
		  }
		}
	  }
	  if (row + 1u == _rows && (*mazeIt & MazeDirection::right) == false) { // ignore right hedges, except for last row
		if ((aboveCell & MazeDirection::right) == false) {
		  __computeVertices(HedgeTiles::verticalDouble, worldPosX + 2.f*worldTileSize[0], worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		else {
		  __computeVertices(HedgeTiles::verticalTopEnd, worldPosX + 2.f*worldTileSize[0], worldPosY,
			                worldTileSize, outVertices, outIndices);
		}
		if ((bottomCell & MazeDirection::right) == true) {
		  __computeVertices(HedgeTiles::vertical, worldPosX + 2.f*worldTileSize[0], worldPosY - 2.f*worldTileSize[1],
							  worldTileSize, outVertices, outIndices);
		  if ((*mazeIt & MazeDirection::down) == true) {
			__computeVertices(HedgeTiles::verticalBottomEnd, worldPosX + 2.f*worldTileSize[0], worldPosY - 3.f*worldTileSize[1],
							  worldTileSize, outVertices, outIndices);
		  }
		}
	  }
	  if (line + 1u == _lines && (*mazeIt & MazeDirection::down) == false) {// ignore bottom hedges, except for last line
		if ((leftCell & MazeDirection::down) == false) {
		  __computeVertices(HedgeTiles::horizontal, worldPosX, worldPosY - 2.f*worldTileSize[1],
			                worldTileSize, outVertices, outIndices);
		  __computeVertices(HedgeTiles::horizontal, worldPosX + worldTileSize[0], worldPosY - 2.f*worldTileSize[1],
			                worldTileSize, outVertices, outIndices);
		}
		else {
		  __computeVertices(HedgeTiles::horizontalLeftEnd, worldPosX, worldPosY - 2.f*worldTileSize[1],
			                worldTileSize, outVertices, outIndices);
		}
		if ((rightCell & MazeDirection::down) == true) {
		  __computeVertices(HedgeTiles::horizontalRightEnd, worldPosX + 2.f*worldTileSize[0], worldPosY - 2.f*worldTileSize[1],
							worldTileSize, outVertices, outIndices);
		}
	  }
	}
  }
}
