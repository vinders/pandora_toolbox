// textured materials -- fragment shader
// -------------------------------------
// Pandora Toolbox examples by Romain Vinders

// To the extent possible under law, the person who associated CC0 with
// Pandora Toolbox examples has waived all copyright and related or neighboring rights
// to Pandora Toolbox examples.
// CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.

// Camera position
cbuffer CameraView : register(b0)
{
  matrix worldMatrix;
  matrix viewMatrix;
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
  float3 tangent : TANGENT;
  float3 bitangent : BITANGENT;
  float2 coords : TEXCOORD;
};

// ---

float4 computeDirectionalLight(float3 position, float3 normal, float3 viewDirection, float4 diffuseMap, float4 specularMap)
{
  float3 intensityVector = -directionalLight.direction.xyz;

  float4 diffuse = saturate((directionalLight.diffuse * diffuseMap) * saturate(dot(normal, intensityVector)));
  float4 ambient = directionalLight.ambient * diffuseMap;
  float spec = pow(max(dot(viewDirection, reflect(-intensityVector, normal)), 0.0), directionalLight.specular.w * specularMap.w);
  float4 specular = float4(spec * directionalLight.specular.xyz * specularMap.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 computePointLight(float3 position, float3 normal, float3 viewDirection, float4 diffuseMap, float4 specularMap)
{
  float3 intensityVector = normalize(pointLight.position.xyz - position);

  float4 diffuse = saturate((pointLight.diffuse * diffuseMap) * saturate(dot(normal, intensityVector)));
  float4 ambient = pointLight.ambient * diffuseMap;
  float spec = pow(max(dot(viewDirection, reflect(-intensityVector, normal)), 0.0), pointLight.specular.w * specularMap.w);
  float4 specular = float4(spec * pointLight.specular.xyz * specularMap.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 main(PS_INPUT input) : SV_TARGET
{
  float3 viewDirection = normalize(cameraPosition.xyz - input.position);

  // texture
  float4 diffuse = Texture.Sample(TextureSampler, input.coords);
  float4 specular = SpecularMap.Sample(TextureSampler, input.coords);

  // normal mapping
  float3 dirToLight = -(directionalLight.direction.xyz);
  float4 normalTexel = (NormalMap.Sample(TextureSampler, input.coords) * 2.0f) - 1.0f;
  float3 bumpNormal = (normalTexel.x * input.tangent) + (normalTexel.y * input.bitangent) + (normalTexel.z * input.normal);
  bumpNormal = normalize(bumpNormal);

  // lighting
  float4 directionalLight = computeDirectionalLight(input.position, bumpNormal, viewDirection, diffuse, specular);
  float4 pointLight = computePointLight(input.position, bumpNormal, viewDirection, diffuse, specular);
  return (directionalLight + pointLight);
}
