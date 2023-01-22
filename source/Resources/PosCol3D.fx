// -----------------------------------------------------
// Global variables
// -----------------------------------------------------

float4x4 gWorldViewProj     : WorldViewProjection;
Texture2D gDiffuseMap       : DiffuseMap;
Texture2D gNormalMap        : NormalMap;
Texture2D gSpecularMap      : SpecularMap;
Texture2D gGlossinessMap    : GlossinessMap;
float4x4 gWorldMatrix       : World;
float4x4 gViewInv           : ViewInverse;


float3 gLightDir = normalize(float3(0.577f, -0.577f, 0.577f));
float gLightIntensity = 7.0f;
float3 gAmbient = float3(0.025f, 0.025f, 0.025f);
float gShininess = 25.0f;
float gPI = 3.14159f;

// -----------------------------------------------------
// 
// 
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};


// -----------------------------------------------------
// Input/Output structs
// -----------------------------------------------------
struct VS_INPUT
{
    float3 Position :   POSITION;
    float2 Uv       :   TEXCOORD;
    float3 Normal   :   NORMAL;
    float3 Tangent  :   TANGENT;
};

struct VS_OUTPUT
{
    float4 Position :   SV_POSITION;
    float4 WorldPos :   COLOR;
    float2 Uv       :   TEXCOORD;
    float3 Normal   :   NORMAL;
    float3 Tangent  :   TANGENT;
};



// -----------------------------------------------------
// Vertex Shader
// -----------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position,1.f), gWorldViewProj);
    output.WorldPos = mul(output.Position, gWorldMatrix);
    output.Uv = input.Uv;
    output.Normal = mul(normalize(input.Normal),(float3x3)gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent),(float3x3)gWorldMatrix);

    return output;
}

// -----------------------------------------------------
// Functions
// -----------------------------------------------------
float4 Lambert(float kd, float4 cd)
{
    return kd * cd / gPI;
}


float Phong(float ks, float exp, float3 l, float3 v, float3 n)
{
    float theta =  max(dot(n,l),0.f);
    float3 r = 2.f * theta * n - l ;
    float alpha =  max(dot(r,v),0.f);


    return  ks * pow(alpha, exp);
}
// -----------------------------------------------------
// Pixel Shaders
// -----------------------------------------------------
float4 PS(VS_OUTPUT input, SamplerState state) : SV_TARGET
{
    float3      binormal = normalize(cross(input.Normal,input.Tangent));
    float4x4    tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    
    float3      normalSample = 2.0f * gNormalMap.Sample(state, input.Uv).rgb - float3(1.0f, 1.0f, 1.0f);
    normalSample = mul(normalize(float4(normalSample, 0.0f)), tangentSpaceAxis);

    float lambertCosine = saturate(dot(normalSample, gLightDir));
    float3 viewDirection = normalize(input.WorldPos.xyz - gViewInv[3].xyz);

    float3 diffuse = Lambert(gLightIntensity, gDiffuseMap.Sample(state, input.Uv));
    float3 specular = gSpecularMap.Sample(state, input.Uv) * Phong(1.f, gShininess * gGlossinessMap.Sample(state, input.Uv).r, gLightDir, viewDirection, normalSample);

    return float4((diffuse + specular + gAmbient) * lambertCosine,1.0f);
    
}

float4 PS_samPoint(VS_OUTPUT input) : SV_TARGET
{
    return PS(input,samPoint);
}

float4 PS_samLinear(VS_OUTPUT input) : SV_TARGET
{
    return PS(input,samLinear);
}

float4 PS_samAnisotropic(VS_OUTPUT input) : SV_TARGET
{
    return PS(input,samAnisotropic);
}

// -----------------------------------------------------
// Technique 
// -----------------------------------------------------
technique11 PointFilterTechnique
{
    pass P0
    {
        SetVertexShader( CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_samPoint()));
    }

}

technique11 LinearFilterTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_samLinear()));
    }
}

technique11 AnisotropicFilterTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_samAnisotropic()));
       
    }
}