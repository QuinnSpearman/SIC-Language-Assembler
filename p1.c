#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
	char *symbol;
	int lineNumber;
	int locCtr;
	struct node *next;
}node;

void firstLine(char line[], int *pLocCtr);
void insert(char *symbol, int locCtr, char line[], int lineNumber);
int hashFunction(char *symbol);
void checkIfValidSymbol(char* symbol, char *line);
void exceedsMemoryError(char *line);
void printNodes();

struct node *hashTable[26];

int lineNumber = 0;
int startingAddress = 0;
bool endFound = false;

		
int main(int argc, char *argv[]){
	
	if(argc != 2){
		printf("USAGE: %s <filename>\n", argv[0]);
		return 1;
	}

	FILE *inputFile;

	inputFile = fopen(argv[1], "r");

	if(!inputFile){
		printf("ERROR: COULD NOT OPEN %s FOR READING\n", argv[0]);
		return 1;
	}

	int locCtr;
	int *pLocCtr = &locCtr;
	int tempInt;
	char *subStr;
	char *tempStr;
	FILE *intermediateFile;
	char *locCtrStr;
	char *tempLine;
	int programLength;

	intermediateFile = fopen("intermediate.txt", "w");
	
	char line[1024];
	
	while(fgets(line, 1024, inputFile)){
		
		// If last character of the line is a newline character
		if(line[strlen(line) - 1] == '\n'){
			// Allocate enough memory for all characters except for final character
			tempLine = malloc(strlen(line) - 1);

			// Set tempLine equal to the string at the address of line
			for(int i = 0; i < strlen(line) - 1; i++){
				tempLine[i] = line[i];
			}	
		}
		else{
			strcpy(tempLine, line);
		}

		lineNumber += 1;

		// If the first character of the line is a #, continue to the next line
		if(line[0] == '#' || line[0] == ' ' || line[0] =='\t'){
			continue;
		}// If the first character is between A and Z
		else if(('A' <= line[0]) && (line[0] <= 'Z')){

			firstLine(line, pLocCtr);

			fprintf(intermediateFile, "%s\n", tempLine);
			fprintf(intermediateFile, "%x\n", locCtr);
			
			break;		
		}
		else{
			printf("%s\n");
			printf("[%d] ERROR: INVALD SYMBOL, STARTING CHARACTER MUST BE CAPITAL\n", lineNumber);
			remove("intermedite.txt");
			exit(0);
		}
	}

	if(locCtr > 32768){
		exceedsMemoryError(line);
	}

	
	
	while(fgets(line, 1024, inputFile)){
		
		
		// If last character of the line is a newline character
		if(line[strlen(line) - 1] == '\n'){
			// Allocate enough memory for all characters except for final character
			tempLine = malloc(strlen(line) - 1);
			// Set tempLine equal to the string at the address of line
			for(int i = 0; i < strlen(line) - 1; i++){
				tempLine[i] = line[i];
			}	
		}
		else{
			strcpy(tempLine, line);
		}

		lineNumber += 1;

		char *token = strtok(line, " \t");
			
		// If line is a comment line, continue to next line		
		if(line[0] == '#'){
			continue;
		}
		// If line is a symbol line, print the symbol and location counter
		else if(('A' <= line[0]) && (line[0] <= 'Z')){
			char *symbol = malloc(strlen(token));

			strcpy(symbol, token);

			checkIfValidSymbol(symbol, line);		                        
													                                         
			insert(symbol, locCtr, tempLine, lineNumber);
			printf("%s\t%x\n", token, locCtr);
			token = strtok(NULL, " \t");
		}
		else if(line[0] != ' ' && line[0] != '\t'){
			printf("%s\n", tempLine);
			printf("[%d] ERROR: INVALID SYMBOL, STARTS WITH INVALID CHARACTER\n", lineNumber);
			remove("intermediate.txt");
			exit(0);
		}

		if(strcmp(token, "END") == 0){
			locCtr += 3;
			fprintf(intermediateFile, "%s\n", tempLine);
		    fprintf(intermediateFile, "%x\n", locCtr);
			endFound = true;
			
			break;
		}
		else if(strcmp(token, "START") == 0){
			printf("%s\n", tempLine);
			printf("[%d] INVALID OPCODE OR DIRECTIVE, START MUST BE ON FIRST LINE\n", lineNumber);
			remove("intermediate.txt");
			exit(0);
		}
		else if(strcmp(token, "RESW") == 0){
			token = strtok(NULL, " \t");
			sscanf(token, "%d", &tempInt);
			locCtr += (3 * tempInt);

		}
		else if(strcmp(token, "RESB") == 0){
			token = strtok(NULL, " \t");
			sscanf(token, "%d", &tempInt);
			locCtr += tempInt;
		}
		else if(strcmp(token, "WORD") == 0){
			token = strtok(NULL, " \t");
			sscanf(token, "%d", &tempInt);

			if(tempInt > 8388608){
				printf("%s\n", tempLine);
				printf("[%d] ERROR: CONSTANT EXCEEDS WORD SIZE\n", lineNumber);
				remove("intermediate.txt");
				exit(0);
			}

			locCtr += 3;
		}
		else if(strcmp(token, "BYTE") == 0){
			token = strtok(NULL, " \t");
			tempStr = strtok(token, "\'");
			tempStr = strtok(NULL, "\'");		

			if(token[0] == 'C'){ 
				locCtr += strlen(tempStr);
			}
			else if(token[0] == 'X'){
				if(strlen(tempStr) % 2 != 0){
					printf("%s\n", tempLine);
					printf("[%d] ERROR: INVALID NUMBER OF CHARACTERS IN HEXADECIMAL INPUT\n", lineNumber);	
					remove("intermediate.txt");
					exit(0);
				}

				for(int i = 0; i < strlen(tempStr); i++){
					if(!(('0' <= tempStr[i] && tempStr[i] <= '9') || ('A' <= tempStr[i] && tempStr[i] <= 'F'))){
						printf("%s\n", tempLine);
						printf("[%d] ERROR: INVALID HEX CHARACTER\n", lineNumber);
						remove("intermediate.txt");
						exit(0);
					}
				}
				locCtr += (strlen(tempStr) / 2);
			}
			else{
				printf("%s\n", tempLine);
				printf("[%d] ERROR: INVALID DATA TYPE FOR BYTE DIRECTIVE \n", lineNumber);
				remove("intermediate.txt");
				exit(0);
			}
		}
		else{
			locCtr += 3;
		}

		if(locCtr > 32768){
			exceedsMemoryError(tempLine);
		}
		

		fprintf(intermediateFile, "%s\n", tempLine);
		fprintf(intermediateFile, "%x\n", locCtr);

	}

	if(!endFound){
		printf("%s\n", tempLine);
		printf("[%d] ERROR: NO END DIRECTIVE FOUND IN FILE\n", lineNumber);
		remove("intermediate.txt");
		exit(0);		
	}	

	programLength = locCtr - startingAddress;

	//printNodes();	

	fclose(intermediateFile);
	fclose(inputFile);

	

	return 0;
}

