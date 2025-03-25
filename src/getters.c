/*****************************************************************************
Copyright (c) 2011  David Guillen Fandos (david@davidgf.net)
Copyright (c) 2024  Alberto Mardegan (mardy@users.sourceforge.net)
All rights reserved.

Attention! Contains pieces of code from others such as Mesa and GRRLib

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of copyright holders nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "debug.h"
#include "utils.h"
#include "state.h"
#include "stencil.h"

#include <GL/gl.h>
#include <string.h>

static const GLubyte gl_null_string[1] = { 0 };
/* This is not static because we might modify it in place */
static GLubyte s_extension_string[] =
    "GL_ARB_multitexture "
    "GL_ARB_vertex_buffer_object ";

static int prepare_extension_strings()
{
    int count = 0;
    for (GLubyte *ptr = s_extension_string; *ptr != '\0'; ptr++) {
        if (*ptr == ' ') {
            *ptr = '\0';
            count++;
        }
    }
}

static GLubyte *get_extension_string(int index)
{
    int count = 0;
    GLubyte *ptr = s_extension_string;
    while (count < index) {
        while (*ptr != '\0') ptr++;
        ptr++;
        if (*ptr == '\0') {
            /* Two null bytes mean that we have reached the end */
            return NULL;
        }
        count++;
    }
    return ptr;
}

GLenum glGetError(void)
{
    GLenum error = glparamstate.error;
    glparamstate.error = GL_NO_ERROR;
    return error;
}

const GLubyte *glGetString(GLenum name)
{
    switch (name) {
    case GL_VENDOR:
        return "opengx";
    case GL_RENDERER:
        return "libogc2";
    case GL_VERSION:
        return ogx_get_proc_address("glUseProgram") ? "3.3" : "1.5";
    case GL_EXTENSIONS:
        return s_extension_string;
    default:
        set_error(GL_INVALID_ENUM);
        return gl_null_string;
    }
}

GLboolean glIsEnabled(GLenum cap)
{
    switch (cap) {
    case GL_ALPHA_TEST:
        return glparamstate.alphatest_enabled;
    case GL_BLEND:
        return glparamstate.blendenabled;
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
        return glparamstate.clip_plane_mask & (1 << (cap - GL_CLIP_PLANE0));
    case GL_COLOR_ARRAY:
        return glparamstate.cs.color_enabled;
    case GL_COLOR_MATERIAL:
        return glparamstate.lighting.color_material_enabled;
    case GL_CULL_FACE:
        return glparamstate.cullenabled;
    case GL_DEPTH_TEST:
        return glparamstate.ztest;
    case GL_FOG:
        return glparamstate.fog.enabled;
    case GL_INDEX_ARRAY:
        return glparamstate.cs.index_enabled;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
        return glparamstate.lighting.lights[cap - GL_LIGHT0].enabled;
    case GL_LIGHTING:
        return glparamstate.lighting.enabled;
    case GL_NORMAL_ARRAY:
        return glparamstate.cs.normal_enabled;
    case GL_POINT_SPRITE:
        return glparamstate.point_sprites_enabled;
    case GL_POLYGON_OFFSET_FILL:
        return glparamstate.polygon_offset_fill;
    case GL_SCISSOR_TEST:
        return glparamstate.scissor_enabled;
    case GL_STENCIL_TEST:
        return glparamstate.stencil.enabled;
    case GL_TEXTURE_2D:
        return glparamstate.texture_enabled & (1 << glparamstate.active_texture);
    case GL_TEXTURE_COORD_ARRAY:
        {
            int unit = glparamstate.cs.active_texture;
            return glparamstate.cs.texcoord_enabled & (1 << unit);
        }
    case GL_TEXTURE_GEN_S:
    case GL_TEXTURE_GEN_T:
    case GL_TEXTURE_GEN_R:
    case GL_TEXTURE_GEN_Q:
        {
            OgxTextureUnit *tu = active_tex_unit();
            return tu->gen_enabled & (1 << (cap - GL_TEXTURE_GEN_S));
        }
    case GL_VERTEX_ARRAY:
        return glparamstate.cs.vertex_enabled;
    default:
        return 0;
    }
}

