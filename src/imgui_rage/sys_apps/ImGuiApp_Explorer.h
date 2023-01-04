#pragma once
#include "imgui_internal.h"
#include "../ImGuiApp.h"
#include "ImGuiApp_Toolbar.h"

namespace sapp
{
	class ImGuiApp_Explorer : public imgui_rage::ImGuiApp
	{
		int m_SelectedPackfileIndex = 1;
		int m_CurrentNodeIndex = 0;

		void DrawPackTreeNode(rage::fiPackfile* pack, const rage::fiPackEntry* entry)
		{
			static ImGuiTreeNodeFlags fileFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			const char* name = pack->GetEntryName(entry);

			// Unique ID for ImGui
			static char buffer[256];
			sprintf_s(buffer, 256, "%s##%s%i", name, name, m_CurrentNodeIndex++);

			if (entry->IsDirectory())
			{
				bool open = ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_SpanFullWidth);
				ImGui::TableNextColumn();
				ImGui::TextDisabled("--");
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Folder");
				if (open)
				{
					u32 endIndex = entry->Directory.ChildStartIndex + entry->Directory.ChildCount;
					for (u32 i = entry->Directory.ChildStartIndex; i < endIndex; i++)
						DrawPackTreeNode(pack, pack->GetEntry(i));
					ImGui::TreePop();
				}
			}
			else if (entry->IsResource())
			{
				ImGui::TreeNodeEx(buffer, fileFlags);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entry->GetOnDiskSize());
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Resource");
			}
			else
			{
				ImGui::TreeNodeEx(buffer, fileFlags);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entry->GetOnDiskSize());
				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Binary File");
			}
		}

		void DrawPackList()
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
			ImGui::TreeNodeEx("Grand Theft Auto V##PACK_TREE", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf);
			ImGui::BeginTable("PACK_TREE_TABLE", 3, flags);

			const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

			// The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
			ImGui::TableHeadersRow();

			m_CurrentNodeIndex = 0;
			// Skip 0 because its reserved for rage::pgRawStreamer
			for (u16 i = 1; i < rage::fiCollection::GetCollectionsSlotCount(); i++)
			{
				rage::fiPackfile* pack = rage::fiCollection::GetCollection(i);

				if (!pack)
					continue;

				if (pack->GetRootEntry() == nullptr)
					continue; // TODO: Happens on x64w.rpf; need investigation

				DrawPackTreeNode(pack, pack->GetRootEntry());
			}

			ImGui::EndTable(); // PACK_TREE_TABLE
			ImGui::TreePop(); // PACK_TREE

			//u16 count = rage::fiCollection::GetCollectionsSlotCount();
			//ImGui::ListBox("##EXPLORER_LIST", &m_SelectedPackfileIndex, count, [](int i) -> const char*
			//	{
			//		// Skip 0'th because it's reserved for rage::pgRawStreamer
			//		if (i == 0) return nullptr;

			//		rage::fiPackfile* pf = rage::fiCollection::GetCollection(i);

			//		// Pack file was unloaded? Can this even happen
			//		if (!pf) return nullptr;

			//		return pf->GetName();
			//	});
		}

		//void DrawPackInfo()
		//{
		//	rage::fiPackfile* pack = rage::fiCollection::GetCollection(m_SelectedPackfileIndex);

		//	ImGui::Text("Pack: %s", pack->GetName());

		//	auto root = pack->GetRootEntry();
		//	ImGui::Text("%s, %#010x", pack->GetEntryNameByOffset(root->nameOffset), root->flags);
		//}
	protected:
		void OnRender() override
		{
			ImGui::Begin("Explorer", &IsVisible);

			DrawPackList();
			//ImGui::SameLine();
			//ImGui::BeginChild("PACK_INFO", ImVec2(0, 0), true);
			//DrawPackInfo();
			//ImGui::EndChild(); // PACK_INFO

			ImGui::End(); // Explorer
		}
	};
}
