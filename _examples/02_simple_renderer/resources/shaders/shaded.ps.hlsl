// shaded materials -- fragment shader
// -------------------------------------

// Camera position
cbuffer CameraView : register(b0)
{
  matrix worldMatrix;
  matrix viewMatrix;
  matrix projectionMatrix;
  float4 cameraPosition;
}
// Active material
cbuffer MaterialBuffer : register(b1)
{
  float4 materialDiffuse;
  float4 materialAmbient;
  float3 materialSpecular;
  float  shininess;
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

// Input data
struct PS_INPUT
{
  float3 position : POSITION;
  float4 projection : SV_POSITION;
  float3 normal : NORMAL;
};

// ---

float4 computeDirectionalLight(float3 position, float3 normal, float3 viewDirection)
{
  float3 lightVector = -directionalLight.direction.xyz;

  float4 diffuse = saturate(dot(lightVector, normal)) * directionalLight.diffuse * materialDiffuse;
  float4 ambient = directionalLight.ambient * materialAmbient;
  float spec = pow(max(dot(viewDirection, reflect(-lightVector, normal)), 0.0), directionalLight.specular.w * shininess);
  float4 specular = float4(spec * directionalLight.specular.xyz * materialSpecular.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 computePointLight(float3 position, float3 normal, float3 viewDirection)
{
  float3 lightVector = normalize(pointLight.position.xyz - position);

  float4 diffuse = saturate(dot(lightVector, normal)) * pointLight.diffuse * materialDiffuse;
  float4 ambient = pointLight.ambient * materialAmbient;
  float spec = pow(max(dot(viewDirection, reflect(-lightVector, normal)), 0.0), pointLight.specular.w * shininess);
  float4 specular = float4(spec * pointLight.specular.xyz * materialSpecular.xyz, 1.0f);
  return diffuse + ambient + specular;
}

float4 main(PS_INPUT input) : SV_TARGET
{
  // lighting
  input.normal = normalize(input.normal);
  float3 viewDirection = normalize(cameraPosition.xyz - input.position);
  float4 directionalLight = computeDirectionalLight(input.position, input.normal, viewDirection);
  float4 pointLight = computePointLight(input.position, input.normal, viewDirection);

  return (directionalLight + pointLight);
}
