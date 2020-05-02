#version 330 core

out vec4 FragColor;

uniform vec3 u_Offset;
uniform vec3 u_Dimensions;
uniform sampler3D u_Grid;
uniform bool u_Scalar;
uniform vec3 u_PlaneX;
uniform vec3 u_PlaneY;

in vec3 WSPosition;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec3 sampleLocation = WSPosition - u_Offset;

    sampleLocation += u_Dimensions / 2.0;
    sampleLocation /= u_Dimensions;
    if (sampleLocation.x > 1.0 || sampleLocation.x <= 0.0 || sampleLocation.y > 1.0 || sampleLocation.y <= 0.0 || sampleLocation.z > 1.0 || sampleLocation.z <= 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {

        vec3 qValue;
        if (u_Scalar) {
            FragColor = vec4(texture(u_Grid, sampleLocation).r, 0.0, 0.0, 1.0);
        } else {
            qValue = texture(u_Grid, sampleLocation).rgb;
        }

        // do some stuff with the quantity
        float x = dot(qValue, normalize(u_PlaneX));
        float y = dot(qValue, normalize(u_PlaneY)); 
        
        float r = sqrt(x * x + y * y);
        float theta =  atan(y, x);

        FragColor = vec4(hsv2rgb(vec3(theta, 1.0, r)), 1.0);
    }
}