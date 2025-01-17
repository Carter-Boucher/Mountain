#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord; // Texture coordinates from the vertex shader

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

uniform sampler2D objectTexture; // Texture sampler
uniform sampler2D normalMap;  

void main()
{
    // Obtain normal from normal map in range [0,1]
    vec3 mappedNormal = texture(normalMap, TexCoord).rgb;    
    // Transform normal vector to range [-1,1] and normalize
    mappedNormal = normalize(mappedNormal * 2.0 - 1.0);

    vec3 objectColor = texture(objectTexture, TexCoord).rgb;
    float distance = length(lightPos - FragPos);

    // Attenuation
    float maxDistance = 400.0;
    float attenuation = clamp(1.0 - (distance / maxDistance), 0.0, 1.0);

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(mappedNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.25;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor * attenuation;
    FragColor = vec4(result, 1.0);

    // Debug possibilities
    // FragColor = vec4(mappedNormal, 1.0);
    // FragColor = vec4(lightDir, 1.0);
    // FragColor = vec4(reflectDir, 1.0);
    // FragColor = vec4(viewDir, 1.0);
}
