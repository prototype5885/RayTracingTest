#if defined(_WIN32) || defined(_WIN64)
#include <SDL2/SDL.h>
#include <array>
#include <windows.h>
#endif

#include <math.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <unordered_map>
#include <filesystem>

using std::cout;
using std::endl;
using std::string;
using std::to_string;

#define WHITE_COLOR ((0 << 24) | (255 << 16) | (255 << 8) | 255)
#define GREY_COLOR ((0 << 24) | (50 << 16) | (50 << 8) | 50)
#define DARKER_GREY_COLOR ((0 << 24) | (30 << 16) | (30 << 8) | 30)
#define BLACK_COLOR ((0 << 24) | (0 << 16) | (0 << 8) | 0)
#define RED_COLOR ((0 << 24) | (255 << 16) | (0 << 8) | 0)
#define GREEN_COLOR ((0 << 24) | (0 << 16) | (255 << 8) | 0)
#define BLUE_COLOR ((0 << 24) | (0 << 16) | (0 << 8) | 255)
#define YELLOW_COLOR ((0 << 24) | (255 << 16) | (255 << 8) | 0)

typedef struct
{
  float x, y;
} Vector2;

typedef struct
{
  int x, y;
} Vector2i;

typedef struct
{
  int8_t x, y;
} Vector2i8;

