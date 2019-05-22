#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "[Error] Usage: %s password-filepath dictionary-filepath\n", argv[0]);
    }
    return 0;
}
