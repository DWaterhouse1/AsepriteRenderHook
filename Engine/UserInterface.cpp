#include "UserInterface.h"

namespace wrengine
{
UserInterface::UserInterface(
	Window& window,
	Device& device,
	VkRenderPass renderPass,
	uint32_t imageCount) : 
	m_device{ device }
{
   // set up imgui descriptor pool
   VkDescriptorPoolSize pool_sizes[] = {
     {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
     {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
     {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
     {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
     {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
     {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
     {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000} };
   VkDescriptorPoolCreateInfo pool_info = {};
   pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
   pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
   pool_info.pPoolSizes = pool_sizes;
   if (vkCreateDescriptorPool(
     m_device.device(),
     &pool_info,
     nullptr,
     &m_descriptorPool) != VK_SUCCESS)
   {
     throw std::runtime_error("failed to set up descriptor pool for imgui");
   }

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   (void)io;
   // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
   // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

   if (enableDocking)
   {
     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;          // Enable Docking
     io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;        // Enable Multiple viewports
   }


   // Setup Dear ImGui style
   ImGui::StyleColorsDark();
   // ImGui::StyleColorsClassic();

   ImGui_ImplGlfw_InitForVulkan(window.getGlfwWindow(), true);
   ImGui_ImplVulkan_InitInfo initInfo{};
   initInfo.Instance = m_device.getInstance();
   initInfo.PhysicalDevice = m_device.getPhysicalDevice();
   initInfo.Device = m_device.device();
   initInfo.QueueFamily = m_device.getGraphicsQueueFamily();
   initInfo.Queue = m_device.graphicsQueue();
   initInfo.PipelineCache = VK_NULL_HANDLE; //  pipeline cache is a potential optimisation, ignore for now
   initInfo.DescriptorPool = m_descriptorPool;
   initInfo.Allocator = VK_NULL_HANDLE; // not using an allocator yet
   initInfo.MinImageCount = 2;
   initInfo.ImageCount = imageCount;
   initInfo.CheckVkResultFn = check_vk_result;
   ImGui_ImplVulkan_Init(&initInfo, renderPass);

   // upload the fonts using a single time command buffer
   VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
   ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
   device.endSingleTimeCommands(commandBuffer);
   ImGui_ImplVulkan_DestroyFontUploadObjects();
}

UserInterface::~UserInterface()
{
   vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
   ImGui_ImplVulkan_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}

/**
* Commence ImGui per frame operations
*/
void UserInterface::startFrame()
{
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}

/**
* Run ImGui end of frame operations.
*/
void UserInterface::endFrame()
{
  if (enableDocking)
  {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}

/**
* Render the ImGui frame and submit.
*/
void UserInterface::render(VkCommandBuffer commandBuffer)
{
   ImGui::Render();
   ImDrawData* drawData = ImGui::GetDrawData();
   ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

/**
* Records commands to render the ImGui example window. Copied from the ImGui
* demo code.
*/
void UserInterface::runExample()
{
  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can
  // browse its code to learn more about Dear ImGui!).
  if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named
  // window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!" and append into it.

    ImGui::Text(
      "This is some useful text.");  // Display some text (you can use a format strings too)
    ImGui::Checkbox(
      "Demo Window",
      &show_demo_window);  // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color",
      (float*)&clear_color);  // Edit 3 floats representing a color

    if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true
                                  // when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text(
      "Application average %.3f ms/frame (%.1f FPS)",
      1000.0f / ImGui::GetIO().Framerate,
      ImGui::GetIO().Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (show_another_window) {
    ImGui::Begin(
      "Another Window",
      &show_another_window);  // Pass a pointer to our bool variable (the window will have a
                              // closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me")) show_another_window = false;
    ImGui::End();
  }
}
} // namespace wrengine