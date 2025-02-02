allD: compileD linkD clean run

allR: compileR linkR clean run

FLAGS += -Wall -Wextra -m64 -mbmi -std=c++20 -O3 -march=native -w

compileD:
	g++ -c main.cpp -IC:\Users\maxbr\Downloads\SFML-master\SFML-master\include $(FLAGS) 

linkD: 
	g++ main.o -o main -LC:\Users\maxbr\Downloads\SFML-master\SFML-Build\lib -lsfml-graphics -lsfml-window -lsfml-system 

compileR:
	g++ -c main.cpp -IC:\Users\maxbr\Downloads\SFML-master\SFML-master\include $(FLAGS) -DSFML_STATIC 

linkR: 
	g++ -static main.o -o mainmak -LC:\Users\maxbr\Downloads\SFML-master\SFML-Build\lib -LC:\Users\maxbr\Downloads\SFML-master\SFML-master\extlibs\libs-mingw\x64 -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

clean:
	del main.o

run:
	.\"main.exe"