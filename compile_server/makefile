compile.exe:compile_server.cc
	g++ -g -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -rf ./temp/*.exe ./temp/*.stdin ./temp/*.stdout ./temp/*.stderr ./temp/*.err compile.exe