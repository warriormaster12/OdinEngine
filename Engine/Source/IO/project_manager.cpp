#include "Include/project_manager.h"
#include "logger.h"
#include "unordered_finder.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, Project> projects;

std::unordered_map<std::string, std::string> uniqueMeshId;
std::vector<std::string> meshList;

std::string currentProjectName; 

void ProjectManager::CreateProject(const std::string& projectName, const std::string& directory)
{
    if(FindUnorderedMap(projectName, projects) == nullptr)
    {
        projects[projectName];
        auto& cProject = *FindUnorderedMap(projectName, projects);

        if(!std::filesystem::exists(directory))
        {
            std::filesystem::create_directory(directory);
        }
        std::filesystem::path path = directory;
        cProject.fullPath = std::filesystem::absolute(directory);
        currentProjectName = projectName;

        
    }
    auto& cProject = *FindUnorderedMap(currentProjectName, projects);
    for(auto& p : std::filesystem::recursive_directory_iterator(cProject.fullPath))
    {
        if(p.path().extension() == cProject.supportedMeshExtensions[0] && FindUnorderedMap(p.path().stem().string(), uniqueMeshId) == nullptr)
        {
            //we store the name of a file as a unique id
            //uniqueMeshId will contain a full path to the file
            uniqueMeshId[p.path().stem().string()] = p.path().string();
            meshList.push_back(p.path().stem().string());
        }
    }
}


std::string& ProjectManager::GetMesh(const std::string& name)
{
    return *FindUnorderedMap(name, uniqueMeshId);
}

std::vector<std::string>& ProjectManager::ListMeshes()
{
    return meshList;
}