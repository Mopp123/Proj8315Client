precision mediump float;

attribute vec3 vertexPos;
attribute vec3 normal;
attribute vec2 uvCoord;

struct Common3D
{
    mat4 projMat;
    mat4 viewMat;
    vec4 camPos;
};
uniform Common3D common;

struct Environment
{
    vec4 ambientColor;
};
uniform Environment environment;

struct DirectionalLight
{
    vec4 direction;
    vec4 color;
};
uniform DirectionalLight directionalLight;

// as push constant here
uniform mat4 transformationMatrix;

varying vec3 var_normal;
varying vec2 var_uvCoord;
varying vec4 var_ambientColor;
varying vec3 var_fragPos;
varying vec3 var_camPos;

varying vec4 var_dirLightDir;
varying vec4 var_dirLightColor;

// TODO: Make this uniform
const float tileWidthPow2 = 32.0; // next closest pow2 as the texture is constructed dynamically (atm 15 * 2 + 1)
varying float var_tileWidthPow2;


void main()
{
    vec4 worldPos = transformationMatrix * vec4(vertexPos, 1.0);
    gl_Position = common.projMat * common.viewMat * worldPos;

    var_normal = normal;

    // Need to add displacement since using vertices as tiles
    float displacement = 1.0 / tileWidthPow2 * 0.5;
    var_uvCoord = uvCoord + vec2(displacement, displacement);
    var_tileWidthPow2 = tileWidthPow2;
    var_ambientColor = environment.ambientColor;

    var_fragPos = worldPos.xyz;

    var_camPos = common.camPos.xyz;

    var_dirLightDir = directionalLight.direction;
    var_dirLightColor = directionalLight.color;
}
