#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__


typedef struct Texture {
    GLuint textureId;
    bool singleChannel;
    vec4 color;
} Texture;
void loadTextureImage(Texture* texture, char* imagePath);
Texture texture_fromFile(char* imagePath, vec4 color) {
    Texture texture;
    loadTextureImage(&texture, imagePath);
    vec4_dup(texture.color, color);
    return texture;
}

void loadTextureImage(Texture* texture, char* imagePath) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    if (data == NULL) {
        printf("failed to load texture: %s\n", imagePath);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (nrChannels == 1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    texture->textureId = textureID;
    texture->singleChannel = nrChannels == 1;
}

#endif