#include <iostream>
#include <Windows.h>
#include <fstream>
using namespace std;

struct Point {
	int x, y;
};
int myStrcmp(const char* str1, const char* str2) {
	while (*str1 && (*str1 == *str2)) {
		++str1;
		++str2;
	}
	return *(unsigned char*)str1 - *(unsigned char*)str2;
}
void gotoxy(int x, int y);
void convertToUpper(char* c);
void printFormat();
int x = 1, y = 4;
class Node {
public:
	char data;
	Node* up;
	Node* down;
	Node* left;
	Node* right;
	int line;
	bool highlighted;
	Node(char data = '\0') {
		this->data = data;
		up = down = left = right = NULL;
		line = 0;
		highlighted = false;
	}
};
void freeMemory(Node*& head);
Node* cloneList(Node* head) {
	if (head == nullptr) {
		return nullptr;
	}
	Node* newHead = new Node(head->data);
	Node* temp = head->right;
	Node* newTemp = newHead;

	while (temp) {
		newTemp->right = new Node(temp->data);
		newTemp->right->left = newTemp;
		newTemp = newTemp->right;
		temp = temp->right;
	}

	Node* rowStart = head;
	Node* newRowStart = newHead;

	while (rowStart->down) {
		rowStart = rowStart->down;

		newRowStart->down = new Node(rowStart->data);
		newRowStart->down->up = newRowStart;
		newRowStart = newRowStart->down;

		temp = rowStart->right;
		newTemp = newRowStart;

		while (temp) {
			newTemp->right = new Node(temp->data);
			newTemp->right->left = newTemp;
			newTemp = newTemp->right;
			temp = temp->right;
		}
	}
	return newHead;

}
struct State {
	Node* headState;  // Snapshot of the head of the linked list
	int cursorX, cursorY;  // Cursor position
	State* next;  // Pointer to the next state

	State(Node* head, int x, int y) {
		headState = cloneList(head);  // Clone the current list state
		cursorX = x;
		cursorY = y;
		next = nullptr;
	}

	~State() {
		freeMemory(headState);  // Clean up the cloned list
	}
};
class Stack {
private:
	State* top;  // Pointer to the top of the stack
	int size;    // Current size of the stack

public:
	Stack() : top(nullptr), size(0) {}

	bool isEmpty() {
		return top == nullptr;
	}

	void push(Node* head, int x, int y) {
		if (size > 5) {  // Limit to 5 states
			State* temp = top;
			//top = top->next;  // Remove the oldest state
			while (temp->next->next != nullptr) {
				temp = temp->next;
			}
			delete temp->next;  // Free the memory of the removed state
			temp->next = nullptr;
			size--;
		}
		State* newState = new State(head, x, y);
		newState->next = top;  // Push new state onto the stack
		top = newState;  // Update top pointer
		size++;
	}

	State* pop() {
		if (isEmpty()) {
			return nullptr;
		}
		State* temp = top;  // Save current top
		top = top->next;  // Move top pointer to the next state
		size--;
		return temp;  // Return the popped state
	}

