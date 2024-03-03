#include "entity.h"
#include "logger.h"
#include "model.h"
#include "persister.h"
#include <memory>
#include <spdlog/common.h>
#define GLFW_EXPOSE_NATIVE_X11
#include "engine.h"
#include "blocks.h"
#include "assets.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  engine->controls->mouseCallback(window, xpos, ypos);
}

void Engine::registerCursorCallback() {
  glfwSetWindowUserPointer(window, (void *)this);
  glfwSetCursorPosCallback(window, mouseCallback);
}

void Engine::setupRegistry() {
  registry = make_shared<EntityRegistry>();
  shared_ptr<SQLPersister> postionablePersister =
      make_shared<PositionablePersister>(registry);
  registry->addPersister(postionablePersister);
  shared_ptr<SQLPersister> modelPersister =
      make_shared<ModelPersister>(registry);
  registry->addPersister(modelPersister);
  shared_ptr<SQLPersister> lightPersister =
      make_shared<LightPersister>(registry);
  registry->addPersister(lightPersister);
  registry->createTablesIfNeeded();
}

Engine::Engine(GLFWwindow* window, char** envp): window(window) {
  setupRegistry();
  registry->loadAll();
  loggerVector = make_shared<LoggerVector>();
  auto imGuiSink = make_shared<ImGuiSink>(loggerVector);
  loggerSink = make_shared<LoggerSink>(fileSink, imGuiSink);
  logger = make_shared<spdlog::logger>("engine", loggerSink);
  logger->set_level(spdlog::level::debug);
  initialize();
  // WARNING: need to fix this to do updates, not always insert
  wm->createAndRegisterApps(envp);
  glfwFocusWindow(window);
  wire();
  registerCursorCallback();
  initImGui();
  }

Engine::~Engine() {
  // may want to remove this because it might be slow on shutdown
  // when trying to get fast dev time
  registry->saveAll();
  delete controls;
  delete renderer;
  delete world;
  delete camera;
  //delete wm;
  delete api;
}

void Engine::initImGui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
}

void Engine::initialize(){
  auto texturePack = blocks::initializeBasicPack();
  wm = new WM(glfwGetX11Window(window), loggerSink);
  camera = new Camera();
  world = new World(registry, camera, texturePack, "/home/collin/midtown/", true, loggerSink);
  api = new Api("tcp://*:3333", world);
  renderer = new Renderer(registry, camera, world, texturePack);
  controls = new Controls(wm, world, camera, renderer, texturePack);
  wm->registerControls(controls);
}


void Engine::wire() {
  world->attachRenderer(renderer);
  wm->attachWorld(world);
  wm->addAppsToWorld();
  //world->loadLatest();
  //world->loadMinecraft();
}

void Engine::renderImGui(double &fps, int frameIndex, const vector<double> &frameTimes) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  // ImGui::ShowDemoWindow();
  // ImGui::SetNextWindowSize(ImVec2(120, 5));
  if (frameIndex == 0) {
    fps = 0;
    for (int i = 0; i < 20; i++) {
      fps = fps + frameTimes[i];
    }
    fps /= 20.0;
    if (fps > 0)
      fps = 1.0 / fps;
  }
  ImGui::Begin("HackMatrix");
  vector<string> debugMessages = loggerVector->fetch();
  if (ImGui::BeginTabBar("Developer Menu")) {
    if (ImGui::BeginTabItem("FPS")) {
      ImGui::Text("%f fps", fps);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Debug Log")) {
      for (const auto &msg : debugMessages) {
        ImGui::TextWrapped("%s", msg.c_str());
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar(); // Close the tab bar
  }
  ImGui::End();
  ImGui::Render();
}

void Engine::loop() {
  vector<double> frameTimes(20, 0);
  double frameStart;
  int frameIndex = 0;
  double fps;
  try {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      renderImGui(fps, frameIndex, frameTimes);
      frameStart = glfwGetTime();
      world->tick();
      renderer->render();
      api->mutateWorld();
      wm->mutateWorld();
      controls->poll(window, camera, world);

      frameTimes[frameIndex] = glfwGetTime() - frameStart;
      frameIndex = (frameIndex + 1) % 20;

      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }
  } catch (const std::exception &e) {
    logger->error(e.what());
    throw;
  }
}
