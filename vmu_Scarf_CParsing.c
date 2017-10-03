#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ScarfCommon.h"
#include "AttributeJsonReader.h"
#include "ScarfXml.h"
#include "ScarfJson.h"

// define the size of the maximum attributes
#define attriSize 20
// A list of entries contains one-on-one mapping from 
// attribute name to a valid bit. The default valid bit 
// for all attributes all are 0.
entry dict[] = {
	"BugId", 0,
	"cweIds", 0,
	"cweIdsCount", 0,
	"cweIdsSize", 0,
	"methodsCount", 0,
	"methodsSize", 0,
	"locationsCount", 0,
	"locationsSize", 0,
	"className", 0,
	"bugSeverity", 0,
	"bugRank", 0,
	"resolutionSuggestion", 0,
	"bugMessage", 0,
	"bugCode", 0,
	"bugGroup", 0,
	"assessmentReportFile", 0,
	"buildId", 0,
	"instanceLocation", 0,
	"methods", 0,
	"locations", 0
};

// Set the valid bit for the attribute to 1 
void setAttributeValid(entry * input, char *attribute)
{
	int i;
	int found = 0;	
	for(i=0;i<attriSize;i++){
		if(!strcmp(dict[i].name,attribute)){
			dict[i].valid = 1;
			found = 1;
			return;
		}
	}
	if(!found){
		fprintf(stderr, "Cannot find %s in the attribute list\n", attribute);
		return;
	}
	return; 
}

// Return the valid bit of the attribute
// Otherwise, return -1 if the attribute is not found.
int getAttributeValid(entry * input, char *attribute)
{
	int i;
	int found = 0;	
	for(i=0;i<attriSize;i++){
		if(!strcmp(dict[i].name,attribute)){
			found = 1;
			return dict[i].valid;
		}
	}
	if (!found) {
		fprintf(stderr, "Cannot find %s in the attribute list\n", attribute);
		return -1;
	}
	return -1;
}

ScarfJSONWriter *writer;

// Handler function on the initial callback
void *initialHandler(Initial *initial, void *reference)
{
	char *initialErrors = CheckStart(initial);
	if(strcmp(initialErrors,"")){
		fprintf(stderr, "Checkstart failed\n");
	}
	ScarfJSONWriterAddStartTag(writer, initial);
	return NULL;
}

// Handler function on the bug instance callback
void *bugHandler(BugInstance *bug, void *reference)
{
	entry *attributeList = (entry *)reference;
	char *bugErrors = CheckBug(bug);
	if(strcmp(bugErrors,"")){
		fprintf(stderr, "Checkbug failed\n");
	}
	ScarfJSONWriterAddBug(writer, bug, attributeList);
	return NULL;
}

// Handler function on the metric callback
void *metricHandler(Metric *metr, void *reference)
{
	char *metricErrors = CheckMetric(metr);
	if(strcmp(metricErrors,"")){
		fprintf(stderr, "Checkmetric failed\n");
	}
	ScarfJSONWriterAddMetric(writer, metr);
	return NULL;
}

// Handler function on the metric summary callback
void *metricSummaryHandler(MetricSummary *metrSum, void *reference)
{
	return NULL;
}

// Handler function on the bug summary callback
void *bugSummaryHandler(BugSummary *bugSum, void *reference)
{
	return NULL;
}

// Handler function on the final callback
void *finalHandler(void *killValue, void *reference)
{
	return NULL;
}

// For debug use
void printDict(){	
	int i;
	printf("Printing the attributes: \n");
	for(i=0;i<20;i++){
		printf("%s -> %d\n", dict[i].name, dict[i].valid);
	}
}

int main(int argc, char **argv) {
    
	//check the input arguments
	if (argc != 7 || strcmp(argv[1], "-input_dir") || strcmp(argv[3], "-output_dir") || strcmp(argv[5], "-tool_name")) {
		fprintf(stderr, "The input arguments should be: ./vmu_Scarf_CParsing.c -input_dir ... -output_dir ... -tool_name ...\n");
		exit(1);
	}
	char *inputFile;
	char *outputFile;
	char *toolName;
	inputFile = strdup(argv[2]);
	outputFile = strdup(argv[4]);
	toolName = strdup(argv[6]);
	
	// Create the ScarfXmlReader from the scarf file
	ScarfXmlReader *reader = NewScarfXmlReaderFromFilename(inputFile, "UTF-8");
    if(reader == NULL){
        fprintf(stderr, "Creating XML reader failed\n");
        exit(1);
    }
	
	// Create the ScarfJsoWriter for output Json
	writer = NewScarfJSONWriterFromFilename(outputFile);
	if (writer == NULL) {
        fprintf(stderr, "Creating JSON writer failed\n");
        exit(1);
    }
	ScarfJSONWriterSetErrorLevel(writer, 1);
    ScarfJSONWriterSetPretty(writer, 1);

	// Create the AttributeJSONReader from the Scarf_ToolList.json
	AttributeJSONReader *attriReader = NewAttributeJSONReaderFromFilename("Scarf_ToolList.json");

	if (attriReader == NULL) {
		fprintf(stderr, "Creating attribute JSON reader failed\n");
		exit(1);
	} 
	// Set the entry list and toolname	
	AttributeJSONReaderSetDict(attriReader, dict);
	AttributeJSONReaderSetToolname(attriReader, toolName);

	AttributeJSONReaderParse(attriReader);
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	printDict();
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	// Set the currAttributes as the customed callback data
	void *context = (void *)dict;
	SetCallbackData(reader, context);
	
	// Set the callback functions associated with the XML reader
	InitialCallback initialFunction = &initialHandler;
	BugCallback bugFunction = &bugHandler;
	BugSummaryCallback bugSummaryFunction = &bugSummaryHandler;
	MetricCallback metricFunction = &metricHandler;
	MetricSummaryCallback metricSummaryFunction = &metricSummaryHandler;
	FinalCallback finishFunction = &finalHandler;

    SetInitialCallback(reader, initialFunction);
    SetBugCallback(reader, bugFunction);
    SetMetricCallback(reader, metricFunction);
    SetBugSummaryCallback(reader, bugSummaryFunction);
    SetMetricSummaryCallback(reader, metricSummaryFunction);
    SetFinalCallback(reader, finishFunction);

	AttributeJSONReaderParse(attriReader);

	// Start parsing the scarf
    Parse(reader);
    
	// End parsing and free the memory
	ScarfJSONWriterAddSummary(writer);
	ScarfJSONWriterAddEndTag(writer);
	DeleteScarfXmlReader(reader);
	DeleteAttributeJSONReader(attriReader);	
    
	return 0;
}
