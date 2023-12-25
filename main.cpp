#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <map>
#include <vector>
#include <pthread.h>
using namespace std;

// Huffman Tree node
struct Node {
  char character;
  int frequency;
  Node *left, *right;

  Node (char c, int f){
    character = c;
    frequency = f;
    left = right = nullptr;
  }
};

// Decompressed Node
struct Decompress {
  Node *top;
  string code;
  vector <int> positions;
  char *beginning;
  char symbol;

  Decompress (Node *Top, string Code, vector <int> Positions, char* beginning){
    top = Top;
    code = Code;
    positions = Positions;
    this->beginning = beginning;
  }
};


// HUFFMAN STUFF

// Used to compare nodes in the Huffman Tree
struct compare {
  bool operator()(Node* l, Node* r)
  {
    if (l->frequency == r->frequency){
      if(l->character == r->character)
        return l < r; // change this later for pg2
      return l->character > r->character;
    }
    return (l->frequency > r->frequency);
  }
};

//Used to print out the leafs from left to right and insert into symbolCodes map
void printCodes(struct Node* root, string str){
  if (!root)
    return;
  
  if (!root->left && !root->right){
    cout << "Symbol: " << root->character << ", Frequency: " << root->frequency << ", Code: " << str << endl;
  }
  
  printCodes(root->left, str + "0");
  printCodes(root->right, str + "1");
}

// Used to build the huffman tree
Node* buildHuffman(vector<char> c, vector<int> f, int size){
  
  //intialize our pointers in the tree
  Node *left, *right, *top;

  // Create a min heap pq & inserts all characters and their frequencies
  priority_queue<Node*, vector<Node*>, compare> pq;

  for (int i = 0; i < size; i++){
    pq.push(new Node(c[i], f[i]));
  }
  
  // Iterate while size of heap doesn't become 1
    while (pq.size() != 1) {
 
        // Extract the two minimum freq items from min heap
        left = pq.top();
        pq.pop();
 
        right = pq.top();
        pq.pop();

        top = new Node(' ', left->frequency + right->frequency);
 
        top->left = left;
        top->right = right;
 
        pq.push(top);
    }
 
    // Print Huffman codes using the Huffman tree built above
    printCodes(pq.top(), "");
    return pq.top();
}

// Decompressor Stuff
void *decompressor(void *arg){
  struct Decompress *arguments = (Decompress*) arg;
  Node *cur = arguments->top;
  for(int i = 0; i < arguments->code.size(); i++){
    if (arguments->code[i] == '0'){
      cur = cur->left;
    }
    else{
      cur = cur->right;
    }
  }

  arguments->symbol = cur->character;

  // Storing the symbols in the positions vector
  for (int i = 0; i < arguments->positions.size(); i++){
    int position = arguments->positions[i];
    *(arguments->beginning + position) = arguments->symbol;
  }

  
  return nullptr;
  
}

int main(){
  
  // Read the input file and input chars and freq into vectors
  ifstream input("input.txt");
  
  string line;
  vector <char> chars;
  vector <int> freqs;
  
  while (getline(input, line)){
    char chara;
    int freq;
    istringstream iss(line);
    if (line.at(0) == ' '){
      chars.push_back(' ');
      iss >> freq;
      freqs.push_back(freq);
    }
    else{
      iss >> chara >> freq;
      chars.push_back(chara);
      freqs.push_back(freq);
    }
  }
  int mes_size = 0;
  for (int i = 0; i < freqs.size(); i++){
    mes_size += freqs[i];
  }
  int size = chars.size();

  // Build Huffman Tree
  Node *root = buildHuffman(chars, freqs, size);

  
  // Read the compressed file and input positions and codes into a map
  ifstream command("compressed.txt");
  
  // thread variables
  static vector <Decompress*> t_args;
  vector <pthread_t> threads;

  //output
  vector <char> final_message(mes_size);
  
  vector<int> positions; 
  string cmdLine;
  while(getline(command, cmdLine)){
    string code;
    int pos;
    istringstream cmdIss(cmdLine);
    cmdIss >> code;
    
    while (cmdIss >> pos)
      positions.push_back(pos);
    
    // thread creation
    pthread_t thread;
    Decompress* t_arg = new Decompress(root, code, positions, final_message.data());

    if (pthread_create(&thread, nullptr, decompressor, t_arg)){
    cout << "Cannot create thread" << endl;
      return 1;
    }

    threads.push_back(thread); // thread vector
    t_args.push_back(t_arg); // vector of Decompress* structs (pushing back each pointer)
    positions.clear();
  }

  for (int i = 0; i < threads.size(); i++){
    pthread_join(threads[i], nullptr);
  }

  cout << "Original message: ";
  for (int i = 0; i < final_message.size(); i++)
    cout << final_message[i];
  
  return 0;
}
