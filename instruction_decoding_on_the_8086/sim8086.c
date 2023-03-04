#include <stdio.h>


struct encoding {
    unsigned int opcode;
    unsigned int d;
    unsigned int w;
    unsigned int mod;
    unsigned int reg;
    unsigned int rm;
};


int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "r");

    int c = getc(fp);
    printf("%x\n", c);

    //while ((c = getc(fp)) != EOF)
    //    printf("%x\n", c);

}

