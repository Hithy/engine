
namespace render {

  void renderQuad();
  void renderBox();
  void renderSphere();

  unsigned int loadHDRFile(const char* path);
  unsigned int genTextureCube(unsigned int width, unsigned int height, bool with_mipmap = false, int NR = 3);
  unsigned int genTexture2D(unsigned int width, unsigned int height, 
    bool with_mipmap = false, int NR = 3, const void* data = nullptr, bool is_hdr = false);
};