#version 330 core
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 vCameraPos;

in vec4 LightSpacePos;

out vec4 FragColor;

uniform sampler2D DepthMap;


struct Material
{
    vec3 color;
    float shininess;
};

uniform Material material;

struct Light
{
    float intensity;
    
    vec3 pos;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    //Light attenuation
	float kc;
	float kl;
	float kq;

    float lightWidth;
};

uniform Light light;


#define EPS 1e-4
#define PI 3.141592653589793
#define PI2 6.283185307179586


highp float rand_1to1(highp float x)
{
    // [0, 1]
    return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv)
{
    // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
	return fract(sin(sn) * c);
}


#define NUM_RINGS 10
#define NUM_SAMPLES 20
vec2 sampleDisk[NUM_SAMPLES];
void getPoissonSamples( const in vec2 randomSeed )
{
    const float angleStep = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
    const float invNumSamples = 1.0 / float(NUM_SAMPLES);

    float angle = rand_2to1( randomSeed ) * PI2;
    float radius = invNumSamples;
    float radiusStep = radius;

    for( int i = 0; i < NUM_SAMPLES; i ++ )
    {
        sampleDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += angleStep;
    }
}
void getUniformSamples(const in vec2 randomSeed) // sample points in a circle
{
    float randNum = rand_2to1(randomSeed);
    float sampleX = rand_1to1(randNum);
    float sampleY = rand_1to1(sampleX);

    float angle = sampleX * PI2; // angle ~ [0, 2 * pi]
    float radius = sqrt(sampleY); // radius ^ 2 ~ [0, 1]

    for(int i = 0; i < NUM_SAMPLES; ++i)
    {
        sampleDisk[i] = vec2(radius * cos(angle), radius * sin(angle));

        // fresh the random nums
        sampleX = rand_1to1(sampleY);
        sampleY = rand_1to1(sampleX);

        angle = sampleX * PI2;
        radius = sqrt(sampleY);
    }
}


float findBlocker(vec2 uv, float zReceiver)
{
    // get a serials of random points
    getPoissonSamples(uv);


    const float radius = 20.0;
    const float pixelSize = 1.0 / 1024.0;


    // to avoid peter pan
    const float bias = 0.005;


    // calculate the average depth of the place around the target uv, only use the uv in the shadow
    float mean_block_depth = 0.0;
    int block_count = 0;
    int isBlocked = 0;
    for(int i = 0; i < NUM_SAMPLES; ++i)
    {
        vec2 this_uv = sampleDisk[i] * radius * pixelSize + uv;
        float zClosest = texture2D(DepthMap, this_uv).r;

        // if is blocked, use it
        if(zClosest + bias < zReceiver)
        {
            mean_block_depth += zClosest;
            ++block_count;
            isBlocked = 1;
        }
    }

    if(isBlocked == 1) return mean_block_depth / float(block_count);
    else return 1.0;
}

float getPenumbraSize(float dReceiver, float dBlocker)
{
    return light.lightWidth * max((dReceiver - dBlocker), 0.0) / dBlocker;
}


float PCF(vec3 texCoordShadow, float penumbraSize)
{
    // get a serials of random points
    getUniformSamples(texCoordShadow.yx);

    // to avoid peter pan
    const float bias = 0.005;


    const float pixelSize = 1.0 / 1024.0;


    float visibility = 0.0;
    for(int i = 0; i < NUM_SAMPLES; ++i)
    {
        vec2 this_uv = sampleDisk[i] * penumbraSize * pixelSize + texCoordShadow.xy;
        float zClosest = texture2D(DepthMap, this_uv).r;
        
        // if the pixel is not behind the other model, which means that the pixel's depth
        // in light space needs to larger than the depth in depth map(DM).
        // also, to avoid peter pan, the pixel's depth should be abviously largert than
        // the depth in DM, or in other world, zReceiver > zClosest + bias
        visibility += (zClosest + bias < texCoordShadow.z) ? 0.0 : 1.0;
    }
    return visibility / float(NUM_SAMPLES);

}


float getVisibilityPCSS(vec3 texCoordShadow)
{
//    return (texture2D(DepthMap, texCoordShadow.xy).r + EPS < texCoordShadow.z) ? 0.0 : 1.0;

    if(texCoordShadow.z > 1.0 || texCoordShadow.z < 0.0) return 1.0;

    float mean_block_depth = findBlocker(texCoordShadow.xy, texCoordShadow.z);
    float penumbraSize = getPenumbraSize(texCoordShadow.z, mean_block_depth);
    return PCF(texCoordShadow, penumbraSize);
}


float getBlinnPhongAttenuation()
{
	float distance = length(light.pos - vPosition); //light length
	return 1.0f / (light.kc + light.kl * distance + light.kq * distance * distance);
}

vec3 getBlinnPhongAmbient()
{
	vec3 ambient = material.color * light.color * light.ambient;
	return light.intensity * ambient;
}

vec3 getBlinnPhongDiffuse()
{
	vec3 lightDir = normalize(light.pos - vPosition);
	float diff = max(dot(lightDir, vNormal), 0.0f);
	vec3 diffuse = material.color * light.color * light.diffuse * diff;
	return light.intensity * diffuse;
}

vec3 getBlinnPhongSpecular()
{
    vec3 lightDir = normalize(light.pos - vPosition);
	vec3 viewDir = normalize(vCameraPos - vPosition);
	vec3 halfwayDir = normalize(lightDir + vCameraPos);
	float spec = pow(max(dot(halfwayDir, vNormal), 0.0f), material.shininess);
	vec3 specular = material.color * light.color * light.specular * spec;
    return light.intensity * specular;
}


void main()
{
    float attenuation = getBlinnPhongAttenuation();
    vec3 ambient = getBlinnPhongAmbient();
    vec3 diffuse = getBlinnPhongDiffuse();
    vec3 specular = getBlinnPhongSpecular();


	vec3 shadowMapTexCoord = LightSpacePos.xyz / LightSpacePos.w;
	shadowMapTexCoord = shadowMapTexCoord * 0.5 + 0.5;
    float visibility = getVisibilityPCSS(shadowMapTexCoord);


    vec3 lighting = (ambient + visibility * (diffuse + specular)) * attenuation;


	FragColor = vec4(vec3(lighting), 1.0);
}