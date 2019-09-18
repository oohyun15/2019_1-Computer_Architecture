#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_BUF     4096

/*******************************************************
 * Function Declaration
 *
 *******************************************************/
char *change_file_ext(char *str);
void del_char(char *, char , int, int);
int parsing(char *, char **, char *);
char *DecToBin(int, int);
char *Rtype(char *, int);
char *Itype(char *, char *, int);
char *Jtype(char *);
char *lui(char **, char *);
char *ori(char **, char *);
char *check_inst(char **);
void process(FILE *, FILE *);

typedef struct dict
{
    char key[128];
    int val[10];
    int count;
    long addr;
} dict;

dict *d_dict;
dict *t_dict;

int d_label_count = 0;
int t_label_count = 0;
int data_count = 0;
int text_count = 0;

long dc = 0x10000000;
long pc = 0x00400000;

/*******************************************************
 * Function: main
 *
 * Parameters:
 *  int
 *      argc: the number of argument
 *  char*
 *      argv[]: array of a sting argument
 *
 * Return:
 *  return success exit value
 *
 * Info:
 *  The typical main function in C language.
 *  It reads system arguments from terminal (or commands)
 *  and parse an assembly file(*.s).
 *  Then, it converts a certain instruction into
 *  object code which is basically binary code.
 *
 *******************************************************/
int
main(int argc, char *argv[])
{
    FILE *input, *output;
    char *filename;
    printf("%s, %s\n", argv[0], argv[1]);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    filename = strdup(argv[1]); // strdup() is not a standard C library but fairy used a lot.
    if(change_file_ext(filename) == NULL) {
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }
    output = fopen(filename, "w");
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    process(input, output);
    fclose(input);
    fclose(output);
    exit(EXIT_SUCCESS);
}

/*******************************************************
 * Function: change_file_ext
 *
 * Parameters:
 *  char
 *      *str: a raw filename (without path)
 *
 * Return:
 *  return NULL if a file is not an assembly file.
 *  return empty string
 *
 * Info:
 *  This function reads filename and converst it into
 *  object extention name, *.o
 *
 *******************************************************/
char
*change_file_ext(char *str)
{
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0)) {
        return NULL;
    }

    str[strlen(str) - 1] = 'o';
    return "";
}


void del_char(char *str, char sep, int first, int end)
{
	// first char
	if (first == 1)
	{
		while (str[0] == sep)
		{
			for (int i = 0; i < strlen(str); i++)
			{
				if (str[i + 1] != '\0') str[i] = str[i + 1];

				else str[i] = '\0';
			}
		}
	}
	// end char
	if (end == 1)
	{
		while (str[strlen(str) - 1] == sep) str[strlen(str) - 1] = '\0';
	} 
}
int parsing(char *opn, char **args, char *op)
{
	char *sep = strtok(opn, op);
	int n = 0;
	while (sep != NULL)
	{
		args[n++] = sep;
		sep = strtok(NULL, op);
	}
	return n;
}

char *DecToBin(int dec, int n)
{
    char *bin = (char *)malloc(sizeof(char)*32);
    for (int i = n-1; i >= 0; --i)
    {
      sprintf(bin + strlen(bin), "%d", (dec >> i) & 1);
    }
    return bin;
}

char *Rtype(char *rt, int type)
{
    char **array = (char **)malloc(sizeof(char *)*32);
    char *Rret = (char *)malloc(sizeof(char)*32);
    int len = parsing(rt, array, " ");
    for (int i = 0; i< len; i++)
    {
        del_char(array[i],'$', 1, 0);
        del_char(array[i], ',', 0, 1);
    }
    // common case
    if (type == 1) sprintf(Rret, "%s%s%s", DecToBin(atoi(array[1]), 5) ,DecToBin(atoi(array[2]), 5), DecToBin(atoi(array[0]), 5));
    // srl, sll
    else if (type == 2) sprintf(Rret, "%s%s%s", DecToBin(atoi(array[1]), 5) ,DecToBin(atoi(array[0]), 5), DecToBin(atoi(array[2]), 5));
    // jr
    else if (type == 3) sprintf(Rret, "%s", DecToBin(atoi(array[0]), 5));
    free(array);
    return Rret;
}
char *Itype(char *it, char *immediate, int type)
{
    char **array = (char **)malloc(sizeof(char *)*32);
    char *Iret = (char *)malloc(sizeof(char)*32);
    int len = parsing(it, array, " ");
    for (int i = 0; i< len; i++)
    {
        del_char(array[i],'$', 1, 0);
        del_char(array[i], ',', 0, 1);
    }
   // la 경우때문에 작성
   if (type == 1 && len < 2)
   {
       len++;
       array[1] = array[0];
   }
    // lw, sw parsing
    // and, ori 계열
    if (type == 1) sprintf(Iret, "%s%s", DecToBin(atoi(array[1]), 5) ,DecToBin(atoi(array[0]), 5));
    // bne, beq 계열
    else if (type == 2) sprintf(Iret, "%s%s", DecToBin(atoi(array[0]), 5) ,DecToBin(atoi(array[1]), 5));
    // lui
    else if (type == 3) sprintf(Iret, "%s", DecToBin(atoi(array[0]), 5));
    // lw, sw
    else if (type == 4)
    {
        char **split_arr = (char **)malloc(sizeof(char *)*32);
        parsing(array[1], split_arr, "(");
        del_char(split_arr[1], '$', 1, 0);
        del_char(split_arr[1], ')', 0, 1);
        len++;
        array[len-1] = array[1];
        array[1] = split_arr[1];
        sprintf(Iret, "%s%s", DecToBin(atoi(array[1]), 5) ,DecToBin(atoi(array[0]), 5));
        free(split_arr);
    }
    strcpy(immediate, array[len-1]);
    free(array);
    return Iret;
}

