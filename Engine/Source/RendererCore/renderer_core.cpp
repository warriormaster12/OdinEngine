#include "Include/renderer_core.h"
#include "Include/material_core.h"
#include "Include/mesh_core.h"

#include "EditorWindow.h"
#include "Imgui_layer.h"


VulkanRenderer vkRenderer;
WindowHandler windowHandler;

auto start = std::chrono::system_clock::now();
auto end = std::chrono::system_clock::now();
float deltaTime;
float timer;
float fps;

std::vector<RenderObject> shadowObjects;




void RendererCore::InitRenderer()
{
    windowHandler.CreateWindow(1920, 1080);
    vkRenderer.Init(windowHandler);
	imgui_layer::InitImguiLayer(vkRenderer);
    InitScene();
}

void RendererCore::UpdateRenderer()
{
    end = std::chrono::system_clock::now();
    using ms = std::chrono::duration<float, std::milli>;
    deltaTime = std::chrono::duration_cast<ms>(end - start).count();
	timer += deltaTime * 0.0001;
    start = std::chrono::system_clock::now();
    vkRenderer.GetCamera().UpdateCamera(deltaTime);
	//imgui_layer::UpdateUi();
	vkRenderer.GetOffscreen().updateLight(timer);
	vkRenderer.GetOffscreen().calculateCascades(vkRenderer.GetCamera());
    vkRenderer.BeginCommands();
	for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
	{
		vkRenderer.GetOffscreen().BeginOffscreenRenderpass(i);
		{
			
			vkRenderer.GetOffscreen().drawOffscreenShadows(shadowObjects, i);
		}
		vkRenderer.GetOffscreen().EndOffscreenRenderpass();
	}
	//In between commands we can specify in which renderPass we are going to draw
	vkRenderer.BeginRenderpass();
	{
		if(windowHandler.GetKInput(GLFW_KEY_TAB) == GLFW_PRESS)
		{
			vkRenderer.GetOffscreen().debugShadows(true);
		}
		vkRenderer.DrawObjects(RendererCore::GetRenderObjects());
		//Draw UI after drawing the 3D world
		//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkRenderer.GetCommandBuffer());
	}
	vkRenderer.EndRenderpass();
	vkRenderer.EndCommands();
}

void RendererCore::RendererEvents()
{
    vkRenderer.GetCamera().ProcessInputEvent(windowHandler);
    
}

void RendererCore::CleanupRenderer()
{
    vkRenderer.CleanUp();
    windowHandler.DestroyWindow();
}

WindowHandler RendererCore::GetWindowHandler()
{
	return windowHandler;
}


void RendererCore::InitScene()
{
    LoadMeshes();
	LoadMaterials();
	// Must be called after the load functions above
	LoadRenderables();
}

