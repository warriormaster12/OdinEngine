#include "Include/renderer_core.h"

VulkanRenderer vkRenderer;
WindowHandler windowHandler;
auto start = std::chrono::system_clock::now();
auto end = std::chrono::system_clock::now();
float deltaTime;

std::vector<RenderObject> renderables;
std::unordered_map<std::string, Mesh> meshes;

void RendererCore::InitRenderer()
{
    windowHandler.CreateWindow(1920, 1080);
    vkRenderer.Init(windowHandler);
	CreateMaterial("texturedmesh2");
	CreateMaterial("texturedmesh");
	CreateMaterial("texturedmesh3");
	CreateMaterial("DamagedHelmetMat");
	CreateMaterial("BarrelMat");
	vkRenderer.InitScene();
    InitScene();
}

void RendererCore::UpdateRenderer()
{
    end = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsed_seconds = end - start;
    deltaTime = elapsed_seconds.count() * 1000.f;
    start = std::chrono::system_clock::now();
   
    vkRenderer.camera.UpdateCamera(deltaTime);
    vkRenderer.BeginDraw();

    vkRenderer.DrawObjects(renderables);
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkRenderer.EndDraw();
}

void RendererCore::RendererEvents()
{
    vkRenderer.camera.ProcessInputEvent(windowHandler);
    
}

void RendererCore::CleanupRenderer()
{
    vkRenderer.CleanUp();
    windowHandler.DestroyWindow();
}

void RendererCore::CreateMaterial(const std::string& name)
{
	vkRenderer.CreateMaterial(vkRenderer.GetMaterial("defaultMat")->pipeline, vkRenderer.GetMaterial("defaultMat")->pipelineLayout, name);
}

Material* RendererCore::GetMaterial(const std::string& name)
{
	return vkRenderer.GetMaterial(name);
}

