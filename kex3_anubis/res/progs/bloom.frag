uniform sampler2D uDiffuse;
uniform float uBloomThreshold;

const vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);

void main() {
    vec2 texcoord = gl_TexCoord[0].st;
    vec4 sample = texture2D(uDiffuse, texcoord);
    
    float brightPassThreshold = uBloomThreshold;
    
    float luminance = dot(luminanceVector, sample.rgb);
    luminance = max(0.0, luminance - brightPassThreshold);
    sample.rgb *= sign(luminance);
    sample.a = 1.0;
    
    gl_FragColor = sample;
}
