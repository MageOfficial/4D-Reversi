CXX = g++
CXXFLAGS += -Wall -Wextra -m64 -mbmi -std=c++20 -O3 -march=native -w -Isrc -Iinclude
LDFLAGS_D = -lsfml-graphics -lsfml-window -lsfml-system
LDFLAGS_R = -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

SRC_DIR = src
OBJ_DIR = obj
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

allD: $(OBJ_DIR) compileD linkD cleanobj run
allR: $(OBJ_DIR) compileR linkR cleanobj run

$(OBJ_DIR):
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

compileD: $(OBJS)

linkD:
	$(CXX) $(OBJS) -o main $(LDFLAGS_D)

compileR:
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -DSFML_STATIC" compileD

linkR:
	$(CXX) -static $(OBJS) -o mainR $(LDFLAGS_R)

clean:
	del main.exe 2>nul || true
	del mainR.exe 2>nul || true

cleanobj:
	del /Q /F $(OBJ_DIR)\*.o 2>nul || true

run:
	.\main.exe
