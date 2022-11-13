#pragma once

#include "Template/atSingleton.h"
#include "Component.h"
#include <memory>
#include <vector>
#include <format>
#include "Logger.h"
#include "Platform/GtaWindow.h"
#include "Graphics/GtaDirectX.h"

inline std::shared_ptr<GtaWindow> g_gtaWindow;
inline std::shared_ptr<GtaDirectX> g_gtaDirectX;

class ComponentMgr : public atSingleton<ComponentMgr>
{
	std::vector<std::shared_ptr<Component>> _components;
public:
	template<typename T>
	std::shared_ptr<T> RegisterComponent()
	{
		static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

		g_logger->Log(std::format("Registering component of type: {}", typeid(T).name()));

		auto component = std::make_shared<T>();
		component->Init();

		_components.push_back(component);

		return component;
	}

	void RegisterComponents()
	{
		g_gtaWindow = RegisterComponent<GtaWindow>();
		g_gtaDirectX = RegisterComponent<GtaDirectX>();
	}

	void UpdateAll() const
	{
		for (auto& component : _components)
		{
			component->Update();
		}
	}
};

extern ComponentMgr* g_componentMgr;
