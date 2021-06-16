#pragma once 

#include <iostream>
#include <filesystem>

#include <string>
#include <vector>


struct Project 
{
    std::string fullPath; 
    std::vector<std::string> supportedMeshExtensions = {".obj"};
    std::vector<std::string> supportedImageExtensions = {".jpg", ".png"};
};
class ProjectManager 
{
public:
    static void CreateProject(const std::string& projectName, const std::string& directory);
    static std::vector<std::string>& ListMeshes();
    static std::string& GetMesh(const std::string& name);

};