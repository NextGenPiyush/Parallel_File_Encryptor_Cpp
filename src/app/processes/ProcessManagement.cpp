#include <iostream>
#include "ProcessManagement.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include "../encryptDecrypt/Cryption.hpp"
#include <sys/mman.h>
#include <atomic>
#include <semaphore.h>
#include <iomanip>  // For std::put_time
#include <ctime>    // For std::time_t, std::localtime

using namespace std;

ProcessManagement::ProcessManagement() {
    itemsSemaphore = sem_open("/items_semaphore", O_CREAT, 0666, 0);
    emptySlotsSemaphore = sem_open("/empty_slots_semaphore", O_CREAT, 0666, 1000);
    if (itemsSemaphore == SEM_FAILED || emptySlotsSemaphore == SEM_FAILED) {
        perror("Semaphore open failed");
        exit(1);
    }

    shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shmFd, sizeof(SharedMemory));
    sharedMem = static_cast<SharedMemory *>(mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
}

ProcessManagement::~ProcessManagement() {
    munmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);

    sem_unlink("/items_semaphore");
    sem_unlink("/empty_slots_semaphore");
}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task) {
    sem_wait(emptySlotsSemaphore);
    std::unique_lock<std::mutex> lock(queueLock);

    if (sharedMem->size.load() >= 1000) {
        return false;
    }

    strcpy(sharedMem->tasks[sharedMem->rear], task->toString().c_str());
    sharedMem->rear = (sharedMem->rear + 1) % 1000;
    sharedMem->size.fetch_add(1);
    lock.unlock();
    sem_post(itemsSemaphore);

    int pid = fork();
    if (pid < 0) {
        return false;
    } else if (pid > 0) {
        std::cout << "\033[1;33m🔶 Entering the parent process\033[0m\n";  // Yellow
    } else {
        std::cout << "\033[1;33m🔶 Entering the child process\033[0m\n";  // Yellow

        executeTask();

        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        cout<<endl;
        std::cout << "\033[1;32m✅ Exiting the encryption/decryption at: "
                  << std::put_time(now, "%Y-%m-%d %H:%M:%S")
                  << "\n✅ Exiting the child process\033[0m\n\n";
        exit(0);  // Make sure the child process terminates here
    }

    return true;
}

void ProcessManagement::executeTask() {
    sem_wait(itemsSemaphore);
    std::unique_lock<std::mutex> lock(queueLock);

    char taskStr[256];
    strcpy(taskStr, sharedMem->tasks[sharedMem->front]);
    sharedMem->front = (sharedMem->front + 1) % 1000;
    sharedMem->size.fetch_sub(1);
    lock.unlock();
    sem_post(emptySlotsSemaphore);

    executeCryption(taskStr);
}
