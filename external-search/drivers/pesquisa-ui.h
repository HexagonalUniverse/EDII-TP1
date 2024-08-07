/*	<drivers/registries-displayer.h

	Where the registries interface is defined. */


#ifndef _REGISTRIES_DISPLAYER_HEADER_FILE_
#define _REGISTRIES_DISPLAYER_HEADER_FILE_


#define to_upper(c)     ((char) (in_range(97, 122, (c)) ? ((c) - 32) : (c)))

static inline bool input_confirmation(void)
{
    printf("[y/n] ");
    fflush(stdout);
    fflush(stdin);

    char c = '\0';
    while ((c = getchar()) != EOF)
    {
        switch (to_upper(c))
        {
        case 'Y':   return true;
        case 'N':   return false;
        default:    break;
        }
    }
    return false;
}



int DisplayRegistryKeys(const uint64_t qtt) {
    // UI for confirmation on printing...
    printf("Deseja prosseguir com a impressão? ");
    
    if (! input_confirmation())
        return ES_SUCCESS;

    REG_STREAM * input_stream;
    if ((input_stream = fopen(INPUT_DATAFILENAME, "rb")) == NULL){
        _ContextErrorMsgf("(show registries keys) ", "Couldn't open registry file <%s>.\n", INPUT_DATAFILENAME);
        return ES_REG_FILE_WONT_OPEN;
    }

    const size_t regpage_item_maximum = 20LLU;
    const size_t total_pages = ceil_div(qtt, regpage_item_maximum) - 1;

    registry_t registries_buffer[regpage_item_maximum];

    size_t page_index = 0;
    size_t i = 0;   // iterator used on rendering...
    size_t qtt_read;

    bool load_page = true;
    char command = '\0';
    bool menu_loop = true;

    size_t select_index = 0;
    size_t element_index = 0;

    bool VM = true;
    while (menu_loop) {
        element_index = page_index * regpage_item_maximum;

        if (load_page)
        {
            fseek(input_stream, sizeof(registry_t) * (regpage_item_maximum * page_index), SEEK_SET);
            qtt_read = fread(registries_buffer, sizeof(registry_t), regpage_item_maximum, input_stream);
            load_page = false;
        }
        
        /*  Rendering */
        aec_reset(); aec_clean();
        printf("\nChaves em " _AEC_FG_YELLOW INPUT_DATAFILENAME _AEC_RESET ":\n\n");
        printf("Page #%llu\n", (unsigned long long) page_index);
        for (i = 0; (i < qtt_read) && ((element_index + i) < qtt); i ++)
        {
            if (i == select_index) {
                aec_bg_rgb(150, 150, 175);
                aec_fg_rgb(43, 43, 43);

            }
            else if (VM) {
                aec_bg_rgb(45, 50, 53); VM = false;
            }
            else {
                aec_bg_rgb(63, 69, 73); VM = true;
            }

            printf("[#%04llu] [", (unsigned long long) (element_index + i));
            aec_fg_rgb(231, 183, 111);
            printf("%-6d", (int) registries_buffer[i].key);
            printf("\e[1;39m");

            if (i == select_index)
                aec_fg_rgb(43, 43, 43);

            printf("] [%06lld] [%.4s] [%.4s]\n",
                (long long int) registries_buffer[i].data_1,
                (char *) registries_buffer[i].data_2, (char *) registries_buffer[i].data_3);
            aec_reset();
        }

        for (; i < regpage_item_maximum; i++)
        {
            if (i == select_index) {
                aec_bg_rgb(150, 150, 175);
                aec_fg_rgb(43, 43, 43);

            }
            else if (VM) {
                aec_bg_rgb(45, 50, 53); VM = false;
            }
            else {
                aec_bg_rgb(63, 69, 73); VM = true;
            }

            printf("[#%04llu] [------] [------] [----] [----]\n",
                (unsigned long long) (i + page_index * regpage_item_maximum));

            aec_reset();
        }

        /*  Footer */
        aec_reset();
        printf("\nComandos de navegação:\n");
        printf("\tw - próximo item\n");
        printf("\ta - próxima página\n");
        printf("\ts - item anterior\n");
        printf("\td - página anterior\n");
        printf("\tq - deixar vizualizador\n");
        printf("\n");
        fflush(stdout);
        fflush(stdin);

        /*  Processing input */
        scanf("%c", &command);
        command = to_upper(command);
        switch (command)
        {
        case 'Q':
            printf("Deseja deixar o vizualizador? ");
            if (input_confirmation())
                menu_loop = false; 
            printf("\n");
            break;

        case 'A': 
            if (page_index > 0)
            {
                page_index --;
                load_page = true;
            }
            break;

        case 'D':
            if (page_index < total_pages)
            {
                page_index ++;
                load_page = true;
            }
            break;

        case 'W':
            if (select_index > 0)
                select_index --;
            break;

        case 'S':
            select_index ++;
            if (select_index >= regpage_item_maximum)
                select_index --;
            break;

        default:    break;
        }
    }

    fclose(input_stream);
    
    return ES_SUCCESS;
}

