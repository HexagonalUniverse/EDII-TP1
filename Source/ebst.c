/*
	TODO's:
	
	. Transcript the dependencies of the searching-method to the lib.

	. Implement the method base in the current conventions.
*/


#include "common.h"
#include "ebst.h"
#include "searching.h"
#include <stdlib.h>
#include <stdbool.h>


// IO operations
// =============

// Seeks and reads a single registry in the input-stream. 
// Returns if the input-stream is ok after operating in it.
// Wrapper around the common sequence seek, read, check for error...
static finline bool 
seekReadRegistry(FILE * __InputStream, size_t pos, registry_t * _ReturnRegistry) 
{
	fseek(__InputStream, pos * sizeof(registry_t), SEEK_SET);
	fread(_ReturnRegistry, sizeof(registry_t), 1, __InputStream);
	return (ferror(__InputStream) == 0);
}

// Writes a single node into the output-stream in the last position.
// Returns if the output-stream is ok after operating in it.
// Wrapper around fwrite for the node.
static finline bool
writeNode(FILE* __OutputStream, ebst_node* _Node) 
{
	fwrite(_Node, sizeof(ebst_node), 1, __OutputStream);
	return (ferror(__OutputStream) == 0);
}


// Stack DS for MRT
// ================

//
typedef struct {
	int left, right;
} stack_item;

//
typedef struct stack_node_t {
	stack_item item;
	struct stack_node_t * next;
} stack_node;

//
typedef struct {
	size_t size;
	stack_node * head;
} ebstStack;


static stack_node * new_stackNode(const stack_item * _Item) {
	stack_node * sn = (stack_node *) malloc(sizeof(stack_node));
	if (sn == NULL)
		return NULL;

	sn -> item = * _Item;
	sn -> next = NULL;
	return sn;
}


//
static const ebstStack new_ebstStack(void) {
	const ebstStack es = { 0, NULL };
	return es;
}

//
static bool ebstStackPush(ebstStack * __Stack, const stack_item * _Item) {
	stack_node * new_node = new_stackNode(_Item);
	if (new_node == NULL)
		return false;

	stack_node * head_next = __Stack -> head;
	__Stack -> head = new_node;
	__Stack -> head -> next = head_next;
	__Stack -> size ++;
	
	return true;
}

//
static bool ebstStackPop(ebstStack * __Stack, stack_item * _ReturnItem) {
	if (__Stack->head == NULL)
		return false;

	stack_node * node_buffer = __Stack -> head;

	__Stack -> head = __Stack -> head -> next;
	__Stack -> size --;

	* _ReturnItem = node_buffer -> item;
	free(node_buffer);

	return true;
}


static void middlepointRootTraversalTranscript(
	FILE * input_stream, FILE * output_stream, const size_t n_registries
) {
	// Pointer for a subfile division in the input-stream.
	long left = 0, right = (long) n_registries - 1, middle;
	
	stack_item split_buffer = { left, right };
	
	ebstStack split_stack = new_ebstStack();
	ebstStackPush(& split_stack, & split_buffer);

	ebst_node buffer_node;

	size_t iterator = 0;

	while (ebstStackPop(& split_stack, & split_buffer))
	{
		left = split_buffer.left; right = split_buffer.right;
		middle = midpoint(left, right);

		buffer_node.left = -1; buffer_node.right = -1;

		if (left <= (middle - 1))
		{
			// Obs: That will already always be the case.
			// split_buffer.left = left; 

			split_buffer.right = middle - 1;

			ebstStackPush(& split_stack, & split_buffer);

			buffer_node.left = iterator + 1 + rightSubfileSize(left, right);
		}

		if ((middle + 1) <= right)
		{
			split_buffer.left = middle + 1;
			split_buffer.right = right;

			ebstStackPush(& split_stack, & split_buffer);

			buffer_node.right = iterator + 1;
		}

		// TODO: (Analysis) is it worth verifying the success of the following two IO ops?

		seekReadRegistry(input_stream, middle, & buffer_node.root_item);
		writeNode(output_stream, & buffer_node);

		iterator ++;
	}

}


// * It is for sure costly; O(n).
static size_t 
registryFilesize(FILE * input_stream)
{
	rewind(input_stream); 
	size_t counter = 0;
	
	registry_t dummy_buffer;
	while (! feof(input_stream)) {
		fread(& dummy_buffer, sizeof(registry_t), 1, input_stream);
		counter ++;
	}
	
	rewind(input_stream);
	return counter - 1;
}


bool assembleEBST_ordered(
	filepath orderedInputFilename, filepath outputFilename
) {
	FILE * input_stream = fopen(orderedInputFilename, "rb");
	if (input_stream == NULL)
		return false;

	FILE * output_stream = fopen(outputFilename, "wb");
	if (output_stream == NULL)
		return false;

	size_t n_registries = registryFilesize(input_stream);
	if (ferror(input_stream)) {
		fclose(input_stream);
		fclose(output_stream);
		return false;
	}

	printf("# registries identified: %u\n", (unsigned int) n_registries);

	middlepointRootTraversalTranscript(input_stream, output_stream, n_registries);

	fclose(input_stream);
	fclose(output_stream);
	return true;
}



search_response ebst_search(const int _key, FILE * __ebstStream, registry_t * __Return)
{
	return 0;
}

