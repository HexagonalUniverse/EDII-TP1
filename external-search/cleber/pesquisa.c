#include <stdio.h>
#include <stdlib.h>
#include "external-search.h"
#include <sys/time.h>


/*  */
typedef enum
{
    INDEXED_SEQUENTIAL_SEARCH,
    EXTERNAL_BINARY_SEARCH,
    BTREE_SEARCH,
    BSTAR_SEARCH
} SEARCHING_METHOD;

/*  */
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
        (long) _Sr->target.data_1, _Sr->target.data_2, _Sr->target.data_3);
}

/*  */
static bool
_ParseArgs(int argc, char ** argsv, SEARCHING_METHOD * _Method, SITUATION * _Situation, key_t * _Key)
{
    //Validation of the searching format "./'searchfile.exe' <method> <quantity> <situation> <key> <-P>"
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Error: incorrect number of arguments: given %d, expected 5.", argc);
        return false;
    }
    
    //Getting the method number from terminal
    * _Method = atoi(argsv[1]);
    
    //Validating the method number
    if (! in_range(0, 3, * _Method)) {
        fprintf(stderr, "[%s] method\n", __func__);
        return false;
    }
        
    //Getting the situation of the file from terminal
    * _Situation = atoi(argsv[3]);
    
    //Validating the situation number
    if (! in_range(1, 3, * _Situation)) {
        fprintf(stderr, "[%s] situation\n", __func__);
        return false;
    }
    
    //Getting the wanted key from terminal
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

/*  */
static bool
__BTREE(const key_t _Key, search_result * result, 
    const char * _InputFilename, const char * _OutputFilename) 
{
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    FILE * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
        return false;
    }
    
    FILE * output_stream = fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output B-TREE file won't open....\n");
        fclose(input_stream);
        return false;
    }

    
    // B_Builder b_builder = { output_stream, 0, & frame };

    // B-Tree data-structure construction
    // ----------------------------------
    
    gettimeofday(& start_time, NULL);
    
    if (! BTree_Build(input_stream, output_stream))
    {
        printf("[%s]: BTREE_BUILD ERROR\n", __func__);

        fclose(input_stream);
        fclose(output_stream);
        return false;
    }
    
	gettimeofday(& end_time, NULL);
	result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    
    fclose(output_stream);
    output_stream = fopen(_OutputFilename, "rb");
    
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output file won't open....\n");
        fclose(input_stream);
        return false;
    }
    
    BTreeStream bs = { output_stream, 0, & frame };
    

    gettimeofday(& start_time, NULL);
    bool search_response = BTree_Search(_Key, & bs, input_stream, & result -> target);
    gettimeofday(& end_time, NULL);

    result -> measures.time_span = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));

    if (! search_response)
    {
        printf("Key not found!\n");

        fclose(input_stream);
        fclose(output_stream);
        return false;
    }
    
    printf("KEY: %d, RESULT: <%p>\n", _Key, result);

    fclose(input_stream);
    fclose(output_stream);
    return true;
}

/*  */
static bool
__BSTAR(const key_t _Key, search_result * result, const char * _InputFilename, const char * _OutputFilename)
{
    // 
	struct timeval start_time, end_time;
    
    FILE * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
        return false;
    }
    
    FILE * output_stream = fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output B-STAR-TREE file won't open....\n");
        fclose(input_stream);
        return false;
    }

    frame_t frame = { 0 };
    makeFrame(& frame, sizeof(b_node));
    

    // B-Star-Tree data-structure construction
    // ----------------------------------
    
    gettimeofday(& start_time, NULL);
    
    if (! BSTree_Build(input_stream, output_stream, & frame))
    {
        printf("[%s]: BSTREE_BUILD ERROR\n", __func__);

        fclose(input_stream);
        fclose(output_stream);
        
        freeFrame(frame);
        return false;
    }

    printf("BSTAR TREE BUILT.\n");
    fflush(stdout);

	gettimeofday(& end_time, NULL);
	result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    
    fclose(output_stream);
    output_stream = fopen(_OutputFilename, "rb");
    
    if (output_stream == NULL) {
        fprintf(stderr, "[ERROR] Output file won't open....\n");
        fclose(input_stream);
        freeFrame(frame);
        return false;
    }
    
    BTreeStream bss = { output_stream, 0, & frame };
    
    gettimeofday(& start_time, NULL);
    bool search_response = BSTree_Search(_Key, & bss, input_stream, & result -> target);
    gettimeofday(& end_time, NULL);

    result -> measures.time_span = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));

    if (! search_response)
    {
        printf("Key not found!\n");

        fclose(input_stream);
        fclose(output_stream);
        freeFrame(frame);
        return false;
    }
    
    printf("KEY: %d, RESULT: <%p>\n", _Key, result);

    fclose(input_stream);
    fclose(output_stream);
    
    freeFrame(frame);
    return true;
}

/*  */
static bool
__ISS(const key_t _Key, search_result * result, const SITUATION situation)
{
    struct timeval start_time, end_time;

    FILE * input_stream = fopen("a.x", "rb");
    if (input_stream == NULL) {
        fprintf(stderr, "[ERROR] Input file won't open...\n");
    }

    // tabela

    frame_t frame = { 0 };
    makeFrame(& frame, sizeof(b_node));

    gettimeofday(& start_time, NULL);
    
    if (situation == ASCENDING_ORDER)
    {
        if (! indexedSequencialSearch_OrderedAscending(_Key, input_stream, result))
        {
            printf("Key not found!\n");

            freeFrame(frame);

            gettimeofday(& end_time, NULL);
            fclose(input_stream);
            return false;
        }

    } 
    else if (! indexedSequencialSearch_OrderedDescending(_Key, input_stream, result)) 
    {
        printf("Key not found!\n");

        freeFrame(frame);
        
        gettimeofday(& end_time, NULL);
        fclose(input_stream);
        return false;
    }
    
    gettimeofday(& end_time, NULL);

    printf("KEY: %d, RESULT: <%p>\n", _Key, result);

    fclose(input_stream);
    freeFrame(frame);
    return true;
}

/*  */
static bool
_RedirectSearch(SEARCHING_METHOD method, SITUATION situation, key_t key, search_result * result)
{
    //Validation in case the Indexed Sequential Search is called but the file is not ordered
    if (method == INDEXED_SEQUENTIAL_SEARCH) {   
        if (situation == DISORDERED) {
            printf("The Indexed Sequential Search doesn't support disordered files!\n");
            return false;
        } 
        
        if (__ISS(key, result, situation))
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
    search_result result = { 0 };
    
    if (! _ParseArgs(argc, argsv, & method, & situation, & key))
        return 2;
    
    printf("parsou zé\n");
    
    if (! _RedirectSearch(method, situation, key, & result))
    {
        fprintf(stderr, "err3\n");
        return 3;
    }
    
    printf("Search results: <%lf, %lf>\n", result.measures.construction_time, result.measures.time_span);
    
    PrintSearchResults(& result);

    return EXIT_SUCCESS;
}
