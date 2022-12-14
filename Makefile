hp:
	@echo " Compile hp_main ..."
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/record.c ./src/hp_file.c ./Logs/Logs.c -lbf -o ./build/hp_main -O2
	@echo "Running hp_main..."
	./build/hp_main

bf:
	@echo "Compile bf_main ..."
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/record.c ./Logs/Logs.c -lbf -o ./build/bf_main
	@echo "Running bf_main..."
	./build/bf_main

ht:
	@echo " Compile hp_main ..."
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c ./Logs/Logs.c -lbf -o ./build/ht_main -O2
	@echo "Running ht_main..."
	./build/ht_main

hpv:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/record.c ./src/hp_file.c ./Logs/Logs.c -lbf -o ./build/hp_main -O2
	valgrind --leak-check=full --show-leak-kinds=all ./build/hp_main

htv:
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/record.c ./src/ht_table.c ./Logs/Logs.c -lbf -o ./build/ht_main -O2
	valgrind --leak-check=full --track-origins=yes 	./build/ht_main

clean:
	@echo "Cleaning up..."
	rm /build/* hp_databases/* ht_databases/*