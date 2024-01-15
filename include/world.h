#ifndef __WORLD_H__
#define __WORLD_H__

#include "app.h"
#include "blocks.h"
#include "coreStructs.h"
#include "cube.h"
#include "chunk.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <octree/octree.h>
#include "logger.h"
#include <unordered_map>
#include <vector>
#include <queue>
#include <future>
#include <optional>
#include "loader.h"

class Renderer;

struct Line {
  glm::vec3 points[2];
  glm::vec3 color;
};

struct App {
  X11App* app;
  glm::vec3 position;
};

enum Action {
  PLACE_CUBE,
  REMOVE_CUBE,
  SELECT_CUBE,
  OPEN_SELECTION_CODE,
  LOG_BLOCK_TYPE
};

class World {
  std::shared_ptr<spdlog::logger> logger;
  Renderer *renderer = NULL;
  Camera *camera = NULL;
  // TODO: rm
  int gotItCount = 0;
  vector<Line> lines;
  std::unordered_map<glm::vec3, int> appCubes;
  std::vector<X11App*> apps;
  deque<deque<shared_ptr<Chunk>>> chunks;
  map<DIRECTION, deque<future<deque<shared_ptr<Chunk>>>>> preloadedChunks;
  int WORLD_SIZE = 7;
  int PRELOAD_SIZE = 3;
  int damageIndex = -1;
  mutex preloadMutex;
  bool isDamaged = false;
  glm::vec3 cameraToVoxelSpace(glm::vec3 cameraPosition);
  Cube *getCube(float x, float y, float z);
  const std::vector<Cube*> getCubes();
  const std::vector<Cube*> getCubes(int x1, int y1, int z1, int x2, int y2, int z2);
  void updateDamage(int index);
  queue<glm::vec3> availableAppPositions;
  void removeCube(WorldPosition position);
  // TODO: rm
  ChunkIndex getChunkIndex(int x, int z);
  // TODO: rm
  ChunkIndex playersChunkIndex();
  // TODO: rm
  ChunkIndex calculateMiddleIndex();
  // TODO: rm
  array<ChunkPosition,2> getNextPreloadedChunkPositions(DIRECTION direction, int nextPreloadCount, bool isInitial);
  // TODO: rm
  OrthoginalPreload orthoginalPreload(DIRECTION direction, preload::SIDE side);
  void loadNextPreloadedChunkDeque(DIRECTION direction, bool initial=false);
  void transferChunksToPreload(DIRECTION movementDirection, deque<shared_ptr<Chunk>> slice);
  ChunkIndex middleIndex;
  void initChunks();
  void initAppPositions();
  void initLogger();
  void loadChunksIfNeccissary();
  void initPreloadedChunks();
  void logCoordinates(array<Coordinate, 2> c, string label);

public : void tick();
  const float CUBE_SIZE = 0.1;
  World(Camera *camera, shared_ptr<blocks::TexturePack> texturePack, string minecraftFolder, bool debug = false);
  ~World();
  void attachRenderer(Renderer *renderer);
  float getViewDistanceForWindowSize(X11App *app);
  Loader *loader;

  X11App *getLookedAtApp();
  Position getLookedAtCube();
  const std::vector<glm::vec3> getAppCubes();
  glm::vec3 getAppPosition(X11App* app);

  void addCube(int x, int y, int z, int blockType);
  void addLine(Line line);
  void removeLine(Line line);
  void addApp(glm::vec3, X11App* app);
  void addApp(X11App* app);
  void removeApp(X11App* app);
  int getIndexOfApp(X11App* app);

  void refreshRendererCubes();
  void action(Action);
  glm::vec3 getAppPosition(int appIndex);
  vector<Line> getLines();

  void save(string filename);
  void load(string filename);
  void loadRegion(Coordinate regionCoordinate);
  void loadMinecraft();
  void initLoader(string folderName, shared_ptr<blocks::TexturePack> texturePack);
  void loadLatest();

  void mesh(bool realTime = true);
  ChunkMesh meshSelectedCube(Position position);
  shared_ptr<Chunk> getChunk(int chunkX, int chunkZ);
};

#endif
