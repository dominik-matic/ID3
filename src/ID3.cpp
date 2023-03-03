#include <iostream>
#include <list>
#include <fstream>
#include <algorithm>
#include <math.h>
#include "ID3.hpp"

ID3::Node* ID3::id3(std::vector<unsigned>& D, std::vector<unsigned>& DParent, std::vector<std::string> X, int depth) {
	std::string v, x;
	if(D.size() == 0) {
		v = mostCommon(DParent);
		return new Node(v);
	}
	v = mostCommon(D);
	if(depth == 0) {
		return new Node(v);
	}

	if(X.size() == 0 or allTheSame(D)) {
		return new Node(v);
	}
	x = mostDiscriminatory(D, X);
	std::map<std::string, Node*> subtrees;
	for(auto v : V[x]) {
		auto newD = reduceD(D, x, v);
		auto newX = reduceX(X, x);
		Node* n = id3(newD, D, newX, depth - 1);
		subtrees.insert(std::make_pair(v, n));
	}
	Node* ret = new Node(x, subtrees);
	return ret;
}

/*void ID3::printD(std::vector<unsigned>& D) {
	std::cout << "######################################\n";

	for(auto ind : D) {
		for(auto kurac : trainingData[ind]) {
			std::cout << kurac << " ";
		}
		std::cout << "\n";
	}
	std::cout << "######################################\n";
}*/

void ID3::fit() {
	mostLikely = "";
	std::vector<unsigned> D;
	std::vector<unsigned> DParent;
	
	std::vector<std::string> X = trainingData[0];
	size_t rowLen = X.size();
	X.pop_back();

	/*popuni početne D i DParent, V mapu i izračunaj entropies*/
	for(size_t i = 1; i < trainingData.size(); ++i) {
		D.push_back(i);
		DParent.push_back(i);

		for(size_t j = 0; j < rowLen; ++j) {
			V[X[j]].insert(trainingData[i][j]);
		}
	}
	root = id3(D, DParent, X, depth);


	std::vector<std::string> path;
	std::cout << "[BRANCHES]:\n";
	printBranches(root, 1, path);
}


std::vector<std::string> ID3::predict() {
	std::vector<std::string> correct;
	std::vector<std::string> result;

	for(size_t i = 1; i < testData.size(); ++i) {
		auto row = testData[i];
		correct.push_back(row.back());
		row.pop_back();
		result.push_back(predictOne(row));
	}

	std::cout << "[PREDICTIONS]: ";
	for(size_t i = 0; i < result.size() - 1; ++i) {
		std::cout << result[i] << " ";
	}
	std::cout << result.back() << "\n";



	std::set<std::string> cMatrixElems;
	double accuracy;
	unsigned correctCount = 0;
	for(size_t i = 0; i < result.size(); ++i) {
		cMatrixElems.insert(result[i]);
		if(result[i] == correct[i]) {
			++correctCount;
		} else {
			cMatrixElems.insert(correct[i]);
		}
	}

	accuracy = (double) correctCount / (double) result.size();
	printf("[ACCURACY]: %.5lf\n", accuracy);

	size_t cMatDim = cMatrixElems.size();
	std::list<std::string> cMatrixElemsSorted;
	for(auto elem : cMatrixElems) {
		cMatrixElemsSorted.push_back(elem);
	}
	cMatrixElems.clear();
	cMatrixElemsSorted.sort();

	std::map<std::string, size_t> elemToIndex;
	size_t index = 0;
	for(auto elem : cMatrixElemsSorted) {
		elemToIndex[elem] = index++;
	}

	unsigned confusionMatrix[cMatDim][cMatDim];
	for(size_t i = 0; i < cMatDim; ++i) {
		for(size_t j = 0; j < cMatDim; ++j) {
			confusionMatrix[i][j] = 0;
		}
	}
	for(size_t i = 0; i < result.size(); ++i) {
		++confusionMatrix[elemToIndex[correct[i]]][elemToIndex[result[i]]];
	}

	std::cout << "[CONFUSION_MATRIX]:\n";
	for(size_t i = 0; i < cMatDim; ++i) {
		size_t j = 0;
		for(; j < cMatDim - 1; ++j) {
			std::cout << confusionMatrix[i][j] << " ";
		}
		std::cout << confusionMatrix[i][j] << "\n";
	}
	
	//std::vector<std::pair<std::string, std::map<std::string, unsigned>>> cMatrix;
	//for(auto elem : cMatrixRaw) {
	//	cMatrix.push_back(elem);
	//}
	//std::sort(cMatrix.begin(), cMatrix.end(),
	//[](const std::pair<std::string, std::map<std::string, unsigned>> &l,
	//const std::pair<std::string, std::map<std::string, unsigned>> &r){
	//	return l.first < r.first;
	//});
	//cMatrix.clear();



	return result;
}

