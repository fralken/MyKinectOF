// fragment shader

#version 150

uniform vec2 frameOffset;
uniform sampler2DRect tex0; // background
uniform sampler2DRect color;
uniform sampler2DRect bodyIndex;
uniform sampler2DRect foreground;
uniform sampler2DRect coordMapping;

in vec2 texCoord;

out vec4 outputColor;

void main()
{
	vec4 b = texture(tex0, texCoord);
	vec4 f = texture(foreground, texCoord);

	outputColor = b;

	if (f.a == 0.0)
	{
		vec2 colorSize = textureSize(color);

		if (texCoord.x >= frameOffset.x && texCoord.x < colorSize.x && texCoord.y >= frameOffset.y && texCoord.y < colorSize.y)
		{
			vec2 depthCoord;
			depthCoord.x = texCoord.x - frameOffset.x;
			depthCoord.y = texCoord.y - frameOffset.y;
			float player = texture(bodyIndex, depthCoord).r * 255.0;

			if (player != 255.0) {
				vec2 mappedCoord = texture(coordMapping, depthCoord).ra;

				mappedCoord.x += 0.5;
				mappedCoord.y += 0.5;

				ivec2 size = textureSize(color);

				if (mappedCoord.x >= 0.0 && mappedCoord.x < size.x && mappedCoord.y >= 0.0 && mappedCoord.y < size.y)
				{
					vec4 t = texture(color, mappedCoord);
					outputColor = t;
				}
			}
		}
	}
}
