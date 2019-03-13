/*
Copyright (C) 2006 Quake2xp Team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

//
// local[0] - distortion strength (comes from cvar, 1.0 is default)
//

static char fragment_program_heathazemask[] =  
"!!ARBfp1.0\n"
"OPTION ARB_precision_hint_fastest;\n"

"TEMP R0, R1, R2;\n"

"TEX R0, fragment.texcoord[0], texture[0], 2D;\n"

// fetch the water texture
"ADD R1, R0, fragment.texcoord[2];\n"

//"TEX R1, R1, texture[2], 2D;\n" // bump 3th tex
"TEX R1, fragment.texcoord[2], texture[2], 2D;\n" // dont bump 3th tex

// scale offset by the distortion factor
"MUL R0, R0, fragment.texcoord[3];\n"   //error was MUL

// apply the alpha mask
"TEX R2, fragment.texcoord[2], texture[3], 2D;\n"
"MUL R0, R0, R2;\n"

// calculate the screen texcoord in the 0.0 to 1.0 range
"MUL R2, fragment.position, program.local[0];\n"

// offset by the scaled ds/dt and clamp it to 0.0 - 1.0
"ADD_SAT R2, R0, R2;\n"

// scale by the screen non-power-of-two-adjust
"MUL R2, R2, program.local[1];\n"

// fetch the screen texture
"TEX R2, R2, texture[1], 2D;\n"

"MUL R1, R1, fragment.color;\n"
//"ADD result.color, R2, R1;\n"
"ADD R2, R2, R1;\n"
"MUL result.color, R2, program.local[3];\n"

"END\n";


/*
// Original BeefQuake R6 (March 13 2005) GLSL program
uniform sampler2D texImage;
uniform sampler2D texDistort;

void main() {
   vec4 col;
   vec4 offset;
   float x,y;
   
   offset = texture2D(texDistort, vec2(gl_TexCoord[1]));
   x = gl_TexCoord[0].r + offset[2];
   y = gl_TexCoord[0].g + offset[3];
   col = texture2D(texImage, vec2(x,y));
   gl_FragColor = col * gl_Color;
}
*/

#if 1
static char fragment_program_warp[] =
"!!ARBfp1.0\n"
//"OPTION ARB_precision_hint_fastest;\n"
"OPTION ARB_precision_hint_nicest;\n"

"PARAM rgbscale = program.local[0];\n"
"TEMP offset, coord, dist, col;\n"

"TEX offset, fragment.texcoord[1], texture[1], 2D;\n"
"MUL offset, offset, 0.5;\n"

// fetch the water texture
"ADD coord.x, fragment.texcoord[0].x, offset.z;\n"
"ADD coord.y, fragment.texcoord[0].y, offset.w;\n"
"TEX dist, coord, texture[0], 2D;\n"
"MUL col, dist, fragment.color;\n"
"MUL result.color, col, rgbscale;\n"

"END\n";

#else

static char fragment_program_warp[] =
"!!ARBfp1.0\n"
//"OPTION ARB_precision_hint_fastest;\n"
"OPTION ARB_precision_hint_nicest;\n"

"PARAM rgbscale = program.local[0];\n"
"TEMP offset, coord, dist, col;\n"

"TEX offset, fragment.texcoord[0], texture[0], 2D;\n"
"MUL offset, offset, 0.5;\n"

// fetch the water texture
"ADD coord.x, fragment.texcoord[1].x, offset.z;\n"
"ADD coord.y, fragment.texcoord[1].y, offset.w;\n"
"TEX dist, coord, texture[1], 2D;\n"
"MUL col, dist, fragment.color;\n"
"MUL result.color, col, rgbscale;\n"

"END\n";
#endif

static char fragment_program_water_distort[] =
"!!ARBfp1.0\n"

// Scroll and scale the distortion texture coordinates.
// Scroll coordinates are specified externally.
"PARAM scroll1 = program.local[0];\n"
"PARAM scroll2 = program.local[1];\n"
"PARAM texScale1 = { 0.008, 0.008, 1.0, 1.0 };\n"
"PARAM texScale2 = { 0.007, 0.007, 1.0, 1.0 };\n"
"TEMP texCoord1;\n"
"TEMP texCoord2;\n"
"MUL texCoord1, fragment.texcoord[1], texScale1;\n"
"MUL texCoord2, fragment.texcoord[1], texScale2;\n"
"ADD texCoord1, texCoord1, scroll1;\n"
"ADD texCoord2, texCoord2, scroll2;\n"

// Load the distortion textures and add them together.
"TEMP distortColor;\n"
"TEMP distortColor2;\n"
"TXP distortColor, texCoord1, texture[1], 2D;\n"
"TXP distortColor2, texCoord2, texture[1], 2D;\n"
"ADD distortColor, distortColor, distortColor2;\n"

// Subtract 1.0 and scale by 2.0.
// Textures will be distorted from -2.0 to 2.0 texels.
"PARAM scaleFactor = { 2.0, 2.0, 2.0, 2.0 };\n"
"PARAM one = { 1.0, 1.0, 1.0, 1.0 };\n"
"SUB distortColor, distortColor, one;\n"
"MUL distortColor, distortColor, scaleFactor;\n"

// Apply distortion to reflection texture coordinates.
"TEMP distortCoord;\n"
"TEMP endColor;\n"
"ADD distortCoord, distortColor, fragment.texcoord[0];\n"
"TXP endColor, distortCoord, texture, 2D;\n"

// Get a vector from the surface to the view origin
"PARAM vieworg = program.local[2];\n"
"TEMP eyeVec;\n"
"TEMP trans;\n"
"SUB eyeVec, vieworg, fragment.texcoord[1];\n"

