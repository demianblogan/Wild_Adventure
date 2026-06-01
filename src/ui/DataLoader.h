#pragma once

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

struct Resources;

namespace Audio
{
	class Mixer;
}

namespace UI
{
	class Element;

	class DataLoader
	{
	public:
		using ElementFactory = std::function<std::unique_ptr<Element>(DataLoader&, const nlohmann::json&)>;

		DataLoader(Resources& resources);

		void RegisterAction(const std::string& name, std::function<void()> action);
		void RegisterBoolAction(const std::string& name, std::function<void(bool)> action);
		void RegisterFloatAction(const std::string& name, std::function<void(float)> action);
		void RegisterStringAction(const std::string& name, std::function<void(const std::string&)> action);

		void RegisterFilter(const std::string& name, std::function<bool(char32_t)> filter);

		std::unique_ptr<Element> LoadElement(const nlohmann::json& data);
		std::unique_ptr<Element> LoadFromFile(const std::string& path);

		Resources& GetResources() { return resources; }

		std::function<void()> FindAction(const std::string& name) const;
		std::function<void(bool)> FindBoolAction(const std::string& name) const;
		std::function<void(float)> FindFloatAction(const std::string& name) const;
		std::function<void(const std::string&)> FindStringAction(const std::string& name) const;
		std::function<bool(char32_t)> FindFilter(const std::string& name) const;

		void SetPrefabDirectory(const std::string& directory);
		void SetButtonSounds(Audio::Mixer& mixer, const std::string& hoverSoundName, const std::string& pressSoundName);

	private:
		void RegisterDefaultFactories();
		const nlohmann::json& LoadPrefab(const std::string& name);

		Resources& resources;

		Audio::Mixer* buttonSoundMixer = nullptr;
		std::string buttonHoverSound;
		std::string buttonPressSound;

		std::string prefabDirectory = "data/ui/prefabs/";
		std::unordered_map<std::string, nlohmann::json> prefabCache;

		std::unordered_map<std::string, ElementFactory> factories;
		std::unordered_map<std::string, std::function<void()>> actions;
		std::unordered_map<std::string, std::function<void(bool)>> boolActions;
		std::unordered_map<std::string, std::function<void(float)>> floatActions;
		std::unordered_map<std::string, std::function<void(const std::string&)>> stringActions;
		std::unordered_map<std::string, std::function<bool(char32_t)>> filters;
	};
}