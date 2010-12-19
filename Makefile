all: sais

sais: sais.cc sais.hh
	g++ -O3 -o sais sais.cc