void firstLine(char line[], int *pLocCtr){

	char *token = strtok(line, " \t");
	char *symbol = malloc(strlen(token));
	
	for(int i = 0; i < strlen(token); i++){
		symbol[i] = token[i];
	}	

	checkIfValidSymbol(symbol, line);
	
	// directive/opcode
	token = strtok(NULL, " \t");
	
	if(strcmp(token, "START" ) == 0){
		token = strtok(NULL, " \t");
		if(!('0' <= token[0] && token[0] <= '9')){
			printf("%s\n", line);
			printf("[%d] ERROR: INVALID START OPERAND, MUST BE NUMERICAL\n", lineNumber);
			remove("intermediate.txt");
			exit(0);			
		}
		sscanf(token, "%x", pLocCtr);
		if(pLocCtr < 0){
			printf("%s\n", line);
			printf("[%d] ERROR: INVALID START OPERAND, MUST BE POSITIVE\n", lineNumber);
			remove("intermediate.txt");
			exit(0);			
		}
	}
	else{
		printf("%s\n", line);
		printf("[%d] ERROR: INVALID START DIRECTIVE, MUST BE \"START\"\n", lineNumber);
		remove("intermediate.txt");
		exit(0);
	}

	startingAddress = *pLocCtr;
	
	insert(symbol, *pLocCtr, line, lineNumber);
	printf("%s\t%x\n",symbol,  *pLocCtr);
}

