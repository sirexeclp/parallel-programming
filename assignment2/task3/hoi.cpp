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
#include <string.h>
#include <algorithm> 

#define INPUT_SIZE 64 // 512bit /8 bits/byte

// Print the MD5 sum as hex-digits.
void print_md5_sum(std::vector<unsigned char> md) {
    int i;
    for(i=0; i <md.size(); i++) {
            printf("%02x",md[i]);
    }
    printf("\n");
}

std::vector<unsigned char> md5_wrap(std::vector<unsigned char> input)
{
    std::vector<unsigned char> result(MD5_DIGEST_LENGTH);
    MD5((unsigned char *) &input[0], input.size() ,&result[0]);
    return result;
}
std::vector<unsigned char> increment(std::vector<unsigned char> input)
{
   for(std::vector<unsigned char>::reverse_iterator it = input.rbegin(); it != input.rend(); ++it) {
        *it = ((*it)+1);
        if(*it != 0)
            return input;
    }
    return input;
}
std::vector<unsigned char> add(std::vector<unsigned char> input, int b)
{
    bool overflow = false;
    int v_pos =0;
    for(int i =0; i< sizeof(int); i++)
    {
        unsigned char sum = b >> (8*i);
        if(overflow)
        {
            input[v_pos] += 1;
            overflow = false;
        }
        if(sum==0)
            continue;
        v_pos = (input.size()-1) - i;
       // std::cout<<v_pos<<"\n";
        input[v_pos] += sum;

        if(input[v_pos] == 0)
            overflow = true;

    }
    if(overflow)
        input[v_pos-1] += 1;

    return input;
}
std::vector<unsigned char> pad_zeros(std::vector<unsigned char> input, int target_length)
{
    while (input.size()<target_length)
    {
       input.push_back(0);
    }
    return input;
}

void print_vec(std::vector<unsigned char> input)
{
    for(unsigned char c: input)
    {
        std::cout << std::hex << (unsigned int)c;
        if(c == 0)
            std::cout << "00";
    }
    std::cout << "\n";
}
const bool less( std::vector<unsigned char> &a, std::vector<unsigned char> &b)
{
    for(int i =0; i< a.size(); i++)
     {   
        if(a[i] > b[i])
            return false;
        if(a[i] < b[i])
            return true;
    }
    return false;
}

std::vector<unsigned char> parse_hex_string(char * input)
{
    std::vector<unsigned char> parsed_input;
    for (int i; i < strlen(input) ; i+=2)
    {
        char c1 = input[i];
        char c2 = input[i+1];
        unsigned char result = 0;
        if(c1>='0' && c1 <= '9')
            result += 16 * (c1 - '0');
        if(c1>='a' && c1 <= 'f')
            result += 16 * (c1 - 'a' + 10);

        if(c2>='0' && c2 <= '9')
            result += (c2 - '0');
        if(c2>='a' && c2 <= 'f')
            result += (c2 - 'a' + 10);    
        parsed_input.push_back(result);          
    }
    return parsed_input;
}


int main(int argc, char * argv[])
{       
    if(argc < 4)
    {
        std::cout << "use: ./hoi hex-prefix number-of-blocks block-ids ..." << "\n";
        return EXIT_FAILURE;
    }
    
    auto input =argv[1];
    auto parsed_input = pad_zeros(parse_hex_string(input), INPUT_SIZE);

    int blocks = atoi(argv[2]);
    std::vector<int> indices ;
    for(int i = 3;i<argc;i++)
    {
        indices.push_back(atoi(argv[i]));
    }

    std::vector<std::vector<unsigned char>> hashes; 
    
    #pragma omp parallel for
    for(int i = 0; i < blocks; i++)
    {
        auto hash_in = add(parsed_input,i);
        auto hash_result = md5_wrap(hash_in);
        //this is very expensive
        #pragma omp critical
        hashes.push_back(hash_result);
    }
    
    std::sort(hashes.begin(),hashes.end(),less);
    
    for(auto i:indices)
        print_md5_sum(hashes[i]);
    
    return EXIT_SUCCESS;
}