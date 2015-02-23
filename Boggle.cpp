#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <stack>
#include <vector>
#include <bitset>


///////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF PATRICIA TRIE TO STORE THE DICTIONARY VALUES//
///////////////////////////////////////////////////////////////////

static int N = 0;
struct StringMatchNode;
// enum to check if the current word is a prefix or a complete word in 
// the dictionary.
// PREFIX - string is a PREFIX
// WORD   - string is a complete WORD
// NONE   - string is neither a PREFIX nor WORD
enum e_StringMatch
{
	WORD, PREFIX, PREFIXWORD, NONE
};



// struct to get the Information on the bitLocation where the 
// input string and string at the current node differ
// bitLocation - bit position where strings differ
// stringPos   - character postion in the strings where the two strings differ
// b_value     - if the input string has 0 or 1 at bitLocation
struct ByteInfo
{
	int bitLocation;
	int stringPos;
	bool b_value;

	ByteInfo(int bitLoc_, int strPos_, bool b_val):
		bitLocation(bitLoc_), stringPos(strPos_), b_value(b_val){}
};

//
// Patricia Trie class
//
// This class is designed as a radix tree class with bitwise comparison for
// strings increasing space efficiency as strings are stored at leaves
// and only bit indexes and common suffix are stored in the intermediate nodes
class Pat_trie
{
public:
// constructor
	Pat_trie():root(NULL){};
// destructor
	~Pat_trie(){ if(root != NULL){ delete root; root = NULL; } }
	//if(parent != NULL){ delete parent; parent = NULL; } }

// public member function
	void addWord(const std::string& input_string);
	bool searchWord(const std::string& input_string);
	StringMatchNode* ifContains(const std::string& input_string);


// private structure for Node information of the Tree
	struct Node
	{
		std::string value;
		bool isLeaf;
		int index;
		Node* left;
		Node* right;
		Node* add(std::string s,int depth);

		Node(std::string input_string):value(input_string),isLeaf(true),
			index(-1),left(NULL),right(NULL){}

		Node():value(""),isLeaf(true),
			index(-1),left(NULL),right(NULL){}

	};
	Node* root;
	
StringMatchNode* ifContains(Node* nptr,const std::string& input_string, int noOfCharMatch);
	
private:
// Private member functions
void addWord(Node* nptr, const std::string& input_string);
Node* createnewNode(const std::string& str1, const std::string& str2);
Node* getNodePositionto_InsertNewNode(Node* nptr,Node* newNode, const std::string& input_string);
int findPosition(int index, const std::string& input_string);


};

Pat_trie* pat_trie = new Pat_trie();

struct StringMatchNode
{
	e_StringMatch e_strMatch;
	Pat_trie::Node* node;
	int noOfCharMatch;

	StringMatchNode(e_StringMatch e_stringmatch, Pat_trie::Node* node_,int noOfCharMatch_):
			e_strMatch(e_stringmatch),node(node_),noOfCharMatch(noOfCharMatch_){}
};

//
// Function : ByteInfo* getByteInfo
//
// Get the bit information of two strings
// If one string 
ByteInfo* getByteInfo(const std::string& string_at_node, const std::string& input_string)
{
	int pos=0;
	
	// If the string at the tree node is empty then both the strings
	// differ at 1st character position and 1st bit postion of the
	// first character position
	if(string_at_node.empty())
		return(new ByteInfo(0,0,std::bitset<8>(input_string[0]).at(0)));
		
	// If none of the strings is empty
	// Get the first postion where the two strings differ
	while(string_at_node.c_str()[pos] == input_string.c_str()[pos]){pos++;}
	
	// If one string is a prefix of another string
	if(((signed int) pos >= (signed int) string_at_node.length())  
		|| ((signed int) pos >= (signed int) input_string.length()))
	{
		if((signed int)pos >=  (signed int)input_string.length())
			return(new ByteInfo(pos*8, pos, false));
		else
			return(new ByteInfo(pos*8, pos,std::bitset<8>(input_string[pos]).at(0)));
	}

	std::bitset<8> stringbit_node(string_at_node[pos]);
	std::bitset<8> stringbit_input(input_string[pos]);

	// If the strings are different ( no prefixing )
	// There is a single character where the strings first differ
	// Getting the first bit where the characters differ.
	for(int i= 0; i <8; i++)
		if(stringbit_node[i]^stringbit_input[i])
			return(new ByteInfo((pos)* 8 + i, pos, stringbit_input[i]));
}

