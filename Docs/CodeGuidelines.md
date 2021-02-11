# General 
this file establishes rules on how to write code for the project. This is a good set of rules to know if you want to contribute to the project. 

## Variables
```
float deltaTime;

bool isWalking;

VkCommandBuffer cmd; 
```
### Pointers
```
void* p_examplePointer;

Material* pMat;
```
### Member variables
```
float m_delta;

RenderObject mObject;
```


## Functions 
```
void InitScene();
```

## Classes
```
class Player 
{
public:
   float positionX;
   float positionY;

   void PublicFunctionExample(function arguments);
private:
   void ExampleFunction();
   bool isDead;
};
```
## Files
```
vk_renderer.h 

vk_renderer.cpp
```

## Folders
```
Engine
Source 
Third-Party
```
