#include "ImGuiWrapper.h"
#include "../../Core/Config.h"
#include "../../Core/stdafx.h"
#include "../../IO/IOService.h"
#include "../../IO/JSONParser.h"
#include "../NodeDescriptorManager.h"
#include "../NodeCompiler.h"

#include <imgui_node_editor.h>
#include "ImGuiNodeUtil/Math2D.h"
#include "ImGuiNodeUtil/Builders.h"
#include "ImGuiNodeUtil/Widgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#if defined WS_OS_WIN
#include "ImGuiWindowWin.h"
#endif

#if defined WS_OS_MAC
#include "ImGuiWindowMac.h"
#endif

#if defined WS_OS_LINUX
#include "ImGuiWindowLinux.h"
#endif

#if defined WS_RENDERER_DIRECTX
#include "ImGuiRendererDX11.h"
#endif

#if defined WS_RENDERER_OPENGL
#include "ImGuiRendererGL.h"
#endif

#if defined WS_RENDERER_VULKAN
#include "ImGuiRendererVK.h"
#endif

#if defined WS_RENDERER_METAL
#include "ImGuiRendererMT.h"
#endif

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;
using namespace Waveless;

using ax::Widgets::IconType;

struct NodeWidget;

using PinValue = uint64_t;

struct PinWidget : public Object
{
	PinDescriptor* Desc;

	ed::PinId ID;
	uintptr_t OldID = 0;

	PinValue Value;
	NodeWidget* NodeWidget;

	PinWidget(int id, const char* name, PinType type) :
		ID(id), NodeWidget(nullptr), Value(0)
	{
	}
};

struct NodeWidget : public Object
{
	NodeDescriptor* Desc;

	ed::NodeId ID;
	uintptr_t OldID = 0;

	ImColor Color;
	ImVec2 Size;

	std::vector<PinWidget> Inputs;
	std::vector<PinWidget> Outputs;

	NodeWidget(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
		ID(id), Color(color), Size(0, 0)
	{
	}
};

struct LinkWidget : public Object
{
	ed::LinkId ID;
	uintptr_t OldID = 0;

	ImColor Color;

	PinWidget* StartPin = 0;
	PinWidget* EndPin = 0;

	LinkWidget(ed::LinkId id, PinWidget* startPin, PinWidget* endPin) :
		ID(id), StartPin(startPin), EndPin(endPin), Color(255, 255, 255)
	{
	}
};

struct NodeIdLess
{
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
	{
		return lhs.AsPointer() < rhs.AsPointer();
	}
};

namespace ImGuiWrapperNS
{
	bool m_isParity = true;

	IImGuiWindow* m_windowImpl;
	IImGuiRenderer* m_rendererImpl;
	ImFontAtlas fontAtlas;

	ed::Config config;
	ed::EditorContext* m_NodeEditorContext = nullptr;

	ed::NodeId contextNodeId = 0;
	ed::LinkId contextLinkId = 0;
	ed::PinId  contextPinId = 0;
	bool createNewNode = false;
	PinWidget* newNodeLinkPin = nullptr;
	PinWidget* newLinkPin = nullptr;

	const int s_PinIconSize = 24;
	std::vector<NodeWidget> s_Nodes;
	std::vector<PinWidget> s_InputPins;
	std::vector<PinWidget> s_OutputPins;
	std::vector<LinkWidget> s_Links;

	ImTextureID s_HeaderBackground = nullptr;

	const float s_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> s_NodeTouchTime;

