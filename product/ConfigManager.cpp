//Copyright © 2025 Henry Frodsham
#include "ConfigManager.h"

ConfigManager::ConfigManager(std::string BaseName, ErrorReporter* ParentReporter, std::string InstanceName)
	: ConfigName(BaseName)
	, Reporter(ParentReporter)
	, NameExtension(InstanceName)
{
    std::filesystem::path CustomPath = std::filesystem::path(SOLUTION_DIR) / "config" / "custom" / ConfigName / NameExtension;
    std::filesystem::path DefaultPath = std::filesystem::path(SOLUTION_DIR) / "config" / "default" / ConfigName;
	LoadOrReload();
}


void ConfigManager::LoadOrReload() {

	if (!std::filesystem::exists(CustomPath)) {
		Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_FILE_NOT_FOUND, fmt::format("the custom json file with specified path {}, doesnt exist", CustomPath.string())));
	}
	else {
		CustomValues = OpenAndParse(CustomPath);
	}

	if (!std::filesystem::exists(DefaultPath)) {
		Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_FAILURE, fmt::format("no default config could be recovered at {}, unable to provide any value ", DefaultPath.string())));
	}
	else {
		DefaultValues = OpenAndParse(DefaultPath);
	}

	

}

nlohmann::json ConfigManager::OpenAndParse(std::filesystem::path JsonPath) {
	nlohmann::json Data = NULL;
	try {
		std::ifstream File(JsonPath);

		if (!File.is_open()) {
			Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_UNOPENED_FILE, fmt::format("unable open the json file {}", JsonPath.string())));
			return Data;
		}

		Data = nlohmann::json::parse(File);
	}
	catch (const nlohmann::json::parse_error& e) {
		Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_PARSE_ERROR, fmt::format("unable to parse the json file {}, invalid syntax? \n original error: {}", JsonPath.string(), e.what())));
	}
	catch (std::exception& e){
		Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_PARSE_ERROR, fmt::format("an unexpected error ({}) has occured whilst parsing json file {} \n original error: {}",typeid(e).name(), JsonPath.string(), e.what())));
	}

	return Data;
}

void ConfigManager::SaveFiles() {


    try {
        if (!CustomPath.parent_path().empty()) {
            std::filesystem::create_directories(CustomPath.parent_path());
        }

        if (!DefaultPath.parent_path().empty()) {
            std::filesystem::create_directories(DefaultPath.parent_path());
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_FAILURE,
            fmt::format("failed to create directories for config files \n original error: {}", e.what())));
        return;
    }

    if (!CustomValues.empty() && !CustomValues.is_null()) {
        try {
            std::ofstream CustomFile(CustomPath);
            if (!CustomFile.is_open()) {
                Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_UNOPENED_FILE,
                    fmt::format("unable to open custom config file {} for writing", CustomPath.string())));
            }
            else {
                // viewable format with 4 space indents
                CustomFile << CustomValues.dump(4);
                CustomFile.close();
            }
        }
        catch (const std::exception& e) {
            Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_FAILURE,
                fmt::format("failed to write custom config file {} \n original error: {}",
                    CustomPath.string(), e.what())));
        }
    }

    if (!DefaultValues.empty() && !DefaultValues.is_null()) {
        try {
            std::ofstream DefaultFile(DefaultPath);
            if (!DefaultFile.is_open()) {
                Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_UNOPENED_FILE,
                    fmt::format("unable to open default config file {} for writing", DefaultPath.string())));
            }
            else {
                // 
                DefaultFile << DefaultValues.dump(4);
                DefaultFile.close();
            }
        }
        catch (const std::exception& e) {
            Reporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::JSON_FAILURE,
                fmt::format("failed to write default config file {} \n original error: {}",
                    DefaultPath.string(), e.what())));
        }
    }
}