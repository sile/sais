all: sais

sais: sais.cc sais.hh
	g++ -O2 -o sais sais.cc
