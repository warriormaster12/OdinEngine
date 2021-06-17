#include "Include/render_object.h"
#include "Include/renderer.h"
#include "Include/material_manager.h"
#include "glm/glm.hpp"
#include "logger.h"
#include <memory>

#include <unordered_map>

#include "unordered_finder.h"


std::unordered_map<std::string, RenderObject> objects;
std::vector<std::string> objectOwnerNames;


void ObjectManager::Init()
{
    const int MAX_OBJECTS = 10000;
    Renderer::CreateShaderUniformBuffer("mesh buffer", false, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPUObjectData) * MAX_OBJECTS);
    Renderer::WriteShaderUniform("object data", "per object layout",0,false,"mesh buffer");

    //Renderer::PrepareIndirectDraw(1000);
}

void ObjectManager::PushObjectToQueue(RenderObject& object, const std::string& owner)
{
    if(FindUnorderedMap(owner, objects) == nullptr)
    {
        objects[owner];
        *FindUnorderedMap(owner, objects) = object;
        objectOwnerNames.push_back(owner);
    }
}

RenderObject* ObjectManager::GetRenderObject(const std::string& owner)
{
    return FindUnorderedMap(owner, objects);
}


void ObjectManager::RenderObjects(const bool& bindMaterials /*= true*/)
{
    if(objectOwnerNames.size() != 0)
    {
        std::vector<GPUObjectData> objectData;
        objectData.reserve(objectOwnerNames.size());
        for(auto& currentObject : objectOwnerNames)
        {
            auto& obj = *FindUnorderedMap(currentObject, objects);
            GPUObjectData objData;
            objData.modelMatrix = obj.transformMatrix;
            objectData.push_back(objData);
        }
        Renderer::UploadVectorUniformDataToShader("mesh buffer",objectData, false);  
        

        //batch draw calls
        std::vector<DrawCall> batch;

        std::shared_ptr<Mesh> pLastMesh;
        std::string* pLastMaterial = nullptr;

        // std::vector<uint32_t> objIndicesSize;

        // for(int i =0; i < objects.size(); i++)
        // {
        //     objIndicesSize.push_back(objects[i].p_mesh->indices.size());
        // }

        // Renderer::UploadIndirectDraw(objects.size(), objIndicesSize, 0);

        
        for (size_t i = 0; i < objectOwnerNames.size(); ++i)
        {
            auto& currentObject = *FindUnorderedMap(objectOwnerNames[i], objects);
            bool isSameMesh = currentObject.p_mesh == pLastMesh;
            bool isSameMaterial = &currentObject.material == pLastMaterial;

            if (i == 0 || !isSameMesh || !isSameMaterial) {
                DrawCall dc;
                dc.p_mesh = currentObject.p_mesh;
                dc.p_material = &currentObject.material;
                dc.transformMatrix = FindUnorderedMap(objectOwnerNames[0], objects)->transformMatrix;
                dc.index = i;
                dc.count = 1;
                dc.descriptorSetCount += 1;
                batch.push_back(dc);

                pLastMesh = currentObject.p_mesh;
                pLastMaterial = &currentObject.material;

            } else {
                ++batch.back().count;
            }
        }
        for(auto& currentDc : batch)
        {
            if(bindMaterials == true)
            {
                if(*currentDc.p_material == "cube")
                {
                    Renderer::BindShader("cube map");
                    Renderer::BindUniforms("camera cube data", 0, 0, true);
                    Renderer::BindUniforms("cube map texture", 1);

                    Renderer::BindVertexBuffer(currentDc.p_mesh->vertexBuffer);
                    Renderer::BindIndexBuffer(currentDc.p_mesh->indexBuffer);

                    Renderer::DrawIndexed(currentDc.p_mesh->indices, currentDc.index);
                }
                else {
                    if(MaterialManager::GetMaterial(*currentDc.p_material).GetTextures().size() != 0)
                    {
                        Renderer::BindShader("default textured world");
                    }
                    else 
                    {
                        Renderer::BindShader("default world");
                    }
                    MaterialManager::BindMaterial(*currentDc.p_material);
                    Renderer::BindUniforms("camera data", 0, 0, true);
                    Renderer::BindUniforms("object data", 1, MaterialManager::GetMaterial(*currentDc.p_material).offset);
                    Renderer::BindVertexBuffer(currentDc.p_mesh->vertexBuffer);
                    Renderer::BindIndexBuffer(currentDc.p_mesh->indexBuffer);

                    Renderer::DrawIndexed(currentDc.p_mesh->indices, currentDc.index);
                }
            }
            else {
                Renderer::BindVertexBuffer(currentDc.p_mesh->vertexBuffer);
                Renderer::BindIndexBuffer(currentDc.p_mesh->indexBuffer);

                Renderer::DrawIndexed(currentDc.p_mesh->indices, currentDc.index);
            }
            //Renderer::DrawIndexedIndirect(currentDc.count, currentDc.index);
        }
    }
}

void ObjectManager::Destroy()
{
    Renderer::RemoveAllocatedBuffer("mesh buffer", false);

    MaterialManager::DeleteAllMaterials();

    std::shared_ptr<Mesh> p_lastMesh;
    for(int i = 0; i < objectOwnerNames.size(); i++)
    {
        auto& currentObject = *FindUnorderedMap(objectOwnerNames[i], objects);
        if(p_lastMesh != currentObject.p_mesh)
        {
            currentObject.p_mesh->DestroyMesh();
            p_lastMesh = currentObject.p_mesh;
        }
    }
}

