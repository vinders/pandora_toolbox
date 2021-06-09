struct ComputeBufferType
{
  int i;
  float f;
};

StructuredBuffer<ComputeBufferType> Buffer0 : register(t0);
StructuredBuffer<ComputeBufferType> Buffer1 : register(t1);
RWStructuredBuffer<ComputeBufferType> BufferOut : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 threadId : SV_DispatchThreadID )
{
  BufferOut[threadId.x].i = Buffer0[threadId.x].i + Buffer1[threadId.x].i;
  BufferOut[threadId.x].f = Buffer0[threadId.x].f + Buffer1[threadId.x].f;
}
