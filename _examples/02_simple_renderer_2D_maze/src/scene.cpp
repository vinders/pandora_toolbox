/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering scene
*******************************************************************************/
#include <cassert>
#include <cstdint>
#include <vector>
#include <time/system_clocks.h>
#include <video/window_keycodes.h>
#include "resources.h"
#include "scene.h"

using namespace video_api;
using pandora::time::SteadyClock;


// -- geometry generators -- ---------------------------------------------------

static void __generateUi(std::shared_ptr<Renderer>& renderer, ResourceStorage& outStorage) {
  // load required resources
  if (outStorage.pipeline.isEmpty())
    loadPipeline(renderer, outStorage);
  if (outStorage.images.find(ImageId::title) == outStorage.images.end())
    loadImage(*renderer, ImageId::title, 160, 32, outStorage);

  constexpr const float worldSizeX  = 2.f*160.f/(float)__BASE_WINDOW_WIDTH;
  constexpr const float worldSizeY = 2.f*32.f/(float)__BASE_WINDOW_HEIGHT;
  const float titleArea[] = { -0.9f, -0.9f + worldSizeY, -0.9f + worldSizeX, -0.9f };

  // create geometry
  const float vertices[] = { titleArea[0],titleArea[1], 0.f,0.f,  titleArea[2],titleArea[1], 1.f,0.f,
                             titleArea[0],titleArea[3], 0.f,1.f,  titleArea[2],titleArea[3], 1.f,1.f };
  uint32_t indices[]{ 0,1,2, 1,3,2 };

  outStorage.entities.push_back(Entity2D{
    ImmutableBuffer(*renderer, BufferType::vertex, sizeof(vertices), vertices),
    ImmutableBuffer(*renderer, BufferType::vertexIndex, sizeof(indices), indices),
    static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
    ImageId::title
  });
}

static void __generateGrass(std::shared_ptr<Renderer>& renderer, ResourceStorage& outStorage) {
  // load required resources
  if (outStorage.pipeline.isEmpty())
    loadPipeline(renderer, outStorage);
  if (outStorage.images.find(ImageId::grass) == outStorage.images.end())
    loadImage(*renderer, ImageId::grass, 32, 32, outStorage);

  constexpr const float textureSize = 32.f;
  constexpr const float repeatCountX = (float)__BASE_WINDOW_WIDTH/textureSize;
  constexpr const float repeatCountY = (float)__BASE_WINDOW_HEIGHT/textureSize;

  // create geometry
  float vertices[]{ -1.f,1.f,  0.f,0.f,           1.f,1.f,  repeatCountX,0.f,
                    -1.f,-1.f, 0.f,repeatCountY,  1.f,-1.f, repeatCountX,repeatCountY };
  uint32_t indices[]{ 0,1,2, 1,3,2 };

  outStorage.entities.push_back(Entity2D{
    ImmutableBuffer(*renderer, BufferType::vertex, sizeof(vertices), vertices),
    ImmutableBuffer(*renderer, BufferType::vertexIndex, sizeof(indices), indices),
    static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
    ImageId::grass
  });
}

static void __generateHedge(std::shared_ptr<Renderer>& renderer, Maze& maze, float outGameAreaOffset[2], ResourceStorage& outStorage) {
  // load required resources
  if (outStorage.pipeline.isEmpty())
    loadPipeline(renderer, outStorage);
  if (outStorage.images.find(ImageId::hedge) == outStorage.images.end())
    loadImage(*renderer, ImageId::hedge, __MAZE_TILESET_SIZE, __MAZE_TILESET_SIZE, outStorage);

  // create geometry
  std::vector<float> vertices;
  std::vector<uint32_t> indices;
  maze.computeRendering(vertices, indices, outGameAreaOffset);

  outStorage.entities.push_back(Entity2D{
    ImmutableBuffer(*renderer, BufferType::vertex, vertices.size()*sizeof(vertices[0]), &vertices[0]),
    ImmutableBuffer(*renderer, BufferType::vertexIndex, indices.size()*sizeof(indices[0]), &indices[0]),
    static_cast<uint32_t>(indices.size()),
    ImageId::hedge
  });
}

