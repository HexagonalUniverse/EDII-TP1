// <pesquisa.c>

#include "external-search.h"
#include <sys/time.h>   // For <gettimeofday>.


/*  Indentifier for each of the searching methods. */
typedef enum
{
    INDEXED_SEQUENTIAL_SEARCH,
    EXTERNAL_BINARY_SEARCH,
    BTREE_SEARCH,
    BSTAR_SEARCH
} SEARCHING_METHOD;

/*  Identifier for the possible file-situations. */
typedef enum
{
    ASCENDING_ORDER = 1,
    DESCENDING_ORDER,
    DISORDERED
} SITUATION;


/*  */
static void
PrintSearchResults(search_result * _Sr)
{
    // printf("\t| printando hehe %p\n", _Sr);
    printf("Registry data 1: %ld\nRegistry data 2: %.4s\nRegistry data 3: %.4s\n", 
        (long) _Sr -> target.data_1, _Sr -> target.data_2, _Sr -> target.data_3);
}

/*  Parses the arguments for the main-program. 
    Returns by ref. the method, situation and the key. */
static bool
_ParseArgs(int argc, char ** argsv, SEARCHING_METHOD * _Method, SITUATION * _Situation, key_t * _Key, uint64_t * _Qtt)
{
    // Validation of the searching format "./'searchfile.exe' <method> <quantity> <situation> <key> <-P>"
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Error: incorrect number of arguments: given %d, expected 5.", argc);
        return false;
    }
    
    // Getting the method number from terminal
    * _Method = atoi(argsv[1]);
    
    // Validating the method number
    if (! in_range(0, 3, * _Method)) {
        fprintf(stderr, "[%s] method\n", __func__);
        return false;
    }
        
    // Getting the situation of the file from terminal
    * _Situation = atoi(argsv[3]);
    
    // Validating the situation number
    if (! in_range(1, 3, * _Situation)) {
        fprintf(stderr, "[%s] situation\n", __func__);
        return false;
    }
    
    int64_t x = atoi(argsv[2]);
    if (x < 0) {
        printf("asd\n");
        return false;
    }
    * _Qtt = x;


    // Getting the wanted key from terminal
    * _Key = atoi(argsv[4]);


    return true;
}

/*  */
static bool
__EBS(const key_t _Key, search_result * result)
{
    

    printf("key: %d, result: <%p>\n", _Key, result);
    return false;
}

/*  Applies the B-Tree construction-search for the given filenames. 
    Searches for the given key. Returns success in the process.
    In case of success, the registry is written into the <target> 
    in the result. */
static bool
__BTREE(const key_t _Key, search_result * result, 
    const char * _InputFilename, const char * _OutputFilename) 
{   
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    // Opening both input and output file-streams 
    // for the construction.

    REG_STREAM * input_stream = (REG_STREAM *) fopen(_InputFilename, "rb");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
        return false;
    }
    
    B_STREAM * output_stream = (B_STREAM *) fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output B-TREE file won't open....\n");
        fclose(input_stream);
        return false;
    }

    // Measuring the time before construction,
    gettimeofday(& start_time, NULL);
    
    if (! BTree_Build(input_stream, output_stream))
    {

        printf("[%s]: BTREE_BUILD ERROR\n", __func__);

        fclose(input_stream);
        fclose(output_stream);
        return false;
    }
    
    // measuring the time after it.
	gettimeofday(& end_time, NULL);

    // Interpreting the time the contruction took in [seconds].
	result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    

    // Re-opening the output B-Tree stream in the read mode.
    fclose(output_stream);
    output_stream = (B_STREAM *) fopen(_OutputFilename, "rb");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output file won't open....\n");
        fclose(input_stream);
        return false;
    }
    
    /*  The frame for search. As there will only be requested one 
        search operation, the role of the frame is not fundamentally
        important and its advantage is not used. */
    frame_t frame; makeFrame(&frame, sizeof(b_node));
    
    // Measuring the time in between the search.
    gettimeofday(& start_time, NULL);
        bool search_response = BTree_Search(_Key, input_stream, output_stream, & frame, & result -> target);
    gettimeofday(& end_time, NULL);

    // Interpreting the time the search took in [seconds].
    result -> measures.time_span = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));

    freeFrame(frame);

    fclose(input_stream);
    fclose(output_stream);

    if (! search_response)
    {
        printf("Key not found!\n");
        return false;
    }
    return true;
}

