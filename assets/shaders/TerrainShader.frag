precision mediump float;

varying vec3 var_normal;
varying vec2 var_uvCoord;
varying vec4 var_ambientColor;
varying vec3 var_fragPos;
varying vec3 var_camPos;

varying vec4 var_dirLightDir;
varying vec4 var_dirLightColor;


struct Material
{
    sampler2D channelTexSampler0;
    sampler2D channelTexSampler1;
    sampler2D channelTexSampler2;
    sampler2D channelTexSampler3;
    sampler2D channelTexSampler4;
    sampler2D channelTexSampler5;
    sampler2D blendmapTexSampler;
};
uniform Material material;


const float verticesPerRow = 31.0;
// Used to be next closest pow2 as the texture is constructed dynamically (atm 15 * 2 + 1)
//const float tileWidthPow2 = 32.0;

//const float textureTiling = 20.0;
void main(void)
{
    vec3 normal = normalize(var_normal);
    vec3 lightDir = normalize(var_dirLightDir.xyz);

    float diffFactor = max(dot(normal, -lightDir), 0.0);
    vec4 diffuseColor = diffFactor * var_dirLightColor;

    // NOTE: Update to below! Seems that we don't need to add any displacement to the texture after all
    // ..a bit wonky tho when moving from tile to another..
    // Need to add displacement since using vertices as tiles
    //float displacement = 1.0 / verticesPerRow * 0.125;
    vec2 u = var_uvCoord;

	vec4 blendmapColor = texture2D(material.blendmapTexSampler, u);
	float blackAmount = 1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b + blendmapColor.a);
	vec2 tiledUv = u * verticesPerRow;

    float yellowAmount = 0.0;
    if (blendmapColor.r > 0.0 && blendmapColor.g > 0.0)
    {
        if (blendmapColor.r > blendmapColor.g)
            yellowAmount = blendmapColor.g;
        else
            yellowAmount = blendmapColor.r;
    }

    float barrenTextureAmount = blackAmount;

    float waterTextureAmount = blendmapColor.r - yellowAmount;
    float rockTextureAmount = blendmapColor.g - yellowAmount;


    float fertileTextureAmount = blendmapColor.b;
    float coldTextureAmount = blendmapColor.a;

	vec4 diffuseColorBlack =		texture2D(material.channelTexSampler0, tiledUv) * barrenTextureAmount;
	vec4 diffuseColorRed =			texture2D(material.channelTexSampler1, tiledUv) * waterTextureAmount;
	vec4 diffuseColorGreen =		texture2D(material.channelTexSampler2, tiledUv) * rockTextureAmount;
	vec4 diffuseColorBlue =			texture2D(material.channelTexSampler3, tiledUv) * fertileTextureAmount;
	vec4 diffuseColorAlpha =		texture2D(material.channelTexSampler4, tiledUv) * coldTextureAmount;
    // TESTING
	vec4 diffuseColorYellow =		texture2D(material.channelTexSampler5, tiledUv) * yellowAmount;


	vec4 blendedColor = diffuseColorBlack + diffuseColorRed + diffuseColorGreen + diffuseColorBlue + diffuseColorAlpha + diffuseColorYellow;

    vec4 finalColor = (var_ambientColor + diffuseColor) * blendedColor;
    gl_FragColor = finalColor;
}
