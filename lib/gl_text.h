#ifndef __GL_TEXT_H__
#define __GL_TEXT_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _WIN32
#define FONT_FILE_NAME "C:/Windows/Fonts/Calibri.ttf"
#else
#define FONT_FILE_NAME "/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf"
#endif

#define glGetChar(char) glTextChars[char - 32]

typedef struct CharInfo {
    uint32_t texId;
    uint32_t width, height;
    uint32_t bitmap_left, bitmap_top;
    uint32_t advanceX, advanceY;
} CharInfo;

const int maxTextHeight = 128;
// All ascii
CharInfo glTextChars[126 - 32];
GLuint glTextDefaultShader = 0, glTextVao = 0, glTextVbo = 0;
GLint glTextColor = -1, glTextTexture = -1, glTextProjection = -1;
void ui_init() {
    FT_Library library; /* handle to library     */
    FT_Face face;       /* handle to face object */

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        printf("Failed to init FreeType\n");
        return;
    }

    error = FT_New_Face(library,
                        FONT_FILE_NAME,
                        0,
                        &face);
    if (error == FT_Err_Unknown_File_Format) {
        printf("Failed to read font file: Unknown format\n");
        return;
    } else if (error) {
        printf("Failed to read font file\n");
        return;
    }

    error = FT_Set_Pixel_Sizes(face, 0, maxTextHeight);
    if (error) {
        printf("Failed to set char size\n");
        return;
    }

    // error = FT_Select_Charmap(
    //     face,                 /* target face object */
    //     FT_ENCODING_BIG5); /* encoding           */
    // if (error) {
    //     printf("Failed select charmap\n");
    //     return;
    // }
    // FT_CharMap found = 0;
    // FT_CharMap charmap;
    // int n;
    // for (n = 0; n < face->num_charmaps; n++) {
    //     charmap = face->charmaps[n];
    //     if (charmap->platform_id == my_platform_id &&
    //         charmap->encoding_id == my_encoding_id) {
    //         found = charmap;
    //         break;
    //     }
    // }
    // if (!found) {
    //     printf("Failed charmap not found\n");
    //     return;
    // }
    // /* now, select the charmap for the face object */
    // error = FT_Set_Charmap(face, found);
    // if (error) {
    //     printf("Failed to set charmap\n");
    //     return;
    // }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int n = 0; n < 126 - 32; n++) {
        FT_Load_Char(face, n + ' ', FT_LOAD_RENDER);
        FT_GlyphSlot glyph = face->glyph;
        // generate texture
        uint32_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            glyph->bitmap.width,
            glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            glyph->bitmap.buffer);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextChars[n].width = glyph->bitmap.width;
        glTextChars[n].height = glyph->bitmap.rows;
        glTextChars[n].bitmap_left = glyph->bitmap_left;
        glTextChars[n].bitmap_top = glyph->bitmap_top;
        glTextChars[n].advanceX = glyph->advance.x;
        glTextChars[n].advanceY = glyph->advance.y;
        glTextChars[n].texId = texture;

        // printf("texture id: %u, %dx%d\n", texture, glTextChars[n].width, glTextChars[n].height);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    glTextDefaultShader = compileShaderProgram(
        "#version 330 core\n\
in vec2 texcoord;\
uniform sampler2D tex;\
uniform vec4 color;\
out vec4 fragColor;\
void main(void) {\
    fragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;\
}",
        "#version 330 core\n\
in vec4 vPos;\
uniform mat4 uProjection;\
out vec2 texcoord;\
void main(void) {\
    gl_Position = uProjection * vec4(vPos.x, vPos.y, 0, 1);\
    gl_Position.y = -gl_Position.y;\
    texcoord = vPos.zw;\
}");

    if (!glTextDefaultShader) {
        printf("Failed to compile text shader\n");
        return;
    }

    glTextColor = glGetUniformLocation(glTextDefaultShader, "color");
    glTextTexture = glGetUniformLocation(glTextDefaultShader, "tex");
    glTextProjection = glGetUniformLocation(glTextDefaultShader, "uProjection");

    // create ui component vao
    glGenVertexArrays(1, &glTextVao);
    glGenBuffers(1, &glTextVbo);
}

void ui_windowSizeUpdate(int width, int height) {
    mat4x4 m;
    glUseProgram(glTextDefaultShader);
    // mat4x4_identity_create(m);
    mat4x4_ortho(m, 0, width, 0, height, -1, 1);
    // mat4x4_translate_in_place(m, 0, 1, 0);

    glUniformMatrix4fv(glTextProjection, 1, GL_FALSE, (float*)m);
}

void ui_drawTexture(GLuint textureId, float x, float y, float w, float h, float angle) {
    GLfloat box[4][4] = {
        {x, y, 0, 0},
        {x + w, y, 1, 0},
        {x, y + h, 0, 1},
        {x + w, y + h, 1, 1},
    };
    glBindVertexArray(glTextVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glTextTexture, 0);

    glBindBuffer(GL_ARRAY_BUFFER, glTextVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 16, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ui_textDrawString(char* str, float x, float y, float charHeight) {
    glUseProgram(glTextDefaultShader);
    glUniform4f(glTextColor, 1, 1, 1, 1);

    float scale = charHeight / maxTextHeight;
    for (; *str; str++) {
        CharInfo* ch = &glGetChar(*str);
        float xpos = x + ch->bitmap_left * scale;
        float ypos = y - ch->bitmap_top * scale;

        float w = ch->width * scale;
        float h = ch->height * scale;
        // printf("%f, %f, %d, %d\n", xpos, ypos, ch->width, ch->height);
        // printf("%d, %d, %d\n", ch->width, ch->height - ch->bearingY, ch->advance >> 6);
        ui_drawTexture(ch->texId, xpos, ypos, w, h, 0.0);
        x += (ch->advanceX >> 6) * scale;
        y += (ch->advanceY >> 6) * scale;
    }
}

uint32_t ui_textCalculateStringWidth(char* str, float charHeight) {
    float scale = charHeight / maxTextHeight;
    uint32_t width = 0;
    for (; *str; str++) {
        CharInfo* ch = &glGetChar(*str);
        width += (ch->advanceX >> 6) * scale;
    }
    return width;
}

void ui_textDrawStringCenter(char* str, float x, float y, float charHeight) {
    uint32_t width = ui_textCalculateStringWidth(str, charHeight);
    ui_textDrawString(str, x - width / 2, y + charHeight / 2, charHeight);
}

#endif