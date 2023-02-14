gcc src/inspection_console.c -lncurses -lm -o bin/inspection
gcc src/command_console.c -lncurses -o bin/command
gcc src/motorx.c -o bin/motorx
gcc src/motorz.c -o bin/motorz
gcc src/world.c -o bin/world
gcc src/master.c -o bin/master

./bin/master
