#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <filesystem>
#include <fileapi.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include "fortpath.h"
#include "injector.h"
#include "utils.h"
#include "newlogging.h"
using namespace web;
using namespace web::http;
using namespace web::http::client;

bool UseBetaLogin = false;

const std::string currentVersion = "4.0";
const std::string cordUrl = "";
const std::string maintenanceReason = "";
const std::string latestVersion = "";
bool maintenance = false;

// Function to send HTTP request and get the response
std::string sendRequest(const std::string &url)
{
	http_client client(web::uri(utility::conversions::to_string_t(url)));
	auto response = client.request(methods::GET).get();
	if (response.status_code() == status_codes::OK)
		return response.extract_utf8string().get();
	else
		throw std::runtime_error("Failed to send HTTP request: " + std::to_string(response.status_code()));
}

// Function to parse the JSON response and extract required information
void parseResponse(const std::string &response, std::string &version, bool &maintenance, std::string &maintenanceReason, std::string &discordUrl)
{
	//std::cout << "Version being passed: " << version << std::endl;
	json::value jsonResponse = json::value::parse(response);

	if (jsonResponse.has_field(U("version")) && jsonResponse[U("version")].is_string())
	{
		version = utility::conversions::to_utf8string(jsonResponse.at(U("version")).as_string());
		//std::cout << "version: (after parse) " << version << std::endl;
	}

	if (jsonResponse.has_field(U("maintenance")) && jsonResponse[U("maintenance")].is_string())
	{
		std::string maintenanceString = utility::conversions::to_utf8string(jsonResponse.at(U("maintenance")).as_string());
		maintenance = (maintenanceString == "true");
	}

	if (jsonResponse.has_field(U("maintenance_reason")) && jsonResponse[U("maintenance_reason")].is_string())
	{
		maintenanceReason = utility::conversions::to_utf8string(jsonResponse.at(U("maintenance_reason")).as_string());
	}

	if (jsonResponse.has_field(U("discord_url")) && jsonResponse[U("discord_url")].is_string())
	{
		discordUrl = utility::conversions::to_utf8string(jsonResponse.at(U("discord_url")).as_string());
	}
}

// Function to download a file from a URL to the specified path
void downloadFile(const std::string &url, const std::string &filePath)
{
	web::http::client::http_client client(web::uri(utility::conversions::to_string_t(url)));
	concurrency::streams::ostream fileStream = concurrency::streams::file_stream<uint8_t>::open_ostream(utility::conversions::to_string_t(filePath)).get();
	auto response = client.request(methods::GET).get();
	if (response.status_code() == status_codes::OK)
		response.body().read_to_end(fileStream.streambuf()).wait();
	else
		throw std::runtime_error("Failed to download file: " + std::to_string(response.status_code()));
}

bool IsAdmin()
{
	BOOL isAdmin = FALSE;
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
	PSID adminGroup;

	if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{
		return false;
	}

	if (!CheckTokenMembership(NULL, adminGroup, &isAdmin))
	{
		FreeSid(adminGroup);
		return false;
	}

	FreeSid(adminGroup);
	return (isAdmin != 0);
}

