

#include "external-search.h"


#ifdef TRANSPARENT_COUNTER
struct __transparent_counter_t transparent_counter = { 0 };
#endif


FILE * andrd_stream = NULL;


static void
_PrintRegistries(const registry_pointer * reg_ptr, const size_t qtd)
{
    fputc('<', andrd_stream);
    if (!qtd)
    {
        fputc('>', andrd_stream);
        return;
    }

    for (size_t i = 0; i < qtd - 1; i++) {
        fprintf(andrd_stream, "%u, ", (unsigned int) reg_ptr[i].key);
    }
    fprintf(andrd_stream, "%u>", (unsigned int) reg_ptr[qtd - 1].key);
}

static void
_PrintChildren(const uint32_t * children, const size_t qtd)
{
    fputc('<', andrd_stream);
    const size_t qtd_m1 = qtd - 1;

    for (size_t i = 0; i < qtd_m1; i++) {
        fprintf(andrd_stream, "%u, ", (unsigned int) children[i]);
    }
    fprintf(andrd_stream, " %u>", (unsigned int) children[qtd_m1]);
}

static void
PrintBNode(const b_node * _Node)
{
    fprintf(andrd_stream, "[q: %u, leaf: %d, registries key: ", (unsigned) _Node->item_count,
        (int) _Node->is_leaf);

    _PrintRegistries(_Node->reg_ptr, _Node->item_count);

    if (!_Node->is_leaf) {
        fprintf(andrd_stream, ", children: ");
        _PrintChildren(_Node->children_ptr, _Node->item_count + 1);
    }
    fprintf(andrd_stream, "]\n");
}

static void
PrintBStream(B_STREAM * _OutputStream)
{
    fseek(_OutputStream, 0, SEEK_SET);

    b_node buffer = { 0 }; size_t iterator = 0;
    while (read_bnode(_OutputStream, iterator, &buffer))
    {
        printf("%u\t", (unsigned int) iterator++);
        PrintBNode(&buffer);
    }
}


#define __OUTPUT_FILENAME   "andrd.btree"


static bool 
__btree_build(int argc, char ** argsv, FILE * _OutputStream) {
    B_Builder b_builder = { 0 };
    b_builder.file_stream = _OutputStream;
    b_builder.root.is_leaf = true;
    b_builder.nodes_qtt = 1;

    if (! frame_make(& b_builder.frame, PAGES_PER_FRAME, sizeof(b_node), B_PAGE))
        return false;

    bool insert_failure = false;
    registry_pointer reg_buffer = { 0 };
    for (int iterator = 1; iterator < argc; iterator ++)
    {
        reg_buffer.key = atoi(argsv[iterator]);

        if (! BTree_insert(& reg_buffer, & b_builder)) {
            insert_failure = true;
            break;
        }

#if IMPL_LOGGING
        PrintBStream(b_builder.file_stream);
#endif
    }

    freeFrame(& b_builder.frame);

    return ! insert_failure;
}


int main(int argc, char ** argsv) {
    andrd_stream = stdout;

#if IMPL_LOGGING
    InitializeLogging();
    andrd_stream = debug_stream;
#endif


    B_STREAM * output_stream = (B_STREAM *) fopen(__OUTPUT_FILENAME, "w+b");
    if (output_stream == NULL)
    {
#if IMPL_LOGGING
        FinalizeLogging();
#endif
        return 1;
    }

    if (! __btree_build(argc, argsv, output_stream))
    {
        fclose(output_stream);

#if IMPL_LOGGING
        FinalizeLogging();
#endif
        return 2;
    }

    PrintBStream(output_stream);
    fclose(output_stream);
	
#if IMPL_LOGGING
    FinalizeLogging();
#endif

    return 0;
}
