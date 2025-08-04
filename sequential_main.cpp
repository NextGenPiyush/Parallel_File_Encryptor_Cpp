#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <fstream>
#include "./src/app/fileHandling/IO.hpp"
#include "./src/app/fileHandling/ReadEnv.cpp"
#include "./src/app/processes/Task.hpp"
#include "./src/app/encryptDecrypt/Cryption.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]){
    std::string directory, action;

    if (argc >= 3) {
        directory = argv[1];
        action = argv[2];
    } else {
        std::cout << "Enter the directory path: ";
        std::getline(std::cin, directory);
        if (directory.empty()) std::getline(std::cin, directory);

        std::cout << "Enter the action (encrypt/decrypt): ";
        std::getline(std::cin, action);
        if (action.empty()) std::getline(std::cin, action);
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << "\n🔐 \033[1;34mStarting SEQUENTIAL encryption/decryption at: " 
              << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "\033[0m\n\n";

    try {
        if (fs::exists(directory) && fs::is_directory(directory)) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();
                    IO io(filePath);
                    std::fstream f_stream = io.getFileStream();

                    if (f_stream.is_open()) {
                        Action taskAction = (action == "encrypt") ? Action::ENCRYPT : Action::DECRYPT;
                        Task task(std::move(f_stream), taskAction, filePath);

                        std::cout << "📄 Processing: " << filePath << " ... ";
                        executeCryption(task.toString());
                        std::cout << "\033[1;32mDone.\033[0m\n";
                    } else {
                        std::cout << "\033[1;31mFailed to open file:\033[0m " << filePath << "\n";
                    }
                }
            }
        } else {
            std::cout << "\033[1;31mInvalid directory path!\033[0m\n";
        }
    } catch (const fs::filesystem_error& ex) {
        std::cout << "\033[1;31mFilesystem error: \033[0m" << ex.what() << "\n";
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "\n\033[1;32m✅ All files processed sequentially.\033[0m\n";
    std::cout << "⏱️ Total time taken: " << elapsed.count() << " seconds\n";

    return 0;
}