int main()
{
	system("Color 5");
	std::cout << "Checking for updates, please wait :D" << std::endl;

	std::string url = "https://vixenfn.tk/launcherv3";
	try
	{
		std::string response = sendRequest(url);

		std::string version;
		std::string maintenanceReason;
		std::string discordUrl;
		logToFile("API RESPONSE: " + response);
		parseResponse(response, version, maintenance, maintenanceReason, discordUrl);
		logToFile("CURRENTVERSION VAR: " + currentVersion + " LATEST VERSION VAR: " + version);
		if (currentVersion != version)
		{
			system("Color A");
			std::cout << "An update is required! Current Version: " << currentVersion << " Latest Version: " << latestVersion << std::endl;
			return 0;
		}

		// Check for maintenance
		if (maintenance)
		{
			system("Color 4");
			std::cout << "VixenFN is currently under maintenance, with the reason: " << maintenanceReason << " see more information at: " << discordUrl << std::endl;
			return 0;
		}

		// Set console app title
		std::string consoleTitle = "VixenFN | " + discordUrl + " | Version " + currentVersion;
		SetConsoleTitleA(consoleTitle.c_str());
		system("Color 5");
		std::cout << "Finished checking for updates!\nPlease wait while we work our magic!" << std::endl;

		std::string tempDirPath = std::filesystem::temp_directory_path().string();
		std::string dllPath = tempDirPath + "VixenFN.dll";
		downloadFile("https://vixenfn.tk/dll", dllPath);

		std::string fileLocation = "C:\\ProgramData\\Epic\\UnrealEngineLauncher\\LauncherInstalled.dat";
		std::string appName = "Fortnite";
		std::string installLocation;
		if (!checkAppInstallation(fileLocation, appName, installLocation))
		{
			system("Color 6");
			std::cout << "We couldn't find your installation. Please verify Fortnite is installed." << std::endl;
			return 0;
		}

		installLocation += "\\FortniteGame\\Binaries\\Win64";
		V2logToFile("fort path: " + installLocation);

		std::string backupFolderPath = installLocation + "\\Vixen-Backups";
		CreateDirectoryA(backupFolderPath.c_str(), NULL);
		V2logToFile("created backups!");

		std::vector<std::string > appNamesToBackup = { "FortniteClient-Win64-Shipping_EAC.exe",
			"FortniteClient-Win64-Shipping_EAC_EOS.exe",
			"FortniteClient-Win64-Shipping_BE.exe" };

		for (const auto &appName: appNamesToBackup)
		{
			std::string sourceFilePath = installLocation + "\\" + appName;
			std::string destinationFilePath = backupFolderPath + "\\" + appName;
			try
			{
				if (!MoveFileA(sourceFilePath.c_str(), destinationFilePath.c_str()))
					throw std::runtime_error("Failed to move file: " + std::to_string(GetLastError()));
			}

			catch (const std::exception &e) {}
		}

		std::string sourceFilePath = installLocation + "\\FortniteClient-Win64-Shipping.exe";
		std::vector<std::string > duplicateAppNames = { "FortniteClient-Win64-Shipping_EAC.exe",
			"FortniteClient-Win64-Shipping_EAC_EOS.exe",
			"FortniteClient-Win64-Shipping_BE.exe" };

		for (const auto &appName: duplicateAppNames)
		{
			std::string destinationFilePath = installLocation + "\\" + appName;
			try
			{
				if (!CopyFileA(sourceFilePath.c_str(), destinationFilePath.c_str(), FALSE))
					throw std::runtime_error("Failed to copy file: " + std::to_string(GetLastError()));
			}

			catch (const std::exception &e) {}
		}

		std::cout << "ABOUT TO LAUNCH FORT!" << std::endl;
		std::string runLauncher = installLocation + "\\FortniteLauncher.exe";
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		std::string commandLauncher = "-epiclocale=en -noeac -nobe -fromfl=eac_kamu -fltoken=3db3ba5dcbd2e16703f3978d -caldera=eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJhY2NvdW50X2lkIjoiYmU5ZGE1YzJmYmVhNDQwN2IyZjQwZWJhYWQ4NTlhZDQiLCJnZW5lcmF0ZWQiOjE2Mzg3MTcyNzgsImNhbGRlcmFHdWlkIjoiMzgxMGI4NjMtMmE2NS00NDU3LTliNTgtNGRhYjNiNDgyYTg2IiwiYWNQcm92aWRlciI6IkVhc3lBbnRpQ2hlYXQiLCJub3RlcyI6IiIsImZhbGxiYWNrIjpmYWxzZX0.VAWQB67RTxhiWOxx7DBjnzDnXyyEnX7OljJm-j2d88G_WgwQ9wrE6lwMEHZHjBd1ISJdUO1UVUqkfLdU5nofBQ";
		/* std::cout << "FORT EXE  path: " << runLauncher << std::endl;
		if (CreateProcessA(runLauncher.c_str(), &commandLauncher[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		    DWORD launcherProcessId = pi.dwProcessId;
		    SuspendProcess(launcherProcessId);
		    Sleep(1500);
		    std::cout << "LAUNCHED LAUNCHER! INFO: ARG1: " << runLauncher.c_str() << ". CMD LINE ARGS: " << &commandLauncher[0] << "GONNA LAUNCH EAC" << std::endl;
		}
		else {
		    std::cout << "FAILED TO LAUNCH LAUNCHER!" << std::endl;
		    return 0;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		*/
		std::string filePath = installLocation + "\\FortniteClient-Win64-Shipping_BE.exe";
		std::string FortPathLOL = "\"" + installLocation + "\\FortniteClient-Win64-Shipping_EAC_EOS.exe\"";
		Sleep(560);
		std::string FortPATHLMFAO = "\"" + installLocation + "\\FortniteClient-Win64-Shipping.exe\"";
		DWORD FortID {};

		V2logToFile("finna launch fort at path: " + FortPATHLMFAO);
		// Path to the command prompt executable

		// Command to run in the command prompt

		std::string command = "cd /d \"" + installLocation + "\" && start FortniteClient-Win64-Shipping.exe";
		V2logToFile("COMMAND: " + command);

		system(command.c_str());
		if (CreateProcessA(filePath.c_str(), (LPSTR)
				"", NULL, NULL, FALSE, 0, NULL, installLocation.c_str(), &si, &pi))
		{
			Sleep(13000);
			DWORD beId = GetProcessIdByName("FortniteClient-Win64-Shipping_BE.exe");
			DWORD cortexId = GetProcessIdByName("CortexLauncherService.exe");
			if (cortexId != 0)
			{
				try {}

				catch (const std::exception &e)
				{
					std::string errorMessage = std::string("Cortex Failed To End (this is fine don't worry.) Error: ") + e.what();
					logToFile(errorMessage);
				}
			}

			if (beId == 0)
			{
				DWORD eacIdlol = GetProcessIdByName("FortniteClient-Win64-Shipping_EAC.exe");
				if (eacIdlol == 0)
				{
					DWORD eacEosIdlol = GetProcessIdByName("FortniteClient-Win64-Shipping_EAC_EOS.exe");
					if (eacEosIdlol == 0)
					{
						std::cout << "Fortnite was not found running. Did it crash? If it is running, please report this issue to conspiracy. :D" << std::endl;
					}
					else
					{
						FortID = eacEosIdlol;
					}
				}
				else
				{
					FortID = eacIdlol;
				}
			}
			else
			{
				FortID = beId;
			}

			DWORD eacProcessId = FortID;
			V2logToFile("LAUNCHED FORT! INFO: ARG1: " + filePath + ". CMD LINE ARGS: " + "NULL" + " YAY. ");
			SuspendProcess(FortID);
			Sleep(150);
			InjectDLL(FortID, dllPath.c_str());
			Sleep(150);
			logToFile("trying to kill epic games launcher with command: system(\"taskkill /im EpicGamesLauncher.exe /f\");");
			system("taskkill /im EpicGamesLauncher.exe /f");
			logToFile("Killed epic games launcher with command: system(\"taskkill /im EpicGamesLauncher.exe /f\");");
			Sleep(150);
			ResumeProcess(FortID);
			Sleep(1500);
			std::cout << "LAUNCHED FORT!" << std::endl;
		}
		else
		{
			std::cout << "FAILED TO LAUNCH FORT!" << std::endl;
			return 0;
		}

		//   CloseHandle(pi.hProcess);
		//  CloseHandle(pi.hThread);
		DWORD eacProcessId = FortID;
		Sleep(3500);
		if (FortID != 0)
		{
			HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, FortID);

			if (processHandle != NULL)
			{
				DWORD waitResult = WaitForSingleObject(processHandle, INFINITE);

				if (waitResult == WAIT_OBJECT_0)
				{
				 		// system("taskkill /im FortniteClient-Win64-Shipping_EAC_EOS.exe /f");
					//  system("taskkill /im FortniteClient-Win64-Shipping_EAC.exe /f");
					//   system("taskkill /im FortniteClient-Win64-Shipping_BE.exe /f");
					// system("taskkill /im FortniteLauncher.exe /f");
					// system("taskkill /im FortniteClient-Win64-Shipping.exe /f");
					V2logToFile("Fort Process  has exited.");
				}
				else {}

				CloseHandle(processHandle);
			}
			else
			{
				std::cout << "Failed to open process with ID " << FortID << std::endl;
			}
		}
		else
		{
			std::cout << "Process '" << FortID << "' not found." << std::endl;
		}
	}

	catch (const std::exception &e)
	{
		system("Color 6");
		std::cout << "An error occurred: " << e.what() << std::endl;
		return 1;
	}

	Sleep(INFINITE);
	return 0;

}