
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"



static finline bool SeekReadRegistry(FILE * __InputStream, size_t pos, registry_t * __ReturnRegistry) {
    if (! in_range(0, 99, pos)) {
        printf("ERROR!\n");
        return false;
    }
    fseek(__InputStream, pos * sizeof(registry_t), SEEK_SET);
    fread(__ReturnRegistry, sizeof(registry_t), 1, __InputStream);
    return (ferror(__InputStream) == 0);
}


static finline bool WriteNode(FILE * __OutputStream, ebst_node * __Node) {
    fwrite(__Node, sizeof(ebst_node), 1, __OutputStream);
    return (ferror(__OutputStream) == 0);
}

static inline bool ReadAppendNode(
    FILE * __InputStream, FILE * __OutputStream, 
    size_t root_pos, int left_pos, int right_pos) {
    
    ebst_node dummy;

    // Retrieving the target registry.
    SeekReadRegistry(__InputStream, root_pos, & dummy.root_item);

    // Writing it.
    dummy.left = left_pos;
    dummy.right = right_pos;
    return WriteNode(__OutputStream, & dummy); // fwrite(& dummy, sizeof(node_t), 1, __OutputStream);
}


static void PrintNode(const ebst_node * __Node) {
    printf("<%d, (%d, %d)>", __Node -> root_item.key, __Node -> left, __Node -> right);
}


static void MiddlepointRootTraversalTranscript(
    FILE * input_stream, FILE * output_stream
) {
    // initialize stack with (0, length - 1)

    long l = 0, r = 99, m;
    stack_item split_buffer = {0, 99};

    Stack split_stack = newStack();
    stackPush(& split_stack, & split_buffer);

    int left_key, right_key;
    ebst_node buffer_node;

    size_t iterator = 0;

    while (stackPop(& split_stack, & split_buffer))
    {
        printStack(& split_stack);

        l = split_buffer.left; r = split_buffer.right;
        m = midpoint(l, r);
        left_key = - 1; right_key = - 1;

        // printf("stack-size: %d, (l, r) = (%d, %d)\n", (int) split_stack.size, (int) l, (int) r);

#define SIZEOFRIGHT(l, r)   ((r - l) - ((r - l) >> 1))

        if (l <= (m - 1))
        {
            split_buffer.left = l;
            split_buffer.right = m - 1;

            stackPush(& split_stack, & split_buffer);
            
            // left_key = midpoint(l, m - 1);   // Actual position in the original file.

            // Position in the current file.
            left_key = iterator + 1 + SIZEOFRIGHT(l, r);
        }

        if ((m + 1) <= r) 
        {
            split_buffer.left = m + 1;
            split_buffer.right = r;

            stackPush(& split_stack, & split_buffer);

            // right_key = midpoint(m + 1, r);
            right_key = iterator + 1;
        }

        SeekReadRegistry(input_stream, m, & buffer_node.root_item);
        buffer_node.left = left_key;
        buffer_node.right = right_key;

        WriteNode(output_stream, & buffer_node);
        
        printf("[W]:\t");
        PrintNode(& buffer_node);
        printf("\n\n");

        iterator++;
    } 

}


static bool AssembleEBST_ordered(  // Returns success.
    filepath orderedInputFilename, filepath outputFilename
) {
    FILE * input_stream = fopen(orderedInputFilename, "rb");
    if (input_stream == NULL) {
        printf("AssembleEBST_ordered. Failed to open input-stream.\n");
        return false;
    }

    FILE * output_stream = fopen(outputFilename, "wb");
    if (output_stream == NULL) {
        printf("AssembleEBST_ordered. Failed to open output-stream.\n");
        return false;
    }

    MiddlepointRootTraversalTranscript(input_stream, output_stream);

    fclose(input_stream);
    fclose(output_stream);
    return true;
}


int main(int argc, char ** argsv) 
{
    // no arg. parsing for instance...

    if (! AssembleEBST_ordered(ORDFILENAME, "ord-data.ebst")) 
    {
        fprintf(stderr, "ERROR!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
