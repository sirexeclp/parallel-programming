kernel void reduce(
		   const unsigned long offset,
		   global unsigned long *workpack_result_low,
		   global unsigned long *workpack_result_high,
                   local unsigned long *tmp_low,
                   local unsigned long *tmp_high)
{
    int lid = get_local_id(0) + (get_local_id(1) * get_local_size(0)) + (get_local_size(0) * get_local_size(1) * get_local_id(2));

    unsigned long global_id = get_global_id(0);

    // input is 64bit so we can init the upper 8bytes with 0
    tmp_low[lid] = offset + global_id;
    tmp_high[lid] = 0;

    barrier(CLK_LOCAL_MEM_FENCE);
    int group_size = get_local_size(0) * get_local_size(1) * get_local_size(2);
    int half_block_size = group_size / 2;

    while (half_block_size > 0)
    {

        if (lid < half_block_size)
        {
            tmp_low[lid] += tmp_low[lid + half_block_size];
            tmp_high[lid] += tmp_high[lid + half_block_size] + (tmp_low[lid] < tmp_low[lid + half_block_size]);

            // if uneven
            if ((half_block_size * 2) < group_size)
            {
                if (lid == 0)
                {
                    tmp_low[0]  += tmp_low[group_size - 1];
                    tmp_high[0] += tmp_high[group_size - 1] + (tmp_low[0] < tmp_low[group_size - 1]);
                }
            }

        }
        barrier(CLK_LOCAL_MEM_FENCE);
	group_size = half_block_size;
        half_block_size = group_size / 2;
    }

    if (lid == 0)
    {
        workpack_result_low[get_group_id(0)] = tmp_low[0];
        workpack_result_high[get_group_id(0)] = tmp_high[0];
    }
}
