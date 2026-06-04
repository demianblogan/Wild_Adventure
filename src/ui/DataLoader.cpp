#include "DataLoader.h"

#include "audio/Mixer.h"
#include "ui/Button.h"
#include "ui/Checkbox.h"
#include "ui/Element.h"
#include "ui/Image.h"
#include "ui/InteractiveElement.h"
#include "ui/Label.h"
#include "ui/Slider.h"
#include "ui/TextField.h"

#include <SFML/Graphics/Color.hpp>

#include <fstream>
#include <stdexcept>

namespace UI
{
	namespace
	{
		void ApplyCommonFields(Element& element, const nlohmann::json& data)
		{
			if (data.contains("name"))
				element.name = data["name"];
			if (data.contains("size"))
				element.size = { data["size"][0], data["size"][1] };
			if (data.contains("anchor"))
				element.anchor = { data["anchor"][0], data["anchor"][1] };
			if (data.contains("pivot"))
				element.pivot = { data["pivot"][0], data["pivot"][1] };
			if (data.contains("offset"))
				element.offset = { data["offset"][0], data["offset"][1] };
			if (data.contains("isVisible"))
				element.isVisible = data["isVisible"];
		}

		sf::Color ParseColor(const nlohmann::json& data)
		{
			return sf::Color(data[0], data[1], data[2], data[3]);
		}

		InteractionState ParseInteractionState(const std::string& name)
		{
			if (name == "Normal")
				return InteractionState::Normal;
			if (name == "Highlighted")
				return InteractionState::Highlighted;
			if (name == "Pressed")
				return InteractionState::Pressed;

			throw std::runtime_error("DataLoader: unknown interaction state '" + name + "'");
		}
	}

	DataLoader::DataLoader(Resources& resources)
		: resources(resources)
	{
		RegisterDefaultFactories();
	}

	void DataLoader::RegisterAction(const std::string& name, std::function<void()> action)
	{
		actions[name] = std::move(action);
	}

	void DataLoader::RegisterBoolAction(const std::string& name, std::function<void(bool)> action)
	{
		boolActions[name] = std::move(action);
	}

	void DataLoader::RegisterFloatAction(const std::string& name, std::function<void(float)> action)
	{
		floatActions[name] = std::move(action);
	}

	void DataLoader::RegisterStringAction(const std::string& name, std::function<void(const std::string&)> action)
	{
		stringActions[name] = std::move(action);
	}

	void DataLoader::RegisterFilter(const std::string& name, std::function<bool(char32_t)> filter)
	{
		filters[name] = std::move(filter);
	}

	std::function<void()> DataLoader::FindAction(const std::string& name) const
	{
		const auto it = actions.find(name);
		return (it != actions.end()) ? it->second : nullptr;
	}

	std::function<void(bool)> DataLoader::FindBoolAction(const std::string& name) const
	{
		const auto it = boolActions.find(name);
		return (it != boolActions.end()) ? it->second : nullptr;
	}

	std::function<void(float)> DataLoader::FindFloatAction(const std::string& name) const
	{
		const auto it = floatActions.find(name);
		return (it != floatActions.end()) ? it->second : nullptr;
	}

	std::function<void(const std::string&)> DataLoader::FindStringAction(const std::string& name) const
	{
		const auto it = stringActions.find(name);
		return (it != stringActions.end()) ? it->second : nullptr;
	}

	std::function<bool(char32_t)> DataLoader::FindFilter(const std::string& name) const
	{
		const auto it = filters.find(name);
		return (it != filters.end()) ? it->second : nullptr;
	}

	void DataLoader::SetPrefabDirectory(const std::string& directory)
	{
		prefabDirectory = directory;
	}

