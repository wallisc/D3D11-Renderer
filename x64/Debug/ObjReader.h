#ifndef OBJ_READER_H
#define OBJ_READER_H

#include <vector>
#include <map>
#include <string>
#include <istream>

namespace ObjReader
{
   typedef struct UV 
   {
      float u, v;
      UV() : u(0.0f), v(0.0f) {}
      UV(float nU, float nV) : u(nU), v(nV) {}
   } UV;
   
   typedef struct Normal 
   {
      float x, y, z;
      Normal() : x(0), y(0), z(0) {}
      Normal(float nX, float nY, float nZ) : x(nX), y(nY), z(nZ) {}
   } Normal;

   typedef struct Vertices
   {
      float x, y, z;
      UV uv;
      Normal norm;

      Vertices(float nX, float nY, float nZ) : x(nX), y(nY), z(nZ) {}
   } Vertices;
   
   typedef struct Face 
   {
      int v1, v2, v3;
      Face(int nV1, int nV2, int nV3) : v1(nV1), v2(nV2), v3(nV3) {}
   } Face;

   typedef struct Color
   {
      float r, g, b;
      Color(float nR, float nG, float nB) : r(nR), g(nG), b(nB) {}
   } Color;
   
   typedef struct Material
   {
      Color ambient;
      Color diffuse;
      Color specular;
      float shininess;
      bool hasTexture;
      std::string textureName;

      Material() : ambient(0.2f, 0.2f, 0.2f), diffuse(0.8f, 0.8f, 0.8f), specular(0.0f, 0.0f, 0.0f), shininess(0.0f), 
         hasTexture(false) {}

      Material(Color nAmbient, Color nDiffuse, Color nSpecular, float nShininess) :
         ambient(nAmbient), diffuse(nDiffuse), specular(nSpecular), shininess(nShininess), hasTexture(false) {}

      Material(Color nAmbient, Color nDiffuse, Color nSpecular, float nShininess, std::string nTextureName) :
         ambient(nAmbient), diffuse(nDiffuse), specular(nSpecular), shininess(nShininess), hasTexture(true), 
         textureName(nTextureName) {}
   } Material;

   typedef std::map<std::string, Material> MaterialMap;

   typedef struct Mesh
   {
      std::string materialName;
      std::vector<Vertices> verts; 
      std::vector<Face> faces;
      std::vector<UV> uvs;
      std::vector<Normal> norms;
      bool UsesTexture;

      Mesh() : UsesTexture(false) {}
   } Mesh;

   typedef struct ObjData
   {
      MaterialMap matMap;
      std::vector<Mesh> meshes;
      int numVertices;
      int numUVCoordinates;
      int numNorms;
   } ObjData;


   static const int RESULT_SUCCESS = 0;
   static const int RESULT_PARSE_ERROR = 1;

   static const int BUFFER_SIZE = 200;

   class MtlReader
   {
   public:
      static int ConvertFromFile(std::string fileName, MaterialMap* data);
   
   private:
      static int ParseMaterials(std::istream &in, MaterialMap *data);
      static int ParseMaterial(std::istream &in, MaterialMap *data);
      static int ParseComment(std::istream &in);

      static int ParseDiffuse(std::istream &in, Material *data);
      static int ParseAmbient(std::istream &in, Material *data);
      static int ParseSpecular(std::istream &in, Material *data);
      static int ParseShininess(std::istream &in, Material *data);
      static int ParseTexture(std::istream &in, Material *data);
   };
   
   class ObjReader
   {
   public:
      static int ConvertFromFile(std::string fileName, ObjData *data);
   
   private:
      static int ParseNormals(std::istream &in, Mesh *mesh);
      static int ParseMaterial(std::istream &in, std::string *materialName);
      static int ParseObject(std::istream &in, ObjData *data);
      static bool IsLineWhitespace(std::string);
      static int ParseVertices(std::istream &in, Mesh *mesh);
      static int ParseUV(std::istream &in, Mesh *mesh);
      static int ParseFace(std::istream &in, Mesh *mesh, ObjData *data, bool *hasUV);
      static int ParseComment(std::istream &in);
   
   };
}
#endif //OBJ_READER_H