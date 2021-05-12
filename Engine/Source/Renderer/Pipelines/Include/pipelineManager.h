#pragma once 

#include <iostream>
#include <memory>

class RendererPipeline
{
public:
    virtual void Init() = 0;

    virtual void Update() = 0;

    virtual void Destroy() = 0;
};

class PipelineManager
{
public: 
    static void AddRendererPipeline(std::unique_ptr<RendererPipeline> inputPipeline);
    static void UpdateRendererPipelines();
    static void DestroyRendererPipelines();
};