	~Stack() {
		while (top != nullptr) {
			State* temp = top;
			top = top->next;  // Move to the next state
			delete temp;  // Free memory
		}
	}
};
Stack undoStack;
Stack redoStack;
void undo(Node*& head, Node*& cursor) {
	if (undoStack.isEmpty()) {
		return;
	}
	State* prevState = undoStack.pop();  // Pop the previous state
	redoStack.push(head, x,y);
	freeMemory(head);  // Free the current memory
	head = prevState->headState;  // Restore the previous state
	cursor = head;
	for (int i = 0; i < prevState->cursorY - 4; i++) {
		cursor = cursor->down;
	}
	while (cursor && cursor->right && cursor->right->data && cursor->right->data != '\n') {
		cursor = cursor->right;
	}
	x = prevState->cursorX;
	y = prevState->cursorY;
	//delete prevState;  // Free the memory of the popped state
}
void redo(Node*& head, Node*& cursor) {
	if (redoStack.isEmpty()) {
		return;
	}
	State* nextState = redoStack.pop();  // Pop the next state
	undoStack.push(head, x, y);
	freeMemory(head);  // Free the current memory
	head = nextState->headState;  // Restore the next state
	cursor = head;
	for (int i = 0; i < nextState->cursorY - 4; i++) {
		cursor = cursor->down;
	}
	while (cursor && cursor->right && cursor->right->data && cursor->right->data != '\n') {
		cursor = cursor->right;
	}
	x = nextState->cursorX;
	y = nextState->cursorY;
	//delete nextState;  // Free the memory of the popped state
}
Node* Lastcursor = nullptr;
void insertion(Node*& cursor, char ch, Node*& head) {

	if (cursor && cursor->data == '|') {
		cursor->data = ch;
		return;
	}
	
	Node* newNode = new Node(ch);


	if (!cursor) {  // If cursor is null (start)
		cursor = newNode;

		if (!head) {  // Case when head is null (empty list)
			head = cursor;
		}
		else {
			// Find the start of the current line through head and x, y coordinates
			Node* temp = head;
			for (int i = 0; i < y - 4 && temp->down; i++) {  // Traverse downwards
				temp = temp->down;
			}

			Node* shifter = temp;  // Start of the current row
			while (shifter->right) {  // Traverse to the end of the current row
				shifter = shifter->right;
			}
			shifter->right = newNode;  // Insert the new node at the end of the row
			newNode->left = shifter;  // Update the left link of the new node
			if (shifter->up && shifter->up->right) {  // If there is a node above the current node
				newNode->up = shifter->up->right;  // Update the up link of the new node
				shifter->up->right->down = newNode;  // Update the down link of the node above
			}
			if (shifter->down && shifter->down->right) {  // If there is a node below the current node
				newNode->down = shifter->down->right;  // Update the down link of the new node
				shifter->down->right->up = newNode;  // Update the up link of the node below
			}
			shifter = shifter->right;  // Move to the new node
			while (shifter != temp) {
				shifter->data = shifter->left->data;  // Shift the data to the right
				shifter = shifter->left;  // Move to the left
			}
			shifter->data = ch;  // Insert the new character at the cursor position
			cursor = shifter;
			// Special case for the first line
			if (y == 4) {
				head = cursor;
			}
		}
	}

	else if (!cursor->right) { //if end
		cursor->right = newNode;
		newNode->left = cursor;
		if (cursor->up && cursor->up->right) {
			newNode->up = cursor->up->right;
			cursor->up->right->down = newNode;
		}
		if (cursor->down && cursor->down->right) {
			newNode->down = cursor->down->right;
			cursor->down->right->up = newNode;
		}
		cursor = newNode;
	}
	else if (cursor->right) {  // if mid
		if (ch != '\n') {
			Node* temp = cursor->right;
			while (temp->right) {
				temp = temp->right;
			}
			temp->right = newNode;
			newNode->left = temp;
			if (temp->up && temp->up->right) {
				newNode->up = temp->up->right;
				temp->up->right->down = newNode;
			}
			if (temp->down && temp->down->right) {
				newNode->down = temp->down->right;
				temp->down->right->up = newNode;
			}
			temp = temp->right;
			while (temp != cursor->right) {
				temp->data = temp->left->data;
				temp = temp->left;
			}
			temp->data = ch;
			cursor = temp;
		}
		else {
			Node* toNewLine = cursor->right;
			toNewLine->left = NULL;
			cursor->right = NULL;
			Node* start = cursor;
			while (start->left) {
				start = start->left;
			}
			while (start && toNewLine) {
				Node* temp = start->down;
				start->down = toNewLine;
				if (toNewLine->up) {
					toNewLine->up->down = nullptr;
				}
				if (toNewLine->down) {
					toNewLine->down->up = nullptr;
				}
				toNewLine->up = start;
				toNewLine->down = temp;
				if (temp) {
					temp->up = toNewLine;
				}
				start = start->right;
				toNewLine = toNewLine->right;
			}
			while (start) {
				if (start->down) {
					start->down->up = nullptr;
					start->down = nullptr;
				}
				start = start->right;
			}
			cursor->right = newNode;
			newNode->left = cursor;
			while (toNewLine) {
				if (toNewLine->up) {
					toNewLine->up->down = nullptr;
				}
				if (toNewLine->down) {
					toNewLine->down->up = nullptr;
				}
				if (toNewLine->left && toNewLine->left->down && toNewLine->left->down->right) {
					toNewLine->down = toNewLine->left->down->right;
					toNewLine->left->down->right->up = toNewLine;
				}
				toNewLine = toNewLine->right;
			}
		}

	}
	if (ch == '\n') {
		while (cursor->left) {
			cursor = cursor->left;
		}
		y++;
		x = 1;
	}
}
void deletion(Node*& cursor, Node*& head) {

	// Case 1: If cursor is null, nothing to delete
	if (!head || cursor == nullptr) {
		return;
	}

	Node* temp = cursor;
	if (cursor) {
		if (cursor->left) {
			cursor = cursor->left;
		}
		else if (y>4){
			y--;
			Node* temp = head;
			for (int i = 0; i < y - 4; i++) {
				temp = temp->down;
			}
			while (temp->right) {
				x++;
				temp = temp->right;
			}
			temp = temp->left;
			if (temp && temp->right) {
				delete temp->right;
				temp->right = nullptr;
			}
			cursor = temp;
		}
		while (temp->right) {
			temp->data = temp->right->data;
			temp = temp->right;
		}
		if (temp->up) {
			temp->up->down = nullptr;
		}
		if (temp->down) {
			temp->down->up = nullptr;
		}
		if (temp->left) {
			temp->left->right = nullptr;
		}
		if (temp == head) {
			if (head->right) {
				head = head->right;
			}
			else if (head->up) {
				head = head->up;
			}
			else if (head->down && head->down->right) {
				head = head->down;
				cursor = head;
			}
			else if (head->left) {
				head = head->left;
			}
			else {
				head = nullptr;
				cursor = nullptr;
			}
		}
		delete temp;
		temp = nullptr;
		return;
	}

}
void print(Node* head) {
	if (head == nullptr) {
		return;
	}

	Node* rowStart = head;  // Start of the current row
	Node* current;
	int x = 1, y = 4;
	// Traverse row by row
	while (rowStart != nullptr) {
		current = rowStart;

		// Traverse each node in the row (from left to right)
		while (current != nullptr) {
			if (current->data && current->data != '|')
				cout << current->data;
			if (current->data == '\n') {
				gotoxy(x, ++y);
			}
			current = current->right;
		}
		rowStart = rowStart->down;  // Move to the next row
	}
}
void freeMemory(Node*& head) {
	while (head) {
		Node* rowStart = head;
		head = head->down;

		while (rowStart) {
			Node* temp = rowStart;
			rowStart = rowStart->right;
			delete temp;
			temp = nullptr;
		}
	}
}
void storeinFile(Node* head) {
	ofstream file;
	file.open("file.txt", ios::out);

	if (!file.is_open()) {  // Check if file was successfully opened
		cout << "Unable to open or create file." << endl;
		return;
	}

	Node* rowStart = head;
	Node* current;
	while (rowStart != nullptr) {
		current = rowStart;
		while (current != nullptr) {
			if (current->data != '|')
				file << current->data;
			current = current->right;
		}
		rowStart = rowStart->down;
	}
	file.close();
}
void loadFromFile(Node*& head, Node*& cursor, int& numOfLines) {
	ifstream file;
	file.open("file.txt", ios::in);
	if (!file.is_open()) {
		cout << "Unable to open file." << endl;
		return;
	}
	char ch;
	while (file.get(ch)) {
		if (ch == '\n') {
			insertion(cursor, ch, head);
			numOfLines++;
			if (cursor->down) {
				cursor = cursor->down;
			}
			else {
				cursor->down = new Node('|');
				cursor->down->up = cursor;
				cursor = cursor->down;
			}
			cursor = cursor->left;
		}
		else {
			insertion(cursor, ch, head);
			x++;
		}
	}
	file.close();
}
void createNewFile(Node*& head, Node*& cursor, int& numOfLines) {
	freeMemory(head);
	head = nullptr;
	cursor = nullptr;
	numOfLines = 1;
	x = 1;
	y = 4;
}
bool isAlphabet(char ch) {
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}
struct dataset {
	char* word;
	Node** wordNodes;
};
class nAryNode {
public:
	char c;
	nAryNode* children[26];
	bool isEndOfWord;
	int frq;
	Node* charNode[100];

