#version 330 core

out vec4 FragColor;

uniform vec3 u_Offset;
uniform vec3 u_Dimensions;
uniform sampler3D u_Grid;
uniform bool u_Scalar;
uniform vec3 u_PlaneX;
uniform vec3 u_PlaneY;

uniform bool u_RenderMask = false;
uniform bool u_RenderTemperature = false;

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
    sampleLocation = sampleLocation.xzy;

    if (sampleLocation.x > 1.0 || sampleLocation.x <= 0.0 || sampleLocation.y > 1.0 || sampleLocation.y <= 0.0 || sampleLocation.z > 1.0 || sampleLocation.z <= 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {

        vec3 qValue;
        if (u_Scalar) {
            // ****************************
            // FOR RENDERING THE WORLD MASK
            // ****************************
            vec4 type = vec4(texture(u_Grid, sampleLocation).a, 0.0, 0.0, 1.0);
            if (type.r == 0.0) {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            } else if (type.r == 1.0) {
                FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            } else {
                FragColor = vec4(0.0, 0.0, 1.0, 1.0);
            }


            // ****************************
            // FOR RENDERING GENERAL CHANGES
            // ****************************
            //float t = texture(u_Grid, sampleLocation).r / 200.0;

            //float r = 0.0;
            //float g = 0.0;
            //float b = 0.0;
            //float pi = 3.1415;

            //if (mod(t,3.0) < 1) {
            //   r = cos((pi/2.0) * mod(t,1.0));
            //   g = sin((pi/2.0) * mod(t,1.0));
            //} else if (mod(t,3.0) < 2) {
            //   g = cos((pi/2.0) * mod(t,1.0));
            //   b = sin((pi/2.0) * mod(t,1.0));
            //} else {
            //   b = cos((pi/2.0) * mod(t,1.0));
            //   r = sin((pi/2.0) * mod(t,1.0));
            //}

            //FragColor = vec4(r, g, b, 1.0);//


            // // ****************************
            // // FOR RENDERING TEMPERATURE
            // // ****************************
            // float t = texture(u_Grid, sampleLocation).r - 293.15;
            // t = 1 / (1 + exp(-0.1*t));

            // float pi = 3.1415;
            // float r = sin((pi/2.0) * t);
            // float b = cos((pi/2.0) * t);
            
            // FragColor = vec4(r, 0.0, b, 1.0);
            return;
        } else {
            qValue = texture(u_Grid, sampleLocation).rgb;
        }

        // do some stuff with the quantity
        float x = dot(qValue, normalize(u_PlaneX));
        float y = dot(qValue, normalize(u_PlaneY)); 
        
        float r = sqrt(x * x + y * y);
        float theta =  atan(y, x);

        FragColor = vec4(hsv2rgb(vec3(theta, 1.0, r)), 1.0);
        //FragColor = vec4(normalize(qValue), 1.0);
    }
}