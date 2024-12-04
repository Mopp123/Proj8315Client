precision mediump float;

attribute vec2 vertexPos;
attribute vec2 uvCoord;

attribute vec3 pos;
attribute vec2 scale;
attribute vec4 color;
attribute float filter;
attribute vec4 textureCropping;

struct Common
{
    mat4 projMat;
};

uniform Common common;

varying vec2 var_uvCoord;
varying vec4 var_color;
varying vec2 var_fragPos;
varying vec2 var_scale;
varying float var_filter;

void main()
{
    vec2 scaledVertex = vertexPos * scale;
    vec3 positionedVertex = vec3(scaledVertex, 0.0) + pos;
    gl_Position = common.projMat * vec4(positionedVertex, 1.0);

    vec2 croppingPos = textureCropping.xy;
    vec2 croppingScale = textureCropping.zw;
    var_uvCoord = (uvCoord + croppingPos) * croppingScale;

    var_color = color;
    vec4 transformedPos = vec4((vertexPos * scale), 0, 1.0);
    var_fragPos = transformedPos.xy;
    var_scale = scale;
    var_filter = filter;
}