static void __generatePlayer(std::shared_ptr<Renderer>& renderer, float gameAreaOffset[2], ResourceStorage& outStorage) {
  // load required resources
  if (outStorage.pipeline.isEmpty())
    loadPipeline(renderer, outStorage);
  if (outStorage.images.find(ImageId::player) == outStorage.images.end())
    loadImage(*renderer, ImageId::player, 128, 128, outStorage);

  constexpr const float spriteSize = 32.f; // size of one orientation / animation state
  constexpr const float worldSizeX = 2.f*spriteSize/(float)__BASE_WINDOW_WIDTH;
  constexpr const float worldSizeY = 2.f*spriteSize/(float)__BASE_WINDOW_HEIGHT;
  const float worldPositionX = -1.f + gameAreaOffset[0];
  const float worldPositionY = 1.f + gameAreaOffset[1];

  // create geometry
  float vertices[]{ worldPositionX,            worldPositionY,            0.25f,0.f,
                    worldPositionX+worldSizeX, worldPositionY,            0.5f, 0.f,
                    worldPositionX,            worldPositionY-worldSizeY, 0.25f,0.25f,
                    worldPositionX+worldSizeX, worldPositionY-worldSizeY, 0.5f, 0.25f };
  uint32_t indices[]{ 0,1,2, 1,3,2 };

  outStorage.entities.push_back(Entity2D{
    ImmutableBuffer(*renderer, BufferType::vertex, sizeof(vertices), vertices),
    ImmutableBuffer(*renderer, BufferType::vertexIndex, sizeof(indices), indices),
    static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
    ImageId::player
  });
}


// -- game logic -- ------------------------------------------------------------

void Scene::storeUserInput(uint32_t keyCode, bool isKeyDown) {
  PlayerDirection keyDirection = (PlayerDirection)-1;
  switch (keyCode) {
    case _P_VK_NUMPAD_2:
    case _P_VK_ARROW_DOWN:  keyDirection = PlayerDirection::down; break;
    case _P_VK_NUMPAD_6:
    case _P_VK_ARROW_RIGHT: keyDirection = PlayerDirection::right; break;
    case _P_VK_NUMPAD_8:
    case _P_VK_ARROW_UP:    keyDirection = PlayerDirection::up; break;
    case _P_VK_NUMPAD_4:
    case _P_VK_ARROW_LEFT:  keyDirection = PlayerDirection::left; break;
    default: return; // not a movement key -> skip part below
  }

  if (isKeyDown) {
    _playerDirection = keyDirection;
    _isPlayerMoving = true;
  }
  else if (_playerDirection == keyDirection) // key-up: only if it's the last key pressed
    _isPlayerMoving = false;
}

// ---

// realign player position with perpendicular row/line he's trying to reach (to make it easier to turn)
static void __alignPlayerPosition(int cellPxCoord, int& inOutPlayerPxCoord) {
  int alignedPx = cellPxCoord*(2*__MAZE_TILE_PIXELS) + __MAZE_TILE_PIXELS; // align pixel to tile origin

  // if at least half the player size is in the target row/line, align player coord
  if (alignedPx - __MAZE_TILE_PIXELS/2 <= inOutPlayerPxCoord && alignedPx + __MAZE_TILE_PIXELS/2 >= inOutPlayerPxCoord) {
    if (alignedPx - inOutPlayerPxCoord <= __KEY_DELTA && inOutPlayerPxCoord - alignedPx <= __KEY_DELTA)
      inOutPlayerPxCoord = alignedPx;    // alignment smaller than keyboard action delta -> set exact position
    else if (alignedPx < inOutPlayerPxCoord)
      inOutPlayerPxCoord -= __KEY_DELTA; // too far from aligned position -> only add delta (avoid "jumps")
    else
      inOutPlayerPxCoord += __KEY_DELTA;
  }
}

