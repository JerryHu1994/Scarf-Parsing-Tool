#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scarf.h"

ScarfJSONWriter *writer;

// Handler function on the initial callback
void *initialHandler(Initial *initial, void *reference)
{
	/*char **list = (char **)reference;
	printf("%s\n", list[0]);*/
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
	CustomedBugInstance *attributes = (CustomedBugInstance *)reference;
	char *bugErrors = CheckBug(bug);
	if(strcmp(bugErrors,"")){
		fprintf(stderr, "Checkbug failed\n");
	}
	ScarfJSONWriterAddBug(writer, bug, attributes);
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

// Set the CustomizedBugInstance based on different tool name
void setCustomizedAttributes(CustomedBugInstance *currAttributes, char * toolName)
{
	//general case
	currAttributes->bugId = 1;
	currAttributes->resolutionSuggestion = 1;
	currAttributes->bugMessage = 1;
	currAttributes->bugGroup = 1;
	currAttributes->instanceLocation = 1;	

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
	
	// Initialize the ScarfJsoWriter for output Json
	writer = NewScarfJSONWriterFromFilename(outputFile);
	if (writer == NULL) {
        fprintf(stderr, "Creating JSON writer failed\n");
        exit(1);
    }
	ScarfJSONWriterSetErrorLevel(writer, 1);
    ScarfJSONWriterSetPretty(writer, 1);

	// Create the CustomedBugInstance struct storing attribute booleans for the JSON output
	CustomedBugInstance	*currAttributes;
	if ((currAttributes = malloc(sizeof(CustomedBugInstance))) == NULL) {
		fprintf(stderr, "Malloc Failed\n");
		exit(1);
	}

	setCustomizedAttributes(currAttributes, toolName);
	
	// Set the currAttributes as the customed callback data
	void *context = (void *)currAttributes;
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

	// Start parsing the scarf
    Parse(reader);
    
	// End parsing and free the memory
	ScarfJSONWriterAddSummary(writer);
	ScarfJSONWriterAddEndTag(writer);
	DeleteScarfXmlReader(reader);
	DeleteScarfJSONWriter(writer);
	free(currAttributes);
    
	return 0;
}