	int s_NextId = 1;
}

using namespace ImGuiWrapperNS;

static inline ImRect ImGui_GetItemRect()
{
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
	auto result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

ImColor GetIconColor(PinType type)
{
	switch (type)
	{
	default:
	case PinType::Flow:     return ImColor(255, 255, 255);
	case PinType::Bool:     return ImColor(220, 48, 48);
	case PinType::Int:      return ImColor(68, 201, 156);
	case PinType::Float:    return ImColor(147, 226, 74);
	case PinType::String:   return ImColor(124, 21, 153);
	case PinType::Vector:   return ImColor(201, 201, 68);
	case PinType::Object:   return ImColor(51, 150, 215);
	case PinType::Function: return ImColor(218, 0, 183);
	case PinType::Delegate: return ImColor(255, 48, 48);
	}
};

void DrawPinIcon(const PinWidget& pin, bool connected, int alpha)
{
	IconType iconType;
	ImColor  color = GetIconColor(pin.Desc->Type);
	color.Value.w = alpha / 255.0f;
	switch (pin.Desc->Type)
	{
	case PinType::Flow:     iconType = IconType::Flow;   break;
	case PinType::Bool:     iconType = IconType::Circle; break;
	case PinType::Int:      iconType = IconType::Circle; break;
	case PinType::Float:    iconType = IconType::Circle; break;
	case PinType::String:   iconType = IconType::Circle; break;
	case PinType::Vector:   iconType = IconType::Circle; break;
	case PinType::Object:   iconType = IconType::Circle; break;
	case PinType::Function: iconType = IconType::Circle; break;
	case PinType::Delegate: iconType = IconType::Square; break;
	default:
		return;
	}

	ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
};

static int GetNextId()
{
	return s_NextId++;
}

static ed::LinkId GetNextLinkId()
{
	return ed::LinkId(GetNextId());
}

static void TouchNode(ed::NodeId id)
{
	s_NodeTouchTime[id] = s_TouchTime;
}

static float GetTouchProgress(ed::NodeId id)
{
	auto it = s_NodeTouchTime.find(id);
	if (it != s_NodeTouchTime.end() && it->second > 0.0f)
		return (s_TouchTime - it->second) / s_TouchTime;
	else
		return 0.0f;
}

static void UpdateTouch()
{
	const auto deltaTime = ImGui::GetIO().DeltaTime;
	for (auto& entry : s_NodeTouchTime)
	{
		if (entry.second > 0.0f)
			entry.second -= deltaTime;
	}
}

static NodeWidget* FindNode(ed::NodeId id)
{
	for (auto& node : s_Nodes)
		if (node.ID == id)
			return &node;

	return nullptr;
}

static LinkWidget* FindLink(ed::LinkId id)
{
	for (auto& link : s_Links)
		if (link.ID == id)
			return &link;

	return nullptr;
}

static PinWidget* FindPin(ed::PinId id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.Inputs)
			if (pin.ID == id)
				return &pin;

		for (auto& pin : node.Outputs)
			if (pin.ID == id)
				return &pin;
	}

	return nullptr;
}

static PinWidget* FindPinByOldID(uint64_t id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.Inputs)
			if (pin.OldID == id)
				return &pin;

		for (auto& pin : node.Outputs)
			if (pin.OldID == id)
				return &pin;
	}

	return nullptr;
}

static bool IsPinLinked(PinWidget* pin)
{
	if (pin == nullptr)
	{
		return false;
	}

	for (auto& link : s_Links)
	{
		if (link.StartPin == pin || link.EndPin == pin)
		{
			return true;
		}
	}

	return false;
}

static bool CanCreateLink(PinWidget* a, PinWidget* b)
{
	if (!a || !b || a == b || a->Desc->Kind == b->Desc->Kind || a->Desc->Type != b->Desc->Type || a->NodeWidget == b->NodeWidget)
		return false;

	return true;
}

static void BuildNode(NodeWidget* node)
{
	for (auto& input : node->Inputs)
	{
		input.NodeWidget = node;
	}

	for (auto& output : node->Outputs)
	{
		output.NodeWidget = node;
	}
}

NodeWidget* SpawnNode(const char* nodeDescriptorName)
{
	auto l_nodeDesc = NodeDescriptorManager::GetNodeDescriptor(nodeDescriptorName);

	s_Nodes.emplace_back(GetNextId(), l_nodeDesc->Name, ImColor(l_nodeDesc->Color[0], l_nodeDesc->Color[1], l_nodeDesc->Color[2]));
	s_Nodes.back().Desc = l_nodeDesc;

	for (int i = 0; i < l_nodeDesc->InputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->InputPinIndexOffset + i, PinKind::Input);
		s_Nodes.back().Inputs.emplace_back(GetNextId(), l_pinDesc->Name, l_pinDesc->Type);
		s_Nodes.back().Inputs.back().Desc = l_pinDesc;
	}
	for (int i = 0; i < l_nodeDesc->OutputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->OutputPinIndexOffset + i, PinKind::Output);
		s_Nodes.back().Outputs.emplace_back(GetNextId(), l_pinDesc->Name, l_pinDesc->Type);
		s_Nodes.back().Outputs.back().Desc = l_pinDesc;
	}

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
};

