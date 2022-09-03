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

   m_elementManager = std::make_shared<ElementManager>();
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
* 
* @param commandBuffer The commund buffer to record ImGui submission commands.
*/
void UserInterface::render(VkCommandBuffer commandBuffer)
{
   ImGui::Render();
   ImDrawData* drawData = ImGui::GetDrawData();
   ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}
} // namespace wrengine