char *Jtype(char *jt)
{
    char **array = (char **)malloc(sizeof(char *)*32);
    char *Jret = (char *)malloc(sizeof(char)*32);
    int addr = 0;
    for (int i = 0; i < t_label_count ; i++)
    {
        if (strcmp(jt, t_dict[i].key) == 0) addr = t_dict[i].val[0] + (int)(pc >> 2);
    }
    sprintf(Jret, "%s", DecToBin(addr, 26));
    free(array);
    return Jret;
}

char *check_inst(char **instruct)
{
    // Rtype
    if (strcmp(instruct[0], "and") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000100100", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "addu") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000100001", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "subu") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000100011", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "or") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000100101", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "nor") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000100111", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "sltu") == 0)
    {
       char *opn = Rtype(instruct[1], 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s00000101011", opn);
       free(opn);
       return reg;
    } 
    else if (strcmp(instruct[0], "sll") == 0)
    {
       char *opn = Rtype(instruct[1], 2);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "00000000000%s000000", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "srl") == 0)
    {
       char *opn = Rtype(instruct[1], 2);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "00000000000%s000010", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "jr") == 0)
    {
       char *opn = Rtype(instruct[1], 3);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000000%s000000000000000001000", opn);
       free(opn);
       return reg;
    }
    // Itype
    else if (strcmp(instruct[0], "andi") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       else num = atoi(immediate);
       sprintf(reg, "001100%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "addiu") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 1);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       
       else num = atoi(immediate);
       
       sprintf(reg, "001001%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "lw") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 4);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16); 
       else num = atoi(immediate);       
       sprintf(reg, "100011%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "sw") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 4);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       
       else num = atoi(immediate);
       
       sprintf(reg, "101011%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "la") == 0)
    {
        char immediate[128];
        char **array = (char **)malloc(sizeof(char *)*32);
        char *reg = (char *)malloc(sizeof(char)*32);
        int len = parsing(instruct[1], array, " ");
        for (int i = 0; i< len; i++)
        {
           del_char(array[i], '$', 1, 0);
           del_char(array[i], ',', 0, 1);
        }

        for (int i = 0; i< d_label_count; i++)
        {
            if (strcmp(array[len -1], d_dict[i].key) == 0)
            {
                char str[9];
                char _hi[7];
                char _lo[7];
                sprintf(str, "%lx", d_dict[i].addr);
                sprintf(_hi,"%s%c%c%c%c", "0x",str[0],str[1],str[2],str[3]);
                sprintf(_lo,"%s%c%c%c%c", "0x",str[4],str[5],str[6],str[7]);
                
                if (strcmp(_lo, "0x0000") != 0) 
                {
                    sprintf(reg, "%s%s", lui(instruct, _hi), ori(instruct, _lo));
                    text_count++;
                }
                else sprintf(reg, "%s", lui(instruct, _hi));
            }
        }
        return reg;
    }
    else if (strcmp(instruct[0], "ori") == 0)
    {
        return ori(instruct, NULL);
    }
    else if (strcmp(instruct[0], "lui") == 0)
    {
       return lui(instruct, NULL);
    }   
    else if (strcmp(instruct[0], "bne") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 2);
       char *reg = (char *)malloc(sizeof(char)*32);
       int addr = 0;
       for (int i = 0; i< t_label_count; i++)
       {
           if (strcmp(immediate, t_dict[i].key) == 0) addr = t_dict[i].val[0] - text_count - 1; 
       }
       sprintf(reg, "000101%s%s", opn, DecToBin(addr, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "beq") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 2);
       char *reg = (char *)malloc(sizeof(char)*32);
       int addr = 0;
       for (int i = 0; i< t_label_count; i++)
       {
           if (strcmp(immediate, t_dict[i].key) == 0) addr = t_dict[i].val[0] - text_count - 1;         
       }       
       sprintf(reg, "000100%s%s", opn, DecToBin(addr, 16));
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "sltiu") == 0)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 1);
       char *reg = (char *)malloc(sizeof(char)*32);  
       int num;
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       else num = atoi(immediate);          
       sprintf(reg, "001011%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }         
    // Jtype   
    else if (strcmp(instruct[0], "j") == 0)
    {
       char *opn = Jtype(instruct[1]);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000010%s", opn);
       free(opn);
       return reg;
    }
    else if (strcmp(instruct[0], "jal") == 0)
    {
       char *opn = Jtype(instruct[1]);
       char *reg = (char *)malloc(sizeof(char)*32);
       sprintf(reg, "000011%s", opn);
       free(opn);
       return reg;
    }  
    else return "";
}
char *lui(char **instruct, char *addr)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 3);
       if (addr != NULL) strcpy(immediate, addr);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       else num = atoi(immediate);

       sprintf(reg, "00111100000%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
char *ori(char **instruct, char *addr)
    {
       char immediate[128];
       char *opn = Itype(instruct[1], immediate, 1);
       if (addr != NULL) strcpy(immediate, addr);
       char *reg = (char *)malloc(sizeof(char)*32);
       int num;
       if (strncmp(immediate, "0x", 2) == 0) num = strtol(immediate, NULL, 16);
       else num = atoi(immediate);

       sprintf(reg, "001101%s%s", opn, DecToBin(num, 16));
       free(opn);
       return reg;
    }
void process(FILE *fdin, FILE *fdout)
{
    d_dict = malloc(sizeof(dict)*10);
    t_dict = malloc(sizeof(dict)*10);
    char buf[MAX_BUF];
    char regs[MAX_BUF];
    int flag = 0;
    while(fgets(buf, 128, fdin))
    {
        del_char(buf, '\t', 1, 0);
        del_char(buf, '\n', 0, 1);
        
        if (strcmp(buf, ".data") == 0)
        {   
            flag = 1;
            continue;
        }
        else if (strcmp(buf, ".text") == 0)
        {
            flag = 2;
            continue;
        }
        else if (flag == 2 && strrchr(buf, ':') != NULL)
        {
            del_char(buf,':', 0, 1);
            strcpy(t_dict[t_label_count].key, buf);
            t_dict[t_label_count].val[0] = text_count;
            t_dict[t_label_count].count++;
            t_label_count++;
            continue;
        }

        if (flag == 1) 
        {
            char *data[MAX_BUF] = {NULL, };
            int len = parsing(buf, data, "\t");
            int num;
            if (strchr(data[0], ':') != NULL)
            {
                del_char(data[0], ':', 0, 1);
                strcpy(d_dict[d_label_count].key, data[0]);
                if (strncmp(data[len-1], "0x", 2) == 0) num = strtol(data[len-1], NULL, 16);
                else num = atoi(data[len-1]);
                d_dict[d_label_count].val[d_dict[d_label_count].count++] = num;
                d_dict[d_label_count].addr = dc;
                dc += 0x4;
                d_label_count++;
            }
            else
            {
                if (strncmp(data[len-1], "0x", 2) == 0) num = strtol(data[len-1], NULL, 16);
                else num = atoi(data[len-1]);
                d_dict[d_label_count-1].val[d_dict[d_label_count-1].count++] = num;        
                dc += 0x4;    
            }
            data_count++;
        }
        else if (flag == 2)
        {
            char *instruct[MAX_BUF] = {NULL, };
            int len = parsing(buf,instruct,"\t");
            char *reg = check_inst(instruct);
            text_count++;  
        } 
    }
    flag = 0;
    text_count = 0;
    dc = 0x10000000;
    pc = 0x00400000;
    fseek(fdin, 0L, SEEK_SET);
    while(fgets(buf, 128, fdin))
    {
        del_char(buf, '\t', 1, 0);
        del_char(buf, '\n', 0, 1);
        if (strcmp(buf, ".data") == 0)
        {   
            flag = 1;
            continue;
        }
        else if (strcmp(buf, ".text") == 0)
        {
            flag = 2;
            continue;
        }
        if (flag == 2 && strrchr(buf, ':') == NULL)
        {
            char *instruct[MAX_BUF] = {NULL, };
            int len = parsing(buf,instruct,"\t");
            char *reg = check_inst(instruct);
            sprintf(regs + strlen(regs), "%s", reg);
            text_count++;  
        } 
    }
    char final_regs[MAX_BUF];
    int data[MAX_BUF];
    int index = 0;
    char *bin_text = DecToBin(4*text_count, 32);
    char *bin_data = DecToBin(4*data_count, 32);
    for (int i = 0; i < d_label_count; i++)
    {
        for (int j = 0; j < d_dict[i].count; j++)
        {
            data[index] = d_dict[i].val[j];
            sprintf(regs + strlen(regs), "%s", DecToBin(data[index], 32));
            index++;
        }
    }
    sprintf(final_regs, "%s%s%s\n", bin_text, bin_data, regs);
    fputs(final_regs, fdout);
    free(bin_text);
    free(bin_data);
}