LinkWidget* SpawnLink(PinWidget* startPin, PinWidget* endPin)
{
	s_Links.emplace_back(LinkWidget(GetNextId(), startPin, endPin));
	s_Links.back().Color = GetIconColor(startPin->Desc->Type);

	return &s_Links.back();
}

static NodeWidget* SpawnComment()
{
	s_Nodes.emplace_back(GetNextId(), "New Comment");
	s_Nodes.back().Desc->Type = NodeType::Comment;
	s_Nodes.back().Size = ImVec2(300, 200);

	return &s_Nodes.back();
}

void BuildNodes()
{
	for (auto& node : s_Nodes)
	{
		BuildNode(&node);
	}
}

void LoadCanvas(const char* fileName)
{
	for (auto& node : s_Nodes)
	{
		node.Inputs.clear();
		node.Outputs.clear();
		node.Inputs.shrink_to_fit();
		node.Outputs.shrink_to_fit();

		ed::DeleteNode(node.ID);
	}
	for (auto& link : s_Links)
	{
		ed::DeleteLink(link.ID);
	}
	s_Nodes.clear();
	s_Links.clear();
	s_Nodes.shrink_to_fit();
	s_Links.shrink_to_fit();

	json j;

	Waveless::JSONParser::loadJsonDataFromDisk(("..//..//Asset//Canvas//" + std::string(fileName)).c_str(), j);

	NodeWidget* node;

	for (auto& j_node : j["Nodes"])
	{
		node = SpawnNode(std::string(j_node["Name"]).c_str());
		node->OldID = j_node["ID"];

		for (auto& j_input : j_node["Inputs"])
		{
			for (auto& pin : node->Inputs)
			{
				if (pin.Desc->Name == j_input["Name"])
				{
					pin.OldID = j_input["ID"];
				}
			}
		}

		for (auto& j_output : j_node["Outputs"])
		{
			for (auto& pin : node->Outputs)
			{
				if (pin.Desc->Name == j_output["Name"])
				{
					pin.OldID = j_output["ID"];
				}
			}
		}

		ed::SetNodePosition(node->ID, ImVec2(j_node["Position"]["X"], j_node["Position"]["Y"]));
	}

	for (auto& j_link : j["Links"])
	{
		auto l_startPin = FindPinByOldID(j_link["StartPinID"]);
		auto l_endPin = FindPinByOldID(j_link["EndPinID"]);

		SpawnLink(l_startPin, l_endPin);
	}

	ed::NavigateToContent();
	BuildNodes();
}

void SaveCanvas(const char* fileName)
{
	json j;

	for (auto& node : s_Nodes)
	{
		auto l_pos = ed::GetNodePosition(node.ID);
		json j_node;
		j_node["ID"] = node.ID.Get();
		j_node["Name"] = node.Desc->Name;
		j_node["NodeType"] = node.Desc->Type;
		j_node["Position"]["X"] = (int)l_pos.x;
		j_node["Position"]["Y"] = (int)l_pos.y;

		for (auto& input : node.Inputs)
		{
			json j_input;
			j_input["ID"] = input.ID.Get();
			j_input["Name"] = input.Desc->Name;
			j_input["Value"] = input.Value;
			j_node["Inputs"].emplace_back(j_input);
		}

		for (auto& output : node.Outputs)
		{
			json j_output;
			j_output["ID"] = output.ID.Get();
			j_output["Name"] = output.Desc->Name;
			j_output["Value"] = output.Value;
			j_node["Outputs"].emplace_back(j_output);
		}

		j["Nodes"].emplace_back(j_node);
	}

	for (auto& link : s_Links)
	{
		json j_link;
		j_link["ID"] = link.ID.Get();
		j_link["StartPinID"] = link.StartPin->ID.Get();
		j_link["EndPinID"] = link.EndPin->ID.Get();
		j["Links"].emplace_back(j_link);
	}

	Waveless::JSONParser::saveJsonDataToDisk(("..//..//Asset//Canvas//" + std::string(fileName)).c_str(), j);
}

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

