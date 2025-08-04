CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -I. -Isrc/app/encryptDecrypt -Isrc/app/fileHandling -Isrc/app/processes

MAIN_TARGET = encrypt_decrypt
CRYPTION_TARGET = cryption
SEQUENTIAL_TARGET = sequential_run

MAIN_SRC = main.cpp \
           src/app/processes/ProcessManagement.cpp \
           src/app/fileHandling/IO.cpp \
           src/app/fileHandling/ReadEnv.cpp \
           src/app/encryptDecrypt/Cryption.cpp

CRYPTION_SRC = src/app/encryptDecrypt/CryptionMain.cpp \
               src/app/encryptDecrypt/Cryption.cpp \
               src/app/fileHandling/IO.cpp \
               src/app/fileHandling/ReadEnv.cpp

SEQUENTIAL_SRC = sequential_main.cpp \
                 src/app/encryptDecrypt/Cryption.cpp \
                 src/app/fileHandling/IO.cpp \
                 src/app/fileHandling/ReadEnv.cpp

MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
CRYPTION_OBJ = $(CRYPTION_SRC:.cpp=.o)
SEQUENTIAL_OBJ = $(SEQUENTIAL_SRC:.cpp=.o)

all: $(MAIN_TARGET) $(CRYPTION_TARGET)

$(MAIN_TARGET): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CRYPTION_TARGET): $(CRYPTION_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(SEQUENTIAL_TARGET): $(SEQUENTIAL_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(MAIN_OBJ) $(CRYPTION_OBJ) $(SEQUENTIAL_OBJ) $(MAIN_TARGET) $(CRYPTION_TARGET) $(SEQUENTIAL_TARGET)

.PHONY: clean all sequential benchmark

sequential: $(SEQUENTIAL_TARGET)

benchmark: clean all sequential
	@echo ""
	@echo "\033[1;36m🚀 Running parallel encryption...\033[0m"
	@./encrypt_decrypt test encrypt
	@echo ""
	@echo "\033[1;36m🧪 Running sequential encryption...\033[0m"
	@./sequential_run test encrypt
