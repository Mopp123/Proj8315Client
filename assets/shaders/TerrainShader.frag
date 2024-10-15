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

    // Need to add displacement since using vertices as tiles
    float displacement = 1.0 / verticesPerRow; //* 0.5;
    vec2 u = var_uvCoord + displacement;

	vec4 blendmapColor = texture2D(material.blendmapTexSampler, u);
	float blackAmount = 1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b + blendmapColor.a);
	vec2 tiledUv = u * verticesPerRow;

	vec4 diffuseColorBlack =		texture2D(material.channelTexSampler0, tiledUv) * blackAmount;
	vec4 diffuseColorRed =			texture2D(material.channelTexSampler1, tiledUv) * blendmapColor.r;
	vec4 diffuseColorGreen =		texture2D(material.channelTexSampler2, tiledUv) * blendmapColor.g;
	vec4 diffuseColorBlue =			texture2D(material.channelTexSampler3, tiledUv) * blendmapColor.b;
	vec4 diffuseColorAlpha =		texture2D(material.channelTexSampler4, tiledUv) * blendmapColor.a;

	vec4 blendedColor = diffuseColorBlack + diffuseColorRed + diffuseColorGreen + diffuseColorBlue + diffuseColorAlpha;

    vec4 finalColor = (var_ambientColor + diffuseColor) * blendedColor;
    gl_FragColor = finalColor;
}
