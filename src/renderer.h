#include "window.h"

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;

private:
};