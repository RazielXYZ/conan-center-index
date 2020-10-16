#include <tinyply.h>

#include <cstdint>
#include <vector>

struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct uint3 { uint32_t x, y, z; };
struct uint4 { uint32_t x, y, z, w; };

struct geometry {
  std::vector<float3> vertices;
  std::vector<float3> normals;
  std::vector<float2> texcoords;
  std::vector<uint3> triangles;
};

geometry make_cube_geometry() {
  geometry cube;

  const struct CubeVertex { float3 position, normal; float2 texCoord; } verts[] = {
    { { -1, -1, -1 },{ -1, 0, 0 },{ 0, 0 } }, { { -1, -1, +1 },{ -1, 0, 0 },{ 1, 0 } },
    { { -1, +1, +1 },{ -1, 0, 0 },{ 1, 1 } }, { { -1, +1, -1 },{ -1, 0, 0 },{ 0, 1 } },
    { { +1, -1, +1 },{ +1, 0, 0 },{ 0, 0 } }, { { +1, -1, -1 },{ +1, 0, 0 },{ 1, 0 } },
    { { +1, +1, -1 },{ +1, 0, 0 },{ 1, 1 } }, { { +1, +1, +1 },{ +1, 0, 0 },{ 0, 1 } },
    { { -1, -1, -1 },{ 0, -1, 0 },{ 0, 0 } }, { { +1, -1, -1 },{ 0, -1, 0 },{ 1, 0 } },
    { { +1, -1, +1 },{ 0, -1, 0 },{ 1, 1 } }, { { -1, -1, +1 },{ 0, -1, 0 },{ 0, 1 } },
    { { +1, +1, -1 },{ 0, +1, 0 },{ 0, 0 } }, { { -1, +1, -1 },{ 0, +1, 0 },{ 1, 0 } },
    { { -1, +1, +1 },{ 0, +1, 0 },{ 1, 1 } }, { { +1, +1, +1 },{ 0, +1, 0 },{ 0, 1 } },
    { { -1, -1, -1 },{ 0, 0, -1 },{ 0, 0 } }, { { -1, +1, -1 },{ 0, 0, -1 },{ 1, 0 } },
    { { +1, +1, -1 },{ 0, 0, -1 },{ 1, 1 } }, { { +1, -1, -1 },{ 0, 0, -1 },{ 0, 1 } },
    { { -1, +1, +1 },{ 0, 0, +1 },{ 0, 0 } }, { { -1, -1, +1 },{ 0, 0, +1 },{ 1, 0 } },
    { { +1, -1, +1 },{ 0, 0, +1 },{ 1, 1 } }, { { +1, +1, +1 },{ 0, 0, +1 },{ 0, 1 } }
  };

  std::vector<uint4> quads = {
    { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 8, 9, 10, 11 },
    { 12, 13, 14, 15 }, { 16, 17, 18, 19 }, { 20, 21, 22, 23 }
  };

  for (auto & q : quads) {
    cube.triangles.push_back({ q.x,q.y,q.z });
    cube.triangles.push_back({ q.x,q.z,q.w });
  }

  for (int i = 0; i < 24; ++i) {
    cube.vertices.push_back(verts[i].position);
    cube.normals.push_back(verts[i].normal);
    cube.texcoords.push_back(verts[i].texCoord);
  }

  return cube;
}

int main() {
  geometry cube = make_cube_geometry();

  tinyply::PlyFile cube_file;

  cube_file.add_properties_to_element(
    "vertex", { "x", "y", "z" },
    tinyply::Type::FLOAT32, cube.vertices.size(),
    reinterpret_cast<uint8_t*>(cube.vertices.data()),
    tinyply::Type::INVALID, 0
  );

  cube_file.add_properties_to_element(
    "vertex", { "nx", "ny", "nz" },
    tinyply::Type::FLOAT32, cube.normals.size(),
    reinterpret_cast<uint8_t*>(cube.normals.data()),
    tinyply::Type::INVALID, 0
  );

  cube_file.add_properties_to_element(
    "vertex", { "u", "v" },
    tinyply::Type::FLOAT32, cube.texcoords.size() ,
    reinterpret_cast<uint8_t*>(cube.texcoords.data()),
    tinyply::Type::INVALID, 0
  );

  cube_file.add_properties_to_element(
    "face", { "vertex_indices" },
    tinyply::Type::UINT32, cube.triangles.size(),
    reinterpret_cast<uint8_t*>(cube.triangles.data()),
    tinyply::Type::UINT8, 3
  );

  cube_file.get_comments().push_back("generated by tinyply");

  return 0;
}
