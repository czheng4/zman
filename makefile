all: zman

zman: zman.cpp
	c++ -O3 zman.cpp -o zman -std=c++11

clean:
	rm zman