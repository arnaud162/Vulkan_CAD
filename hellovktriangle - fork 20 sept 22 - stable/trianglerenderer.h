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

#include <QVulkanWindow>
#include <QMutex>
#include "camera.h"
#include <QVector2D>
#include "struct_vertex.h"




class TriangleRenderer : public QVulkanWindowRenderer
{
public:
    TriangleRenderer(QVulkanWindow *w, bool msaa = false);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void initResources() override;
    //void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void createUbDpDs( VkDevice dev, int concurrentFrameCount) ; //Uniform Buffers, Descriptor Pool, Descriptor Set

    void startNextFrame() override;

    void yaw(float degrees);
    void pitch(float degrees);
    void walk(float amount);
    void strafe(float amount);
    void rotate(float degerees);

    void changeColor(int code);

    void reloadVkModel();

    std::vector <Vertex> *pvertices;


protected:
    VkShaderModule createShader(const QString &name);

    QVulkanWindow *m_window;
    QVulkanDeviceFunctions *m_devFuncs;

    VkDeviceMemory m_VertexBufMem = VK_NULL_HANDLE;
    VkBuffer m_VertexBuf = VK_NULL_HANDLE;
    VkDeviceMemory m_IndexBufMem = VK_NULL_HANDLE;
    VkBuffer m_IndexBuf = VK_NULL_HANDLE;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorBufferInfo m_uniformBufInfo[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];

    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet[QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT];

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;


    QMatrix4x4 m_proj;
    float m_rotation = 0.0f;

    QMutex m_guiMutex;

private:
    Camera m_cam;
    bool m_pressed = false;
    QPoint m_lastPos;
    int necessaryChangeInbuffers = 1;


};
