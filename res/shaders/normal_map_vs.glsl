uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;

attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoord;

varying vec3 v_Position;
varying vec2 v_TexCoord;
varying vec3 v_Normal;

void main() {
    v_Position = vec3(u_MVMatrix * vec4(a_Position, 1.0));
    v_TexCoord = a_TexCoord;
    v_Normal = normalize(vec3(u_MVMatrix * vec4(a_Normal, 0.0)));

    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
