

#include <stdio.h>
#include <external-search.h>
#include "tests.h"

#ifdef TRANSPARENT_COUNTER
struct __transparent_counter_t transparent_counter = { 0 };
#endif

static int __Test_FrameRefresh(void)
{
	REG_STREAM * input_stream = (REG_STREAM *) fopen(TEST_DATA_FILENAME, "rb");
	if (input_stream == NULL) {
		printf("arrquivooasdpkjna sdn\n");
		return -1;
	}
	
	unsigned int frame_size = 0;
	if (! scanf("%u", & frame_size))
		return -2;

	frame_t frame = { 0 };
	if (! frame_make(& frame, frame_size, sizeof(regpage_t), REG_PAGE))
	{
		fclose(input_stream);
		return -3;
	}

	unsigned int n_iterations = 0;
	if (! scanf("%u", & n_iterations))
		return -4;

	int page_index = 0;
	int return_value = 0;

	for (unsigned int i = 0; i < n_iterations; i ++)
	{
		if (! scanf("%u", & page_index)) {
			return_value = -5;
			break;
		}

		if (page_index > TEST_DATA_FILESIZE)
		{
			return_value = 1;
			break;
		}

		frame_add(page_index, & frame, input_stream);
	}
	
	if (return_value == 0) {
		
		putchar('<');
		const unsigned int max_size_m1 = frame.max_size - 1;
		for (unsigned int j = 0; j < max_size_m1; j ++)
		{
			printf("%u:%d, ",
				(unsigned int) frame.indexes[j],
				(int) ((regpage_t *) frame.pages)[j].reg[0].key
			);
		}
		printf("%u:%d>\n",
			(unsigned int) frame.indexes[max_size_m1],
			(int) ((regpage_t *) frame.pages)[max_size_m1].reg[0].key
		);
	}

	fclose(input_stream);
	freeFrame(& frame);
	return return_value;
}


int main(void) {
#if IMPL_LOGGING
	InitializeLogging();
#endif

	int response = __Test_FrameRefresh();

#if IMPL_LOGGING
	FinalizeLogging();
#endif

	return response;
}
