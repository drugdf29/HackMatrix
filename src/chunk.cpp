#include "chunk.h"

Chunk::Chunk() {
  data = make_unique<Cube* []>(size[0] * size[1] * size[2]);
  for(int i=0; i < size[0]*size[1]*size[2]; i++) {
    data[i] = NULL;
  }
}

Cube *Chunk::getCube(int x, int y, int z) {
  Cube *rv = data[index(x, y, z)];
  if(rv == NULL) {
    return &null;
  }
  return rv;
}

Cube *Chunk::getCube_(int x, int y, int z) {
  return data[index(x, y, z)];
}

void Chunk::removeCube(int x, int y, int z) {
  delete data[index(x, y, z)];
  data[index(x, y, z)] = NULL;
}
void Chunk::addCube(Cube c, int x, int y, int z) {
  data[index(x, y, z)] = new Cube(c);
}

ChunkMesh Chunk::mesh() {
  ChunkMesh rv;
  int x,y,z;
  int totalSize = size[0] * size[1] * size[2];
  ChunkCoords neighborCoords;
  Cube* neighbor;
  for(int i = 0; i<totalSize; i++) {
    if(data[i] != NULL) {
      ChunkCoords ci = getCoords(i);
      ChunkCoords neighbors[6] = {
        ChunkCoords{ci.x - 1, ci.y, ci.z},
        ChunkCoords{ci.x + 1, ci.y, ci.z},
        ChunkCoords{ci.x, ci.y - 1, ci.z},
        ChunkCoords{ci.x, ci.y + 1, ci.z},
        ChunkCoords{ci.x, ci.y, ci.z - 1},
        ChunkCoords{ci.x, ci.y, ci.z + 1}};

      for(int neighborIndex = 0; neighborIndex < 6; neighborIndex++) {
        neighborCoords = neighbors[neighborIndex];
        neighbor = getCube(neighborCoords.x, neighborCoords.y, neighborCoords.z);
      }
    }
  }
  return rv;
}

int Chunk::index(int x, int y, int z) {
  return x * size[1] * size[2] + y * size[2] + z;
}

ChunkCoords Chunk::getCoords(int index) {
  ChunkCoords rv;
  rv.z = index % size[2];
  index /= size[2];

  rv.y = index % size[1];
  index /= size[1];

  rv.x = index;
  return rv;
}

const vector<int> Chunk::getSize() {
  return size;
}
