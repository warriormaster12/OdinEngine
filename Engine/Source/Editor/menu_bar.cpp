#include "Include/menu_bar.h"

#include "imgui.h"

#include "vk_swapchain.h"

#include "logger.h"


#include "window_handler.h"


#include "Include/mesh_adder.h"
#include "Include/material_editor.h"
#include "Include/light_adder.h"


void MenuBar::ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        static bool showAbout = false; 
        static bool showDemo = false; 
        static bool showMeshAdder = false;
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
            if(ImGui::BeginMenu("Add Window"))
            {
                
                ImGui::MenuItem("Inspector");
                ImGui::MenuItem("Scene Graph", NULL, &showMeshAdder);
                ImGui::MenuItem("Content Browser");

                ImGui::EndMenu();
            }
            ImGui::EndMenu(); 
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("Show Demo Window",NULL, &showDemo);

            ImGui::MenuItem("About Engine",NULL, &showAbout);
            
            ImGui::EndMenu(); 
        }
        ImGui::EndMainMenuBar();
        
        if(showAbout)
        {
            ImGui::ShowAboutWindow();
        }
        if(showDemo)
        {
            ImGui::ShowDemoWindow();
        }
        if(showMeshAdder)
        {
            MeshAdder::ShowMeshAdderWindow();
            MaterialEditor::ShowMaterialWindow();
            LightAdder::ShowLightAdderWindow();
        }
    }
    
}