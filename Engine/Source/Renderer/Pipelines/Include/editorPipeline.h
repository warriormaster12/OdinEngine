#pragma once

#include "pipelineManager.h"


class EditorPipeline : public RendererPipeline
{
public: 
    void Init() override;
    
    void Update() override;

    void Destroy() override;
};