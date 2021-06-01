#include "Include/render_object.h"
#include "Include/renderer.h"
#include "Include/material_manager.h"
#include "glm/glm.hpp"
#include "logger.h"
#include <memory>

#include <glm/glm.hpp>


std::vector<RenderObject> objects;


void ObjectManager::Init()
{
    const int MAX_OBJECTS = 10000;
    Renderer::CreateShaderUniformBuffer("mesh buffer", false, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPUObjectData) * MAX_OBJECTS);
    Renderer::WriteShaderUniform("object data", "per object layout",0,false,"mesh buffer");

    //Renderer::PrepareIndirectDraw(1000);
}

void ObjectManager::PushObjectToQueue(RenderObject& object)
{
    objects.push_back(object);
}

void ObjectManager::RenderObjects()
{
    if(objects.size() != 0)
    {
        std::vector<GPUObjectData> objectData;
        objectData.reserve(objects.size());
        for(const RenderObject& obj : objects)
        {
            GPUObjectData objData;
            objData.modelMatrix = obj.transformMatrix;
            objectData.push_back(objData);
        }
        Renderer::UploadVectorUniformDataToShader("mesh buffer",objectData, false);  
        

        //batch draw calls
        std::vector<DrawCall> batch;

        Mesh* pLastMesh = nullptr;
        std::string* pLastMaterial = nullptr;

        // std::vector<uint32_t> objIndicesSize;

        // for(int i =0; i < objects.size(); i++)
        // {
        //     objIndicesSize.push_back(objects[i].p_mesh->indices.size());
        // }

        // Renderer::UploadIndirectDraw(objects.size(), objIndicesSize, 0);

        
        for (size_t i = 0; i < objects.size(); ++i)
        {
            bool isSameMesh = objects[i].p_mesh == pLastMesh;
            bool isSameMaterial = &objects[i].material == pLastMaterial;

            if (i == 0 || !isSameMesh || !isSameMaterial) {
                DrawCall dc;
                dc.p_mesh = objects[i].p_mesh;
                dc.p_material = &objects[i].material;
                dc.transformMatrix = objects[0].transformMatrix;
                dc.index = i;
                dc.count = 1;
                dc.descriptorSetCount += 1;
                batch.push_back(dc);

                pLastMesh = objects[i].p_mesh;
                pLastMaterial = &objects[i].material;

            } else {
                ++batch.back().count;
            }
        }
        for(auto& currentDc : batch)
        {
            if(*currentDc.p_material == "cube")
            {
                Renderer::BindShader("cube map");
                Renderer::BindUniforms("cube camera", 0, 0, true);
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
            //Renderer::DrawIndexedIndirect(currentDc.count, currentDc.index);
        }
    }
}

void ObjectManager::Destroy()
{
    Renderer::RemoveAllocatedBuffer("mesh buffer", false);

    MaterialManager::DeleteAllMaterials();

    Mesh* p_lastMesh = nullptr;
    for(int i = 0; i < objects.size(); i++)
    {
        if(p_lastMesh != objects[i].p_mesh)
        {
            objects[i].p_mesh->DestroyMesh();
            p_lastMesh = objects[i].p_mesh;
        }
    }
}

