#pragma once
#include <iostream>
#include <fstream>
#include <windows.h>
#include <filesystem>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdlib>

std::filesystem::path currentPath = std::filesystem::current_path();
std::string cwd = currentPath.string();
std::string logFilePath = cwd + "/VixenLauncher.log";
#ifndef logToFile
void logToConsole(const std::string& text) {

}
void ErrorToConsole(const std::string& text) {

}
void WarnToConsole(const std::string& text) {

}

void FatalToConsole(const std::string& text) {

}

void LogUnrecoverable(const std::string& text) {

    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {

        file << " [FATAL ERROR]: This is unrecoverable. " << text << std::endl;
        file.close();
        system("color 4");
        std::cout << "[FATAL ERROR] An error has occured, this error is unrecoverable and VixenFN will now close. Info: " << text << ". Exiting in 10 seconds." << std::endl;
        system("color 7");
        Sleep(10000);
        exit(0);
        return;
    }
    else {
        std::cout << "Unable to log FATAL ERROR to the log file: " << logFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}

void logToFile(const std::string& text) {


    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {

        file << " [LOG]: " << text << std::endl;
        file.close();
    }
    else {
        std::cout << "Unable to log to the log file file: " << logFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}
void warnToFile(const std::string& text) {


    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {
        file  << " [WARN]: " << text << std::endl;
        file.close();
    }
    else {
        std::cout << "Unable to log to the log file file: " << logFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}
void ErrorToFile(const std::string& text) {

    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {

        file << " [ERROR]: " << text << std::endl;
        file.close();
    }
    else {
        std::cout << "Unable to log to the log file file: " << logFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}

void FatalToFile(const std::string& text) {
    std::ofstream file(logFilePath, std::ios::app);
    if (file.is_open()) {

        file << " [Fatal]: " << text << std::endl;
        file.close();
    }
    else {
        std::cout << "Unable to log to the log file file: " << logFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}
#endif