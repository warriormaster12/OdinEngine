#include "Include/pipeline_manager.h"
#include "logger.h"

#include <memory>
#include <vector>



std::vector <std::shared_ptr<RendererPipeline>> rPipelines;

int currentIndex = -1;


void PipelineManager::AddRendererPipeline(std::unique_ptr<RendererPipeline> inputPipeline)
{
    rPipelines.push_back(std::move(inputPipeline));
    currentIndex += 1;

    rPipelines[currentIndex]->Init();
}

void PipelineManager::UpdateRendererPipelines()
{
    for(int i = 0; i < rPipelines.size() ; i++)
    {
        rPipelines[i]->Update();
    }
}

void PipelineManager::DestroyRendererPipelines()
{
    for(int i = rPipelines.size() - 1; i >= 0 ; i--)
    {
        rPipelines[i]->Destroy();
    }
}
