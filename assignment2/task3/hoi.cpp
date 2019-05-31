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
    for(int i =sizeof(int)-1; i>=0; i--)
    {
        unsigned char sum = b >> (8*i);
        if(sum==0)
            continue;
        int v_pos = input.size()-((sizeof(int)-1)-i);
       // std::cout<<v_pos<<"\n";
        input[v_pos] += sum;
        if(overflow)
            input[v_pos] += 1;
        if(input[v_pos] == 0)
            overflow = true;
        else 
            overflow = false;

    }
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
     {   if(a[i] > b[i])
            return false;
        if(a[i] < b[i])
            return true;
    }
    return false;
}


int main(int argc, char * argv[])
{
    unsigned char result[MD5_DIGEST_LENGTH];
   
    std::vector<unsigned char> parsed_input;//512/8
    std::vector<int> indices ;
    auto input =argv[1];
    int blocks = atoi(argv[2]);
    for(int i = 3;i<argc;i++)
    {
        indices.push_back(atoi(argv[i]));
    }
    std::istringstream hex_chars_stream(input);
    unsigned int c;
    while (hex_chars_stream >> std::hex >> c)
    {
        if(c>>24 != 0)
            parsed_input.push_back(c>>24);
        if(c>>16 != 0)
        parsed_input.push_back(c>>16);
        if(c>>8 != 0)
        parsed_input.push_back(c>>8);
        if(c != 0)
        parsed_input.push_back(c);
    }
    
    parsed_input = pad_zeros(parsed_input, INPUT_SIZE);
     //print_vec(parsed_input);

    std::vector<std::vector<unsigned char>> hashes; 
    //#pragma omp parallel for shared(hashes)
    for(int i = 0; i < blocks; i++)
    {
        //print_vec(parsed_input);
    //    auto hash_in = add(parsed_input,i);
        auto hash_result = md5_wrap(parsed_input);  
        hashes.push_back(hash_result);
      // print_md5_sum(hash_result);
     
     // print_vec(hash_result);
      
         parsed_input = increment(parsed_input);
    }


    std::sort(hashes.begin(),hashes.end(),less);
    for(auto i:indices)
    {
        print_md5_sum(hashes[i]);
    }
//     print_md5_sum(hashes[0]);
// // print_md5_sum(hashes[1]);
//     print_md5_sum(hashes[2]);
// print_md5_sum(hashes[3]);



    //print_md5_sum(hash_result);
   // std::string input = "Test";
    //MD5((unsigned char *) input.c_str(), input.length() ,result);
    //print_md5_sum(result);
}