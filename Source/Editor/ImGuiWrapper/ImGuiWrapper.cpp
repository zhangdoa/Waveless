#include "ImGuiWrapper.h"
#include "../../Core/Config.h"
#include "../../Core/stdafx.h"
#include "../../IO/IOService.h"

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

using ax::Widgets::IconType;

enum class PinType
{
	Flow,
	Bool,
	Int,
	Float,
	String,
	Vector,
	Object,
	Function,
	Delegate,
};

enum class PinKind
{
	Output,
	Input
};

enum class NodeType
{
	Blueprint,
	Comment
};

struct Node;

struct Pin
{
	ed::PinId   ID;
	::Node*     Node;
	std::string Name;
	PinType     Type;
	PinKind     Kind;

	Pin(int id, const char* name, PinType type) :
		ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
	{
	}
};

struct Node
{
	ed::NodeId ID;
	std::string Name;
	std::vector<Pin> Inputs;
	std::vector<Pin> Outputs;
	ImColor Color;
	NodeType Type;
	ImVec2 Size;

	std::string State;
	std::string SavedState;

	Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
		ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
	{
	}
};

struct Link
{
	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	ImColor Color;

	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
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
	Pin* newNodeLinkPin = nullptr;
	Pin* newLinkPin = nullptr;

	const int            s_PinIconSize = 24;
	std::vector<Node>    s_Nodes;
	std::vector<Link>    s_Links;
	ImTextureID          s_HeaderBackground = nullptr;

	const float          s_TouchTime = 1.0f;
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

static Node* FindNode(ed::NodeId id)
{
	for (auto& node : s_Nodes)
		if (node.ID == id)
			return &node;

	return nullptr;
}

static Link* FindLink(ed::LinkId id)
{
	for (auto& link : s_Links)
		if (link.ID == id)
			return &link;

	return nullptr;
}

static Pin* FindPin(ed::PinId id)
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

static bool IsPinLinked(ed::PinId id)
{
	if (!id)
		return false;

	for (auto& link : s_Links)
		if (link.StartPinID == id || link.EndPinID == id)
			return true;

	return false;
}

static bool CanCreateLink(Pin* a, Pin* b)
{
	if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
		return false;

	return true;
}

static void BuildNode(Node* node)
{
	for (auto& input : node->Inputs)
	{
		input.Node = node;
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs)
	{
		output.Node = node;
		output.Kind = PinKind::Output;
	}
}

static Node* SpawnInputNode()
{
	s_Nodes.emplace_back(GetNextId(), "Input", ImColor(255, 128, 128));
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "WorldPosition", PinType::Vector);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnOutputNode()
{
	s_Nodes.emplace_back(GetNextId(), "Output");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnWavePlayerNode()
{
	s_Nodes.emplace_back(GetNextId(), "WavePlayer");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "FileName", PinType::String);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnSequencerNode()
{
	s_Nodes.emplace_back(GetNextId(), "Sequencer");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnSelectorNode()
{
	s_Nodes.emplace_back(GetNextId(), "Selector");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "WaveObject", PinType::Object);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnBranchNode()
{
	s_Nodes.emplace_back(GetNextId(), "Branch");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnDoNNode()
{
	s_Nodes.emplace_back(GetNextId(), "Do N");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "N", PinType::Int);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnPrintStringNode()
{
	s_Nodes.emplace_back(GetNextId(), "Print String");
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "In String", PinType::String);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnSetTimerNode()
{
	s_Nodes.emplace_back(GetNextId(), "Set Timer", ImColor(128, 195, 248));
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
	s_Nodes.back().Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
	s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&s_Nodes.back());

	return &s_Nodes.back();
}

static Node* SpawnComment()
{
	s_Nodes.emplace_back(GetNextId(), "New Comment");
	s_Nodes.back().Type = NodeType::Comment;
	s_Nodes.back().Size = ImVec2(300, 200);

	return &s_Nodes.back();
}

void BuildNodes()
{
	for (auto& node : s_Nodes)
		BuildNode(&node);
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

void DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
	IconType iconType;
	ImColor  color = GetIconColor(pin.Type);
	color.Value.w = alpha / 255.0f;
	switch (pin.Type)
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

bool ImGuiWrapper::Setup()
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
	}

	return true;
}

bool ImGuiWrapper::Initialize()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->initialize();
		ImGuiWrapperNS::m_rendererImpl->initialize();

		ImGui::StyleColorsDark();

		config.SettingsFile = "WsEditor.json";

		config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
		{
			auto node = FindNode(nodeId);
			if (!node)
				return 0;

			if (data != nullptr)
				memcpy(data, node->State.data(), node->State.size());
			return node->State.size();
		};

		config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			auto node = FindNode(nodeId);
			if (!node)
				return false;

			node->State.assign(data, size);

			TouchNode(nodeId);

			return true;
		};

