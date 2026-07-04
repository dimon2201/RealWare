const float kPi = 3.14159265f;
const float kEpsilon = 0.0001f;
const float kRoughnessEpsilon = 0.04f;

float D_GGX(vec3 N, vec3 H, float k)
{
	const float a = k * k;
	const float a2 = a * a;
	const float nh_pow2 = pow(max(dot(N, H), 0.0f), 2.0f);
	const float a2sub1 = a2 - 1.0f;
	const float nh2_mul_a2sub1 = nh_pow2 * a2sub1;
	const float denom = max(kPi * pow(nh2_mul_a2sub1 + 1.0f, 2.0f), kEpsilon);
	const float ggx = a2 / denom;

	return ggx;
}

float G_Smith_G1(float NdotV, float k)
{
	const float w = pow(k + 1.0f, 2.0f) / 8.0f;
	const float nx = max(NdotV, 0.0f);
	const float one_sub_w = 1.0f - w;
	const float g1 = nx / (nx * one_sub_w + w);

	return g1;
}

float G_Smith(vec3 N, vec3 V, vec3 L, float k)
{
	const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
	const float smith = G_Smith_G1(NdotV, k) * G_Smith_G1(NdotL, k);

	return smith;
}

vec3 F_Schlick(vec3 V, vec3 H, vec3 F0)
{
	const vec3 one_sub_f0 = 1.0f - F0;
	const float one_sub_vh_pow5 = pow(1.0f - max(dot(V, H), 0.0f), 5.0f);
	const vec3 fresnel = F0 + one_sub_f0 * one_sub_vh_pow5;

	return fresnel;
}

vec3 PBR(
	in vec3 diffuseColor,
	in float lightRadiance,
	in float roughness,
	in float metallic,
	in vec3 normalWorldSpace,
	in vec3 viewDirWorldSpace,
	in vec3 lightDirWorldSpace
)
{
	const float correctedRoughness = max(roughness, kRoughnessEpsilon);

	const vec3 halfWorldSpace = normalize(viewDirWorldSpace + lightDirWorldSpace);

	const vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
	const float D = D_GGX(normalWorldSpace, halfWorldSpace, correctedRoughness);
	const float G = G_Smith(normalWorldSpace, viewDirWorldSpace, lightDirWorldSpace, correctedRoughness);
	const vec3 F = F_Schlick(viewDirWorldSpace, halfWorldSpace, F0);
	
	const vec3 DGF = D * G * F;
	const float NdotL = max(dot(normalWorldSpace, lightDirWorldSpace),0.0);
	const float NdotV = max(dot(normalWorldSpace, viewDirWorldSpace),0.0);
	const float BRDF_a = max(4.0 * NdotL * NdotV, kEpsilon);
	const vec3 BRDF = DGF / BRDF_a;

	const vec3 kD = (1.0f - F) * (1.0f - metallic);
	const vec3 PBR_a = kD * (diffuseColor / kPi);
	const vec3 PBR = PBR_a + BRDF;

	const vec3 color = PBR * lightRadiance * NdotL;

	return color;
}