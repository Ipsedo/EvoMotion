#version 150

precision mediump float;

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;

uniform float u_distance_coef;
uniform float u_light_coef;

uniform vec4 u_ambient_color;
uniform vec4 u_diffuse_color;
uniform vec4 u_specular_color;
uniform float u_shininess;

varying vec3 v_position;
varying vec3 v_normal;


void main(){
    float distance = length(u_light_pos - v_position);
    vec3 lightVector = normalize(u_light_pos - v_position);

    float diffuse_coeff = max(dot(v_normal, lightVector), 0.1) * u_light_coef;
    diffuse_coeff = diffuse_coeff * (1.0 / (1.0 + (u_distance_coef * distance * distance)));

    vec4 diffuse = diffuse_coeff * u_diffuse_color;

    float specularCoefficient = 0.0;
    if(diffuse_coeff > 0.0){
        vec3 incidenceVector = -lightVector;
        vec3 reflectionVector = reflect(incidenceVector, v_normal);
        vec3 surfaceToCamera = normalize(u_cam_pos - v_position);
        float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
        specularCoefficient = pow(cosAngle, u_shininess) * u_light_coef;
    }

    vec4 specular = specularCoefficient * u_specular_color;

    vec4 ambient = 0.1 * u_ambient_color;

    gl_FragColor = ambient + diffuse + specular;
}
