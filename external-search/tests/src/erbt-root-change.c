
#include <stdio.h>
#include <external-search.h>
#include "tests.h"

#ifdef TRANSPARENT_COUNTER
struct __transparent_counter_t transparent_counter = { 0 };
#endif


static bool __print_root_change_response(ERBT_STREAM * _Stream, size_t _NodesQtt)
{
	fseek(_Stream, 0, SEEK_SET);

	ERBT_Header header = { 0 };
	if (! fread(& header, sizeof(ERBT_Header), 1, _Stream)) {
		return false;
	}

	printf("%u\n", (unsigned int) header.root_ptr);

	erbt_node node_buffer = { 0 };
	size_t node_iterator = 0;
	while (read_erbtnode(_Stream, node_iterator ++, & node_buffer)) {
		printf("%d %d %d %d ",
			(int) node_buffer.reg_ptr.key,
			(int) node_buffer.left,
			(int) node_buffer.right,
			(int) node_buffer.father);

		if (node_buffer.color == RED) {
			printf("RED");
		}
		else {
			printf("BLACK");
		}

		putchar('\n');
	}
	return _NodesQtt == node_iterator;
}

static int __Test_ERBT_root_change(frame_t * _Frame)
{
	unsigned int number_of_keys = 0;
	if (scanf("%u", & number_of_keys) <= 0)
	{
		return -2;
	}
	fprintf(stderr, "number of keys: %u\n", number_of_keys);

	registry_pointer reg_ptr = { 0 };
	ERBT_Builder builder = { 0 };
	builder.registries_written = 0;
	builder.frame = * _Frame;

	if ((builder.file_stream = (ERBT_STREAM *) fopen(TEST_ERBT_FILENAME, "wb+")) == NULL)
		return -3;
	
	if (! fwrite(& builder.header, sizeof(ERBT_Header), 1, builder.file_stream))
		return -4;

	for (unsigned int keys_iterator = 0; keys_iterator < number_of_keys; keys_iterator ++)
	{
		if (scanf("%d", & reg_ptr.key) <= 0)
		{
			fclose(builder.file_stream);
			return -3;
		}
		fprintf(stderr, "key: %d\n", reg_ptr.key);

		if (! ERBT_insert(& builder, & reg_ptr))
		{
			fclose(builder.file_stream);
			printf("couldn't insert bro\n");
			return 1;
		}

		ERBT_Balance(& builder, builder.registries_written - 1);
	}
	
	__print_root_change_response(builder.file_stream, builder.registries_written);
	fclose(builder.file_stream);
	return 0;
}


int main(void)
{
#if IMPL_LOGGING
	InitializeLogging();
#endif

	frame_t frame;
	if (! frame_make(& frame, PAGES_PER_FRAME, sizeof(erbt_node), ERBT_PAGE))
		return -1;

	int response = __Test_ERBT_root_change(& frame);

	freeFrame(& frame);

#if IMPL_LOGGING
	FinalizeLogging();
#endif

	fprintf(stderr, "retorno: %d\n", response);
	return response;
}
