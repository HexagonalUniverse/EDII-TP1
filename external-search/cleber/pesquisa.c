/*  <pesquisa.c>
    
    Main driver for searching. */


#include <external-search.h>
#include <sys/time.h>   // For <gettimeofday>.
#include <string.h>     // for strcmp


finline static double 
time_diff_sec(struct timeval start_time, struct timeval end_time) {
    return ((double) (end_time.tv_usec - start_time.tv_usec) / 1e6) + ((double) (end_time.tv_sec - start_time.tv_sec));
}


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
        printf("\t\t| data1 = %ld\n", (long int) _Sr->target.data_1);
        printf("\t\t| data2 = %.4s\n", _Sr->target.data_2);
        printf("\t\t| data3 = %.4s\n", _Sr->target.data_3);
    }
    else {
        printf("\t- Failure\n");
    }
}


/*  */
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

        gettimeofday(& start_time, NULL);
        if (! ERBT_Build(input_stream, (ERBT_STREAM *) output_stream)) {
            fclose(output_stream);
            return _SE_ERBTBUILD;
        }
        gettimeofday(& end_time, NULL);
        result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    // Re-opening the erbt file in read-mode.
    fclose(output_stream);
    output_stream = fopen(_ERBTFilename, "rb");
    if (output_stream == NULL)
        return _SE_EBST_FILE;

    {   // Searching for the key.

        gettimeofday(& start_time, NULL);
        result -> success = ERBT_Search((ERBT_STREAM *) output_stream, input_stream, parameters -> key, & result -> target);
        gettimeofday(& end_time, NULL);
        result -> measures.time_span = time_diff_sec(start_time, end_time);
    }

    fclose(output_stream);
    return SEARCH_FAILURE;
}

