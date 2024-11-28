precision mediump float;

attribute vec3 vertexPos;
attribute vec3 normal;
attribute vec2 uvCoord;

attribute vec4 weights;
// NOTE: ivecN and int attributes aren't allowed in webgl!
attribute vec4 joints;

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

const int maxJoints = 50;
const int maxJointInfluences = 4;
uniform mat4 jointMatrices[maxJoints];

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
    float weightSum = weights[0] + weights[1] + weights[2] + weights[3];
    mat4 jointTransform = jointMatrices[0];
    if (weightSum >= 1.0)
    {
        jointTransform =  jointMatrices[int(joints[0])] * weights[0];
	      jointTransform += jointMatrices[int(joints[1])] * weights[1];
	      jointTransform += jointMatrices[int(joints[2])] * weights[2];
	      jointTransform += jointMatrices[int(joints[3])] * weights[3];
    }
    else
    {
        jointTransform = jointMatrices[int(joints[0])];
    }

    mat4 finalTransform = transformationMatrix * jointTransform;

    vec4 worldPos = finalTransform * vec4(vertexPos, 1.0);
    gl_Position = common.projMat * common.viewMat * worldPos;

    // not sure if this is fucked?
    vec4 rotatedNormal = finalTransform * vec4(normal, 0.0);
    var_normal = rotatedNormal.xyz;
    var_uvCoord = uvCoord;
    var_ambientColor = environment.ambientColor;

    var_fragPos = worldPos.xyz;

    var_camPos = common.camPos.xyz;

    var_dirLightDir = directionalLight.direction;
    var_dirLightColor = directionalLight.color;

    var_distToCam = gl_Position.w;
}