void ShowLeftPane(float paneWidth)
{
	auto& io = ImGui::GetIO();

	ImGui::BeginChild("Nodes", ImVec2(paneWidth, 0));

	paneWidth = ImGui::GetContentRegionAvailWidth();

	static char buffer[128] = "DefaultCanvas.json";
	static bool wasActive = false;

	ImGui::PushItemWidth(200.0f);
	ImGui::InputText("##edit", buffer, 127);
	ImGui::PopItemWidth();
	if (ImGui::IsItemActive() && !wasActive)
	{
		ed::EnableShortcuts(false);
		wasActive = true;
	}
	else if (!ImGui::IsItemActive() && wasActive)
	{
		ed::EnableShortcuts(true);
		wasActive = false;
	}

	ImGui::BeginHorizontal("Serialization Function", ImVec2(paneWidth, 0));

	if (ImGui::Button("Load"))
	{
		LoadCanvas(&buffer[0]);
	}
	ImGui::Spring(0.0f);
	if (ImGui::Button("Save"))
	{
		SaveCanvas(&buffer[0]);
	}
	ImGui::Spring();
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("Compiler Function", ImVec2(paneWidth, 0));
	if (ImGui::Button("Compile"))
	{
		NodeCompiler::Compile(&buffer[0], &buffer[0]);
	}
	ImGui::Spring();
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("Simulator Function", ImVec2(paneWidth, 0));
	if (ImGui::Button("Trigger"))
	{
	}
	ImGui::Spring();
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("Canvas Function", ImVec2(paneWidth, 0));
	if (ImGui::Button("Zoom to Content"))
	{
		ed::NavigateToContent();
	}
	ImGui::Spring(0.0f);
	if (ImGui::Button("Show Flow"))
	{
		for (auto& link : s_Links)
		{
			ed::Flow(link.ID);
		}
	}
	ImGui::Spring();
	ImGui::EndHorizontal();

	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;
	selectedNodes.resize(ed::GetSelectedObjectCount());
	selectedLinks.resize(ed::GetSelectedObjectCount());

	int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
	int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

	selectedNodes.resize(nodeCount);
	selectedLinks.resize(linkCount);

	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Nodes");
	ImGui::Indent();
	for (auto& node : s_Nodes)
	{
		ImGui::PushID(node.ID.AsPointer());
		auto start = ImGui::GetCursorScreenPos();

		if (const auto progress = GetTouchProgress(node.ID))
		{
			ImGui::GetWindowDrawList()->AddLine(
				start + ImVec2(-8, 0),
				start + ImVec2(-8, ImGui::GetTextLineHeight()),
				IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
		}

		bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();
		if (ImGui::Selectable((std::string(node.Desc->Name) + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer()))).c_str(), &isSelected))
		{
			if (io.KeyCtrl)
			{
				if (isSelected)
					ed::SelectNode(node.ID, true);
				else
					ed::DeselectNode(node.ID);
			}
			else
				ed::SelectNode(node.ID, false);

			ed::NavigateToSelection();
		}
		auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer())) + ")";
		auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
		auto iconPanelPos = start + ImVec2(
			paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - ImGui::GetStyle().ItemInnerSpacing.x * 1,
			(ImGui::GetTextLineHeight()) / 2);
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(iconPanelPos.x - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y),
			IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

		ImGui::PopID();
	}
	ImGui::Unindent();

	ImGui::EndChild();
}

