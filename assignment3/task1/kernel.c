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
//#define __UINT32_MAX__ 0xffffffffU
kernel void reduce(global unsigned long *workpack_result,
                   local unsigned long *tmp)
{
    int lid = get_local_id(0) + (get_local_id(1) * get_local_size(0)) + (get_local_size(0) * get_local_size(1) * get_local_id(2));
    int group_size = get_local_size(0) * get_local_size(1) * get_local_size(2);
    
    unsigned long start = (get_global_offset(1) << 32) + get_global_offset(0);
    unsigned long range = ((get_global_size(1)-1)<< 32) + get_global_size(0);

    unsigned long global_id =  get_global_id(0) + (get_global_id(1) << 32) -1;

    unsigned long group_id =(get_group_id(0) + get_num_groups(0) *  get_group_id(1) + get_group_id(2)*get_num_groups(0)*get_num_groups(1) );

    tmp[lid] = start + global_id;
    barrier(CLK_LOCAL_MEM_FENCE);
    group_size = range - group_id;
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
        workpack_result[group_id] = tmp[0];
}