precision mediump float;

attribute vec2 vertexPos;
attribute vec2 uvCoord;

attribute vec2 pos;
attribute vec2 scale;
attribute vec4 color;
attribute vec4 borderColor;
attribute float borderThickness;

struct Common
{
    mat4 projMat;
};

uniform Common common;

varying vec2 var_uvCoord;
varying vec4 var_color;
varying vec2 var_fragPos;
varying vec2 var_scale;
varying vec4 var_borderColor;
varying float var_borderThickness;

void main()
{
    gl_Position = common.projMat * vec4((vertexPos * scale) + pos, 0, 1.0);
    var_uvCoord = uvCoord;

    var_color = color;
    vec4 transformedPos = vec4((vertexPos * scale), 0, 1.0);
    var_fragPos = transformedPos.xy;
    var_scale = scale;
    var_borderColor = borderColor;
    var_borderThickness = borderThickness;
}