class RGB32
{
private:
  uint8_t r, g, b;

public:
  RGB32() : r(0), g(0), b(0) {}
  RGB32(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

  void CreateRGB(uint32_t rgb)
  {
    r = (rgb >> 16) & 0xFF;
    g = (rgb >> 8) & 0xFF;
    b = (rgb) & 0xFF;
  }

  uint32_t ReturnRGB()
  {
    uint32_t rgb = 0;
    rgb |= 0 << 24;
    rgb |= r << 16;
    rgb |= g << 8;
    rgb |= b;

    return rgb;
  }

  void Multiply(float multiplier)
  {
    r *= multiplier;
    g *= multiplier;
    b *= multiplier;
  }

  void Display() const { cout << "R: " << static_cast<int>(r) << ", G: " << static_cast<int>(g) << ", B: " << static_cast<int>(b) << std::endl; }
};

typedef struct
{
  int width, height, size;
  uint32_t *pixels;
} DisplayData;

std::unordered_map<std::string, std::string> readConfig()
{
  const char *filename = "config.txt";

  std::unordered_map<std::string, std::string> config;
  std::string line;

  std::ifstream fileExists(filename);
  if (!fileExists.good())
  {
    std::ofstream file(filename);
    file << "fullscreen=false\n";
    file << "width=1280\n";
    file << "height=720\n";
    file << "resolutionPercentage=100\n";
  }

  std::ifstream file(filename);

  while (std::getline(file, line))
  {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream iss(line);
    std::string key, value;

    if (std::getline(iss, key, '=') && std::getline(iss, value))
    {
      config[key] = value;
    }
  }

  return config;
}

long GetMicroTime()
{
  auto const now = std::chrono::steady_clock::now();

  auto const duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

int CalculateAverageFps(int executionTime)
{
  const int FPS_HISTORY_SIZE = 8;

  static int fpsHistory[8];

  for (int i = FPS_HISTORY_SIZE; i >= 0; i--)
  {
    int nexti = i + 1;
    if (nexti <= FPS_HISTORY_SIZE - 1)
    {
      fpsHistory[nexti] = fpsHistory[i];
    }
  }
  fpsHistory[0] = 1000000 / executionTime;

  int32_t sumFps = 0;
  for (int i = 0; i < FPS_HISTORY_SIZE; i++)
  {
    sumFps += fpsHistory[i];
  }
  const int avgFps = sumFps / FPS_HISTORY_SIZE;
  return avgFps;
}

Vector2 rotate2d(float x, float y, float ang)
{
  const float x1 = -(x * cosf(ang) - y * sinf(ang));
  const float y1 = -(x * sinf(ang) + y * cosf(ang));

  return Vector2{x1, y1};
}

int main(int, char **)
{
  auto config = readConfig();

  int windowWidth = 1280;
  int windowHeight = 720;
  float resPercentage = 100;
  int windowMode = SDL_WINDOW_SHOWN;

  try
  {
    windowWidth = std::stoi(config["width"]);
    windowHeight = std::stoi(config["height"]);

    resPercentage = std::stoi(config["resolutionPercentage"]);

    if (config["fullscreen"] == "true")
    {
      windowMode = SDL_WINDOW_FULLSCREEN;
    }
    else
    {
      windowMode = SDL_WINDOW_SHOWN;
    }
  }
  catch (int error)
  {
    windowHeight = 1280;
    windowWidth = 720;
    resPercentage = 100;
    windowMode = SDL_WINDOW_SHOWN;
  }

  int scrw = windowWidth * (resPercentage / 100.0);
  int scrh = windowHeight * (resPercentage / 100.0);

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    SDL_Log("SDL video could not initialize! SDL_Error: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, windowMode);
  if (window == NULL)
  {
    SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
  {
    SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_RenderSetLogicalSize(renderer, scrw, scrh);

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, scrw, scrh);
  if (texture == NULL)
  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // READ spheres
  // DATA 6
  const std::vector<std::array<float, 4>> sphereDataRaw = {
      {-0.3, -0.8, 3, 0.6},
      {0.9, -1.4, 3.5, 0.35},
      {0.7, -0.45, 2.5, 0.4},
      {-0.5, -0.3, 1.5, 0.25},
      {1.0, -0.2, 1.5, 0.2},
      {-0.1, -0.2, 1.25, 0.2},
  };
  const int spheresCount = sphereDataRaw.size();

  // DIM c(spheres, 9), r(spheres), q(spheres), cl(4) AS _UNSIGNED LONG
  std::vector<std::array<float, 9>> c(spheresCount);
  std::vector<float> r(spheresCount);
  std::vector<float> q(spheresCount);

  // w = scrw / 2
  // h = scrh / 4
  // s = 0
  int w = scrw / 2;
  int h = scrh / 4;
  float s = 0;

  // cl(1) = _RGB32(120, 65, 45) ' shaddow
  // cl(2) = _RGB32(0, 0, 100)
  // cl(3) = _RGB32(255, 255, 0)
  // cl(4) = _RGB32(0, 0, 200)
  std::array<uint32_t, 4> cl = {RGB32(120, 65, 45).ReturnRGB(), RGB32(0, 0, 100).ReturnRGB(), RGB32(255, 255, 0).ReturnRGB(), RGB32(0, 0, 200).ReturnRGB()};

  // RANDOMIZE TIMER
  std::mt19937 gen(std::random_device());
  std::uniform_real_distribution<> randFloat(0.0, 1.0);

  // FOR k = 1 TO spheres
  //     READ a, b, c, d
  //     c(k, 5) = a
  //     c(k, 2) = -(d + .3 + 2 * RND(1))
  //     c(k, 6) = c
  //     c(k, 4) = .1
  //     r = d
  //     r(k) = r
  //     q(k) = r * r
  // NEXT k
  for (int k = 0; k < spheresCount; ++k)
  {
    const auto &data = sphereDataRaw[k];
    const float initial_x = data[0];
    const float initial_y_for_bounce = data[1];
    const float initial_z = data[2];
    const float radius = data[3];

    c[k][4] = initial_x;

    c[k][1] = -(initial_y_for_bounce + 0.3 + 2.0 * randFloat(gen));
    c[k][5] = initial_z;
    c[k][3] = 0.1;
    r[k] = radius;
    q[k] = radius * radius;
  }

  // me(1) = -0.2: me(2) = -3: me(0) = c(1, 1): me(3) = _PI
  std::array<float, 4> me = {c[0][4], -0.2, -3.0, M_PI};

  float mouseX = 0.0f;
  float mouseY = 0.0f;

  const uint8_t *keyStates = SDL_GetKeyboardState(nullptr);
  bool limitSpeed = false;
  float deltaTime = 1.0f;
  long currentTime = GetMicroTime();

  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_Event event;
  bool running = true;
  while (running)
  {
    long startTime = GetMicroTime();

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_MOUSEMOTION:
        if (SDL_GetRelativeMouseMode())
        {
          // mousex = 0: mousey = 0: WHILE _MOUSEINPUT: mousex = mousex + _MOUSEMOVEMENTX: mousey = mousey + _MOUSEMOVEMENTY: WEND
          mouseX += event.motion.xrel;
          mouseY += event.motion.yrel;
        }
        break;
      }
    }

    if (keyStates[SDL_SCANCODE_ESCAPE])
    {
      running = false;
      break;
    }

    // lock the texture
    uint32_t *pixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch) != 0)
    {
      SDL_DestroyTexture(texture);
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      printf("SDL_LockTexture Error: %s\n", SDL_GetError());
      SDL_Quit();
    }

    // FOR k = 1 TO spheres
    //     c(k, 4) = c(k, 4) + .01
    //     ck = c(k, 2) + c(k, 4)
    //     IF -ck < r(k) THEN c(k, 4) = -.2: ck = -r(k)
    //     c(k, 2) = ck
    // NEXT k
    for (int k = 0; k < spheresCount; k++)
    {
      c[k][3] = c[k][3] + 0.01 * deltaTime / 5;
      float ck = c[k][1] + c[k][3] * deltaTime / 5;
      if (-ck < r[k])
      {
        c[k][3] = -0.2;
        ck = -r[k];
      }
      c[k][1] = ck;
    }

    // me(3) = me(3) + mousex * .02
    me[3] = me[3] + mouseX * 0.01;

    if (me[3] < -M_PI)
    {
      me[3] += 2 * M_PI;
    }
    else if (me[3] > M_PI)
    {
      me[3] -= 2 * M_PI;
    }

    // h = h + mousey * .5
    h = h + mouseY * 6;

    const float max = 1000;
    const float min = -600;
    if (h > max)
    {
      h = max;
    }
    else if (h < min)
    {
      h = min;
    }

    mouseX = 0;
    mouseY = 0;

    // k_a = _KEYDOWN(ASC("a")): k_d = _KEYDOWN(ASC("d")): k_w = _KEYDOWN(ASC("w")): k_s = _KEYDOWN(ASC("s"))
    uint8_t kW = keyStates[SDL_SCANCODE_W] ? 1 : 0;
    uint8_t kA = keyStates[SDL_SCANCODE_A] ? 1 : 0;
    uint8_t kD = keyStates[SDL_SCANCODE_D] ? 1 : 0;
    uint8_t kS = keyStates[SDL_SCANCODE_S] ? 1 : 0;

    // go_ang = -(k_a * 90 + k_d * -90 + k_s * 180 - 180) * (_PI / 180) - me(3)
    // go = (k_a + k_d + k_w + k_s = -1) * .2
    float goAng = -(kA * 90 + kD * -90 + kW * 180.0 - 180.0) * (M_PI / 180.0) - me[3];

    // go = (kA + kD + kW + kS == -1) ? 0.2 * deltaTime : 0.0;
    float go = 0.0;
    uint8_t wasd = kA + kD + kW + kS;
    if (wasd == 1)
    {
      go = wasd / 20.0 * deltaTime;
    }

    // me(0) = me(0) + SIN(go_ang) * go
    // me(2) = me(2) - COS(go_ang) * go
    me[0] = me[0] + sinf(goAng) * go;
    me[2] = me[2] - cosf(goAng) * go;

    // me(1) = me(1) + (_KEYDOWN(ASC("+")) - _KEYDOWN(ASC("-"))) * .02
    me[1] = me[1] + (keyStates[SDL_SCANCODE_LCTRL] - keyStates[SDL_SCANCODE_SPACE]) * 0.03 * deltaTime;
    if (me[1] > -0.1)
    {
      me[1] = -0.1;
    }

    // 'spheres rotating
    // FOR k = 1 TO spheres
    //     c(k, 1) = c(k, 5) - me(0)
    //     c(k, 3) = c(k, 6) - me(2)
    //     rotate_2d c(k, 1), c(k, 3), me(3)
    //     c(k, 1) = c(k, 1) + me(0)
    //     c(k, 3) = c(k, 3) + me(2)
    // NEXT k
    for (int k = 0; k < spheresCount; k++)
    {
      c[k][0] = c[k][4] - me[0];
      c[k][2] = c[k][5] - me[2];

      Vector2 rotated = rotate2d(c[k][0], c[k][2], me[3]);
      c[k][0] = rotated.x;
      c[k][2] = rotated.y;

      c[k][0] = c[k][0] + me[0];
      c[k][2] = c[k][2] + me[2];
    }

    for (int iCanvas = 0; iCanvas < scrh; iCanvas++)
    {
      int iBasicSimulated = scrh - iCanvas;
      for (int j = 0; j < scrw; j++)
      {
        // x = me(0): y = me(1): z = me(2): ba = 3
        float x = me[0];
        float y = me[1];
        float z = me[2];
        byte ba = 3;

        // dx = j - w: dy = h - i: dz = scrw
        float dx = j - w;
        float dy = h - iBasicSimulated;
        float dz = scrw;

        // dd = dx * dx + dy * dy + dz * dz
        float dd = dx * dx + dy * dy + dz * dz;

        // neverend_cycle_agent = 0
        int neverEndCycleAgent = 0;

        // DO: neverend_cycle_agent = neverend_cycle_agent + 1: IF neverend_cycle_agent > 2000 THEN EXIT DO
        do
        {
          neverEndCycleAgent++;
          if (neverEndCycleAgent > 2000)
            break;

          // n = (y >= 0 OR dy <= 0) '* -1   <<< Makes $1000 for knowing where to tap the hammer
          // IF n = 0 THEN s = (y / dy) * -1
          // dd_temp = 1 / SQR(dd)
          int n = y >= 0 || dy <= 0 ? -1 : 0;
          if (n == 0)
            s = (y / dy) * -1;

          float ddTemp = 1 / sqrtf(dd);

          // FOR k = 1 TO spheres
          for (int k = 0; k < spheresCount; k++)
          {
            // px = c(k, 1) - x: py = c(k, 2) - y: pz = c(k, 3) - z
            const float px = c[k][0] - x;
            const float py = c[k][1] - y;
            const float pz = c[k][2] - z;

            // pp = px * px + py * py + pz * pz
            // sc = px * dx + py * dy + pz * d
            const float pp = px * px + py * py + pz * pz;
            float sc = px * dx + py * dy + pz * dz;

            // IF sc > 0 THEN
            if (sc > 0)
            {
              // bb = sc * sc / dd
              // aa = q(k) - pp + bb
              float bb = sc * sc / dd;
              float aa = q[k] - pp + bb;

              // IF aa > 0 THEN
              if (aa > 0)
              {
                // sc = (SQR(bb) - SQR(aa)) * dd_temp
                sc = (sqrtf(bb) - sqrtf(aa)) * ddTemp;

                // IF sc < s OR n < 0 THEN n = k: s = sc
                if (sc < s || n < 0)
                {
                  n = k + 1; // + 1 important to make reflections work
                  s = sc;
                }
              }
            }
          }

          // IF n < 0 THEN
          if (n < 0)
          {
            //  PSET (j, scrh - i), _RGB32(128 * (scrh - i) / scrh + 128 * (dy * dy / dd), 128 * (scrh - i) / scrh + 128 * (dy * dy / dd), 200 + 55 * (dy * dy / dd))
            RGB32 rgb = RGB32{uint8_t(128 * (scrh - iBasicSimulated) / scrh + 128 * (dy * dy / dd)), uint8_t(128 * (scrh - iBasicSimulated) / scrh + 128 * (dy * dy / dd)), uint8_t(200 + 55 * (dy * dy / dd))};
            pixels[iCanvas * scrw + j] = rgb.ReturnRGB();
            break;
          }
          else
          {
            // dx = dx * s: dy = dy * s: dz = dz * s: dd = dd * s * s
            dx = dx * s;
            dy = dy * s;
            dz = dz * s;

            // x = x + dx: y = y + dy: z = z + dz
            x = x + dx;
            y = y + dy;
            z = z + dz;

            // IF n <> 0 THEN
            if (n != 0)
            {
              //  nx = x - c(n, 1): ny = y - c(n, 2): nz = z - c(n, 3)
              const int sphereIdx = n - 1;
              const float nx = x - c[sphereIdx][0];
              const float ny = y - c[sphereIdx][1];
              const float nz = z - c[sphereIdx][2];

              // nn = nx * nx + ny * ny + nz * nz
              // l = 2 * (dx * nx + dy * ny + dz * nz) / nn
              const float nn = nx * nx + ny * ny + nz * nz;
              const float l = (2.0 * (dx * nx + dy * ny + dz * nz)) / nn;

              // dx = dx - nx * l: dy = dy - ny * l: dz = dz - nz * l
              dx = dx - nx * l;
              dy = dy - ny * l;
              dz = dz - nz * l;
            }
            else
            {
              // FOR k = 1 TO spheres
              for (int k = 0; k < spheresCount; k++)
              {
                // u = c(k, 1) - x
                // v = c(k, 3) - z
                const float u = c[k][0] - x;
                const float v = c[k][2] - z;

                // IF u * u + v * v <= q(k) THEN ba = 1: EXIT FOR
                if (u * u + v * v <= q[k])
                {
                  ba = 1;
                  break;
                }
              }

              // x2 = x - me(0): z2 = z - me(2): rotate_2d x2, z2, -me(3): x2 = x2 + me(0): z2 = z2 + me(2) 'field rotating
              float x2 = x - me[0];
              float z2 = z - me[2];
              Vector2 rotatedField = rotate2d(x2, z2, -me[3]);
              x2 = rotatedField.x + me[0];
              z2 = rotatedField.y + me[2];

              // IF (x2 - INT(x2) > .5) = (z2 - INT(z2) > .5) THEN 'x,y pepita size
              // cout << x2 << endl;
              if ((int(roundf(x2)) % 2 == 0) == (int(roundf(z2)) % 2 == 0))
              {
                // PSET (j, scrh - i), cl(ba)
                pixels[iCanvas * scrw + j] = cl[ba - 1];
              }
              else
              {
                // PSET (j, scrh - i), cl(ba + 1)
                pixels[iCanvas * scrw + j] = cl[ba + 1 - 1];
              }
              break;
            }
          }
        } while (true);
      }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    if (limitSpeed)
    {
      int executionTime = GetMicroTime() - startTime;
      int timeToSleep = 16666 - executionTime;

      if (timeToSleep > 0)
      {
        std::this_thread::sleep_for(std::chrono::microseconds(timeToSleep));
      }
    }

    long const elapsedTime = GetMicroTime() - currentTime;
    long const executionTimeWithSleep = GetMicroTime() - startTime;

    int const avgFps = CalculateAverageFps(executionTimeWithSleep);
    deltaTime = (GetMicroTime() - startTime) * 60.0f / 1000000.0f;

    if (elapsedTime >= 1000000)
    {
      const string title = to_string(scrw) + "x" + to_string(scrh) + " - " + to_string(avgFps) + " fps";

      SDL_SetWindowTitle(window, title.c_str());
      currentTime = GetMicroTime();
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
