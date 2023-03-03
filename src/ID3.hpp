#include <vector>
#include <map>
#include <set>

class ID3 {
	typedef std::vector<std::vector<std::string>> TABLE; 

	struct Node {
		std::string label;
		std::map<std::string, Node*> subtrees;

		Node(std::string label) : label(label) {}
		Node(std::string label, std::map<std::string, Node*> subtrees) : label(label), subtrees(subtrees) {}
	};

	int depth = -1;
	TABLE trainingData;
	TABLE testData;
	std::map<std::string, std::set<std::string>> V;
	//std::map<std::string, std::map<std::string, unsigned>> counts;
	//double totalEntropy;
	Node* root;

	std::string mostLikely;


	std::vector<std::string> parseLine(std::string, char);
	void loadData(std::string, TABLE&);

	void fit();

	Node* id3(std::vector<unsigned>&, std::vector<unsigned>&, std::vector<std::string>, int);
	std::string mostCommon(std::vector<unsigned>&);
	std::string mostDiscriminatory(std::vector<unsigned>&, std::vector<std::string>&);
	std::vector<unsigned> reduceD(std::vector<unsigned>&, std::string&, std::string&);
	std::vector<std::string> reduceX(std::vector<std::string>&, std::string&);
	bool allTheSame(std::vector<unsigned>&);
	void printBranches(Node*, unsigned, std::vector<std::string>&);

	//void printD(std::vector<unsigned>&);
	
	std::vector<std::string> predict();
	std::string predictOne(std::vector<std::string>&);
	std::string mostCommon();


	public:
	ID3() { }
	ID3(unsigned depth) : depth(depth) {}

	void fit(std::string);
	void fit(std::vector<std::vector<std::string>>&);
	std::vector<std::string> predict(std::string);
	std::vector<std::string> predict(std::vector<std::vector<std::string>>&);



	void printTables();
};