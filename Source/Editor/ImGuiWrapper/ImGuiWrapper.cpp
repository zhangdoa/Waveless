#include "ImGuiWrapper.h"
#include "../../Core/Config.h"
#include "../../Core/stdafx.h"
#include "../../Core/Math.h"
#include "../../Core/String.h"
#include "../../IO/IOService.h"
#include "../../IO/JSONParser.h"
#include "../NodeDescriptorManager.h"
#include "../NodeModelManager.h"
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
	ed::PinId ID;

	PinModel* Model;
	NodeWidget* Owner;

	PinWidget(uint64_t id, PinModel* model) : ID(id), Model(model)
	{
	}
};

struct NodeWidget : public Object
{
	ed::NodeId ID;

	NodeModel* Model;

	ImColor Color = ImColor(255, 255, 255);
	ImVec2 Size = ImVec2(0, 0);

	std::vector<PinWidget> Inputs;
	std::vector<PinWidget> Outputs;

	NodeWidget(uint64_t id, NodeModel* model) : ID(id), Model(model)
	{
		Color = ImColor(model->Desc->Color[0], model->Desc->Color[1], model->Desc->Color[2]);
	}
};

struct LinkWidget : public Object
{
	ed::LinkId ID;

	LinkModel* Model;

	ImColor Color;

	PinWidget* StartPin = 0;
	PinWidget* EndPin = 0;

