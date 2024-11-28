precision mediump float;

attribute vec3 vertexPos;
attribute vec3 normal;
attribute vec2 uvCoord;
// Had to put some weird userVertexData to accomplish having "temperature effect"
// This needs to be between 0 and 1 and used to mix "cold" and "hot" color effects
// where towards 0 colder -> towards 1 hotter
attribute vec2 userVertexData;


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

varying float var_distToCam;


void main()
{
    vec4 worldPos = transformationMatrix * vec4(vertexPos, 1.0);
    gl_Position = common.projMat * common.viewMat * worldPos;

    var_normal = normal;

    var_uvCoord = uvCoord;
    var_ambientColor = environment.ambientColor;

    var_fragPos = worldPos.xyz;

    var_camPos = common.camPos.xyz;

    var_dirLightDir = directionalLight.direction;
    var_dirLightColor = directionalLight.color;

    var_distToCam = gl_Position.w;
}