//
// Function -- Pat_trie::addWord(std::string input_string)
// Arguments -- std::string 
//
// Function is called to add a new to the tree
// If root is empty new string becomes the root
// Else private member function is called passing
// in root and the input string as arguments
void Pat_trie::addWord(const std::string& input_string)
{
	if(root == NULL)
	{
		Node* newNode = new Node(input_string);
		newNode->isLeaf = true;
		root = newNode;
		return;
	}
	else
		addWord(root,input_string);
}
//
// Function -- Pat_trie::findPosition
// Arguments -- int, std::string
//
// Function to find the Bit of the input string at 
// the given location index. It returns either 0 or 1
int Pat_trie::findPosition(int index, const std::string& input_string)
{
	if(index/8 > input_string.length())
		return 0;

	return(std::bitset<8>(input_string[index/8]).at(index%8));
}

// Function -- Pat_trie::ifContains
// Arguments -- std::string
//
// Function to find if the given string is contained in the tree
StringMatchNode* Pat_trie::ifContains(const std::string& input_string)
{
	return(ifContains(root,input_string,0));
}

//
// Function -- e_StringMatch Pat_trie::ifContains
//
// Function to check if the input string 'string_input'
// is contained in the tree as a PREFIX or a complete WORD
// or neither of these returning NONE
StringMatchNode* Pat_trie::ifContains(Node* nptr, const std::string& input_string,int noOfCharMatch)
{
	// If the node value is empty, then it is one of the intermediate node
	// and with the bitcomparison of the index value of the node and the
	// corresponding bit of the string, we continue in that branch
	// i.e if 0 , proceed to left
	//     if 1 , proceed to right
	if(nptr->value.empty())
	{ 
		if(findPosition(nptr->index, input_string))
			ifContains(nptr->right,input_string,noOfCharMatch);
		else
			ifContains(nptr->left,input_string,noOfCharMatch);
	}
	// check if the input string is a prefix or a complete word inside a dictionary
	else
	{
		// check for the position where the node value and input string differs
		int in_len = input_string.length()-noOfCharMatch; 
		int n_len = nptr->value.length()-noOfCharMatch;
		int k=noOfCharMatch;
		while(in_len > 0 && n_len > 0 && input_string[k] == nptr->value[k]){in_len--; n_len--; k++;}

		// if the input string length is lesser than the node values length 
		// and matches with the prefix of the node value
		// then the string is a prefix
		if(in_len == 0 && (!nptr->isLeaf || (nptr->isLeaf && k < nptr->value.length())))
		{
			if(nptr->left != NULL && nptr->right != NULL && 
				((strcmp(nptr->left->value.c_str(),input_string.c_str())==0) 
				|| (strcmp(nptr->right->value.c_str(),input_string.c_str())==0)))
				return(new StringMatchNode(PREFIXWORD,nptr,k));
			
			return(new StringMatchNode(PREFIX, nptr,k));
		}
		// if the node is a leaf and the string and node value matches exactly
		// then the string is a word in the dictionary
		else if (nptr->isLeaf && in_len==0 && k == nptr->value.length())
			return(new StringMatchNode(WORD,nptr,k));
		// if none of the above conditions satisfy and the node is a leaf
		// already then the input string will not be matched with any other
		// string and hence not conatained in the dictionary as either
		// prefix nor word
		else if (nptr->isLeaf)
			return(new StringMatchNode(NONE,nptr,k));
		// else proceed to the next node, as there is no common prefix for the
		// node underneath the current node
		else
		{
			if(findPosition(nptr->index, input_string))
				ifContains(nptr->right,input_string,k);
			else
				ifContains(nptr->left,input_string,k); 
		}
	}
}

