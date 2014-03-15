#include <fstream>
#include <cassert>
#include "ObjReader.h"

using std::string;
using std::vector;
using std::ifstream;
using std::istream;

namespace ObjReader
{
   int MtlReader::ConvertFromFile(std::string fileName, MaterialMap* data)
   {
       ifstream in(fileName);
   
       if (!in.good()) return RESULT_PARSE_ERROR;

       return ParseMaterials(in, data);
   }

   int MtlReader::ParseMaterials(std::istream &in, MaterialMap *data)
   {
      string word;
      int result = RESULT_SUCCESS;
   
      while( !in.eof() && result == RESULT_SUCCESS )
      {
         in >> word;
         if (word.length() == 0) continue;
   
         if( word.compare("newmtl") == 0 )
         {
            result = ParseMaterial(in, data);
         }
         else if( word.c_str()[0] == '#') 
         {
            result = ParseComment(in);
         }
      }
      return result;
   }

   int MtlReader::ParseMaterial(std::istream &in, MaterialMap *data)
   {
      int result = RESULT_SUCCESS;
      string materialName, word;
      in >> materialName;
      Material mat;
      while( !in.eof() && result == RESULT_SUCCESS )
      {
         in >> word;
         if (word.length() == 0) continue;
   
         if( word.compare("Ka") == 0 )
         {
            result = ParseAmbient(in, &mat);
         }
         else if( word.compare("Kd") == 0 )
         {
            result = ParseDiffuse(in, &mat);
         }
         else if( word.compare("Ks") == 0 )
         {
            result = ParseSpecular(in, &mat);
         }
         else if( word.compare("Ns") == 0 )
         {
            result = ParseShininess(in, &mat);
         }
         else if( word.compare("map_Kd") == 0 )
         {
            result = ParseTexture(in, &mat);
         }
         else if( word.compare("newmtl") == 0 )
         {
            result = ParseMaterial(in, data);
         }
         else
         {
            result = ParseComment(in);
         }
      }
      (*data)[materialName] = mat;
      return result;
   }

   int MtlReader::ParseDiffuse(std::istream &in, Material *data)
   {
      in >> data->diffuse.r;
      in >> data->diffuse.g;
      in >> data->diffuse.b;

      return RESULT_SUCCESS;
   }

   int MtlReader::ParseAmbient(std::istream &in, Material *data)
   {
      in >> data->ambient.r;
      in >> data->ambient.g;
      in >> data->ambient.b;

      return RESULT_SUCCESS;
   }

   int MtlReader::ParseSpecular(std::istream &in, Material *data)
   {
      in >> data->specular.r;
      in >> data->specular.g;
      in >> data->specular.b;

      return RESULT_SUCCESS;
   }

   int MtlReader::ParseShininess(std::istream &in, Material *data)
   {
      in >> data->shininess;

      return RESULT_SUCCESS;
   }

   int MtlReader::ParseTexture(std::istream &in, Material *data)
   {
      in >> data->textureName;
      data->hasTexture = true;

      return RESULT_SUCCESS;
   }

   int MtlReader::ParseComment(istream &in)
   {
      char buffer[BUFFER_SIZE];
      in.getline(buffer, BUFFER_SIZE);
   
      return RESULT_SUCCESS;
   }

   int ObjReader::ConvertFromFile(string fileName, ObjData *data)
   {
       ifstream in(fileName);
   
       if (!in.good()) return RESULT_PARSE_ERROR;

       return ParseObject(in, data);
   }
   
   int ObjReader::ParseObject(istream &in, ObjData *data)
   {
      string word;
      string material;
      int result = RESULT_SUCCESS;
      Mesh mesh;
      bool parsingVertices = false;

      data->numVertices = 0;
      data->numUVCoordinates = 0;
      data->numNorms = 0;
   
      while( !in.eof() && result == RESULT_SUCCESS )
      {
         in >> word;
         if (in.eof()) break;
         if (!in.good()) return RESULT_PARSE_ERROR;

         if (word.length() == 0) continue;
   
         if( word.compare("v") == 0 )
         {
            if (!parsingVertices)
            {
               mesh.UsesTexture = data->matMap[mesh.materialName].hasTexture;
               if (mesh.faces.size() > 0) data->meshes.push_back(mesh);

               data->numVertices += static_cast<int>(mesh.verts.size());
               data->numUVCoordinates += static_cast<int>(mesh.uvs.size());
               data->numNorms += static_cast<int>(mesh.norms.size());
               parsingVertices = true;
               mesh.faces.clear();
               mesh.verts.clear();
               mesh.uvs.clear();
               mesh.norms.clear();
            }

            result = ParseVertices(in, &mesh);
            continue;
         }

         parsingVertices = false;
         
         if( word.compare("vt") == 0 )
         {
            result = ParseUV(in, &mesh);
         }
         else if( word.compare("vn") == 0 )
         {
            result = ParseNormals(in, &mesh);
         }
         else if( word.compare("usemtl") == 0 )
         {
            in >> material;
         }
         else if( word.compare("f") == 0 )
         {
            mesh.materialName = material;
            bool hasUV;
            result = ParseFace(in, &mesh, data, &hasUV);

            if( hasUV )
            {
               mesh.UsesTexture = true;
            }
         }
         else if( word.compare("mtllib") == 0 )
         {
            in >> word;
            result = MtlReader::ConvertFromFile(word, &data->matMap);
         }
         else if( word.c_str()[0] == '#') 
         {
            result = ParseComment(in);
         }
      }

      if (mesh.faces.size() > 0) data->meshes.push_back(mesh);
      data->numVertices += static_cast<int>(mesh.verts.size());
   
      return result;
   }
   
