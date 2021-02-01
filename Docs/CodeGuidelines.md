# General 
this file establishes rules on how to write code for the project. This is a good set of rules to know if you want to contribute to the project. 

## Variables
### public 
```
float _deltaTime;
```
### private and local 
```
bool isWalking;

VkCommandBuffer cmd; 
```

## Functions 
```
void init_scene();
```

## Classes
```
class Player 
{
public:
   float _positionX;
   float _positionY;

   void public_function_example(function arguments);
private:
   void example_function();
   bool isDead;
};
``` 
