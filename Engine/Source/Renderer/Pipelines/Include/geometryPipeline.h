#pragma once 

#include "pipelineManager.h"


class GeometryPipeline : public RendererPipeline
{
public: 
    void Init() override;
    
    void Update() override;

    void Destroy() override;
};