   int ObjReader::ParseComment(istream &in)
   {
      char buffer[BUFFER_SIZE];
      in.getline(buffer, BUFFER_SIZE);
   
      return RESULT_SUCCESS;
   }
   
   int ObjReader::ParseNormals(istream &in, Mesh *mesh)
   {
      float x, y, z;
      in >> x;
      in >> y;
      in >> z;
      mesh->norms.push_back(Normal(x, y, z));

      return RESULT_SUCCESS;
   }

   int ObjReader::ParseUV(istream &in, Mesh *mesh)
   {
      float u, v;
      in >> u;
      in >> v;
      mesh->uvs.push_back(UV(u, v));

      return RESULT_SUCCESS;
   }

   int ObjReader::ParseFace(istream &in, Mesh *mesh, ObjData *data, bool *hasUV)
   {
      int v1, v2, v3;
      int vt1, vt2, vt3;
      int vn1, vn2, vn3;

      v1 = v2 = v3 = 0;
      vn1 = vn2 = vn3 = 0;
      vt1 = vt2 = vt3 = 0;

      static char line[BUFFER_SIZE];
      in.getline(line, BUFFER_SIZE);
      if (sscanf_s(line, " %d %d %d ", &v1, &v2, &v3) == 3)
      {
         mesh->faces.push_back(
            Face(v1 - 1 - data->numVertices, v2 - 1 - data->numVertices, v3 - 1 - data->numVertices));

         assert(false); // Need to calculate normals
         *hasUV = false;
      }
      else if (sscanf_s(line, " %d/%d %d/%d %d/%d ", &v1, &vt1, &v2, &vt2, &v3, &vt3) == 6)
      {
         Face face(v1 - 1 - data->numVertices, v2 - 1 - data->numVertices, v3 - 1 - data->numVertices);

         mesh->faces.push_back(face);
         mesh->verts[face.v1].uv = mesh->uvs[vt1 - 1 - data->numUVCoordinates];
         mesh->verts[face.v2].uv = mesh->uvs[vt2 - 1 - data->numUVCoordinates];
         mesh->verts[face.v3].uv = mesh->uvs[vt3 - 1 - data->numUVCoordinates];

         assert(false); // Need to calculate normals
         *hasUV = true;
      } 
      else if (sscanf_s(line, " %d/%d/%d %d/%d/%d %d/%d/%d ", &v1, &vt1, &vn1, 
                                                              &v2, &vt2, &vn2, 
                                                              &v3, &vt3, &vn3) == 9)
      {
         Face face(v1 - 1 - data->numVertices, v2 - 1 - data->numVertices, v3 - 1 - data->numVertices);

         mesh->faces.push_back(face);
         mesh->verts[face.v1].uv = mesh->uvs[vt1 - 1 - data->numUVCoordinates];
         mesh->verts[face.v2].uv = mesh->uvs[vt2 - 1 - data->numUVCoordinates];
         mesh->verts[face.v3].uv = mesh->uvs[vt3 - 1 - data->numUVCoordinates];
         
         mesh->verts[face.v1].norm = mesh->norms[vn1 - 1 - data->numNorms];
         mesh->verts[face.v2].norm = mesh->norms[vn2 - 1 - data->numNorms];
         mesh->verts[face.v3].norm = mesh->norms[vn3 - 1 - data->numNorms];
         *hasUV = true;
      }
      else if( sscanf_s(line, " %d//%d %d//%d %d//%d ", &v1, &vn1, 
                                                       &v2, &vn2, 
                                                       &v3, &vn3) == 6)
      {
         Face face(v1 - 1 - data->numVertices, v2 - 1 - data->numVertices, v3 - 1 - data->numVertices);

         mesh->faces.push_back(face);
         
         mesh->verts[face.v1].norm = mesh->norms[vn1 - 1 - data->numNorms];
         mesh->verts[face.v2].norm = mesh->norms[vn2 - 1 - data->numNorms];
         mesh->verts[face.v3].norm = mesh->norms[vn3 - 1 - data->numNorms];
         *hasUV = false;
      }
      else
      {
         assert(false);
         return RESULT_PARSE_ERROR;
      }
      return RESULT_SUCCESS;
   }
   
   int ObjReader::ParseVertices(istream &in, Mesh *mesh)
   {
      float x, y, z;
      in >> x;
      in >> y;
      in >> z;
      mesh->verts.push_back(Vertices(x, y, z));
   
      return RESULT_SUCCESS;
   }
}