void glGetBooleanv(GLenum pname, GLboolean *params)
{
    int num_params = 1;

    switch (pname) {
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
    case GL_COLOR_ARRAY:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_FOG:
    case GL_INDEX_ARRAY:
    case GL_LIGHTING:
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    case GL_NORMAL_ARRAY:
    case GL_POINT_SPRITE:
    case GL_POLYGON_OFFSET_FILL:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_TEXTURE_GEN_S:
    case GL_TEXTURE_GEN_T:
    case GL_TEXTURE_GEN_R:
    case GL_TEXTURE_GEN_Q:
    case GL_VERTEX_ARRAY:
        *params - glIsEnabled(pname);
        return;
    case GL_CURRENT_RASTER_POSITION_VALID:
        *params = glparamstate.raster_pos_valid ? GL_TRUE : GL_FALSE;
        return;
    case GL_INDEX_MODE:
        *params = GL_FALSE;
        return;
    case GL_PACK_LSB_FIRST:
        *params = glparamstate.pack_lsb_first;
        return;
    case GL_PACK_SWAP_BYTES:
        *params = glparamstate.pack_swap_bytes;
        return;
    case GL_RGBA_MODE:
        *params = GL_TRUE;
        return;
    case GL_STEREO:
        *params = GL_FALSE;
        return;
    case GL_UNPACK_LSB_FIRST:
        *params = glparamstate.unpack_lsb_first;
        return;
    case GL_UNPACK_SWAP_BYTES:
        *params = glparamstate.unpack_swap_bytes;
        return;
    }

    GLint p[4];
    glGetIntegerv(pname, p);
    for (int i = 0; i < num_params; i++)
        params[i] = p[i];
}

void glGetDoublev(GLenum pname, GLdouble *params)
{
    float paramsf[16];
    int n = 1;

    glGetFloatv(pname, paramsf);
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
        n = 4; break;
    case GL_DEPTH_RANGE:
        n = 2; break;
    case GL_MODELVIEW_MATRIX:
    case GL_PROJECTION_MATRIX:
        n = 16; break;
    };
    for (int i = 0; i < n; i++) {
        params[i] = paramsf[i];
    }
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
    switch (pname) {
    case GL_CURRENT_RASTER_POSITION:
        floatcpy(params, glparamstate.raster_pos, 4);
        break;
    case GL_DEPTH_BIAS:
        *params = glparamstate.transfer_depth_bias;
        break;
    case GL_DEPTH_RANGE:
        params[0] = glparamstate.depth_near;
        params[1] = glparamstate.depth_far;
        break;
    case GL_DEPTH_SCALE:
        *params = glparamstate.transfer_depth_scale;
        break;
    case GL_MODELVIEW_MATRIX:
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 4; j++)
                params[j * 4 + i] = glparamstate.modelview_matrix[i][j];
        params[3] = params[7] = params[11] = 0.0f;
        params[15] = 1.0f;
        return;
    case GL_PROJECTION_MATRIX:
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                params[j * 4 + i] = glparamstate.projection_matrix[i][j];
        return;
    default:
        return;
    };
}

