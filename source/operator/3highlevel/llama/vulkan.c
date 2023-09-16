#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
typedef unsigned int u32;
typedef unsigned short u16;


const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};




//instance
static VkInstance instance;
//device
static VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;
static VkDevice logicaldevice;
//compute
static int computeindex;
static VkQueue computeQueue;
static VkCommandPool computePool;
//graphic
static int graphicindex;
static VkQueue graphicQueue;
static VkCommandPool graphicPool;
//present
static int presentindex;
static VkQueue presentQueue;
//surface,swapchain
static VkSurfaceKHR surface = 0;
static VkSwapchainKHR swapChain = 0;
static VkExtent2D widthheight;
static uint32_t imagecount;
//attachment
struct attachment{
	VkFormat format;
	VkDeviceMemory memory;
	VkImage image;
	VkImageView view;
};
static struct attachment attachcolor[8];




int vulkan_exit()
{
	vkDestroyInstance(instance, 0);
	return 0;
}
void* vulkan_init(int cnt, const char** ext)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);

	VkLayerProperties availableLayers[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
	printf("vkEnumerateInstanceLayerProperties:\n");

	int j;
	for(j=0;j<layerCount;j++){
		printf("%4d:%s\n", j, availableLayers[j].layerName);
	}


	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = cnt;
	createInfo.ppEnabledExtensionNames = ext;
	createInfo.enabledLayerCount = 0;
	//createInfo.ppEnabledLayerNames = validationLayers;
	createInfo.pNext = 0;

	if (vkCreateInstance(&createInfo, 0, &instance) != VK_SUCCESS) {
		printf("failed to create instance!\n");
		return 0;
	}
	printf("instance=%p\n", instance);

/*
	VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
	debugReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMessageCallback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &debugReportCreateInfo, nullptr, &debugReportCallback));
*/
	printf("----------------instance ok----------------\n\n");
	return instance;
}
void* vulkan_surface_create()
{
	return 0;
}
void vulkan_surface_delete(VkSurfaceKHR face)
{
	vkDestroySurfaceKHR(instance, face, 0);
}




