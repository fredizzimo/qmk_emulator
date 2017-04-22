#!/bin/bash
# You need to install glad to use this script
# https://github.com/Dav1dde/glad

glad --out-path="lib/glad" --api="gl=2.1" --generator="c" --spec="gl" --extensions="GL_EXT_framebuffer_object,GL_EXT_framebuffer_sRGB,GL_ARB_texture_float"