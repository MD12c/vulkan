#include "Model.h"

#include <iostream>

void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    std::cout << "Loading model from: " << path << std::endl;

    directory = path.substr(0, path.find_last_of('/'));
    fileType  = path.substr(path.find_last_of('.') + 1);

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex>  vertices(mesh->mNumVertices);
    std::vector<GLuint>  indices(mesh->mNumFaces * 3);
    // int                  materialID;

    // Vertices
    {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            // positions
            vertices[i].position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z);

            // normals
            if (mesh->HasNormals())
            {
                vertices[i].normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z);
            }
            else
                vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);

            // texture coords
            if (mesh->mTextureCoords[0])
            {
                vertices[i].texUV = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y);
            }
            else
                vertices[i].texUV = glm::vec2(0.0f, 0.0f);
        }
    }

    // indices
    {
        unsigned int         index = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < 3; j++)
                indices[index++] = face.mIndices[j];
        }
    }

    // Tangents
    {
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            Vertex& v0 = vertices[indices[i]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1    = v1.position - v0.position;
            glm::vec3 edge2    = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.texUV - v0.texUV;
            glm::vec2 deltaUV2 = v2.texUV - v0.texUV;

            float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            if (std::abs(denom) < 1e-8f)
                continue;

            float     invDet  = 1.0f / denom;
            glm::vec3 tangent = invDet * (deltaUV2.y * edge1 - deltaUV1.y * edge2);

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }
    }

    // Normalize tangents
    {
        for (auto& v : vertices)
        {
            if (glm::length(v.tangent) > 1e-8f)
                v.tangent = glm::normalize(v.tangent);
            else
            {
                glm::vec3 up = (abs(v.normal.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
                v.tangent    = glm::normalize(glm::cross(up, v.normal));
            }
        }
    }

    // materials / textures
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        auto findPath = [&](aiTextureType type) -> std::string
        {
            aiString relPath;
            bool     success = (material->GetTexture(type, 0, &relPath) == AI_SUCCESS);
            return (relPath.length && success) ? directory + "/" + relPath.C_Str() : "";
        };

        // for (int i = aiTextureType_DIFFUSE; i <= aiTextureType_GLTF_METALLIC_ROUGHNESS; i++)
        // {
        //     aiTextureType type = static_cast<aiTextureType>(i);
        //     std::string   path = findPath(type);
        //     if (path == "")
        //         continue;

        //     const char* name = aiTextureTypeToString(type);
        //     std::cout << name << ": " << path << std::endl;
        // }

        // materialID = MaterialManager::LoadMaterialSpecular(
        //     std::string(material->GetName().C_Str()),
        //     glm::vec3(0.7f), glm::vec3(0.5f), 16,
        //     findPath(aiTextureType_DIFFUSE),
        //     findPath(aiTextureType_SPECULAR),
        //     findPath(aiTextureType_NORMALS),
        //     findPath(aiTextureType_DISPLACEMENT));

        // if (fileType == "gltf" || fileType == "glb")
        //     materialID = MaterialManager::LoadMaterialPBRgltf(
        //         std::string(material->GetName().C_Str()),
        //         0.85f, 0.0f,
        //         findPath(aiTextureType_DIFFUSE),
        //         findPath(aiTextureType_AMBIENT_OCCLUSION),
        //         findPath(aiTextureType_GLTF_METALLIC_ROUGHNESS),
        //         findPath(aiTextureType_NORMALS),
        //         findPath(aiTextureType_DISPLACEMENT));
        // else
        //     materialID = MaterialManager::LoadMaterialPBRobj(
        //         std::string(material->GetName().C_Str()),
        //         0.85f, 0.0f,
        //         findPath(aiTextureType_DIFFUSE),
        //         "",
        //         findPath(aiTextureType_DIFFUSE_ROUGHNESS),
        //         findPath(aiTextureType_METALNESS),
        //         findPath(aiTextureType_NORMALS),
        //         findPath(aiTextureType_DISPLACEMENT));
    }

    // std::cout << "mesh verts: " << mesh->mNumVertices << ", faces: " << mesh->mNumFaces << std::endl;
    return std::move(Mesh(device, vertices, indices));
}

// void Model::setCustomMaterial(MaterialID materialID)
// {
//     for (auto& mesh : meshes)
//         mesh.materialID = materialID;
// }

// void Model::setMeshMetalicRoughness(int meshIndex, float metalic, float roughness)
// {
//     if (meshIndex == -1)
//     {
//         for (size_t i = 0; i < meshes.size(); i++)
//         {
//             PBRMaterial* pbr = dynamic_cast<PBRMaterial*>(&MaterialManager::getMatAt(meshes.at(i).materialID));
//             pbr->metalic     = metalic;
//             pbr->roughness   = roughness;
//         }
//     }
//     else
//     {
//         PBRMaterial* pbr = dynamic_cast<PBRMaterial*>(&MaterialManager::getMatAt(meshes.at(meshIndex).materialID));
//         pbr->metalic     = metalic;
//         pbr->roughness   = roughness;
//     }
// }