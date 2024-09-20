precision mediump float;

varying vec2 var_uvCoord;
varying vec4 var_color;

uniform sampler2D texSampler;

void main()
{
    vec4 texColor = texture2D(texSampler, var_uvCoord);
    vec4 finalColor = var_color * texColor.a * 1.0; // The last * 1.0 is thickness..

    if(texColor.a <= 0.0)
        discard;

    //gl_FragColor = vec4(finalColor.rgb, 1.0);
    gl_FragColor = vec4(var_color.rgb, texColor.a);
}
