#include "ComponentMgr.h"

void ComponentMgr::RegisterComponent(std::shared_ptr<Component> component)
{
	_components.push_back(component);
}

void ComponentMgr::RegisterComponents()
{
	// Actually register only ones that need update every tick

	g_gtaCommon = std::make_shared<GtaCommon>();
	g_gtaWindow = std::make_shared<GtaWindow>();
	g_gtaDirectX = std::make_shared<GtaDirectX>();
}

void ComponentMgr::UpdateAll() const
{
	for (auto& component : _components)
	{
		component->Update();
	}
}

ComponentMgr* g_componentMgr = ComponentMgr::GetInstance();

std::shared_ptr<GtaCommon> g_gtaCommon = nullptr;
std::shared_ptr<GtaWindow> g_gtaWindow = nullptr;
std::shared_ptr<GtaDirectX> g_gtaDirectX = nullptr;