	nAryNode(char c = '\0') : c(c){
		isEndOfWord = false;
		for (int i = 0; i < 26; i++) {
			children[i] = nullptr;
		}
		frq = 0;
		for (int i = 0; i < 100; i++) {
			charNode[i] = nullptr;
		}
	}
	void pushNode(Node* node) {
		node->line = y - 3;
		charNode[frq] = node;
		frq++;
	}
	void popNode(Node* node) {
		for (int i = 0; i < frq; i++) {
			if (charNode[i] == node) {
				for (int j = i; j < frq - 1; j++) {
					charNode[j] = charNode[j + 1];
				}
				frq--;
				return;
			}
		}
	}
	//~nAryNode() {
	//	for (int i = 0; i < 26; i++) {
	//		if (children[i]) {
	//			delete children[i];
	//		}
	//	}
	//}
};
void highlightSearch(Node* head) {
	if (head == nullptr) {
		return;
	}

	Node* rowStart = head;  // Start of the current row
	Node* current;
	int x = 1, y = 4;
	// Traverse row by row
	while (rowStart != nullptr) {
		current = rowStart;

		// Traverse each node in the row (from left to right)
		while (current != nullptr) {
			if (current->data && current->data != '|') {
				if (current->highlighted) {
					cout << "\033[1;35;47m" << current->data << "\033[0m";
					current->highlighted = false;
				}
				else
					cout << current->data;
			}
			if (current->data == '\n') {
				gotoxy(x, ++y);
			}
			current = current->right;
		}
		rowStart = rowStart->down;  // Move to the next row
	}
}
class nAryTree {
public:
	nAryNode* root;


	nAryTree() : root(new nAryNode()) {}

	~nAryTree() {
		delete root;
	}

	void insert(const dataset* word) {
		if (!word) return;
		nAryNode* currentNode = root;
		for (int i = 0; word->word[i] != '\0'; i++) {
			int index = word->word[i] - 'A';
			if (index < 0 || index > 25) {
				//std::cerr << "Invalid character in word: " << word[i] << std::endl;
				return;
			}
			if (!currentNode->children[index]) {
				currentNode->children[index] = new nAryNode(word->word[i]);
			}
			currentNode->children[index]->pushNode(word->wordNodes[i]);
			currentNode = currentNode->children[index];
		}
		currentNode->isEndOfWord = true;
	}