//
// Function -- Pat_trie::createnewNode
// Arguments -- const std::string& , const std::string& 
//
// Function is to create a new node 
// If there is a common prefix between the string, it goes into the node value
// the index holds the bit position where the strings differ
// the input string node goes into the the left or right node of the
// newnode corresponding to the bit at the index postion of the string
// if 0 - input string goes to the left node
// if 1 - input string goes to the right node
Pat_trie::Node* Pat_trie::createnewNode(const std::string& string_at_node, const std::string& input_string)
{
	Node* newNode = new Node();
	ByteInfo* byteInfo = getByteInfo(string_at_node, input_string);
	newNode->value = string_at_node.substr(0,byteInfo->stringPos);
	newNode->index = byteInfo->bitLocation;
	newNode->isLeaf = false;
			
	if(byteInfo->b_value)
		newNode->right = new Node(input_string);
	else
		newNode->left = new Node(input_string);
	
	delete byteInfo;

	return newNode;
}

//
// Function -- Pat_trie::getNodePositionto_InsertNewNode
// Arguments -- Node*, Node*, const std::string
//
// This function is to get the position in the tree to insert 
// the newly created node either an intermediate node or
// a prefix node
// Input
//  nptr -- passed as root
//  newNode -- the newlyCreatedNode 
//  
Pat_trie::Node* Pat_trie::getNodePositionto_InsertNewNode(Node* nptr, Node* newNode, const std::string& input_string)
{
	Node* parent = root;
	bool found = false;
	while(!found)
	{
		if(findPosition(parent->index, input_string))
		{
			if(parent->right->index < newNode->index && parent->right != nptr)
				parent = parent->right;
			else
				found = true;
		}
		else
		{
			if(parent->left->index < newNode->index && parent->left != nptr)
				parent = parent->left;
			else
				found = true;
		}
		return parent;
	}
}

//
// Function -- Pat_trie::addWord
// Arguments -- Node*, std::string
//
// Function to addnew word to existing Tree. It is a 
// private member function of the tree
void Pat_trie::addWord(Node* nptr,const std::string& input_string)
{
	if(!nptr->isLeaf)
	{
		if(findPosition(nptr->index, input_string))
			addWord(nptr->right, input_string);
		else
			addWord(nptr->left, input_string);
	}
	else
	{
		if(std::strcmp(nptr->value.c_str(), input_string.c_str())==0)
			return;

		Node* newNode = createnewNode(nptr->value, input_string);
			
		if(root->isLeaf)
		{
			if(newNode->left == NULL)
				newNode->left = nptr;
			else
				newNode->right = nptr;
			root = newNode;
		}
		else
		{
			Node* parent = getNodePositionto_InsertNewNode(nptr, newNode, input_string);
			if(parent == root && newNode->index < root->index)
			{
				if(newNode->left == NULL)
					newNode->left = root;
				else
					newNode->right = root;
				root = newNode;
			}
			else
			{
				if(findPosition(parent->index,input_string) == 0)
				{
					if(newNode->left == NULL)
						newNode->left = parent->left;
					else
						newNode->right = parent->left;
					parent->left = newNode;
				}
				else
				{
					if(newNode->left == NULL)
						newNode->left = parent->right;
					else
						newNode->right = parent->right;
					parent->right = newNode;
				}
			}
		}
	}
}
			


////////////////////////////////////////////////////////////////
///////        BOGGLE GAME BOARD       ////////////////////////
///////////////////////////////////////////////////////////////


// function to recursively find out if the word generated
// is contained in the dictionary as a prefix or the whole word

// Array to find the next index position from the current position
// in the 2D array
const static int nextPosX[8] = {-1,-1,-1, 0, 0, 1,1,1};
const static int nextPosY[8] = {-1, 0, 1,-1, 1,-1,0,1};

//
// structure to get the Board Position
//
struct Position
{
	int x;
	int y;

	Position(int X, int Y):x(X),y(Y){}
	Position(){}
};

