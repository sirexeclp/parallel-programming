kernel void sum(global long *input,
    global long *output)
{
    size_t i = get_global_id(0);
    output[i] = input[(i*2)] + input[(i*2)+1];
}