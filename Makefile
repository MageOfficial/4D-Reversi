allD: compileD linkD clean run

allR: compileR linkR clean run

FLAGS += -Wall -Wextra -m64 -mbmi -std=c++20 -O3 -march=native -w

compileD:
	g++ -c main.cpp $(FLAGS)

linkD: 
	g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system

compileR:
	g++ -c main.cpp $(FLAGS) -DSFML_STATIC 

linkR: 
	g++ main.o -o main -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

clean:
	del main.o

run:
	.\"main.exe"