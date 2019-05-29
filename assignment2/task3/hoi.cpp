#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <openssl/md5.h>
#include <sstream>
#include <iostream>


// Print the MD5 sum as hex-digits.
void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
    printf("\n");
}


int main(int argc, char * argv[])
{
    unsigned char result[MD5_DIGEST_LENGTH];
    char input [64]; //512/8
    for(int i =0; i<64; i++)
    {
        input[i] = '0';
    }
    unsigned int x;
    auto start =argv[1];
    std::stringstream ss;
    ss << std::hex << start;
    ss >> input;
    for(auto a:input)
    {
        std::cout << a;
    }
   // std::string input = "Test";
    //MD5((unsigned char *) input.c_str(), input.length() ,result);
    //print_md5_sum(result);
}