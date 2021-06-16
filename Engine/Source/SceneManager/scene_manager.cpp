#include "Include/scene_manager.h"

#include <unordered_map>
#include <vector>
#include "unordered_finder.h"

static std::unordered_map<std::string, Scene> scenes;
static std::vector<std::string> sceneNames;

void SceneManager::CreateScene(const std::string& name)
{
    scenes[name];
    sceneNames.push_back(name);
}

Scene& SceneManager::GetScene(const std::string& name)
{
    return *FindUnorderedMap(name, scenes);
}