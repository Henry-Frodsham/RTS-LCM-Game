//Copyright © 2025 Henry Frodsham
#pragma once
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ErrorReporter.h"

// universal config class to handle any config file
// each instance is unique to a config file
// e.g InputTranslator will need an instance to control input sensitivity
class ConfigManager {
public:
	// each instance of the game can have seperate configs for different things
	// however, not everything is seperated into instances
	// updating each ConfigManager is impractical so just use the callers reporter
	ConfigManager(std::string BaseName, ErrorReporter* ParentReporter, std::string InstanceName = "");

    template<typename DataType>
    DataType GetValueOrDefault(std::string Key) {
        
        if (CustomValues.contains(Key)) {
            try {
                return CustomValues[Key].get<DataType>();
            }
            catch (const std::exception& e) {
                Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::CUSTOM_JSON_VALUE_FAILURE, fmt::format("failed to retrieve custom value from key {} in {}, falling back to default \n original error : {}",
                    Key, ConfigName, e.what())));
            }
        }

        if (DefaultValues.contains(Key)) {
            try {
                return DefaultValues[Key].get<DataType>();
            }
            catch (const nlohmann::json::exception& e) {
                Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::DEFAULT_JSON_VALUE_FAILURE, fmt::format("failed to retrieve default value from key {} in {} \n original error : {}",
                    Key, ConfigName, e.what())));
                return DataType{};
            }
        }
        
        Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_VALUE_MISSING_KEYS, fmt::format("no corresponding values could be found for key {} in {}", 
            Key, ConfigName)));
        return DataType{};
    }

    template<typename DataType>
    void UpdateValue(std::string Key, DataType NewData) {
        if (CustomValues.contains(Key)) {
            CustomValues[Key] = NewData;
            SaveFiles();
            return;
        }
        Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_VALUE_MISSING_KEYS, fmt::format("failed to update value key {} in {}",
            Key, ConfigName)));
    }

	void LoadOrReload();
    void SaveFiles();

private:
	nlohmann::json DefaultValues;
	nlohmann::json CustomValues;
	
	ErrorReporter* Reporter;
	std::string ConfigName;
	std::string NameExtension;

    std::filesystem::path DefaultPath;
    std::filesystem::path CustomPath;

	nlohmann::json OpenAndParse(std::filesystem::path JsonPath);


};