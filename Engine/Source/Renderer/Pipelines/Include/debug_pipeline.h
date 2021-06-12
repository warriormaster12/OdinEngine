#pragma once

#include "pipeline_manager.h"


class DebugPipeline : public RendererPipeline
{
public: 
    void Init() override;
    
    void Update() override;

    void Destroy() override;
};