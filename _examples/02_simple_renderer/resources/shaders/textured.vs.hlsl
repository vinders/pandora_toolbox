// textured materials -- vertex shader
// -----------------------------------

// World/view/projection matrices
cbuffer CameraView : register(b0)
{
  matrix modelViewMatrix;
  matrix projectionMatrix;
  float4 cameraPosition;
}

// Input data
struct VS_INPUT
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 coords : TEXCOORD;
};
// Output data
struct PS_INPUT
{
  float3 position : POSITION;
  float4 projection : SV_POSITION;
  float3 normal : NORMAL;
  float2 coords : TEXCOORD;
};

// ---

PS_INPUT main(VS_INPUT input)
{
  PS_INPUT output;
  float4 modelViewPosition = mul(modelViewMatrix, float4(input.position, 1.0));
  output.position = modelViewPosition.xyz;
  output.projection = mul(projectionMatrix, modelViewPosition);
  output.normal = input.normal;
  output.coords = input.coords;
  return output;
}
