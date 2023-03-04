#include <stdio.h>



int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "r");

    int c = getc(fp);
    printf("%x\n", c);

    //while ((c = getc(fp)) != EOF)
    //    printf("%x\n", c);

}