	void markEndOfWord(const char* word) {
		nAryNode* currentNode = root;
		for (int i = 0; word[i] != '\0'; i++) {
			int index = word[i] - 'A';
			currentNode = currentNode->children[index];
			currentNode->frq++;
		}
		currentNode->isEndOfWord = true;
	}
	void search(const char* word, Node* head) {
		int length = 0;
		for (int i = 0; word[i] != '\0'; i++) {
			length++;
		}
		char toCheck[100] = { '\0' };
		int x = 0;
		for (int i = length - 1; i >= 0 && word[i] != ' '; i--) {
			toCheck[x++] = word[i];
		}
		toCheck[x] = '\0';
		for (int i = 0; i < x / 2; i++) {
			char temp = toCheck[i];
			toCheck[i] = toCheck[x - i - 1];
			toCheck[x - i - 1] = temp;
		}
		int linesFound[100] = { 0 };
		nAryNode* currentNode = root;
		for (int i = 0; toCheck[i] != '\0'; i++) {
			int index = toCheck[i] - 'A';
			if (index < 0 || index > 25 || !currentNode->children[index]) {
				std::cout << "Word not found.\n";
				return;
			}
			currentNode = currentNode->children[index];
			//currentNode->charNode[0]->highlighted = true; //temporary. we need to find a way to highlight only the words (stack maybe)
		}
		int linearrIndex = 0;
		for (int i = 0; i < currentNode->frq; i++) {
			bool found = true;
			Node* temp = currentNode->charNode[i];
			int j;
			for (j = length - 1; j >= 0 && temp; j--) {
				char ch = temp->data;
				convertToUpper(&ch);
				if (word[j] != ch) {
					found = false;
					break;
				}
				temp = temp->left;
			}
			if (!temp && j > 0) {
				found = false;
			}

			if (found) {
				temp = currentNode->charNode[i];
				if (temp)
					linesFound[linearrIndex++] = temp->line;
				for (int j = 0; j < length && temp; j++) {
					temp->highlighted = true;
					temp = temp->left;
				}
			}
		}
		gotoxy(x, y);
		printFormat();
		gotoxy(1, 4);
		highlightSearch(head);
		gotoxy(x, y);
		int sy = 3;
		gotoxy(101, sy++);
		if (linesFound[0] == 0) {
			cout << "Word not found.\n";
			return;
		}
		std::cout << "Word found. Lines:";
		gotoxy(101, sy++);
		for (int i = 0; i < 100; i++) {
			if (linesFound[i]) {
				std::cout << "Line " << linesFound[i];
				gotoxy(101, sy++);
			}
		}
	}


	void deletion(const dataset* word) {
		if (!word) return;
		nAryNode* currentNode = root;
		for (int i = 0; word->word[i] != '\0'; i++) {
			int index = word->word[i] - 'A';
			currentNode->children[index]->popNode(word->wordNodes[i]);
			if (currentNode->children[index]->frq <= 0) {
				delete currentNode->children[index];
				currentNode->children[index] = nullptr;
				return;
			}
			currentNode = currentNode->children[index];
		}
		currentNode->isEndOfWord = false;
	}

	void printWords(nAryNode* node, char* word, int depth) {
		if (!node) return;
		if (node->isEndOfWord) {
			word[depth] = '\0';
			std::cout << word << std::endl;
		}
		for (int i = 0; i < 26; i++) {
			if (node->children[i]) {
				word[depth] = 'A' + i;
				printWords(node->children[i], word, depth + 1);
			}
		}
	}

