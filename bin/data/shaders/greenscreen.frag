// fragment shader

#version 150

#define DEPTH_WIDTH 512.0
#define DEPTH_HEIGHT 424.0

uniform sampler2DRect tex0;
uniform sampler2DRect background;
uniform sampler2DRect foreground;
uniform sampler2DRect depthInColorMap;
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
		vec2 mappedCoord = texture(depthInColorMap, texCoord).ra;

		outputColor = b;

		if ((!isinf(mappedCoord.x) && !isnan(mappedCoord.x) && mappedCoord.x != 0.0) || 
			(!isinf(mappedCoord.y) && !isnan(mappedCoord.y) && mappedCoord.y != 0.0))
		{
			mappedCoord.x += 0.5;
			mappedCoord.y += 0.5;
			
			if (mappedCoord.x >= 0.0 && mappedCoord.x < DEPTH_WIDTH && mappedCoord.y >= 0.0 && mappedCoord.y < DEPTH_HEIGHT)
			{
				float player = texture(bodyIndex, mappedCoord).r * 255.0;
				
				if (player != 255.0)
					outputColor = t;
			}
		}
		else
			outputColor = b;
	}
	else
		outputColor = f;

}