		m_NodeEditorContext = ed::CreateEditor(&config);
		ed::SetCurrentEditor(m_NodeEditorContext);

		Node* node;
		node = SpawnInputNode();      ed::SetNodePosition(node->ID, ImVec2(-252, 220));
		node = SpawnOutputNode();     ed::SetNodePosition(node->ID, ImVec2(71, 80));
		node = SpawnWavePlayerNode();         ed::SetNodePosition(node->ID, ImVec2(168, 512));

		node = SpawnBranchNode();           ed::SetNodePosition(node->ID, ImVec2(-300, 351));
		node = SpawnDoNNode();              ed::SetNodePosition(node->ID, ImVec2(-238, 504));
		node = SpawnSetTimerNode();         ed::SetNodePosition(node->ID, ImVec2(168, 316));

		node = SpawnPrintStringNode();      ed::SetNodePosition(node->ID, ImVec2(-69, 652));
		node = SpawnComment();              ed::SetNodePosition(node->ID, ImVec2(112, 576));

		ed::NavigateToContent();
		BuildNodes();

		s_HeaderBackground = ImGuiWrapperNS::m_rendererImpl->LoadTexture("../../GitSubmodules/imgui-node-editor/Data/BlueprintBackground.png");
	}

	return true;
}

void ShowEditorCanvas()
{
	ed::Begin("WsEditor");
	{
		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		util::BlueprintNodeBuilder builder(s_HeaderBackground, ImGuiWrapperNS::m_rendererImpl->GetTextureWidth(s_HeaderBackground), ImGuiWrapperNS::m_rendererImpl->GetTextureHeight(s_HeaderBackground));

		for (auto& node : s_Nodes)
		{
			if (node.Type != NodeType::Blueprint)
				continue;

			bool hasOutputDelegates = false;
			for (auto& output : node.Outputs)
				if (output.Type == PinType::Delegate)
					hasOutputDelegates = true;

			builder.Begin(node.ID);

			builder.Header(node.Color);
			ImGui::Spring(0);
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::Spring(1);
			ImGui::Dummy(ImVec2(0, 28));
			if (hasOutputDelegates)
			{
				ImGui::BeginVertical("delegates", ImVec2(0, 28));
				ImGui::Spring(1, 0);
				for (auto& output : node.Outputs)
				{
					if (output.Type != PinType::Delegate)
						continue;

					auto alpha = ImGui::GetStyle().Alpha;
					if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
						alpha = alpha * (48.0f / 255.0f);

					ed::BeginPin(output.ID, ed::PinKind::Output);
					ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
					ed::PinPivotSize(ImVec2(0, 0));
					ImGui::BeginHorizontal(output.ID.AsPointer());
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
					if (!output.Name.empty())
					{
						ImGui::TextUnformatted(output.Name.c_str());
						ImGui::Spring(0);
					}
					DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
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
				DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
				ImGui::Spring(0);

				if (!input.Name.empty())
				{
					ImGui::TextUnformatted(input.Name.c_str());
					ImGui::Spring(0);
				}

				if (input.Type == PinType::Bool)
				{
					static bool l;
					ImGui::Checkbox("", &l);
					ImGui::Spring(0);
				}
				ImGui::PopStyleVar();
				builder.EndInput();
			}

			for (auto& output : node.Outputs)
			{
				if (output.Type == PinType::Delegate)
					continue;

				auto alpha = ImGui::GetStyle().Alpha;
				if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				{
					alpha = alpha * (48.0f / 255.0f);
				}

				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
				builder.Output(output.ID);
				if (output.Type == PinType::String)
				{
					static char buffer[128] = "Edit Me\nMultiline!";
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
				if (!output.Name.empty())
				{
					ImGui::Spring(0);
					ImGui::TextUnformatted(output.Name.c_str());
				}
				ImGui::Spring(0);
				DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
				ImGui::PopStyleVar();
				builder.EndOutput();
			}

			builder.End();
		}

		for (auto& node : s_Nodes)
		{
			if (node.Type != NodeType::Comment)
				continue;

			const float commentAlpha = 0.75f;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
			ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
			ed::BeginNode(node.ID);
			ImGui::PushID(node.ID.AsPointer());
			ImGui::BeginVertical("content");
			ImGui::BeginHorizontal("horizontal");
			ImGui::Spring(1);
			ImGui::TextUnformatted(node.Name.c_str());
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
				ImGui::TextUnformatted(node.Name.c_str());
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

		for (auto& link : s_Links)
		{
			ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);
		}

		if (!createNewNode)
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

					if (startPin->Kind == PinKind::Input)
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
						else if (endPin->Kind == startPin->Kind)
						{
							showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (endPin->Type != startPin->Type)
						{
							showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
						}
						else
						{
							showLabel("+ Create Link", ImColor(32, 45, 32, 180));
							if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
							{
								s_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
								s_Links.back().Color = GetIconColor(startPin->Type);
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
						showLabel("+ Create Node", ImColor(32, 45, 32, 180));
					}

					if (ed::AcceptNewItem())
					{
						createNewNode = true;
						newNodeLinkPin = FindPin(pinId);
						newLinkPin = nullptr;
						ed::Suspend();
						ImGui::OpenPopup("Create New Node");
						ed::Resume();
					}
				}
			}
			else
			{
				newLinkPin = nullptr;
			}

			ed::EndCreate();

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

		ImGui::SetCursorScreenPos(cursorTopLeft);
	}

	auto openPopupPosition = ImGui::GetMousePos();
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
	{
		ImGui::OpenPopup("Node Context Menu");
	}
	else if (ed::ShowPinContextMenu(&contextPinId))
	{
		ImGui::OpenPopup("Pin Context Menu");
	}
	else if (ed::ShowLinkContextMenu(&contextLinkId))
	{
		ImGui::OpenPopup("Link Context Menu");
	}
	else if (ed::ShowBackgroundContextMenu())
	{
		ImGui::OpenPopup("Create New Node");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Node Context Menu"))
	{
		auto node = FindNode(contextNodeId);

		ImGui::TextUnformatted("Node Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %p", node->ID.AsPointer());
			ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : "Comment");
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
			if (node->Name == "Sequencer" || node->Name == "Selector")
			{
				if (ImGui::MenuItem("+ Create Input Node"))
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

	if (ImGui::BeginPopup("Pin Context Menu"))
	{
		auto pin = FindPin(contextPinId);

		ImGui::TextUnformatted("Pin Context Menu");
		ImGui::Separator();
		if (pin)
		{
			ImGui::Text("ID: %p", pin->ID.AsPointer());
			if (pin->Node)
				ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
			else
				ImGui::Text("Node: %s", "<none>");
		}
		else
			ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Link Context Menu"))
	{
		auto link = FindLink(contextLinkId);

		ImGui::TextUnformatted("Link Context Menu");
		ImGui::Separator();
		if (link)
		{
			ImGui::Text("ID: %p", link->ID.AsPointer());
			ImGui::Text("From: %p", link->StartPinID.AsPointer());
			ImGui::Text("To: %p", link->EndPinID.AsPointer());
		}
		else
			ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteLink(contextLinkId);
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Create New Node"))
	{
		auto newNodePostion = openPopupPosition;

		Node* node = nullptr;
		if (ImGui::MenuItem("Input"))
			node = SpawnInputNode();
		if (ImGui::MenuItem("Output"))
			node = SpawnOutputNode();
		if (ImGui::MenuItem("WavePlayer"))
			node = SpawnWavePlayerNode();
		if (ImGui::MenuItem("Sequencer"))
			node = SpawnSequencerNode();
		if (ImGui::MenuItem("Selector"))
			node = SpawnSelectorNode();
		if (ImGui::MenuItem("Branch"))
			node = SpawnBranchNode();
		if (ImGui::MenuItem("Do N"))
			node = SpawnDoNNode();
		if (ImGui::MenuItem("Set Timer"))
			node = SpawnSetTimerNode();
		if (ImGui::MenuItem("Print String"))
			node = SpawnPrintStringNode();
		ImGui::Separator();
		if (ImGui::MenuItem("Comment"))
			node = SpawnComment();

		if (node)
		{
			createNewNode = false;

			ed::SetNodePosition(node->ID, newNodePostion);

			if (auto startPin = newNodeLinkPin)
			{
				auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

				for (auto& pin : pins)
				{
					if (CanCreateLink(startPin, &pin))
					{
						auto endPin = &pin;
						if (startPin->Kind == PinKind::Input)
							std::swap(startPin, endPin);

						s_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
						s_Links.back().Color = GetIconColor(startPin->Type);

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

	ed::End();
}

bool Frame()
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

		ShowEditorCanvas();

		ImGui::End();

		ImGui::Render();

		ImGuiWrapperNS::m_rendererImpl->render();

		return true;
	}
	return false;
}

bool ImGuiWrapper::Render()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		return Frame();
	}
	return false;
}

bool ImGuiWrapper::Terminate()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->terminate();
		ImGuiWrapperNS::m_rendererImpl->terminate();
		ImGui::DestroyContext();
	}
	return true;
}