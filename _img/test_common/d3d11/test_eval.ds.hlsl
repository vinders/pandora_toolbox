cbuffer MatrixBuffer
{
  matrix worldMatrix;
  matrix viewMatrix;
  matrix projectionMatrix;
};

struct ConstantOutputType
{
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

struct HullOutputType
{
  float3 position : POSITION;
  float4 color : COLOR;
};

struct PixelInputType
{
  float4 position : SV_POSITION;
  float4 color : COLOR;
};

[domain("tri")]
PixelInputType DSMain(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch)
{
  float3 vertexPosition;
  PixelInputType output;
  vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

  output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
  output.position = mul(output.position, viewMatrix);
  output.position = mul(output.position, projectionMatrix);
  output.color = patch[0].color;
  return output;
}
