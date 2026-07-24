#version 440 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D pieceTexture;

void main()
{
    vec4 texColor = texture(pieceTexture, TexCoord);

    // Discard fully transparent pixels so the board square underneath shows through
    // instead of getting covered by a solid (usually black) fragment.
    if (texColor.a < 0.1)
        discard;

    FragColor = texColor;
}