void DisplayHelpInfo(void) {
    printf(_AEC_FG_BLUE "\npesquisa.exe" _AEC_RESET " - help-menu\n\n");
    printf("O programa "  "pesquisa.exe" " realiza a construção de estruturas de dados em memória externa "
    "e a pesquisa de chaves potencialmente presentes no arquivo de registros padrão " _AEC_FG_YELLOW INPUT_DATAFILENAME _AEC_RESET);
    printf("\n\nO arquivo " _AEC_FG_YELLOW INPUT_DATAFILENAME _AEC_RESET " contém uma quantidade arbitrária de registros da forma:\n");
    printf(_AEC_FG_GREEN "\tchave " _AEC_RESET " - Valor identificador de pesquisa do tipo inteiro.\n"
            _AEC_FG_GREEN "\tdado_1" _AEC_RESET " - Dado arbitrário do tipo long (int64).\n"
            _AEC_FG_GREEN "\tdado_2" _AEC_RESET " - Cadeia de 1000 caracteres.\n"
            _AEC_FG_GREEN "\tdado_3" _AEC_RESET " - Cadeia de 5000 caracteres.\n\n");
    printf("O arquivo de registros pode ser gerado utilizando a aplicação " _AEC_FG_BLUE "data-gen.exe" _AEC_RESET " disponível em: " 
    _AEC_FG_CYAN "bin/exe/data-gen.exe\n" _AEC_RESET);
    printf("\nUso de " _AEC_FG_BLUE "pesquisa.exe" _AEC_RESET ":\nEm external-search, utilize: ./bin/exe/pesquisa.exe [<método>] [<quantidade>] [<situação>] [<chave>] [-p]\n");
    printf(_AEC_FG_GREEN "\t<método>    " _AEC_RESET " Representa o método de pesquisa externa (Estrutura de dados) a ser executado, podendo "
    "ser um valor inteiro de 1 a 4*.\n"
            _AEC_FG_GREEN "\t<quantidade>" _AEC_RESET " Representa a quantidade de registros presentes do arquivo " _AEC_FG_YELLOW INPUT_DATAFILENAME "\n" _AEC_RESET
            _AEC_FG_GREEN "\t<situação>  " _AEC_RESET " Representa a situação de ordem do arquivo podendo ser um valor inteiro de 1 a 3**\n"
            _AEC_FG_GREEN "\t<chave>     " _AEC_RESET " Representa a chave de interesse a ser pesquisada.\n"
            _AEC_FG_GREEN "\t[-p]        " _AEC_RESET " Argumento opicioal que pode ser adicionado quando se deseja a exibição de "
            "todas as chaves de pesquisa existentes no arquivo " _AEC_FG_YELLOW INPUT_DATAFILENAME _AEC_RESET ".\n");
    printf("\n* Métodos de pesquisa:\t[1] Acesso Sequêncial Indexado\n"
            "\t\t\t[2] Árvore binária de pesquisa externa\n"
            "\t\t\t[3] Árvore B\n"
            "\t\t\t[4] Árvore B*\n");
    printf("\n** Situações de ordem:\t[1] Ordenado ascendentemente\n"
            "\t\t\t[2] Ordenado descendentemente\n"
            "\t\t\t[3] Desordenado\n\n");
}

#endif //_REGISTRIES_DIPLAYER_HEADR_FILE_