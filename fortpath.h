#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <windows.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <fileapi.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <cpprest/details/basic_types.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;



bool checkAppInstallation(const std::string& fileLocation, const std::string& appName, std::string& installLocation) {
    std::ifstream file(fileLocation);
    if (file.is_open()) {
        // Read the contents of the file into a string
        std::string jsonString((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        // Parse the JSON string
        web::json::value jsonData = web::json::value::parse(jsonString);

        // Access the InstallationList array
        if (jsonData.has_field(U("InstallationList")) && jsonData[U("InstallationList")].is_array()) {
            auto& installationList = jsonData.at(U("InstallationList")).as_array();

            // Iterate through the installations
            for (const auto& installation : installationList) {
                if (installation.is_object()) {
                    // Check if the app name matches
                    const auto& appNameProp = installation.as_object().at(U("AppName"));
                    if (appNameProp.is_string()) {
                        std::string appNameStr = utility::conversions::to_utf8string(appNameProp.as_string());
                        if (appNameStr == appName) {
                            const auto& installLocationProp = installation.as_object().at(U("InstallLocation"));
                            if (installLocationProp.is_string()) {
                                installLocation = utility::conversions::to_utf8string(installLocationProp.as_string());
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    installLocation = "None";
    return false;
}