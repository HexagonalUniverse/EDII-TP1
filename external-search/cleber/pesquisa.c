/*  <pesquisa.c>
    
    Main driver for searching. */


#include <external-search.h>
#include <sys/time.h>   // For <gettimeofday>.
#include <string.h>     // for strcmp


/*  Profiling and output
    ==================== */

#define PROFILE_FILENAME        "temp/cache/_last_pesquisa_run.log"     // * Relative to <external-search/>

/*  Returns the time difference, in seconds, between two time measures. */
finline static double time_diff_sec(struct timeval start_time, struct timeval end_time) {
    return ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
}


static bool log_search_results(const search_result * sr) {
    FILE * log_file = fopen(PROFILE_FILENAME, "w");
    if (log_file == NULL)
        return false;

    // assumes that, once open, the writing won't raise problems

    fprintf(log_file, "%d %lld %.4s %.4s\n",
        sr->target.key, sr->target.data_1, sr->target.data_2, sr->target.data_3); // registry-data.

    fprintf(log_file, "%lf %lf\n", sr->measures.construction_time, sr->measures.search_time);

    #if TRANSPARENT_COUNTER
        struct tuple_u64 { uint64_t x, y; } comparisons = { 0 };
        for (size_t i = 0; i < 6; i ++) {
            comparisons = ((struct tuple_u64 *) (&transparent_counter))[i];
            fprintf(log_file, "%llu %llu ", (unsigned long long) comparisons.x, (unsigned long long) comparisons.y);
        }
        fprintf(log_file, "\n");
    #endif
    fclose(log_file);
    return true;
}

/*  Outputs on stdout the search result. 
    Logs it in case the search was successfull. */
static void output_search_results(const search_result * sr)
{
    // stdout printing

    for (uint8_t i = 0; i < 20; i++)
        putchar('-');
    putchar('\n');

    printf("Searching for key <%d>:\n", sr->target.key);

    printf("\t> Data-structures construction-time: %lf [s]\n", sr->measures.construction_time);
    printf("\t> Searching time: %lf [s]\n", sr->measures.search_time);

    if (sr->success) {
        printf("\t> The registry:\n");
        printf("\t\t| data1 = %lld\n", sr->target.data_1);
        printf("\t\t| data2 = %.4s\n", sr->target.data_2);
        printf("\t\t| data3 = %.4s\n", sr->target.data_3);
    }
    else {
        printf("\t- Failure\n");
    }

    // logging

    if ((sr->success) && (! log_search_results(sr)))
        _ContextWarningMsgf("(logging) ", "Couldn't log search results.\n");
}


/*  Program parameters
    ================== */

/*  Identifier for each of the searching methods. */
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

/*  Tracks all input parameters to the application. */
struct application_parameters {
    SEARCHING_METHOD method;    // Specifies the searching method.
    SITUATION situation;        // Specifies the registries file order situation.
    key_t key;                  // Specifies the key that will be searched.
    uint64_t reg_qtt;           // Specifies how many registries is in the data file.
    
    bool display_help;          // Specifies whether the enter in help mode.
};


/*  Searching Handles
    ================= */

/*  By the program's specification, all data-structures will be attempted being
    constructed at the searching handles.     */

