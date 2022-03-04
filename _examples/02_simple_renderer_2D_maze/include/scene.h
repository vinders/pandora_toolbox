/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering scene
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include "renderer_context.h"
#include "resources.h"
#include "maze.h"

#define __BASE_WINDOW_WIDTH  928
#define __BASE_WINDOW_HEIGHT 640
#define __KEY_DELTA 3 // keyboard action delta (pixels per clock cycle)

// Rendering scene: scenery and entities
// --> contains camera view, entities/models, textures/materials/shaders...
class Scene final {
public:
  // Special game status
  enum class Status : int {
    none = 0,
    updated = 1,  // pending frame to draw
    finished = 2  // game over -> should be restarted
  };

  // ---

  // Create rendering scene components
  Scene(RendererContext& renderer) {
    restartScene(renderer);
  }
  // Create new maze + re-initialize game data and rendering scene components
  // --> throws on scene creation failure
  void restartScene(RendererContext& renderer);

  Scene() = default;
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  ~Scene() noexcept { release(); }

  // Re-initialize scene rendering components (only called if the RendererContext instance is re-created during game)
  // --> throws on scene creation failure
  void initResources(RendererContext& renderer);
  // Destroy scene components (should be called before destroying associated RendererContext)
  void release() noexcept;


  // -- operations --

  // Store user input events
  void storeUserInput(uint32_t keyCode, bool isKeyDown);
  // Process game business logic (based on user events)
  void processGameLogic();

  bool isUpdated() const noexcept { return (_status != Status::none); }      // Verify if a frame is pending
  bool isFinished() const noexcept { return (_status == Status::finished); } // Verify if the game is over
  void refreshScreen() noexcept { // Force pending frame repaint
    if (_status != Status::finished)
      _status = Status::updated;
  }

  // Compute and draw scene entities
  // --> throws if device lost -> recreate RendererContext + call init)
  void render();

  // ---

private:
  // Player orientation (for visual representation)
  // --> for simplicity, use same order as spritesheet lines -> can be used as line index
  enum class PlayerDirection : int {
    down = 0,  // coord: 0*32px
    right = 1, // coord: 1*32px
    up = 2,    // coord: 2*32px
    left = 3   // coord: 3*32px
  };
  // Player position on screen (pixels)
  struct PlayerPosition final {
    int x; // pixels
    int y; // pixels
  };

  void _updatePlayerSprite(float animationCoord); // update player appearance (orientation / animation)

private:
  RendererContext* _renderer = nullptr;
  ResourceStorage _resources;
  Maze _maze;

  Status _status = Status::none;
  PlayerPosition _playerPosition{ 0, 0 };
  PlayerDirection _playerDirection = PlayerDirection::down;
  bool _isPlayerMoving = false;

  int64_t _lastClockTicks = 0;
  uint32_t _playerAnimationFrame = 0;
};
