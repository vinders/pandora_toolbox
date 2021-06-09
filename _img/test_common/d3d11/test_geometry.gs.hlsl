struct GSOutput
{
    float4 pos : SV_POSITION;
};

[maxvertexcount(4)]
void GSMain( point float4 input[1] : SV_POSITION, inout TriangleStream<GSOutput> OutputStream )
{
  GSOutput gsout;
  gsout.pos = float4(input[0].x + 0.5, input[0].y + 0.5, input[0].z, input[0].w);
  OutputStream.Append(gsout);
  gsout.pos = float4(input[0].x - 0.5, input[0].y + 0.5, input[0].z, input[0].w);
  OutputStream.Append(gsout); 
  gsout.pos = float4(input[0].x - 0.5, input[0].y - 0.5, input[0].z, input[0].w);
  OutputStream.Append(gsout); 
  gsout.pos = float4(input[0].x + 0.5, input[0].y - 0.5, input[0].z, input[0].w);
  OutputStream.Append(gsout); 
}
