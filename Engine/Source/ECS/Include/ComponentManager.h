    #pragma once

    #include "ComponentArray.h"
    #include "Types.h"
    #include <any>
    #include <memory>
    #include <unordered_map>


    class ComponentManager
    {
    public:
    	template<typename T>
    	void RegisterComponent()
    	{
    		const char* typeName = typeid(T).name();

    		assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");

    		mComponentTypes.insert({typeName, mNextComponentType});
    		mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

    		++mNextComponentType;
    	}

    	template<typename T>
    	ComponentType GetComponentType()
    	{
    		const char* typeName = typeid(T).name();

    		assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

    		return mComponentTypes[typeName];
    	}

    	template<typename T>
    	void AddComponent(Entity entity, T component)
    	{
    		GetComponentArray<T>()->InsertData(entity, component);
    	}

    	template<typename T>
    	void RemoveComponent(Entity entity)
    	{
    		GetComponentArray<T>()->RemoveData(entity);
    	}

    	template<typename T>
    	T& GetComponent(Entity entity)
    	{
    		return GetComponentArray<T>()->GetData(entity);
    	}

    	void EntityDestroyed(Entity entity)
    	{
    		for (auto const& pair : mComponentArrays)
    		{
    			auto const& component = pair.second;

    			component->EntityDestroyed(entity);
    		}
    	}

    private:
    	std::unordered_map<const char*, ComponentType> mComponentTypes{};
    	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays{};
    	ComponentType mNextComponentType{};


    	template<typename T>
    	std::shared_ptr<ComponentArray<T>> GetComponentArray()
    	{
    		const char* typeName = typeid(T).name();

    		assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

    		return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
    	}
    };
