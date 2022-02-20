/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - maze container + generator
*******************************************************************************/
#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include <system/preprocessor_tools.h>

#define __MAZE_TILESET_SIZE 128
#define __MAZE_TILE_PIXELS  32
#define __MAZE_TILE_PIXEL_SUBMASK 0x1F

enum class MazeDirection : uint8_t {
  invalid = 0,
  up    = 0x1,
  right = 0x2,
  down  = 0x4,
  left  = 0x8,
  all = 0xF
};
_P_FLAGS_OPERATORS(MazeDirection, uint8_t)
#define __MAZE_DIRECTION_FIRST MazeDirection::up
#define __MAZE_DIRECTION_LAST MazeDirection::left

struct CellPosition final {
  int x;
  int y;
};

// ---

// Maze container
class Maze final {
public:
  // Generate a new maze pattern
  // -> baseDisplayWidth/baseDisplayHeight: unscaled/original window size
  Maze(uint32_t baseDisplayWidth, uint32_t baseDisplayHeight);

  Maze() noexcept = default;
  Maze(const Maze&) = delete;
  Maze(Maze&&) noexcept = default;
  Maze& operator=(const Maze&) = delete;
  Maze& operator=(Maze&&) noexcept = default;

  // Generate vertices/indices, based on texture file "hedge.png"
  void computeRendering(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices,
                        float outGameAreaOffset[2]);
  
  const CellPosition& getStartPosition() noexcept { return _startPosition; } // Start position of player
  const CellPosition& getEndPosition() noexcept { return _endPosition; } // End position to reach

  // Verify if a player move is allowed
  inline bool isCellDirectionValid(int cellX, int cellY, MazeDirection direction) noexcept {
    return (cellX < 0 || cellY < 0 || cellX >= (int)_rows || cellY >= (int)_lines // out of bounds: allow all moves to come back
         || (_cells[cellY*(int)_rows + cellX] & direction) == true);              // valid cell: check constraints
  }

private:
  std::unique_ptr<MazeDirection[]> _cells = nullptr;
  uint32_t _lines = 0;
  uint32_t _rows = 0;
  CellPosition _startPosition{ 0, 0 };
  CellPosition _endPosition{ 0, 0 };

  uint32_t _baseDisplayWidth = 0;
  uint32_t _baseDisplayHeight = 0;
};
