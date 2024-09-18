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
    sampler2D channelTexSampler3; // TODO: channel texture for alpha
    sampler2D blendmapTexSampler;
};
uniform Material material;


const float textureTiling = 20.0;
void main(void)
{
    vec3 normal = normalize(var_normal);
    vec3 lightDir = normalize(var_dirLightDir.xyz);

    float diffFactor = max(dot(normal, -lightDir), 0.0);
    vec4 diffuseColor = diffFactor * var_dirLightColor;

	vec4 blendmapColor = texture2D(material.blendmapTexSampler, var_uvCoord);
	float blackAmount = 1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b);
	vec2 tiledUv = var_uvCoord * textureTiling;

	vec4 diffuseColorBlack =		texture2D(material.channelTexSampler0, tiledUv) * blackAmount;
	vec4 diffuseColorRed =			texture2D(material.channelTexSampler1, tiledUv) * blendmapColor.r;
	vec4 diffuseColorGreen =		texture2D(material.channelTexSampler2, tiledUv) * blendmapColor.g;
	vec4 diffuseColorBlue =			texture2D(material.channelTexSampler3, tiledUv) * blendmapColor.b;

	vec4 blendedColor = diffuseColorBlack + diffuseColorRed + diffuseColorGreen + diffuseColorBlue;

    vec4 finalColor = (var_ambientColor + diffuseColor) * blendedColor;
    gl_FragColor = finalColor;
}
