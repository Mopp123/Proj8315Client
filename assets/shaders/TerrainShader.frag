precision mediump float;

varying vec3 var_normal;
varying vec2 var_uvCoord;
varying vec4 var_ambientColor;
varying vec3 var_fragPos;
varying vec3 var_camPos;

varying vec4 var_dirLightDir;
varying vec4 var_dirLightColor;

varying float var_distToCam;


struct Material
{
    sampler2D channelTexSampler0;
    sampler2D channelTexSampler1;
    sampler2D channelTexSampler2;
    sampler2D channelTexSampler3;
    sampler2D channelTexSampler4;

    sampler2D blendmapTexSampler;

    sampler2D customDataTexSampler;
};
uniform Material material;


const float verticesPerRow = 31.0;
// Used to be next closest pow2 as the texture is constructed dynamically (atm 15 * 2 + 1)
//const float tileWidthPow2 = 32.0;

//const float textureTiling = 20.0;


vec4 alter_brightness_contrast(vec4 color, float brightness, float contrast)
{
    vec4 averageLuminance = vec4(1, 1, 1, 1);
    return mix(color * brightness, mix(averageLuminance, color, contrast), 0.5);
}

// For gui to properly get rendered we have to discard some fragments
// here... otherwise the 3d bleeds through gui when model's face
// "impales" the camera (clipping due to zNear)
// TODO: Some better way of dealing with this!
const float overrideZNear = 1.0;

void main(void)
{
    if (var_distToCam <= overrideZNear)
        discard;

    vec3 normal = normalize(var_normal);
    vec3 lightDir = normalize(var_dirLightDir.xyz);

    float diffFactor = max(dot(normal, -lightDir), 0.0);
    vec4 diffuseColor = diffFactor * var_dirLightColor;

    // NOTE: Update to below! Seems that we don't need to add any displacement to the texture after all
    // ..a bit wonky tho when moving from tile to another..
    // Need to add displacement since using vertices as tiles
    //float displacement = 1.0 / verticesPerRow * 0.125;
    vec2 u = var_uvCoord;

    // moves on oppo dir when divide
    // moves on move dir when mul

	vec4 blendmapColor = texture2D(material.blendmapTexSampler, u);
	float blackAmount = max(1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b + blendmapColor.a), 0.0);

	vec2 tiledUv = u * verticesPerRow;
	vec2 tiledDuneUv = u * 12.0; // make look dunes bigger... TODO: better dune texture

	vec4 diffuseColorBlack =	texture2D(material.channelTexSampler0, tiledUv) * blackAmount;
	vec4 diffuseColorRed =		texture2D(material.channelTexSampler1, tiledUv) * blendmapColor.r;
	vec4 diffuseColorGreen =	texture2D(material.channelTexSampler2, tiledUv) * blendmapColor.g;
	vec4 diffuseColorBlue =		texture2D(material.channelTexSampler3, tiledUv) * blendmapColor.b;
	vec4 diffuseColorAlpha =	texture2D(material.channelTexSampler4, tiledDuneUv) * blendmapColor.a;



    // Test temperature effect
    vec4 coldColor = vec4(0.1, 0.2, 0.55, 1.0);
    vec4 hotColor = vec4(0.70, 0.36, 0.0, 1.0);

    vec4 customDataBuffer = texture2D(material.customDataTexSampler, var_uvCoord);

    float temperatureMultiplier = mod(customDataBuffer.a, 0.5) * 2.0;
    if (customDataBuffer.a < 0.5)
        temperatureMultiplier = 1.0 - temperatureMultiplier;

    float coldnessVal = 1.0 - customDataBuffer.a;
    float hotnessVal = customDataBuffer.a;

    vec4 temperatureColor = (hotColor * hotnessVal + coldColor * coldnessVal) * temperatureMultiplier;

    // test some special cases...
    // Make water discard the temperature color
    // TODO: Make water look frozen in cold temperature
    if (blendmapColor.r > 0.5)
        temperatureColor = temperatureColor * (1.0 - blendmapColor.r);

	vec4 blendedColor = diffuseColorBlack + diffuseColorRed + diffuseColorGreen + diffuseColorBlue + diffuseColorAlpha;
    gl_FragColor = (var_ambientColor + diffuseColor) * (blendedColor + temperatureColor);
}
