/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - window creation + main loop
*******************************************************************************/
#include <cstdint>
#include <thread>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <system/align.h>
#include <video/message_box.h>
#include <video/window_keycodes.h>
#include <video/window.h>

#if defined(_WINDOWS)
# include <system/api/windows_api.h>
# include <system/api/windows_app.h>
# include "../../../_img/test_win32/resources.h" // --> replace with your own resource file
# define _SYSTEM_STR(str) L"" str
#else
# define _SYSTEM_STR(str) str
#endif

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
# include <video/d3d11/buffer.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/buffer.h>
  namespace video_api = pandora::video::vulkan;
#endif

using namespace pandora::video;
using namespace video_api;

// --> an event storage system would be preferable
bool g_isRefreshed = false;
bool g_hasClicked = false;


// -- window creation -- -------------------------------------------------------

std::unique_ptr<Window> createWindow() { // throws on failure
# ifdef _WINDOWS
    auto mainIcon = WindowResource::buildIconFromPackage(MAKEINTRESOURCE(IDI_LOGO_BIG_ICON));
    auto cursor = WindowResource::buildCursorFromPackage(MAKEINTRESOURCE(IDC_BASE_CUR));
# else
    auto mainIcon = WindowResource::buildIconFromPackage("logo_big.png");
    auto cursor = WindowResource::buildCursorFromPackage("base_cur.png");
# endif
  
  Window::Builder builder;
  return builder.setDisplayMode(WindowType::window, WindowBehavior::globalContext,
                                ResizeMode::fixed)
        .setSize(800, 600)
        .setPosition(Window::Builder::centeredPosition(), Window::Builder::centeredPosition())
        .setIcon(mainIcon)
        .setCursor(cursor)
        .setBackgroundColor(WindowResource::buildColorBrush(WindowResource::rgbColor(0,0,0)))
        .create(_SYSTEM_STR("APP_WINDOW0"), _SYSTEM_STR("Example Window"));
}


// -- graphics pipeline creation -- --------------------------------------------

// vertex data type -- force 16-byte memory alignment
__align_type(16,
struct VertexData final {
  float position[4];
  float color[4];
});

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  constexpr const char* vertexShaderCode() noexcept {
    return 
      "struct VertexInputType {"
      "  float4 position : POSITION;"
      "  float4 color : COLOR;"
      "};"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR;"
      "};"
      "PixelInputType main(VertexInputType input) {"
      "  PixelInputType output;"
      "  output.position = input.position;"
      "  output.color = input.color;"
      "  return output;"
      "}";
  }
  constexpr const char* fragmentShaderCode() noexcept {
    return 
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR;"
      "};"
      "float4 main(PixelInputType input) : SV_TARGET {"
      "  return input.color;"
      "}";
  }
#else
    constexpr const char* vertexShaderCode() noexcept {
      return "";//TODO...
    }
    constexpr const char* fragmentShaderCode() noexcept {
      return "";//TODO...
    }
#endif

// ---

GraphicsPipeline createPipeline(Renderer& renderer, uint32_t width, uint32_t height) {
  // create shaders
  auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, vertexShaderCode(),
                                                      strlen(vertexShaderCode()), "main");
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
        0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT,
        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    size_t inputLayoutLength = sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr);
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer.resourceManager(),
                                                      inputLayoutDescr, inputLayoutLength);
# elif defined(_VIDEO_VULKAN_SUPPORT)
    //TODO...
# endif

  auto vertexShader = vertexShaderBuilder.createShader(renderer.resourceManager());
  auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, fragmentShaderCode(),
                                                 strlen(fragmentShaderCode()), "main")
                                        .createShader(renderer.resourceManager());
  
  // create viewport/scissor area
  Viewport viewport(0,0, width, height, 0.,1.);
  ScissorRectangle scissor(0,0, width, height);
  
  // configure pipeline
  return GraphicsPipeline::Builder(renderer)
         .setInputLayout(inputLayout)
         .setVertexTopology(VertexTopology::triangles)
         .attachShaderStage(vertexShader)
         .attachShaderStage(fragmentShader)
         .setRasterizerState(RasterizerParams{})               // default (back-culling, clockwise)
         .setDepthStencilState(DepthStencilParams(false,false))// depth test off
         .setBlendState(BlendParams(false))                    // blending off
         .setViewports(&viewport, size_t{ 1u }, &scissor, size_t{ 1u }, false)
         .build();
}


// -- window handlers -- -------------------------------------------------------

// window/hardware event handler --> should never throw!
bool onWindowEvent(Window* sender, WindowEvent event, uint32_t status, int32_t, int32_t, void*) {
  switch (event) {
    case WindowEvent::windowClosed: { // close event received -> confirmation
      auto reply = MessageBox::show("Confirmation", "Are you sure you want to exit?", 
                                    MessageBox::ActionType::yesNo, // "Yes/No"
                                    MessageBox::IconType::question, true);
      if (reply == MessageBox::Result::action2) // 2nd button for "Yes/No" -> "No"
        return true; // cancel default close event
      break;
    }
    case WindowEvent::stateChanged: {
      if ((WindowActivity)status != WindowActivity::hidden)
        g_isRefreshed = true;
      break;
    }
    default: break;
  }
  return false;
}