// process game business logic (based on user events)
void Scene::processGameLogic() {
  if (_isPlayerMoving && _status != Status::finished) {
    int64_t clockTicks = SteadyClock::countTicks();
    if (clockTicks != _lastClockTicks) { // prevent game logic from running more than once per clock cycle
      _lastClockTicks = clockTicks;

      int cellX = _playerPosition.x / (2*__MAZE_TILE_PIXELS);
      int cellY = _playerPosition.y / (2*__MAZE_TILE_PIXELS);

      switch (_playerDirection) {
        case PlayerDirection::down: {
          if (_playerPosition.y + __KEY_DELTA < __BASE_WINDOW_HEIGHT - __MAZE_TILE_PIXELS) {
            if ((_playerPosition.x & __MAZE_TILE_PIXEL_SUBMASK) == 0
            && (_playerPosition.x / __MAZE_TILE_PIXELS) & 0x1) { // only if perfectly aligned with path
              int nextCellY = (cellY + 1)*(2*__MAZE_TILE_PIXELS);

              if (_playerPosition.y + __KEY_DELTA + __MAZE_TILE_PIXELS <= nextCellY // same tile
              || _maze.isCellDirectionValid(cellX, cellY, MazeDirection::down))     // or tile change + direction allowed
                _playerPosition.y += __KEY_DELTA;
              else
                _playerPosition.y = nextCellY - __MAZE_TILE_PIXELS; // no tile change allowed -> move to closest available location
            }
            else // help player being aligned with path
              __alignPlayerPosition(cellX, _playerPosition.x);
          }
          else // max boundary -> limit value
            _playerPosition.y = __BASE_WINDOW_HEIGHT - __MAZE_TILE_PIXELS;
          break;
        }
        case PlayerDirection::right: {
          if (_playerPosition.x < __BASE_WINDOW_WIDTH - __MAZE_TILE_PIXELS) {
            if ((_playerPosition.y & __MAZE_TILE_PIXEL_SUBMASK) == 0
            && (_playerPosition.y / __MAZE_TILE_PIXELS) & 0x1) { // only if perfectly aligned with path
              int nextCellX = (cellX + 1)*(2*__MAZE_TILE_PIXELS);

              if (_playerPosition.x + __KEY_DELTA + __MAZE_TILE_PIXELS <= nextCellX // same tile
              || _maze.isCellDirectionValid(cellX, cellY, MazeDirection::right))    // or tile change + direction allowed
                _playerPosition.x += __KEY_DELTA;
              else
                _playerPosition.x = nextCellX - __MAZE_TILE_PIXELS; // no tile change allowed -> move to closest available location
            }
            else // help player being aligned with path
              __alignPlayerPosition(cellY, _playerPosition.y);
          }
          else // max boundary -> limit value
            _playerPosition.x = __BASE_WINDOW_WIDTH - __MAZE_TILE_PIXELS;
          break;
        }
        case PlayerDirection::up: {
          if (_playerPosition.y - __KEY_DELTA > 0) {
            if ((_playerPosition.x & __MAZE_TILE_PIXEL_SUBMASK) == 0
            && (_playerPosition.x / __MAZE_TILE_PIXELS) & 0x1) { // only if perfectly aligned with path
              int prevCellY = cellY*(2*__MAZE_TILE_PIXELS) + __MAZE_TILE_PIXELS - 1;

              if (_playerPosition.y - __KEY_DELTA > prevCellY                 // same tile
              || _maze.isCellDirectionValid(cellX, cellY, MazeDirection::up)) // or tile change + direction allowed
                _playerPosition.y -= __KEY_DELTA;
              else
                _playerPosition.y = prevCellY + 1; // no tile change allowed -> move to closest available location
            }
            else // help player being aligned with path
              __alignPlayerPosition(cellX, _playerPosition.x);
          }
          else // min boundary -> limit value
            _playerPosition.y = 0;
          break;
        }
        case PlayerDirection::left: {
          if (_playerPosition.x - __KEY_DELTA > 0) {
            if ((_playerPosition.y & __MAZE_TILE_PIXEL_SUBMASK) == 0
            && (_playerPosition.y / __MAZE_TILE_PIXELS) & 0x1) { // only if perfectly aligned with path
              int prevCellX = cellX*(2*__MAZE_TILE_PIXELS) + __MAZE_TILE_PIXELS - 1;

              if (_playerPosition.x - __KEY_DELTA > prevCellX                   // same tile
              || _maze.isCellDirectionValid(cellX, cellY, MazeDirection::left)) // or tile change + direction allowed
                _playerPosition.x -= __KEY_DELTA;
              else
                _playerPosition.x = prevCellX + 1; // no tile change allowed -> move to closest available location
            }
            else // help player being aligned with path
              __alignPlayerPosition(cellY, _playerPosition.y);
          }
          else // min boundary -> limit value
            _playerPosition.x = 0;
          break;
        }
        default: break;
      }

      // animate walking player
      ++_playerAnimationFrame;
      float animationCoord = -0.25f + 0.25f*static_cast<float>((_playerAnimationFrame%32) >> 3);
      if (animationCoord > 0.25f)
        animationCoord = 0.f;
      _updatePlayerSprite(animationCoord);

      // update status: new frame pending (+ game finished if end position reached)
      _status = (_maze.getEndPosition().x == cellX && _maze.getEndPosition().y == cellY) ? Status::finished : Status::updated;
    }
  }
  else if (_playerAnimationFrame != 0) { // player was moving in previous frame -> restore idle sprite
    _playerAnimationFrame = 0;
    _updatePlayerSprite(0.f);
    _status = Status::updated;
  }
}