Mesh* RendererCore::GetMesh(const std::string& name)
{
	auto it = meshes.find(name);
	if (it == meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

WindowHandler RendererCore::GetWindowHandler()
{
	return windowHandler;
}


void RendererCore::LoadMeshes()
{
	Mesh cubeMesh{};
	//make the array 3 vertices long
	cubeMesh.vertices.resize(105);

	//vertex positions
	cubeMesh.vertices[0].position = {  -0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[1].position = {  0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[2].position = {  0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[3].position = {  0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[4].position = { -0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[5].position = {  -0.5f, -0.5f, -0.5f};

	cubeMesh.vertices[6].position = {-0.5f, -0.5f,  0.5f};  
	cubeMesh.vertices[7].position = {0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[8].position = {0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[9].position = {0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[10].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[11].position = {-0.5f, -0.5f,  0.5f};

	cubeMesh.vertices[12].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[13].position = {-0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[14].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[15].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[16].position = {-0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[17].position = {-0.5f,  0.5f,  0.5f};

	cubeMesh.vertices[18].position = {0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[19].position = {0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[20].position = {0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[21].position = {0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[22].position = {0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[23].position = {0.5f,  0.5f,  0.5f};

	cubeMesh.vertices[24].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[25].position = {0.5f, -0.5f, -0.5f};
	cubeMesh.vertices[26].position = {0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[27].position = {0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[28].position = {-0.5f, -0.5f,  0.5f};
	cubeMesh.vertices[29].position = {-0.5f, -0.5f, -0.5f};

	cubeMesh.vertices[30].position = {-0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[31].position = {0.5f,  0.5f, -0.5f};
	cubeMesh.vertices[32].position = {0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[33].position = {0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[34].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh.vertices[35].position = {-0.5f,  0.5f, -0.5f};

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
	


	//load the monkey
	Mesh monkeyMesh{};
	Mesh lostEmpire{};
	Mesh viking_room{};
	Mesh DamagedHelmet{};
	Mesh Barrel{};
	monkeyMesh.LoadFromObj("EngineAssets/Meshes/monkey_smooth.obj");
	lostEmpire.LoadFromObj("EngineAssets/Meshes/lost_empire.obj");
	viking_room.LoadFromObj("EngineAssets/Meshes/viking_room.obj");
	DamagedHelmet.LoadFromObj("EngineAssets/DamagedHelmet/DamagedHelmet.obj");
	Barrel.LoadFromObj("EngineAssets/Meshes/Barrel.obj");


	//UploadMesh(triMesh);
	vkRenderer.UploadMesh(monkeyMesh);
	vkRenderer.UploadMesh(lostEmpire);
	vkRenderer.UploadMesh(cubeMesh);
	vkRenderer.UploadMesh(viking_room);
	vkRenderer.UploadMesh(DamagedHelmet);
	vkRenderer.UploadMesh(Barrel);

	meshes["monkey"] = monkeyMesh;
	meshes["skyBox"] = cubeMesh;	
	meshes["empire"] = lostEmpire;
	meshes["viking_room"] = viking_room;
	meshes["DamagedHelmet"] = DamagedHelmet;
	meshes["Barrel"] = Barrel;
}

void RendererCore::InitScene()
{
    LoadMeshes();
	GetMaterial("texturedmesh")->albedo = glm::vec4(1.0f);
	GetMaterial("texturedmesh")->metallic = 0.5f;
	GetMaterial("texturedmesh")->roughness = 0.5f;
	GetMaterial("texturedmesh")->ao = 1.0f;
	GetMaterial("texturedmesh")->emissionColor = glm::vec4(0.0f,0.0f,0.0f,0.0f);
	GetMaterial("texturedmesh")->emissionPower = 1.0f;

	GetMaterial("texturedmesh2")->albedo = glm::vec4(0.0f,0.0f,0.0f,1.0f);
	GetMaterial("texturedmesh2")->metallic = 0.5f;
	GetMaterial("texturedmesh2")->roughness = 0.5f;
	GetMaterial("texturedmesh2")->ao = 1.0f;
	GetMaterial("texturedmesh2")->emissionColor = glm::vec4(0.0f,0.0f,0.0f,0.0f);
	GetMaterial("texturedmesh2")->emissionPower = 1.0f;

	GetMaterial("texturedmesh3")->albedo = glm::vec4(0.5f,0.5f,0.5f,1.0f);
	GetMaterial("texturedmesh3")->metallic = 0.5f;
	GetMaterial("texturedmesh3")->roughness = 0.5f;
	GetMaterial("texturedmesh3")->ao = 1.0f;
	GetMaterial("texturedmesh3")->emissionColor = glm::vec4(0.0f,0.0f,0.0f,0.0f);
	GetMaterial("texturedmesh3")->emissionPower = 1.0f;

	GetMaterial("DamagedHelmetMat")->albedo = glm::vec4(1.0f);
	GetMaterial("DamagedHelmetMat")->metallic = 1.0f;
	GetMaterial("DamagedHelmetMat")->roughness = 1.0f;
	GetMaterial("DamagedHelmetMat")->ao = 1.0f;
	GetMaterial("DamagedHelmetMat")->emissionColor = glm::vec4(1.0f);
	GetMaterial("DamagedHelmetMat")->emissionPower = 8.0f;

	GetMaterial("BarrelMat")->albedo = glm::vec4(1.0f);
	GetMaterial("BarrelMat")->metallic = 1.0f;
	GetMaterial("BarrelMat")->roughness = 1.0f;
	GetMaterial("BarrelMat")->ao = 1.0f;
	GetMaterial("BarrelMat")->emissionColor = glm::vec4(1.0f, 0.3f, 0.0f, 1.0f);
	GetMaterial("BarrelMat")->emissionPower = 8.0f;
	
	RenderObject monkey;
	monkey.p_mesh = GetMesh("monkey");
	monkey.p_material = GetMaterial("texturedmesh2");
	monkey.transformMatrix =  glm::translate(glm::vec3{ 0,0,0 });

	renderables.push_back(monkey);

	RenderObject monkey2;
	monkey2.p_mesh = GetMesh("monkey");
	monkey2.p_material = GetMaterial("texturedmesh2");
	monkey2.transformMatrix =  glm::translate(glm::vec3{ 3,2,0 });

	renderables.push_back(monkey2);

	RenderObject monkey3;
	monkey3.p_mesh = GetMesh("monkey");
	monkey3.p_material = GetMaterial("texturedmesh2");
	monkey3.transformMatrix =  glm::translate(glm::vec3{ -3,2,0 });

	renderables.push_back(monkey3);

	RenderObject map;
	map.p_mesh = GetMesh("empire");
	map.p_material = GetMaterial("texturedmesh");
	map.transformMatrix = glm::translate(glm::vec3{ 5,-12,0 }); 

	renderables.push_back(map);

	RenderObject viking_room;
	viking_room.p_mesh = GetMesh("viking_room");
	viking_room.p_material = GetMaterial("texturedmesh3");
	viking_room.transformMatrix = glm::translate(glm::vec3{ 0,1.0f,3.0f }); 

	renderables.push_back(viking_room);

	RenderObject DamagedHelmet;
	DamagedHelmet.p_mesh = GetMesh("DamagedHelmet");
	DamagedHelmet.p_material = GetMaterial("DamagedHelmetMat");
	DamagedHelmet.transformMatrix = glm::translate(glm::vec3{ 0,3.0f,-5.0f }); 

	renderables.push_back(DamagedHelmet);

	RenderObject Barrel;
	Barrel.p_mesh = GetMesh("Barrel");
	Barrel.p_material = GetMaterial("BarrelMat");
	Barrel.transformMatrix = glm::translate(glm::vec3{ 0,3.0f,5.0f }); 

	renderables.push_back(Barrel);
	
	RenderObject skyBox;
	skyBox.p_mesh = GetMesh("skyBox");
	skyBox.p_material = GetMaterial("texturedmesh2");
	glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(0, 2, 0));
	glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(1.0f, 1.0f, 1.0f));
	skyBox.transformMatrix = translation * scale;
	renderables.push_back(skyBox);
}