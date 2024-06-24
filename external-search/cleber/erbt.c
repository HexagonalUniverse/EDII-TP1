/*	<erbt.c> 
	
	A driver program for directly constructing a ERBT (External Red Black Tree). */


#include <stdio.h>
#include <external-search.h>


#define OUT_ERBT_FILENAME		"temp/cache/last-instance.erbt"

static bool __print_erbt_state(ERBT_STREAM * _Stream, size_t _NodesQtt)
{
	rewind(_Stream);

	ERBT_Header header = { 0 };
	if (! fread(&header, sizeof(ERBT_Header), 1, _Stream))
		return false;

	printf("%u\n", (unsigned int) header.root_ptr);

	erbt_node node_buffer = { 0 };
	size_t node_iterator = 0;
	while (read_erbtnode(_Stream, node_iterator ++, &node_buffer)) {
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

static int __build_erbt_from_stdin_sequence(frame_t * _Frame)
{
	unsigned int number_of_keys = 0;
	if (scanf("%u", &number_of_keys) <= 0)
		return -2;

	#if IMPL_LOGGING
		DebugPrintf("Number of keys to insert: %u.\n", number_of_keys);
	#endif

	registry_pointer reg_ptr = { 0 };
	ERBT_Builder builder = { .frame = * _Frame };
	builder.file_stream = (ERBT_STREAM *) fopen(OUT_ERBT_FILENAME, "wb+");
	if (builder.file_stream == NULL) {
		_ContextErrorMsgf("(build erbt stdin) ", "Couldn't open output ERBT data structure file.\n", NULL);
		return -3;
	}
	
	if (! fwrite(&builder.header, sizeof(ERBT_Header), 1, builder.file_stream))
		return -4;

	for (unsigned int keys_iterator = 0; keys_iterator < number_of_keys; keys_iterator ++)
	{
		if (scanf("%d", &reg_ptr.key) <= 0)
		{
			fclose(builder.file_stream);
			return -5;
		}

		#if IMPL_LOGGING
			DebugPrintf("Inserting key <%d>\n", reg_ptr.key);
		#endif

		if (! ERBT_insert(&builder, &reg_ptr))
		{
			fclose(builder.file_stream);
			
			_ContextErrorMsgf("(build erbt stdin) ", "Insertion of key <%d> failed.\n", reg_ptr.key);
			return 1;
		}

		ERBT_Balance(&builder, builder.registries_written - 1);
	}
	
	__print_erbt_state(builder.file_stream, builder.registries_written);
	fclose(builder.file_stream);
	return 0;
}

/*	The keys for the construction are taken as arguments, if they're passed. 
	Return negative values in case of errors on fundamental processes, and positive
	values for error in the building itself. */
int main(void)
{
	#if IMPL_LOGGING
	if (! InitializeLogging())
		return -6;
	#endif	

	frame_t frame;
	if (! frame_make(&frame, PAGES_PER_FRAME, sizeof(erbt_node), ERBT_PAGE))
		return -1;

	int response = __build_erbt_from_stdin_sequence(&frame);

	freeFrame(&frame);

	#if IMPL_LOGGING
		FinalizeLogging();
	#endif
	
	return response;
}
