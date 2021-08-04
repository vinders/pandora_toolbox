// 2D/sprites -- vertex shader
// -----------------------------------
// Pandora Toolbox examples by Romain Vinders

// To the extent possible under law, the person who associated CC0 with
// Pandora Toolbox examples has waived all copyright and related or neighboring rights
// to Pandora Toolbox examples.
// CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.

// Input data
struct VS_INPUT
{
  float2 position : POSITION;
  float2 coords : TEXCOORD;
};
// Output data
struct PS_INPUT
{
  float4 position : SV_POSITION;
  float2 coords : TEXCOORD;
};

// ---

PS_INPUT main(VS_INPUT input)
{
  PS_INPUT output;
  output.position = float4(input.position.x, input.position.y, 0.0, 1.0);
  output.coords = input.coords;
  return output;
}