std::string ID3::predictOne(std::vector<std::string>& properties) {
	Node* n;
	for(n = root; n->subtrees.size() != 0 ;) {
		// odredi index aktualnog propertyja
		unsigned ind = 0;
		for(; testData[0][ind] != n->label; ++ind);
		//odredi property
		std::string property = properties[ind];
		// pomakni pointer na iduci cvor
		auto it = n->subtrees.find(property);
		if(it == n->subtrees.end()) {
			return mostCommon();
		}
		n = n->subtrees[property]; // risky business, moze doc do segfaulta
	}


	return n->label;
}


void ID3::printBranches(Node* n, unsigned depth, std::vector<std::string>& path) {
	if(n->subtrees.size() == 0) {
		for(auto str : path) {
			std::cout << str << " ";
		}
		std::cout << n->label << "\n";
		return;
	}

	std::string prefix = std::to_string(depth) + ":" + n->label + "=";
	for(auto next : n->subtrees) {
		path.push_back(prefix + next.first);
		printBranches(next.second, depth + 1, path);
		path.pop_back();
	}
}

std::string ID3::mostCommon(std::vector<unsigned>& ind) {
	std::map<std::string, unsigned> m;
	std::list<std::string> result; // mozda isti broj
	unsigned max = 0;
	unsigned lastCell = trainingData[0].size() - 1;
	for(auto i : ind) {
		++m[trainingData[i][lastCell]];
	}
	for(auto entry : m) {
		if(entry.second == max) {
			result.push_back(entry.first);
		} else if(entry.second > max) {
			max = entry.second;
			result.clear();
			result.push_back(entry.first);
		}
	}
	result.sort();
	return result.front();
}

std::string ID3::mostCommon() {
	if(mostLikely.size() > 0) {
		return mostLikely;
	}
	std::map<std::string, unsigned> m;
	std::list<std::string> result;
	unsigned max = 0;
	unsigned lastCell = trainingData[0].size() - 1;
	for(size_t i = 1; i < trainingData.size(); ++i) {
		++m[trainingData[i][lastCell]];
	}

	for(auto entry : m) {
		if(entry.second == max) {
			result.push_back(entry.first);
		} else if(entry.second > max) {
			max = entry.second;
			result.clear();
			result.push_back(entry.first);
		}
	}
	result.sort();
	mostLikely = result.front();
	return mostLikely;
}

