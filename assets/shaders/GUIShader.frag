precision mediump float;

#define FILTER_TYPE_NONE 0.0
#define FILTER_TYPE_EMBOSS 1.0
#define FILTER_TYPE_ENGRAVE 2.0

varying vec2 var_uvCoord;
varying vec4 var_color;
varying vec2 var_fragPos;
varying vec2 var_scale;
varying float var_filter;

uniform sampler2D texSampler;

const float embossBorderThickness = 2.0;
const float embossBorderBrightness = 1.5;

void main()
{
    vec4 texColor = texture2D(texSampler, var_uvCoord);
    if (texColor.a == 0.0)
        discard;

    vec4 finalColor = gl_FragColor = texColor * var_color;

    bool emboss = bool(var_filter == FILTER_TYPE_EMBOSS);
    bool engrave = bool(var_filter == FILTER_TYPE_ENGRAVE);

    if (emboss || engrave)
    {
        // left and bottom shadow
        if (var_fragPos.x >= var_scale.x - embossBorderThickness || var_fragPos.y <= -var_scale.y + embossBorderThickness)
        {
            if (emboss)
                finalColor = vec4(0, 0, 0, 1);
            else
                finalColor = finalColor * embossBorderBrightness;
        }
        // left and top bright
        else if (var_fragPos.x <= embossBorderThickness || var_fragPos.y >= -embossBorderThickness)
        {
            if (emboss)
                finalColor = finalColor * embossBorderBrightness;
            else
                finalColor = vec4(0, 0, 0, 1);
        }
    }
    gl_FragColor = finalColor;
}
