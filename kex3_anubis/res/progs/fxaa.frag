uniform sampler2D uDiffuse;
uniform float uViewWidth;
uniform float uViewHeight;
uniform float uMaxSpan;
uniform float uReduceMax;
uniform float uReduceMin;

void main() {
    vec2 texcoord = gl_TexCoord[0].st;
    vec2 frameBufSize = vec2(uViewWidth, uViewHeight);
    
    vec3 rgbNW = texture2D(uDiffuse, texcoord + (vec2(-1.0, -1.0) / frameBufSize)).xyz;
    vec3 rgbNE = texture2D(uDiffuse, texcoord + (vec2( 1.0, -1.0) / frameBufSize)).xyz;
    vec3 rgbSW = texture2D(uDiffuse, texcoord + (vec2(-1.0,  1.0) / frameBufSize)).xyz;
    vec3 rgbSE = texture2D(uDiffuse, texcoord + (vec2( 1.0,  1.0) / frameBufSize)).xyz;
    vec3 rgbM  = texture2D(uDiffuse, texcoord).xyz;
    
    vec3 luma = vec3(0.299, 0.587, 0.114);
    
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * (1.0 / uReduceMax)),
                          (1.0 / uReduceMin));
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2(uMaxSpan, uMaxSpan),
              max(vec2(-uMaxSpan, -uMaxSpan),
                  dir * rcpDirMin)) / frameBufSize;
    
    vec3 rgbA = (1.0/2.0) * (texture2D(uDiffuse, texcoord + dir * (1.0/3.0 - 0.5)).xyz +
                             texture2D(uDiffuse, texcoord + dir * (2.0/3.0 - 0.5)).xyz);
    
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (texture2D(uDiffuse, texcoord + dir * (0.0/3.0 - 0.5)).xyz +
                                                texture2D(uDiffuse, texcoord + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);
    
    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        gl_FragColor = vec4(rgbA, 1.0);
        return;
    }
    
    gl_FragColor = vec4(rgbB, 1.0);
}
