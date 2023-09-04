#pragma once
#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <filesystem>

std::filesystem::path LogFilePath = std::filesystem::current_path().string() + "VixenFN.log";

#ifndef V2logToFile
void V2logToFile(const std::string& text) {
    std::ofstream file(LogFilePath, std::ios::app);  // Open the log file in append mode
    if (file.is_open()) {
        file << "[LOG]" << text << std::endl;  // Append the text and a newline character
        file.close();  // Close the file
    }
    else {
        std::cout << "Unable to open the log file: " << LogFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}
#endif

#ifndef V2ErrorToFile
void V2ErrorToFile(const std::string& text) {
    std::ofstream file(LogFilePath, std::ios::app);  // Open the log file in append mode
    if (file.is_open()) {
        file << "[ERROR]" << text << std::endl;  // Append the text and a newline character
        file.close();  // Close the file
    }
    else {
        std::cout << "Unable to open the log file: " << LogFilePath << std::endl;
        // Alternatively, you can throw an exception here if desired
    }
}
#endif