/*  The EBST (by MRT) search-engine. */
static SEARCH_RESPONSE
__EBST(const struct application_parameters * parameters, search_result * result, REG_STREAM * input_stream, const char * _EBSTFilename)
{
    // Time-measure variables.
    struct timeval start_time, end_time;

    EBST_STREAM * output_stream = NULL;     // File over which the EBST will be constructed at.
    frame_t frame = { 0 };                  // Registries frame used in the MRT build.

    {   // Initialization.

        output_stream = (EBST_STREAM *) fopen(_EBSTFilename, "w+b");
        if (output_stream == NULL)
            return _SE_EBST_FILE;

        if (! frame_make(& frame, PAGES_PER_FRAME, sizeof(regpage_t), REG_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }
    }

    {   // Building the data-structure.

        gettimeofday(& start_time, NULL);
        if (! EBST_MRT_Build(input_stream, output_stream, & frame, parameters->situation == ASCENDING_ORDER, parameters->reg_qtt)) {
            fclose(output_stream);
            freeFrame(& frame);

            return _SE_EBSTMRTBUILD;
        }
        gettimeofday(& end_time, NULL);

        // Registering the construction time.
        result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    freeFrame(& frame);

    // Re-opening the data-structure file in read-mode.
    fclose(output_stream);
    output_stream = fopen(_EBSTFilename, "rb");
    if (output_stream == NULL)
        return _SE_EBST_FILE;

    {   // Searching for the key.
     
        gettimeofday(& start_time, NULL);
        result -> success = EBST_Search((EBST_STREAM *) output_stream, input_stream, parameters -> key, & result -> target);
        gettimeofday(& end_time, NULL);

        // Registering the searching time.
        result -> measures.time_span = time_diff_sec(start_time, end_time);
    }

    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  Handles the B tree searching. 
    First, the data-structure is attempted being constructed. */
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

        gettimeofday(& start_time, NULL);
        if (! BTree_Build(input_stream, output_stream))
        {
            fclose(output_stream);
            return _SE_BBUILD;
        }
	    gettimeofday(& end_time, NULL);
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
        frame_t frame = { 0 };
        if (!  frame_make(& frame, PAGES_PER_FRAME, sizeof(b_node), B_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }

        gettimeofday(& start_time, NULL);
        result -> success = BTree_Search(parameters -> key, input_stream, output_stream, & frame, & result -> target);
        gettimeofday(& end_time, NULL);
        result -> measures.time_span = time_diff_sec(start_time, end_time);

        freeFrame(& frame);
    }

    fclose(output_stream);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  The B* tree search-engine. */
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

        gettimeofday(& start_time, NULL);
        if (! BSTree_Build(input_stream, output_stream))
        {
            fclose(output_stream);
            return _SE_BSTARBUILD;
        }
	    gettimeofday(& end_time, NULL);
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
        frame_t frame = { 0 }; 
        if (!  frame_make(& frame, PAGES_PER_FRAME, sizeof(bstar_node), BSTAR_PAGE)) {
            fclose(output_stream);
            return _SE_MAKEFRAME;
        }
        
        gettimeofday(& start_time, NULL);
        result -> success = BSTree_Search(parameters -> key, input_stream, output_stream, & frame, & result -> target);
        gettimeofday(& end_time, NULL);
        result -> measures.time_span = time_diff_sec(start_time, end_time);

        freeFrame(& frame);
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

        gettimeofday(& start_time, NULL);
        if (! buildIndexTable(& index_table, parameters -> reg_qtt, input_stream))
            return _SE_INDEXTABLE;
        gettimeofday(& end_time, NULL);
        result -> measures.construction_time = time_diff_sec(start_time, end_time);
    }

    frame_t frame = { 0 }; 
    if (! frame_make(& frame, PAGES_PER_FRAME, sizeof(regpage_t), REG_PAGE)) {
        deallocateIndexTable(& index_table);
        return _SE_MAKEFRAME;
    }

    {   // Searching for the key.

        gettimeofday(& start_time, NULL);
        result -> success = indexedSequencialSearch(parameters -> key, input_stream, & index_table, & frame, result, (parameters -> situation == ASCENDING_ORDER) ? true : false);
        gettimeofday(& end_time, NULL);
        result -> measures.time_span = time_diff_sec(start_time, end_time);
    }

    deallocateIndexTable(& index_table);
    freeFrame(& frame);
    return result -> success ? SEARCH_SUCCESS : SEARCH_FAILURE;
}

/*  The actual redirector to the search-engines. 
    The error logging happens here. 
    Returns success in the (data-building) and searching without error occurrences. */
static bool
_RedirectSearch(const struct application_parameters * parameters, search_result * result)
{
    REG_STREAM * input_stream = (REG_STREAM *) fopen(INPUT_DATAFILENAME, "rb");
    if (input_stream == NULL) {
        /*
            erro fudido
        */

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
            and erbt is hereby done. */
        switch (parameters -> situation) {
        case DISORDERED:
            search_response = __ERBT(parameters, result, input_stream, OUTPUT_ERBT_FILENAME);
            break;
        
        // Reduced algorithmic complexity is taken in advantage in the ordered case.
        default:
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

    /*  Search-engines error representation. */
    if ((search_response != SEARCH_SUCCESS) && (search_response != SEARCH_FAILURE))
    {
        /*
            NOT YET IMPLEMENTED.
        */
        printf("erro erro cmrd, %d\n", search_response);

        return false;
    }

    result -> success = search_response == SEARCH_SUCCESS;  // * redundant

    // Finished-up execution without errors...
    return true;
}

/*  Parses the arguments for the main-program. */
static bool
_ParseArgs(int argc, char ** argsv, struct application_parameters * parameters)
{
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
    else if (x > 2147483647LL) {
        _ContextWarningMsgf("parsing warning: ", "The passed quantity (\"%s\" -> %" PRIi64 ") is way too large.\n",
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

    // TODO: Verify if the parameters are numerals....

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
    if (! _ParseArgs(argc, argsv, & parameters))
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
    if (! _RedirectSearch(& parameters, & result))
    {
        #if IMPL_LOGGING
            FinalizeLogging();
        #endif
        // in case an error happened, propagetes it on output.
        return -2;
    }
    
    /* Representing the computation. */
    PrintSearchResults(& result);

    #if TRANSPARENT_COUNTER
        transparent_counter_print();
    #endif
    

    #if IMPL_LOGGING
        FinalizeLogging();
    #endif

    return EXIT_SUCCESS;
}
