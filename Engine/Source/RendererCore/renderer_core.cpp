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
	


	//load the monkey
	Mesh monkeyMesh{};
	Mesh lostEmpire{};
	Mesh viking_room{};
	monkeyMesh.LoadFromObj("EngineAssets/Meshes/monkey_smooth.obj");
	lostEmpire.LoadFromObj("EngineAssets/Meshes/lost_empire.obj");
	viking_room.LoadFromObj("EngineAssets/Meshes/viking_room.obj");

	//UploadMesh(triMesh);
	vkRenderer.UploadMesh(monkeyMesh);
	vkRenderer.UploadMesh(lostEmpire);
	vkRenderer.UploadMesh(cubeMesh);
	vkRenderer.UploadMesh(viking_room);

	meshes["monkey"] = monkeyMesh;
	meshes["skyBox"] = cubeMesh;	
	meshes["empire"] = lostEmpire;
	meshes["viking_room"] = viking_room;
}

void RendererCore::InitScene()
{
    LoadMeshes();
	RenderObject monkey;
	monkey.p_mesh = GetMesh("monkey");
	monkey.p_material = vkRenderer.GetMaterial("texturedmesh2");
	monkey.transformMatrix = glm::mat4{ 1.0f };

	renderables.push_back(monkey);

	RenderObject map;
	map.p_mesh = GetMesh("empire");
	map.p_material = vkRenderer.GetMaterial("texturedmesh");
	map.transformMatrix = glm::translate(glm::vec3{ 5,-12,0 }); 

	renderables.push_back(map);

	RenderObject viking_room;
	viking_room.p_mesh = GetMesh("viking_room");
	viking_room.p_material = vkRenderer.GetMaterial("texturedmesh2");
	viking_room.transformMatrix = glm::translate(glm::vec3{ 0,0,2.0f }); 

	renderables.push_back(viking_room);
	
	RenderObject skyBox;
	skyBox.p_mesh = GetMesh("skyBox");
	skyBox.p_material = vkRenderer.GetMaterial("texturedmesh2");
	glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(0, 2, 0));
	glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(1.0f, 1.0f, 1.0f));
	skyBox.transformMatrix = translation * scale;
	renderables.push_back(skyBox);
}