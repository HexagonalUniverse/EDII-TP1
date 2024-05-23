// <pesquisa.c>

#include "external-search.h"
#include <sys/time.h>   // For <gettimeofday>.


#ifdef TRANSPARENT_COUNTER
struct __transparent_counter_t transparent_counter = { 0 };


static void
PrintCounter(void)
{
    printf("Counters:\n");
    printf("\treg:\n\t\t| read = %" PRIu64 "\n\t\t| read = %" PRIu64 "\n", 
        transparent_counter.reg.read,
        transparent_counter.reg.write);

    printf("\tb:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.b.read,
        transparent_counter.b.write);

    printf("\tb*:\n\t\t| read = %" PRIu64 "\n\t\t| write = %" PRIu64 "\n",
        transparent_counter.bs.read,
        transparent_counter.bs.write);
    
    printf("\tcomparisons:\n\t\t| search = %" PRIu64 "\n\t\t| build = %" PRIu64 "\n",
        transparent_counter.comparisons.search,
        transparent_counter.comparisons.build);
}
#endif



// klahsbdhkoas
finline static double 
time_diff_sec(struct timeval start_time, struct timeval end_time) {
    return ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
}


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
    for (uint8_t i = 0; i < 20; i++)
        putchar('-');
    putchar('\n');

    printf("Searching for key <%d>:\n", _Sr->target.key);

    printf("\t> Data-structures construction-time: %lf [s]\n", _Sr->measures.construction_time);
    printf("\t> Searching time: %lf [s]\n", _Sr->measures.time_span);

    if (_Sr->success) {
        printf("\t> The registry:\n");
        printf("\t\t| data1 = %" PRIi64 "\n", (long long int) _Sr->target.data_1);
        printf("\t\t| data2 = %.4s\n", _Sr->target.data_2);
        printf("\t\t| data3 = %.4s\n", _Sr->target.data_3);
    }
    else {
        printf("\t- Failure\n");
    }
}

/*  Parses the arguments for the main-program. 
    Returns by ref. the method, situation and the key. */