	LinkWidget(uint64_t id, PinWidget* startPin, PinWidget* endPin) :
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
	ImColor  color = GetIconColor(pin.Model->Desc->Type);
	color.Value.w = alpha / 255.0f;
	switch (pin.Model->Desc->Type)
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

static uint64_t GetNextId()
{
	return Waveless::Math::GenerateUUID();
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

static PinWidget* FindPinByUUID(uint64_t id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.Inputs)
			if (pin.Model->UUID == id)
				return &pin;

		for (auto& pin : node.Outputs)
			if (pin.Model->UUID == id)
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
	if (!a || !b || a == b
		|| a->Model->Desc->Kind == b->Model->Desc->Kind
		|| a->Model->Desc->Type != b->Model->Desc->Type
		|| a->Owner == b->Owner)
		return false;

	return true;
}

static void BuildNode(NodeWidget* node)
{
	for (auto& input : node->Inputs)
	{
		input.Owner = node;
	}

	for (auto& output : node->Outputs)
	{
		output.Owner = node;
	}
}

NodeWidget* SpawnNodeWidget(NodeModel* model)
{
	s_Nodes.emplace_back(GetNextId(), model);

	for (int i = 0; i < model->Desc->InputPinCount; i++)
	{
		auto l_pinModel = NodeModelManager::GetPinModel(model->InputPinIndexOffset + i);
		s_Nodes.back().Inputs.emplace_back(GetNextId(), l_pinModel);
	}
	for (int i = 0; i < model->Desc->OutputPinCount; i++)
	{
		auto l_pinModel = NodeModelManager::GetPinModel(model->OutputPinIndexOffset + i);
		s_Nodes.back().Outputs.emplace_back(GetNextId(), l_pinModel);
	}

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

NodeWidget* SpawnNodeWidget(const char* nodeDescriptorName)
{
	auto l_nodeModel = NodeModelManager::SpawnNodeModel(nodeDescriptorName);

	return SpawnNodeWidget(l_nodeModel);
};

LinkWidget* SpawnLinkWidget(PinWidget* startPin, PinWidget* endPin)
{
	s_Links.emplace_back(LinkWidget(GetNextId(), startPin, endPin));
	s_Links.back().Color = GetIconColor(startPin->Model->Desc->Type);

	return &s_Links.back();
}

static NodeWidget* SpawnComment()
{
	//s_Nodes.emplace_back(GetNextId(), "New Comment");
	//s_Nodes.back().Model->Desc->Type = NodeType::Comment;
	//s_Nodes.back().Size = ImVec2(300, 200);

	//return &s_Nodes.back();
	return nullptr;
}

static void BuildNodes()
{
	for (auto& node : s_Nodes)
	{
		BuildNode(&node);
	}
}

static void LoadCanvas(const char* fileName)
{
	NodeModelManager::LoadCanvas(fileName);

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

	auto l_nodeModels = NodeModelManager::GetAllNodeModels();

	for (auto l_nodeModel : l_nodeModels)
	{
		auto l_nodeWidget = SpawnNodeWidget(l_nodeModel);
		ed::SetNodePosition(l_nodeWidget->ID, ImVec2(l_nodeModel->InitialPosition[0], l_nodeModel->InitialPosition[1]));
	}

	auto l_linkModels = NodeModelManager::GetAllLinkModels();

	for (auto l_linkModel : l_linkModels)
	{
		auto l_startPin = FindPinByUUID(l_linkModel->StartPin->UUID);
		auto l_endPin = FindPinByUUID(l_linkModel->EndPin->UUID);

		auto l_linkWidget = SpawnLinkWidget(l_startPin, l_endPin);
		l_linkWidget->Model = l_linkModel;
	}

	ed::NavigateToContent();
	BuildNodes();
}

static void SaveCanvas(const char* fileName)
{
	json j;

	for (auto& node : s_Nodes)
	{
		auto l_pos = ed::GetNodePosition(node.ID);
		json j_node;
		j_node["ID"] = node.ID.Get();
		j_node["Name"] = node.Model->Desc->Name;
		j_node["NodeType"] = node.Model->Desc->Type;
		j_node["Position"]["X"] = (int)l_pos.x;
		j_node["Position"]["Y"] = (int)l_pos.y;

		for (auto& input : node.Inputs)
		{
			json j_input;
			j_input["ID"] = input.ID.Get();
			j_input["Name"] = input.Model->Desc->Name;
			if (input.Model->Desc->Type == PinType::String)
			{
				j_input["Value"] = StringManager::FindString(input.Model->Value).value;
			}
			else
			{
				j_input["Value"] = input.Model->Value;
			}

			j_node["Inputs"].emplace_back(j_input);
		}

		for (auto& output : node.Outputs)
		{
			json j_output;
			j_output["ID"] = output.ID.Get();
			j_output["Name"] = output.Model->Desc->Name;
			if (output.Model->Desc->Type == PinType::String)
			{
				j_output["Value"] = StringManager::FindString(output.Model->Value).value;
			}
			else
			{
				j_output["Value"] = output.Model->Value;
			}
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

static void ShowLeftPane(float paneWidth)
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
		if (ImGui::Selectable((std::string(node.Model->Desc->Name) + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer()))).c_str(), &isSelected))
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
		ImGui::OpenPopup("Owner Context Menu");
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
		ImGui::OpenPopup("Create New Owner");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Owner Context Menu"))
	{
		auto node = FindNode(contextNodeId);

		ImGui::TextUnformatted("Owner Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %p", node->ID.AsPointer());
			ImGui::Text("Type: %d", node->Model->Desc->Type);
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
			if (node->Model->Desc->Name == "Sequencer" || node->Model->Desc->Name == "Selector" || node->Model->Desc->Name == "Mixer")
			{
				if (ImGui::MenuItem("+ Create Input Owner"))
				{
					// @TODO: Dynamically change the pin
					//node->Inputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);
					//BuildNode(node);
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
			if (pin->Owner)
				ImGui::Text("Owner: %p", pin->Owner->ID.AsPointer());
			else
				ImGui::Text("Owner: %s", "<none>");
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

	if (ImGui::BeginPopup("Create New Owner"))
	{
		auto newNodePostion = openPopupPosition;

		NodeWidget* node = nullptr;

		auto l_nodeDescs = NodeDescriptorManager::GetAllNodeDescriptors();

		for (auto i : l_nodeDescs)
		{
			if (ImGui::MenuItem(i->Name))
				node = SpawnNodeWidget(i->Name);
		}

		if (node)
		{
			createNewNode = false;

			ed::SetNodePosition(node->ID, newNodePostion);

			if (auto startPin = newNodeLinkPin)
			{
				auto& pins = startPin->Model->Desc->Kind == PinKind::Input ? node->Outputs : node->Inputs;

				for (auto& pin : pins)
				{
					if (CanCreateLink(startPin, &pin))
					{
						auto endPin = &pin;
						if (startPin->Model->Desc->Kind == PinKind::Input)
							std::swap(startPin, endPin);

						SpawnLinkWidget(startPin, endPin);
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
		if (output.Model->Desc->Type == PinType::Delegate)
			hasOutputDelegates = true;

	builder.Begin(node.ID);

	builder.Header(node.Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(node.Model->Desc->Name);
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	if (hasOutputDelegates)
	{
		ImGui::BeginVertical("delegates", ImVec2(0, 28));
		ImGui::Spring(1, 0);
		for (auto& output : node.Outputs)
		{
			if (output.Model->Desc->Type != PinType::Delegate)
				continue;

			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			ed::BeginPin(output.ID, ed::PinKind::Output);
			ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
			ed::PinPivotSize(ImVec2(0, 0));
			ImGui::BeginHorizontal(output.ID.AsPointer());
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			if (strcmp(output.Model->Desc->Name, ""))
			{
				ImGui::TextUnformatted(output.Model->Desc->Name);
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

		if (strcmp(input.Model->Desc->Name, ""))
		{
			ImGui::TextUnformatted(input.Model->Desc->Name);
			ImGui::Spring(0);
		}

		ImGui::PopStyleVar();
		builder.EndInput();
	}

	for (auto& output : node.Outputs)
	{
		if (output.Model->Desc->Type == PinType::Delegate)
			continue;

		auto alpha = ImGui::GetStyle().Alpha;
		if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
		{
			alpha = alpha * (48.0f / 255.0f);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		builder.Output(output.ID);

		if (strstr(node.Model->Desc->Name, "Const") != nullptr)
		{
			if (output.Model->Desc->Type == PinType::Bool)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::Checkbox("", reinterpret_cast<bool*>(&output.Model->Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Model->Desc->Type == PinType::Int)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::InputInt("", reinterpret_cast<int32_t*>(&output.Model->Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Model->Desc->Type == PinType::Float)
			{
				ImGui::PushItemWidth(100.0f);
				ImGui::InputFloat("", reinterpret_cast<float*>(&output.Model->Value));
				ImGui::PopItemWidth();
				ImGui::Spring(0);
			}
			if (output.Model->Desc->Type == PinType::String)
			{
				// @TODO: Multiple buffer
				static char buffer[128] = "";

				if (output.Model->Value)
				{
					auto l_string = StringManager::FindString(output.Model->Value);
					std::memcpy(buffer, l_string.value, strlen(l_string.value));
				}

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
					auto l_UUID = output.Model->Value;
					StringManager::DeleteString(l_UUID);
					output.Model->Value = StringManager::SpawnString(&buffer[0]).UUID;
				}
				ImGui::Spring(0);
			}
		}

		if (strcmp(output.Model->Desc->Name, ""))
		{
			ImGui::Spring(0);
			ImGui::TextUnformatted(output.Model->Desc->Name);
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
	ImGui::TextUnformatted(node.Model->Desc->Name);
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
		ImGui::TextUnformatted(node.Model->Desc->Name);
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

			if (startPin->Model->Desc->Kind == PinKind::Input)
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
				else if (endPin->Model->Desc->Kind == startPin->Model->Desc->Kind)
				{
					showLabel("x Incompatible PinWidget Kind", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				else if (endPin->Model->Desc->Type != startPin->Model->Desc->Type)
				{
					showLabel("x Incompatible PinWidget Type", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
				}
				else
				{
					showLabel("+ Create LinkWidget", ImColor(32, 45, 32, 180));
					if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
					{
						SpawnLinkWidget(startPin, endPin);
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
			if (node.Model->Desc->Type == NodeType::Comment)
			{
				ShowComments(builder, node);
			}
			else
			{
				ShowFunctionsAndVars(builder, node);
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