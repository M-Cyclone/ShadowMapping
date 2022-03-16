#version 330 core
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 vCameraPos;

in vec4 LightSpacePos;

out vec4 FragColor;

uniform sampler2D DepthMap;

struct Light
{    
    vec3 pos;
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
    getUniformSamples(uv);


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
        if(zClosest < zReceiver + bias)
        {
            mean_block_depth += zClosest;
            ++block_count;
            isBlocked = 1;
        }
    }

    return mean_block_depth / float(block_count);
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
        visibility += (zClosest + bias > texCoordShadow.z) ? 1.0 : 0.0;
    }
    return visibility / float(NUM_SAMPLES);

}


float getVisibilityPCSS(vec3 texCoordShadow)
{
    float mean_block_depth = findBlocker(texCoordShadow.xy, texCoordShadow.z);
    float penumbraSize = getPenumbraSize(texCoordShadow.z, mean_block_depth);
    return PCF(texCoordShadow, penumbraSize);
}


void main()
{
	vec3 shadowMapTexCoord = LightSpacePos.xyz / LightSpacePos.w;
	shadowMapTexCoord = shadowMapTexCoord * 0.5 + 0.5;
    //float visibility = PCF(shadowMapTexCoord, 20);
    float visibility = getVisibilityPCSS(shadowMapTexCoord);

	FragColor = vec4(vec3(visibility), 1.0);
}