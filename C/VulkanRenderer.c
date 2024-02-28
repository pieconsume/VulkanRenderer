//Defs
 #define uint unsigned int
 #define GLFW_INCLUDE_VULKAN
 #include "GLFW/glfw3.h"
 #include <stdio.h>
 extern int   printf(const char* str, ...);
 extern void* malloc(unsigned long long size);
 extern void  free(void*);
 extern int   strcmp(const char* str1, const char* str2);
 extern void* memcpy(void* dest, const void* src, unsigned long long count);
 VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

int main()
{
 int width = 800, height = 600, maxframes = 2, curframe = 0;
 //Window init
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  void* window = glfwCreateWindow(width, height, "Vulkan Renderer", 0, 0);
 //Instance init
  VkInstance instance;
  VkApplicationInfo appinfo = {
   VK_STRUCTURE_TYPE_APPLICATION_INFO,
   0,                        //Next
   "Vulkan Renderer",        //Name
   VK_MAKE_VERSION(1, 0, 0), //AppVersion
   "None",                   //EngineName
   VK_MAKE_VERSION(1, 0, 0), //EngineVersion
   VK_API_VERSION_1_0};      //ApiVersion
  uint layercount = 1;
  const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
  VkDebugUtilsMessengerCreateInfoEXT debuginfo = {
   VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
   0,             //Next
   0,             //Flags
   VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
   VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
   debugCallback, //Callback
   0};            //Userdata
  uint glfwextcount, ourextcount = 1;
  const char** glfwexts = glfwGetRequiredInstanceExtensions(&glfwextcount);
  const char*  ourexts[] = { "VK_EXT_debug_utils" };
  const char** insexts = malloc((glfwextcount+ourextcount) * sizeof(char*));
  for (int i = 0; i < glfwextcount; i++) { insexts[i] = glfwexts[i]; }
  for (int i = 0; i < ourextcount; i++) { insexts[i+glfwextcount] = ourexts[i]; }
  uint insextcount = glfwextcount+ourextcount;
  VkInstanceCreateInfo instanceinfo = {
   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
   &debuginfo,  //Next
   0,           //Flags
   &appinfo,    //AppInfo
   layercount,  //LayerCount
   layers,      //Layers
   insextcount, //ExtCount
   insexts};    //Extensions
  if (vkCreateInstance(&instanceinfo, 0, &instance) != 0) { printf("Failed to create instance\n"); return 0; }
 //Surface init
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance, window, 0, &surface) != 0) { printf("Failed to create surface\n"); return 0; }
 //Physical device init
  VkPhysicalDevice phdevice;
  VkQueueFamilyProperties* qfamprops;
  VkSurfaceCapabilitiesKHR surfacecaps;
  VkSurfaceFormatKHR* surfaceformats;
  VkPresentModeKHR* surfacepresmodes;
  uint phcount, gfxfamidx, prsfamidx, formatcount, presmodecount;
  vkEnumeratePhysicalDevices(instance, &phcount, 0);
  if (phcount == 0) { printf("No available physical devices\n"); return 0; }
  VkPhysicalDevice* phdevices = malloc(phcount * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &phcount, phdevices);
  const char* devexts[] = { "VK_KHR_swapchain" }; //Extension checking must be changed if more than one extension is required
  VkExtensionProperties* phexts;
  for (int i = 0; i < phcount; i++){
   //Queue family check
    uint qfamcount;
    gfxfamidx = -1; prsfamidx = -1;
    vkGetPhysicalDeviceQueueFamilyProperties(phdevices[i], &qfamcount, 0);
    qfamprops = malloc(qfamcount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(phdevices[i], &qfamcount, qfamprops);
    for (int j = 0; j < qfamcount; j++) { 
     if (qfamprops[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) { gfxfamidx = j; }
     VkBool32 prssupport = 0;
     vkGetPhysicalDeviceSurfaceSupportKHR(phdevices[i], j, surface, &prssupport);
     if (prssupport) { prsfamidx = j; } 
     if (gfxfamidx != -1 && prsfamidx != -1) { break; } }
    free(qfamprops);
    if (gfxfamidx == -1 || prsfamidx == -1) { continue; }
   //Extensions check
    int validexts = 0;
    uint phextcount;
    vkEnumerateDeviceExtensionProperties(phdevices[i], 0, &phextcount, 0);
    phexts = malloc(phextcount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(phdevices[i], 0, &phextcount, phexts);
    for (int j = 0; j < phextcount; j++) { if (strcmp(devexts[0], phexts[j].extensionName) == 0) { validexts = 1; break; } }
    if (!validexts) { free(phexts); continue; }
   //Swapchain check
    vkGetPhysicalDeviceSurfaceFormatsKHR(phdevices[i], surface, &formatcount, 0);
    vkGetPhysicalDeviceSurfacePresentModesKHR(phdevices[i], surface, &presmodecount, 0);
    if (formatcount == 0 || presmodecount == 0) { free(phexts); continue; }
    surfaceformats = malloc(formatcount * sizeof(VkSurfaceFormatKHR));
    surfacepresmodes = malloc(presmodecount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(phdevices[i], surface, &formatcount, surfaceformats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(phdevices[i], surface, &presmodecount, surfacepresmodes);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phdevices[i], surface, &surfacecaps);
    phdevice = phdevices[i]; break; }
  if (phdevice == 0) { printf("No valid physical device found\n"); return 0; }
 //Logical device init
  VkDevice lgdevice;
  VkQueue gfxqueue, prsqueue;
  VkDeviceQueueCreateInfo* queueinfos;
  uint qcount;
  float qprio = 1.0f;
  if (gfxfamidx == prsfamidx){
   qcount = 1;
   VkDeviceQueueCreateInfo queueinfo = {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    0,         //Next
    0,         //Flags
    gfxfamidx, //Index
    1,         //QueueCount
    &qprio};   //QueuePriority
   queueinfos = &queueinfo;}
  else {
   qcount = 2;
   VkDeviceQueueCreateInfo gfxqueueinfo = {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    0,         //Next
    0,         //Flags
    gfxfamidx, //Index
    1,         //QueueCount
    &qprio};   //QueuePriority
   VkDeviceQueueCreateInfo prsqueueinfo = {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    0,         //Next
    0,         //Flags
    prsfamidx, //Index
    1,         //QueueCount
    &qprio};   //QueuePriority
   VkDeviceQueueCreateInfo qinfoarr[] = { gfxqueueinfo, prsqueueinfo };
   queueinfos = qinfoarr;}
  VkPhysicalDeviceFeatures features = {};
  VkDeviceCreateInfo deviceinfo = {
   VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
   0,          //Next
   0,          //Flags
   qcount,     //QueueCount
   queueinfos, //QueueInfos
   layercount, //LayerCount
   layers,     //Layers
   1,          //ExtCount
   devexts,    //Extensions
   &features}; //Features
  if (vkCreateDevice(phdevice, &deviceinfo, 0, &lgdevice) != 0) { printf("Unable to create logical device\n"); return 0; }
  vkGetDeviceQueue(lgdevice, gfxfamidx, 0, &gfxqueue);
  vkGetDeviceQueue(lgdevice, prsfamidx, 0, &prsqueue);
 //Swapchain init
  VkSwapchainKHR swapchain;
  VkSurfaceFormatKHR surfaceformat;
  VkPresentModeKHR prsmode;
  VkExtent2D extent = { width, height };
  uint minimagecount = surfacecaps.minImageCount+1;
  int formatset = 0;
  for (int i = 0; i < formatcount; i++) { if (surfaceformats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceformats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { formatset = 1; surfaceformat = surfaceformats[i]; break; } }
  if (!formatset) { surfaceformat = surfaceformats[0]; }
  prsmode = VK_PRESENT_MODE_FIFO_KHR; //This value is always available
  if (surfacecaps.currentExtent.width != -1) {
   extent.width = (extent.width > surfacecaps.minImageExtent.width ? extent.width : surfacecaps.minImageExtent.width) < surfacecaps.maxImageExtent.width ? extent.width : surfacecaps.maxImageExtent.width;
   extent.height = (extent.height > surfacecaps.minImageExtent.height ? extent.height : surfacecaps.minImageExtent.height) < surfacecaps.maxImageExtent.height ? extent.height : surfacecaps.maxImageExtent.height;}
  else { extent = surfacecaps.currentExtent; }
  minimagecount = surfacecaps.maxImageCount > 0 && minimagecount > surfacecaps.maxImageCount ? surfacecaps.maxImageCount : minimagecount;
  VkSwapchainCreateInfoKHR chaininfo = {
   VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
   0,                                   //Next
   0,                                   //Flags
   surface,                             //Surface
   minimagecount,                       //MinImageCount
   surfaceformat.format,                //ImageFormat
   surfaceformat.colorSpace,            //ColorSpace
   extent,                              //Extent
   1,                                   //ImageArrayLayers
   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //Usage
   VK_SHARING_MODE_EXCLUSIVE,           //ShareMode
   0,                                   //QFamIdxCount
   0,                                   //QFamIndices
   surfacecaps.currentTransform,        //PreTransform
   VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,   //CompositeAlpha
   prsmode,                             //PresentationMode
   1,                                   //Clipped
   0};                                  //OldSwapchain
  if (gfxfamidx != prsfamidx) {
   uint indices[] = { gfxfamidx, prsfamidx };
   chaininfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
   chaininfo.queueFamilyIndexCount = 2;
   chaininfo.pQueueFamilyIndices = indices;}
  if (vkCreateSwapchainKHR(lgdevice, &chaininfo, 0, &swapchain) != 0) { printf("Unable to create swapchain\n"); return 0; }
 //Image init
  VkImage* images;
  uint imagecount;
  vkGetSwapchainImagesKHR(lgdevice, swapchain, &imagecount, 0);
  images = malloc(imagecount * sizeof(VkImage));
  VkImageView* imageviews = malloc(imagecount * sizeof(VkImageView));
  vkGetSwapchainImagesKHR(lgdevice, swapchain, &imagecount, images);
  for (int i = 0; i < imagecount; i++){
   VkImageViewCreateInfo viewinfo = {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    0,                                          //Next
    0,                                          //Flags
    images[i],                                  //Image
    VK_IMAGE_VIEW_TYPE_2D,                      //ViewType
    surfaceformat.format,                       //Format
    { 0, 0, 0, 0 },                             //Components, VK_COMPONENT_SWIZZLE_IDENTITY
    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }}; //SubRsrcRange
   if (vkCreateImageView(lgdevice, &viewinfo, 0, &imageviews[i])) { printf("Unable to create image view\n"); return 0; } }
 //Renderpass init
  VkRenderPass renderpass;
  VkAttachmentDescription attinfo = {
   0,                                //Flags
   surfaceformat.format,             //Format
   VK_SAMPLE_COUNT_1_BIT,            //Samples
   VK_ATTACHMENT_LOAD_OP_CLEAR,      //LoadOp
   VK_ATTACHMENT_STORE_OP_STORE,     //StoreOp
   VK_ATTACHMENT_LOAD_OP_DONT_CARE,  //StencilLoadOp
   VK_ATTACHMENT_STORE_OP_DONT_CARE, //StencilStoreOp
   VK_IMAGE_LAYOUT_UNDEFINED,        //InitialLayout
   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR}; //FinalLayout
  VkAttachmentReference attref = {
   0,                                         //Attachment
   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}; //Layout
  VkSubpassDescription subpassdesc = {
   0,                               //Flags
   VK_PIPELINE_BIND_POINT_GRAPHICS, //BindPoint
   0,                               //InputAttCount
   0,                               //InputAtts
   1,                               //ColorAttCount
   &attref,                         //ColorAtts
   0,                               //ResolveAtts
   0,                               //DepthStencilAtt
   0,                               //PreserveAttCount
   0};                              //PreserveAtts
  VkSubpassDependency subpassdepend = {
   VK_SUBPASS_EXTERNAL,                           //SrcSubpass
   0,                                             //DstSubpass
   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, //SrcStageMask
   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, //DstStageMask
   0,                                             //SrcAccessMask
   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,          //DstAccessMask
   0};                                            //DependencyFlags
  VkRenderPassCreateInfo renderpassinfo = {
   VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
   0,               //Next
   0,               //Flags
   1,               //ColorAttCount
   &attinfo,        //ColorAtt
   1,               //SubpassCount
   &subpassdesc,    //Subpasses
   1,               //DependencyCount
   &subpassdepend}; //Dependencies
  if (vkCreateRenderPass(lgdevice, &renderpassinfo, 0, &renderpass) != 0) { printf("Unable to create render pass\n"); return 0; }
 //Pipeline init
  FILE *vertfd, *fragfd;
  if (fopen_s(&vertfd, "vert.spv", "rb") != 0 || fopen_s(&fragfd, "frag.spv", "rb") != 0) { printf("Unable to open a shader file\n"); return 0; }
  fseek(vertfd, 0, 2); fseek(fragfd, 0, 2); int vertsz = ftell(vertfd), fragsz = ftell(fragfd); fseek(vertfd, 0, 0); fseek(fragfd, 0, 0);
  uint *vertshdbuf = malloc(vertsz), *fragshdbuf = malloc(fragsz);
  fread(vertshdbuf, 1, vertsz, vertfd); fread(fragshdbuf, 1, fragsz, fragfd);
  VkShaderModuleCreateInfo vertinfo = {
   VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
   0,           //Next
   0,           //Flags
   vertsz,      //CodeSize
   vertshdbuf}; //Code
  VkShaderModuleCreateInfo fraginfo = {
   VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
   0,           //Next
   0,           //Flags
   fragsz,      //CodeSize
   fragshdbuf}; //Code
  VkShaderModule vertmod, fragmod;
  VkPipelineLayout pipelayout;
  VkPipeline gfxpipe;
  if (vkCreateShaderModule(lgdevice, &vertinfo, 0, &vertmod) != 0) { printf("Unable to create vertex shader module\n"); return 0; }
  if (vkCreateShaderModule(lgdevice, &fraginfo, 0, &fragmod) != 0) { printf("Unable to create fragment shader module\n"); return 0; }
  VkPipelineShaderStageCreateInfo vertstageinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
   0,                          //Next
   0,                          //Flags
   VK_SHADER_STAGE_VERTEX_BIT, //Stage
   vertmod,                    //Module
   "main",                     //Name
   0};                         //SpecInfo
  VkPipelineShaderStageCreateInfo fragstageinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
   0,                            //Next
   0,                            //Flags
   VK_SHADER_STAGE_FRAGMENT_BIT, //Stage
   fragmod,                      //Module
   "main",                       //Name
   0};                           //SpecInfo
  VkPipelineShaderStageCreateInfo stageinfos[] = { vertstageinfo, fragstageinfo };
  VkVertexInputBindingDescription vertbind = {
   0,                            //Binding
   24,                           //Stride
   VK_VERTEX_INPUT_RATE_VERTEX}; //InputRate
  VkVertexInputAttributeDescription vertposattr = {
   0,                          //Location
   0,                          //Binding
   VK_FORMAT_R32G32B32_SFLOAT, //Format
   0};                         //Offset
  VkVertexInputAttributeDescription vertcolattr = {
   1,                          //Location
   0,                          //Binding
   VK_FORMAT_R32G32B32_SFLOAT, //Format
   12};                        //Offset
  VkVertexInputAttributeDescription vertattrs[] = { vertposattr, vertcolattr };
  VkPipelineVertexInputStateCreateInfo vertinputinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
   0,          //Next
   0,          //Flags
   1,          //VertBindCount
   &vertbind,  //VertBinds
   2,          //VertAttrCount
   vertattrs}; //VertAttrs
  VkPipelineInputAssemblyStateCreateInfo assemblyinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
   0,                                   //Next
   0,                                   //Flags
   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, //Topology
   0};                                  //RestartEnable
  VkPipelineViewportStateCreateInfo viewportinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
   0,  //Next
   0,  //Flags
   1,  //ViewportCount
   0,  //Viewports
   1,  //ScissorCount
   0}; //Scissors
  VkPipelineRasterizationStateCreateInfo rastinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
   0,                       //Next
   0,                       //Flags
   0,                       //DepthClampEnable
   0,                       //RasterizerDiscardEnable
   VK_POLYGON_MODE_FILL,    //PolygonMode
   VK_CULL_MODE_NONE,       //CullMode
   VK_FRONT_FACE_CLOCKWISE, //FrontFace
   0,                       //DepthBiasEnable
   0,                       //DepthBiasConstant
   0,                       //DepthBiasClamp
   0,                       //DepthBiasSlope
   1.0f};                   //LineWidth
  VkPipelineMultisampleStateCreateInfo sampleinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
   0,                     //Next
   0,                     //Flags
   VK_SAMPLE_COUNT_1_BIT, //RastSamples
   0,                     //SampleEnable
   0.0f,                  //MinShading
   0,                     //Mask
   0,                     //AlphaToCoverage
   0};                    //AlphaToOne
  VkPipelineColorBlendAttachmentState coloratt = {
   0,     //BlendEnable
   0,     //SrcColorBlend
   0,     //DstColorBlend
   0,     //ColorOp
   0,     //SrcAlphaBlend
   0,     //DstAlphaBlend
   0,     //AlphaOp
   0x0F}; //WriteMask (RGBA)
  VkPipelineColorBlendStateCreateInfo colorinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
   0,                          //Next
   0,                          //Flags
   0,                          //LogicOpEnable
   VK_LOGIC_OP_COPY,           //LogicOp
   1,                          //AttCount
   &coloratt,                  //Atts
   { 0.0f, 0.0f, 0.0f, 0.0f}}; //BlendConstants
  VkDynamicState dynstates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynstateinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
   0,          //Next
   0,          //Flags
   2,          //StateCount
   dynstates}; //States
  VkPipelineLayoutCreateInfo layoutinfo = {
   VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
   0,  //Next
   0,  //Flags
   0,  //LayoutCount
   0,  //Layouts
   0,  //ConstRangeCount
   0}; //ConstRanges
  if (vkCreatePipelineLayout(lgdevice, &layoutinfo, 0, &pipelayout) != 0) { printf("Unable to create pipeline layout\n"); return 0; }
  VkGraphicsPipelineCreateInfo pipeinfo = {
   VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
   0,              //Next
   0,              //Flags
   2,              //StageCount
   stageinfos,     //Stages
   &vertinputinfo, //VertexInputState
   &assemblyinfo,  //AssemblyState
   0,              //TessellationState
   &viewportinfo,  //ViewportState
   &rastinfo,      //RasterizationState
   &sampleinfo,    //SampleState
   0,              //StencilState
   &colorinfo,     //BlendState
   &dynstateinfo,  //DynamicState
   pipelayout,     //Layout
   renderpass,     //Renderpass
   0,              //Subpass
   0,              //BasePipeHandle
   0,              //BasePipeIndex
   };
  if (vkCreateGraphicsPipelines(lgdevice, 0, 1, &pipeinfo, 0, &gfxpipe) != 0) {  printf("Unable to create graphics pipeline\n"); }
 //Framebuffer init
  VkFramebuffer* framebuffers = malloc(imagecount * sizeof(VkFramebuffer));
  for (int i = 0; i < imagecount; i++) {
   VkImageView atts[] = { imageviews[i] };
   VkFramebufferCreateInfo frameinfo = {
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    0,             //Next
    0,             //Flags
    renderpass,    //Renderpass
    1,             //AttCount
    atts,          //Atts
    extent.width,  //Width
    extent.height, //Height
    1};            //Layers
   if (vkCreateFramebuffer(lgdevice, &frameinfo, 0, &framebuffers[i]) != 0) { printf("Unable to create framebuffer\n"); return 0; } }
 //Commandpool init
  VkCommandPool cmdpool;
  VkCommandPoolCreateInfo poolinfo = {
   VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
   0,                                               //Next
   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, //Flags
   gfxfamidx};                                      //Index
  if (vkCreateCommandPool(lgdevice, &poolinfo, 0, &cmdpool) != 0) { printf("Unable to create command pool\n"); return 0; }
 //Buffer init
  VkBuffer vertbuf;
  VkDeviceMemory vertbufmem;
  float vertices[] = {
   -0.5f,-0.5f,+0.0f,+0.0f,+0.0f,+0.0f,
   -0.5f,+0.5f,+0.0f,+0.0f,+1.0f,+0.0f,
   +0.5f,+0.5f,+0.0f,+1.0f,+1.0f,+0.0f,
   +0.5f,-0.5f,+0.0f,+1.0f,+0.0f,+0.0f,};
  short int indices[] = {
   0, 1, 2, 3};
  VkBufferCreateInfo bufinfo = {
   VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
   0,                                 //Next
   0,                                 //Flags
   sizeof(vertices),                  //Size
   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, //Usage
   VK_SHARING_MODE_EXCLUSIVE,         //ShareMode
   0,                                 //QFamIndices
   0};                                //QFams
  if (vkCreateBuffer(lgdevice, &bufinfo, 0, &vertbuf) != 0) { printf("Failed to create vertex buffer\n"); return 0; }
  VkMemoryRequirements memreqs;
  VkPhysicalDeviceMemoryProperties memprops;
  vkGetBufferMemoryRequirements(lgdevice, vertbuf, &memreqs);
  vkGetPhysicalDeviceMemoryProperties(phdevice, &memprops);
  int memidx, memidxset; for (memidx = 0; memidx < memprops.memoryTypeCount; memidx++) { if (memreqs.memoryTypeBits & (1 < memidx) && (memprops.memoryTypes[memidx].propertyFlags & 0x06) == 0x06) { memidxset = 1; break; } };
  if (!memidxset) { printf("No valid memory index\n"); return 0; }
  VkMemoryAllocateInfo allocinfo = {
   VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
   0,            //Next
   memreqs.size, //Size
   memidx};      //Index
  if (vkAllocateMemory(lgdevice, &allocinfo, 0, &vertbufmem) != 0) { printf("Failed to allocate memory for vertex buffer\n"); return 0; }
  vkBindBufferMemory(lgdevice, vertbuf, vertbufmem, 0);
  void* vertdata;
  vkMapMemory(lgdevice, vertbufmem, 0, sizeof(vertices), 0, &vertdata);
  memcpy(vertdata, vertices, sizeof(vertices));
  vkUnmapMemory(lgdevice, vertbufmem);
 //Commandbuffer init
  VkCommandBuffer cmdbufs[maxframes];
  VkCommandBufferAllocateInfo cmdbufinfo = {
   VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
   0,                               //Next
   cmdpool,                         //Commandpool
   VK_COMMAND_BUFFER_LEVEL_PRIMARY, //Level
   maxframes};                      //Count
  if (vkAllocateCommandBuffers(lgdevice, &cmdbufinfo, cmdbufs) != 0) { printf("Unable to create command buffers\n"); return 0; }
 //Sync init
  VkSemaphore waitsems[maxframes];
  VkSemaphore donesems[maxframes];
  VkFence flightfences[maxframes];
  VkSemaphoreCreateInfo seminfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
  VkFenceCreateInfo fenceinfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 0, VK_FENCE_CREATE_SIGNALED_BIT };
  for (int i = 0; i < maxframes; i++){
   if (vkCreateSemaphore(lgdevice, &seminfo, 0, &waitsems[i]) != 0) { printf("Unable to create semaphore\n"); return 0; }
   if (vkCreateSemaphore(lgdevice, &seminfo, 0, &donesems[i]) != 0) { printf("Unable to create semaphore\n"); return 0; }
   if (vkCreateFence(lgdevice, &fenceinfo, 0, &flightfences[i]) != 0) { printf("Unable to create fence\n"); return 0; }
   };
 while (!glfwWindowShouldClose(window))
 {
  //Wait
   vkWaitForFences(lgdevice, 1, &flightfences[curframe], 1, -1);
   vkResetFences(lgdevice, 1, &flightfences[curframe]);
   uint imgidx;
   vkAcquireNextImageKHR(lgdevice, swapchain, -1, waitsems[curframe], 0, &imgidx);
  //Send commands
   //Init buffer
    vkResetCommandBuffer(cmdbufs[curframe], 0);
    VkCommandBufferBeginInfo begininfo = {
     VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
     0,  //Next
     0,  //Flags
     0}; //InheritInfo
    if (vkBeginCommandBuffer(cmdbufs[curframe], &begininfo) != 0) { printf("Failed to begin command buffer\n"); return 0; }
    VkClearValue clearval = { 0.0f, 0.0f, 0.0f, 1.0f };
    VkRenderPassBeginInfo passinfo = {
     VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
     0,                    //Next
     renderpass,           //Renderpass
     framebuffers[imgidx], //Framebuffer
     { { 0, 0 }, extent }, //RenderArea
     1,                    //ClearColorCount
     &clearval};           //ClearColors
   //Begin render pass
    vkCmdBeginRenderPass(cmdbufs[curframe], &passinfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdbufs[curframe], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxpipe);
    VkBuffer vertbufs[] = { vertbuf };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdbufs[curframe], 0, 1, vertbufs, offsets);
    VkViewport viewport = {
     0.0f,   //X
     0.0f,   //Y
     width,  //Width
     height, //Height
     0.0f,   //MinDepth
     1.0f};  //MaxDepth
    vkCmdSetViewport(cmdbufs[curframe], 0, 1, &viewport);
    VkRect2D scissor = {
     { 0, 0 }, //Offset
     extent }; //Extent
    vkCmdSetScissor(cmdbufs[curframe], 0, 1, &scissor);
    vkCmdDraw(cmdbufs[curframe], 3, 1, 0, 0);
    vkCmdEndRenderPass(cmdbufs[curframe]);
    if (vkEndCommandBuffer(cmdbufs[curframe]) != 0) { printf("Failed to end command buffer\n"); return 0; }
  //Submit queue
   VkSemaphore submitsems[] = { waitsems[curframe] };
   VkSemaphore sigsems[] = { donesems[curframe] };
   VkPipelineStageFlags submitflags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
   VkSubmitInfo submitinfo = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    0,                  //Next
    1,                  //SemCount
    submitsems,         //WaitSems
    submitflags,        //StageMask
    1,                  //CmdBufCount
    &cmdbufs[curframe], //CmdBufs
    1,                  //SigSemCount
    sigsems};           //SigSems
   if (vkQueueSubmit(gfxqueue, 1, &submitinfo, flightfences[curframe]) != 0) { printf("Failed to submit graphics queue\n"); return 0; }
  //Present queue
   VkSwapchainKHR swapchains[] = { swapchain };
   VkPresentInfoKHR prsinfo = {
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    0,          //Next
    1,          //WaitSemCount
    sigsems,    //WaitSems
    1,          //SwapchainCount
    swapchains, //Swapchains
    &imgidx,    //ImageIndex
    0};         //Results
   vkQueuePresentKHR(prsqueue, &prsinfo);
  curframe = (curframe + 1) % maxframes;
  glfwPollEvents();
 }
}

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
 printf("VkDebug: %s\n", callbackData->pMessage);
 return 0;
}