// size/position event handler --> should never throw!
bool onPositionEvent(Window*, PositionEvent, int32_t, int32_t, uint32_t, uint32_t) {
  g_isRefreshed = true;
  return false;
}

// keyboard event handler --> should never throw!
bool onKeyboardEvent(Window* sender, KeyboardEvent event, uint32_t keyCode, uint32_t) {
  switch (event) {
    case KeyboardEvent::keyDown: {
      if (keyCode == _P_VK_ESC) { // ESC pressed -> send close event to Window
        // never show message-box in fullscreen mode
        if (sender->displayMode() == WindowType::fullscreen)
          sender->show(Window::VisibilityCommand::minimize);
        Window::sendCloseEvent(sender->handle());
      }
      break;
    }
    default: break;
  }
  return false;
}

// mouse event handler --> should never throw!
bool onMouseEvent(Window*, MouseEvent event, int32_t, int32_t, int32_t index, uint8_t) {
  switch (event) {
    case MouseEvent::buttonDown: { // click -> report user action
      if ((MouseButton)index == MouseButton::left)
        g_hasClicked = true; // --> only for example
      break;
    }
    default: break;
  }
  return false;
}


// -- main loop -- -------------------------------------------------------------

inline void mainAppLoop() {
  try {
    auto window = createWindow();
    window->setMinClientAreaSize(400, 300);
    auto width = window->getClientSize().width;
    auto height = window->getClientSize().height;
    
    window->setWindowHandler(&onWindowEvent);
    window->setPositionHandler(&onPositionEvent);
    window->setKeyboardHandler(&onKeyboardEvent);
    window->setMouseHandler(&onMouseEvent, Window::CursorMode::visible);
    window->show();

    pandora::hardware::DisplayMonitor defaultMonitor;
    Renderer renderer(defaultMonitor);
    
    // scope for renderer components/resources
    // -> always destroyed before Renderer (even if exception)
    {
      // renderer components
      SwapChain::Descriptor params;
      params.width = width;
      params.height = height;
      params.framebufferCount = 2u;
      params.refreshRate = RefreshRate(60u, 1u);
      SwapChain framebuffer(DisplaySurface(renderer, window->handle()),
                            params, DataFormat::rgba8_sRGB);
      GraphicsPipeline pipeline = createPipeline(renderer, width, height);

      // vertices
      VertexData triangleVertices[] = {
        {{0.0f,0.5f,0.f,1.f},  {1.f,0.f,0.f,1.f}}, // pos{ X,Y,Z,div }, color{R,G,B,A}
        {{0.5f,-0.5f,0.f,1.f}, {0.f,1.f,0.f,1.f}},
        {{-0.5f,-0.5f,0.f,1.f},{0.f,0.f,1.f,1.f}}
      };
      uint32_t triangleIndices[] = { 0,1,2 };
      Buffer<ResourceUsage::staticGpu> vertexArray(renderer, BufferType::vertex,
                                                   sizeof(triangleVertices),
                                                   triangleVertices);
      Buffer<ResourceUsage::immutable> vertexIndex(renderer, BufferType::vertexIndex,
                                                   sizeof(triangleIndices),
                                                   triangleIndices);
      
      while (Window::pollEvents()) {
        // input + logic management
        if (g_hasClicked) {
          g_hasClicked = false; // unset flag
          
          // change vertex colors
          if (triangleVertices[0].color[0] == 1.f) { // change colors (black/cyan/magenta)
            triangleVertices[0].color[0] = 0.f;
            triangleVertices[1].color[2] = 1.f;
            triangleVertices[2].color[0] = 1.f;
          }
          else { // reset colors (red/green/blue)
            triangleVertices[0].color[0] = 1.f;
            triangleVertices[1].color[2] = 0.f;
            triangleVertices[2].color[0] = 0.f;
          }
          vertexArray.write(triangleVertices); // update vertex buffer
          g_isRefreshed = true; // force to redraw
        }
        
        // drawing
        if (g_isRefreshed) {
          renderer.setCleanActiveRenderTarget(framebuffer.getRenderTargetView(), nullptr);
          renderer.bindGraphicsPipeline(pipeline.handle());
       
          renderer.bindVertexArrayBuffer(0, vertexArray.handle(), sizeof(VertexData));
          renderer.bindVertexIndexBuffer(vertexIndex.handle(), VertexIndexFormat::r32_ui);
          renderer.drawIndexed(sizeof(triangleIndices)/sizeof(*triangleIndices));
          
          framebuffer.swapBuffers(); // display on screen
          g_isRefreshed = false; // unset flag
        }

        std::this_thread::sleep_for(std::chrono::microseconds(16600LL)); // 60Hz
      }
    }// end of scope -> resources destroyed (before Renderer)
  }
  catch (const std::exception& exc) {
    MessageBox::flushEvents();
    MessageBox::show("Fatal error", exc.what(), MessageBox::ActionType::ok,
                     MessageBox::IconType::error, true);
    exit(-1);
  }
}


// -- entry point -- -----------------------------------------------------------

#if defined(_WINDOWS)
  int APIENTRY WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE,
                       _In_ LPSTR cmdLine, _In_ int cmdShow) {
    pandora::system::WindowsApp::instance().init((pandora::system::AppInstanceHandle)appInstance);
    mainAppLoop();
    return 0;
  }
#else
  int main() {
    mainAppLoop();
    return 0;
  }
#endif