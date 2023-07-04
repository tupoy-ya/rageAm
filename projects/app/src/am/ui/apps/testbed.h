#pragma once

#include "imgui.h"
#include "implot.h"
#include "am/asset/factory.h"
#include "am/integration/hooks/streaming.h"
#include "am/integration/memory/hook.h"
#include "am/ui/app.h"
#include "am/ui/context.h"
#include "am/ui/font_icons/icons_am.h"
#include "rage/paging/builder/builder.h"
//#include "am/asset/factory.h"
//#include "am/desktop/window.h"
//#include "am/integration/hooks/streaming.h"
//#include "rage/grcore/texture/texturedict.h"
//#include "helpers/format.h"
//#include "rage/paging/builder/builder.h"
//#include "rage/paging/compiler/compiler.h"
//#include "rage/streaming/streamengine.h"
//#include "rage/streaming/streaming.h"

namespace rageam::ui
{
	ID3D11ShaderResourceView* (*gImpl_GetView_Hook)(rage::grcTextureDX11*);

	inline ID3D11ShaderResourceView* GetView_Hook(rage::grcTextureDX11* inst)
	{
		if (strcmp(inst->GetName(), "head_diff_000_a_whi") == 0)
		{
			int i = 0;
		}

		return gImpl_GetView_Hook(inst);
	}

	class TestbedApp : public App
	{
	public:
		void OnStart() override
		{
			//auto addr = gmAddress::Scan("48 8B 41 78 C3");
			//Hook::Create(addr, GetView_Hook, &gImpl_GetView_Hook, true);
		}

		void OnRender() override
		{
			return;
			//ImPlot::ShowDemoWindow();

			//ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_Always);
			ImGui::Begin("RageAm Testbed");
			//auto icon = Gui->Icons.GetIcon("pc", IconSize_256);
			//icon->Render(256, 256);

			//ImGui::Separator();

			//icon = Gui->Icons.GetIcon("pc", IconSize_16);
			//icon->Render(256, 256);
			
			/*auto module = hooks::Streaming::GetModule("ytd");
			rage::strLocalIndex slot;
			module->GetSlotIndexByName(slot, "head_diff_000_a_whi");

			if(slot >= 0)
			{
				ImGui::Text("head_diff_000_a_whi.ytd");
				ImGui::Text("Slot: %i, Data: %x",
					slot,
					module->GetResource(slot));
			}
			else
			{
				ImGui::Text("%i", slot);
			}*/

			//
			//static rage::pgTextureDictionary s_Txd;

			//auto module = hooks::Streaming::GetModule("ytd");
			//rage::strLocalIndex slot;
			//module->GetSlotIndexByName(slot, "adder");

			//// Streaming
			//{
			//	rage::strStreamingInfo* info = hooks::Streaming::GetInfo(module->GetGlobalIndex(slot));
			//	if (module && info)
			//	{

			//		ImGui::Text("adder.ytd");
			//		ImGui::Text("Slot: %i, Streamed: %s, Data: %x",
			//			slot,
			//			module->GetResource(slot) ? "True" : "False",
			//			info->Data);
			//	}
			//}

			//if (SlGui::Button("Stream"))
			//{
			//	auto txd = asset::AssetFactory::LoadFromPath<asset::TxdAsset>(
			//		"C:/Users/falco/Desktop/Assets/adder.itd");

			//	if (txd && txd->CompileToGame(&s_Txd))
			//	{
			//		module->Set(slot, &s_Txd);
			//		rage::strStreamingInfo* info = hooks::Streaming::GetInfo(module->GetGlobalIndex(slot));

			//		info->Data = 0x8000009; // Mark as loaded
			//	}
			//}
			//ImGui::SameLine();

			//if (SlGui::Button("Tests"))
			//{
			//	rage::atMap<ConstString, ConstString> test;
			//	test.InitAndAllocate(10000);
			//	test.Insert("Hello", "Hello");
			//}
			//ImGui::SameLine();
			//if (SlGui::Button("Compile Asset"))
			//{
			//	amPtr<asset::TxdAsset> txd = asset::AssetFactory::LoadFromPath<asset::TxdAsset>(
			//		"C:/Users/falco/Source/Repos/rageAm/rageAm/Resources/ranstar.pack/ranstar.itd");

			//	if (txd)
			//		txd->CompileToFile("C:/Users/falco/Source/Repos/rageAm/rageAm/Resources/ranstar.pack/ranstar.ytd");
			//}
			//ImGui::SameLine();
			//if (SlGui::Button("Create Asset"))
			//{
			//	asset::TxdAsset txd("C:/Users/falco/Source/Repos/rageAm/rageAm/Resources/ranstar.pack/ranstar.itd");
			//	txd.Refresh();
			//	txd.SaveConfig();
			//}

			ImGui::End();
		}
	};
}
