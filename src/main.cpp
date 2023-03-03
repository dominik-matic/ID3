#include <iostream>
#include <math.h>

#include "ID3.hpp"

int main(int argc, char* argv[]) {
	ID3* model;

	if(argc < 3) {
		std::cerr << "Too few arguments\n";
		exit(-1);
	}

	if(argc >= 4) {
		std::string arg;
		arg = argv[3];
		int depth = stoi(arg);
		if(depth < 0) {
			std::cerr << "Invalid depth\n";
			exit(-1);
		}
		model = new ID3(depth);
	} else {
		model = new ID3();
	}
	
	std::string trainingDataPath;
	std::string testDataPath;

	trainingDataPath = argv[1];
	testDataPath = argv[2];

	model->fit(trainingDataPath);
	model->predict(testDataPath);

	//model->printTables();

	return 0;
}