void insert(char *symbol, int locCtr, char line[], int lineNumber){

	int key = hashFunction(symbol);

	// Allocates memory for the new node and assigns values
	node *newNode = (struct node*)malloc(sizeof(struct node));
	newNode->symbol = symbol;
	newNode->lineNumber = lineNumber;
	newNode->locCtr = locCtr;

	char *newSymbolStr = malloc(strlen(symbol));

	strcpy(newSymbolStr, symbol);

	if(hashTable[key] == NULL){
		hashTable[key] = (struct node*)malloc(sizeof(struct node));
		hashTable[key] = newNode;
	}
	else{
		struct node *temp;
		temp = hashTable[key];

		char *oldSymbolStr = malloc(strlen(symbol));

		while(temp->next != NULL){

			strcpy(oldSymbolStr, temp->symbol);

			if(strcmp(oldSymbolStr, newSymbolStr) == 0){
				printf("%s\n", line);
				printf("[%d] ERROR: DUPLICATE SYMBOL\n", lineNumber);
				remove("intermediate.txt");
				exit(0);
			}
			temp = temp->next;
		}
		temp->next = newNode;

		strcpy(oldSymbolStr, temp->symbol);

        if(strcmp(oldSymbolStr, newSymbolStr) == 0){
		printf("%s\n", line);
		printf("[%d] ERROR: DUPLICATE SYMBOL\n", lineNumber);
		remove("intermediate.txt");
	  	exit(0);	
		}

	}



}

int hashFunction(char *symbol){
	return (symbol[0] + symbol[strlen(symbol) - 1]) % 26;
}

void checkIfValidSymbol(char *symbol, char *line){
	if(strcmp(symbol, "START") == 0 || strcmp(symbol, "END") == 0 || strcmp(symbol, "BYTE") == 0 || strcmp(symbol, "WORD") == 0 || strcmp(symbol, "RESB") == 0 || strcmp(symbol, "RESW") == 0 || strcmp(symbol, "RESR") == 0 || strcmp(symbol, "EXPORTS") == 0){
		printf("%s\n", line);
		printf("[%d] ERROR: INVALID SYMBOL, MATCHES DIRECTIVE\n", lineNumber);
		remove("intermediate.txt");
		exit(0);
	}
	else if(strlen(symbol) > 6){
		printf("%s\n", line);
		printf("[%d] ERROR: INVALID SYMBOL, EXCEEDS 6 CHARACTERS IN LENGTH\n", lineNumber);
		remove("intermediate.txt");
		exit(0);
	}

	for(int i = 0; i < strlen(symbol); i++){
		if(symbol[i] == '$' || symbol[i] == '!' || symbol[i] == '=' || symbol[i] == '+' || symbol[i] == '-' || symbol[i] == ',' || symbol[i] == '@'){
		printf("%s\n", line);
		printf("[%d] ERROR: INVALID SYMBOL, CONTAINS INVALID CHARACTER\n", lineNumber);
		remove("intermediate.txt");
		exit(0);
		}
	}
}

void exceedsMemoryError(char* line){
		printf("%s\n", line);
		printf("[%d] ERROR: LOCATION COUNTER EXCEEDS SIC SYSTEM MEMORY\n", lineNumber);
		remove("intermediate.txt");
		exit(0);	
}