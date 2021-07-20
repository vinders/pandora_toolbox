// 2D/sprites -- fragment shader
// -------------------------------------

// Input data
struct PS_INPUT
{
  float4 position : SV_POSITION;
  float2 coords : TEXCOORD;
};

SamplerState TextureSampler : register(s0);
Texture2D Sprite : register(t0);

// ---

float4 main(PS_INPUT input) : SV_TARGET
{
  return Sprite.Sample(TextureSampler, input.coords);
}
