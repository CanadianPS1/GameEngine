#include "EngineDescriptors.hpp"
// std
#include <cassert>
#include <stdexcept>
namespace engine{
    EngineDescriptorSetLayout::Builder &EngineDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count){
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }
    std::unique_ptr<EngineDescriptorSetLayout> EngineDescriptorSetLayout::Builder::build() const {return std::make_unique<EngineDescriptorSetLayout>(etDevice, bindings);}
    EngineDescriptorSetLayout::EngineDescriptorSetLayout(EngineDevice &etDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : etDevice{etDevice}, bindings{bindings}{
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for(auto kv : bindings) setLayoutBindings.push_back(kv.second);
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        if(vkCreateDescriptorSetLayout(etDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor set layout!");
    }
    EngineDescriptorSetLayout::~EngineDescriptorSetLayout(){vkDestroyDescriptorSetLayout(etDevice.device(), descriptorSetLayout, nullptr);}
    EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count){
        poolSizes.push_back({descriptorType, count});
        return *this;
    }
    EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags){
        poolFlags = flags;
        return *this;
    }
    EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::setMaxSets(uint32_t count){
        maxSets = count;
        return *this;
    }
    std::unique_ptr<EngineDescriptorPool> EngineDescriptorPool::Builder::build() const {return std::make_unique<EngineDescriptorPool>(etDevice, maxSets, poolFlags, poolSizes);}
    EngineDescriptorPool::EngineDescriptorPool(EngineDevice &etDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes) : etDevice{etDevice}{
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;
        if(vkCreateDescriptorPool(etDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor pool!");
    } 
    EngineDescriptorPool::~EngineDescriptorPool(){vkDestroyDescriptorPool(etDevice.device(), descriptorPool, nullptr);}
    bool EngineDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;
        if(vkAllocateDescriptorSets(etDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) return false;
        return true;
    }
    void EngineDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {vkFreeDescriptorSets(etDevice.device(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());}
    void EngineDescriptorPool::resetPool(){vkResetDescriptorPool(etDevice.device(), descriptorPool, 0);}
    EngineDescriptorWriter::EngineDescriptorWriter(EngineDescriptorSetLayout &setLayout, EngineDescriptorPool &pool) : setLayout{setLayout}, pool{pool} {}
    EngineDescriptorWriter &EngineDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo){
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;
        writes.push_back(write);
        return *this;
    }
    EngineDescriptorWriter &EngineDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo){
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;
        writes.push_back(write);
        return *this;
    }
    bool EngineDescriptorWriter::build(VkDescriptorSet &set){
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if(!success) return false;
        overwrite(set);
        return true;
    }
    void EngineDescriptorWriter::overwrite(VkDescriptorSet &set){
        for(auto &write : writes) write.dstSet = set;
        vkUpdateDescriptorSets(pool.etDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }
}