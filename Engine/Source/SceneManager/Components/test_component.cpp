#include "Include/test_component.h"

#include "logger.h"

static float pos;

void Test::Start()
{
    ENGINE_CORE_INFO("this component has been activated");
}

void Test::Update(const float& deltaTime)
{
    pos ++;
    pos = pos * deltaTime;

    ENGINE_CORE_INFO(pos);
}

void Test::Destroy()
{

}