static bool
_ParseArgs(int argc, char ** argsv, SEARCHING_METHOD * _Method, SITUATION * _Situation, key_t * _Key, uint64_t * _Qtt)
{
    // Validation of the searching format "./'searchfile.exe' <method> <quantity> <situation> <key> <-P>"
    if (! in_range(5, 6, argc)) {
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
static SEARCH_RESPONSE
__EBST(const key_t _Key, search_result * result, SITUATION _Situation, uint64_t _Qtt,
    const char * _InputFilename, const char * _EBSTFilename, const char * _ERBTFilename)
{
    // Time-measure variables.
    struct timeval start_time, end_time;
    
    // Opening both input and output file-streams
    // for the construction.

    REG_STREAM * input_stream = (REG_STREAM *) fopen(_InputFilename, "rb");
    if (input_stream == NULL)
        return _SE_REGDATAFILE;
    
    FILE * output_stream = NULL;
    

    frame_t frame = { 0 };
    if (!makeFrame(&frame, sizeof(regpage_t)))
        return _SE_MAKEFRAME;
    
    /*  If the input registries file is disordered, then
        the external red-black tree is built. */
    if ((_Situation == DISORDERED) || (_Situation == DESCENDING_ORDER)) {
        output_stream = fopen(_ERBTFilename, "w+b");
        if (output_stream == NULL) {
            fclose(input_stream);
            return _SE_EBST_FILE;
        }

        gettimeofday(&start_time, NULL);
        if (! ERBT_Build(input_stream, (ERBT_STREAM *) output_stream)) {
            fclose(input_stream); fclose(output_stream);
            return _SE_ERBTBUILD;
        }

        gettimeofday(&end_time, NULL);
        result->measures.construction_time = time_diff_sec(start_time, end_time);

        printRedBlackTree(output_stream);

    /*  If it is ordered in ascending order, then the EBST 
        is built by MRT. */
    } else {    // _Situation == ASCENDING_ORDER
        output_stream = fopen(_EBSTFilename, "w+b");
        if (output_stream == NULL) {    
            fclose(input_stream);
            return _SE_EBST_FILE;
        }

        printf("before building\n"); fflush(stdout);
        gettimeofday(& start_time, NULL);
        if (! EBST_MRT_Build(input_stream, (EBST_STREAM *) output_stream, & frame, _Situation == ASCENDING_ORDER, _Qtt)) {
            fclose(input_stream); fclose(output_stream);
            return _SE_EBSTMRTBUILD;
        }
        gettimeofday(& end_time, NULL);
        result -> measures.construction_time = time_diff_sec(start_time, end_time);

        printf(">>> ebst built\n"); fflush(stdout);

        fclose(output_stream);
        output_stream = fopen(_EBSTFilename, "rb");
        if (output_stream == NULL) {
            fclose(input_stream);
            return _SE_EBST_FILE;
        }

        gettimeofday(& start_time, NULL);
            result -> success = EBST_Search((EBST_STREAM *) output_stream, input_stream, _Key, & result -> target);
        gettimeofday(& end_time, NULL);

        result -> measures.time_span = time_diff_sec(start_time, end_time);

    }

    freeFrame(frame);
    fclose(input_stream); fclose(output_stream);
    
    if (result->success) {
        return SEARCH_SUCCESS;
    }

    return SEARCH_FAILURE;
}

/*  B-Tree search engine. Applies the B-Tree construction and 
    search for the given filenames. In case of success, the registry 
    is written into the <target> in the result. */
static SEARCH_RESPONSE
__BTREE(const key_t _Key, search_result * result, 
    const char * _InputFilename, const char * _OutputFilename) 
{   
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    // Opening both input and output file-streams 
    // for the construction.

    REG_STREAM * input_stream = (REG_STREAM *) fopen(_InputFilename, "rb");
    if (input_stream == NULL)
        return _SE_REGDATAFILE;
    
    B_STREAM * output_stream = (B_STREAM *) fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fclose(input_stream);
        return _SE_BFILE;
    }

    // Measuring the time before construction,
    gettimeofday(& start_time, NULL);
    
    if (! BTree_Build(input_stream, output_stream))
    {
        fclose(input_stream);
        fclose(output_stream);
        return _SE_BBUILD;
    }
    
    // measuring the time after it.
	gettimeofday(& end_time, NULL);

    // Interpreting the time the contruction took in [seconds].
	result -> measures.construction_time = time_diff_sec(start_time, end_time);
    

    // Re-opening the output B-Tree stream in the read mode.
    fclose(output_stream);
    output_stream = (B_STREAM *) fopen(_OutputFilename, "rb");
    if (output_stream == NULL) {
        fclose(input_stream);
        return _SE_BFILE;
    }
    
    /*  The frame for search. As there will only be requested one 
        search operation, the role of the frame is not fundamentally
        important and its advantage is not used. */
    frame_t frame = { 0 };
    if (! makeFrame(& frame, sizeof(b_node))) {
        fclose(input_stream);
        fclose(output_stream);
        return _SE_MAKEFRAME;
    }
    
    // Measuring the time in between the search.
    gettimeofday(& start_time, NULL);
        bool search_response = BTree_Search(_Key, input_stream, output_stream, & frame, & result -> target);
    gettimeofday(& end_time, NULL);

    // Interpreting the time the search took in [seconds].
    result -> measures.time_span = time_diff_sec(start_time, end_time);

    freeFrame(frame);

    fclose(input_stream);
    fclose(output_stream);

    if (! search_response)
    {
        printf("Key not found!\n");
        return SEARCH_FAILURE;
    }
    return SEARCH_SUCCESS;
}

/*  The B*Tree search-engine. */
static SEARCH_RESPONSE
__BSTAR(const key_t _Key, search_result * result, const char * _InputFilename, const char * _OutputFilename)
{
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    // Opening both input and output file-streams 
    // for the construction.

    REG_STREAM * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL)
        return _SE_REGDATAFILE;
    
    BSTAR_STREAM * output_stream = fopen(_OutputFilename, "w+b");
    if (output_stream == NULL) {
        fclose(input_stream);
        return _SE_BSTARFILE;
    }

    // Measuring the time before construction,
    gettimeofday(& start_time, NULL);
    
    if (! BSTree_Build(input_stream, output_stream))
    {
        fclose(input_stream);
        fclose(output_stream);
        return _SE_BSTARBUILD;
    }
    
    // measuring the time after it.
	gettimeofday(& end_time, NULL);

    // Interpreting the time the contruction took in [seconds].
	result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
    
    // Re-opening the output B-Tree stream in the read mode.
    fclose(output_stream);
    output_stream = fopen(_OutputFilename, "rb");
    if (output_stream == NULL) {
        fclose(input_stream);
        return _SE_BSTARFILE;
    }
    
    /*  The frame for search. As there will only be requested one 
        search operation, the role of the frame is not fundamentally
        important and its advantage is not used. */
    frame_t frame = { 0 }; 
    if (! makeFrame(& frame, sizeof(bstar_node))) {
        fclose(input_stream);
        fclose(output_stream);
        return _SE_MAKEFRAME;
    }
    
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
        return SEARCH_FAILURE;
    }
    return SEARCH_SUCCESS;
}