// -- rendering scene components -- --------------------------------------------

void Scene::restartScene(RendererContext& renderer, uint32_t width, uint32_t height) {
  // create maze + game status
  _maze = Maze(__BASE_WINDOW_WIDTH, __BASE_WINDOW_HEIGHT);

  _playerPosition.x = _maze.getStartPosition().x ? ((_maze.getStartPosition().x << 1) + 1)*__MAZE_TILE_PIXELS : 0;
  _playerPosition.y = _maze.getStartPosition().y ? ((_maze.getStartPosition().y << 1) + 1)*__MAZE_TILE_PIXELS : 0;
  _playerDirection = (_playerPosition.x == 0) ? PlayerDirection::right : PlayerDirection::down;
  _isPlayerMoving = false;

  // create rendering components
  initResources(renderer, width, height);
}

void Scene::initResources(RendererContext& renderer, uint32_t width, uint32_t height) {
  release();
  auto sharedRenderer = renderer.renderer();
  _renderer = &renderer;
  if (_status != Status::finished)
    _status = Status::updated;

  // order by depth position (background pixels first)
  float gameAreaOffset[2]{};
  __generateGrass(sharedRenderer, _resources);
  __generateHedge(sharedRenderer, _maze, gameAreaOffset, _resources);
  __generatePlayer(sharedRenderer, gameAreaOffset, _resources);
  __generateUi(sharedRenderer, _resources);

  // offset uniform buffers (to move entity locations)
  float worldOffset[4]{ 0.f,0.f,0.f,0.f };
  _resources.worldOffset = ImmutableBuffer(*sharedRenderer, BufferType::uniform, sizeof(float) * 4, worldOffset);
  _resources.playerOffset = StaticBuffer(*sharedRenderer, BufferType::uniform, sizeof(float)*4);
  _resources.playerOffsetStaging = StagingBuffer(*sharedRenderer, BufferType::uniform, sizeof(float)*4);
  _updatePlayerSprite(0.f);
}

void Scene::release() noexcept {
  if (_renderer != nullptr) {
    _resources.clear();
    _renderer = nullptr;
  }
}

// update player appearance (orientation / animation)
void Scene::_updatePlayerSprite(float animationCoord) {
  float playerLocation[4]{ (2.f/__BASE_WINDOW_WIDTH)*(float)_playerPosition.x,
                           (2.f/__BASE_WINDOW_HEIGHT)*(float)_playerPosition.y,
                           animationCoord,(float)_playerDirection*0.25f };
  _resources.playerOffsetStaging.write(playerLocation);
  _resources.playerOffset.copy(_resources.playerOffsetStaging);
}

// ---

// draw scene entities
// --> optimization: regroup meshes by texture (to limit resource changes).
// --> no depth buffer used: order polygons from farthest to closest.
void Scene::render() {
  if (_renderer != nullptr) {
    if (_status != Status::finished)
      _status = Status::none;
    auto renderer = _renderer->renderer();
    renderer->bindGraphicsPipeline(_resources.pipeline.handle());

    // 2D/UI entities
    ImageId curImage = (ImageId)-1;
    for (auto& entity : _resources.entities) {
      // bind entity texture + entity offset uniform
      if (entity.image != curImage) {
        curImage = entity.image;
        auto imageView = _resources.images[curImage].resourceView();
        renderer->bindFragmentTextures(0, &imageView, 1);

        if (entity.image == ImageId::player)
          renderer->bindVertexUniforms(0, _resources.playerOffset.handlePtr(), 1u);
        else
          renderer->bindVertexUniforms(0, _resources.worldOffset.handlePtr(), 1u);
      }
      // draw entity
      renderer->bindVertexArrayBuffer(0, entity.vertices.handle(), _resources.shaders.strideBytes);
      renderer->bindVertexIndexBuffer(entity.indices.handle(), VertexIndexFormat::r32_ui);
      renderer->drawIndexed(entity.indexCount);
    }
  }
}
