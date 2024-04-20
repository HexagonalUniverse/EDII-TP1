

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "src/common.h"


static void PrintRegistry(const registry_t * __Registry) {
    printf("R | %8d %16d %c %c |", 
        __Registry -> key, 
        (int) __Registry -> data_1, 
        __Registry -> data_2[0], 
        __Registry -> data_3[0]);
}


static void PrintEBSTNode(const ebst_node * __Node) {
    printf("N [ %8d <%3d, %3d> ]", __Node -> root_item.key, __Node -> left, __Node -> right);
}


static char * fileExtension(filepath __FilePath) {
    const size_t length = strlen(__FilePath);

    size_t i;
    for (i = length - 1; i > 0 && __FilePath[i] != '.'; i --);
    
    char * extension = (char *) malloc(sizeof(char) * (length - i));

    for (size_t j = i; j < length; j ++)
        extension[j - i] = __FilePath[j + 1];

    return extension;
}


static finline bool streq(const char * __String1, const char * __String2) {
    size_t iterator = 0;
    while (__String1[iterator] && __String2[iterator]) {
        if (__String1[iterator] != __String2[iterator])
            return false;
        iterator ++;
    }
    return __String1[iterator] == __String2[iterator];
}



static void ShowBinaryDataFile(FILE * input_stream) {
    registry_t reg_buffer;
    for (size_t j = 0; j < 100; j ++) 
    {
        fread(& reg_buffer, sizeof(registry_t), 1, input_stream);
        printf("#%2d\t", j);
        PrintRegistry(& reg_buffer);
        printf("\n");
    }
}


static void ShowEBSTFile(FILE * input_stream) {
    ebst_node buffer;
    for (size_t j = 0; j < 100; j ++) 
    {
        fread(& buffer, sizeof(ebst_node), 1, input_stream);
        printf("#%2d\t", j);
        PrintEBSTNode(& buffer);
        printf("\n");
    }
}


int main(int argc, char ** argsv) {
    FILE * input_stream;
    char * extension;

    for (int i = 1; i < argc; i++) 
    {
        printf("%s ---\n", argsv[i]);
        
        extension = fileExtension(argsv[i]);
        printf("extension: <%s>\n", extension);
        
        input_stream = fopen(argsv[i], "rb");

        if (streq(extension, "bin"))
            ShowBinaryDataFile(input_stream);

        else if (streq(extension, "ebst"))
            ShowEBSTFile(input_stream);
        else
            fprintf(stderr, "Unknown extension...\n");


        fclose(input_stream);
        free(extension);
    }

    return EXIT_SUCCESS;
}