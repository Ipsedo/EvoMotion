precision mediump float;

uniform sampler2D u_tex;
uniform sampler2D u_normalMap;

uniform vec3 u_LightPos;
uniform vec3 u_cam_pos;
uniform float u_distance_coef;
uniform float u_light_coef;

varying vec3 v_Position;
varying vec2 v_TexCoord;
varying vec3 v_Normal;

// https://geeks3d.developpez.com/normal-mapping-glsl/
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // récupère les vecteurs du triangle composant le pixel
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // résout le système linéaire
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construit une trame invariante à l'échelle
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 get_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // N, la normale interpolée et
    // V, le vecteur vue (vertex dirigé vers l'œil)
    vec3 map = texture2D(u_normalMap, texcoord ).rgb;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}

void main() {
    vec3 normalColor = texture2D(u_normalMap, v_TexCoord).rgb;
    vec3 texColor = texture2D(u_tex, v_TexCoord).rgb;

    vec3 lightVector = u_LightPos - v_Position;
    vec3 eyeVec = u_cam_pos - v_Position;

    vec2 uv = v_TexCoord;

    vec3 N = normalize(v_Normal);
    vec3 L = normalize(lightVector);
    vec3 V = normalize(eyeVec);
    vec3 PN = get_normal(N, V, uv);

    float lambertTerm = max(dot(PN, L), 0.1);
    /*if (lambertTerm > 0.0) {
        final_color = lambertTerm * texColor;
        vec3 E = normalize(Vertex_EyeVec.xyz);
        vec3 R = reflect(-L, PN);
        float specular = pow( max(dot(R, E), 0.0), material_shininess);
        final_color += light_specular * material_specular * specular;
    }*/

    gl_FragColor = vec4(texColor, 1.0) * lambertTerm;
}
