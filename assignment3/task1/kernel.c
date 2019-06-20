kernel void init(const unsigned long offset, global unsigned long *output)
{
    size_t i = get_global_id(0);
    output[i] = i + offset;
}

kernel void add128(const unsigned long a_high, const unsigned long a_low, const unsigned long b_high, const unsigned long b_low, global unsigned long *r_high, global unsigned long *r_low)
{
    size_t i = get_global_id(0);
    r_low[i] = a_low + b_low;
    r_high[i] = a_high + b_high + (r_low[i] < a_low);
}

// kernel void add128(global unsigned long *data)
// {
//     size_t i = get_global_id(0)*6;

//     data[i+4] = data[i] + data[i+2];
//     data[i+5] = data[i+1] + data[i+3] + (data[i+4] < data[i]);
// }

// kernel void reduce(local unsigned long *input,
//     global  unsigned long *output)
// {
//     // size_t i = get_global_id(0);
//     // output[i] = input[(i*2)] + input[(i*2)+1];
//     // input[i]=0;

//     int lid = get_local_id(0);
//     int group_size = get_local_size(0);
//   //  output[lid] = input[get_global_id(0)];
//     barrier(CLK_LOCAL_MEM_FENCE);

//     for(int i = group_size/2; i>0; i >>= 1) {
//         if(lid < i) {
//             input[lid] += input[lid + i];
//         }
//         barrier(CLK_LOCAL_MEM_FENCE);
//     }

// }

kernel void reduce(const unsigned long workpack_id,
                   global unsigned long *workpack_start,
                   global unsigned long *workpack_end,
                   global unsigned long *workpack_result,
                   local unsigned long *tmp)
{

    unsigned long start = workpack_start[workpack_id];
    unsigned long end = workpack_start[workpack_id];

    int lid = get_local_id(0);
    tmp[lid] = start + get_global_id(0);
    barrier(CLK_LOCAL_MEM_FENCE);
    int group_size = get_local_size(0);
    int half_block_size = group_size / 2;
    while (half_block_size > 0)
    {

        if (lid < half_block_size)
        {
            tmp[lid] += tmp[lid + half_block_size];
            //if uneven
            if ((half_block_size * 2) < group_size)
            {
                if (lid == 0)
                {
                    tmp[0] += tmp[group_size - 1];
                }
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        group_size = half_block_size;
        half_block_size >>= 1;
    }
    //barrier(CLK_LOCAL_MEM_FENCE);
    if (lid == 0)
        workpack_result[workpack_id] = tmp[0];
}