#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#define ITENS_PER_PAGE 5
#define PAGES_PER_FRAME 5

#define RD2_SIZE 1000
#define RD3_SIZE 5000

typedef struct{

    int key;

    int64_t data_1;
    char data_2[RD2_SIZE];
    char data_3[RD3_SIZE];

} registry_t;

typedef struct{

    registry_t reg[ITENS_PER_PAGE];
    int num_page;   
} page_t;

typedef struct{

    page_t pages[PAGES_PER_FRAME];
    int first;
    int last;
    int sized;
}frame_t;


void makeFrame(frame_t *frame){

    frame->first = -1;
    frame->last = -1;
    frame->sized = 0;
}


bool removePage(frame_t *frame){

    if(isFrameEmpty(frame)){
        
        return false;
    }

    if(frame->last == frame->first){
        frame->last = -1;
        frame->first = -1;
    }
    else{
        frame->first = (frame->first + 1) % PAGES_PER_FRAME;
    }
    frame->sized--;

    return true;
}


bool addPage(int num_page, frame_t *frame, FILE *arq){

    printf("Frame->sized = %d\n", frame->sized);
    if(isFrameFull(frame)){
        if(!removePage(frame))
            return false;
    }
    if(isFrameEmpty(frame)){

        frame->first = 0;
        frame->last = 0;
    }
    else{
        frame->last = (frame->last + 1) % PAGES_PER_FRAME;
    }

    frame->pages[frame -> last].num_page = num_page;
    num_page--;
    
    fseek(arq, num_page * sizeof(registry_t) * ITENS_PER_PAGE, 0);
    fread(frame->pages[frame -> last].reg, sizeof(registry_t), ITENS_PER_PAGE, arq);
    
    frame->sized++;

    return true;
}


void showFrame(frame_t *frame){

    if(isFrameEmpty(frame)){
        return;
    }

    for (int i = 0; i < PAGES_PER_FRAME; i++) {
        printf("Page %d | (%d)\t", i+1, frame -> pages[i].num_page);
        if (i == frame -> first) {
            printf("<- first");
        } else if (frame -> last == i) {
            printf("<- last");
        }

        putchar('\n');

        for(int j=0; j<ITENS_PER_PAGE; j++){
            printf("Reg %d\n", frame->pages[i].reg[j].key);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

int main(){

    frame_t frame = { 0 };
    makeFrame(&frame);

    FILE *arq = fopen("reg.bin", "rb");

    addPage(1,&frame,arq);
    showFrame(&frame);
    addPage(2,&frame,arq);
    showFrame(&frame);
    addPage(3,&frame,arq);
    showFrame(&frame);
    addPage(4,&frame,arq);
    showFrame(&frame);
    addPage(5,&frame,arq);
    showFrame(&frame);
    addPage(6,&frame,arq);
    showFrame(&frame);

    fclose(arq);

    return 0;
}
