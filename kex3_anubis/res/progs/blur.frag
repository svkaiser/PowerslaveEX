uniform sampler2D uDiffuse;
uniform float uBlurRadius;
uniform float uSize;
uniform int uDirection;

vec2 getBlurStep(const in float stepCount,
                 const in float blur,
                 const in float hstep,
                 const in float vstep) {
    return vec2(gl_TexCoord[0].s + stepCount * blur * hstep,
                gl_TexCoord[0].t + stepCount * blur * vstep);
}

void main() {
    vec4 sum = vec4(0.0);
    float blur = uBlurRadius / uSize;
    float hstep;
    float vstep;
    
    if(uDirection == 0) {
        hstep = 0.0;
        vstep = 1.0;
    }
    else {
        hstep = 1.0;
        vstep = 0.0;
    }
    
    sum += texture2D(uDiffuse, getBlurStep(-4.0, blur, hstep, vstep)) * 0.0162162162;
    sum += texture2D(uDiffuse, getBlurStep(-3.0, blur, hstep, vstep)) * 0.0540540541;
    sum += texture2D(uDiffuse, getBlurStep(-2.0, blur, hstep, vstep)) * 0.1216216216;
    sum += texture2D(uDiffuse, getBlurStep(-1.0, blur, hstep, vstep)) * 0.1945945946;
    
    sum += texture2D(uDiffuse, gl_TexCoord[0].st) * 0.2270270270;
    
    sum += texture2D(uDiffuse, getBlurStep(1.0, blur, hstep, vstep)) * 0.1945945946;
    sum += texture2D(uDiffuse, getBlurStep(2.0, blur, hstep, vstep)) * 0.1216216216;
    sum += texture2D(uDiffuse, getBlurStep(3.0, blur, hstep, vstep)) * 0.0540540541;
    sum += texture2D(uDiffuse, getBlurStep(4.0, blur, hstep, vstep)) * 0.0162162162;
    
    gl_FragColor = vec4(sum.rgb, 1.0);
}
