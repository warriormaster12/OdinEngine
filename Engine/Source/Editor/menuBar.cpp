#include "Include/menuBar.h"

#include "imgui.h"

#include "vk_swapchain.h"

#include "logger.h"


#include "window_handler.h"


void MenuBar::ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        //checking what is the size of the menu bar
        //ENGINE_CORE_INFO("bar size, x = {0} y = {1}", ImGui::GetWindowSize().x,  ImGui::GetWindowSize().y);
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Project")) {}
            if (ImGui::MenuItem("Open Project")) {}
            if (ImGui::MenuItem("Save Project")) {}
            if (ImGui::MenuItem("Save Project As")) {}
            if (ImGui::MenuItem("Exit Application")) 
            {
                windowHandler.WindowClose();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu(); 
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About Engine"))
            {
            }
            ImGui::EndMenu(); 
        }
        ImGui::EndMainMenuBar();
    }
    
}