/*  Handles the EBST (by RBT) searching. */
static SEARCH_RESPONSE 
__ERBT(const struct application_parameters * parameters, search_result * result, REG_STREAM * input_stream, const char * _ERBTFilename)
{
    // Time-measure variables.
    struct timeval start_time, end_time;

    // File over which the ERBT will be constructed at.
    ERBT_STREAM * output_stream = (ERBT_STREAM *) fopen(_ERBTFilename, "w+b");
    if (output_stream == NULL)
        return _SE_EBST_FILE;

    {   // Building the ERBT.

        gettimeofday(&start_time, NULL);
        if (! ERBT_Build(input_stream, (ERBT_STREAM *) output_stream)) {
            fclose(output_stream);
            return _SE_ERBTBUILD;
        }
        gettimeofday(&end_time, NULL);
        result->measures.construction_time = time_diff_sec(start_time, end_time);
    }

    // Re-opening the erbt file in read-mode.
    fclose(output_stream);
    output_stream = fopen(_ERBTFilename, "rb");
    if (output_stream == NULL)
        return _SE_EBST_FILE;

    Frame frame = { 0 };
    if (! frame_make(&frame, ERBT_PAGE))
        return _SE_MAKEFRAME;

    {   // Searching for the key.

        gettimeofday(&start_time, NULL);
        result -> success = ERBT_Search((ERBT_STREAM *) output_stream, input_stream, &frame, parameters -> key, &result -> target);
        gettimeofday(&end_time, NULL);
        result -> measures.search_time = time_diff_sec(start_time, end_time);
    }

    frame_free(&frame);
    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  Handles the EBST (by MRT) searching. */
static SEARCH_RESPONSE
__EBST(const struct application_parameters * parameters, search_result * result, REG_STREAM * input_stream, const char * _EBSTFilename)
{
    // Time-measure variables.
    struct timeval start_time, end_time;

    EBST_STREAM * output_stream = NULL;     // File over which the EBST will be constructed at.
    Frame frame = { 0 };                  // Registries frame used in the MRT build.

    {   // Initialization.

        output_stream = (EBST_STREAM *) fopen(_EBSTFilename, "w+b");
        if (output_stream == NULL)
            return _SE_EBST_FILE;

        if (! frame_make(&frame, REG_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }
    }

    {   // Building the data-structure.

        gettimeofday(&start_time, NULL);
        if (! EBST_MRT_Build(input_stream, output_stream, &frame, parameters->situation == ASCENDING_ORDER, parameters->reg_qtt)) {
            fclose(output_stream);
            frame_free(&frame);

            return _SE_EBSTMRTBUILD;
        }
        gettimeofday(&end_time, NULL);

        // Registering the construction time.
        result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    frame_free(&frame);

    // Re-opening the data-structure file in read-mode.
    fclose(output_stream);
    output_stream = fopen(_EBSTFilename, "rb");
    if (output_stream == NULL)
        return _SE_EBST_FILE;

    {   // Searching for the key.
     
        gettimeofday(&start_time, NULL);
        result -> success = EBST_Search((EBST_STREAM *) output_stream, input_stream, parameters -> key, &result -> target);
        gettimeofday(&end_time, NULL);

        // Registering the searching time.
        result -> measures.search_time = time_diff_sec(start_time, end_time);
    }

    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  Handles the B tree searching. */
static SEARCH_RESPONSE
__BTREE(const struct application_parameters * parameters, search_result * result,
    REG_STREAM * input_stream, const char * _OutputFilename) 
{   
    // Time-measure variables.
	struct timeval start_time, end_time;

    // File over which the B tree data-structure will be constructed at.
    B_STREAM * output_stream = (B_STREAM *) fopen(_OutputFilename, "w+b");
    if (output_stream == NULL)
        return _SE_BFILE;

    {   // Building the data-structure.

        gettimeofday(&start_time, NULL);
        if (! BTree_Build(input_stream, output_stream))
        {
            fclose(output_stream);
            return _SE_BBUILD;
        }
	    gettimeofday(&end_time, NULL);
	    result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    // Re-opening the B tree file in read mode.
    fclose(output_stream);
    output_stream = (B_STREAM *) fopen(_OutputFilename, "rb");
    if (output_stream == NULL)
        return _SE_BFILE;
    
    {   // Searching for the key.

        /*  The frame for search. As there will only be requested one
        search operation, the role of the frame is not fundamentally
        important and its advantage is not used. */
        Frame frame = { 0 };
        if (!  frame_make(&frame, B_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }

        gettimeofday(&start_time, NULL);
        result -> success = BTree_Search(parameters -> key, input_stream, output_stream, &frame, &result -> target);
        gettimeofday(&end_time, NULL);
        result -> measures.search_time = time_diff_sec(start_time, end_time);

        frame_free(&frame);
    }

    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  Handles the B* tree searching. */
static SEARCH_RESPONSE
__BSTAR(const struct application_parameters * parameters, search_result * result, REG_STREAM * input_stream, const char * _OutputFilename)
{
    // Time-measure variables.
	struct timeval start_time, end_time;
    
    // File over which the B* tree data-structure will be constructed at.
    BSTAR_STREAM * output_stream = fopen(_OutputFilename, "w+b");
    if (output_stream == NULL)
        return _SE_BSTARFILE;

    {   // Building the data-structure.

        gettimeofday(&start_time, NULL);
        if (! BSTree_Build(input_stream, output_stream))
        {
            fclose(output_stream);
            return _SE_BSTARBUILD;
        }
	    gettimeofday(&end_time, NULL);
	    result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    // Re-opening the output B* tree stream in the read mode.
    fclose(output_stream);
    output_stream = fopen(_OutputFilename, "rb");
    if (output_stream == NULL)
        return _SE_BSTARFILE;
    
    {   // Searching for the key

        /*  The frame for search. As there will only be requested one 
            search operation, the role of the frame is not fundamentally
            important and its advantage is not used. */
        Frame frame = { 0 }; 
        if (!  frame_make(&frame, BSTAR_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }
        
        gettimeofday(&start_time, NULL);
        result -> success = BSTree_Search(parameters -> key, input_stream, output_stream, &frame, &result -> target);
        gettimeofday(&end_time, NULL);
        result -> measures.search_time = time_diff_sec(start_time, end_time);

        frame_free(&frame);
    }
    
    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  Indexed Sequential Search search-engine. */
static SEARCH_RESPONSE
__ISS(const struct application_parameters * parameters, search_result * result, REG_STREAM * input_stream)
{
    // As ISS only works when there are order, a validation is done for the disordered case.
    if (parameters -> situation == DISORDERED)
        return _SE_UNORDERED_ISS;

    // Time-measure variables.
    struct timeval start_time, end_time;
    
    IndexTable index_table = { 0 };

    {   // Building indexed-table.

        gettimeofday(&start_time, NULL);
        if (! buildIndexTable(&index_table, parameters -> reg_qtt, input_stream))
            return _SE_INDEXTABLE;
        gettimeofday(&end_time, NULL);
        result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    Frame frame = { 0 }; 
    if (! frame_make(&frame, REG_PAGE)) {
        deallocateIndexTable(&index_table);
        return _SE_MAKEFRAME;
    }

    {   // Searching for the key.

        gettimeofday(&start_time, NULL);
        result -> success = indexedSequencialSearch(parameters -> key, input_stream, &index_table, &frame, result, (parameters -> situation == ASCENDING_ORDER) ? true : false);
        gettimeofday(&end_time, NULL);
        result -> measures.search_time = time_diff_sec(start_time, end_time);
    }

    deallocateIndexTable(&index_table);
    frame_free(&frame);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  The actual redirector to the searching handles. 
    The error logging happens here. 
    Returns success in the (data-building) and searching without error occurrences. */
static bool
_RedirectSearch(const struct application_parameters * parameters, search_result * result)
{
    REG_STREAM * input_stream = (REG_STREAM *) fopen(INPUT_DATAFILENAME, "rb");
    if (input_stream == NULL) { // erro fudido *-*
        _ContextErrorMsgf("(redirect search) ", "Couldn't open registry file <%s>.\n", INPUT_DATAFILENAME);
        return false;
    }
    
    /*  Tracks the search state for error, in case of, or success. */
    SEARCH_RESPONSE search_response = SEARCH_FAILURE;
    
    switch (parameters -> method) {
    case INDEXED_SEQUENTIAL_SEARCH:
        search_response = __ISS(parameters, result, input_stream);
        break;

    case EXTERNAL_BINARY_SEARCH:
        /*  For EBS, the decision between a ebst (by mrt) 
            and erbt is hereby made. */
        switch (parameters -> situation) {
        case DISORDERED:
            search_response = __ERBT(parameters, result, input_stream, OUTPUT_ERBT_FILENAME);
            break;
        
        default:    // As such, reduced algorithmic complexity is taken in advantage in the ordered case.
            search_response = __EBST(parameters, result, input_stream, OUTPUT_EBST_FILENAME);
        } break;

    case BTREE_SEARCH:
        search_response = __BTREE(parameters, result, input_stream, OUTPUT_BTREE_FILENAME);
        break;

    case BSTAR_SEARCH:
        search_response = __BSTAR(parameters, result, input_stream, OUTPUT_BSTAR_FILENAME);
        break;

    default:
        /*  This should be unreacheable. */
        break;
    }
    
    fclose(input_stream);

    /*  Search handles error representation. */
    if ((search_response != SEARCH_SUCCESS) && (search_response != SEARCH_FAILURE))
    {
        /*
            NOT YET IMPLEMENTED.
        */
        fprintf(stderr, "erro erro cmrd, %d\n", search_response);

        return false;
    }

    result -> success = search_response == SEARCH_SUCCESS;  // * redundant

    // Finished-up execution without errors...
    return true;
}

/*  Parses the program's arguments. Returns whether the combination were valid. */
static bool
_ParseArgs(int argc, char ** argsv, struct application_parameters * parameters)
{
    // * mostly a naive implementation...

    if ((argc == 2) && (! strcmp(argsv[1], "-h")))
    {
        // TODO: "-h" menu.

        parameters -> display_help = true;
        return true;
    }

    // Validation of the searching format "./'searchfile.exe' <method> <quantity> <situation> <key> <-P>"
    if (! in_range(5, 6, argc)) {
        _ContextErrorMsgf("parsing error: ", "Incorrect number of arguments: given " _ES_FG_RED() "%d" _AEC_RESET ", expected 5.\n", argc);
        printf(">>\t"); _TracebackErrorArg(argc, argsv, 1); putchar('\n');
        fflush(stdout);
        return false;
    }
    
    if (argc == 6) {
        if (strcmp("-p", argsv[5])) {
            _ContextErrorMsgf("parsing error: ", "Expected \"-p\" but received \"" _ES_FG_RED() "%s" _AEC_RESET "\".\n", argsv[5]);
            printf(">>\t"); _TracebackErrorArg(argc, argsv, 5); putchar('\n');
            return false;
        }
    }

    // Getting the method number from terminal
    parameters -> method = atoi(argsv[1]);
    
    // Validating the method number
    if (! in_range(0, 3, parameters -> method)) {
        _ContextErrorMsgf("parsing error: ", "The specified method is incorrect. Passed " _ES_FG_RED() "%d" _AEC_RESET ", but actually expected"
            " one in the range 0 to 3, inclusive.\n", parameters -> method);
        printf(">>\t"); _TracebackErrorArg(argc, argsv, 1); putchar('\n');
        return false;
    }
    
    int64_t x = atoll(argsv[2]);
    if (x < 0) {
        _ContextErrorMsgf("parsing error: ", "Trouble interpreting the passed quantity. \"%s\" is interpreted as "
            _ES_FG_RED() "%" PRIi64 _AEC_RESET ".\n", argsv[2], x);
        printf(">>\t"); _TracebackErrorArg(argc, argsv, 2); putchar('\n');
        return false;
    }
    else if (x > 2147483647LL) {    // Warning in case x > INT_MAX.
        _ContextWarningMsgf("parsing warning: ", "Have in mind that the passed quantity (\"%s\" -> %" PRIi64 ") is way too large.\n",
            argsv[2], x);
    }
    parameters -> reg_qtt = x;

    // Getting the situation of the file from terminal
    parameters-> situation = atoi(argsv[3]);
    
    // Validating the situation number
    if (! in_range(1, 3, parameters -> situation)) {
        _ContextErrorMsgf("parsing error: ", "The specified file order situation is incorrect. " 
            "Passed " _ES_FG_RED() "%d" _AEC_RESET ", but actually expected one in the range 1 to 3, inclusive.\n", parameters -> situation);
        printf(">>\t"); _TracebackErrorArg(argc, argsv, 3); putchar('\n');
        return false;
    }

    // Getting the wanted key from terminal
    parameters -> key = atoi(argsv[4]);

    // TODO: Verify if the parameters are numerals before converting them....

    return true;
}


/*  SPECS.
    
    $ pesquisa method register-quantity-in-file file-order-situation searching-key <-P> 
        <-in=registries-input-data-file>

    return-code:
        0:      successful flux;
        -1:     parsing error;
        -2:     searching error;
        -3:     logging initialization error.
*/
int main(int argc, char ** argsv)
{
    printf("B node size: %llu\n", sizeof(b_node));

    #if IMPL_LOGGING
        if (! InitializeLogging())
        {
            // se precisar mostrar isso algum dia é triste *-*
            printf("Failed initializing the logging system.\n");
            return -3;
        }
    #endif // IMPL_LOGGING

    /*  Parsing program parameters. */
    struct application_parameters parameters = { 0 };
    if (! _ParseArgs(argc, argsv, &parameters))
    {
        #if IMPL_LOGGING
            FinalizeLogging();
        #endif

        return -1;
    }
    
    if (parameters.display_help)
    {
        // print help on stdout for "-h"...
        printf("help yeei\n");

        #if IMPL_LOGGING
            FinalizeLogging();
        #endif
        return 0;
    }

    search_result result = { 0 };
    if (! _RedirectSearch(&parameters, &result))
    {
        #if IMPL_LOGGING
            FinalizeLogging();
        #endif
        // in case an error happened, propagetes it on output.
        return -2;
    }
    
    #if TRANSPARENT_COUNTER
        transparent_counter_print();
    #endif
    
    /* Representing the computation. */
    output_search_results(&result);

    #if IMPL_LOGGING
        FinalizeLogging();
    #endif

    return EXIT_SUCCESS;
}
