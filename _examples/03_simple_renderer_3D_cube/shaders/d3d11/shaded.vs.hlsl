// shaded materials -- vertex shader
// -----------------------------------
// Pandora Toolbox examples by Romain Vinders

// To the extent possible under law, the person who associated CC0 with
// Pandora Toolbox examples has waived all copyright and related or neighboring rights
// to Pandora Toolbox examples.
// CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.

// World/view/projection matrices
cbuffer CameraView : register(b0)
{
  matrix worldMatrix;
  matrix viewMatrix;
  matrix projectionMatrix;
  float4 cameraPosition;
}

// Input data
struct VS_INPUT
{
  float3 position : POSITION;
  float3 normal : NORMAL;
};
// Output data
struct PS_INPUT
{
  float3 position : POSITION;
  float4 projection : SV_POSITION;
  float3 normal : NORMAL;
};

// ---

PS_INPUT main(VS_INPUT input)
{
  PS_INPUT output;
  output.projection = mul(worldMatrix, float4(input.position, 1.0));
  output.projection = mul(viewMatrix, output.projection);
  output.position = output.projection.xyz;
  output.projection = mul(projectionMatrix, output.projection);

  output.normal = mul((float3x3)worldMatrix, input.normal);
  output.normal = normalize(output.normal);
  return output;
}