// XXX: Need to finish glGets, important!!!
void glGetIntegerv(GLenum pname, GLint *params)
{
    switch (pname) {
    case GL_ACTIVE_TEXTURE:
        *params = GL_TEXTURE0 + glparamstate.active_texture;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        *params = glparamstate.bound_vbo_array;
        break;
    case GL_CLIENT_ACTIVE_TEXTURE:
        *params = GL_TEXTURE0 + glparamstate.cs.active_texture;
        break;
    case GL_COLOR_ARRAY_SIZE:
        *params = STATE_ARRAY(CLR).size;
        return;
    case GL_COLOR_ARRAY_STRIDE:
        *params = STATE_ARRAY(CLR).stride;
        return;
    case GL_COLOR_ARRAY_TYPE:
        *params = STATE_ARRAY(CLR).type;
        return;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        *params = glparamstate.bound_vbo_element_array;
        break;
    case GL_AUX_BUFFERS:
        *params = 0;
        break;
    case GL_DRAW_BUFFER:
    case GL_READ_BUFFER:
        *params = glparamstate.active_buffer;
        break;
    case GL_INDEX_OFFSET:
        *params = glparamstate.transfer_index_offset;
        break;
    case GL_INDEX_SHIFT:
        *params = glparamstate.transfer_index_shift;
        break;
    case GL_MAX_CLIP_PLANES:
        *params = MAX_CLIP_PLANES;
        return;
    case GL_MAX_TEXTURE_COORDS:
    case GL_MAX_TEXTURE_IMAGE_UNITS:
    case GL_MAX_TEXTURE_UNITS:
        *params = MAX_TEXTURE_UNITS;
        break;
    case GL_MAX_TEXTURE_SIZE:
        *params = 1024;
        return;
    case GL_MAX_VERTEX_ATTRIBS:
        *params = MAX_VERTEX_ATTRIBS;
        return;
    case GL_MODELVIEW_STACK_DEPTH:
        *params = MAX_MODV_STACK;
        return;
    case GL_PROJECTION_STACK_DEPTH:
        *params = MAX_PROJ_STACK;
        return;
    case GL_MAX_NAME_STACK_DEPTH:
        *params = MAX_NAME_STACK_DEPTH;
        return;
    case GL_MAX_PIXEL_MAP_TABLE:
        *params = MAX_PIXEL_MAP_TABLE;
        break;
    case GL_NAME_STACK_DEPTH:
        *params = glparamstate.name_stack_depth;
        return;
    case GL_NUM_EXTENSIONS:
        *params = prepare_extension_strings();
        return;
    case GL_NORMAL_ARRAY_STRIDE:
        *params = STATE_ARRAY(NRM).stride;
        return;
    case GL_NORMAL_ARRAY_TYPE:
        *params = STATE_ARRAY(NRM).type;
        return;
    case GL_PACK_ROW_LENGTH:
        *params = glparamstate.pack_row_length;
        break;
    case GL_PACK_IMAGE_HEIGHT:
        *params = glparamstate.pack_image_height;
        break;
    case GL_PACK_SKIP_ROWS:
        *params = glparamstate.pack_skip_rows;
        break;
    case GL_PACK_SKIP_PIXELS:
        *params = glparamstate.pack_skip_pixels;
        break;
    case GL_PACK_SKIP_IMAGES:
        *params = glparamstate.pack_skip_images;
        break;
    case GL_PACK_ALIGNMENT:
        *params = glparamstate.pack_alignment;
        break;
    case GL_PIXEL_MAP_I_TO_I_SIZE:
    case GL_PIXEL_MAP_S_TO_S_SIZE:
    case GL_PIXEL_MAP_I_TO_R_SIZE:
    case GL_PIXEL_MAP_I_TO_G_SIZE:
    case GL_PIXEL_MAP_I_TO_B_SIZE:
    case GL_PIXEL_MAP_I_TO_A_SIZE:
    case GL_PIXEL_MAP_R_TO_R_SIZE:
    case GL_PIXEL_MAP_G_TO_G_SIZE:
    case GL_PIXEL_MAP_B_TO_B_SIZE:
    case GL_PIXEL_MAP_A_TO_A_SIZE:
        if (glparamstate.pixel_maps) {
            int index = pname - GL_PIXEL_MAP_I_TO_I_SIZE;
            *params = glparamstate.pixel_maps->sizes[index];
        } else {
            /* By default, there's one entry (0.0) in the table */
            *params = 1;
        }
        break;
    case GL_STENCIL_BITS:
        *params = _ogx_stencil_flags & OGX_STENCIL_8BIT ? 8 : 4;
        break;
    case GL_STENCIL_CLEAR_VALUE:
        *params = glparamstate.stencil.clear;
        break;
    case GL_STENCIL_FAIL:
        *params = glparamstate.stencil.op_fail;
        break;
    case GL_STENCIL_FUNC:
        *params = gl_compare_from_gx(glparamstate.stencil.func);
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        *params = glparamstate.stencil.op_zfail;
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        *params = glparamstate.stencil.op_zpass;
        break;
    case GL_STENCIL_REF:
        *params = glparamstate.stencil.ref;
        break;
    case GL_STENCIL_TEST:
        *params = glparamstate.stencil.enabled;
        break;
    case GL_STENCIL_VALUE_MASK:
        *params = glparamstate.stencil.mask;
        break;
    case GL_STENCIL_WRITEMASK:
        *params = glparamstate.stencil.wmask;
        break;
    case GL_TEXTURE_COORD_ARRAY_SIZE:
        {
            int unit = glparamstate.cs.active_texture;
            *params = STATE_ARRAY_TEX(unit).size;
        }
        return;
    case GL_TEXTURE_COORD_ARRAY_STRIDE:
        {
            int unit = glparamstate.cs.active_texture;
            *params = STATE_ARRAY_TEX(unit).stride;
        }
        return;
    case GL_TEXTURE_COORD_ARRAY_TYPE:
        {
            int unit = glparamstate.cs.active_texture;
            *params = STATE_ARRAY_TEX(unit).type;
        }
        return;
    case GL_UNPACK_ROW_LENGTH:
        *params = glparamstate.unpack_row_length;
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        *params = glparamstate.unpack_image_height;
        break;
    case GL_UNPACK_SKIP_ROWS:
        *params = glparamstate.unpack_skip_rows;
        break;
    case GL_UNPACK_SKIP_PIXELS:
        *params = glparamstate.unpack_skip_pixels;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        *params = glparamstate.unpack_skip_images;
        break;
    case GL_UNPACK_ALIGNMENT:
        *params = glparamstate.unpack_alignment;
        break;
    case GL_VERTEX_ARRAY_SIZE:
        *params = STATE_ARRAY(POS).size;
        return;
    case GL_VERTEX_ARRAY_STRIDE:
        *params = STATE_ARRAY(POS).stride;
        return;
    case GL_VERTEX_ARRAY_TYPE:
        *params = STATE_ARRAY(POS).type;
        return;
    case GL_VIEWPORT:
        memcpy(params, glparamstate.viewport, 4 * sizeof(int));
        return;
    case GL_RENDER_MODE:
        *params = glparamstate.render_mode;
        return;
    case GL_ZOOM_X:
        *params = glparamstate.pixel_zoom_x;
        break;
    case GL_ZOOM_Y:
        *params = glparamstate.pixel_zoom_y;
        break;
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
    case GL_CURRENT_RASTER_POSITION_VALID:
    case GL_PACK_LSB_FIRST:
    case GL_PACK_SWAP_BYTES:
    case GL_UNPACK_LSB_FIRST:
    case GL_UNPACK_SWAP_BYTES:
        {
            GLboolean b;
            glGetBooleanv(pname, &b);
            *params = b;
        }
        return;
    default:
        return;
    };
}

void glGetPointerv(GLenum pname, GLvoid **params)
{
    switch (pname) {
    case GL_COLOR_ARRAY_POINTER:
        *params = (void*)STATE_ARRAY(CLR).pointer;
        return;
    case GL_NORMAL_ARRAY_POINTER:
        *params = (void*)STATE_ARRAY(NRM).pointer;
        return;
    case GL_TEXTURE_COORD_ARRAY_POINTER:
        {
            int unit = glparamstate.cs.active_texture;
            *params = (void*)STATE_ARRAY_TEX(unit).pointer;
        }
        return;
    case GL_VERTEX_ARRAY_POINTER:
        *params = (void*)STATE_ARRAY(POS).pointer;
        return;
    }
}

const GLubyte *glGetStringi(GLenum name, GLuint index)
{
    if (name == GL_EXTENSIONS) {
        return get_extension_string(index);
    }
    return NULL;
}