	const nlohmann::json& DataLoader::LoadPrefab(const std::string& name)
	{
		const auto it = prefabCache.find(name);
		if (it != prefabCache.end())
			return it->second;

		const std::string path = prefabDirectory + name + ".json";
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error("DataLoader: cannot open prefab '" + path + "'");

		nlohmann::json prefabData;
		file >> prefabData;

		if (!prefabData.is_object() || prefabData.size() != 1)
			throw std::runtime_error("DataLoader: prefab '" + name + "' must be a JSON object with one type key");

		return prefabCache[name] = std::move(prefabData);
	}

	std::unique_ptr<Element> DataLoader::LoadElement(const nlohmann::json& data)
	{
		if (!data.is_object() || data.size() != 1)
			throw std::runtime_error("DataLoader: element JSON must be an object with one key");

		const auto it = data.begin();
		const std::string typeName = it.key();
		nlohmann::json parameters = it.value();

		if (parameters.contains("prefab"))
		{
			const std::string prefabName = parameters["prefab"];
			const nlohmann::json& prefabRoot = LoadPrefab(prefabName);

			const auto prefabIt = prefabRoot.begin();
			const std::string& prefabType = prefabIt.key();
			const nlohmann::json& prefabParameters = prefabIt.value();

			if (prefabType != typeName)
				throw std::runtime_error("DataLoader: prefab '" + prefabName +
					"' is of type '" + prefabType + "' but used as '" + typeName + "'");

			parameters.erase("prefab");

			nlohmann::json merged = prefabParameters;
			merged.merge_patch(parameters);
			parameters = std::move(merged);
		}

		const auto factoryIt = factories.find(typeName);
		if (factoryIt == factories.end())
			throw std::runtime_error("DataLoader: unknown element type '" + typeName + "'");

		return factoryIt->second(*this, parameters);
	}

	std::unique_ptr<Element> DataLoader::LoadFromFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error("DataLoader: cannot open file '" + path + "'");

		nlohmann::json data;
		file >> data;

