#version 450 core
layout (location = 0) in vec3 aPos;
//in vec3 vertex;
//out vec3 out_vertex;
//attribute vec3 vertex;

//uniform mat4 mvp_matrix;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//attribute vec4 a_position;
//attribute vec2 a_texcoord;
//attribute vec4 gl_Vertex;

//varying vec2 v_texcoord;

void main()
{
    // Calculate vertex position in screen space
//    gl_Position = mvp_matrix * aPos;
//    gl_PointSize = 5;
//    out_vertex = vertex;
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//    gl_Position = projection * view * model * vec4(vertex, 1);
//    gl_Position =  vec4(vertex, 1);
//    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

//    gl_Position = vec4(0, 0, 0, 0);

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
//    v_texcoord = a_texcoord;
}
