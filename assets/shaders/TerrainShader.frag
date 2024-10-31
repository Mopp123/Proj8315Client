precision mediump float;

varying vec3 var_normal;
varying vec2 var_uvCoord;
varying vec4 var_ambientColor;
varying vec3 var_fragPos;
varying vec3 var_camPos;

varying vec4 var_dirLightDir;
varying vec4 var_dirLightColor;

varying vec2 var_userVertexData;


struct Material
{
    sampler2D channelTexSampler0;
    sampler2D channelTexSampler1;
    sampler2D channelTexSampler2;
    sampler2D channelTexSampler3;
    sampler2D channelTexSampler4;

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

    // quickly fix "dune" texture -> looks too small with regular tiling
    vec2 duneTiledUv = u * (verticesPerRow * 0.5);

	vec4 diffuseColorBlack =	texture2D(material.channelTexSampler0, tiledUv) * blackAmount;
	vec4 diffuseColorRed =		texture2D(material.channelTexSampler1, tiledUv) * blendmapColor.r;
	vec4 diffuseColorGreen =	texture2D(material.channelTexSampler2, tiledUv) * blendmapColor.g;
	vec4 diffuseColorBlue =		texture2D(material.channelTexSampler3, tiledUv) * blendmapColor.b;
	vec4 diffuseColorAlpha =	texture2D(material.channelTexSampler4, duneTiledUv) * blendmapColor.a;

	vec4 blendedColor = diffuseColorBlack + diffuseColorRed + diffuseColorGreen + diffuseColorBlue + diffuseColorAlpha;


    // atm just testing userVertexData
    vec4 coldColor = vec4(0.75, 0.75, 0.85, 1.0);
    vec4 mildColor = vec4(0, 0, 0, 0);
    vec4 hotColor = vec4(0.75, 0.65, 0.0, 1.0);

    vec4 mixColor = mildColor;

    float m = 0.0;

    if (var_userVertexData.x < 0.5)
    {
        m = 1.0 - mod(var_userVertexData.x, 0.5);
        mixColor = coldColor;
    }
    else if (var_userVertexData.x >= 0.5)
    {
        m = mod(var_userVertexData.x, 0.5);
        mixColor = hotColor;
    }
    blendedColor = mix(blendedColor, mixColor, 0.04);

    vec4 finalColor = (var_ambientColor + diffuseColor) * blendedColor;
    gl_FragColor = finalColor;
}
