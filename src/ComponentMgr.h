#pragma once

#include "Template/atSingleton.h"
#include "Component.h"
#include <memory>
#include <vector>
#include "GtaCommon.h"
#include "Platform/GtaWindow.h"
#include "Graphics/GtaDirectX.h"

class GtaDirectX;

class ComponentMgr : public atSingleton<ComponentMgr>
{
	std::vector<std::shared_ptr<Component>> _components;
public:
	void RegisterComponent(std::shared_ptr<Component> component);
	void RegisterComponents();
	void UpdateAll() const;
};

extern ComponentMgr* g_componentMgr;

extern std::shared_ptr<GtaCommon> g_gtaCommon;
extern std::shared_ptr<GtaWindow> g_gtaWindow;
extern std::shared_ptr<GtaDirectX> g_gtaDirectX;