WsResult ImGuiWrapper::Setup()
{
#if defined WS_OS_WIN
	m_windowImpl = new ImGuiWindowWin();
#if defined WS_RENDERER_DIRECTX
	m_rendererImpl = new ImGuiRendererDX11();
#elif WS_RENDERER_VULKAN
	m_rendererImpl = new ImGuiRendererVK();
#elif defined WS_RENDERER_OPENGL
	m_rendererImpl = new ImGuiRendererGL();
#endif
#endif
#if defined WS_OS_MAC
	ImGuiWrapperNS::m_isParity = false;
#if defined WS_RENDERER_METAL
	ImGuiWrapperNS::m_isParity = false;
#endif
#endif
#if defined WS_OS_LINUX
	ImGuiWrapperNS::m_isParity = false;
#endif

	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->setup();
		ImGuiWrapperNS::m_rendererImpl->setup();

		IMGUI_CHECKVERSION();

		auto l_workingDir = IOService::getWorkingDirectory();
		l_workingDir += "..//..//Asset//Fonts//RobotoCondensed-Regular.ttf";

		ImFontConfig l_fontConfig;
		l_fontConfig.OversampleH = 4;
		l_fontConfig.OversampleV = 4;
		l_fontConfig.PixelSnapH = false;

		fontAtlas.AddFontFromFileTTF(l_workingDir.c_str(), 22.0f, &l_fontConfig);
		fontAtlas.Build();

		ImGui::CreateContext(&fontAtlas);
		auto& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		return WsResult::Success;
	}
	else
	{
		return WsResult::NotImplemented;
	}
}

WsResult ImGuiWrapper::Initialize()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->initialize();
		ImGuiWrapperNS::m_rendererImpl->initialize();

		ImGui::StyleColorsDark();

		config.SettingsFile = "WsEditor.json";

		m_NodeEditorContext = ed::CreateEditor(&config);
		ed::SetCurrentEditor(m_NodeEditorContext);

		s_HeaderBackground = ImGuiWrapperNS::m_rendererImpl->LoadTexture("../../GitSubmodules/imgui-node-editor/Data/BlueprintBackground.png");
		return WsResult::Success;
	}
	else
	{
		return WsResult::NotImplemented;
	}
}

