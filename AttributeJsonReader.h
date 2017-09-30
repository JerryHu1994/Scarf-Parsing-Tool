#ifndef ATTRIBUTEMAPPING_H
#define ATTRIBUTEMAPPING_H
// An entry maps from a attribute name to valid bit
#ifndef ENTRY
#define ENTRY
typedef struct entry
{
	char *name;
	int valid;
}entry; 
#endif

AttributeJSONReader * NewAttributeJSONReaderFromFile(FILE *file);

AttributeJSONReader * NewAttributeJSONReaderFromFilename(char * filename);

AttributeJSONReader * NewAttributeJSONReaderFromFd(int fd);

void DeleteAttributeJSONReader (AttributeJSONReader * reader);

void AttributeJSONReaderSetUTF8(AttributeJSONReader * reader, int value);

int AttributeJSONReaderGetUTF8(AttributeJSONReader * reader);

void AttributeJSONReaderSetToolname(AttributeJSONReader * reader, char * toolName);

char * AttributeJSONReaderGetToolname(AttributeJSONReader * reader);

void AttributeJSONReaderSetDict(AttributeJSONReader * reader, entry * dict);
void setAttributeValid(entry * input, char *attribute);
int getAttributeValid(entry * input, char *attribute);
#endif

