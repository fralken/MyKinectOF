// fragment shader

#version 150

uniform sampler2DRect tex0;
uniform sampler2DRect background;
uniform sampler2DRect foreground;
uniform sampler2DRect coordMapping;
uniform sampler2DRect color;

in vec2 texCoord;

out vec4 outputColor;

void main()
{
	vec4 f = texture(foreground, texCoord);
	vec4 b = texture(background, texCoord);

	if (f.a == 0.0)
	{
		float player = texture(tex0, texCoord).r * 255.0;

		outputColor = b;

		if (player != 255.0) {
			vec2 mappedCoord = texture(coordMapping, texCoord).ra;

			mappedCoord.x += 0.5;
			mappedCoord.y += 0.5;

			ivec2 size = textureSize(color);

			if (mappedCoord.x >= 0.0 && mappedCoord.x < size.x && mappedCoord.y >= 0.0 && mappedCoord.y < size.y) {
				vec4 t = texture(color, mappedCoord);
				outputColor = t;
			}
		}
	}
	else
		outputColor = f;
}