// Normalize the vector to the eye position
"TEMP temp;\n"
"TEMP invLen;\n"
"DP3 temp, eyeVec, eyeVec;\n"
"RSQ invLen, temp.x;\n"
"MUL eyeVec, eyeVec, invLen;\n"
"ABS eyeVec.z, eyeVec.z;\n" // so it works underwater, too

// Load the ripple normal map
"TEMP normalColor;\n"
"TEMP normalColor2;\n"
// Scale texture
"MUL texCoord1, fragment.texcoord[2], texScale2;\n"
"MUL texCoord2, fragment.texcoord[2], texScale1;\n"
// Scroll texture
"ADD texCoord1, texCoord1, scroll1;\n"
"ADD texCoord2, texCoord2, scroll2;\n"
// Get texel color
"TXP normalColor, texCoord1, texture[2], 2D;\n"
"TXP normalColor2, texCoord2, texture[2], 2D;\n"
// Combine normal maps
"ADD normalColor, normalColor, normalColor2;\n"
"SUB normalColor, normalColor, 1.0;\n"

// Normalize normal texture
"DP3 temp, normalColor, normalColor;\n"
"RSQ invLen, temp.x;\n"
"MUL normalColor, invLen, normalColor;\n"

// Fresenel approximation
"DP3 trans.w, normalColor, eyeVec;\n"
"SUB endColor.w, 1.0, trans.w;\n"
"MAX endColor.w, endColor.w, 0.2;\n" // MAX sets the min?  How odd.
"MIN endColor.w, endColor.w, 0.8;\n" // Leave a LITTLE bit of transparency always

// Put the color in the output (TODO: put this in final OP)
"MOV result.color, endColor;\n"

"END\n";


static char vertex_program_distort[] =
"!!ARBvp1.0\n"
"OPTION ARB_position_invariant;\n"

"PARAM vec = { 1.0, 0.0, 0.0, 1.0 };\n"
"PARAM dtc = { 0.0, 0.5, 0.0, 1.0 };\n"

"TEMP R0, R1, R2;\n"

"MOV          R0, vec;\n"
"DP4          R0.z, vertex.position, state.matrix.modelview.row[2];\n"

"DP4          R1, R0, state.matrix.projection.row[0];\n"
"DP4          R2, R0, state.matrix.projection.row[3];\n"

// don't let the recip get near zero for polygons that cross the view plane
"MAX          R2, R2, 1.0;\n"

"RCP          R2, R2.w;\n"
"MUL          R1, R1, R2;\n"

// clamp the distance so the deformations don't get too wacky near the view
"MIN          R1, R1, 0.02;\n"

"MOV          result.texcoord[0], dtc;\n"
"DP4          result.texcoord[0].x, vertex.texcoord[0], state.matrix.texture[0].row[0];\n"
"DP4          result.texcoord[0].y, vertex.texcoord[0], state.matrix.texture[0].row[1];\n"

"MOV          result.texcoord[2], dtc;\n"
"DP4          result.texcoord[2].x, vertex.texcoord[2], state.matrix.texture[2].row[0];\n"
"DP4          result.texcoord[2].y, vertex.texcoord[2], state.matrix.texture[2].row[1];\n"

"MUL          result.texcoord[3], R1, program.local[0];\n"

"MOV          result.color, vertex.color;\n"

"END\n";


/*
=============================================================================

  PROGRAM MANAGEMENT

=============================================================================
*/

static char *fragment_progs[NUM_FRAGMENT_PROGRAM] = 
{
	fragment_program_heathazemask,
	fragment_program_warp,
	fragment_program_water_distort
};

static char *vertex_progs[NUM_VERTEX_PROGRAM] = 
{
    vertex_program_distort
};

GLuint fragment_programs[NUM_FRAGMENT_PROGRAM];
GLuint vertex_programs[NUM_VERTEX_PROGRAM];


/*
=============
R_Compile_ARB_Programs
=============
*/
void R_Compile_ARB_Programs (void)
{
	int			i, error_pos;
	const char	*errors;
	
	if (!glConfig.arb_fragment_program) // not supported/enabled
		return;

	// fragment programs
	qglGenProgramsARB(NUM_FRAGMENT_PROGRAM, &fragment_programs[0]);
    for (i = 0; i < NUM_FRAGMENT_PROGRAM; i++)
    {
		qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragment_programs[i]);
		qglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen(fragment_progs[i]), fragment_progs[i]);
		qglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
		if (error_pos != -1) 
		{
			errors = qglGetString(GL_PROGRAM_ERROR_STRING_ARB);	
			VID_Printf (PRINT_ALL, S_COLOR_RED"FragmentProgram error at position %d in shader %d\nARB_ERROR: %s\n", 
						error_pos, fragment_programs[i], errors);
			qglDeleteProgramsARB(1, &fragment_programs[i]);
			break;
		}
	}

	// vertex programs
	if (glConfig.arb_vertex_program)
	{
		qglGenProgramsARB(NUM_VERTEX_PROGRAM, &vertex_programs[0]);
		for (i = 0; i < NUM_VERTEX_PROGRAM; i++)
		{
			qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertex_programs[i]);
			qglProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
								strlen(vertex_progs[i]), vertex_progs[i]);
			qglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
			if (error_pos != -1)
			{
				errors = qglGetString(GL_PROGRAM_ERROR_STRING_ARB);	
				VID_Printf (PRINT_ALL, S_COLOR_RED"VertexProgram error at position %d in shader %d\nARB_ERROR: %s\n", 
							error_pos, vertex_programs[i], errors);
				qglDeleteProgramsARB(1, &vertex_programs[i]);
				break;
			}
		}
	}
	VID_Printf (PRINT_ALL, "...loading ARB programs... succeeded\n");
}