	void printAllWords() {
		char words[100];
		printWords(root, words, 0);
	}


};
dataset* getWord(Node* endNode) {
	if (!endNode) return nullptr;

	Node* startNode = endNode;
	while (startNode->left && startNode->left->data != ' ') {
		startNode = startNode->left;
	}
	int length = 0;
	Node* temp = startNode;
	while (temp && temp->data != ' ' && temp->data != '\n') {
		length++;
		temp = temp->right;
	}
	char* word = new char[length + 1];
	Node** wordNodes = new Node*[length];

	Node* currentNode = startNode;
	int i = 0;
	while (currentNode && currentNode->data != ' ' && currentNode->data != '\n' && currentNode->data!='|') {
		wordNodes[i] = currentNode;
		word[i++] = currentNode->data & ~32; //converting to capital letter :) W logic
		currentNode = currentNode->right;
	}
	word[i] = '\0';
	dataset *data = new dataset();
	data->word = word;
	data->wordNodes = wordNodes;

	return data;
}
void gotoxy(int x, int y)
{
	COORD c = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
void printFormat() {
	system("cls");
	gotoxy(2, 0);
	cout<<"MENU  =>  Exit: esc   Save and Quit: 0   Save: 1   Load: 2   New File: 3   Search: 4";
	gotoxy(2, 1);
	cout << "Welcome to the Notepad.";
	gotoxy(2, 2);
	cout << "This is the area where you are supposed to write the content.";
	for (int i = 0; i < 25; i++) {
		gotoxy(0, i);
		cout << "|";
		gotoxy(100, i);
		cout << "|";
	}
	gotoxy(1, 24);
	for (int i = 1; i < 100; i++) {
		cout << "_";
	}
	gotoxy(1, 25);
	cout << "Word Suggestions: ";
	gotoxy(101, 0);
	cout << "Search";
}
void convertToUpper(char* str) {
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] >= 'a' && str[i] <= 'z') {
			str[i] = str[i] & ~32;
		}
	}
}
void removeDuplicates(char* words[], int& count) {
	int newCount = 0; // Counter for unique words

	for (int i = 0; i < count; ++i) {
		bool isDuplicate = false;

		// Check if the current word is a duplicate
		for (int j = 0; j < newCount; ++j) {
			if (myStrcmp(words[i], words[j]) == 0) {
				isDuplicate = true;
				break;
			}
		}

		// If not a duplicate, add it to the unique list
		if (!isDuplicate) {
			words[newCount++] = words[i];
		}
	}

	// Update count to the number of unique words
	count = newCount;
}
bool isDuplicate(char* words[], int uniqueCount, const char* word) {
	for (int i = 0; i < uniqueCount; ++i) {
		if (words[i]) {
			if (strcmp(words[i], word) == 0) {
				return true; // Duplicate found
			}
		}
	}
	return false; // No duplicate found
}
void printWordSuggestions(nAryTree*& nAry, Node*& cursor, Node*& head) {
	gotoxy(1, 26);
	dataset* word = getWord(cursor);
	if (!word) {
		cout<<"No suggestions available.";
		return;
	}
	char* wordStr = word->word;
	nAryNode* currentNode = nAry->root;
	int length = 0;
	for (int i = 0; wordStr[i] != '\0'; i++) {
		length++;
	}
	for (int i = 0; wordStr[i] != '\0'; i++) {
		int index = wordStr[i] - 'A';
		if (index < 0 || index > 25 || !currentNode->children[index]) {
			cout<<"No suggestions available.";
			return;
		}
		currentNode = currentNode->children[index];
	}
	
	char** suggestions = new char*[currentNode->frq];
	int sugindex = 0;
	int j = 0;
	for (int i = 0; i < currentNode->frq; i++) {
		Node* temp = currentNode->charNode[i];
		if (!temp->right || !isAlphabet(temp->right->data)) {
			continue;
		}
		char* sug = getWord(temp)->word;

		suggestions[sugindex] = sug;
		cout << ++sugindex << ". " << sug << "  ";
	}
	if (sugindex == 0) {
		cout<<"No suggestions available.";
		return;
	}
	cout<<"Enter number to insert suggestion: ";
	int num;
	cin >> num;
	if (num > 0 && num < currentNode->frq) {
		gotoxy(x, y);
		char* suggestion = suggestions[num - 1];
		for (int i = length; suggestion[i] != '\0'; i++) {
			if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
				nAry->deletion(getWord(cursor->right));
			}
			else if (cursor && cursor->data != ' ') {
				nAry->deletion(getWord(cursor));
			}
			insertion(cursor, suggestion[i] | 0x20, head);  // Call the insertion function
			nAry->insert(getWord(cursor));
			//cout << keyPressed;
			x++;  // Move cursor right after insertion
			/*gotoxy(x, y);
			printFormat();
			gotoxy(1, 4);
			print(head);
			gotoxy(x, y);*/
			char ch = suggestion[i] | 0x20;
			cout << ch;
		}
	}
	else {
		cout<<"Invalid number.";
	}
	
}
class VisitedStack {
private:
	bool* stack[1000]; // Stack array to hold pointers to visited flags
	int top;

public:
	VisitedStack() : top(-1) {
		for (int i = 0; i < 1000; i++) {
			stack[i] = nullptr;
		}
	}

	// Push a visited flag pointer onto the stack
	void push(bool* visitedFlag) {
		if (top < 999) {
			stack[++top] = visitedFlag;
		}
		else {
			//std::cout << "Stack overflow!" << std::endl;
			return;
		}
	}

	// Pop all visited flags and reset them to false
	void resetVisited() {
		while (top >= 0) {
			*stack[top--] = false;
		}
	}

	// Check if the stack is empty
	bool isEmpty() const {
		return top == -1;
	}
};
VisitedStack visitedStack;
struct ChilliMilli;
void traverse(ChilliMilli*& node, ChilliMilli*& prev, char* target);
struct ChilliMilli {
	char* word;
	ChilliMilli* next[100];
	int nextNum;
	bool visited;

	ChilliMilli(char* word = nullptr) : word(word), nextNum(0) {
		for (int i = 0; i < 100; i++) {
			next[i] = nullptr;
		}
		visited = false;
	}
	~ChilliMilli() {
		delete[] word;
		for (int i = 0; i < 100; i++) {
			if (next[i]) {
				delete next[i];
			}
		}
	}

	void insert(char* word, ChilliMilli*& t) {
		if (!word) return;
		ChilliMilli* temp = nullptr;
		traverse(t, temp, word);
		visitedStack.resetVisited();
		if (!temp) {
			next[nextNum++] = new ChilliMilli(word);	//hello->my->name
			return;
		}
		for (int i = 0; i < 100; i++) {
			if (next[i] == temp) {
				return;		//already a link exists		//hello->my
			}
		}
		next[nextNum++] = temp;		//hello->my->hello
	}
};

class ChilliMilliTree {
	public:
	ChilliMilli* root;
	ChilliMilli* LastInserted;

	ChilliMilliTree() : root(NULL), LastInserted(NULL) {}

	~ChilliMilliTree() {
		delete root;
	}

