#include <iostream>
#include <filesystem>
#include "./src/app/processes/ProcessManagement.hpp"
#include "./src/app/processes/Task.hpp"

using namespace std;

namespace fs = std::filesystem;

int main(int argc, char* argv[]){
    string directory;
    string action;

    cout << "Enter the directory path: ";
    getline(std::cin, directory);

    cout << "Enter the action (encrypt/decrypt): ";
    getline(cin, action);

    try{
        if(fs::exists(directory) && fs::is_directory(directory)){
            ProcessManagement processManagement;

            for(const auto& entry : fs::recursive_directory_iterator(directory)){
                if(entry.is_regular_file()){
                    string filePath = entry.path().string();
                    IO io(filePath);
                    fstream f_stream = std::move(io.getFileStream());

                    if(f_stream.is_open()){
                        Action taskAction = (action == "encrypt") ? Action::ENCRYPT : Action::DECRYPT;
                        auto task = std::make_unique<Task>(std::move(f_stream), taskAction, filePath);
                        processManagement.submitToQueue(std::move(task));
                    } 
                    else cout << "Unable to open file: " << filePath << std::endl;
                }
            }

            processManagement.executeTasks();
        } 
        else cout << "Invalid directory path!" << std::endl;
    }
    catch(const fs::filesystem_error& ex) 
        std::cout << "Filesystem error: " << ex.what() << std::endl;

    return 0;
}