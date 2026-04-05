#include "./xxh3.h"
#include <stdio.h>


int main(int argc, char const *argv[])
{
    char current = 0;
    int length = 0;
    int index = 0;

    if (argc < 2)
    {
        printf("Please provode a string to be hashed:\n\t> ./hasher.exe [input]\n");
        return 0;
    }

    do
    {
        current = (argv[1])[index];
        length++;
        index++;
    } while (current != 0);

    printf("String to be hashed: %s, length: %i\n", argv[1], length - 1);
    XXH64_hash_t hash = XXH64(argv[1], length - 1, 0);
    printf("Hash: 0x%llx\n", hash);
    return 0;
}
