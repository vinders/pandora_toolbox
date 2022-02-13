// 2D/sprites -- fragment shader
// -------------------------------------
// Pandora Toolbox examples by Romain Vinders

// To the extent possible under law, the person who associated CC0 with
// Pandora Toolbox examples has waived all copyright and related or neighboring rights
// to Pandora Toolbox examples.
// CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.

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
