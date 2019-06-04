#version 450 core
//in vec3 in_vertex;
//out vec4 fragColor;
//uniform sampler2D texture;

//varying vec2 v_texcoord;

void main()
{
    // Set fragment color from texture
//    gl_FragColor = texture2D(texture, v_texcoord);

    vec3 color = vec3(1.0, 0.5, 0.2);

//    fragColor = vec4(color, 1.0);
    gl_FragColor = vec4(color, 0.5);
}