/*  The "Indexed Sequential Search" search-engine. */
static SEARCH_RESPONSE
__ISS(const key_t _Key, search_result * result, const SITUATION situation, const uint64_t reg_qtt,
    const char * _InputFilename)
{
    // As ISS only works when there are order, a validation is done for the disordered case.
    if (situation == DISORDERED)
        return _SE_UNORDERED_ISS;

    struct timeval start_time, end_time;
    
    FILE * input_stream = fopen(_InputFilename, "rb");
    if (input_stream == NULL)
        return _SE_REGDATAFILE;
    
    IndexTable index_table = { 0 }; 

    gettimeofday(& start_time, NULL);
    if (! buildIndexTable(& index_table, reg_qtt, input_stream))
        return _SE_INDEXTABLE;
    gettimeofday(& end_time, NULL);

    result -> measures.construction_time = ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));

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
        return SEARCH_FAILURE;
    }
    return SEARCH_SUCCESS;
}

/*  The actual redirector to the search-engines. 
    The error logging happens here. 
    Returns success in the searching without error occurrences. */
static bool
_RedirectSearch(SEARCHING_METHOD method, SITUATION situation, key_t key, uint64_t qtt, search_result * result)
{
    /*  Tracks the search state for error, in case of, or success. */
    SEARCH_RESPONSE search_response = SEARCH_FAILURE;

    
    if (method == INDEXED_SEQUENTIAL_SEARCH) {
        printf("[%s]: ISS\n", __func__);
        search_response = __ISS(key, result, situation, qtt, INPUT_DATAFILENAME);

    } else if (method == EXTERNAL_BINARY_SEARCH) {
        printf("[%s]: EBS\n", __func__);
        search_response = __EBST(key, result, situation, qtt, INPUT_DATAFILENAME, OUTPUT_EBST_FILENAME, OUTPUT_ERBT_FILENAME);
    }
    
    else if (method == BTREE_SEARCH) {
        printf("[%s]: BTREE\n", __func__);
        search_response = __BTREE(key, result, INPUT_DATAFILENAME, OUTPUT_BTREE_FILENAME);
    }
    
    else if (method == BSTAR_SEARCH) {
        printf("[%s]: SBTAR\n", __func__);
        search_response = __BSTAR(key, result, INPUT_DATAFILENAME, OUTPUT_BSTAR_FILENAME);
    }
    
    /*  Error representation */
    if ((search_response != SEARCH_SUCCESS) && (search_response != SEARCH_FAILURE))
    {
        /*
            TODO: Error debug system.
        */
        printf("erro erro cmrd, %d\n", search_response);

        return false;
    }

    result -> success = search_response == SEARCH_SUCCESS;
    return true;
}


/* "pesquisa <método> <quantidade> <situação> <chave>" */
int main(int argc, char ** argsv)
{
#ifdef hebert
    printf("hebert: %u\n", (unsigned int) hebert);
#else
    printf("hebert não definito\n");
#endif


    SEARCHING_METHOD method = 0;
    SITUATION situation = 0;
    key_t key = 0;
    uint64_t reg_qtt = 0;
    search_result result = { 0 };
    
    if (! _ParseArgs(argc, argsv, & method, & situation, & key, & reg_qtt))
        return -1;
    
    if (! _RedirectSearch(method, situation, key, reg_qtt, & result))
    {
        // in case an error happened, propagetes it on output.
        return -2;
    }
    
    /* Representing the computation. */
    PrintSearchResults(& result);

    PrintCounter();

    return EXIT_SUCCESS;
}