void ShowContextMenu()
{
	auto openPopupPosition = ImGui::GetMousePos();
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
	{
		ImGui::OpenPopup("NodeWidget Context Menu");
	}
	else if (ed::ShowPinContextMenu(&contextPinId))
	{
		ImGui::OpenPopup("PinWidget Context Menu");
	}
	else if (ed::ShowLinkContextMenu(&contextLinkId))
	{
		ImGui::OpenPopup("LinkWidget Context Menu");
	}
	else if (ed::ShowBackgroundContextMenu())
	{
		ImGui::OpenPopup("Create New NodeWidget");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("NodeWidget Context Menu"))
	{
		auto node = FindNode(contextNodeId);

		ImGui::TextUnformatted("NodeWidget Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %p", node->ID.AsPointer());
			ImGui::Text("Type: %s", node->Desc->Type == NodeType::Function ? "Function" : "Comment");
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
			if (node->Desc->Name == "Sequencer" || node->Desc->Name == "Selector" || node->Desc->Name == "Mixer")
			{
				if (ImGui::MenuItem("+ Create Input NodeWidget"))
				{
					node->Inputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);
					BuildNode(node);
				}
			}
		}
		else
		{
			ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
		{
			ed::DeleteNode(contextNodeId);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PinWidget Context Menu"))
	{
		auto pin = FindPin(contextPinId);

		ImGui::TextUnformatted("PinWidget Context Menu");
		ImGui::Separator();
		if (pin)
		{
			ImGui::Text("ID: %p", pin->ID.AsPointer());
			if (pin->NodeWidget)
				ImGui::Text("NodeWidget: %p", pin->NodeWidget->ID.AsPointer());
			else
				ImGui::Text("NodeWidget: %s", "<none>");
		}
		else
			ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("LinkWidget Context Menu"))
	{
		auto link = FindLink(contextLinkId);

		ImGui::TextUnformatted("LinkWidget Context Menu");
		ImGui::Separator();
		if (link)
		{
			ImGui::Text("ID: %p", link->ID.AsPointer());
			ImGui::Text("From: %p", link->StartPin->ID.AsPointer());
			ImGui::Text("To: %p", link->EndPin->ID.AsPointer());
		}
		else
			ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteLink(contextLinkId);
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Create New NodeWidget"))
	{
		auto newNodePostion = openPopupPosition;

		NodeWidget* node = nullptr;
		if (ImGui::MenuItem("BreakVector"))
			node = SpawnNode("BreakVector");

		ImGui::Separator();

		if (ImGui::MenuItem("Input"))
			node = SpawnNode("Input");
		if (ImGui::MenuItem("Output"))
			node = SpawnNode("Output");

		ImGui::Separator();

		if (ImGui::MenuItem("Const Bool"))
			node = SpawnNode("ConstBool");
		if (ImGui::MenuItem("Const Int"))
			node = SpawnNode("ConstInt");
		if (ImGui::MenuItem("Const Float"))
			node = SpawnNode("ConstFloat");
		if (ImGui::MenuItem("Const String"))
			node = SpawnNode("ConstString");

		ImGui::Separator();

		if (ImGui::MenuItem("WavePlayer"))
			node = SpawnNode("WavePlayer");
		if (ImGui::MenuItem("Sequencer"))
			node = SpawnNode("Sequencer");
		if (ImGui::MenuItem("Selector"))
			node = SpawnNode("Selector");
		if (ImGui::MenuItem("Attenuator"))
			node = SpawnNode("Attenuator");
		if (ImGui::MenuItem("Mixer"))
			node = SpawnNode("Mixer");

		ImGui::Separator();

		if (ImGui::MenuItem("Branch"))
			node = SpawnNode("Branch");
		if (ImGui::MenuItem("Do N"))
			node = SpawnNode("DoN");
		if (ImGui::MenuItem("Print String"))
			node = SpawnNode("PrintString");

		ImGui::Separator();

		if (ImGui::MenuItem("Comment"))
			node = SpawnComment();

		if (node)
		{
			createNewNode = false;

			ed::SetNodePosition(node->ID, newNodePostion);

			if (auto startPin = newNodeLinkPin)
			{
				auto& pins = startPin->Desc->Kind == PinKind::Input ? node->Outputs : node->Inputs;

				for (auto& pin : pins)
				{
					if (CanCreateLink(startPin, &pin))
					{
						auto endPin = &pin;
						if (startPin->Desc->Kind == PinKind::Input)
							std::swap(startPin, endPin);

						SpawnLink(startPin, endPin);
						break;
					}
				}
			}
		}

		ImGui::EndPopup();
	}
	else
	{
		createNewNode = false;
	}
	ImGui::PopStyleVar();
	ed::Resume();
}

void ShowFunctionsAndVars(util::BlueprintNodeBuilder& builder, NodeWidget& node)
{
	bool hasOutputDelegates = false;
	for (auto& output : node.Outputs)
		if (output.Desc->Type == PinType::Delegate)
			hasOutputDelegates = true;

	builder.Begin(node.ID);

	builder.Header(node.Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(node.Desc->Name);
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	if (hasOutputDelegates)
	{
		ImGui::BeginVertical("delegates", ImVec2(0, 28));
		ImGui::Spring(1, 0);
		for (auto& output : node.Outputs)
		{
			if (output.Desc->Type != PinType::Delegate)
				continue;

			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			ed::BeginPin(output.ID, ed::PinKind::Output);
			ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
			ed::PinPivotSize(ImVec2(0, 0));
			ImGui::BeginHorizontal(output.ID.AsPointer());
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			if (strcmp(output.Desc->Name, ""))
			{
				ImGui::TextUnformatted(output.Desc->Name);
				ImGui::Spring(0);
			}
			DrawPinIcon(output, IsPinLinked(&output), (int)(alpha * 255));
			ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
			ImGui::EndHorizontal();
			ImGui::PopStyleVar();
			ed::EndPin();
		}
		ImGui::Spring(1, 0);
		ImGui::EndVertical();
		ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
	}
	else
	{
		ImGui::Spring(0);
	}
	builder.EndHeader();

	for (auto& input : node.Inputs)
	{
		auto alpha = ImGui::GetStyle().Alpha;
		if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
		{
			alpha = alpha * (48.0f / 255.0f);
		}

		builder.Input(input.ID);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		DrawPinIcon(input, IsPinLinked(&input), (int)(alpha * 255));
		ImGui::Spring(0);

		if (strcmp(input.Desc->Name, ""))
		{
			ImGui::TextUnformatted(input.Desc->Name);
			ImGui::Spring(0);
		}

		ImGui::PopStyleVar();
		builder.EndInput();
	}

	for (auto& output : node.Outputs)
	{
		if (output.Desc->Type == PinType::Delegate)
			continue;

		auto alpha = ImGui::GetStyle().Alpha;
		if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
		{
			alpha = alpha * (48.0f / 255.0f);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		builder.Output(output.ID);

		if (strstr(node.Desc->Name, "Const") != nullptr)
		{
			if (output.Desc->Type == PinType::Bool)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::Checkbox("", reinterpret_cast<bool*>(&output.Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Desc->Type == PinType::Int)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::InputInt("", reinterpret_cast<int32_t*>(&output.Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Desc->Type == PinType::Float)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::InputFloat("", reinterpret_cast<float*>(&output.Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Desc->Type == PinType::String)
			{
				static char buffer[128] = "Content";
				static bool wasActive = false;

				ImGui::PushItemWidth(100.0f);
				ImGui::InputText("##edit", buffer, 127);
				ImGui::PopItemWidth();
				if (ImGui::IsItemActive() && !wasActive)
				{
					ed::EnableShortcuts(false);
					wasActive = true;
				}
				else if (!ImGui::IsItemActive() && wasActive)
				{
					ed::EnableShortcuts(true);
					wasActive = false;
				}
				ImGui::Spring(0);
			}
		}

		if (strcmp(output.Desc->Name, ""))
		{
			ImGui::Spring(0);
			ImGui::TextUnformatted(output.Desc->Name);
		}
		ImGui::Spring(0);
		DrawPinIcon(output, IsPinLinked(&output), (int)(alpha * 255));
		ImGui::PopStyleVar();
		builder.EndOutput();
	}

	builder.End();
}

void ShowComments(util::BlueprintNodeBuilder& builder, const NodeWidget& node)
{
	const float commentAlpha = 0.75f;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	ed::BeginNode(node.ID);
	ImGui::PushID(node.ID.AsPointer());
	ImGui::BeginVertical("content");
	ImGui::BeginHorizontal("horizontal");
	ImGui::Spring(1);
	ImGui::TextUnformatted(node.Desc->Name);
	ImGui::Spring(1);
	ImGui::EndHorizontal();
	ed::Group(node.Size);
	ImGui::EndVertical();
	ImGui::PopID();
	ed::EndNode();
	ed::PopStyleColor(2);
	ImGui::PopStyleVar();

	if (ed::BeginGroupHint(node.ID))
	{
		auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

		auto min = ed::GetGroupMin();

		ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
		ImGui::BeginGroup();
		ImGui::TextUnformatted(node.Desc->Name);
		ImGui::EndGroup();

		auto drawList = ed::GetHintBackgroundDrawList();

		auto hintBounds = ImGui_GetItemRect();
		auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

		drawList->AddRectFilled(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

		drawList->AddRect(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);
	}
	ed::EndGroupHint();
}

void CreateNodes()
{
	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
		auto showLabel = [](const char* label, ImColor color)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
			auto size = ImGui::CalcTextSize(label);

			auto padding = ImGui::GetStyle().FramePadding;
			auto spacing = ImGui::GetStyle().ItemSpacing;

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

			auto rectMin = ImGui::GetCursorScreenPos() - padding;
			auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
			ImGui::TextUnformatted(label);
		};

		ed::PinId startPinId = 0, endPinId = 0;
		if (ed::QueryNewLink(&startPinId, &endPinId))
		{
			auto startPin = FindPin(startPinId);
			auto endPin = FindPin(endPinId);

			newLinkPin = startPin ? startPin : endPin;

			if (startPin->Desc->Kind == PinKind::Input)
			{
				std::swap(startPin, endPin);
				std::swap(startPinId, endPinId);
			}

			if (startPin && endPin)
			{
				if (endPin == startPin)
				{
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (endPin->Desc->Kind == startPin->Desc->Kind)
				{
					showLabel("x Incompatible PinWidget Kind", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (endPin->Desc->Type != startPin->Desc->Type)
				{
					showLabel("x Incompatible PinWidget Type", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
				}
				else
				{
					showLabel("+ Create LinkWidget", ImColor(32, 45, 32, 180));
					if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
					{
						SpawnLink(startPin, endPin);
					}
				}
			}
		}

		ed::PinId pinId = 0;
		if (ed::QueryNewNode(&pinId))
		{
			newLinkPin = FindPin(pinId);
			if (newLinkPin)
			{
				showLabel("+ Create NodeWidget", ImColor(32, 45, 32, 180));
			}

			if (ed::AcceptNewItem())
			{
				createNewNode = true;
				newNodeLinkPin = FindPin(pinId);
				newLinkPin = nullptr;
				ed::Suspend();
				ImGui::OpenPopup("Create New NodeWidget");
				ed::Resume();
			}
		}
	}
	else
	{
		newLinkPin = nullptr;
	}

	ed::EndCreate();
}

void DeleteNodes()
{
	if (ed::BeginDelete())
	{
		ed::LinkId linkId = 0;
		while (ed::QueryDeletedLink(&linkId))
		{
			if (ed::AcceptDeletedItem())
			{
				auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
				if (id != s_Links.end())
					s_Links.erase(id);
			}
		}

		ed::NodeId nodeId = 0;
		while (ed::QueryDeletedNode(&nodeId))
		{
			if (ed::AcceptDeletedItem())
			{
				auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
				if (id != s_Nodes.end())
					s_Nodes.erase(id);
			}
		}
	}
	ed::EndDelete();
}

void ShowEditorCanvas()
{
	ed::Begin("WsEditor");
	{
		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		util::BlueprintNodeBuilder builder(s_HeaderBackground, ImGuiWrapperNS::m_rendererImpl->GetTextureWidth(s_HeaderBackground), ImGuiWrapperNS::m_rendererImpl->GetTextureHeight(s_HeaderBackground));

		for (auto& node : s_Nodes)
		{
			if (node.Desc->Type == NodeType::Function || node.Desc->Type == NodeType::ConstVar)
			{
				ShowFunctionsAndVars(builder, node);
			}
			else if (node.Desc->Type == NodeType::Comment)
			{
				ShowComments(builder, node);
			}
		}

		for (auto& link : s_Links)
		{
			ed::Link(link.ID, link.StartPin->ID, link.EndPin->ID, link.Color, 2.0f);
		}

		if (!createNewNode)
		{
			CreateNodes();
			DeleteNodes();
		}

		ImGui::SetCursorScreenPos(cursorTopLeft);
	}

	ShowContextMenu();

	ed::End();
}

WsResult Frame()
{
	if (ImGuiWrapperNS::m_windowImpl->update())
	{
		ImGuiWrapperNS::m_windowImpl->newFrame();
		ImGuiWrapperNS::m_rendererImpl->newFrame();

		ImGui::NewFrame();

		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
		ImGui::Begin("Content", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus);

		UpdateTouch();

		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ed::SetCurrentEditor(m_NodeEditorContext);

		static float leftPaneWidth = 400.0f;
		static float rightPaneWidth = 800.0f;
		Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

		ShowLeftPane(leftPaneWidth - 4.0f);

		ImGui::SameLine(0.0f, 12.0f);

		ShowEditorCanvas();

		ImGui::End();

		ImGui::Render();

		ImGuiWrapperNS::m_rendererImpl->render();

		return WsResult::Success;
	}
	return WsResult::Fail;
}

WsResult ImGuiWrapper::Render()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		return Frame();
	}
	else
	{
		return WsResult::NotImplemented;
	}
}

WsResult ImGuiWrapper::Terminate()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->terminate();
		ImGuiWrapperNS::m_rendererImpl->terminate();
		ImGui::DestroyContext();
		return WsResult::Success;
	}
	else
	{
		return WsResult::NotImplemented;
	}
}