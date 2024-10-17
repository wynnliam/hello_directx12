// Liam Wynn, 9/27/2024, Hello DirectX 12

/*
    A very simple texture shader.

    TODO:
    1. Supply an MVP matrix
*/

struct vertex_pos_uv
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

struct model_view_projection
{
    matrix mvp;
};

// t registers are for shader resource views. So we are putting
// my_texture in register t0, the first. Not explictly written
// is that our texture is in register space 0. I think this
// can be specified as register(t0, space0). But not sure.
Texture2D my_texture : register(t0);
// s registers are for samplers. In our case, we put
// this generic sampler in the 0th register of the 0th
// space.
SamplerState my_sampler : register(s0);
// b registers are for constant buffers. Constant buffers
// are for anything that is constant across all the threads.
// So things like an MVP matrix can be placed in a cbuffer
ConstantBuffer<model_view_projection> my_mvp : register(b0);

vertex_pos_uv vs_main(float3 position : POSITION, float2 uv : TEXCOORD)
{
    vertex_pos_uv result;
    
    result.position = mul(
        my_mvp.mvp,
        float4(position, 1.0f)
    );
    
    result.uv = uv;
    
    return result;
}

float4 ps_main(vertex_pos_uv input) : SV_Target
{
    return my_texture.Sample(my_sampler, input.uv);

}