	void insert(char* word, Node* cursor) {
		if (!word) return;
		if (!root) {
			root = new ChilliMilli(word);
			return;
		}
		// get previous word
		Node* temp = cursor;
		while (temp->left && temp->left->data != ' ') {
			temp = temp->left;
		}
		temp = temp->left;
		if (!temp) {
			
			/*traverse(root, LastInserted, word);
			visitedStack.resetVisited();
			LastInserted->insert(word, root);*/

			return;
		}
		while (temp->left && temp->left->data != ' ') {
			temp = temp->left;
		}
		char* prevWord = getWord(temp)->word;
		traverse(root, LastInserted, prevWord);
		visitedStack.resetVisited();
		LastInserted->insert(word, root);

	}
	void sentenceComp(Node*& cursor, Node*& head, nAryTree*& nAry) {
		char* target = getWord(cursor)->word;
		ChilliMilli* temp = nullptr;
		traverse(root, temp, target);
		visitedStack.resetVisited();
		gotoxy(1, 26);
		if (!temp) {
			cout << "No suggestions available.";
			return;
		}
		char** suggestions = new char* [temp->nextNum];
		int sugindex = 0;
		for (int i = 0; i < temp->nextNum; i++) {
			if (temp->next[i]) {
				cout << ++sugindex << ". " << temp->next[i]->word << "  ";
				suggestions[sugindex - 1] = temp->next[i]->word;
			}
		}
		if (sugindex == 0) {
			cout << "No suggestions available.";
			return;
		}
		cout << "Enter number to insert suggestion: ";
		int num;
		cin >> num;
		if (num > 0 && num <= temp->nextNum) {
			gotoxy(x, y);
			char* suggestion = suggestions[num - 1];
			for (int i = 0; suggestion[i] != '\0'; i++) {
				if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
					nAry->deletion(getWord(cursor->right));
				}
				else if (cursor && cursor->data != ' ') {
					nAry->deletion(getWord(cursor));
				}
				insertion(cursor, suggestion[i] | 0x20, head);  // Call the insertion function
				nAry->insert(getWord(cursor));
				//cout << keyPressed;
				x++;  // Move cursor right after insertion
				/*gotoxy(x, y);
				printFormat();
				gotoxy(1, 4);
				print(head);
				gotoxy(x, y);*/
				char ch = suggestion[i] | 0x20;
				cout << ch;
			}
		}
		else {
			cout << "Invalid number.";
		}
	}
	void deletion(char* word) {
		if (!word) return;
		ChilliMilli* temp = nullptr;
		traverse(root, temp, word);
		visitedStack.resetVisited();
		if (!temp) {
			cout << "Word not found.";
			return;
		}
		for (int i = 0; i < 100; i++) {
			if (temp->next[i] && myStrcmp(temp->next[i]->word, word) == 0) {
				temp->next[i] = nullptr;
				return;
			}
		}
		cout << "Word not found.";
	}
	void printTree(ChilliMilli* node) {
		if (!node) return;
		if (node->visited) return;
		node->visited = true;
		cout << node->word << endl;
		for (int i = 0; i < node->nextNum; i++) {
			printTree(node->next[i]);
		}
	}

};
int limit = 0;
void traverse(ChilliMilli*& node, ChilliMilli*& prev, char* target) {
	if (!node) return;
	if (node->visited) return;
	node->visited = true; //uncomment this line to make everything else but chillimilli work
	if (limit++ < 990)
		visitedStack.push(&node->visited);
	if (myStrcmp(node->word, target) == 0) {
		node->visited = true;
		prev = node;
		visitedStack.resetVisited();
		limit = 0;
		return;
	}
	for (int i = 0; i < node->nextNum; i++) {
		traverse(node->next[i], prev, target);
	}
}

