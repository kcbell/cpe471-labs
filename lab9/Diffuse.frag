varying vec3 vLightVector;
varying vec3 vNormal;

uniform vec3 uColor;

void main()
{
    vec3 AmbientColor = vec3(0.3, 0.3, 0.3);
    vec3 DiffuseColor = uColor * clamp(dot(normalize(vNormal), normalize(vLightVector)), 0.0, 1.0);
    gl_FragColor = vec4(AmbientColor + DiffuseColor, 1.0);
    //gl_FragColor = vec4(abs(normalize(vNormal)), 1.0); // Show Normals
}