int checkDeviceProperties(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties prop;
	vkGetPhysicalDeviceProperties(device, &prop);

	int score = -1;
	printf("vkGetPhysicalDeviceProperties:\n");
	printf("	apiver=%x\n", prop.apiVersion);
	printf("	drvver=%x\n", prop.driverVersion);
	printf("	vendor=%x\n", prop.vendorID);
	printf("	device=%x\n", prop.deviceID);
	printf("	name=%s\n", prop.deviceName);
	printf("	type=");
	switch(prop.deviceType){
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		printf("other\n");
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		printf("igpu\n");
		score = 1;
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		printf("dgpu\n");
		score = 2;
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		printf("vgpu\n");
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		printf("cpu\n");
		break;
	default:
		printf("unknown\n");
		break;
	}

	printf("\n");
	return score;
}
int checkDeviceExtensionProperties(VkPhysicalDevice device, void* name) {
	uint32_t cnt;
	vkEnumerateDeviceExtensionProperties(device, 0, &cnt, 0);

	VkExtensionProperties ext[cnt];
	vkEnumerateDeviceExtensionProperties(device, 0, &cnt, ext);
	printf("vkEnumerateDeviceExtensionProperties:\n");

	int j;
	int ret = -1;
	for(j=0;j<cnt;j++) {
		printf("%d:	ver=%04d,str=%s\n", j, ext[j].specVersion, ext[j].extensionName);
		if(0 == strcmp(ext[j].extensionName, name)){
			if(ret < 0)ret = j;
		}
	}
	printf("=>VK_KHR_swapchain@%d\n\n",ret);
	return ret;
}
//#define VK_QUEUE_GRAPHICS_BIT         0x00000001
//#define VK_QUEUE_COMPUTE_BIT          0x00000002
//#define VK_QUEUE_TRANSFER_BIT         0x00000004
#define VK_QUEUE_SPARSE_BINDING_BIT   0x00000008
#define VK_QUEUE_PROTECTED_BIT        0x00000010
#define VK_QUEUE_VIDEO_DECODE_BIT_KHR 0x00000020
#define VK_QUEUE_VIDEO_ENCODE_BIT_KHR 0x00000040
int checkPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device, VkSurfaceKHR face, int what){
	//printf("dev=%p\n",device);

	uint32_t cnt = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &cnt, 0);

	VkQueueFamilyProperties fam[cnt];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &cnt, fam);

	int j;
	int firstgraphicwithpresent = -1;
	int firsttransfer = -1;
	int firstcompute = -1;
	int firstgraphic = -1;
	int firstpresent = -1;
	VkBool32 supportsurface = 0;
	printf("vkGetPhysicalDeviceQueueFamilyProperties:\n");
	for(j=0;j<cnt;j++) {
		printf("%d:	%d=", j, fam[j].queueFlags);
		if(fam[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR){	//64
			printf("encode,");
		}
		if(fam[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR){	//32
			printf("decode,");
		}
		if(fam[j].queueFlags & VK_QUEUE_PROTECTED_BIT){	//16
			printf("protected,");
		}
		if(fam[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT){	//8
			printf("sprase,");
		}
		if(fam[j].queueFlags & VK_QUEUE_TRANSFER_BIT){	//4
			printf("transfer,");
			if(firsttransfer < 0)firsttransfer = j;
		}
		if(fam[j].queueFlags & VK_QUEUE_COMPUTE_BIT){	//2
			printf("compute,");
			if(firstcompute < 0)firstcompute = j;
		}
		if(fam[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){	//1
			printf("graphic,");
			if(firstgraphic < 0)firstgraphic = j;
		}

		if(face){
			supportsurface = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, j, face, &supportsurface);

			if(supportsurface){
				printf("present");
				if(firstpresent < 0)firstpresent = j;

				if(fam[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
					if(firstgraphicwithpresent < 0)firstgraphicwithpresent = j;
				}
			}
		}

		printf("\n");
	}

	if(face){
		if( (firstgraphicwithpresent < 0) && (firstpresent < 0) )return -1;
	}
	if(what & VK_QUEUE_GRAPHICS_BIT){
		if(firstgraphic < 0)return -1;
	}
	if(what & VK_QUEUE_COMPUTE_BIT){
		if(firstcompute < 0)return -1;
	}
	return 1;
}
int checkSwapChain(VkPhysicalDevice device, VkSurfaceKHR face) {
	//format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, face, &formatCount, 0);
	if(0 == formatCount)return -1;

	VkSurfaceFormatKHR formats[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, face, &formatCount, formats);

	int j;
	printf("vkGetPhysicalDeviceSurfaceFormatsKHR:\n");
	for(j=0;j<formatCount;j++){
		printf("%d:	format=%08x,colorspace=%08x\n", j, formats[j].format, formats[j].colorSpace);
	}
	printf("\n");

	//presentmode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, face, &presentModeCount, 0);
	if(0 == presentModeCount)return -2;

	VkPresentModeKHR presentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, face, &presentModeCount, presentModes);

	printf("vkGetPhysicalDeviceSurfacePresentModesKHR:\n");
	for(j=0;j<formatCount;j++){
		printf("%d:	%08x=", j, presentModes[j]);
		switch(presentModes[j]){
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			printf("IMMEDIATE");
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			printf("MAILBOX");
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			printf("FIFO");
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			printf("FIFO_RELAXED");
			break;
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			printf("SHARED_DEMAND_REFRESH");
			break;
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			printf("SHARED_CONTINUOUS_REFRESH");
			break;
		default:
			printf("unknown");
			break;
		}
		printf("\n");
	}
	printf("\n");

	return 1;
}
int freephysicaldevice() {
	return 0;
}
void* initphysicaldevice(int what, VkSurfaceKHR face) {
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, 0);
	if(0 == count) {
		printf("error@vkEnumeratePhysicalDevices\n");
		return 0;
	}

	VkPhysicalDevice devs[count];
	vkEnumeratePhysicalDevices(instance, &count, devs);
	printf("vkEnumeratePhysicalDevices:\n");

	int j,best=-1;
	int chkdev,chkext,chkfam,chksur;
	physicaldevice = VK_NULL_HANDLE;
	for(j=0;j<count;j++) {
		printf("%d:physicaldevice{\n", j);
		chkdev = checkDeviceProperties(devs[j]);
		chkext = checkDeviceExtensionProperties(devs[j], VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		chkfam = checkPhysicalDeviceQueueFamilyProperties(devs[j], face, what);
		if( (chkdev > 0) && (chkext > 0) && (chkfam > 0) && (best < 0) ){
			if(face){
				chksur = checkSwapChain(devs[j], face);
				if(chksur > 0)best = j;
			}
			else{
				best = j;
			}
		}
		printf("score=%d,%d,%d,%d\n",chkdev,chkext,chkfam,chksur);
		printf("}\n\n");
	}
	if(best < 0){
		printf("no physicaldevice\n");
		return 0;
	}

	printf("=>choose device=%d(%p)\n", best, devs[best]);
	return devs[best];
}




int getPhysicalDeviceQueueFamilyProperties(
	VkPhysicalDevice device, VkSurfaceKHR face,
	int* gg, int* pp, int* cc)
{
	//printf("dev=%p\n",device);

	uint32_t cnt = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &cnt, 0);

	VkQueueFamilyProperties fam[cnt];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &cnt, fam);

	int j;
	int firstgraphicwithpresent = -1;
	int firsttransfer = -1;
	int firstcompute = -1;
	int firstgraphic = -1;
	int firstpresent = -1;
	VkBool32 supportsurface = 0;
	printf("vkGetPhysicalDeviceQueueFamilyProperties:\n");
	for(j=0;j<cnt;j++) {
		printf("%d:	%d=", j, fam[j].queueFlags);
		if(fam[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR){	//64
			printf("encode,");
		}
		if(fam[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR){	//32
			printf("decode,");
		}
		if(fam[j].queueFlags & VK_QUEUE_PROTECTED_BIT){	//16
			printf("protected,");
		}
		if(fam[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT){	//8
			printf("sprase,");
		}
		if(fam[j].queueFlags & VK_QUEUE_TRANSFER_BIT){	//4
			printf("transfer,");
			if(firsttransfer < 0)firsttransfer = j;
		}
		if(fam[j].queueFlags & VK_QUEUE_COMPUTE_BIT){	//2
			printf("compute,");
			if(firstcompute < 0)firstcompute = j;
		}
		if(fam[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){	//1
			printf("graphic,");
			if(firstgraphic < 0)firstgraphic = j;
		}

		supportsurface = 0;
		if(face){
			vkGetPhysicalDeviceSurfaceSupportKHR(device, j, face, &supportsurface);
		}
		if(supportsurface){
			printf("present");
			if(firstpresent < 0)firstpresent = j;

			if(fam[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
				if(firstgraphicwithpresent < 0)firstgraphicwithpresent = j;
			}
		}

		printf("\n");
	}

	if(face){		//onscreen: need graphic and present
		if(firstgraphicwithpresent >= 0){
			if(gg)gg[0] = firstgraphicwithpresent;
			if(pp)pp[0] = firstgraphicwithpresent;
		}
		if((firstgraphic >= 0) && (firstpresent >= 0)){
			if(gg)gg[0] = firstgraphic;
			if(pp)pp[0] = firstpresent;
		}
	}
	else{		//offscreen: need graphic
		if(firstgraphic >= 0){
			if(gg)gg[0] = firstgraphic;
		}
	}
	if(firstcompute > 0){
		if(cc)cc[0] = firstcompute;
	}

	return 0;
}
int freelogicaldevice() {
	return 0;
}
int initlogicaldevice(int what, VkSurfaceKHR face) {
	//queue index
	getPhysicalDeviceQueueFamilyProperties(physicaldevice, face, &graphicindex, &presentindex, &computeindex);
	printf("graphic,present=%d,%d, compute=%d\n",graphicindex,presentindex, computeindex);

	//queue create info
	VkDeviceQueueCreateInfo queueCreateInfos[3] = {};
	float queuePriority = 1.0f;
	int queuecount = 0;
	printf("queueCreateInfos:");
	if((what&VK_QUEUE_GRAPHICS_BIT) && (graphicindex >= 0)){
		printf("%d=graphic,", queuecount);
		queueCreateInfos[queuecount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[queuecount].queueFamilyIndex = graphicindex;
		queueCreateInfos[queuecount].queueCount = 1;
		queueCreateInfos[queuecount].pQueuePriorities = &queuePriority;
		queuecount += 1;
	}
	if((what&VK_QUEUE_COMPUTE_BIT) && (computeindex >= 0)){
		printf("%d=compute,", queuecount);
		queueCreateInfos[queuecount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[queuecount].queueFamilyIndex = presentindex;
		queueCreateInfos[queuecount].queueCount = 1;
		queueCreateInfos[queuecount].pQueuePriorities = &queuePriority;
		queuecount += 1;
	}
	if(face && (presentindex >= 0) && (graphicindex != presentindex)){
		printf("%d=present,", queuecount);
		queueCreateInfos[queuecount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[queuecount].queueFamilyIndex = presentindex;
		queueCreateInfos[queuecount].queueCount = 1;
		queueCreateInfos[queuecount].pQueuePriorities = &queuePriority;
		queuecount += 1;
	}
	printf("\n");


	//device feature
	VkPhysicalDeviceFeatures deviceFeatures = {};

	//device extension
	const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	//devicecreateinfo
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queuecount;
	createInfo.pQueueCreateInfos = queueCreateInfos;
	createInfo.enabledExtensionCount = face ? 1 : 0;
	createInfo.ppEnabledExtensionNames = deviceExtensions;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = 0;
	if (vkCreateDevice(physicaldevice, &createInfo, 0, &logicaldevice) != VK_SUCCESS) {
		printf("error@vkCreateDevice\n");
		return -1;
	}
	printf("logicaldevice=%p\n", logicaldevice);

	//graphic: queue, pool
	if((what&VK_QUEUE_GRAPHICS_BIT) && (graphicindex >= 0)){
		vkGetDeviceQueue(logicaldevice, graphicindex, 0, &graphicQueue);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicindex;
		if (vkCreateCommandPool(logicaldevice, &poolInfo, 0, &graphicPool) != VK_SUCCESS) {
			printf("error@vkCreateCommandPool\n");
		}
		printf("graphicpool=%p\n",graphicPool);
	}

	//compute: queue, pool
	if((what&VK_QUEUE_COMPUTE_BIT) && (computeindex >= 0)){
		vkGetDeviceQueue(logicaldevice, computeindex, 0, &computeQueue);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = computeindex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(logicaldevice, &poolInfo, 0, &computePool) != VK_SUCCESS) {
			printf("error@vkCreateCommandPool\n");
		}
		printf("computepool=%p\n",computePool);
	}

	//present: queue
	if(face){
		vkGetDeviceQueue(logicaldevice, presentindex, 0, &presentQueue);
	}
	return 0;
}




int freeswapchain() {
	return 0;
}
int initswapchain(VkSurfaceKHR face) {
	//capability
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicaldevice, face, &capabilities);

	if(capabilities.currentExtent.width != UINT32_MAX) {
		widthheight.width = capabilities.currentExtent.width;
		widthheight.height= capabilities.currentExtent.height;
	}
	else{
		int w = 1024, h = 768;
		if(w > capabilities.maxImageExtent.width)w = capabilities.maxImageExtent.width;
		if(w < capabilities.minImageExtent.width)w = capabilities.minImageExtent.width;
		if(h > capabilities.maxImageExtent.height)h = capabilities.maxImageExtent.height;
		if(h < capabilities.minImageExtent.height)h = capabilities.minImageExtent.height;

		widthheight.width = capabilities.currentExtent.width;
		widthheight.height= capabilities.currentExtent.height;
	}

	uint32_t imageCount = capabilities.minImageCount + 1;
	if((capabilities.maxImageCount > 0) && (imageCount > capabilities.maxImageCount)) {
		imageCount = capabilities.maxImageCount;
	}

	//format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicaldevice, face, &formatCount, 0);
	if(0 == formatCount)return -1;

	VkSurfaceFormatKHR formats[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicaldevice, face, &formatCount, formats);

	int j;
	VkSurfaceFormatKHR surfaceFormat = formats[0];
	for(j=0;j<formatCount;j++){
		if(	(formats[j].format == VK_FORMAT_B8G8R8A8_SRGB) &&
			(formats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) ){
			surfaceFormat = formats[j];
		}
	}

	//presentmode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicaldevice, face, &presentModeCount, 0);
	if(0 == presentModeCount)return -2;

	VkPresentModeKHR presentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicaldevice, face, &presentModeCount, presentModes);

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for(j=0;j<presentModeCount;j++){
		if(presentModes[j] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = presentModes[j];
		}
	}

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = face;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = widthheight;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {graphicindex, presentindex};
	if (graphicindex != presentindex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(logicaldevice, &createInfo, 0, &swapChain) != VK_SUCCESS) {
		printf("error@vkCreateSwapchainKHR\n");
		return -1;
	}
	printf("swapchain=%p\n",swapChain);


	vkGetSwapchainImagesKHR(logicaldevice, swapChain, &imagecount, 0);
	VkImage swapChainImages[imagecount];
	vkGetSwapchainImagesKHR(logicaldevice, swapChain, &imagecount, swapChainImages);

	printf("swapchain imagecount=%d\n", imagecount);
	for(j=0;j<imagecount; j++) {
		attachcolor[j].format = surfaceFormat.format;
		attachcolor[j].image = swapChainImages[j];
		attachcolor[j].memory = 0;

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[j];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = attachcolor[j].format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logicaldevice, &createInfo, 0, &attachcolor[j].view) != VK_SUCCESS) {
			printf("error@vkCreateImageView:%d\n",j);
		}
		printf("%d:image=%p,view=%p\n", j, swapChainImages[j], attachcolor[j].view);
	}
	return 0;
}




int vulkan_device_delete()
{
	if(surface){
		freeswapchain();
	}

	freelogicaldevice();

	freephysicaldevice();

	return 0;
}
void* vulkan_device_create(int what, VkSurfaceKHR face)
{
	//logicaldevice <- physicaldevice
	physicaldevice = initphysicaldevice(what, face);
	if(0 == physicaldevice)return 0;
	printf("----------------physicaldevice ok----------------\n\n");

	initlogicaldevice(what, face);
	if(0 == logicaldevice)return 0;
	printf("----------------logicaldevice and queue ok----------------\n\n");

	//swapchain <- physicaldevice, logicaldevice, surface
	if(face){
		initswapchain(face);
		surface = face;
		printf("----------------swapchain and image ok----------------\n\n");
	}

	return physicaldevice;
}




void vulkan_physicaldevice_logicdevice(VkPhysicalDevice* pdev, VkDevice* ldev)
{
	*pdev = physicaldevice;
	*ldev = logicaldevice;
}
void vulkan_graphicqueue_graphicpool(VkQueue* queue, VkCommandPool* pool)
{
	*queue = graphicQueue;
	*pool = graphicPool;
}
void vulkan_computequeue_computepool(VkQueue* queue, VkCommandPool* pool)
{
	*queue = computeQueue;
	*pool = computePool;
}
void vulkan_presentqueue_swapchain(VkQueue* queue, void** chain)
{
	*queue = presentQueue;
	*chain = swapChain;
}
void vulkan_widthheight_imagecount_attachcolor(VkExtent2D* wh, uint32_t* cnt, struct attachment* attach)
{
	wh->width = widthheight.width;
	wh->height = widthheight.height;

	*cnt = imagecount;

	int j;
	for(j=0;j<imagecount;j++){
		attach[j].format = attachcolor[j].format;
		attach[j].image = attachcolor[j].image;
		attach[j].view = attachcolor[j].view;
	}
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>


#define u64 unsigned long long
#ifdef _WIN32
#include <windows.h>
static u64 time_in_ns()
{
	LARGE_INTEGER count,freq;
	int ret = QueryPerformanceFrequency(&freq);
	if(ret && freq.QuadPart){
		ret = QueryPerformanceCounter(&count);
		//say("count=%lld,freq=%lld,time=%lld\n", count.QuadPart, freq.QuadPart, (u64)count.QuadPart*1000*1000 / (freq.QuadPart/1000));
		if(ret && count.QuadPart)return (u64)count.QuadPart*1000*1000 / (freq.QuadPart/1000);		//without (u64)=overflow, 10^9*count/freq = overflow
	}

	return 1000 * 1000 * GetTickCount64();
}
#elif __APPLE__
#include <mach/mach_time.h>
#define lseek64 lseek
static u64 time_in_ns()
{
	return mach_absolute_time();
}
#else
#include <time.h>
static u64 time_in_ns()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (u64)t.tv_sec*1000*1000*1000 + t.tv_nsec;
}
#endif


//VkQueue computeQueue;
//VkCommandPool computePool;
//
VkShaderModule shaderModule;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkFence computefence;
//
VkPipeline computepipeline;
VkPipelineCache pipelineCache;
VkPipelineLayout pipelineLayout;
//
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSet;
VkDescriptorSetLayout descriptorSetLayout;
//
VkBuffer hostBuffer[3];
VkDeviceMemory hostMemory[3];
VkBuffer deviceBuffer[3];
VkDeviceMemory deviceMemory[3];
//
int xdim = 16384;		//11008
int ydim = 32000;
int outputbuffersize = 0;
int vectorbuffersize = 0;
int matrixbuffersize = 0;
unsigned int pushconst[4] = {0, 0, 0, 0};





int findMemoryType(VkPhysicalDevice pdev, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(pdev, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}
VkResult createBuffer(
	VkBuffer* buffer, VkDeviceMemory* memory,
	VkPhysicalDevice pdev, VkDevice ldev,
	VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
	int size, void *data)
{
	//buffer handle
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(ldev, &bufferInfo, 0, buffer);


	//buffer memory
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(ldev, *buffer, &memReqs);

	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(pdev, &deviceMemoryProperties);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = findMemoryType(pdev, memReqs.memoryTypeBits, memoryPropertyFlags);
	vkAllocateMemory(ldev, &memAlloc, 0, memory);


	//copy
	if (data) {
		void *mapped = 0;
		vkMapMemory(ldev, *memory, 0, size, 0, &mapped);
		memcpy(mapped, data, size);
		vkUnmapMemory(ldev, *memory);
	}


	//bind
	vkBindBufferMemory(ldev, *buffer, *memory, 0);

	return VK_SUCCESS;
}


int createShaderModule(VkShaderModule* mod, char* url) {
	FILE* fp = fopen(url, "rb+");
	if(!fp)return -1;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	if(size <= 0){fclose(fp);return -2;}

	uint32_t* code = (uint32_t*)malloc(size);
	if(0 == code){fclose(fp);return -3;}

	fseek(fp, 0, SEEK_SET);
	if(fread(code,size,1,fp) < 1){fclose(fp);return -4;}
	fclose(fp);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = code;

	if (vkCreateShaderModule(logicaldevice, &createInfo, 0, mod) != VK_SUCCESS) {
		printf("error@vkCreateShaderModule\n");
		return -1;
	}
	return 0;
}
void createcomputepipeline()
{
	//descpool
	VkDescriptorPoolSize poolSizes[2] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = 3;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;
	int ret = vkCreateDescriptorPool(logicaldevice, &poolInfo, 0, &descriptorPool);
	if(VK_SUCCESS != ret){
		printf("error@vkCreateDescriptorPool\n");
		return;
	}
	printf("DescriptorPool=%p\n", descriptorPool);


	//descsetlayoutbinding
	VkDescriptorSetLayoutBinding setLayoutBindings[3] = {};
	setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setLayoutBindings[0].descriptorCount = 1;
	setLayoutBindings[0].binding = 0;
	setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setLayoutBindings[1].descriptorCount = 1;
	setLayoutBindings[1].binding = 1;
	setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setLayoutBindings[2].descriptorCount = 1;
	setLayoutBindings[2].binding = 2;
	setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


	//descriptorSetLayout = descsetlayoutbinding + xxx
	VkDescriptorSetLayoutCreateInfo descsetlayoutinfo = {};
	descsetlayoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descsetlayoutinfo.bindingCount = 3;
	descsetlayoutinfo.pBindings = setLayoutBindings;
	ret = vkCreateDescriptorSetLayout(logicaldevice, &descsetlayoutinfo, 0, &descriptorSetLayout);
	if(VK_SUCCESS != ret){
		printf("error@vkCreateDescriptorSetLayout:%d\n",ret);
		return;
	}
	printf("DescriptorSetLayout=%p\n", descriptorSetLayout);


	//descriptorSet = descriptorsetlayout + xxx
	VkDescriptorSetAllocateInfo descsetallocInfo = {};
	descsetallocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descsetallocInfo.descriptorPool = descriptorPool;
	descsetallocInfo.descriptorSetCount = 1;
	descsetallocInfo.pSetLayouts = &descriptorSetLayout;
	ret = vkAllocateDescriptorSets(logicaldevice, &descsetallocInfo, &descriptorSet);
	if(VK_SUCCESS != ret){
		printf("error@vkAllocateDescriptorSets\n");
		return;
	}
	printf("DescriptorSets=%p\n", descriptorSet);


	VkPushConstantRange pushconstant;
	pushconstant.offset = 0;
	pushconstant.size = 16;
	pushconstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


	//pipelinelayoutpipelineLayout = descriptorsetlayout + xxx
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushconstant;
	ret = vkCreatePipelineLayout(logicaldevice, &pipelineLayoutInfo, 0, &pipelineLayout);
	if(VK_SUCCESS != ret){
		printf("error@vkCreatePipelineLayout\n");
		return;
	}
	printf("PipelineLayout=%p\n", pipelineLayout);


	//pipelinecache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	ret = vkCreatePipelineCache(logicaldevice, &pipelineCacheCreateInfo, 0, &pipelineCache);
	if(VK_SUCCESS != ret){
		printf("error@vkCreatePipelineCache\n");
		return;
	}
	printf("PipelineCache=%p\n", pipelineCache);


	//shader
	ret = createShaderModule(&shaderModule, "shader.comp.spv");
	if(ret < 0){
		printf("error:%d@createShaderModule:compute\n",ret);
		return;
	}
	printf("ShaderModule=%p\n", shaderModule);

	VkPipelineShaderStageCreateInfo shaderStageinfo = {};
	shaderStageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageinfo.module = shaderModule;
	shaderStageinfo.pName = "main";


	//computepipeline = shader + pipelinelayout + pipeline cache
	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.stage = shaderStageinfo;
	computePipelineCreateInfo.layout = pipelineLayout;
	ret = vkCreateComputePipelines(logicaldevice, pipelineCache, 1, &computePipelineCreateInfo, 0, &computepipeline);
	if(VK_SUCCESS != ret){
		printf("error@vkCreateComputePipelines\n");
		return;
	}
	printf("ComputePipelines=%p\n", computepipeline);


	//command buffer
	VkCommandBufferAllocateInfo cmdbufallocInfo = {};
	cmdbufallocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdbufallocInfo.commandPool = computePool;
	cmdbufallocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdbufallocInfo.commandBufferCount = 1;
	ret = vkAllocateCommandBuffers(logicaldevice, &cmdbufallocInfo, &commandBuffer);
	if(VK_SUCCESS != ret){
		printf("error@vkAllocateCommandBuffers\n");
		return;
	}
	printf("CommandBuffers=%p\n", commandBuffer);


	//fence
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	ret = vkCreateFence(logicaldevice, &fenceCreateInfo, 0, &computefence);
	if(VK_SUCCESS != ret){
		printf("error@vkCreateFence\n");
		return;
	}
	printf("Fence=%p\n", computefence);
}


void vulkan_myctx_create()
{
	outputbuffersize = 4*ydim;
	vectorbuffersize = 4*xdim;
	matrixbuffersize = 4*xdim*ydim;

	physicaldevice = vulkan_device_create(2, 0);
	if(0 == physicaldevice)return;

	//vulkan_physicaldevice_logicdevice(&physicaldevice, &logicaldevice);

	//vulkan_computequeue_computepool(&computeQueue, &computePool);

	//compute pipeline
	createcomputepipeline();

	for(int j=0;j<3;j++){
		int size = 0;
		if(j==0)size = outputbuffersize;
		if(j==1)size = vectorbuffersize;
		if(j==2)size = matrixbuffersize;

		//gpu mem
		printf("gpumem%d: allocing\n", j);
		createBuffer(
			&deviceBuffer[j],
			&deviceMemory[j],
			physicaldevice,
			logicaldevice,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			size,
			0
		);
		printf("gpumem%d: fd=%p,mem=%p\n", j, deviceBuffer[j], deviceMemory[j]);

		//cpu mem
		printf("cpumem%d: allocing\n", j);
		createBuffer(
			&hostBuffer[j],
			&hostMemory[j],
			physicaldevice,
			logicaldevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			size,
			0
		);
		printf("cpumem%d: fd=%p,mem=%p\n", j, hostBuffer[j], hostMemory[j]);
	}

	//update data
	VkDescriptorBufferInfo bufferDescriptor[3] = {};
	VkWriteDescriptorSet descriptorWrites[3] = {};
	for(int j=0;j<3;j++){
		bufferDescriptor[j].buffer = deviceBuffer[j];
		bufferDescriptor[j].offset = 0;
		bufferDescriptor[j].range = VK_WHOLE_SIZE;

		descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[j].dstSet = descriptorSet;
		descriptorWrites[j].dstBinding = j;
		descriptorWrites[j].dstArrayElement = 0;
		descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[j].descriptorCount = 1;
		descriptorWrites[j].pBufferInfo = &bufferDescriptor[j];
	}
	vkUpdateDescriptorSets(logicaldevice, 3, descriptorWrites, 0, NULL);
}
void vulkan_myctx_delete()
{
	vkQueueWaitIdle(computeQueue);
/*
	vkDestroyBuffer(logicaldevice, deviceBuffer, 0);
	vkFreeMemory(logicaldevice, deviceMemory, 0);
	vkDestroyBuffer(logicaldevice, hostBuffer, 0);
	vkFreeMemory(logicaldevice, hostMemory, 0);

	vkDestroyPipelineLayout(logicaldevice, pipelineLayout, 0);
	vkDestroyDescriptorSetLayout(logicaldevice, descriptorSetLayout, 0);
	vkDestroyDescriptorPool(logicaldevice, descriptorPool, 0);
	vkDestroyPipeline(logicaldevice, pipeline, 0);
	vkDestroyPipelineCache(logicaldevice, pipelineCache, 0);
	vkDestroyFence(logicaldevice, fence, 0);
	vkDestroyCommandPool(logicaldevice, commandPool, 0);
	vkDestroyShaderModule(logicaldevice, shaderModule, 0);
*/
	vulkan_device_delete(physicaldevice);
}




void gpu_compute()
{
	int x,y;

	//compute work
	VkCommandBufferBeginInfo cmdBufbeginInfo = {};
	cmdBufbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	int ret = vkBeginCommandBuffer(commandBuffer, &cmdBufbeginInfo);

	//command copy from cpu to gpu
	VkBufferCopy vectorcopyregion = {};
	vectorcopyregion.size = vectorbuffersize;
	vkCmdCopyBuffer(commandBuffer, hostBuffer[1], deviceBuffer[1], 1, &vectorcopyregion);

	VkBufferCopy matrixcopyregion = {};
	matrixcopyregion.size = matrixbuffersize;
	vkCmdCopyBuffer(commandBuffer, hostBuffer[2], deviceBuffer[2], 1, &matrixcopyregion);

	// Barrier to ensure that input buffer transfer is finished before compute shader reads from it
	VkBufferMemoryBarrier bufferBarrier[2] = {};
	bufferBarrier[0].buffer = deviceBuffer[1];
	bufferBarrier[0].size = VK_WHOLE_SIZE;
	bufferBarrier[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	bufferBarrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bufferBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier[1].buffer = deviceBuffer[2];
	bufferBarrier[1].size = VK_WHOLE_SIZE;
	bufferBarrier[1].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	bufferBarrier[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	bufferBarrier[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, 0,
		2, &bufferBarrier[0],
		0, 0);

	//compute command
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computepipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, 0);
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 16, pushconst);

	vkCmdDispatch(commandBuffer, ydim/32, 1/1, 1/1);

	// Barrier to ensure that shader writes are finished before buffer is read back from GPU
	bufferBarrier[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	bufferBarrier[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	bufferBarrier[0].buffer = deviceBuffer[0];
	bufferBarrier[0].size = VK_WHOLE_SIZE;
	bufferBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, 0,
		1, &bufferBarrier[0],
		0, 0);

	// Read back to host visible buffer
	vectorcopyregion.size = outputbuffersize;
	vkCmdCopyBuffer(commandBuffer, deviceBuffer[0], hostBuffer[0], 1, &vectorcopyregion);

	// Barrier to ensure that buffer copy is finished before host reading from it
	bufferBarrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	bufferBarrier[0].dstAccessMask = VK_ACCESS_HOST_READ_BIT;
	bufferBarrier[0].buffer = hostBuffer[0];
	bufferBarrier[0].size = VK_WHOLE_SIZE;
	bufferBarrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_HOST_BIT,
		0,
		0, 0,
		1, &bufferBarrier[0],
		0, 0);

	ret = vkEndCommandBuffer(commandBuffer);

	// Submit compute work
	vkResetFences(logicaldevice, 1, &computefence);
	const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo computeSubmitInfo = {};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computefence);
}
void cpu_compute(float* tmp0, float* tmp1, float* tmp2)
{
	int x,y;
	for(y=0;y<ydim;y++){
		float tmp = 0.0;
		for(x=0;x<xdim;x++){
		tmp += tmp2[y*xdim+x] * tmp1[x];
		}
		tmp0[y] = tmp;
	}
}
void vulkan_bf16tofloat(u32* out, u16* in, int cnt)
{
	int x;
	for(x=0;x<cnt;x++){
		out[x] = (u32)in[x]<<16;
	}
}
void vulkan_muladd(float* xout, float* xin, __bf16* w, int n, int d, int handle)
{
	unsigned long long t0 = time_in_ns();

	xdim = n;
	ydim = d;
	outputbuffersize = 4*ydim;
	vectorbuffersize = 4*xdim;
	matrixbuffersize = 4*xdim*ydim;
	pushconst[0] = xdim;
	pushconst[1] = ydim;

	float* tmp1;
	float* tmp2;
	vkMapMemory(logicaldevice, hostMemory[1], 0, VK_WHOLE_SIZE, 0, (void*)&tmp1);
	vkMapMemory(logicaldevice, hostMemory[2], 0, VK_WHOLE_SIZE, 0, (void*)&tmp2);

	VkMappedMemoryRange mappedRange[3] = {};
	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[1];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	mappedRange[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[1].memory = hostMemory[2];
	mappedRange[1].offset = 0;
	mappedRange[1].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 2, mappedRange);

	unsigned long long t1 = time_in_ns();

	int x,y;
	for(x=0;x<xdim;x++){
		tmp1[x] = xin[x];
	}
	vulkan_bf16tofloat((void*)tmp2, (void*)w, xdim*ydim);

	unsigned long long t2 = time_in_ns();

	gpu_compute();

	unsigned long long t3 = time_in_ns();

	vkWaitForFences(logicaldevice, 1, &computefence, VK_TRUE, UINT64_MAX);

	unsigned long long t4 = time_in_ns();

	float* tmp0;
	vkMapMemory(logicaldevice, hostMemory[0], 0, VK_WHOLE_SIZE, 0, (void*)&tmp0);

	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[0];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 1, mappedRange);

	//cpu_compute(tmp0,tmp1,tmp2);
	//printf("cpu_compute:xdim=%d,ydim=%d,first=%f,last=%f\n",xdim,ydim,xout[0],xout[ydim-1]);

	for(y=0;y<ydim;y++)xout[y] = tmp0[y];
	//printf("gpu_compute:xdim=%d,ydim=%d,first=%f,767=%f,last=%f\n",xdim,ydim,xout[0],xout[767],xout[ydim-1]);

	vkUnmapMemory(logicaldevice, hostMemory[2]);
	vkUnmapMemory(logicaldevice, hostMemory[1]);
	vkUnmapMemory(logicaldevice, hostMemory[0]);

	unsigned long long t5 = time_in_ns();

	//printf("%f,%f,%f,%f,%f\n", (t1-t0)*1e-9, (t2-t1)*1e-9, (t3-t2)*1e-9, (t4-t3)*1e-9, (t5-t4)*1e-9);
}
void vulkan_muladd2(
	float* xout0, float* xin0, __bf16* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, __bf16* w1, int n1, int d1, int handle1)
{
	unsigned long long t0 = time_in_ns();

	xdim = n0;		//must ensure n0=n1=n2
	ydim = d0+d1;
	outputbuffersize = 4*ydim;
	vectorbuffersize = 4*xdim;
	matrixbuffersize = 4*xdim*ydim;
	pushconst[0] = xdim;
	pushconst[1] = ydim;

	float* tmp1;
	float* tmp2;
	vkMapMemory(logicaldevice, hostMemory[1], 0, VK_WHOLE_SIZE, 0, (void*)&tmp1);
	vkMapMemory(logicaldevice, hostMemory[2], 0, VK_WHOLE_SIZE, 0, (void*)&tmp2);

	VkMappedMemoryRange mappedRange[3] = {};
	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[1];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	mappedRange[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[1].memory = hostMemory[2];
	mappedRange[1].offset = 0;
	mappedRange[1].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 2, mappedRange);

	unsigned long long t1 = time_in_ns();

	int x,y;
	for(x=0;x<n0;x++)tmp1[x] = xin0[x];
	for(x=0;x<n1;x++)tmp1[n0+x] = xin1[x];
	vulkan_bf16tofloat((void*)tmp2, (void*)w0, n0*d0);
	vulkan_bf16tofloat((void*)&tmp2[n0*d0], (void*)w1, n1*d1);

	unsigned long long t2 = time_in_ns();

	gpu_compute();

	unsigned long long t3 = time_in_ns();

	vkWaitForFences(logicaldevice, 1, &computefence, VK_TRUE, UINT64_MAX);

	unsigned long long t4 = time_in_ns();

	float* tmp0;
	vkMapMemory(logicaldevice, hostMemory[0], 0, VK_WHOLE_SIZE, 0, (void*)&tmp0);

	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[0];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 1, mappedRange);

	//cpu_compute(tmp0,tmp1,tmp2);
	//printf("cpu_compute:xdim=%d,ydim=%d,first=%f,last=%f\n",xdim,ydim,xout[0],xout[ydim-1]);

	for(y=0;y<d0;y++)xout0[y] = tmp0[y];
	for(y=0;y<d1;y++)xout1[y] = tmp0[d0+y];
	//printf("gpu_compute:xdim=%d,ydim=%d,first=%f,767=%f,last=%f\n",xdim,ydim,xout[0],xout[767],xout[ydim-1]);

	vkUnmapMemory(logicaldevice, hostMemory[2]);
	vkUnmapMemory(logicaldevice, hostMemory[1]);
	vkUnmapMemory(logicaldevice, hostMemory[0]);

	unsigned long long t5 = time_in_ns();

	//printf("%f,%f,%f,%f,%f\n", (t1-t0)*1e-9, (t2-t1)*1e-9, (t3-t2)*1e-9, (t4-t3)*1e-9, (t5-t4)*1e-9);
}
void vulkan_muladd3(
	float* xout0, float* xin0, __bf16* w0, int n0, int d0, int handle0,
	float* xout1, float* xin1, __bf16* w1, int n1, int d1, int handle1,
	float* xout2, float* xin2, __bf16* w2, int n2, int d2, int handle2)
{
	unsigned long long t0 = time_in_ns();

	xdim = n0;		//must ensure n0=n1=n2
	ydim = d0+d1+d2;
	outputbuffersize = 4*ydim;
	vectorbuffersize = 4*xdim;
	matrixbuffersize = 4*xdim*ydim;
	pushconst[0] = xdim;
	pushconst[1] = ydim;

	float* tmp1;
	float* tmp2;
	vkMapMemory(logicaldevice, hostMemory[1], 0, VK_WHOLE_SIZE, 0, (void*)&tmp1);
	vkMapMemory(logicaldevice, hostMemory[2], 0, VK_WHOLE_SIZE, 0, (void*)&tmp2);

	VkMappedMemoryRange mappedRange[3] = {};
	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[1];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	mappedRange[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[1].memory = hostMemory[2];
	mappedRange[1].offset = 0;
	mappedRange[1].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 2, mappedRange);

	unsigned long long t1 = time_in_ns();

	int x,y;
	for(x=0;x<n0;x++)tmp1[x      ] = xin0[x];
	for(x=0;x<n1;x++)tmp1[n0+x   ] = xin1[x];
	for(x=0;x<n2;x++)tmp1[n0+n1+x] = xin2[x];
	vulkan_bf16tofloat((void*)tmp2, (void*)w0, n0*d0);
	vulkan_bf16tofloat((void*)&tmp2[n0*d0], (void*)w1, n1*d1);
	vulkan_bf16tofloat((void*)&tmp2[n0*d0+n1*d1], (void*)w2, n2*d2);

	unsigned long long t2 = time_in_ns();

	gpu_compute();

	unsigned long long t3 = time_in_ns();

	vkWaitForFences(logicaldevice, 1, &computefence, VK_TRUE, UINT64_MAX);

	unsigned long long t4 = time_in_ns();

	float* tmp0;
	vkMapMemory(logicaldevice, hostMemory[0], 0, VK_WHOLE_SIZE, 0, (void*)&tmp0);

	mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange[0].memory = hostMemory[0];
	mappedRange[0].offset = 0;
	mappedRange[0].size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(logicaldevice, 1, mappedRange);

	//cpu_compute(tmp0,tmp1,tmp2);
	//printf("cpu_compute:xdim=%d,ydim=%d,first=%f,last=%f\n",xdim,ydim,xout[0],xout[ydim-1]);

	for(y=0;y<d0;y++)xout0[y] = tmp0[y      ];
	for(y=0;y<d1;y++)xout1[y] = tmp0[d0+y   ];
	for(y=0;y<d2;y++)xout2[y] = tmp0[d0+d1+y];
	//printf("gpu_compute:xdim=%d,ydim=%d,first=%f,767=%f,last=%f\n",xdim,ydim,xout[0],xout[767],xout[ydim-1]);

	vkUnmapMemory(logicaldevice, hostMemory[2]);
	vkUnmapMemory(logicaldevice, hostMemory[1]);
	vkUnmapMemory(logicaldevice, hostMemory[0]);

	unsigned long long t5 = time_in_ns();

	//printf("%f,%f,%f,%f,%f\n", (t1-t0)*1e-9, (t2-t1)*1e-9, (t3-t2)*1e-9, (t4-t3)*1e-9, (t5-t4)*1e-9);
}
/*
int main()
{
        //init
        void* ins = vulkan_init(0, 0);
        if(0 == ins)return -1;

        //vulkan: things
        vulkan_myctx_create(0, 0);

        //once
        drawframe();

        //vulkan
        vulkan_myctx_delete();

        //exit
        vulkan_exit();
        return 0;
}*/
