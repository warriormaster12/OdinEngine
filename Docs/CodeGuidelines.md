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
