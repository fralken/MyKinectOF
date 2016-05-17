// fragment shader

#version 150

uniform sampler2DRect tex0;
uniform sampler2DRect background;
uniform sampler2DRect foreground;
uniform sampler2DRect coordMapping;
uniform sampler2DRect bodyIndex;

in vec2 texCoord;

out vec4 outputColor;

void main()
{
	vec4 t = texture(tex0, texCoord);
	vec4 f = texture(foreground, texCoord);
	vec4 b = texture(background, texCoord);

	if (f.a == 0.0)
	{
		vec2 mappedCoord = texture(coordMapping, texCoord).ra;

		outputColor = b;

		if ((!isinf(mappedCoord.x) && !isnan(mappedCoord.x)) || 
			(!isinf(mappedCoord.y) && !isnan(mappedCoord.y)))
		{
			ivec2 size = textureSize(bodyIndex);

			mappedCoord.x += 0.5;
			mappedCoord.y += 0.5;
			
			if (mappedCoord.x >= 0.0 && mappedCoord.x < size.x && mappedCoord.y >= 0.0 && mappedCoord.y < size.y)
			{
				float player = texture(bodyIndex, mappedCoord).r * 255.0;
				
				if (player != 255.0)
					outputColor = t;
			}
		}
	}
	else
		outputColor = f;
}