void RendererCore::LoadMeshes()
{
	Mesh cubeMesh{};
	//make the array 105 vertices long
	cubeMesh.vertices.resize(105);

	//vertex positions
	cubeMesh.vertices[0].position = {  -1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[1].position = {  -1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[2].position = {  1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[3].position = {  1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[4].position = {  1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[5].position = {  -1.0f,  1.0f, -1.0f};

	cubeMesh.vertices[6].position = {-1.0f, -1.0f,  1.0f};  
	cubeMesh.vertices[7].position = {-1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[8].position = {-1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[9].position = {-1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[10].position = {-1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[11].position = {-1.0f, -1.0f,  1.0f};

	cubeMesh.vertices[12].position = {1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[13].position = {1.0f, -1.0f,  1.0f};
	cubeMesh.vertices[14].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[15].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[16].position = {1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[17].position = {1.0f, -1.0f, -1.0f};

	cubeMesh.vertices[18].position = {-1.0f, -1.0f,  1.0f};
	cubeMesh.vertices[19].position = {-1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[20].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[21].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[22].position = {1.0f, -1.0f,  1.0f};
	cubeMesh.vertices[23].position = {-1.0f, -1.0f,  1.0f};

	cubeMesh.vertices[24].position = {-1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[25].position = {1.0f,  1.0f, -1.0f};
	cubeMesh.vertices[26].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[27].position = {1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[28].position = {-1.0f,  1.0f,  1.0f};
	cubeMesh.vertices[29].position = {-1.0f,  1.0f, -1.0f};

	cubeMesh.vertices[30].position = {-1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[31].position = {-1.0f, -1.0f,  1.0f};
	cubeMesh.vertices[32].position = { 1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[33].position = {1.0f, -1.0f, -1.0f};
	cubeMesh.vertices[34].position = {-1.0f, -1.0f,  1.0f};
	cubeMesh.vertices[35].position = {1.0f, -1.0f,  1.0f};

	//uv coords
	cubeMesh.vertices[0].uv = {0.0f, 0.0f};
	cubeMesh.vertices[1].uv = {1.0f, 0.0f};
	cubeMesh.vertices[2].uv = {1.0f, 1.0f};
	cubeMesh.vertices[3].uv = {1.0f, 1.0f};
	cubeMesh.vertices[4].uv = {0.0f, 1.0f};
	cubeMesh.vertices[5].uv = {0.0f, 0.0f};

	cubeMesh.vertices[6].uv = {0.0f, 0.0f};
	cubeMesh.vertices[7].uv = {1.0f, 0.0f};
	cubeMesh.vertices[8].uv = {1.0f, 1.0f};
	cubeMesh.vertices[9].uv = {1.0f, 1.0f};
	cubeMesh.vertices[10].uv = {0.0f, 1.0f};
	cubeMesh.vertices[11].uv = {0.0f, 0.0f};

	cubeMesh.vertices[12].uv = {1.0f, 0.0f};
	cubeMesh.vertices[13].uv = {1.0f, 1.0f};
	cubeMesh.vertices[14].uv = {0.0f, 1.0f};
	cubeMesh.vertices[15].uv = {0.0f, 1.0f};
	cubeMesh.vertices[16].uv = {0.0f, 0.0f};
	cubeMesh.vertices[17].uv = {1.0f, 0.0f};

	cubeMesh.vertices[18].uv = {1.0f, 0.0f};
	cubeMesh.vertices[19].uv = {1.0f, 1.0f};
	cubeMesh.vertices[20].uv = {0.0f, 1.0f};
	cubeMesh.vertices[21].uv = {0.0f, 1.0f};
	cubeMesh.vertices[22].uv = {0.0f, 0.0f};
	cubeMesh.vertices[23].uv = {1.0f, 0.0f};

	cubeMesh.vertices[24].uv = {0.0f, 1.0f};
	cubeMesh.vertices[25].uv = {1.0f, 1.0f};
	cubeMesh.vertices[26].uv = {1.0f, 0.0f};
	cubeMesh.vertices[27].uv = {1.0f, 0.0f};
	cubeMesh.vertices[28].uv = {0.0f, 0.0f};
	cubeMesh.vertices[29].uv = {0.0f, 1.0f};

	cubeMesh.vertices[30].uv = {0.0f, 1.0f};
	cubeMesh.vertices[31].uv = {1.0f, 1.0f};
	cubeMesh.vertices[32].uv = {1.0f, 0.0f};
	cubeMesh.vertices[33].uv = {1.0f, 0.0f};
	cubeMesh.vertices[34].uv = {0.0f, 0.0f};
	cubeMesh.vertices[35].uv = {0.0f, 1.0f};


	CreateMeshFromFile("monkey", "EngineAssets/Meshes/monkey_smooth.obj", vkRenderer);
	CreateMeshFromFile("viking_room", "EngineAssets/Meshes/viking_room.obj", vkRenderer);
	CreateMeshFromFile("empire", "EngineAssets/Meshes/Floor.obj", vkRenderer);
	CreateMeshFromFile("DamagedHelmet", "EngineAssets/DamagedHelmet/DamagedHelmet.obj", vkRenderer);
	CreateMeshFromFile("Barrel", "EngineAssets/Meshes/Barrel.obj", vkRenderer);
	CreateMesh("skyBox", cubeMesh, vkRenderer);
}

void RendererCore::LoadMaterials()
{
	//lost empire
	std::vector<std::string> lostEmpireTextures = {
		"EngineAssets/Textures/lost_empire-RGBA.png"
	};
	//viking room
	std::vector<std::string> vikingRoomTextures = {
		"EngineAssets/Textures/viking_room.png"
	};
	

	//DamagedHelmet
	std::vector<std::string> DamagedHelmetTextures = {
		"EngineAssets/DamagedHelmet/Default_albedo.jpg",
		"EngineAssets/DamagedHelmet/Default_normal.jpg",
		"EngineAssets/DamagedHelmet/Default_emissive.jpg",
		"EngineAssets/DamagedHelmet/Default_AO.jpg",
		"EngineAssets/DamagedHelmet/Default_metalRoughness.jpg",
	};
	//Barrel
	std::vector<std::string> BarrelTextures = {
		"EngineAssets/Textures/ExplosionBarrel Diffuse.png",
		"",
		"EngineAssets/Textures/ExplosionBarrel Emission.png",
		"",
		"",
		"EngineAssets/Textures/ExplosionBarrel Metallic.png",
		"EngineAssets/Textures/ExplosionBarrel Roughness.png",
	};

	CreateMaterial("texturedmesh2", vkRenderer);
	CreateMaterial("texturedmesh", vkRenderer);
	CreateMaterial("texturedmesh3", vkRenderer);
	CreateMaterial("DamagedHelmetMat", vkRenderer);
	CreateMaterial("BarrelMat", vkRenderer);

	// Materials have a default configuration. Here we do custom config
	GetMaterial("texturedmesh")->albedo = glm::vec4(1.0f);
	GetMaterial("texturedmesh")->metallic = 0.5f;
	GetMaterial("texturedmesh")->roughness = 0.5f;
	GetMaterial("texturedmesh")->ao = 1.0f;
	GetMaterial("texturedmesh")->emissionColor = glm::vec4(0.0f,0.0f,0.0f,0.0f);
	GetMaterial("texturedmesh")->emissionPower = 1.0f;
	//GetMaterial("texturedmesh")->textures = UpdateTextures("texturedmesh", lostEmpireTextures);

	GetMaterial("texturedmesh3")->albedo = glm::vec4(1.0f,1.0f,1.0f,1.0f);
	GetMaterial("texturedmesh3")->metallic = 0.5f;
	GetMaterial("texturedmesh3")->roughness = 0.5f;
	GetMaterial("texturedmesh3")->ao = 1.0f;
	GetMaterial("texturedmesh3")->emissionColor = glm::vec4(0.0f,0.0f,0.0f,0.0f);
	GetMaterial("texturedmesh3")->emissionPower = 1.0f;
	GetMaterial("texturedmesh3")->textures = UpdateTextures("texturedmesh3", vikingRoomTextures);

	GetMaterial("DamagedHelmetMat")->albedo = glm::vec4(1.0f);
	GetMaterial("DamagedHelmetMat")->metallic = 1.0f;
	GetMaterial("DamagedHelmetMat")->roughness = 0.5f;
	GetMaterial("DamagedHelmetMat")->ao = 1.0f;
	GetMaterial("DamagedHelmetMat")->emissionColor = glm::vec4(1.0f);
	GetMaterial("DamagedHelmetMat")->emissionPower = 8.0f;
	GetMaterial("DamagedHelmetMat")->textures = UpdateTextures("DamagedHelmetMat",DamagedHelmetTextures);

	GetMaterial("BarrelMat")->albedo = glm::vec4(1.0f);
	GetMaterial("BarrelMat")->metallic = 1.0f;
	GetMaterial("BarrelMat")->roughness = 1.0f;
	GetMaterial("BarrelMat")->ao = 1.0f;
	GetMaterial("BarrelMat")->emissionColor = glm::vec4(1.0f, 0.3f, 0.0f, 1.0f);
	GetMaterial("BarrelMat")->emissionPower = 8.0f;
	GetMaterial("BarrelMat")->textures = UpdateTextures("BarrelMat",BarrelTextures);

}




void RendererCore::LoadRenderables()
{

	RenderObject monkey;
	monkey.position = glm::vec3( 0,0,0 );
	monkey.p_mesh = GetMesh("monkey");
	monkey.p_material = GetMaterial("texturedmesh2");
	monkey.transformMatrix = glm::translate(monkey.position);
	RendererCore::CreateRenderObject(monkey);
	


	RenderObject monkey2;
	monkey2.position = glm::vec3( 3,2,0 );
	monkey2.p_mesh = GetMesh("monkey");
	monkey2.p_material = GetMaterial("texturedmesh2");
	monkey2.transformMatrix =  glm::translate(monkey2.position);

	RendererCore::CreateRenderObject(monkey2);

	RenderObject monkey3;
	monkey3.position = glm::vec3( -3,2,0 );
	monkey3.transformMatrix = glm::translate(monkey3.position);
	monkey3.p_mesh = GetMesh("monkey");
	monkey3.p_material = GetMaterial("texturedmesh2");

	RendererCore::CreateRenderObject(monkey3);

	RenderObject viking_room;
	viking_room.position = glm::vec3( 0,1.0f,3.0f );
	viking_room.p_mesh =  GetMesh("viking_room");
	viking_room.p_material = GetMaterial("texturedmesh3");
	viking_room.transformMatrix = glm::translate(viking_room.position); 

	RendererCore::CreateRenderObject(viking_room);


	RenderObject DamagedHelmet;
	DamagedHelmet.position = glm::vec3( 0,3.0f,-5.0f );
	DamagedHelmet.transformMatrix = glm::translate(DamagedHelmet.position);
	DamagedHelmet.p_mesh =  GetMesh("DamagedHelmet");
	DamagedHelmet.p_material = GetMaterial("DamagedHelmetMat"); 

	RendererCore::CreateRenderObject(DamagedHelmet);


	RenderObject Barrel;
	Barrel.position = glm::vec3(0,1.0f,10.0f);
	Barrel.transformMatrix = glm::translate(Barrel.position);
	Barrel.p_mesh =  GetMesh("Barrel");
	Barrel.p_material = GetMaterial("BarrelMat");  

	RendererCore::CreateRenderObject(Barrel);
	

	RenderObject map;
	map.position = glm::vec3( 5,-2,0 );
	map.transformMatrix = glm::translate(map.position); 
	map.p_mesh =  GetMesh("empire");
	map.p_material = GetMaterial("texturedmesh"); 
	RendererCore::CreateRenderObject(map); 
	for(size_t i = 0; i < RendererCore::GetRenderObjects().size(); i++)
	{
		shadowObjects.push_back(RendererCore::GetRenderObjects()[i]);
	}
	

	

	
	


	// RenderObject skyBox;
	// glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(0, 2, 0));
	// glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(1.0f, 1.0f, 1.0f));
	// skyBox.transformMatrix = translation * scale;
	// skyBox.p_mesh =  GetMesh("skyBox");
	// skyBox.p_material = GetMaterial("skyMat");  
	// RendererCore::CreateRenderObject(skyBox);

	
}
