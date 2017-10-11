/*
 * The AttributeJSONReader will be used to parse the Scarf_ToolList.json, and fetch a list of attributes
 * associated with a certain assessement tool. Yajl API will be used to parse the json file. 
 * Author: Jieru Hu
*/

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ScarfCommon.h"
#include "AttributeJsonReader.h"

extern entry dict[];

// The state contains a list of state variables for the JSON parser
typedef struct State {
	int inGenericList; // indicates the praser is in the Generic Tool list
	int inScarfAttributes; // indicates the parser is in the ScarfAtrributes
	int inGenericAttributes; // indicates the parser is in the Generic Attributes
	int inNonGenericAttributes; // indicates the parser in the nonGeneric Attributes matched with tool
	int genericTool; //indicates if the assessment tool is generic for fetching attributes
	char *toolName;
	int intoGenericTool;
	entry *entryList;	
} State;

// The reader for the attribute JSON file
struct AttributeJSONReader {
	FILE *file;
	yajl_handle reader;
	State *state;
	int utf8;
};

////////////////////////////handlers//////////////////////////////
static int handle_null(void * ctx)
{
	(void)ctx;
	//printf("Handling a NULL\n");
	return 1;
}

static int handle_boolean(void *data, int boolean)
{
	//printf("Handling a boolean\n");
	return 1;
}

static int handle_number(void * data, const char * s, size_t l)
{
	//printf("Handling a number\n");
	return 1;
}

static int handle_string(void * data, const unsigned char *string, size_t stringLen)
{
	//printf("Handling a string\n");
	//printf("%s   %d \n", currString, stringLen);
	State *currState = (State *)data;
	char *fullString = (char *)string;
	char *currString = malloc(stringLen + 1);
	assert(currString != NULL);	
	strncpy(currString, fullString, stringLen);

	// If the parser is in the Generic-Toolist, check if the toolName matches one of the generic tool
	if (currState->inGenericList) {
		if (!strcmp(currString, currState->toolName) && !(currState->genericTool)){
			printf("Found the generic tool!\n");
			currState->genericTool = 1;
			return 1;	
		}
	}
	// IF the parser is in the Generic Attributes and the tool is generic, set all the attributes in the 
	// generic list to valid
	if (currState->genericTool && currState->inGenericAttributes) {
		setAttributeValid(currState->entryList, currString);	
		return 1;
	}

	//If the parser has found its nonGeneric tool
	if (currState->inNonGenericAttributes){
		setAttributeValid(currState->entryList, currString);	
		return 1;
	}	
	return 1;
}

static int handle_map_key(void * data, const unsigned char * string, size_t stringLen)
{
	//printf("Handling a map key\n");
	//printf("%s   %d \n", currString, stringLen);
	State *currState = (State *)data;
	char *fullString = (char *)string;
	char *currString = malloc(stringLen + 1);
	assert(currString != NULL);	
	strncpy(currString, fullString, stringLen);


	// hitting the Generic-ToolList label and save the current state	
	if (!strcmp(currString, "Generic-Toollist")) {
		currState->inGenericList = 1;
	}
	// hitting the ScarfAttributes
	if (!strcmp(currString, "ScarfAttributes")) {
		currState->inScarfAttributes = 1;
	}
	// hitting the Generic in the ScarfAttributes
	if (currState->inScarfAttributes && !strcmp(currString, "generic")) {
		currState->inGenericAttributes = 1;
	}
	// if a toolname is not found both in Generic-Toollist and NonGenericAttributes, set it to Generic-Tool
	if (currState->inScarfAttributes && !strcmp(currString, "generic") && !(currState->inNonGenericAttributes)) {
		currState->genericTool = 1;
		currState->inGenericAttributes = 1;
	}
	// hitting the nongeneric matched tool
	if (currState->inScarfAttributes && !currState->genericTool && !strcmp(currString, currState->toolName)){
		currState->inNonGenericAttributes = 1;		
	}
	
	return 1;
}
static int handle_start_map(void * data)
{
	//printf("Handling a start map\n");
	return 1;
}
static int handle_end_map(void * data)
{
	//printf("Handling a end map\n");
	return 1;
}
static int handle_start_array(void * data)
{
	//printf("Handling a start array\n");
	return 1;
}
static int handle_end_array(void * data)
{
	//printf("Handling a end array\n");
	State *currState = (State *)data;
	// leaving the Generic toollist array
	if(currState->inGenericList) {
		currState->inGenericList = 0;
	}
	// leaving the Scarf Attributes array
	if(currState->inGenericAttributes) {
		currState->inGenericAttributes = 0;
	}
	// leaving the Non Generic Attributes
	if(currState->inNonGenericAttributes) {
		currState->inNonGenericAttributes = 0;
	}
	return 1;
}

