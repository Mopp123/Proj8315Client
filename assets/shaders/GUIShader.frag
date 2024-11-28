precision mediump float;

varying vec2 var_uvCoord;
varying vec4 var_color;
varying vec2 var_fragPos;
varying vec2 var_scale;
varying vec4 var_borderColor;
varying float var_borderThickness;

uniform sampler2D texSampler;

void main()
{
    vec4 texColor = texture2D(texSampler, var_uvCoord);
    if (texColor.a == 0.0)
        discard;

    if (var_fragPos.x >= var_scale.x - var_borderThickness || var_fragPos.x <= var_borderThickness ||
            var_fragPos.y <= -var_scale.y + var_borderThickness || var_fragPos.y >= -var_borderThickness)
    {
        gl_FragColor = var_borderColor;
    }
    else
    {
        gl_FragColor = texColor * var_color;
    }
}