/*  */
static bool
__BSTAR(const key_t _Key, search_result * result, const char * _InputFilename, const char * _OutputFilename)
{
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    // Opening both input and output file-streams 
    // for the construction.

    REG_STREAM * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
        return false;
    }
    
    BSTAR_STREAM * output_stream = fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output B-STAR-TREE file won't open....\n");
        fclose(input_stream);
        return false;
    }

    // Measuring the time before construction,
    gettimeofday(& start_time, NULL);
    
    if (! BSTree_Build(input_stream, output_stream))
    {
        printf("[%s]: BSTREE_BUILD ERROR\n", __func__);

        fclose(input_stream);
        fclose(output_stream);
        return false;
    }
    
    // measuring the time after it.
	gettimeofday(& end_time, NULL);

    // Interpreting the time the contruction took in [seconds].
	result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    
    // Re-opening the output B-Tree stream in the read mode.
    fclose(output_stream);
    output_stream = fopen(_OutputFilename, "rb");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output file won't open....\n");
        fclose(input_stream);
        return false;
    }
    
    /*  The frame for search. As there will only be requested one 
        search operation, the role of the frame is not fundamentally
        important and its advantage is not used. */
    frame_t frame = { 0 }; makeFrame(& frame, sizeof(b_node));
    
    // Measuring the time in between the search.
    gettimeofday(& start_time, NULL);
        bool search_response = BSTree_Search(_Key, input_stream, output_stream, & frame, & result -> target);
    gettimeofday(& end_time, NULL);

    // Interpreting the time the search took in [seconds].
    result -> measures.time_span = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));

    fclose(input_stream);
    fclose(output_stream);
    freeFrame(frame);


    if (! search_response)
    {
        printf("Key not found!\n");
        return false;
    }
    return true;
}

/*  */
static bool
__ISS(const key_t _Key, search_result * result, const SITUATION situation, const uint64_t reg_qtt,
    const char * _InputFilename)
{
    printf("%u\n", (unsigned int) sizeof(void *));
    struct timeval start_time, end_time;
    
    FILE * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL) {
        DebugPrintR("[ERROR] Input file won't open...\n", NULL);
    }
    
    IndexTable index_table = { 0 }; 

    gettimeofday(& start_time, NULL);
    if (! buildIndexTable(& index_table, reg_qtt, input_stream)) {
        DebugPrintR("iss:err1\n", NULL);
        return false;
    }
    gettimeofday(& end_time, NULL);

    result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    printf("ALLOCATED INDEX\n");


    frame_t frame = { 0 }; makeFrame(& frame, sizeof(regpage_t));

    gettimeofday(& start_time, NULL);
        bool search_response = indexedSequencialSearch(_Key, input_stream, & index_table, & frame, result, (situation == ASCENDING_ORDER) ? true : false);
    gettimeofday(& end_time, NULL);

    result -> measures.time_span = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    
    deallocateIndexTable(&index_table);
    fclose(input_stream);
    freeFrame(frame);

    if (! search_response) {
        printf("Key not found!\n");
        return false;
    }
    return true;
}

/*  */
static bool
_RedirectSearch(SEARCHING_METHOD method, SITUATION situation, key_t key, uint64_t qtt, search_result * result)
{
    //Validation in case the Indexed Sequential Search is called but the file is not ordered
    if (method == INDEXED_SEQUENTIAL_SEARCH) {   
        if (situation == DISORDERED) {
            printf("The Indexed Sequential Search doesn't support disordered files!\n");
            return false;
        } 
        
        if (__ISS(key, result, situation, qtt, INPUT_DATAFILENAME))
            return true;

        printf("[%s]: ISS\n", __func__);

    } else if (method == EXTERNAL_BINARY_SEARCH) {
        if (__EBS(key, result))
            return true;
        printf("[%s]: EBS\n", __func__);
    }
    
    else if (method == BTREE_SEARCH) {
        if (__BTREE(key, result, INPUT_DATAFILENAME, OUTPUT_BTREE_FILENAME))
            return true;
        printf("[%s]: BTREE\n", __func__);
    }
    
    else if (method == BSTAR_SEARCH) {
        if (__BSTAR(key, result, INPUT_DATAFILENAME, OUTPUT_BSTAR_FILENAME)) 
            return true;
        
        printf("[%s]: SBTAR\n", __func__);
    }
    
    return false;
}

/* "pesquisa <método> <quantidade> <situação> <chave>" */
int main(int argc, char ** argsv)
{
    SEARCHING_METHOD method = 0;
    SITUATION situation = 0;
    key_t key = 0;
    uint64_t reg_qtt = 0;
    search_result result = { 0 };
    
    if (! _ParseArgs(argc, argsv, & method, & situation, & key, & reg_qtt))
        return 2;
    
    printf("parsou zé\n");
    
    if (! _RedirectSearch(method, situation, key, reg_qtt, & result))
    {
        fprintf(stderr, "err3\n");
        return 3;
    }
    
    printf("Search results: <%lf, %lf>\n", result.measures.construction_time, result.measures.time_span);
    
    PrintSearchResults(& result);

    return EXIT_SUCCESS;
}
