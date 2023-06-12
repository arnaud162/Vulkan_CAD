/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/



#include <QVulkanFunctions>
#include <QtConcurrentRun>
#include <QFile>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <vector>
#include "trianglerenderer.h"
#include "lecture_csv_qt.h"


// Note that the vertex data and the projection matrix assume OpenGL. With
// Vulkan Y is negated in clip space and the near/far plane is at 0/1 instead
// of -1/1. These will be corrected for by an extra transformation when
// calculating the modelview-projection matrix.



struct Pair verticesEtIndices = lecture_csv();
std::vector<Vertex> vertices =verticesEtIndices.vertices;
std::vector<Vertex> verticesOriginaux = vertices;
std::vector<uint32_t> indices = verticesEtIndices.indices;
vector <vector<int>> correspondanceVueExcelVueVulkan = verticesEtIndices.correspondanceVueExcelVueVulkan;
int reloadAPartirDuFichieNecessaire =0;

struct UniformBufferObject {
    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 proj;
};


TriangleRenderer::TriangleRenderer(QVulkanWindow *w, bool msaa)
    : m_window(w),
    m_cam(QVector3D(0.0f, 0.0f, 0.0f)) // starting camera position
{
     pvertices=&vertices;
    if (msaa) {
        const QVector<int> counts = w->supportedSampleCounts();
        qDebug() << "Supported sample counts:" << counts;
        for (int s = 16; s >= 4; s /= 2) {
            if (counts.contains(s)) {
                qDebug("Requesting sample count %d", s);
                m_window->setSampleCount(s);
                break;
            }
        }
    }
}

