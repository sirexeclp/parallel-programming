kernel void update_cell(local const unsigned int width,
                        local const unsigned int height,
                        global unsigned float *map_before,
                        global unsigned float *map_after)
{
    float acc = 0;
    int x = get_global_id(0);
    int y = get_global_id(1);

    for (int i = std::max(x-1, 0); i < std::min(x+2, offset); i++)
    {
        for (int j = std::max(y-1, 0); j < std::min(y+2, height); j++)
        {
            acc += map_before[j * offset + i];
        }
    }

    map_after[x + y * offset] = acc / 9.0;
}
