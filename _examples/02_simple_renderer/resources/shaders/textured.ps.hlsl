// textured materials -- fragment shader
// -------------------------------------

// Camera position
cbuffer CameraView : register(b0)
{
  matrix modelViewMatrix;
  matrix projectionMatrix;
  float4 cameraPosition;
}

// Scene lighting
struct PointLight {
  float4 diffuse;
  float4 ambient;
  float4 specular;
  float4 position;
};
struct DirectionalLight {
  float4 diffuse;
  float4 ambient;
  float4 specular;
  float4 direction;
};
cbuffer LightBuffer : register(b2)
{
  PointLight pointLight;
  DirectionalLight directionalLight;
}

SamplerState TextureSampler : register(s0); // trilinear / anisotropic sampler
Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);

// Input data
struct PS_INPUT
{
  float3 position : POSITION;
  float4 projection : SV_POSITION;
  float3 normal : NORMAL;
  float2 coords : TEXCOORD;
};

// ---

float4 computeDirectionalLight(float3 position, float3 normal, float3 viewDirection, float4 diffuseMap, float4 specularMap)
{
  float3 lightVector = -directionalLight.direction.xyz;

  float4 diffuse = saturate(dot(lightVector, normal)) * directionalLight.diffuse * diffuseMap;
  float4 ambient = directionalLight.ambient * diffuseMap;
  float spec = pow(max(dot(viewDirection, reflect(-lightVector, normal)), 0.0), directionalLight.specular.w * specularMap.w);
  float4 specular = float4(spec * directionalLight.specular.xyz * specularMap.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 computePointLight(float3 position, float3 normal, float3 viewDirection, float4 diffuseMap, float4 specularMap)
{
  float3 lightVector = normalize(pointLight.position.xyz - position);

  float4 diffuse = saturate(dot(lightVector, normal)) * pointLight.diffuse * diffuseMap;
  float4 ambient = pointLight.ambient * diffuseMap;
  float spec = pow(max(dot(viewDirection, reflect(-lightVector, normal)), 0.0), pointLight.specular.w * specularMap.w);
  float4 specular = float4(spec * pointLight.specular.xyz * specularMap.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 main(PS_INPUT input) : SV_TARGET
{
  float3 viewDirection = normalize(cameraPosition.xyz - input.position);

  // texture
  float4 diffuse = Texture.Sample(TextureSampler, input.coords);
  float4 specular = SpecularMap.Sample(TextureSampler, input.coords);

  input.normal = normalize(input.normal);
  //float3 normal = NormalMap.Sample(TextureSampler, input.coords).xyz;
  //normal = normalize(normal * 2.0 - 1.0); // to range [-1;1]

  // lighting
  float4 directionalLight = computeDirectionalLight(input.position, input.normal, viewDirection, diffuse, specular);
  float4 pointLight = computePointLight(input.position, input.normal, viewDirection, diffuse, specular);
  return (directionalLight + pointLight);
}
