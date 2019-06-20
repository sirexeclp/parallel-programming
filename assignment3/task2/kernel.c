kernel void update_cell(
                        global float *map_before,
                        global float *map_after)
{
    float acc = 0;
    int x = get_global_id(0);
    int y = get_global_id(1);
    int width = get_global_size(0);
    int height = get_global_size(1);

    acc += (( ((y-1)<0) || ( (x-1)<0 )   )           ?0: map_before[(y-1) * width + (x-1)]);
    acc += (( ((y-1)<0)                  )           ?0: map_before[(y-1) * width +     x]);
    acc += (( ((y-1)<0) || ( (x+1)>=width ))         ?0: map_before[(y-1) * width + (x+1)]);

    acc +=( ( (x-1)<0 )                              ?0: map_before[(y) * width + (x-1)]);
    acc += (                                             map_before[(y) * width +     x]);
    acc +=( ( (x+1)>=width )                         ?0: map_before[(y) * width + (x+1)]);


    acc +=( ( ((y+1)>=height) || ( (x-1)<0 ))        ?0: map_before[(y+1) * width + (x-1)]);
    acc +=( ( ((y+1)>=height) )                     ?0: map_before[(y+1) * width +     x]);
    acc +=( ( ((y+1)>=height) || ( (x+1)>=width ))    ?0: map_before[(y+1) * width + (x+1)]);



//     acc += map_before[y * width + ((x-1)<0?0:(x-1))];
//     acc += map_before[y * width + x];
//     acc += map_before[y * width + ((x+1)<width?(x+1):0)];

//  acc += map_before[y * width + ((x-1)<0?0:(x-1))];
//     acc += map_before[y * width + x];
//     acc += map_before[y * width + ((x+1)<width?(x+1):0)];


//     for (int i = std::max(x-1, 0); i < std::min(x+2, width); i++)
//     {
//         for (int j = std::max(y-1, 0); j < std::min(y+2, height); j++)
//         {
//             acc += map_before[j * width + i];
//         }
//     }

    map_after[x + y * width] = acc / 9.0;
}