VkShaderModule TriangleRenderer::createShader(const QString &name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Failed to read shader %s", qPrintable(name));
        return VK_NULL_HANDLE;
    }
    QByteArray blob = file.readAll();
    file.close();

    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(shaderInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = blob.size();
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(blob.constData());
    VkShaderModule shaderModule;
    VkResult err = m_devFuncs->vkCreateShaderModule(m_window->device(), &shaderInfo, nullptr, &shaderModule);
    if (err != VK_SUCCESS) {
        qWarning("Failed to create shader module: %d", err);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

void TriangleRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkDevice dev = m_window->device();
        VkBufferCreateInfo bufferInfo{};
        memset(&bufferInfo, 0, sizeof(bufferInfo));
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult err = m_devFuncs->vkCreateBuffer(dev, &bufferInfo, nullptr, &buffer);
        if (err != VK_SUCCESS)
            qFatal("Failed to create buffer: %d", err);


        VkMemoryRequirements memRequirements;
        m_devFuncs->vkGetBufferMemoryRequirements(dev, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo={
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            nullptr,
            memRequirements.size,
            m_window->hostVisibleMemoryIndex()};
        //allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (m_devFuncs->vkAllocateMemory(dev, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        m_devFuncs->vkBindBufferMemory(dev, buffer, bufferMemory, 0);
    }

void TriangleRenderer::createUbDpDs( VkDevice dev, int concurrentFrameCount){
   //Uniform Buffers, Descriptor Pool, Descriptor Set
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(concurrentFrameCount); // concurrentFrameCount correspond à swapchainimages.size()
    uniformBuffersMemory.resize(concurrentFrameCount);
    for (size_t i = 0; i < concurrentFrameCount; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        }

       //correspond à createDescriptorPool()
       VkDescriptorPoolSize poolSize{};
       poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       poolSize.descriptorCount = static_cast<uint32_t>(concurrentFrameCount);

       VkDescriptorPoolCreateInfo poolInfo{};
       poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
       poolInfo.poolSizeCount = 1;
       poolInfo.pPoolSizes = &poolSize;

       poolInfo.maxSets = static_cast<uint32_t>(concurrentFrameCount);

       if (m_devFuncs->vkCreateDescriptorPool(dev, &poolInfo, nullptr, &m_descPool) != VK_SUCCESS) {
           throw std::runtime_error("echec de la creation de la pool de descripteurs!");
       }

       //correspond à createDescriptorSets()
       std::vector<VkDescriptorSetLayout> layouts(concurrentFrameCount, m_descSetLayout);
       VkDescriptorSetAllocateInfo allocInfo{};
       allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
       allocInfo.descriptorPool = m_descPool;
       allocInfo.descriptorSetCount = static_cast<uint32_t>(concurrentFrameCount);
       allocInfo.pSetLayouts = layouts.data();

       descriptorSets.resize(concurrentFrameCount);
       if (m_devFuncs->vkAllocateDescriptorSets(dev, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
           throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
       }

       for (int i = 0; i < concurrentFrameCount; i++) {
           VkDescriptorBufferInfo bufferInfo{};
           bufferInfo.buffer = uniformBuffers[i];
           bufferInfo.offset = 0;
           bufferInfo.range = sizeof(UniformBufferObject);

           VkWriteDescriptorSet descriptorWrite{};
           descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
           descriptorWrite.dstSet = descriptorSets[i];
           descriptorWrite.dstBinding = 0;
           descriptorWrite.dstArrayElement = 0;
           descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
           descriptorWrite.descriptorCount = 1;
           descriptorWrite.pBufferInfo = &bufferInfo;
           descriptorWrite.pImageInfo = nullptr; // Optionnel
           descriptorWrite.pTexelBufferView = nullptr; // Optionnel

           m_devFuncs->vkUpdateDescriptorSets(dev, 1, &descriptorWrite, 0, nullptr);
       }

}

void TriangleRenderer::initResources()
{
    qDebug("initResources");

    VkDevice dev = m_window->device();
    m_devFuncs = m_window->vulkanInstance()->deviceFunctions(dev);

    /*Prepare the vertex and uniform data. The vertex data will never
    // change so one buffer is sufficient regardless of the value of
    // QVulkanWindow::CONCURRENT_FRAME_COUNT. Uniform data is changing per
    // frame however so active frames have to have a dedicated copy.

    // Use just one memory allocation and one buffer. We will then specify the
    // appropriate offsets for uniform buffers in the VkDescriptorBufferInfo.
    // Have to watch out for
    // VkPhysicalDeviceLimits::minUniformBufferOffsetAlignment, though.

    // The uniform buffer is not strictly required in this example, we could
    // have used push constants as well since our single matrix (64 bytes) fits
    // into the spec mandated minimum limit of 128 bytes. However, once that
    // limit is not sufficient, the per-frame buffers, as shown below, will
    // become necessary.*/


    const int concurrentFrameCount = m_window->concurrentFrameCount();
    const VkPhysicalDeviceLimits *pdevLimits = &m_window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
    qDebug("uniform buffer offset alignment is %u", (uint) uniAlign);

    quint8 *p;
    VkResult err;
//    VkDeviceSize vBufferSize = sizeof(vertices[0]) * vertices.size();

////    createBuffer(vertexAllocSize+ concurrentFrameCount * uniformAllocSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,0,m_VertexBuf,m_VertexBufMem);
//    createBuffer(vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,0,m_VertexBuf,m_VertexBufMem);


//    err = m_devFuncs->vkMapMemory(dev, m_VertexBufMem, 0, vBufferSize, 0, reinterpret_cast<void **>(&p));

//    if (err != VK_SUCCESS)
//        qFatal("Failed to map vertex memory: %d", err);
//    memcpy(p, vertices.data(), (size_t) vBufferSize);

//    m_devFuncs->vkUnmapMemory(dev, m_VertexBufMem);


//    VkDeviceSize iBufferSize = sizeof(indices[0]) * indices.size();
////    createBuffer(indexAllocSize+ concurrentFrameCount * uniformAllocSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT,0,m_IndexBuf,m_IndexBufMem);
//    createBuffer(iBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT,0,m_IndexBuf,m_IndexBufMem);

//   err=m_devFuncs->vkMapMemory(dev, m_IndexBufMem,0,iBufferSize,0,reinterpret_cast<void **>(&p));
//    if (err != VK_SUCCESS)
//        qFatal("Failed to map index memory: %d", err);
//    memcpy(p, indices.data(), iBufferSize);
//    m_devFuncs->vkUnmapMemory(dev, m_IndexBufMem);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto vertexBindingDesc = Vertex::getBindingDescription();
    auto vertexAttrDesc = Vertex::getAttributeDescriptions();

    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

    // Set up descriptor set and its layout.
    VkDescriptorPoolSize descPoolSizes = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(concurrentFrameCount) };
    VkDescriptorPoolCreateInfo descPoolInfo;
    memset(&descPoolInfo, 0, sizeof(descPoolInfo));
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = concurrentFrameCount;
    descPoolInfo.poolSizeCount = 1;
    descPoolInfo.pPoolSizes = &descPoolSizes;
    err = m_devFuncs->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &m_descPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor pool: %d", err);

    VkDescriptorSetLayoutBinding layoutBinding = {
        0, // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr
    };

    VkDescriptorSetLayoutCreateInfo descLayoutInfo{} ;
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = 1;
    descLayoutInfo.pBindings = &layoutBinding;

    err = m_devFuncs->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &m_descSetLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create descriptor set layout: %d", err);



    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = m_devFuncs->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline cache: %d", err);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; //1 selon vulkan tutorial, mettre à 1 selon le message des validatations layers
    //pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout;
    err = m_devFuncs->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (err != VK_SUCCESS)
        qFatal("Failed to create pipeline layout: %d", err);


    // Shaders
    VkShaderModule vertShaderModule = createShader(QStringLiteral(":/color_vert.spv"));
    VkShaderModule fragShaderModule = createShader(QStringLiteral(":/color_frag.spv"));

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShaderModule,
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShaderModule,
            "main",
            nullptr
        }
    };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia;
    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo.pInputAssemblyState = &ia;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp;
    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineRasterizationStateCreateInfo rs;
    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms;
    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // Enable multisampling.
    ms.rasterizationSamples = m_window->sampleCountFlagBits();
    pipelineInfo.pMultisampleState = &ms;

    VkPipelineDepthStencilStateCreateInfo ds;
    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendStateCreateInfo cb;
    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // no blend, write out all of rgba. en fait non
    VkPipelineColorBlendAttachmentState att;
    memset(&att, 0, sizeof(att));
    att.colorWriteMask = 0xF;
    cb.attachmentCount = 1;
    cb.pAttachments = &att;
    pipelineInfo.pColorBlendState = &cb;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;

    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_window->defaultRenderPass();

    err = m_devFuncs->vkCreateGraphicsPipelines(dev, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (err != VK_SUCCESS)
        qFatal("Failed to create graphics pipeline: %d", err);

    if (vertShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, vertShaderModule, nullptr);
    if (fragShaderModule)
        m_devFuncs->vkDestroyShaderModule(dev, fragShaderModule, nullptr);
}


void TriangleRenderer::releaseSwapChainResources()
{
    VkDevice dev = m_window->device();
    qDebug("releaseSwapChainResources");

    for (size_t i = 0; i < m_window->concurrentFrameCount(); i++) {
            m_devFuncs->vkDestroyBuffer(dev, uniformBuffers[i], nullptr);
            m_devFuncs->vkFreeMemory(dev, uniformBuffersMemory[i], nullptr);
        }

    m_devFuncs-> vkDestroyDescriptorPool(dev, m_descPool, nullptr);

}

void TriangleRenderer::releaseResources()
{
    qDebug("releaseResources");

    VkDevice dev = m_window->device();

    if (m_pipeline) {
        m_devFuncs->vkDestroyPipeline(dev, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout) {
        m_devFuncs->vkDestroyPipelineLayout(dev, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineCache) {
        m_devFuncs->vkDestroyPipelineCache(dev, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_descSetLayout) {
        m_devFuncs->vkDestroyDescriptorSetLayout(dev, m_descSetLayout, nullptr);
        m_descSetLayout = VK_NULL_HANDLE;
    }

    if (m_descPool) {
        m_devFuncs->vkDestroyDescriptorPool(dev, m_descPool, nullptr);
        m_descPool = VK_NULL_HANDLE;
    }

    if (m_VertexBuf) {
        m_devFuncs->vkDestroyBuffer(dev, m_VertexBuf, nullptr);
        m_VertexBuf = VK_NULL_HANDLE;
    }

    if (m_VertexBufMem) {
        m_devFuncs->vkFreeMemory(dev, m_VertexBufMem, nullptr);
        m_VertexBufMem = VK_NULL_HANDLE;
    }

    if (m_IndexBuf) {
        m_devFuncs->vkDestroyBuffer(dev, m_IndexBuf, nullptr);
        m_IndexBuf = VK_NULL_HANDLE;
    }

    if (m_IndexBufMem) {
        m_devFuncs->vkFreeMemory(dev, m_IndexBufMem, nullptr);
        m_IndexBufMem = VK_NULL_HANDLE;
    }


}

void TriangleRenderer::startNextFrame()
{
    VkDevice dev = m_window->device();



if( necessaryChangeInbuffers!=0 or reloadAPartirDuFichieNecessaire){
    if (necessaryChangeInbuffers==-2){ // situation où un élément a été deselectionné
        qDebug() << "renderer pov : model should be reloaded now, 2";
        //vertices = verticesOriginaux;
    }

    else if (necessaryChangeInbuffers==-1){ // situation où un élément a été deselectionné
        vertices = verticesOriginaux;
    }
    else if (necessaryChangeInbuffers>=1){ // devrait être >=1
        int necessaryChangeInbuffersDeebug = necessaryChangeInbuffers;
        vertices = verticesOriginaux; // si on clique sur deux elmts selectionnes a la suite les deux sont select. cette ligne évite cette situation
        int indicePremierVertexDeLelementN = correspondanceVueExcelVueVulkan[necessaryChangeInbuffers+1][0]; // necessaryChangeInbuffers+1 car la première ligne est la légende
        int indiceDernierVertexDeLelementN = correspondanceVueExcelVueVulkan[necessaryChangeInbuffers+1][1];
        for (int i =indicePremierVertexDeLelementN; i<indiceDernierVertexDeLelementN;i++){
            vertices[i].color={255,555,0};
        }
    }

    if(reloadAPartirDuFichieNecessaire){ // rechargement du modéle quand l'utilisateur modifie
        verticesEtIndices = lecture_csv();
        vertices =verticesEtIndices.vertices;
        verticesOriginaux = vertices;
        indices = verticesEtIndices.indices;
        correspondanceVueExcelVueVulkan = verticesEtIndices.correspondanceVueExcelVueVulkan;
        reloadAPartirDuFichieNecessaire=0;
    }

    qDebug()<<"initialisation des buffers vertex et indices";

    quint8 *p;
    VkDeviceSize vBufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,0,m_VertexBuf,m_VertexBufMem);

    VkResult err = m_devFuncs->vkMapMemory(dev, m_VertexBufMem, 0, vBufferSize, 0, reinterpret_cast<void **>(&p));


    if (err != VK_SUCCESS)
        qFatal("Failed to map vertex memory: %d", err);
    memcpy(p, vertices.data(), (size_t) vBufferSize);

    m_devFuncs->vkUnmapMemory(dev, m_VertexBufMem);

    VkDeviceSize iBufferSize = sizeof(indices[0]) * indices.size();
 //    createBuffer(indexAllocSize+ concurrentFrameCount * uniformAllocSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT,0,m_IndexBuf,m_IndexBufMem);
    createBuffer(iBufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT,0,m_IndexBuf,m_IndexBufMem);

    err=m_devFuncs->vkMapMemory(dev, m_IndexBufMem,0,iBufferSize,0,reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map index memory: %d", err);
    memcpy(p, indices.data(), iBufferSize);
    m_devFuncs->vkUnmapMemory(dev, m_IndexBufMem);

    necessaryChangeInbuffers=0;
}






    VkCommandBuffer cb = m_window->currentCommandBuffer();
    const QSize sz = m_window->swapChainImageSize();

    VkClearColorValue clearColor = {{ 0, 0, 0, 1 }};
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[3];
    memset(clearValues, 0, sizeof(clearValues));
    clearValues[0].color = clearValues[2].color = clearColor;
    clearValues[1].depthStencil = clearDS;

    VkRenderPassBeginInfo rpBeginInfo;
    memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = m_window->sampleCountFlagBits() > VK_SAMPLE_COUNT_1_BIT ? 3 : 2;
    rpBeginInfo.pClearValues = clearValues;






    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Logique de temps écoulé
    /*static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();l
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/
    UniformBufferObject ubo{};

    ubo.model = m_window -> clipCorrectionMatrix();
    ubo.model.perspective(45.0f, (float)sz.width()/sz.height(), 0.01f, 500.0f);
    ubo.model.lookAt({15.0f,15.0f,30.0f}, {0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f});
    ubo.model.rotate(250.0f,0.0f,1.0f,0.0f);

   ubo.model = ubo.model*m_cam.viewMatrix();


  /*

    ubo.model.lookAt(QVector3D (1,1,1), QVector3D(0,0,0), QVector3D(0,0,1));
    ubo.model.rotate(time * 30,0, 0, 1);
    ubo.view.lookAt(QVector3D (1,1,1), QVector3D(0,0,0), QVector3D(0,0,1));
    ubo.proj.perspective(-45.0f, (float)sz.width()/sz.height(), 0.01f, 1000.0f);
    float values;
    ubo.model.copyDataTo(&values);

   */


   //int concurrentFrameCount = m_window->concurrentFrameCount();
   createUbDpDs(dev, m_window->concurrentFrameCount());






    void* data;
    m_devFuncs->vkMapMemory(dev, uniformBuffersMemory[m_window->currentFrame()], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
    m_devFuncs->vkUnmapMemory(dev, uniformBuffersMemory[m_window->currentFrame()]);
    /*quint8 *p;
    VkResult err = m_devFuncs->vkMapMemory(dev, m_VertexBufMem, m_uniformBufInfo[m_window->currentFrame()].offset,
            UNIFORM_DATA_SIZE, 0, reinterpret_cast<void **>(&p));
    if (err != VK_SUCCESS)
        qFatal("Failed to map memory: %d", err);
    QMatrix4x4 m = m_proj;
    m.rotate(m_rotation, 0, 1, 0);
    memcpy(p, m.constData(), 16 * sizeof(float));
    m_devFuncs->vkUnmapMemory(dev, m_VertexBufMem);*/

    // Not exactly a real animation system, just advance on every frame for now.
//    m_rotation += 1.0f;

    m_devFuncs->vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    //m_devFuncs->vkCmdDraw(cb, 4, 1,0,  0);


    VkDeviceSize vbOffset = 0;
    m_devFuncs->vkCmdBindVertexBuffers(cb, 0, 1, &m_VertexBuf, &vbOffset);
    m_devFuncs->vkCmdBindIndexBuffer(cb, m_IndexBuf,0, VK_INDEX_TYPE_UINT32);
    m_devFuncs->vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,&descriptorSets[m_window->currentFrame()], 0, nullptr);

    VkViewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = sz.width();
    viewport.height = sz.height();
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    m_devFuncs->vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    m_devFuncs->vkCmdSetScissor(cb, 0, 1, &scissor);

    m_devFuncs->vkCmdDrawIndexed(cb,indices.size() , 1,0,0,  0);

    m_devFuncs->vkCmdEndRenderPass(cmdBuf);

    m_window->frameReady();
    m_window->requestUpdate(); // render continuously, throttled by the presentation rate
}
void TriangleRenderer::yaw(float degrees)
{
    QMutexLocker locker(&m_guiMutex);
    m_cam.yaw(degrees);
    //markViewProjDirty();
}

void TriangleRenderer::pitch(float degrees)
{
    QMutexLocker locker(&m_guiMutex);
    m_cam.pitch(degrees);
    //markViewProjDirty();
}

void TriangleRenderer::walk(float amount)
{
    QMutexLocker locker(&m_guiMutex);
    m_cam.walk(amount);
    //markViewProjDirty();
}

void TriangleRenderer::strafe(float amount)
{
    QMutexLocker locker(&m_guiMutex);
    m_cam.strafe(amount);
    //markViewProjDirty();
}

void TriangleRenderer::rotate(float degrees)
{
    QMutexLocker locker(&m_guiMutex);
    m_cam.rotate(degrees);
    //markViewProjDirty();
}

void TriangleRenderer::changeColor(int codeChangementCouleur)
{
    //codeChangementCouleur  0 : on fait rien  -1 on revient aux couleurs initiales, 1 ou + on colorie un élément specifique
    necessaryChangeInbuffers=codeChangementCouleur;
}

void TriangleRenderer::reloadVkModel()
{
    qDebug() << "renderer pov : model will be reloaded now";
    reloadAPartirDuFichieNecessaire=1; // cause une segmentation fault pour une raison incompréhensible
}


