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
#include <omp.h>

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
    unsigned char c;
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
    
    // print_md5_sum(parsed_input);
    parsed_input = pad_zeros(parsed_input, INPUT_SIZE);

    std::vector<std::vector<unsigned char>> hashes; 
    
    int work_block_size = blocks / omp_get_max_threads();

    std::cout << "wb:" << work_block_size<< "\n";
    // #pragma omp declare reduction (merge : std::vector<std::vector<unsigned char>> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))

    #pragma omp parallel
    {
        std::vector<std::vector<unsigned char>> local_hashes; 
        // std::cout << work_block_size * omp_get_thread_num()<< "--" <<std::min(work_block_size * (omp_get_thread_num()+1),blocks) << std::endl;
        int start = work_block_size * omp_get_thread_num();
        int end = std::min(start + work_block_size ,blocks);
        int i = start;
        #pragma omp for private(i)
        for( i = start; i < end; i++)
        {
            //print_vec(parsed_input);
            auto hash_in = add(parsed_input,i);
            auto hash_result = md5_wrap(hash_in);
            
            local_hashes.push_back(hash_result);
            // print_md5_sum(hash_result);
            
            // print_vec(hash_result);
            
            // parsed_input = increment(parsed_input);
        }
        std::cout <<"size:" <<local_hashes.size() << "\n";
        #pragma omp critical
        hashes.insert(hashes.end(),local_hashes.begin(),local_hashes.end());
    }
    #pragma omp barrier
    // print_md5_sum(parsed_input);

    std::cout <<"size:" <<hashes.size() << "\n";
    std::sort(hashes.begin(),hashes.end(),less);
    for(auto i:indices)
    {
        print_md5_sum(hashes[i]);
    }
    // for(auto i:hashes)
    // {
    //     print_md5_sum(i);
    // }

    // std::cout << (char (255 +1)) << "\n";
//     print_md5_sum(hashes[0]);
// // print_md5_sum(hashes[1]);
//     print_md5_sum(hashes[2]);
// print_md5_sum(hashes[3]);



    //print_md5_sum(hash_result);
   // std::string input = "Test";
    //MD5((unsigned char *) input.c_str(), input.length() ,result);
    //print_md5_sum(result);
}