//
// Function to get all the valid board positions from the current position
//
std::vector<Position*> getAllvalidNextPosition(Position* curr_position, bool** hasVisited)
{
	std::vector<Position*> next_pos;
	next_pos.clear();
	for(int i=0; i<8; i++)
	{
		int pos_X = curr_position->x+nextPosX[i];
		int pos_Y = curr_position->y+nextPosY[i];
		if((pos_X < 3 && pos_X>= 0 && pos_Y < 3 && pos_Y >= 0 )&&(!hasVisited[pos_X][pos_Y]))
			next_pos.push_back(new Position(pos_X,pos_Y));
	}
	return next_pos;
}

//
// Backtracking method employed to find the valid words formed from the board
// validating from the dictionary
//
void findWordsfromBoard(Pat_trie::Node* node, char** board, Position* curr_position, 
	bool** hasVisited, std::string word, int noOfCharMatch) 
{
	if(word.length() > N*N)
		return;

	// Form the word from the board position
	word =  word + board[curr_position->x][curr_position->y];
	// check if the formed word is contained in the dictionary
	StringMatchNode* strMatchNode = pat_trie->ifContains(node,word,noOfCharMatch);

	hasVisited[curr_position->x][curr_position->y] = true;

	// If the formed word is a valid word, then print it out
	if(strMatchNode->e_strMatch == WORD)
	{
		std::cout << word.c_str() << std::endl;
		return;
	}

	// If the word is a valid prefix then continue to 
	// add new characters to the present word
	if(strMatchNode->e_strMatch != NONE)
	{
		std::vector<Position*> next_pos = getAllvalidNextPosition(curr_position,hasVisited);
		for(int i=0; i< next_pos.size(); i++)
		{
			if(strMatchNode->e_strMatch == PREFIX)
				findWordsfromBoard(strMatchNode->node,board,next_pos[i], hasVisited, word,strMatchNode->noOfCharMatch);
			if(strMatchNode->e_strMatch == PREFIXWORD)
			{
				std::cout << word.c_str() << std::endl;
				findWordsfromBoard(strMatchNode->node,board, next_pos[i], hasVisited, word,strMatchNode->noOfCharMatch);
			}
		}
		
		for(int i=0; i<next_pos.size();i++)
			delete next_pos[i];
	}

	// if the word is not present in the dictionary backtrack
	if(strMatchNode->e_strMatch == NONE)
		hasVisited[curr_position->x][curr_position->y] = false;

	delete strMatchNode;
	
}

void initialize(char** input, bool** isVisited)
{
	for(int i=0; i< N; i++)
	{
		input[i] = new char[N];
		isVisited[i] = new bool[N];
	}

	for(int i=0; i< N ;i++)
	{
		for(int j=0; j<N; j++)
		{
			input[i][j] = '\0';
			isVisited[i][j] = false;
		}
	}

	std::cout << "Get all words into a 2D array of character" << std::endl;
	std::cout << "enter values one by one" << std::endl;
	for(int i=0; i< N; i++)
		for(int j=0; j<N;j++)
			std::cin>> input[i][j];
}

//
// Main function called with the dictionary file as input
//
int main(int argV, char* argC)
{
	Pat_trie* pTree = new Pat_trie();
	
	if(argV < 2)
	{
		std::cout << "not enough arguments " << std::endl;
		return -1;
	}

	std::string line="";
	std::ifstream file((const char*) argC[1], std::ios::in);
	int index = 0;
	if(file.is_open())
	{
		while(file.good())
		{
			std::getline(file,line);
			pTree->addWord(line);
		}
	}

	char** input = new (char*)();
	bool** isVisited = new (bool*)();
	std::cout << "Enter the dimension of the board" << std::endl;
	std::cin >> N;
	initialize(input,isVisited);

	for(int i=0; i< N; i++)
	{
		for(int j=0; j< N; j++)
		{
			Position* position = new Position(i,j);
			findWordsfromBoard(pTree->root,input, position, isVisited, "",0);
			delete position;
		}
	}

	delete pTree;
}