// define the callbacks
static yajl_callbacks callbacks = {
	handle_null,
	handle_boolean,
	NULL,
	NULL,
	handle_number,
	handle_string,
	handle_start_map,
	handle_map_key,
	handle_end_map,
	handle_start_array,
	handle_end_array
};

// Initialzie the JSON reader from a FILE pointer
AttributeJSONReader * NewAttributeJSONReaderFromFile(FILE *file)
{
	AttributeJSONReader * reader = calloc(1, sizeof(struct AttributeJSONReader));
	if (reader == NULL) {
		fprintf(stderr, "JSON reader could not be created\n");	
	}
	reader->file = file;
	reader->state = calloc(1, sizeof(State));	
	//reader->state->entryList = dict; 
	reader->utf8 = 1;
	return reader;
}

// Initialize the JSON reader from the file name
AttributeJSONReader * NewAttributeJSONReaderFromFilename(char * filename)
{
	AttributeJSONReader * reader = calloc(1, sizeof(struct AttributeJSONReader));
	if (reader == NULL) {
		fprintf(stderr, "JSON reader could not be created\n");	
	}
	reader->file = fopen(filename, "r");
	if (reader->file == NULL) {
		fprintf(stderr, "File could not be opened\n");
		return NULL;
	}
	reader->state = calloc(1, sizeof(State));	
	//reader->state->entryList = dict;
	reader->utf8 = 1;
	return reader;	
}

// Initialize the JSON reader from a file descriptor
AttributeJSONReader * NewAttributeJSONReaderFromFd(int fd)
{
	AttributeJSONReader * reader = calloc(1, sizeof(struct AttributeJSONReader));
	if (reader == NULL) {
		fprintf(stderr, "JSON reader could not be created\n");	
	}
	reader->file = fdopen(fd, "w");
	reader->state = calloc(1, sizeof(State));	
	reader->utf8 = 1;
	return reader;	
}

// Delete the JSON reader
void DeleteAttributeJSONReader (AttributeJSONReader * reader)
{
	yajl_free(reader->reader);
	fclose(reader->file);
	free(reader);
}

// Set the UTF8 of the attribute reader
void AttributeJSONReaderSetUTF8(AttributeJSONReader * reader, int value)
{
	reader->utf8 = value;
}

// Return the UTF8 of the attribute reader
int AttributeJSONReaderGetUTF8(AttributeJSONReader * reader)
{
	return reader->utf8;
}

// Set the toolname of the attribute reader
void AttributeJSONReaderSetToolname(AttributeJSONReader * reader, char * toolName)
{
	if (toolName == NULL) {
		fprintf(stderr, "The toolname cannot be NULL\n");
		return;
	}
	if (reader == NULL) {
		fprintf(stderr, "The AttributeJSONReader cannot be NULL\n");
		return;
	}
	reader->state->toolName = toolName; 
}
// Retrun the toolName of the attribute reader
char * AttributeJSONReaderGetToolname(AttributeJSONReader * reader)
{
	if (reader == NULL) {
		fprintf(stderr, "The AttributeJSONReader cannot be NULL\n");
		return;
	}
	return reader->state->toolName;
}

// Set the dict list into the attribute reader
void AttributeJSONReaderSetDict(AttributeJSONReader * reader, entry * dict){
	if(dict == NULL) {
		fprintf(stderr, "The entry list cannot be NULL\n");	
		return;
	}
	reader->state->entryList = dict;
	return;
}

// Start parsing the Scarf_ToolList.json with the attribute reader
void * AttributeJSONReaderParse(AttributeJSONReader * hand)
{
	hand->reader = yajl_alloc(&callbacks, NULL, hand->state);
	yajl_config(hand->reader, yajl_dont_validate_strings, hand->utf8);
	int retVal = 0;
	static unsigned char fileData[65536];
	size_t rd;
	yajl_status stat;
	FILE * fh = hand->file;
	for (;;) {	
		rd = fread((void *)fileData, 1, sizeof(fileData)-1, fh);
		if(rd == 0){
			if(!feof(fh)){
				fprintf(stderr, "Error on the file reading\n");
				retVal =1;
			}else {
				printf("Reaching the end of file\n");
			}
			break;
		}
		fileData[rd] = 0;
		stat = yajl_parse(hand->reader, fileData, rd);
		if (stat != yajl_status_ok)	break;
	}
	
	// return the final value
	return 0;
}