		return LoadElement(data);
	}

	void DataLoader::RegisterDefaultFactories()
	{
		factories["Element"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				auto element = std::make_unique<Element>();
				ApplyCommonFields(*element, data);

				if (data.contains("children"))
				{
					for (const auto& childData : data["children"])
						element->AddChild(loader.LoadElement(childData));
				}

				return element;
			};

		factories["Label"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				const std::string fontName = data.at("fontName");

				auto label = std::make_unique<Label>(loader.GetResources(), fontName);
				ApplyCommonFields(*label, data);

				if (data.contains("characterSize"))
					label->SetCharacterSize(data["characterSize"]);
				if (data.contains("color"))
					label->SetColor(ParseColor(data["color"]));
				if (data.contains("outlineColor"))
					label->SetOutlineColor(ParseColor(data["outlineColor"]));
				if (data.contains("outlineThickness"))
					label->SetOutlineThickness(data["outlineThickness"]);
				if (data.contains("text"))
					label->SetText(data["text"]);

				return label;
			};

		factories["Image"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				auto image = std::make_unique<Image>(loader.GetResources());
				ApplyCommonFields(*image, data);

				if (data.contains("textureName"))
					image->SetTexture(data["textureName"]);
				if (data.contains("color"))
					image->SetColor(ParseColor(data["color"]));

				return image;
			};

		factories["Button"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				auto button = std::make_unique<Button>();
				ApplyCommonFields(*button, data);

				if (data.contains("backgrounds"))
				{
					for (const auto& [stateName, slotData] : data["backgrounds"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						button->SetBackground(state, loader.LoadElement(slotData));
					}
				}

				if (data.contains("foregrounds"))
				{
					for (const auto& [stateName, slotData] : data["foregrounds"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						button->SetForeground(state, loader.LoadElement(slotData));
					}
				}

				if (data.contains("foregroundColors"))
				{
					for (const auto& [stateName, colorData] : data["foregroundColors"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						button->SetForegroundColor(state, ParseColor(colorData));
					}
				}

				std::function<void()> action;
				if (data.contains("action"))
					action = loader.FindAction(data["action"]);

				if (loader.buttonSoundMixer != nullptr)
				{
					Audio::Mixer* mixer = loader.buttonSoundMixer;
					const std::string hoverSound = loader.buttonHoverSound;
					const std::string pressSound = loader.buttonPressSound;

					if (!hoverSound.empty())
						button->SetOnHighlighted([mixer, hoverSound] { mixer->PlaySound(hoverSound); });

					button->SetOnPressed([mixer, pressSound, action]
						{
							if (!pressSound.empty())
								mixer->PlaySound(pressSound);
							if (action)
								action();
						});
				}
				else if (action)
				{
					button->SetOnPressed(std::move(action));
				}

				return button;
			};

		factories["Checkbox"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				auto checkbox = std::make_unique<Checkbox>();
				ApplyCommonFields(*checkbox, data);

				if (data.contains("backgrounds"))
				{
					for (const auto& [stateName, slotData] : data["backgrounds"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						checkbox->SetBackground(state, loader.LoadElement(slotData));
					}
				}

				if (data.contains("checkedView"))
					checkbox->SetCheckedView(loader.LoadElement(data["checkedView"]));
				if (data.contains("uncheckedView"))
					checkbox->SetUncheckedView(loader.LoadElement(data["uncheckedView"]));

				if (data.contains("checked"))
					checkbox->SetChecked(data["checked"]);

				if (data.contains("action"))
				{
					const std::string actionName = data["action"];
					auto action = loader.FindBoolAction(actionName);
					if (action)
						checkbox->SetOnCheckedChanged(std::move(action));
				}

				return checkbox;
			};

		factories["Slider"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				auto slider = std::make_unique<Slider>();
				ApplyCommonFields(*slider, data);

				if (data.contains("min") && data.contains("max"))
					slider->SetRange(data["min"], data["max"]);
				if (data.contains("step"))
					slider->SetStep(data["step"]);
				if (data.contains("value"))
					slider->SetValue(data["value"]);

				if (data.contains("track"))
					slider->SetTrack(loader.LoadElement(data["track"]));
				if (data.contains("fill"))
					slider->SetFill(loader.LoadElement(data["fill"]));

				if (data.contains("handles"))
				{
					for (const auto& [stateName, slotData] : data["handles"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						slider->SetHandle(state, loader.LoadElement(slotData));
					}
				}

				if (data.contains("activatedHandle"))
					slider->SetActivatedHandle(loader.LoadElement(data["activatedHandle"]));

				if (data.contains("action"))
				{
					const std::string actionName = data["action"];
					auto action = loader.FindFloatAction(actionName);
					if (action)
						slider->SetOnValueChanged(std::move(action));
				}

				return slider;
			};

		factories["TextField"] = [](DataLoader& loader, const nlohmann::json& data) -> std::unique_ptr<Element>
			{
				const std::string fontName = data.at("fontName");

				auto textField = std::make_unique<TextField>(loader.GetResources(), fontName);
				ApplyCommonFields(*textField, data);

				if (data.contains("backgrounds"))
				{
					for (const auto& [stateName, slotData] : data["backgrounds"].items())
					{
						const InteractionState state = ParseInteractionState(stateName);
						textField->SetBackground(state, loader.LoadElement(slotData));
					}
				}

				if (data.contains("filter"))
				{
					const std::string filterName = data["filter"];
					auto filter = loader.FindFilter(filterName);
					if (filter)
						textField->SetFilter(std::move(filter));
				}

				if (data.contains("text"))
					textField->SetText(data["text"]);

				if (data.contains("action"))
				{
					const std::string actionName = data["action"];
					auto action = loader.FindStringAction(actionName);
					if (action)
						textField->SetOnTextChanged(std::move(action));
				}

				return textField;
			};
	}

	void DataLoader::SetButtonSounds(Audio::Mixer& mixer, const std::string& hoverSoundName, const std::string& pressSoundName)
	{
		buttonSoundMixer = &mixer;
		buttonHoverSound = hoverSoundName;
		buttonPressSound = pressSoundName;
	}
}