std::string ID3::mostDiscriminatory(std::vector<unsigned>& D, std::vector<std::string>& X) {

	/* Odredi koje sve stupce treba uzeti u obzir, pretpostavka je da je
	X sortiran kao i trainingData[0] */
	unsigned XInd = 0;
	std::vector<unsigned> cols;
	for(size_t i = 0; i < trainingData[0].size(); ++i) {
		if(trainingData[0][i] == X[XInd]) {
			cols.push_back(i);
			++XInd;
		}
	}

	unsigned lastCol = trainingData[0].size() - 1;
	std::map<std::string, unsigned> classToCount;
	std::map<std::string, std::map<std::string, std::map<std::string, unsigned>>> counts;

	for(unsigned row : D) {
		++classToCount[trainingData[row][lastCol]];
		for(unsigned col : cols) {
			++counts[trainingData[0][col]][trainingData[row][col]][trainingData[row][lastCol]];
		}
	}

	double totalRows = D.size();
	double totalEntropy = 0;

	for(auto entry : classToCount) {
		double fraction = (double) entry.second / totalRows;
		totalEntropy -= (fraction * std::log2(fraction));
	}
	
	//std::cout << "totalEntropy: " << totalEntropy << "\n";

	std::list<std::string> x;
	double maxIG = 0;
	double IG;

	for(auto c : X) {
		IG = totalEntropy;
		for(std::string property : V[c]) { // suncano, oblacno...
			auto cTc = counts[c][property];	// map<string, unsigned> (da, 2), (ne, 3)
			double divisor = 0;
			double total = 0;
			
			for(auto entry : cTc) {
				divisor += entry.second;
			}
			for(auto entry : cTc) {
				double fraction = (double) entry.second / divisor;
				total -= (fraction * std::log2(fraction));
			}
			IG -= (divisor / totalRows) * total;
		}
		if(IG == maxIG) {
			x.push_back(c);
		} else if(IG > maxIG) {
			maxIG = IG;
			x.clear();
			x.push_back(c);
		}
		std::cout << "IG(" << c << "): " << IG << " ";
	} std::cout << "\n";
	
	x.sort();
	return x.front();
}

std::vector<unsigned> ID3::reduceD(std::vector<unsigned>& D, std::string& x, std::string& v) {
	std::vector<unsigned> reduced;
	
	unsigned index = 0;
	for(auto elem : trainingData[0]) {
		if(elem == x) {
			break;
		}
		++index;
	}

	for(auto ind : D) {
		if(trainingData[ind][index] == v) {
			reduced.push_back(ind);
		}
	}

	return reduced;
}

std::vector<std::string> ID3::reduceX(std::vector<std::string>& X, std::string& x) {
	std::vector<std::string> reduced;
	for(auto s : X) {
		if(s != x) {
			reduced.push_back(s);
		}
	}
	return reduced;
}

bool ID3::allTheSame(std::vector<unsigned>& D) {
	unsigned lastCol = trainingData[0].size() - 1;
	std::string toCheck = trainingData[D[0]][lastCol];
	for(auto index : D) {
		if(trainingData[index][lastCol] != toCheck) {
			return false;
		}
	}
	return true;
}

/* Splitta liniju po delimiteru i vraća je kao vector stringova*/
std::vector<std::string> ID3::parseLine(std::string line, char delimiter) {
	std::vector<std::string> result;
	std::string token;

	size_t lastBeginning = 0;
	for(size_t i = lastBeginning; i < line.size(); ++i) {
		if(line[i] == delimiter) {
			token = line.substr(lastBeginning, i - lastBeginning);
			result.push_back(token);
			lastBeginning = i + 1;
		}
	}
	result.push_back(line.substr(lastBeginning));

	return result;
}

/* Učitava data iz file-a u table*/
void ID3::loadData(std::string filename, TABLE& table) {
	std::fstream file;
	file.open(filename, std::ios::in);
	if(!file.is_open()) {
		std::cerr << "Could not open file: " << filename << std::endl;
		exit(-1);
	}

	std::string line;
	while(getline(file, line)) {
		table.push_back(parseLine(line, ','));
	}
}

/* fit iz file-a */
void ID3::fit(std::string filename) {
	loadData(filename, trainingData);
	fit();
}

/* fit iz data */
void ID3::fit(std::vector<std::vector<std::string>>& data) {
	trainingData = data;
	fit();
}

/* predict iz file-a*/
std::vector<std::string> ID3::predict(std::string filename) {
	loadData(filename, testData);
	return predict();
}

/* predict iz data*/
std::vector<std::string> ID3::predict(std::vector<std::vector<std::string>>& data) {
	testData = data;
	return predict();
}







void ID3::printTables() {
	std::cout << "Training data:\n";
	for(auto row : trainingData) {
		for(auto cell : row) {
			std::cout << cell << "\t";
		}
		std::cout << "\n";
	}

	std::cout << "Test data:\n";
	for(auto row : testData) {
		for(auto cell : row) {
			std::cout << cell << "\t";
		}
		std::cout << "\n";
	}
	
}