int main() {

	printFormat();

	HANDLE  rhnd = GetStdHandle(STD_INPUT_HANDLE);  // handle to read console

	DWORD Events = 0;     // Event count
	DWORD EventsRead = 0; // Events read from console

	bool Running = true;
	
	gotoxy(x, y);


	Node* head = NULL;
	Node* cursor = NULL;
	nAryTree* nAry = new nAryTree();
	ChilliMilliTree* chilliMilli = new ChilliMilliTree();
	int numOfLines = 1;
	int mode = 0;  // 0: Insertion mode, 1: Search mode
	//programs main loop
	while (Running) {
		// gets the systems current "event" count
		GetNumberOfConsoleInputEvents(rhnd, &Events);

		if (Events != 0) { // if something happened we will handle the events we want

			// create event buffer the size of how many Events
			INPUT_RECORD eventBuffer[200];

			// fills the event buffer with the events and saves count in EventsRead
			ReadConsoleInput(rhnd, eventBuffer, Events, &EventsRead);
			
			// loop through the event buffer using the saved count
			for (DWORD i = 0; i < EventsRead; ++i) {	

				// check if event[i] is a key event && if so is a press not a release
				if (eventBuffer[i].EventType == KEY_EVENT && eventBuffer[i].Event.KeyEvent.bKeyDown) {
					
					Node* newLine = nullptr;
					bool spaceInsertedInBW = false;
					char toSearch[100];
					// check if the key press was an arrow key
					switch (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode) {
					case VK_ESCAPE: // Escape key
						Running = false;
						//nAry->printAllWords();
						chilliMilli->printTree(chilliMilli->root);
						break;
					case '0': // Save and quit
						storeinFile(head);
						Running = false;
						break;
					case '1': // Save to file
						storeinFile(head);
						break;
					case '2': // Load from file
						if ((eventBuffer[i].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0) {
							// '@' detected
							printWordSuggestions(nAry, cursor, head);
							gotoxy(x, y);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);
							break;
						}
						freeMemory(head);
						head = nullptr;
						cursor = nullptr;
						numOfLines = 1;
						loadFromFile(head, cursor, numOfLines);
						printFormat();
						gotoxy(1, 4);
						print(head);
						gotoxy(x, y);
						break;
					case '8':
						if ((eventBuffer[i].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0) {
							// '*' detected
							chilliMilli->sentenceComp(cursor, head, nAry);
							gotoxy(x, y);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);
							break;
						}
						break;
					case '3': // Create new file
						createNewFile(head, cursor, numOfLines);
						printFormat();
						gotoxy(x, y);
						break;
					case '4': // Search mode
						mode = 1;
						printFormat();
						gotoxy(1, 4);
						print(head);
						gotoxy(101, 2);
						std::cin.getline(toSearch, sizeof(toSearch));
						convertToUpper(toSearch);
						gotoxy(101, 3);
						nAry->search(toSearch, head); // need to convert highlight back to normal
						gotoxy(x, y);
						break;
					case 'Z': // ASCII code for 'Z'
						if (eventBuffer[i].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
							// Ctrl + Z detected, perform undo operation
							//undo
							undo(head, cursor);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);
						}
						else {
							// Normal 'Z' press, insert character 'z'
							if (isAlphabet(eventBuffer[i].Event.KeyEvent.uChar.AsciiChar) && y < 24) {
								char keyPressed = eventBuffer[i].Event.KeyEvent.uChar.AsciiChar;
								if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
									nAry->deletion(getWord(cursor->right));
								}
								else if (cursor && cursor->data != ' ') {
									nAry->deletion(getWord(cursor));
								}
								insertion(cursor, keyPressed, head);  // Call the insertion function
								nAry->insert(getWord(cursor));
								//cout << keyPressed;
								x++;  // Move cursor right after insertion
								gotoxy(x, y);
								printFormat();
								gotoxy(1, 4);
								print(head);
								gotoxy(x, y);
								//cout<< keyPressed;
							}
							if (x >= 100) {

								insertion(cursor, '\n', head);  // Create a new line node

								undoStack.push(head, x, y);
								while (!redoStack.isEmpty()) redoStack.pop();
								numOfLines++;  // Increment the number of lines

								if (cursor->down) {
									cursor = cursor->down;
								}
								else {
									cursor->down = new Node('|');
									cursor->down->up = cursor;
									cursor = cursor->down;
								}
								cursor = cursor->left;
								gotoxy(x, y);
								printFormat();
								gotoxy(1, 4);
								print(head);
								gotoxy(x, y);

							}

							break;
						}
						break;

						// Handle Ctrl+Y (Redo)
					case 'Y': // ASCII code for 'Y'
						if (eventBuffer[i].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
							// Ctrl + Y detected, perform redo operation
							//redo
							redo(head, cursor);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);
						}
						else {
							// Normal 'Y' press, insert character 'y'
							if (isAlphabet(eventBuffer[i].Event.KeyEvent.uChar.AsciiChar) && y < 24) {
								char keyPressed = eventBuffer[i].Event.KeyEvent.uChar.AsciiChar;
								if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
									nAry->deletion(getWord(cursor->right));
								}
								else if (cursor && cursor->data != ' ') {
									nAry->deletion(getWord(cursor));
								}
								insertion(cursor, keyPressed, head);  // Call the insertion function
								nAry->insert(getWord(cursor));
								//cout << keyPressed;
								x++;  // Move cursor right after insertion
								gotoxy(x, y);
								printFormat();
								gotoxy(1, 4);
								print(head);
								gotoxy(x, y);
								//cout << keyPressed;
							}
							if (x >= 100) {

								insertion(cursor, '\n', head);  // Create a new line node

								undoStack.push(head, x, y);
								while (!redoStack.isEmpty()) redoStack.pop();
								numOfLines++;  // Increment the number of lines

								if (cursor->down) {
									cursor = cursor->down;
								}
								else {
									cursor->down = new Node('|');
									cursor->down->up = cursor;
									cursor = cursor->down;
								}
								cursor = cursor->left;
								gotoxy(x, y);
								printFormat();
								gotoxy(1, 4);
								print(head);
								gotoxy(x, y);

							}

							break;
						}
						break;
					case VK_UP: //up
						
						if (cursor && cursor->up && cursor->up->data != '\n') {
							cursor = cursor->up;
							y--;
							gotoxy(x, y);
						}
						else if (head && !cursor) {
							Node* temp = head;
							for (int i = 0; i < y - 4; i++) {  // Traverse downwards
								temp = temp->down;
							}
							//cursor = temp->up;
							if (y > 4) {
								y--;
								gotoxy(x, y);
							}
						}
						break;
					case VK_DOWN: //down
						
						if (cursor && cursor->down && cursor->down->data != '\n') {
							cursor = cursor->down;
							y++;
							gotoxy(x, y);
						}
						else if (head && !cursor) {
							Node* temp = head;
							for (int i = 0; i < y - 4; i++) {  // Traverse downwards
								temp = temp->down;
							}
							//cursor = temp;
							if (y < numOfLines + 3) {
								y++;
								gotoxy(x, y);
							}
						}
						break;
					case VK_RIGHT: //right
						
						if (cursor && cursor->right && cursor->right->data!='\n') {
							cursor = cursor->right;
							x++;
							gotoxy(x, y);
						}
						else if (head && !cursor) {
							Node* temp = head;
							for (int i = 0; i < y - 4; i++) {  // Traverse downwards
								temp = temp->down;
							}
							cursor = temp;
							x++;
							gotoxy(x, y);
						}
						break;
					case VK_LEFT: //left
						
						if (cursor /*&& cursor->left*/) {
							Lastcursor = cursor;
							cursor = cursor->left;
							x--;
							gotoxy(x, y);

						}
						break;
					case VK_BACK: // backspace
						if (cursor && cursor->data != ' ' && cursor->data != '|') {
							nAry->deletion(getWord(cursor));
						}
						else if (cursor && cursor->left && cursor->left->data != ' ') {
							nAry->deletion(getWord(cursor->left));
						}
						deletion(cursor, head);  // Call the deletion 
						if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
							nAry->insert(getWord(cursor->right));
						}
						else {
							nAry->insert(getWord(cursor));
						}
						x--;
						if (x < 1) {
							x = 1;
							if (y > 4) {
								
								y--;
								cursor = head;
								for (int i = 0; i < y - 4; i++) {
									cursor = cursor->down;
								}
								Node* temp = cursor->down;
								while (cursor && cursor->right && cursor->right->data!='\n' && cursor->data!='\n') {
									cursor = cursor->right;
									x++;
								}
								if (cursor) {
									cursor->right = temp;
								}
								if (temp) {
									temp->left = cursor;
								}

								x++;
								numOfLines--;
								gotoxy(x, y);
							}
						}
						//cout<<"\b \b";  // Move cursor back and print a space to 'erase' the character
						printFormat();
						gotoxy(1, 4);
						print(head);
						gotoxy(x, y);
						break;
					case VK_RETURN: // Enter key
						// Create a new line
						insertion(cursor, '\n', head);  // Create a new line node

						undoStack.push(head, x, y);
						while (!redoStack.isEmpty()) redoStack.pop();


						numOfLines++;  // Increment the number of lines

						if (cursor->down) {
							cursor = cursor->down;
						}
						else {
							cursor->down = new Node('|');
							cursor->down->up = cursor;
							cursor = cursor->down;
						}
						cursor = cursor->left;

						gotoxy(x, y);
						printFormat();
						gotoxy(1, 4);
						print(head);
						gotoxy(x, y);
						break;

					case VK_SPACE: // Space key
						if (y >= 24) {
							break;
						}
						if (cursor && cursor->left && cursor->left->data != ' ' && cursor->right && cursor->right->data != ' ') {
							nAry->deletion(getWord(cursor));
							spaceInsertedInBW = true;
						}
						
						// Insert a space character
						insertion(cursor, ' ', head);  // Call the insertion function with space

						if (spaceInsertedInBW) {	//split word into two
							nAry->insert(getWord(cursor->left));
							nAry->insert(getWord(cursor->right));
						}
						if (cursor->left && cursor->left->data != ' ' && cursor->left->data != '|') {
							chilliMilli->insert(getWord(cursor)->word, cursor->left);
						}
						//nAry->markEndOfWord(getWord(cursor));
						//cout << " ";  // Print the space character
						x++;  // Move cursor right after insertion
						if (cursor->left && cursor->left->data != ' ') {
							undoStack.push(head, x, y);
							while (!redoStack.isEmpty()) redoStack.pop();
						}
						if (x >= 100) {

							insertion(cursor, '\n', head);  // Create a new line node

							undoStack.push(head, x, y);
							while (!redoStack.isEmpty()) redoStack.pop();
							numOfLines++;  // Increment the number of lines

							if (cursor->down) {
								cursor = cursor->down;
							}
							else {
								cursor->down = new Node('|');
								cursor->down->up = cursor;
								cursor = cursor->down;
							}
							cursor = cursor->left;

						}
						gotoxy(x, y);
						printFormat();
						gotoxy(1, 4);
						print(head);
						gotoxy(x, y);
						//cout<<" ";
						break;
					default:
						// Only allow alphabetic characters (A-Z, a-z) to be inserted
						if (isAlphabet(eventBuffer[i].Event.KeyEvent.uChar.AsciiChar) && y<24) {
							char keyPressed = eventBuffer[i].Event.KeyEvent.uChar.AsciiChar;
							if (cursor && cursor->data == ' ' && cursor->right && isAlphabet(cursor->right->data)) {
								nAry->deletion(getWord(cursor->right));
							}
							else if (cursor && cursor->data != ' '){
								nAry->deletion(getWord(cursor));
							}
							insertion(cursor, keyPressed, head);  // Call the insertion function
							nAry->insert(getWord(cursor));
							//cout << keyPressed;
							x++;  // Move cursor right after insertion
							gotoxy(x, y);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);
							//cout << keyPressed;
						}
						if (x >= 100) {
							
							insertion(cursor, '\n', head);  // Create a new line node

							undoStack.push(head, x, y);
							while (!redoStack.isEmpty()) redoStack.pop();
							numOfLines++;  // Increment the number of lines
							
							if (cursor->down) {
								cursor = cursor->down;
							}
							else {
								cursor->down = new Node('|');
								cursor->down->up = cursor;
								cursor = cursor->down;
							}
							cursor = cursor->left;
							gotoxy(x, y);
							printFormat();
							gotoxy(1, 4);
							print(head);
							gotoxy(x, y);

						}

						break;
					}					
				}
			} // end EventsRead loop
			
		}
		
	} // end program loop
	freeMemory(head);
	return 0;
}